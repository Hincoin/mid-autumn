#include "config.h"
#include <cmath>
#include <omp.h>
#include "cl_scene.h"
#include "photon_map.h"
#include "reflection.h"
#include "random_number_generator_mt19937.h"
#include "permuted_halton.h"

void estimate_e(photon_map_t* photon_map,photon_kd_tree_t* map,int count,
				const point3f_t& p, const normal3f_t& n,
				spectrum_t* ret)
{
	if(!map) {vclr(*ret);return;}
	//lookup nearby photons at irradiance computation point
	photon_process_data_t proc_data;	

	photon_process_data_init(&proc_data,photon_map->n_lookup);
	proc_data.photons = (close_photon_t*)alloca(photon_map->n_lookup *
		sizeof(close_photon_t));
	float md2 = photon_map->max_dist_squared;
	kd_tree_lookup(*map,p,(&proc_data),photon_process,md2);
	//accumulate irradiance value from nearby photons
	close_photon_t *photons = proc_data.photons;
	spectrum_t e;
	vclr(e);
	for (unsigned i = 0; i < proc_data.found_photons;++i)
	{
		if(vdot(n,photons[i].photon->wi) > 0.f)
		{
			vadd(e,e,photons[i].photon->alpha);
		}
	}
	vsmul(*ret,1.f/((float)count * md2 * FLOAT_PI),e);
}
INLINE bool unsuccessful(int needed, int found, int shot) {
		return (found < needed &&
		        (found == 0 || found < shot / 1024));
	}

void photon_map_preprocess(photon_map_t* photon_map,cl_scene_info_t scene_info,RandomNumberGeneratorMT19937& rng);

void photon_map_init(photon_map_t* photon_map,cl_scene_info_t scene_info,RandomNumberGeneratorMT19937 &rng)
{
	photon_map_preprocess(photon_map,scene_info,rng);
}

