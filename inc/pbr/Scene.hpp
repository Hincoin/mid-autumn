#ifndef _MA_INCLUDED_SCENE_HPP_
#define _MA_INCLUDED_SCENE_HPP_

#include "CRTPInterfaceMacro.hpp"
#include <vector>

namespace ma
{
	template<typename Conf>
	class Scene
	{
	public:
		ADD_SAME_TYPEDEF(Conf,camera_ptr)
		ADD_SAME_TYPEDEF(Conf,surface_integrator_ptr)
		ADD_SAME_TYPEDEF(Conf,volume_integrator_ptr)
		ADD_SAME_TYPEDEF(Conf,sampler_ptr)
		ADD_SAME_TYPEDEF(Conf,primitive_ptr)
		ADD_SAME_TYPEDEF(Conf,light_ptr)
		ADD_SAME_TYPEDEF(Conf,volume_region_ptr)
		ADD_SAME_TYPEDEF(Conf,ray_t)
		ADD_SAME_TYPEDEF(Conf,intersection_t)
		ADD_SAME_TYPEDEF(Conf,bbox_t)
		ADD_SAME_TYPEDEF(Conf,spectrum_t)
		ADD_SAME_TYPEDEF(Conf,ray_differential_t)
		ADD_SAME_TYPEDEF(Conf,sample_ptr)
		ADD_SAME_TYPEDEF(Conf,sample_t)
		ADD_SAME_TYPEDEF(Conf,scalar_t)
		typedef Scene<Conf> class_type;

		static const int default_sample_grain_size = 256;
	public:
		void render();
		Scene(camera_ptr c,surface_integrator_ptr in,
			volume_integrator_ptr vi,sampler_ptr s,
			primitive_ptr accel,const std::vector<light_ptr>& lts,
			volume_region_ptr vr);
		~Scene();
		bool intersect(const ray_t& ray,intersection_t& isect)const{
			//return intersect(aggregate,ray,isect);
			return aggregate->intersect(ray,isect);
		}
		bool intersect_p(const ray_t& ray)const{
			//return intersect_p(aggregate,ray);
			return aggregate->intersectP(ray);
		}
		const bbox_t& worldBound()const;
		spectrum_t li(const ray_differential_t& ray,const sample_ptr sample,
			scalar_t& alpha)const;
		spectrum_t li(const ray_differential_t& ray,const sample_ptr sample)const;
		spectrum_t transmittance(const ray_t& ray)const;
		spectrum_t le(const ray_differential_t& r)const{
			spectrum_t L;
			for (unsigned i = 0; i < lights.size(); ++i)
				L +=  lights[i]->le(r);
			return L;
		}
		std::vector<light_ptr> lights;
	protected:
	private:
		primitive_ptr aggregate;

		camera_ptr camera_;
		volume_region_ptr volume_region;
		surface_integrator_ptr surface_integrator;
		volume_integrator_ptr volume_integrator;
		sampler_ptr sampler;
		bbox_t bound;

		
	};
}



//implementation
#include <stdlib.h>
#include "parallel_compute.hpp"

#include <ctime>
namespace ma{

	namespace parallel{
		template<typename SceneT,int MAX_GRAIN_SIZE = 256>
		struct input_sample{
			//typename SceneT::scalar_t ray_weight;
			typename SceneT::sampler_ptr samplers[MAX_GRAIN_SIZE]; //these are managed by the scene sampler_ptr
			typename SceneT::sample_ptr samples[MAX_GRAIN_SIZE];
			input_sample(){
				::memset(samples,0,sizeof(samples));
			}
			~input_sample(){
				for (size_t i = 0;i < MAX_GRAIN_SIZE; ++i)
				{
					delete_ptr(samples[i]);
				}
			}
		};
		template<typename SceneT>
		struct output_info{
			ADD_SAME_TYPEDEF(SceneT,ray_differential_t);
			ADD_SAME_TYPEDEF(SceneT,scalar_t);

			ADD_SAME_TYPEDEF(SceneT,spectrum_t);
			ADD_SAME_TYPEDEF(SceneT,sample_t);
			ray_differential_t ray;
			spectrum_t ls;
			scalar_t alpha;
			typename sample_t::camera_sample_t camera_sample;
			bool processed;
			output_info():alpha(0),processed(false){}
		};
		template<typename S>
		struct ray_tracing:parallel_range_processor<ray_tracing<S> >{
			ADD_SAME_TYPEDEF(S,sample_ptr);
			ADD_SAME_TYPEDEF(S,camera_ptr)
			ADD_SAME_TYPEDEF(S,scalar_t)

			ADD_SAME_TYPEDEF(S,ray_differential_t);
			ADD_SAME_TYPEDEF(S,sample_t)
			ADD_SAME_TYPEDEF(S,spectrum_t)

			typedef input_sample<S,S::default_sample_grain_size> input_sample_t;
			//typedef output_info<S> output_info_t;
			//typedef const std::vector<input_ray_t>& input_seq_t;
			//typedef std::vector<output_info_t>& output_seq_t;
			typedef input_sample_t& input_seq_t;
			//typedef output_info_t* output_seq_t;

			input_seq_t input_;
			//output_seq_t output_;
			const S* scene;
			/*const*/ camera_ptr camera_;
			const size_t* sizes_array;

			ray_tracing(
				input_seq_t in,
				const S* s,/*const*/ camera_ptr c)
				:input_(in),scene(s),camera_(c){}

