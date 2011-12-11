#include <cmath>
#include <iterator>
#include <omp.h>
#include "config.h"
#include "progressive_photon_map_renderer.h"
#include "ray_buffer.h"
#include "photon_map.h"

#include "film.h"
#include "camera.h"
#include "random_number_generator_mt19937.h"
#include "opencl_device.h"
#include "scene_data.h"

#include <ctime> 


PPMRenderer::PPMRenderer(Camera* c,Film* im,Sampler* s,photon_map_t* photon_map)
:camera_(c),image_(im),sampler_(s),photon_map_(photon_map)
{
	device_ = new OpenCLDevice(CL_DEVICE_TYPE_CPU);
	device_->SetKernelFile("rendering_kernel.cl", "render");
	photon_intersect_device_ = new OpenCLDevice(CL_DEVICE_TYPE_CPU);
	photon_intersect_device_->SetKernelFile("intersect_kernel.cl","photon_intersect");
}
PPMRenderer::~PPMRenderer()
{
	delete camera_;
	delete sampler_;
	delete image_;
	delete device_;
	delete photon_intersect_device_;
}
void PPMRenderer::InitializeDeviceData(const scene_info_memory_t& scene_info)
{
	device_->SetReadOnlyArg(2,scene_info.light_data);
	device_->SetReadOnlyArg(3,scene_info.material_data);
	device_->SetReadOnlyArg(4,scene_info.shape_data);
	device_->SetReadOnlyArg(5,scene_info.texture_data);//to be texture data
	device_->SetReadOnlyArg(7,scene_info.accelerator_data);
	device_->SetReadOnlyArg(8,scene_info.primitives);
	device_->SetReadOnlyArg(9,scene_info.lghts);
	device_->SetReadOnlyArg(11,(unsigned int)scene_info.primitives.size());
	device_->SetReadOnlyArg(12,(unsigned int)scene_info.lghts.size());

	photon_intersect_device_->SetReadOnlyArg(2,scene_info.light_data);
	photon_intersect_device_->SetReadOnlyArg(3,scene_info.material_data);
	photon_intersect_device_->SetReadOnlyArg(4,scene_info.shape_data);
	photon_intersect_device_->SetReadOnlyArg(5,scene_info.texture_data);//to be texture data
	photon_intersect_device_->SetReadOnlyArg(6,scene_info.accelerator_data);
	photon_intersect_device_->SetReadOnlyArg(7,scene_info.primitives);
	photon_intersect_device_->SetReadOnlyArg(8,scene_info.lghts);
	photon_intersect_device_->SetReadOnlyArg(13,(unsigned int)scene_info.primitives.size());
	photon_intersect_device_->SetReadOnlyArg(14,(unsigned int)scene_info.lghts.size());
}
static std::vector<float> as_float_array(const photon_kd_tree_t& photon_kd_tree)
{
	std::vector<float> v;
	v.push_back(as_float(photon_kd_tree.n_nodes));
	for (unsigned int i = 0;i < photon_kd_tree.n_nodes; ++i)
	{
		v.push_back((photon_kd_tree.nodes[i].split_pos));
		v.push_back(as_float(photon_kd_tree.nodes[i].data));
	}
	for(unsigned int i = 0;i < photon_kd_tree.n_nodes; ++i)
	{

		v.push_back((photon_kd_tree.node_data[i].p.x));
		v.push_back((photon_kd_tree.node_data[i].p.y));
		v.push_back((photon_kd_tree.node_data[i].p.z));

		v.push_back((photon_kd_tree.node_data[i].alpha.x));
		v.push_back((photon_kd_tree.node_data[i].alpha.y));
		v.push_back((photon_kd_tree.node_data[i].alpha.z));

		v.push_back((photon_kd_tree.node_data[i].wi.x));
		v.push_back((photon_kd_tree.node_data[i].wi.y));
		v.push_back((photon_kd_tree.node_data[i].wi.z));
	}
	v.push_back(as_float(photon_kd_tree.next_free_node));
	return v;
}
static std::vector<float> as_float_array(const radiance_photon_kd_tree_t& photon_kd_tree)
{
	//
	std::vector<float> v;
	v.push_back(as_float(photon_kd_tree.n_nodes));
	for (unsigned int i = 0;i < photon_kd_tree.n_nodes; ++i)
	{
		v.push_back((photon_kd_tree.nodes[i].split_pos));
		v.push_back(as_float(photon_kd_tree.nodes[i].data));
	}
	for(unsigned int i = 0;i < photon_kd_tree.n_nodes; ++i)
	{

		v.push_back((photon_kd_tree.node_data[i].p.x));
		v.push_back((photon_kd_tree.node_data[i].p.y));
		v.push_back((photon_kd_tree.node_data[i].p.z));

		v.push_back((photon_kd_tree.node_data[i].n.x));
		v.push_back((photon_kd_tree.node_data[i].n.y));
		v.push_back((photon_kd_tree.node_data[i].n.z));

		v.push_back((photon_kd_tree.node_data[i].lo.x));
		v.push_back((photon_kd_tree.node_data[i].lo.y));
		v.push_back((photon_kd_tree.node_data[i].lo.z));
	}
	v.push_back(as_float(photon_kd_tree.next_free_node));
	return v;
}
static std::vector<float> as_float_array(const photon_map_t& photon_map)
{
	std::vector<float> v;
	v.push_back(as_float(photon_map.n_caustic_photons));
	v.push_back(as_float(photon_map.n_indirect_photons));
	v.push_back(as_float(photon_map.n_lookup));
	v.push_back(as_float(photon_map.max_specular_depth));
	v.push_back(as_float(photon_map.max_dist_squared));
	v.push_back(as_float(photon_map.rr_threshold));
	v.push_back(as_float(photon_map.n_caustic_paths));
	v.push_back(as_float(photon_map.n_indirect_paths));
	// Declare sample parameters for light source sampling

	std::vector<float> caustic_array = as_float_array(photon_map.caustic_map);
	std::vector<float> indirect_array = as_float_array(photon_map.indirect_map);
	std::vector<float> radiance_array = as_float_array(photon_map.radiance_map);
	std::copy(caustic_array.begin(),caustic_array.end(),std::back_inserter(v));
	std::copy(indirect_array.begin(),indirect_array.end(),std::back_inserter(v));
	std::copy(radiance_array.begin(),radiance_array.end(),std::back_inserter(v));

	v.push_back(as_float(photon_map.total_photons));
	v.push_back(as_float(photon_map.final_gather));
	v.push_back(as_float(photon_map.cos_gather_angle));
	v.push_back(as_float(photon_map.gather_samples));
	v.push_back(as_float(photon_map.progressive_iteration));
	v.push_back(as_float(photon_map.alpha));
	return v;
}
void PPMRenderer::Render(const scene_info_memory_t& scene_info_mem)
{
	RandomNumberGeneratorMT19937 *rng = new RandomNumberGeneratorMT19937(rand() << 16 | rand());

	scene_info_memory_t scene_info = scene_info_mem;
	InitializeDeviceData(scene_info);
	const int buffer_size = 1024*1024*2;
	std::vector<Seed> seeds;
	Seed seed;
	for (int i = 0;i < buffer_size; ++i)
	{
		init_rng(rng->RandomUnsignedInt(),&seed);
		seeds.push_back(seed);
	}
	device_->SetReadWriteArg(1,seeds);

	int iteration = 0;
	while(true)
	{
		clock_t t0;
		t0 = clock();

		sampler_->ResetSamplePosition();
		photon_map_init(photon_map_,scene_info,*rng, photon_intersect_device_);
		
		scene_info.integrator_data = as_float_array(*photon_map_);
		device_->SetReadOnlyArg(6,scene_info.integrator_data);//to be texture data
		
		
		bool has_more_sample = true;

		sampler_->ResetSamplePosition();
		int ray_traced = 0;
		while(has_more_sample)//do eye pass
		{
			std::vector<spectrum_t> local_color_buffer;
			std::vector<camera_sample_t> local_samples;//all sample per path
			//
			std::vector<ray_differential_t> ray_buffer;
			ray_buffer.reserve(buffer_size);

			camera_sample_t sample;
			ray_differential_t ray;
			float ray_weight;

			while(ray_buffer.size() < buffer_size && has_more_sample)
			{
				if(sampler_->GetNextSample(&sample))
				{
					camera_->GenerateRay(sample, &ray, &ray_weight);

					ray_buffer.push_back(ray);
					ray_buffer.back().ray_id = unsigned(local_samples.size());
					local_samples.push_back(sample);
				}
				else
					has_more_sample = false;
			}

			local_color_buffer.resize(ray_buffer.size(),spectrum_t());
#define USE_OPENCL
#ifndef USE_OPENCL
			photon_map_t loaded_photon_map;
			load_photon_map(&loaded_photon_map,&scene_info.integrator_data[0]);
			//#pragma omp parallel for schedule(dynamic, 32)
			for(unsigned int i = 0;i < ray_buffer.size(); ++i)
			{
				if(i == 151+ 19 * 256)
				{
					int xxxxx= 0;
				}
				photon_map_li(&loaded_photon_map,&ray_buffer[i],as_cl_scene_info(scene_info),&seeds[i],&local_color_buffer[i]);
				image_->AddSample(local_samples[i],local_color_buffer[i]);
			}
#else
			if(!ray_buffer.empty())
			{
				device_->SetReadWriteArg(0,local_color_buffer);
				device_->SetReadOnlyArg(10,ray_buffer);
				device_->SetReadOnlyArg(13,(unsigned int)local_color_buffer.size());
				device_->Run(local_color_buffer.size());
				device_->ReadBuffer(0,&local_color_buffer[0],(unsigned)local_color_buffer.size());
				//////////////////////////////////////////////////////////////////////////

				for(size_t i = 0;i < local_samples.size(); ++i)
					image_->AddSample(local_samples[i],local_color_buffer[i]);
				ray_traced += ray_buffer.size();
				printf("\r%d ray traced...",ray_traced);
			}
#endif
		}
		photon_map_destroy(photon_map_);

		if(0 == iteration % 2)
			image_->WriteImage(iteration);
		iteration ++;
		//photon_map_->max_dist_squared = photon_map_->max_dist_squared * (iteration + photon_map_->alpha_)/(iteration + 1);
		photon_map_->progressive_iteration = iteration;
		printf("iteration:%d total_photons: %d clocks: %ld\n",iteration,photon_map_->total_photons,clock() - t0);
	}
	delete rng;
}