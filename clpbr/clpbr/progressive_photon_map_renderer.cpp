#include <cmath>
#include <iterator>
#include <omp.h>
#include <CL/cl.hpp>
#include <fstream>
#include "config.h"
#include "progressive_photon_map_renderer.h"
#include "ray_buffer.h"
#include "photon_map.h"

#include "film.h"
#include "camera.h"
#include "random_number_generator_mt19937.h"
#include "scene_data.h"

#include <ctime> 

static cl::Context ray_tracing_context_;
static cl::Program ray_tracing_program_;
static std::vector<cl::Device>  ray_tracing_devices_;
static cl::CommandQueue ray_tracing_command_queue_;
static cl::Kernel ray_tracing_kernel_;

static cl::Buffer ray_tracing_light_data;
static cl::Buffer ray_tracing_material_data;
static cl::Buffer ray_tracing_shape_data;
static cl::Buffer ray_tracing_texture_data;
static cl::Buffer ray_tracing_accelerator_data;
static cl::Buffer ray_tracing_primitives;
static cl::Buffer ray_tracing_lghts;
static cl::Buffer ray_tracing_colors;
static cl::Buffer ray_tracing_seeds;
static cl::Buffer ray_tracing_rays;


static cl::Context photon_intersect_context_;
static cl::Program photon_intersect_program_;
static std::vector<cl::Device>  photon_intersect_devices_;
static cl::CommandQueue photon_intersect_command_queue_;
static cl::Kernel photon_intersect_kernel_;

static cl::Buffer photon_intersect_light_data;
static cl::Buffer photon_intersect_material_data;
static cl::Buffer photon_intersect_shape_data;
static cl::Buffer photon_intersect_texture_data;
static cl::Buffer photon_intersect_accelerator_data;
static cl::Buffer photon_intersect_primitives;
static cl::Buffer photon_intersect_lghts;
static cl::Buffer photon_intersect_lights_power;
static cl::Buffer photon_intersect_light_cdf;
static cl::Buffer photon_intersect_halton;

static cl::Context photon_generate_context_;
static cl::Program photon_generate_program_;
static std::vector<cl::Device>  photon_generate_devices_;
static cl::CommandQueue photon_generate_command_queue_;
static cl::Kernel photon_generate_kernel_;

static cl::Buffer photon_generate_light_data;
static cl::Buffer photon_generate_material_data;
static cl::Buffer photon_generate_shape_data;
static cl::Buffer photon_generate_texture_data;
static cl::Buffer photon_generate_accelerator_data;
static cl::Buffer photon_generate_primitives;
static cl::Buffer photon_generate_lghts;
static cl::Buffer photon_generate_photons;


void ReadSource(const char *filename, std::string *source_string)
{
	size_t size;
	char*  str;

	std::fstream f(filename, (std::fstream::in | std::fstream::binary));

	if(f.is_open())
	{
		std::streamsize fileSize;
		f.seekg(0, std::fstream::end);
		size = fileSize = f.tellg();
		f.seekg(0, std::fstream::beg);

		str = new char[size+1];
		if(!str)
		{
			f.close();
			return ;
		}

		f.read(str, fileSize);
		f.close();
		str[size] = '\0';
	
		*source_string = str;
		delete[] str;
	}
	else
	{
		//std::cout << "\nFile containg the kernel code(\".cl\") not found. Please copy the required file in the folder containg the executable.\n";
	}
}

