#ifndef _PHOTON_MAP_H_
#define _PHOTON_MAP_H_

//
#include "kdtree.h"
#include "spectrum.h"
#include "geometry.h"
#include "sampling.h"
#include "random_number_generator.h"
#include "primitive_funcs.h"
#include "integrator_funcs.h"

typedef struct{
	point3f_t p;
	spectrum_t alpha;
	vector3f_t wi;
} photon_t;

INLINE void photon_init(photon_t* photon,const point3f_t* p,const spectrum_t* wt,const vector3f_t* w)
{
	photon->p = *p;
	photon->alpha = *wt;
	photon->wi = *w;
}

typedef struct {
	point3f_t p;
	normal3f_t n;
	spectrum_t lo;
}radiance_photon_t ;

INLINE void radiance_photon_init(radiance_photon_t* photon,const point3f_t* p,const normal3f_t* n)
{
	photon->p = *p;
	photon->n = *n;
	vclr(photon->lo);
}

typedef struct{
	const photon_t* photon;
	float distance_squared;
}
close_photon_t;

INLINE void close_photon_init(close_photon_t* close_photon,const photon_t* photon,float dist_sqr)
{
	close_photon->photon = photon;
	close_photon->distance_squared = dist_sqr;
}
INLINE bool close_photon_less_than(close_photon_t a,close_photon_t b)
{
	return a.distance_squared == b.distance_squared? (a.photon < b.photon) :
		a.distance_squared < b.distance_squared;
}
typedef struct  
{
	const point3f_t* p;
	close_photon_t *photons;
	unsigned n_lookup;
	unsigned found_photons;
}photon_process_data_t;

INLINE void photon_process_data_init(photon_process_data_t* photon_process_data,
						 unsigned mp,
						 const point3f_t* p)
{
	photon_process_data->p = p;
	photon_process_data->photons = 0;
	photon_process_data->n_lookup = mp;
	photon_process_data->found_photons = 0;
}
INLINE void photon_process(photon_process_data_t* data,const photon_t* photon,
					float dist2,float* max_dist_sqred)
{
	if (data->found_photons < data->n_lookup) {
		// Add photon to unordered array of photons
		close_photon_init(&data->photons[data->found_photons++] ,photon, dist2);
		if (data->found_photons == data->n_lookup) {
			c_make_heap(close_photon_t,&data->photons[0], &data->photons[data->n_lookup],close_photon_less_than);
			*max_dist_sqred = data->photons[0].distance_squared;
		}
	}
	else {
		// Remove most distant photon from heap and add new photon
		close_photon_t *begin = data->photons;
		close_photon_t *end = data->photons + data->n_lookup;
		c_pop_heap(close_photon_t,begin, end,close_photon_less_than);
		close_photon_init(&data->photons[data->n_lookup-1] ,photon, dist2);
		c_push_heap(close_photon_t,begin, end,close_photon_less_than);
		*max_dist_sqred = data->photons[0].distance_squared;
	}
}

typedef struct{
	const point3f_t *p;
	const normal3f_t *n;
	radiance_photon_t *photon;
} radiance_photon_process_data_t;
INLINE void radiance_photon_process_data_init(radiance_photon_process_data_t *data,const point3f_t* pp,const normal3f_t *nn)
{
	data->p = pp;
	data->n = nn;
	data->photon = 0;
}

INLINE void radiance_photon_process(radiance_photon_process_data_t *data,radiance_photon_t *rp,
							 float dist_sqr,float *max_dist)
{
	if(vdot(rp->n,*data->n)>0)
	{
		data->photon = rp;
		*max_dist = dist_sqr;
	}
}



typedef kd_tree_t(photon_t) photon_kd_tree_t;
typedef kd_tree_t(radiance_photon_t) radiance_photon_kd_tree_t;

typedef struct {
	// ExPhotonIntegrator Private Data
    unsigned n_caustic_photons, n_indirect_photons;
	unsigned  n_lookup;
	int max_specular_depth;
	float max_dist_squared, rr_threshold;
	float cos_gather_angle;
	int gather_samples;
	// Declare sample parameters for light source sampling
	int n_caustic_paths, n_indirect_paths;
	photon_kd_tree_t caustic_map;
	photon_kd_tree_t indirect_map;
	radiance_photon_kd_tree_t radiance_map;
    bool final_gather;
}
photon_map_t;