			input_seq_t input()const{return input_;}
			bool run(size_t i )const //return bool to decide continue or break
			{
				sample_ptr sample = input_.samples[i];
				while( input_.samplers[i]->getNextSample(*sample))
				{
					ray_differential_t ray;
					spectrum_t ls;
					scalar_t alpha = 0;
					typename sample_t::camera_sample_t camera_sample;

					scalar_t ray_weight = camera::generateRay(camera_,sample,ray);
					camera_sample = *sample;
					if (ray_weight > 0)
						ls = ray_weight * scene->li(ray, sample,alpha);
					//this is thread-safe because the image space is divided into different sections
					camera::addSample(camera_,  &camera_sample, ray, ls, alpha);
				}
				return true;
			}
		};
	}
template<typename Conf>
void Scene<Conf>::render()
{
	clock_t tick = clock();
#ifdef TBB_PARALLEL
	thread_observer task_observer;
	tbb::task_scheduler_init init(std::min(hardware_concurrency(),
		std::min<unsigned>(MAX_PARALLEL,default_sample_grain_size)));
#endif

	
	//FILE* fp = fopen("ray.txt","wb");
	//while(getNextSample(sampler,sample))
	//parallel this
#ifdef TBB_PARALLEL
	 //what if the size is > the height of the image
	const int extra_room = 8;
	//size_t reserved_size = 0;
	//size_t base_offsets [default_sample_grain_size+1] = {0};

	typedef parallel::input_sample<class_type,default_sample_grain_size> input_sample_t;
	input_sample_t sampled_rays;
	MA_ASSERT( default_sample_grain_size >= hardware_concurrency());
	unsigned concurrency = default_sample_grain_size;//std::min<unsigned>(default_grain_size,hardware_concurrency());
	sampler_ptr sampler_divided = sampler->subdivide(concurrency);
	for (unsigned i = 0;i < concurrency; ++i)
	{
		sampled_rays.samples[i] = sample_t::make_sample(surface_integrator,volume_integrator,this);
		sampled_rays.samplers[i] = sampler_divided+i;
		//base_offsets[i] = reserved_size;
		//reserved_size += sampled_rays.samplers[i]->totalSamples() + extra_room;
	}
	//base_offsets [concurrency] = reserved_size;
	

	surface_integrator->preprocess(this);

	//typedef parallel::output_info<class_type> output_info_t;
	//output_info_t* outputs = new output_info_t[reserved_size];
	//size_t sample_count = reserved_size;
#endif

	//exit(0);
	clock_t before_parallel = clock()-tick;
	clock_t after_parallel = 0;
#ifdef TBB_PARALLEL
	//do compute
	typedef parallel::ray_tracing<class_type> ray_tracing_func_t;
	ray_tracing_func_t tracing_f(sampled_rays,this,camera_ );
	parallel_for::run(tracing_f,concurrency);
	after_parallel = clock();
	//for (size_t i = 0;i < sample_count; ++i)
	//{
	//	if(outputs[i].processed)
	//		camera->addSample( outputs[i].camera_sample,outputs[i].ray,outputs[i].ls,outputs[i].alpha);
	//}
	after_parallel = clock() - after_parallel;
#else
	sample_ptr sample = sample_t::make_sample(surface_integrator,volume_integrator,this);

	surface_integrator->preprocess(this);
	//preprocess(surface_integrator,this);
	//preprocess(volume_integrator,this);
	while(sampler->getNextSample(*sample))
	{
		ray_differential_t ray;
		scalar_t ray_weight = camera::generateRay(camera_,sample->cameraSample(),ray);//generateRay(camera,*sample,ray);
		scalar_t alpha=0;
		spectrum_t ls;
		if (ray_weight > 0)
			ls = ray_weight * li(ray, sample,alpha);
		//if (alpha != 0)
		//{
		//	printf("intersect %.2f,%.2f ! \n",sample->image_x,sample->image_y);
		//}
		camera::addSample(camera_, &sample->cameraSample(),ray,ls,alpha);
	}
/**/
	//fflush(fp);
	//fclose(fp);
	delete_ptr(sample);
#endif

	printf("render time before li:%ld do li:%ld clocks ; after parallel: %ld \n",
		before_parallel,(long)(clock()-tick),(long)after_parallel);
	camera::writeImage(camera_);
}
template<typename Conf>
Scene<Conf>::~Scene(){
	delete_ptr(camera_);
	delete_ptr(sampler);
	delete_ptr(surface_integrator);
	//delete_ptr(volume_integrator);
	virtual_delete(aggregate);
	//delete_ptr(volume_region);

	delete_ptr(lights.begin(),lights.end());
}

template<typename Conf>
Scene<Conf>::Scene(camera_ptr c,surface_integrator_ptr in,
			volume_integrator_ptr vi,sampler_ptr s,
			primitive_ptr accel,const std::vector<light_ptr>& lts,
			volume_region_ptr vr
		)
{
lights = lts;
aggregate = accel;
camera_ = c;
sampler = s;
surface_integrator = in;
volume_integrator = vi;
volume_region = vr;
bound = aggregate->worldBound();


}
template<typename Conf>
const typename Conf::bbox_t& Scene<Conf>::worldBound() const{
	return bound;
}
template<typename Conf>
typename Conf::spectrum_t Scene<Conf>::li(const ray_differential_t& r,
		const sample_ptr sample,scalar_t& alpha)const
{
	spectrum_t lo = surface_integrator->li(const_cast<Scene<Conf>*>(this),r,sample,alpha);
//	spectrum_t t =
//
	return lo;
}
template<typename Conf>
typename Conf::spectrum_t Scene<Conf>::li(const ray_differential_t& r,
										  const sample_ptr sample)const
{
	scalar_t alpha = 0;
	spectrum_t lo = surface_integrator->li(const_cast<Scene<Conf>*>(this),r,sample,alpha);
	//	spectrum_t t =
	//
	return lo;
}
template <typename Conf>
typename Conf::spectrum_t Scene<Conf>::transmittance(const ray_t& ray)const{
	return spectrum_t(1);
}
}

#endif
