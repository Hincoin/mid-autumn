#include "config.h"
#include <cmath>
#include <omp.h>
#include <ctime>
#include "cl_scene.h"
#include "photon_map.h"
#include "reflection.h"
#include "random_number_generator_mt19937.h"
#include "permuted_halton.h"
#include "photon_intersection_data.h"
#include "opencl_device.h"


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

void photon_map_preprocess(photon_map_t* photon_map,scene_info_memory_t& scene_info,RandomNumberGeneratorMT19937& rng, 
	cl_photon_init_device_info_t& device_info
	);

void photon_map_init(photon_map_t* photon_map,scene_info_memory_t& scene_info,RandomNumberGeneratorMT19937 &rng, 
	cl_photon_init_device_info_t& device_info
	)
{
	photon_map_preprocess(photon_map,scene_info,rng, device_info);
}

void photon_map_destroy(photon_map_t* photon_map)//free up kd-tree memory
{
	kd_tree_destroy<photon_t>(&(photon_map->caustic_map));
	kd_tree_destroy<photon_t>(&photon_map->indirect_map);
	kd_tree_destroy<radiance_photon_t>(&photon_map->radiance_map);
}
void photon_map_preprocess(photon_map_t* photon_map,scene_info_memory_t& scene_info_memory,RandomNumberGeneratorMT19937 &rng,
			cl_photon_init_device_info_t& device_info
		)
{
	photon_map->n_lookup = min(photon_map->n_lookup,MAX_CLOSE_PHOTON_LOOKUP);
	photon_map->max_specular_depth = min(photon_map->max_specular_depth,MAX_RAY_DEPTH);
	if (scene_info_memory.lghts.empty() ) return ;

	cl_scene_info_t scene_info = as_cl_scene_info(scene_info_memory);
	//
	using namespace std;
	vector<photon_t> caustic_photons;
	vector<photon_t> indirect_photons;
	vector<photon_t> direct_photons;
	vector<radiance_photon_t> radiance_photons;
	bool caustice_map_inited = false,indirect_map_inited = false,radiance_map_inited=false;

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
	vector<permuted_halton16_t> halton;
	halton.resize(1);
	permuted_halton16_initialize(&halton[0],rng);


	//static const int buffer_size = 1 * 16 * 128;
	printf("start shooting photons\n");
	clock_t t0 = clock();
	static const unsigned int buffer_size =  1024 * 1024;
	std::vector<photon_intersection_data_t> other_datas;
	std::vector<generated_photon_t> generated_photons;
	std::vector<ray_t> photon_rays;
	generated_photons.resize(buffer_size);
	other_datas.resize(buffer_size);
	::memset(&other_datas[0],0,sizeof(other_datas[0]) * buffer_size);
	for(size_t i = 0;i < other_datas.size(); ++i)
	{
		init_rng(rng.RandomUnsignedInt(),&other_datas[i].seed);
	}
	photon_rays.resize(buffer_size);

	cl::Buffer lights_power_buffer = cl::Buffer(device_info.photon_intersect_context,CL_MEM_READ_ONLY|CL_MEM_COPY_HOST_PTR,n_lights * sizeof(float),lights_power);
	cl::Buffer light_cdf_buffer = cl::Buffer(device_info.photon_intersect_context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, (n_lights + 1) * sizeof(float),light_cdf);
	cl::Buffer photon_rays_buffer = cl::Buffer(device_info.photon_intersect_context, CL_MEM_READ_WRITE | CL_MEM_USE_HOST_PTR,
		photon_rays.size() * sizeof(photon_rays[0]),&photon_rays[0]);
	cl::Buffer photon_intersect_data_buffer = cl::Buffer(device_info.photon_intersect_context,CL_MEM_READ_WRITE|CL_MEM_USE_HOST_PTR,
		other_datas.size() * sizeof(other_datas[0]),&other_datas[0]);

	device_info.photon_intersect_kernel.setArg(9,lights_power_buffer);
	device_info.photon_intersect_kernel.setArg(10,light_cdf_buffer);
	device_info.photon_intersect_kernel.setArg(12,total_power);
	device_info.photon_intersect_kernel.setArg(16,buffer_size);
	device_info.photon_intersect_kernel.setArg(1,photon_rays_buffer);
	device_info.photon_intersect_kernel.setArg(0,photon_intersect_data_buffer);

	device_info.photon_generate_kernel.setArg(0, photon_intersect_data_buffer);
	device_info.photon_generate_kernel.setArg(1,photon_rays_buffer);

	cl::Buffer generated_photons_buffer = cl::Buffer(device_info.photon_generate_context,CL_MEM_READ_WRITE|CL_MEM_COPY_HOST_PTR,
		generated_photons.size() * sizeof(generated_photons[0]),&generated_photons[0]);
	device_info.photon_generate_kernel.setArg(9,generated_photons_buffer);
	device_info.photon_generate_kernel.setArg(10,(unsigned int)scene_info_memory.primitives.size());
	device_info.photon_generate_kernel.setArg(11,(unsigned int)scene_info_memory.lghts.size());
	device_info.photon_generate_kernel.setArg(12,buffer_size);
	bool is_first_pass = true;
	while (!caustic_done || !indirect_done )//todo: add not photon_rays.empty
	{
		permuted_halton16_initialize(&halton[0],rng);
		cl::Buffer halton_buffer = cl::Buffer(device_info.photon_intersect_context,CL_MEM_READ_ONLY|CL_MEM_COPY_HOST_PTR,
			halton.size() * sizeof(halton[0]),&halton[0]);
		device_info.photon_intersect_kernel.setArg(11,halton_buffer);
		device_info.photon_intersect_kernel.setArg(15,unsigned int(photon_map->total_photons + n_shot));

		device_info.photon_intersect_command_queue.enqueueNDRangeKernel(device_info.photon_intersect_kernel,cl::NullRange,
			cl::NDRange(buffer_size),cl::NDRange(128));
		device_info.photon_intersect_command_queue.finish();
		device_info.photon_generate_command_queue.enqueueNDRangeKernel(device_info.photon_generate_kernel,cl::NullRange,
			cl::NDRange(buffer_size),cl::NDRange(128));
		device_info.photon_generate_command_queue.enqueueReadBuffer(photon_intersect_data_buffer,CL_TRUE,0,other_datas.size()*sizeof(other_datas[0]),&other_datas[0]);
		device_info.photon_generate_command_queue.enqueueReadBuffer(generated_photons_buffer,CL_TRUE,0,generated_photons.size() * sizeof(generated_photons[0]),&generated_photons[0]);
		/*
		photon_map_preprocess_cl(&other_datas[0],&photon_rays[0],scene_info.light_data,scene_info.material_data,scene_info.shape_data,scene_info.texture_data,
			scene_info.integrator_data,scene_info.accelerator_data,scene_info.primitives,scene_info.lghts,lights_power,light_cdf,&halton[0],total_power,scene_info.primitive_count
			,scene_info.lght_count,photon_map->total_photons + n_shot,buffer_size,rng);
			*/
		//
		if(!is_first_pass)
		{
			for(size_t i = 0;i < other_datas.size();++i)
			{
				n_shot += (photon_intersection_data_n_intersections(&other_datas[i]) <= 1); //1 or 0
				if(photon_intersection_data_n_intersections(&other_datas[i]) == 0)
				{
				}
				else
				{
					if (photon_intersection_data_has_non_specular(&other_datas[i]))
					{
						//deposit photon at surface
						photon_t &photon = generated_photons[i].photon;
						bool deposited = false;
						if(photon_intersection_data_n_intersections(&other_datas[i]) == 1)
						{
							if(!indirect_done && photon_map->final_gather)
							{
								direct_photons.push_back(photon);
								deposited = true;
							}
						}
						else
						{
							if(photon_intersection_data_is_specular_path(&other_datas[i]))
							{
								if (!caustic_done)
								{
									caustic_photons.push_back(photon);
									deposited = true;
								}
							}
							else
							{
								if (!indirect_done)
								{
									deposited = true;
									indirect_photons.push_back(photon);
								}
							}
						}
						if(deposited && photon_map->final_gather && rng.RandomFloat() < photon_map->rr_threshold)
						{
							//store data for radiance photon
							radiance_photons.push_back(generated_photons[i].radiance_photon);
							rp_reflectances.push_back(generated_photons[i].rho_r);
							rp_transmittances.push_back(generated_photons[i].rho_t);
						}
					}
				}
			}

			if (caustic_photons.size() >= photon_map->n_caustic_photons && !caustic_done)
			{
				caustic_done = true;
				photon_map->n_caustic_paths = int(n_shot);
				kd_tree_init(&(photon_map->caustic_map),
					caustic_photons);
				caustice_map_inited = true;
			}
			if (indirect_photons.size() >= photon_map->n_indirect_photons && !indirect_done)
			{
				indirect_done = true;
				photon_map->n_indirect_paths = (int)n_shot;
				kd_tree_init(&(photon_map->indirect_map),
					indirect_photons);
				indirect_map_inited = true;
			}
			//give up if we are not storing enough photons
			if (n_shot > 500000 && 
				(unsuccessful(photon_map->n_caustic_photons,
				(unsigned)caustic_photons.size(),
				buffer_size)) && 
				(unsuccessful(photon_map->n_indirect_photons,
				(unsigned)indirect_photons.size(),
				buffer_size)))
			{
				printf("Warning: No photon found!\n");
				break;
			}
		}
		is_first_pass = false;
		printf("\r%d/%d photon shooted. time: %d, indirect_photons: %d",n_shot,photon_map->total_photons,clock()-t0,indirect_photons.size());
	}
	printf("\n");
	photon_map->total_photons += n_shot;
	if (photon_map->final_gather)
	{
		//precompute radiance at a subset of the photons
		photon_kd_tree_t direct_map;
		kd_tree_init(&direct_map,direct_photons);
		int n_direct_paths = n_shot;

#pragma omp parallel for schedule(dynamic, 32)
		for (int i = 0; i < (int)radiance_photons.size(); ++i)
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
