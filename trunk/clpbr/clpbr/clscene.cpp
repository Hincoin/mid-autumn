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

	//todo
}
CLScene::~CLScene()
{
	free(lights_power_);
	free(light_cdf_);
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

			float u1 = random_float(0);
			float u2 = random_float(0);
			float u3 = random_float(0);

			vassign(photon_hit.n, bsdf.nn);
			photon_hit.pos = bsdf.dg_shading.p;

			
			spectrum_t fr;
			bsdf_sample_f(&bsdf,&photon_hit.wo,&photon_wi,u1,u2,u3,&pdf,BSDF_ALL,&flags,&fr);
			if(pdf <= 0.f || color_is_black(fr))
				ray.flux.x = ray.flux.y = ray.flux.z = 0.f;
			else
			{
				//float co = fabs(vdot(photon_wi,bsdf.nn)) / pdf;
				//vmul(ray.flux,ray.flux,fr);
				//vsmul(ray.flux,co,ray.flux);
			}
			photon_hits->push_back(photon_hit);
		}
	}
}
void CLScene::RayHit(const RayBuffer<ray_differential_t> &rays, std::vector<ray_hit_point_t> *ray_hits)const
{
	//todo
}
