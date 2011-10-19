#include <cmath>
#include <omp.h>
#include "config.h"
#include "progressive_photon_map_renderer.h"
#include "ray_buffer.h"
#include "photon_map.h"

#include "film.h"
#include "camera.h"
#include "scene.h"
#include "random_number_generator_mt19937.h"

PPMRenderer::~PPMRenderer()
{
	delete camera_;
	delete sampler_;
	delete image_;
}
void PPMRenderer::Render(const cl_scene_info_t scene_info)
{
	Seed *seed = new Seed;

	int iteration = 0;

	RandomNumberGeneratorMT19937 rng(rand() << 16 | rand());
	while(true)
	{
		seed->s1 = rand()<<16 | rand();
		seed->s2 = rand()<<16 | rand();
		seed->s3 = rand()<<16 | rand();
		
		sampler_->ResetSamplePosition();
		photon_map_init(photon_map_,scene_info,rng);
		
		
		bool has_more_sample = true;
		std::vector<camera_sample_t> samples;//all sample per path
		std::vector<spectrum_t> color_buffer;

		sampler_->ResetSamplePosition();
		while(has_more_sample)//do eye pass
		{
			//
			RayBuffer<ray_differential_t> ray_buffer(1024*1024);//todo change to configurable size

			camera_sample_t sample;
			ray_differential_t ray;
			float ray_weight;

			while(!ray_buffer.IsFull() && has_more_sample)
			{
				if(sampler_->GetNextSample(&sample))
				{
					camera_->GenerateRay(sample, &ray, &ray_weight);

					ray_buffer.Push(ray);
					ray_buffer.Back().ray_id = samples.size();
					samples.push_back(sample);
				}
				else
					has_more_sample = false;
			}

			color_buffer.resize(color_buffer.size()+ray_buffer.size(),spectrum_t());
			#pragma omp parallel for schedule(dynamic, 32)
			for(int i = 0;i < ray_buffer.size(); ++i)
			{
				if(i == 163+ 140    * 68)
				{
					int xxxxx= 0;
				}
				photon_map_li(photon_map_,&ray_buffer[i],scene_info,seed,&color_buffer[i]);
			}
		}
		photon_map_destroy(photon_map_);

		for(size_t i = 0;i < samples.size(); ++i)
			image_->AddSample(samples[i],color_buffer[i]);
		image_->WriteImage(iteration);
		iteration ++;
		//photon_map_->max_dist_squared = photon_map_->max_dist_squared * (iteration + photon_map_->alpha_)/(iteration + 1);
		photon_map_->progressive_iteration = iteration;
		printf("iteration:%d max_dist_sqr:%.3f ,total_photons: %d\n",iteration,photon_map_->max_dist_squared,photon_map_->total_photons);
	}
	delete seed;
}