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

		camera_ptr camera;
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
		template<typename SceneT>
		struct input_ray{
			typename SceneT::scalar_t ray_weight;
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
			output_info():alpha(0){}
		};
		template<typename S>
		struct ray_tracing:parallel_range_processor<ray_tracing<S> >{
			ADD_SAME_TYPEDEF(S,sample_ptr);

			typedef input_ray<S> input_ray_t;
			typedef output_info<S> output_info_t;
			//typedef const std::vector<input_ray_t>& input_seq_t;
			//typedef std::vector<output_info_t>& output_seq_t;
			typedef const input_ray_t* input_seq_t;
			typedef output_info_t* output_seq_t;

			input_seq_t input_;
			output_seq_t output_;
			const S* scene;
			const sample_ptr sample;

			ray_tracing(
				input_seq_t in,output_seq_t out,
				const S* s,const sample_ptr sam):input_(in),output_(out),scene(s),sample(sam){}

			input_seq_t input()const{return input_;}
			void run(size_t i )const
			{
				const input_ray_t& in = input_[i];
				output_info_t& o = output_[i];
				if (in.ray_weight > 0)
					o.ls = in.ray_weight * scene->li(o.ray, sample,o.alpha);
			}
		};
		template<typename SceneT>
		struct sample_producer
		{
		private:
			ADD_SAME_TYPEDEF(SceneT,sampler_ptr)
			ADD_SAME_TYPEDEF(SceneT,sample_ptr)
			ADD_SAME_TYPEDEF(SceneT,sample_t)
			typedef typename sample_t::camera_sample_t camera_sample_t ;
			sampler_ptr sampler;
			sample_ptr sample;
		public:
			//typedef typename boost::add_reference<sample_ptr>::type argument_type;
			typedef camera_sample_t argument_value_type;
			typedef argument_value_type argument_type;
			argument_type& start(){return *sample;}
			bool operator()(argument_type& s) {
				if(sampler->getNextSample(*sample)){
					s = sample->cameraSample();
					return true;
				}
				return false;
			}

			sample_producer(sampler_ptr smplr,sample_ptr smpl ):sampler(smplr),sample(smpl){}
		};
		template<typename SceneT>
		struct ray_generator:parallel_item_processor<ray_generator<SceneT> >{
			ADD_SAME_TYPEDEF(SceneT,camera_ptr)
			ADD_SAME_TYPEDEF(SceneT,sample_t)
			typedef typename sample_t::camera_sample_t camera_sample_t ;

			typedef parallel::input_ray<SceneT> input_ray_t;
			typedef std::vector<input_ray_t> ray_seq_t;
			typedef parallel::output_info<SceneT> output_info_t;
			typedef std::vector<output_info_t> output_seq_t;
			//typedef typename boost::add_reference<sample_ptr>::type reference_type;
			//typedef typename boost::add_const<reference_type>::type argument_type;
			typedef camera_sample_t argument_value_type;
			typedef argument_value_type argument_type;


			ray_generator(ray_seq_t& r,output_seq_t& o,camera_ptr c,core::details::mutex_t& m)
				:input_rays(r),output_info(o),camera(c),mutex_(m){}
			void run(const argument_type& sample)const{
				input_ray_t r;
				output_info_t o;
				r.ray_weight = camera->generateRay(sample,o.ray);
				o.camera_sample = sample;
				{
					core::details::scope_lock<core::details::mutex_t> lock(mutex_);
					input_rays.push_back(r);
					output_info.push_back(o);
				}

			}
		private:
			ray_seq_t& input_rays;
			output_seq_t& output_info;
			core::details::mutex_t& mutex_;
			camera_ptr camera;

		};


		//////////////////////////////////////////////////////////////////////////
		template<typename SceneT>
		struct sample_pipeline_filter:pipeline_filter<sample_pipeline_filter<SceneT> >
		{
		private:
			ADD_SAME_TYPEDEF(SceneT,sampler_ptr)
			ADD_SAME_TYPEDEF(SceneT,sample_ptr)
			ADD_SAME_TYPEDEF(SceneT,sample_t)
			typedef pipeline_filter<sample_pipeline_filter<SceneT> > parent_type;
			typedef typename sample_t::camera_sample_t camera_sample_t ;
			sampler_ptr sampler;
			sample_ptr sample;
		public:
			void* run(void* ) {
				if(sampler->getNextSample(*sample)){
					return sample;
				}
				return 0;
			}

			sample_pipeline_filter(sampler_ptr smplr,sample_ptr smpl ):parent_type(true),sampler(smplr),sample(smpl){}
		};
		template<typename SceneT>
		struct ray_pipeline_filter:pipeline_filter<ray_pipeline_filter<SceneT> >{
			ADD_SAME_TYPEDEF(SceneT,camera_ptr)
			ADD_SAME_TYPEDEF(SceneT,sample_t)
			ADD_SAME_TYPEDEF(SceneT,sample_ptr)
			typedef typename sample_t::camera_sample_t camera_sample_t ;
			typedef pipeline_filter<ray_pipeline_filter<SceneT> > parent_type;

			typedef parallel::input_ray<SceneT> input_ray_t;
			typedef parallel::output_info<SceneT> output_info_t;

			//typedef std::vector<input_ray_t>& ray_seq_t;
			//typedef std::vector<output_info_t>& output_seq_t;
			typedef input_ray_t* ray_seq_t;
			typedef output_info_t* output_seq_t;
			//typedef typename boost::add_reference<sample_ptr>::type reference_type;
			//typedef typename boost::add_const<reference_type>::type argument_type;
			typedef camera_sample_t argument_value_type;
			typedef argument_value_type argument_type;


			ray_pipeline_filter(ray_seq_t r,output_seq_t o,camera_ptr c)
				:parent_type(true),input_rays(r),output_info(o),camera(c),pos(0){}
			void* run(void* s){
				sample_ptr sample = static_cast<sample_ptr>(s);
				input_rays[pos].ray_weight = camera->generateRay(*sample,output_info[pos].ray);
				output_info[pos].camera_sample = sample->cameraSample();
				pos++;
				return 0;
			}
		private:
			ray_seq_t  input_rays;
			output_seq_t  output_info;
			camera_ptr camera;
			size_t pos;
		};
	}