PPMRenderer::PPMRenderer(Camera* c,Film* im,Sampler* s,photon_map_t* photon_map)
:camera_(c),image_(im),sampler_(s),photon_map_(photon_map)
{
	std::vector<cl::Platform> platforms;
	cl::Platform::get(&platforms);
	cl_context_properties cprops[]={
		CL_CONTEXT_PLATFORM,
		(cl_context_properties)(platforms[0])(),
		0
	};
	photon_intersect_context_ = cl::Context(CL_DEVICE_TYPE_GPU, cprops);
	photon_intersect_devices_ = photon_intersect_context_.getInfo<CL_CONTEXT_DEVICES>();
	photon_intersect_command_queue_ = cl::CommandQueue(photon_intersect_context_,photon_intersect_devices_[0],0);

	ray_tracing_context_ = cl::Context(CL_DEVICE_TYPE_CPU,cprops);
	ray_tracing_devices_ = ray_tracing_context_.getInfo<CL_CONTEXT_DEVICES>();
	ray_tracing_command_queue_ = cl::CommandQueue(ray_tracing_context_,ray_tracing_devices_[0],0);

	photon_generate_context_ = photon_intersect_context_;//cl::Context(CL_DEVICE_TYPE_GPU, cprops);
	photon_generate_devices_ = photon_generate_context_.getInfo<CL_CONTEXT_DEVICES>();
	photon_generate_command_queue_ = cl::CommandQueue(photon_generate_context_,photon_generate_devices_[0],0);

	std::string source_string;
	ReadSource("intersect_kernel.cl",&source_string);
	cl::Program::Sources photon_intersect_sources(1,std::make_pair(source_string.c_str(),0));
	photon_intersect_program_ = cl::Program(photon_intersect_context_,photon_intersect_sources);
	photon_intersect_program_.build(photon_intersect_devices_,"-I. ");
	photon_intersect_kernel_ = cl::Kernel(photon_intersect_program_,"photon_intersect");

	source_string.clear();
	ReadSource("rendering_kernel.cl",&source_string);
	cl::Program::Sources ray_tracing_sources(1,std::make_pair(source_string.c_str(),0));
	ray_tracing_program_ = cl::Program(ray_tracing_context_,ray_tracing_sources);
	ray_tracing_program_.build(ray_tracing_devices_,"-I. ");
	ray_tracing_kernel_ = cl::Kernel(ray_tracing_program_,"render");

	source_string.clear();
	ReadSource("photon_generation.cl",&source_string);
	cl::Program::Sources photon_generate_sources(1,std::make_pair(source_string.c_str(),0));
	photon_generate_program_ = cl::Program(photon_generate_context_,photon_generate_sources);
	photon_generate_program_.build(photon_generate_devices_,"-I. ");
	photon_generate_kernel_ = cl::Kernel(photon_generate_program_,"photon_generation");
	
}
PPMRenderer::~PPMRenderer()
{
	delete camera_;
	delete sampler_;
	delete image_;
}
template<typename T>
cl::Buffer CreateBuffer(cl::Context context, cl_mem_flags flag, const std::vector<T>& data)
{
	return cl::Buffer(context,flag | CL_MEM_COPY_HOST_PTR,sizeof(T) * data.size(),(void*)&data[0]);
}
void PPMRenderer::InitializeDeviceData(const scene_info_memory_t& scene_info)
{
	ray_tracing_light_data = CreateBuffer(ray_tracing_context_, CL_MEM_READ_ONLY,scene_info.light_data);
	ray_tracing_material_data = CreateBuffer(ray_tracing_context_, CL_MEM_READ_ONLY, scene_info.material_data);
	ray_tracing_shape_data = CreateBuffer(ray_tracing_context_, CL_MEM_READ_ONLY, scene_info.shape_data);
	ray_tracing_texture_data= CreateBuffer(ray_tracing_context_, CL_MEM_READ_ONLY, scene_info.texture_data);
	ray_tracing_accelerator_data = CreateBuffer(ray_tracing_context_, CL_MEM_READ_ONLY, scene_info.accelerator_data);
	ray_tracing_primitives = CreateBuffer(ray_tracing_context_, CL_MEM_READ_ONLY, scene_info.primitives);
	ray_tracing_lghts = CreateBuffer(ray_tracing_context_, CL_MEM_READ_ONLY, scene_info.lghts);

	ray_tracing_kernel_.setArg(2,ray_tracing_light_data);
	ray_tracing_kernel_.setArg(3,ray_tracing_material_data);
	ray_tracing_kernel_.setArg(4,ray_tracing_shape_data);
	ray_tracing_kernel_.setArg(5,ray_tracing_texture_data);
	//ray_tracing_kernel_.setArg(6,ray_tracing_integrator_data);
	ray_tracing_kernel_.setArg(7,ray_tracing_accelerator_data);
	ray_tracing_kernel_.setArg(8,ray_tracing_primitives);
	ray_tracing_kernel_.setArg(9,ray_tracing_lghts);
	ray_tracing_kernel_.setArg(11,(unsigned int)scene_info.primitives.size());
	ray_tracing_kernel_.setArg(12,(unsigned int)scene_info.lghts.size());
	

	photon_generate_light_data = photon_intersect_light_data = CreateBuffer(photon_intersect_context_, CL_MEM_READ_ONLY,scene_info.light_data);
	photon_generate_material_data = photon_intersect_material_data = CreateBuffer(photon_intersect_context_, CL_MEM_READ_ONLY, scene_info.material_data);
	photon_generate_shape_data = photon_intersect_shape_data = CreateBuffer(photon_intersect_context_, CL_MEM_READ_ONLY, scene_info.shape_data);
	photon_generate_texture_data = photon_intersect_texture_data= CreateBuffer(photon_intersect_context_, CL_MEM_READ_ONLY, scene_info.texture_data);
	photon_generate_accelerator_data = photon_intersect_accelerator_data = CreateBuffer(photon_intersect_context_, CL_MEM_READ_ONLY, scene_info.accelerator_data);
	photon_generate_primitives = photon_intersect_primitives = CreateBuffer(photon_intersect_context_, CL_MEM_READ_ONLY, scene_info.primitives);
	photon_generate_lghts = photon_intersect_lghts = CreateBuffer(photon_intersect_context_, CL_MEM_READ_ONLY, scene_info.lghts);

	photon_intersect_kernel_.setArg(2,photon_intersect_light_data);
	photon_intersect_kernel_.setArg(3,photon_intersect_material_data);
	photon_intersect_kernel_.setArg(4,photon_intersect_shape_data);
	photon_intersect_kernel_.setArg(5,photon_intersect_texture_data);
	photon_intersect_kernel_.setArg(6,photon_intersect_accelerator_data);
	photon_intersect_kernel_.setArg(7,photon_intersect_primitives);
	photon_intersect_kernel_.setArg(8,photon_intersect_lghts);
	photon_intersect_kernel_.setArg(13,(unsigned int)scene_info.primitives.size());
	photon_intersect_kernel_.setArg(14,(unsigned int)scene_info.lghts.size());

	photon_generate_kernel_.setArg(2,photon_generate_light_data);
	photon_generate_kernel_.setArg(3,photon_generate_material_data);
	photon_generate_kernel_.setArg(4,photon_generate_shape_data);
	photon_generate_kernel_.setArg(5,photon_generate_texture_data);
	photon_generate_kernel_.setArg(6,photon_generate_accelerator_data);
	photon_generate_kernel_.setArg(7,photon_generate_primitives);
	photon_generate_kernel_.setArg(8,photon_generate_lghts);
	photon_generate_kernel_.setArg(10,(unsigned int)scene_info.primitives.size());
	photon_generate_kernel_.setArg(11,(unsigned int)scene_info.lghts.size());
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
	ray_tracing_seeds = CreateBuffer(ray_tracing_context_,CL_MEM_READ_WRITE,seeds);
	ray_tracing_kernel_.setArg(1,ray_tracing_seeds);

	int iteration = 0;
	while(true)
	{
		clock_t t0;
		t0 = clock();

		sampler_->ResetSamplePosition();
		cl_photon_init_device_info_t device_info;
		device_info.photon_generate_command_queue = photon_generate_command_queue_;
		device_info.photon_generate_context = photon_generate_context_;
		device_info.photon_generate_kernel = photon_generate_kernel_;
		device_info.photon_intersect_command_queue = photon_intersect_command_queue_;
		device_info.photon_intersect_context = photon_intersect_context_;
		device_info.photon_intersect_kernel = photon_intersect_kernel_;
		photon_map_init(photon_map_,scene_info,*rng, device_info);
		
		scene_info.integrator_data = as_float_array(*photon_map_);
		cl::Buffer ray_tracing_integrator_data = 
			CreateBuffer(ray_tracing_context_,CL_MEM_READ_ONLY,scene_info.integrator_data);
		ray_tracing_kernel_.setArg(6,ray_tracing_integrator_data);
		
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
				cl::Buffer ray_tracing_color_buffer = CreateBuffer(ray_tracing_context_,CL_MEM_READ_WRITE,local_color_buffer);
				ray_tracing_kernel_.setArg(0,ray_tracing_color_buffer);
				cl::Buffer ray_tracing_ray_buffer = CreateBuffer(ray_tracing_context_,CL_MEM_READ_ONLY,ray_buffer);
				ray_tracing_kernel_.setArg(10,ray_tracing_ray_buffer);
				ray_tracing_kernel_.setArg(13,(unsigned int) local_color_buffer.size());
				ray_tracing_command_queue_.enqueueNDRangeKernel(ray_tracing_kernel_,cl::NullRange,cl::NDRange(ray_buffer.size()),cl::NullRange);
				ray_tracing_command_queue_.enqueueReadBuffer(ray_tracing_color_buffer,CL_TRUE,0,
					local_color_buffer.size() * sizeof(local_color_buffer[0])
					,&local_color_buffer[0]);
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