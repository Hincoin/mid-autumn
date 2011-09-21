#include <cmath>
#include <stdlib.h>
#include "clscene.h"
#include "photon_ray.h"
#include "light_funcs.h"

#include "primitive_funcs.h"
#include "reflection.h"

CLScene::CLScene(const cl_scene_info_t &scene_info)
{
	scene_info_ = scene_info;


	//precompute
	int n_lights = int(scene_info.lght_count);
	lights_power_ = (float*)calloc(n_lights , sizeof(float));
	light_cdf_ = (float*)calloc((n_lights+1) , sizeof(float));
	spectrum_t lpower;
	for (int i = 0;i < n_lights; ++i)
	{
		light_power(scene_info.lghts+i,scene_info,&lpower);
		lights_power_[i] = spectrum_y(&lpower);
	}
	compute_step_1d_cdf(lights_power_,n_lights,&total_power_,light_cdf_);

	seed_ = new Seed();
	seed_->s1 = rand();
	seed_->s2 = rand();
	seed_->s3 = rand();
	//todo
}
CLScene::~CLScene()
{
	free(lights_power_);
	free(light_cdf_);
	delete seed_;
}
photon_ray_t CLScene::GeneratePhotonRay() const
{
	//todo

	static int n_shot = 0;
	float u[4];
	u[0]  = radical_inverse((int)n_shot + 1, 2);
	u[1]  = radical_inverse((int)n_shot + 1, 3);
	u[2]  = radical_inverse((int)n_shot + 1, 5);
	u[3]  = radical_inverse((int)n_shot + 1, 7);

	//choose light of shoot photon from
	int n_lights = int(scene_info_.lght_count);
	float lpdf;
	float uln = radical_inverse((int)n_shot + 1,11);
	int lnum = floor(sample_step_1d(lights_power_,light_cdf_,
		total_power_,n_lights,uln,&lpdf));
	lnum = min(lnum, n_lights - 1);

	light_info_t* light = scene_info_.lghts+lnum;
	float pdf;
	photon_ray_t photon_ray;
	normal3f_t nl;
	spectrum_t alpha;
	light_ray_sample_l(light,scene_info_,u[0],u[1],u[2],u[3],
		&photon_ray,&nl,&pdf,&alpha);
	//if (pdf == 0.f || color_is_black(alpha))continue;
	vsmul(alpha,(fabs(vdot(nl,photon_ray.d))/(pdf*lpdf)),alpha);
	photon_ray.flux = alpha;

	n_shot++;
	return photon_ray;
}
void CLScene::PhotonHit(RayBuffer<photon_ray_t> &photon_rays, std::vector<photon_ray_hit_point_t> *photon_hits)const
{
	//todo
	intersection_t photon_isect;
	photon_ray_hit_point_t photon_hit;
	bsdf_t bsdf;
	for(size_t i = 0;i < photon_rays.size(); ++i)
	{
		photon_ray_t& ray = photon_rays[i];

		if(intersect(scene_info_.accelerator_data,scene_info_.shape_data,scene_info_.primitives,scene_info_.primitive_count,
					&ray,&photon_isect))
		{

			intersection_get_bsdf(&photon_isect,scene_info_,
					&ray, &bsdf);
			vector3f_t photon_wi ; 
			vneg(photon_wi,ray.d);
			float pdf;
			BxDFType flags;

			float u1 = random_float(seed_);
			float u2 = random_float(seed_);
			float u3 = random_float(seed_);

			vassign(photon_hit.n, bsdf.nn);
			photon_hit.pos = bsdf.dg_shading.p;

			
			spectrum_t fr;
			bsdf_sample_f(&bsdf,&photon_wi,&photon_hit.wo,u1,u2,u3,&pdf,BSDF_ALL,&flags,&fr);
			if(pdf <= 0.f || color_is_black(fr))
				ray.flux.x = ray.flux.y = ray.flux.z = 0.f;
			else
			{
				float co = fabs(vdot(photon_wi,bsdf.nn)) / pdf;
				vmul(ray.flux,ray.flux,fr);
				vsmul(ray.flux,co,ray.flux);
				ray.o = photon_hit.pos;
				ray.d = photon_hit.wo;
				ray.ray_depth ++;
			}
		}
		else
		{
			ray.flux.x = ray.flux.y = ray.flux.z = 0.f;
			photon_hit = photon_ray_hit_point_t();
		}
		photon_hits->push_back(photon_hit);
	}
}