void photon_map_destroy(photon_map_t* photon_map)//free up kd-tree memory
{
	kd_tree_destroy<photon_t>(&(photon_map->caustic_map));
	kd_tree_destroy<photon_t>(&photon_map->indirect_map);
	kd_tree_destroy<radiance_photon_t>(&photon_map->radiance_map);
}
void photon_map_preprocess(photon_map_t* photon_map,cl_scene_info_t scene_info,RandomNumberGeneratorMT19937 &rng)
{
	Seed seed;
	seed.s1 = rng.RandomUnsignedInt();
	seed.s2 = rng.RandomUnsignedInt();
	seed.s3 = rng.RandomUnsignedInt();
	//seed.s3 = rng.RandomUnsignedInt();
	if (scene_info.lght_count == 0) return ;
	//
	using namespace std;
	vector<photon_t> caustic_photons;
	vector<photon_t> indirect_photons;
	vector<photon_t> direct_photons;
	vector<radiance_photon_t> radiance_photons;
	bool caustice_map_inited = false,indirect_map_inited = false,radiance_map_inited=false;
	photon_map->n_lookup = min(photon_map->n_lookup,MAX_CLOSE_PHOTON_LOOKUP);

	caustic_photons.reserve(photon_map->n_caustic_photons);
	indirect_photons.reserve(photon_map->n_indirect_photons);

	int n_shot=0;
	bool caustic_done = (photon_map->n_caustic_photons == 0);
	bool indirect_done = (photon_map->n_indirect_photons == 0);
	//compute light power cdf for photon shooting
	int n_lights = int(scene_info.lght_count);
	float *lights_power = (float*)alloca	(n_lights * sizeof(float));
	float *light_cdf = (float*)alloca((n_lights+1) * sizeof(float));
	spectrum_t lpower;
	for (int i = 0;i < n_lights; ++i)
	{
		light_power(scene_info.lghts+i,scene_info,&lpower);
		lights_power[i] = spectrum_y(&lpower);
	}
	float total_power;
	compute_step_1d_cdf(lights_power,n_lights,&total_power,light_cdf);
	//radiance photon reflectance arrays
	vector<spectrum_t> rp_reflectances,rp_transmittances;

	static const int halton_dimension = 8;
	PermutedHalton halton(halton_dimension,rng);
	while (!caustic_done || !indirect_done)
	{
		++n_shot;
		//give up if we are not storing enough photons
		if (n_shot > 500000 && 
			(unsuccessful(photon_map->n_caustic_photons,
			(unsigned)caustic_photons.size(),
			n_shot)) && 
			(unsuccessful(photon_map->n_indirect_photons,
			(unsigned)indirect_photons.size(),
			n_shot)))
		{
			return;
		}
		float u[halton_dimension];
		int total_shot = photon_map->total_photons + n_shot;
		halton.Sample(total_shot+1,u);

		//choose light of shoot photon from
		float lpdf;

		bool debug_tracing = false;//total_shot == 2;
		if(debug_tracing)
			printf("debug\nt");

		int lnum = (int)floor(sample_step_1d(lights_power,light_cdf,
			total_power,n_lights,u[0],&lpdf));
		lnum = min(lnum, n_lights - 1);

		light_info_t* light = scene_info.lghts+lnum;
		float pdf;
		ray_t photon_ray;
		normal3f_t nl;
		spectrum_t alpha;
		light_ray_sample_l(light,scene_info,u[1],u[2],u[3],u[4],
			&photon_ray,&nl,&pdf,&alpha);
		if (pdf == 0.f || color_is_black(alpha))continue;
		vsmul(alpha,(fabs(vdot(nl,photon_ray.d))/(pdf*lpdf)),alpha);
		
		//follow photon path through scene and record intersections
		bool specular_path = true;
		intersection_t photon_isect;
		int n_intersections = 0;
		spectrum_t ltranmittance;

	
		while (intersect(scene_info.accelerator_data,scene_info.shape_data,scene_info.primitives,scene_info.primitive_count,
					&photon_ray,&photon_isect))
		{
			++n_intersections;
			//alpha *= scene->transmittance
			scene_tranmittance(scene_info,&photon_ray,&ltranmittance);
			vmul(alpha,alpha,ltranmittance);
			vector3f_t wo ; 
			vneg(wo,photon_ray.d);
			bsdf_t photon_bsdf;

			intersection_get_bsdf(&photon_isect,scene_info,&photon_ray,
				&photon_bsdf);
			BxDFType specular_type = 
				BxDFType(BSDF_REFLECTION|BSDF_TRANSMISSION|BSDF_SPECULAR);
			bool has_non_specular = (
				photon_bsdf.n_bxdfs > bsdf_num_components(&photon_bsdf,specular_type)
				);
			if (has_non_specular)
			{
				//deposit photon at surface
				photon_t photon;
				photon_init(&photon,&photon_isect.dg.p,&alpha,&wo);
				
				bool deposited = false;
				if(n_intersections == 1)
				{
					if(!indirect_done && photon_map->final_gather)
					{
						direct_photons.push_back(photon);
						deposited = true;
					}
				}
				else
				{
					if(specular_path)
					{
						if (!caustic_done)
						{
							caustic_photons.push_back(photon);
							deposited = true;
							if (caustic_photons.size() == photon_map->n_caustic_photons)
							{
								caustic_done = true;
								photon_map->n_caustic_paths = int(n_shot);
								kd_tree_init(&(photon_map->caustic_map),
									caustic_photons);
								caustice_map_inited = true;
							}
						}
					}
					else
					{
						if (!indirect_done)
						{
							deposited = true;
							indirect_photons.push_back(photon);
							if (indirect_photons.size() == photon_map->n_indirect_photons)
							{
								indirect_done = true;
								photon_map->n_indirect_paths = (int)n_shot;
								kd_tree_init(&(photon_map->indirect_map),
									indirect_photons);
								indirect_map_inited = true;
							}
						}
					}
				}
				if(deposited && photon_map->final_gather && rng.RandomFloat() < photon_map->rr_threshold)
				{
					//store data for radiance photon
					normal3f_t n = photon_isect.dg.nn;
					if(vdot(n,photon_ray.d) > 0.f)vneg(n,n);
					radiance_photon_t r;
					radiance_photon_init(&r,&(photon_isect.dg.p),(&n));
					radiance_photons.push_back(r);
					spectrum_t rho_r;
					bsdf_rho_hh(&photon_bsdf,&seed,BSDF_ALL_REFLECTION,&rho_r);
					rp_reflectances.push_back(rho_r);
					spectrum_t rho_t;
					bsdf_rho_hh(&photon_bsdf,&seed,BSDF_ALL_TRANSMISSION,&rho_t);
					rp_transmittances.push_back(rho_t);
				}
			}
			if (n_intersections > 8)
			{
				break;
			}
			//sample new photon ray direction
			vector3f_t wi;
			float pdf;
			BxDFType flags;
			float u1,u2,u3;
			if(n_intersections == 1)
			{
				u1 = u[5];//radical_inverse((int)total_shot + 1, 13);
				u2 = u[6];//radical_inverse((int)total_shot + 1, 17);
				u3 = u[7];//radical_inverse((int)total_shot + 1, 19);
			}
			else
			{
				u1 = rng.RandomFloat();//random_float(seed);
				u2 = rng.RandomFloat();//random_float(seed);
				u3 = rng.RandomFloat();//random_float(seed);
				//printf("%.3f,%.3f,%.3f\t",u1,u2,u3);
			}
			//compute new photon weight and possibly terminate with rr
			spectrum_t fr;
			bsdf_sample_f(&photon_bsdf,&wo,&wi,u1,u2,u3,&pdf,BSDF_ALL,&flags,&fr);
			if(color_is_black(fr) || pdf == 0.f)
				break;
			spectrum_t anew;
			vmul(anew,alpha,fr);
			vsmul(anew,fabs(vdot(wi,photon_bsdf.dg_shading.nn))/pdf,anew);
				float continue_prob = min(1.f,spectrum_y(&anew)/spectrum_y(&alpha));
				if (rng.RandomFloat() > continue_prob  )
				{
					break;
				}
				vsmul(alpha,1.f/continue_prob,anew);
			specular_path = (specular_path)
				&& ((flags & BSDF_SPECULAR) != 0);
			if(indirect_done && !specular_path)break;
			rinit(photon_ray,photon_isect.dg.p,wi);
			photon_ray.mint = photon_isect.ray_epsilon;
			if(debug_tracing)
				printf("%d:(%.3f,%.3f,%.3f),%.5f, alpha(%.3f,%.3f,%.3f)\n",n_intersections,photon_isect.dg.p.x,photon_isect.dg.p.y,photon_isect.dg.p.z,
				photon_isect.ray_epsilon,alpha.x,alpha.y,alpha.z);
			if(alpha.x > -1 && alpha.x < 1 && alpha.y-1<17741612 && alpha.y+1 > 17741612 && alpha.z < 1 && alpha.z > -1)
			{
				int debug_break = 0;
			}
		}
	}
	photon_map->total_photons += n_shot;
	if (photon_map->final_gather)
	{
		//precompute radiance at a subset of the photons
		photon_kd_tree_t direct_map;
		kd_tree_init(&direct_map,direct_photons);
		int n_direct_paths = n_shot;

		//#pragma omp parallel for schedule(dynamic, 32)
		for (unsigned int i = 0; i < radiance_photons.size(); ++i)
		{
			radiance_photon_t &rp = radiance_photons[i];
			const spectrum_t& rho_r = rp_reflectances[i];
			const spectrum_t& rho_t = rp_transmittances[i];
			spectrum_t E,E1,E2;
			point3f_t p = rp.p;
			normal3f_t n = rp.n;
			if (!color_is_black(rho_r))
			{
				vclr(E);
				vclr(E1);
				vclr(E2);
				estimate_e(photon_map,&(direct_map),n_direct_paths,p,n,&E);
				if(indirect_map_inited)
					estimate_e(photon_map,&(photon_map->indirect_map),
						photon_map->n_indirect_paths,p,n,&E1);
				if(caustice_map_inited)
					estimate_e(photon_map,&(photon_map->caustic_map),
					photon_map->n_caustic_paths,p,n,&E2);
				vadd(E,E,E1);
				vadd(E,E,E2);
				vmul(E,E,rho_r);
				vsmul(E,INV_PI,E);
				vadd(rp.lo,rp.lo,E);
			}
			if(!color_is_black(rho_t))
			{
				vclr(E);
				vclr(E1);
				vclr(E2);
				vneg(n,n);
				estimate_e(photon_map,&(direct_map),n_direct_paths,p,n,&E);
				if(indirect_map_inited)
				estimate_e(photon_map,&(photon_map->indirect_map),
					photon_map->n_indirect_paths,p,n,&E1);
				if(caustice_map_inited)
				estimate_e(photon_map,&(photon_map->caustic_map),
					photon_map->n_caustic_paths,p,n,&E2);
				vadd(E,E,E1);
				vadd(E,E,E2);
				vmul(E,E,rho_t);
				vsmul(E,INV_PI,E);
				vadd(rp.lo,rp.lo,E);
			}
		}
		kd_tree_init(&(photon_map->radiance_map),radiance_photons);
		radiance_map_inited = true;
	}
}