template<typename Conf>
void Scene<Conf>::render()
{
	clock_t tick = clock();
#ifdef TBB_PARALLEL
	thread_observer task_observer;
	tbb::task_scheduler_init init;
#endif

	sample_ptr sample = sample_t::make_sample(surface_integrator,volume_integrator,this);
	surface_integrator->preprocess(this);
//preprocess(surface_integrator,this);
//preprocess(volume_integrator,this);
	//FILE* fp = fopen("ray.txt","wb");
	//while(getNextSample(sampler,sample))
	//parallel this
#ifdef TBB_PARALLEL
	const size_t reserved_size = sampler->totalSamples() + 256; //estamated size
	typedef parallel::input_ray<class_type> input_ray_t;
	//std::vector<input_ray_t> sampled_rays;
	//sampled_rays.reserve(reserved_size);
	//sampled_rays.resize(reserved_size);
	input_ray_t* sampled_rays = new input_ray_t[reserved_size];

	typedef parallel::output_info<class_type> output_info_t;
	//std::vector<output_info_t> outputs;
	//outputs.reserve(reserved_size) ;
	//outputs.resize(reserved_size) ;
	output_info_t* outputs = new output_info_t[reserved_size];

	//typedef parallel::sample_producer<class_type> sample_generator_t;
	//typedef parallel::ray_generator<class_type> ray_generator_t;
	//sample_generator_t sample_gen(sampler,sample);
	//parallel_stream<sample_generator_t> stream(sample_gen);
	//core::details::mutex_t m;
	//ray_generator_t ray_gen(sampled_rays,outputs,camera,m);
	//parallel_while::run(stream,ray_gen);

	//typedef parallel::sample_pipeline_filter<class_type> sample_filter_t;
	//typedef parallel::ray_pipeline_filter<class_type> ray_filter_t;
	//sample_filter_t sample_filter(sampler,sample);
	//ray_filter_t ray_filter(sampled_rays,outputs,camera);
	//pipeline pipe;
	//pipe.add_filter(sample_filter);
	//pipe.add_filter(ray_filter);
	//pipe.run(1);

	size_t sample_count = 0;
	while(sampler->getNextSample(*sample) && sample_count < reserved_size)
	{
		sampled_rays[sample_count].ray_weight = camera->generateRay(*sample,outputs[sample_count].ray);
		outputs[sample_count].camera_sample = sample->cameraSample();
		sample_count++;
	}
#endif

	//exit(0);
	clock_t before_parallel = clock()-tick;
#ifdef TBB_PARALLEL
	//do compute
	typedef parallel::ray_tracing<class_type> ray_tracing_func_t;
	ray_tracing_func_t tracing_f(sampled_rays,outputs,this,sample);
	parallel_for::run(tracing_f,sample_count);

	for (size_t i = 0;i < sample_count; ++i)
	{
		camera->addSample( outputs[i].camera_sample,outputs[i].ray,outputs[i].ls,outputs[i].alpha);
	}
	delete []sampled_rays;
	delete []outputs;
#else
	while(sampler->getNextSample(*sample))
	{
		ray_differential_t ray;
		scalar_t ray_weight = camera->generateRay(*sample,ray);//generateRay(camera,*sample,ray);
		scalar_t alpha=0;
		spectrum_t ls;
		if (ray_weight > 0)
			ls = ray_weight * li(ray, sample,alpha);
		//if (alpha != 0)
		//{
		//	printf("intersect %.2f,%.2f ! \n",sample->image_x,sample->image_y);
		//}
		camera->addSample( sample->cameraSample(),ray,ls,alpha);
	}

#endif
/**/
	//fflush(fp);
	//fclose(fp);
	delete_ptr(sample);
	printf("render time before li:%ld do li:%ld clocks \n",before_parallel,(long)(clock()-tick));
	camera->writeImage();
}
template<typename Conf>
Scene<Conf>::~Scene(){
	delete_ptr(camera);
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
camera = c;
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