INLINE float photon_map_kernel(const photon_t *photon, const point3f_t *p,
		float md2) {
//	return 1.f / (md2 * M_PI); // NOBOOK
	float s = (1.f - distance_squared(photon->p, *p) / md2);
	return 3.f / (md2 * FLOAT_PI) * s * s;
}

#define MAX_CLOSE_PHOTON_LOOKUP 100

INLINE void photon_map_lphoton(photon_map_t* photon_map,
							   photon_kd_tree_t *map,
							   int n_paths,
							   int n_lookup,
							   bsdf_t *bsdf,
							   const intersection_t* isect,
							   const vector3f_t *wo,
							   float max_dist_sqr,
							   cl_scene_info_t scene_info,
							   Seed *seed,
							   spectrum_t *f)
{
	vclr(*f);
	if(map->n_nodes == 0) return;
	int non_specular = BSDF_REFLECTION | BSDF_TRANSMISSION |  BSDF_DIFFUSE | BSDF_GLOSSY;
	if (bsdf_num_components(bsdf,non_specular) == 0)
	{
		return;
	}
	photon_process_data_t photon_map_data;
	photon_process_data_init(&photon_map_data,photon_map->n_lookup,
		&isect->dg.p);
	close_photon_t close_photon_data_store[MAX_CLOSE_PHOTON_LOOKUP];
	photon_map_data.photons = close_photon_data_store;
	kd_tree_lookup(*map,isect->dg.p,&photon_map_data,photon_process,max_dist_sqr);
	//estimate reflected light from photons
	close_photon_t *photons = photon_map_data.photons;
	int n_found = photon_map_data.found_photons;
	normal3f_t n_f;
	if(vdot(*wo,bsdf->dg_shading.nn) < 0)
	{
		vneg(n_f,bsdf->dg_shading.nn);
	}
	else
		vassign(n_f,bsdf->dg_shading.nn);
	if (bsdf_num_components(bsdf,BSDF_REFLECTION|BSDF_TRANSMISSION|BSDF_GLOSSY)> 0)
	{
		spectrum_t L;
		//compute exitant radiance from photons for glossy
		for(int i = 0;i < n_found; ++i)
		{
			const photon_t *p = photons[i].photon;
			BxDFType flag = vdot(n_f,p->wi) > 0 ?BSDF_ALL_REFLECTION:BSDF_ALL_TRANSMISSION;
			float k = photon_map_kernel(p,&isect->dg.p, max_dist_sqr);
			bsdf_f(bsdf,wo,&p->wi,flag,&L);
			vmul(L,L,p->alpha);
			vsmul(L,(k/n_paths),L);
			vadd(*f,*f,L);
		}
	}
	else
	{
		spectrum_t Lr,Lt;
		vclr(Lr);vclr(Lt);
		spectrum_t L;
		for (int i = 0;i < n_found; ++i)
		{
			float k = photon_map_kernel(photons[i].photon,&isect->dg.p,
				max_dist_sqr);
			if(vdot(n_f,photons[i].photon->wi) > 0.f)
			{
				vsmul(L,(k/n_paths),photons[i].photon->alpha);
				vadd(Lr,Lr,L);
			}
			else
			{
				vsmul(L,(k/n_paths),photons[i].photon->alpha);
				vadd(Lt,Lt,L);
			}
		}
		bsdf_rho_hd(bsdf,wo,seed,BSDF_ALL_REFLECTION,&L);
		vmul(Lr,Lr,L);
		vsmul(Lr,INV_PI,Lr);
		bsdf_rho_hd(bsdf,wo,seed,BSDF_ALL_TRANSMISSION,&L);
		vmul(Lt,Lt,L);
		vsmul(Lt,INV_PI,Lt);
		vadd(*f,*f,Lr);
		vadd(*f,*f,Lt);
	}
}
#define MAX_RAY_DEPTH 16
//todo optimization
INLINE void photon_map_final_gather(photon_map_t* photon_map,cl_scene_info_t scene_info,
									Seed* seed,
									const point3f_t p,
									const normal3f_t n,
									const vector3f_t wo,
									bsdf_t bsdf,
									spectrum_t *l)
{
	int non_specular = BSDF_REFLECTION|BSDF_TRANSMISSION|BSDF_DIFFUSE|BSDF_GLOSSY;
	if (bsdf_num_components(&bsdf,non_specular)>0)
	{
#define n_indir_sample_photons 50

		photon_process_data_t proc_data;
		photon_process_data_init(&proc_data,
			n_indir_sample_photons,&p);
		close_photon_t photon_buffer[n_indir_sample_photons];
		proc_data.photons = photon_buffer;
		float search_dist2 = photon_map->max_dist_squared;
		while (proc_data.found_photons < n_indir_sample_photons)
		{
			float md2 = search_dist2;
			proc_data.found_photons = 0;
			kd_tree_lookup(photon_map->indirect_map,
				p,&proc_data,photon_process,md2);
			search_dist2 *= 2.f;
		}
		vector3f_t photon_dirs [n_indir_sample_photons];
		for (unsigned i = 0;i < n_indir_sample_photons; ++i)
		{
			vassign(photon_dirs[i],proc_data.photons[i].photon->wi);
		}
		spectrum_t Li;vclr(Li);
		for(int i = 0; i < photon_map->gather_samples; ++i)
		{
			vector3f_t wi;float pdf;
			spectrum_t fr;
			float u1 = random_float(seed);
			float u2 = random_float(seed);
			float u3 = random_float(seed);
			BxDFType sampled_type;
			bsdf_sample_f(&bsdf,&wo,&wi,u1,u2,u3,&pdf,(BxDFType)(BSDF_ALL&~BSDF_SPECULAR),&sampled_type,&fr);
			if (pdf == 0.f || color_is_black(fr))
			{
				continue;
			}
			ray_t bounce_ray;
			rinit(bounce_ray,p,wi);
			intersection_t gather_isect;
			if (intersect(scene_info.accelerator_data,scene_info.shape_data,scene_info.primitives,
				scene_info.primitive_count,&bounce_ray,&gather_isect))
			{
				spectrum_t L_indir;vclr(L_indir);
				normal3f_t normal_gather = gather_isect.dg.nn;
				if(vdot(normal_gather,bounce_ray.d)>0)vneg(normal_gather,normal_gather);
				radiance_photon_process_data_t  radiance_proc_data;
				radiance_photon_process_data_init(&radiance_proc_data,
					&gather_isect.dg.p,&normal_gather);
				float md2 = FLT_MAX;
				kd_tree_lookup(photon_map->radiance_map,
					gather_isect.dg.p,&radiance_proc_data,
					radiance_photon_process,
					md2);
				if(radiance_proc_data.photon) L_indir = radiance_proc_data.photon->lo;
				spectrum_t trans;
				scene_tranmittance(scene_info,&bounce_ray,&trans);
				vmul(L_indir,L_indir,trans);

				float photon_pdf = 0.f;
				float cone_pdf = uniform_cone_pdf(photon_map->cos_gather_angle);
				for (int j = 0; j < n_indir_sample_photons;++j)
				{
					if(vdot(photon_dirs[j],wi) > 0.999f * photon_map->cos_gather_angle)photon_pdf += cone_pdf;
				}
				photon_pdf /= n_indir_sample_photons;
				float wt = power_heuristic(photon_map->gather_samples,pdf,photon_map->gather_samples,photon_pdf);
				vsmul(L_indir,(wt*fabs(vdot(wi,n))/pdf),L_indir);
				vmul(fr,fr,L_indir);
				vadd(Li,fr,Li);

			}
		}
		vsmul(Li,1.f/photon_map->gather_samples,Li);
		vadd(*l,*l,Li);
		vclr(Li);
		for (int i = 0;i < photon_map->gather_samples;++i)
		{
			float u1 = random_float(seed);
			float u2 = random_float(seed);
			float u3 = random_float(seed);
			int photon_num = 
				min((int)n_indir_sample_photons -1,
				(int)(u1 * n_indir_sample_photons));
			vector3f_t vx,vy;
			coordinate_system(&photon_dirs[photon_num],&vx,&vy);
			vector3f_t wi;
			uniform_sample_cone(u2,u3,
				photon_map->cos_gather_angle,
				&vx,&vy,photon_dirs+photon_num,
				&wi);
			spectrum_t fr;
			bsdf_f(&bsdf,&wo,&wi,BSDF_ALL,&fr);
			if (color_is_black(fr))
				continue;
			ray_t bounce_ray;
			rinit(bounce_ray,p,wi);
			intersection_t gather_isect;
			if(intersect(scene_info.accelerator_data,scene_info.shape_data,scene_info.primitives,
				scene_info.primitive_count,&bounce_ray,&gather_isect))
			{
				spectrum_t L_indir;vclr(L_indir);
				normal3f_t nn;vassign(nn,gather_isect.dg.nn);
				if(vdot(nn,bounce_ray.d) > 0.f) vneg(nn,nn);
				radiance_photon_process_data_t  radiance_proc_data;
				radiance_photon_process_data_init(&radiance_proc_data,
					&gather_isect.dg.p,&nn);
				float md2 = FLT_MAX;
				kd_tree_lookup(photon_map->radiance_map,
					gather_isect.dg.p,&radiance_proc_data,
					radiance_photon_process,
					md2);
				if(radiance_proc_data.photon)
					vassign(L_indir,radiance_proc_data.photon->lo );
				spectrum_t trans;
				scene_tranmittance(scene_info,&bounce_ray,&trans);
				vmul(L_indir,L_indir,trans);


				float photon_pdf = 0.f;
				float cone_pdf = uniform_cone_pdf(
					photon_map->cos_gather_angle);
				for (unsigned j = 0; j < n_indir_sample_photons;++j)
				{
					if(vdot(photon_dirs[j],wi) > 0.999f * photon_map->cos_gather_angle)
						photon_pdf += cone_pdf;
				}
				photon_pdf /= n_indir_sample_photons;

				float bsdfPdf = bsdf_pdf(&bsdf,&wo,&wi,BSDF_ALL);
				float wt = power_heuristic(photon_map->gather_samples,
					photon_pdf,
					photon_map->gather_samples,
					bsdfPdf);
				vsmul(L_indir,(wt*fabs(vdot(wi,n))/photon_pdf),L_indir);
				vmul(fr,fr,L_indir);
				vadd(Li,fr,Li);
			}
		}
		vsmul(Li,1.f/photon_map->gather_samples,Li);
		vadd(*l,*l,Li);
	}
}
INLINE void photon_map_li(photon_map_t* photon_map,
						  const ray_t *ray,
						  cl_scene_info_t scene_info,
						  Seed *seed,
						  spectrum_t* color
						  )
{
	photon_map->n_lookup = min(photon_map->n_lookup,MAX_CLOSE_PHOTON_LOOKUP);
	photon_map->max_specular_depth = min(photon_map->max_specular_depth,MAX_RAY_DEPTH);
	ray_t ray_stack[MAX_RAY_DEPTH];
    spectrum_t passthrough[MAX_RAY_DEPTH];
	bsdf_t bsdf_stack[MAX_RAY_DEPTH];
	bool left_stack[MAX_RAY_DEPTH];//todo: change to bit 
	int ray_stack_top = 0;
	rassign(ray_stack[ray_stack_top],*ray);
    vinit(passthrough[ray_stack_top],1,1,1);
	left_stack[ray_stack_top] = true;
	ray_stack_top++;
    enum{pre,in,post} visit=pre;
	vclr(*color);
	
	intersection_t isect;
	while ( ray_stack_top > 0)
	{
		ray_t cur_ray;rassign(cur_ray,ray_stack[ray_stack_top-1]);

		if (intersect(scene_info.accelerator_data,scene_info.shape_data,scene_info.primitives,
			scene_info.primitive_count,&cur_ray,&isect))
		{
			//evaluate bsdf at hit point
			if(visit == pre)
			{
				vector3f_t wo;
				vneg(wo,cur_ray.d);
				//compute emitted light if ray hit an area light
				spectrum_t l;
				spectrum_t li_val;
				vclr(li_val);
				vclr(l);
				intersection_le(&isect,scene_info,&wo,&l);
				vadd(li_val,li_val,l);
				intersection_get_bsdf(&isect,scene_info,
					&cur_ray,bsdf_stack+ray_stack_top-1);
				point3f_t p = bsdf_stack[ray_stack_top-1].dg_shading.p;
				normal3f_t n = bsdf_stack[ray_stack_top-1].dg_shading.nn;
				uniform_sample_all_lights(scene_info,
					seed,&p,&n,&wo,bsdf_stack+ray_stack_top-1,&l);
				vadd(li_val,li_val,l);
//#define DIRECT_LIGHTING
#ifndef DIRECT_LIGHTING
				photon_map_lphoton(photon_map,
					&photon_map->caustic_map,photon_map->n_caustic_paths,
					photon_map->n_lookup,bsdf_stack + ray_stack_top - 1,&isect,&wo,photon_map->max_dist_squared
					,scene_info,seed,&l);
				vadd(li_val,li_val,l);
				if (photon_map->final_gather)
				{
#if 1
					photon_map_final_gather(photon_map,
						scene_info,seed,p,n,wo,bsdf_stack [ ray_stack_top - 1],&l);
#else
					normal3f_t nn;
					vassign(nn,n);
					if(vdot(nn,cur_ray.d) > 0.f) vneg(nn,n);
					radiance_photon_process_data_t rpd;
					radiance_photon_process_data_init(&rpd,&p,&nn);
					float md2 = FLT_MAX;
					kd_tree_lookup( photon_map->radiance_map,p,&rpd, radiance_photon_process,md2);
					if(rpd.photon)
						vadd(li_val,li_val,rpd.photon->lo);
#endif
				}


				else{
					photon_map_lphoton(photon_map,
						&photon_map->indirect_map,photon_map->n_indirect_paths,
						photon_map->n_lookup,bsdf_stack+ray_stack_top-1,&isect,&wo,photon_map->max_dist_squared
						,scene_info,seed,&l);
				}
#endif
				vadd(li_val,li_val,l);
				vmul(li_val,li_val,passthrough[ray_stack_top-1]);
				vadd(*color,*color,li_val);
			}

			if(ray_stack_top < photon_map->max_specular_depth && visit != post)
			{
                if(visit == pre)
                {
                    specular_reflect(&cur_ray,bsdf_stack+ray_stack_top-1,seed,&isect,scene_info,passthrough+ray_stack_top,
                            ray_stack+ray_stack_top);
                    if(color_is_black(passthrough[ray_stack_top]))
                    {
                        //go on try transmission rays
                        specular_transmit(&cur_ray,bsdf_stack + ray_stack_top -1,seed,&isect,scene_info,passthrough+ray_stack_top,
                                ray_stack+ray_stack_top);
                        if(color_is_black(passthrough[ray_stack_top]))
                        {
                            visit = post;
                        }
                        else
                        {
							if (ray_stack_top == 1)
							{
								bool specular_trans = true;
							}
							vmul(*(passthrough+ray_stack_top),*(passthrough+ray_stack_top),*(passthrough+ray_stack_top-1));
							left_stack[ray_stack_top] = false;
                            visit = pre;
                            ray_stack_top++;
                        }
                    }
                    else{
                        //multiply passthrough
						vmul(*(passthrough+ray_stack_top),*(passthrough+ray_stack_top),*(passthrough+ray_stack_top-1));
						left_stack[ray_stack_top] = true;
                        visit = pre;
                        ray_stack_top++;
                    }
                }
                else if(visit == in)
                {
                    //go on try transmission rays
                    specular_transmit(&cur_ray,bsdf_stack + ray_stack_top - 1,seed,&isect,scene_info,passthrough+ray_stack_top,
                            ray_stack+ray_stack_top);
                    if(color_is_black(passthrough[ray_stack_top]))
                    {
                        visit = post;
                    }
                    else
                    {
						if (ray_stack_top == 1)
						{
							bool specular_trans = true;
						}
						vmul(*(passthrough+ray_stack_top),*(passthrough+ray_stack_top),*(passthrough+ray_stack_top-1));
						left_stack[ray_stack_top] = false;
                        visit = pre;
                        ray_stack_top++;
                    }
                }
			}
            else
            {
                if(visit == in)visit = post;
				else if(visit == post && left_stack[ray_stack_top-1])
					visit = in;
                else if(visit == pre)
					if(left_stack[ray_stack_top-1])visit = in;
					else visit = post;

                ray_stack_top--;
            }
		}
		else
		{
			//handle no intersection
			if(visit == in)visit = post;
			else if(visit == post && left_stack[ray_stack_top-1])
				visit = in;
			else if(visit == pre)
				if(left_stack[ray_stack_top-1])visit = in;
				else visit = post;

			ray_stack_top--;

			//ray_stack_top--;
			//visit = pre;
		}
	}

}


#ifndef CL_KERNEL

void photon_map_init(photon_map_t* photon_map,GLOBAL float* light_data,GLOBAL float* material_data,GLOBAL float* shape_data,
					 GLOBAL float* texture_data,GLOBAL float* integrator_data,GLOBAL float* accelerator_data,
					 GLOBAL primitive_info_t* primitives,const unsigned int primitive_count,
					 GLOBAL light_info_t* lghts, const unsigned int lght_count,
					 Seed* seed);
#endif
#endif
