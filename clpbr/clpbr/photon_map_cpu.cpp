#include "config.h"
#include <cmath>
#include "cl_scene.h"
#include "photon_map.h"
#include "reflection.h"

void estimate_e(photon_map_t* photon_map,photon_kd_tree_t* map,int count,
				const point3f_t& p, const normal3f_t& n,
				spectrum_t* ret)
{
	if(!map) {vclr(*ret);return;}
	//lookup nearby photons at irradiance computation point
	photon_process_data_t proc_data;	

	photon_process_data_init(&proc_data,photon_map->n_lookup,&p);
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

void photon_map_preprocess(photon_map_t* photon_map,cl_scene_info_t scene_info,Seed* seed);

void photon_map_init(photon_map_t* photon_map,GLOBAL float* light_data,GLOBAL float* material_data,GLOBAL float* shape_data,
					 GLOBAL float* texture_data,GLOBAL float* integrator_data,GLOBAL float* accelerator_data,
					 GLOBAL primitive_info_t* primitives,const unsigned int primitive_count,
					 GLOBAL light_info_t* lghts, const unsigned int lght_count,
					 Seed* seed)
{
	cl_scene_info_t scene_info;
	scene_info.accelerator_data = accelerator_data;
	scene_info.material_data = material_data;
	scene_info.lght_count = lght_count;
	scene_info.lghts = lghts;
	scene_info.light_data = light_data;
	scene_info.primitive_count = primitive_count;
	scene_info.primitives = primitives;
	scene_info.shape_data = shape_data;
	scene_info.texture_data = texture_data;
	scene_info.integrator_data = integrator_data;
	photon_map_preprocess(photon_map,scene_info,seed);
}
void photon_map_preprocess(photon_map_t* photon_map,cl_scene_info_t scene_info,Seed* seed)
{
	
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

	while (!caustic_done || !indirect_done)
	{
		++n_shot;
		//give up if we are not storing enough photons
		if (n_shot > 500000 && 
			(unsuccessful(photon_map->n_caustic_photons,
			caustic_photons.size(),
			n_shot)) && 
			(unsuccessful(photon_map->n_indirect_photons,
			indirect_photons.size(),
			n_shot)))
		{
			return;
		}
		float u[4];
		u[0]  = radical_inverse((int)n_shot + 1, 2);
		u[1]  = radical_inverse((int)n_shot + 1, 3);
		u[2]  = radical_inverse((int)n_shot + 1, 5);
		u[3]  = radical_inverse((int)n_shot + 1, 7);

		//choose light of shoot photon from
		float lpdf;
		float uln = radical_inverse((int)n_shot + 1,11);
		int lnum = floor(sample_step_1d(lights_power,light_cdf,
			total_power,n_lights,uln,&lpdf));
		lnum = min(lnum, n_lights - 1);

		light_info_t* light = scene_info.lghts+lnum;
		float pdf;
		ray_t photon_ray;
		normal3f_t nl;
		spectrum_t alpha;
		light_ray_sample_l(light,scene_info,u[0],u[1],u[2],u[3],
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
				if(deposited && photon_map->final_gather && random_float(seed) < 0.125f)
				{
					//store data for radiance photon
					normal3f_t n = photon_isect.dg.nn;
					if(vdot(n,photon_ray.d) > 0.f)vneg(n,n);
					radiance_photon_t r;
					radiance_photon_init(&r,&(photon_isect.dg.p),(&n));
					radiance_photons.push_back(r);
					spectrum_t rho_r;
					bsdf_rho_hh(&photon_bsdf,seed,BSDF_ALL_REFLECTION,&rho_r);
					rp_reflectances.push_back(rho_r);
					spectrum_t rho_t;
					bsdf_rho_hh(&photon_bsdf,seed,BSDF_ALL_TRANSMISSION,&rho_t);
					rp_transmittances.push_back(rho_t);
				}
			}
			if (n_intersections > 10)
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
				u1 = radical_inverse((int)n_shot + 1, 13);
				u2 = radical_inverse((int)n_shot + 1, 17);
				u3 = radical_inverse((int)n_shot + 1, 19);
			}
			else
			{
				u1 = random_float(seed);
				u2 = random_float(seed);
				u3 = random_float(seed);
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
			if (random_float(seed) > continue_prob  )
			{
				break;
			}
			vsmul(alpha,1.f/continue_prob,anew);
			specular_path = (specular_path)
				&& ((flags & BSDF_SPECULAR) != 0);
			rinit(photon_ray,photon_isect.dg.p,wi);
		}
	}
	//precompute radiance at a subset of the photons
	photon_kd_tree_t direct_map;
	kd_tree_init(&direct_map,direct_photons);
	int n_direct_paths = n_shot;
	if (photon_map->final_gather)
	{
		#pragma omp parallel for schedule(dynamic, 32)
		for (int i = 0; i < radiance_photons.size(); ++i)
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