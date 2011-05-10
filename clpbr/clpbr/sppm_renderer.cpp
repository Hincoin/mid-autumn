#include <cassert>
#include <cmath>
#include "photon_ray.h"
#include "sppm_renderer.h"
#include "film.h"
#include "camera.h"
#include "scene.h"
#include "sppm_hash_grid.h"

void SPPMRenderer::Render(const Scene *scene)
{
	std::vector<final_hit_point_t> final_hit_points;//all hit points
	num_total_photons_ = 0;
	while(true)//todo add stop condition
	{
		std::vector<camera_sample_t> samples;//all sample per path
		std::vector<ray_hit_point_t> ray_hit_points;//all hit points
	
		bool has_more_sample = true;
		while(has_more_sample)//do eye pass
		{
			//
			RayBuffer<ray_differential_t> ray_buffer(1024*1024);//todo change to configurable size
			std::vector<ray_hit_point_t> ray_hit_buffer;

			camera_sample_t sample;
			while(!ray_buffer.IsFull() && has_more_sample)
			{
				if(sampler_->GetNextSample(&sample))
				{
					ray_buffer.Push(camera_->GenerateRay(sample));
					ray_buffer.Back().ray_id = samples.size();
					samples.push_back(sample);
				}
				else
					has_more_sample = false;
			}
			scene->RayHit(ray_buffer,&ray_hit_buffer);
			//add ray_hit_points
			std::copy(ray_hit_buffer.begin(),ray_hit_buffer.end(),
				std::back_inserter(ray_hit_points));
		}
		//photon pass
		std::vector<accum_hit_point_t> accum_hit_points;
		//initialize accum hit points
		for(size_t i = 0;i < ray_hit_points.size(); ++i)
		{
			accum_hit_points.push_back(accum_hit_point_t(max_photon_r2_));
		}
		//build hash lookup grid

		bool is_first_pass = final_hit_points.empty();
		SPPMHashGrid hash_grid(ray_hit_points,final_hit_points,max_photon_r2_,image_->GetWidth(),image_->GetHeight(),
			is_first_pass,accum_hit_points);

		PhotonTrace(scene,hash_grid,ray_hit_points,accum_hit_points);
		
		//final pass
		if(is_first_pass)
		{
			final_hit_points.resize(ray_hit_points.size());
		}
		assert(final_hit_points.size() == ray_hit_points.size());
		assert(samples.size() == ray_hit_points.size());

		//compute radiance
		unsigned max_photon_r2 = 0;
		for(unsigned i = 0;i < final_hit_points.size(); ++i)
		{
			final_hit_point_t& fhp = final_hit_points[i];
			const ray_hit_point_t& rhp = ray_hit_points[i];
			const accum_hit_point_t& ahp = accum_hit_points[i];
			unsigned pcount = fhp.photon_count + ahp.accum_photon_count;
			if(rhp.type == hp_constant_color)
			{
				fhp.constant_hit_count ++;
				vadd(fhp.accum_radiance,fhp.accum_radiance,rhp.throughput);
			}
			else if(rhp.type == hp_surface)
			{
				if(ahp.accum_photon_count > 0)
				{

					unsigned pcount = fhp.photon_count + ahp.accum_photon_count;
					const float g = alpha_ * pcount / (fhp.photon_count*alpha_+ahp.accum_photon_count);
					spectrum_t flux ;
					vadd(flux,fhp.flux,ahp.accum_flux);
					vsmul(fhp.flux,g,flux);
					fhp.r2 = ahp.r2 * g;
					fhp.photon_count = pcount;
					//ahp.accum_photon_count = 0;
					//vclr(ahp.accum_flux);
					// recompute max_photon_r2_ 
					max_photon_r2 = max(max_photon_r2,fhp.r2);
				}
				fhp.surface_hit_count ++;
			}
			const unsigned hit_count = fhp.constant_hit_count + fhp.surface_hit_count;
			if(hit_count > 0)
			{
				//todo: compute final radiance
				float k = 1.f / (FLOAT_PI * fhp.r2 * num_total_photons_);
				spectrum_t f;
				vsmul(f,fhp.surface_hit_count*k,fhp.flux);
				vadd(f,f,fhp.accum_radiance);
				vsmul(fhp.radiance,1.f/hit_count,f);
			}
		}
		if(max_photon_r2 > 0) max_photon_r2_ = max_photon_r2;

		for(size_t i = 0;i < samples.size(); ++i)
			image_->AddSample(samples[i],final_hit_points[i].radiance);
		image_->WriteImage();
	}
}
SPPMRenderer::~SPPMRenderer()
{
	delete camera_;
	delete sampler_;
	delete image_;
}

void SPPMRenderer::PhotonTrace(const Scene* scene,
		const SPPMHashGrid& hash_grid,const std::vector<ray_hit_point_t>& ray_hits,
		std::vector<accum_hit_point_t>& accum_hits)
{
	for(unsigned num_photons = 0;num_photons < num_photons_per_pass_;)
	{
		RayBuffer<photon_ray_t> photon_ray_buffer(1024);
		std::vector<photon_ray_hit_point_t> photon_hits;
		photon_hits.reserve(1024);
        do
        {
            while (num_photons++ < num_photons_per_pass_ && !photon_ray_buffer.IsFull())
            {
                photon_ray_buffer.Push(scene->GeneratePhotonRay());//todo use sampler to generate light samples
				num_total_photons_ ++;
            }
            scene->PhotonHit(photon_ray_buffer,&photon_hits);
			assert(photon_ray_buffer.size() == photon_hits.size());
            //iterate through photon_ray_buffer and swap not hitted one with last one
            for(unsigned i = 0;i < photon_ray_buffer.size();)
            {
                if(color_is_black(photon_ray_buffer[i].flux))
				{
					photon_ray_buffer.DeleteWithoutOrder(i);
					photon_hits[i] = photon_hits.back();
					photon_hits.pop_back();
				}
                else
				{
                	//compute and add estimated flux
					const photon_ray_hit_point_t& hp = photon_hits[i];
					std::pair<SPPMHashGrid::range_iterator,SPPMHashGrid::range_iterator> range = 
						hash_grid.EqualRange(hp.pos);
					for(SPPMHashGrid::range_iterator it = range.first;it != range.second;++it)
					{
						unsigned idx = *it;
						const ray_hit_point_t& ray_hp = ray_hits[idx];
						if(vdot(ray_hp.normal,hp.n) > 1e-3f && 
								vdot(ray_hp.normal,ray_hp.normal) <= accum_hits[idx].r2)
						{
							accum_hits[idx].accum_photon_count ++; 
							spectrum_t f ;
							vector3f_t wo,wi;
							vsub(wi,hp.pos,ray_hp.pos);
							wo = hp.wo;
						   	bsdf_f(&ray_hp.bsdf,&wo,&wi,BSDF_ALL,&f);
							vadd(accum_hits[idx].accum_flux,accum_hits[idx].accum_flux,f);
						}
					}
                    i++;
				}
            }
        }
        while(!photon_ray_buffer.Empty());
    }
	
}