INLINE bool is_hit_light(intersection_t* isect,cl_scene_info_t scene_info)
{
	return (scene_info.primitives[isect->primitive_idx].material_info.material_type == LIGHT_MATERIAL);//light type
}
void CLScene::RayTrace(const ray_differential_t& ray, ray_hit_point_t *hit_point)const
{
	int depth = 0;
	int max_depth = 5;
	int rr_depth = 3;
	int specular_bounce = 1;
	spectrum_t throughtput; vinit(throughtput,1,1,1);

	ray_t cur_ray = static_cast<ray_t>(ray);
	intersection_t isect;


	bool is_debug_ray = false;
	for (;;++depth)
	{
		if (depth > max_depth ||!intersect(scene_info_.accelerator_data, scene_info_.shape_data, scene_info_.primitives, 
			scene_info_.primitive_count, &cur_ray,&isect) )
		{
			hit_point->type = hp_constant_color;
			vclr(hit_point->throughput);
			return;
		}

		if (depth > rr_depth && !specular_bounce)
		{
			float continue_prob = .5f;
			if (random_float(seed_) > continue_prob)
			{
				hit_point->type = hp_constant_color;
				vclr(hit_point->throughput);
				return;
			}
			continue_prob = 1.f/continue_prob;
			vsmul(throughtput,continue_prob,throughtput);
		}
		if (depth == 0 || specular_bounce)
		{
			spectrum_t tmp;
			vector3f_t v;
			vneg(v,cur_ray.d);
			vclr(tmp)
			intersection_le(&isect,scene_info_,&v,&tmp);
			vmul(tmp,throughtput,tmp);
			if(is_hit_light(&isect,scene_info_))
			{
				hit_point->type = hp_constant_color;
				hit_point->throughput = tmp;
				return;
			}
		}
		bsdf_t bsdf;

		//compute
		intersection_get_bsdf(&isect,scene_info_,&cur_ray,&bsdf);//do following
		//compute_differential_geometry(isect.dg,ray);
		//differential_geometry dgs;
		//get_dg_shading_geometry(shape,dg,&dgs)//dgs = dg;
		//material_get_bsdf(scene,material,dg,dgs,&bsdf);
		point3f_t p = bsdf.dg_shading.p;
		normal3f_t n = bsdf.dg_shading.nn;
		vector3f_t wo;vneg(wo,cur_ray.d);

		float bs1 = random_float(seed_);
		float bs2 = random_float(seed_);
		float bcs = random_float(seed_);
		vector3f_t wi;
		float pdf;
		BxDFType flags;
		spectrum_t f;
		bsdf_sample_f(&bsdf,&wo,&wi,bs1,bs2,bcs,&pdf,BSDF_ALL,&flags,&f);
		if (pdf <= 0.f || color_is_black(f))
		{
			hit_point->type = hp_constant_color;
			vclr(hit_point->throughput);
			break;
		}
		specular_bounce = (flags & BSDF_SPECULAR) != 0;
		float co = fabs(vdot(wi,n)) / pdf;
		vmul(throughtput,throughtput,f);
		vsmul(throughtput,co,throughtput);


		if (!specular_bounce)
		{
			hit_point->type = hp_surface;
			hit_point->bsdf = bsdf;
			hit_point->pos = p;
			hit_point->normal = n;
			hit_point->wo = wo;
			hit_point->throughput = throughtput;
			return;
		}


		cur_ray.o = p;
		cur_ray.d = wi;
		cur_ray.mint = EPSILON;
		cur_ray.maxt = FLT_MAX;
	}
}
void CLScene::RayHit(const RayBuffer<ray_differential_t> &rays, std::vector<ray_hit_point_t> *ray_hits)const
{
	//todo
	ray_hit_point_t hit_point;	
	float zmax,zmin;
	zmax = -1000;
	zmin = 1000;
	for(int i = 0;i < rays.size(); ++i)
	{
		RayTrace(rays[i],&hit_point);
		hit_point.index = i;
		ray_hits->push_back(hit_point);
		if(hit_point.type != hp_constant_color)
		{
			zmax = max(zmax, hit_point.pos.z);
			zmin = min(zmin, hit_point.pos.z);
		}
	}
	printf("zmax, zmin : %.3f, %.3f",zmax, zmin);
}
