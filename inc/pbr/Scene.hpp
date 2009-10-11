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

		//while(range.next()) do something with range.cur();
		template<typename SceneT,unsigned MIN_PARTITION=256>
		struct sample_range{
			typedef sample_range<SceneT> class_type;
			ADD_SAME_TYPEDEF(SceneT,sampler_ptr)
			ADD_SAME_TYPEDEF(SceneT,sample_ptr)
			ADD_SAME_TYPEDEF(SceneT,surface_integrator_ptr)
			ADD_SAME_TYPEDEF(SceneT,volume_integrator_ptr)
			ADD_SAME_TYPEDEF(SceneT,sample_t)

			struct iterator{
				class_type* range_;
				sample_ptr cur_;
				typedef sample_ptr value_type;
				int i;
				iterator& operator++(){
					++i;
					if (i < range_->upper && range_->sampler_->getNextSample(*cur_)  )
						return *this;
					return range_->end_;
				}
				value_type operator*(){return cur_;}
				iterator():range_(0),i(0){}
				iterator(class_type* r,sample_ptr c):range_(r),cur_(c){}
				bool operator==(const iterator& other)
				{return other.range_ == range_ && cur_ == other.cur_ && i == other.i;}
				int pos()const{return i;}
			};
			typedef const iterator const_iterator;
			bool empty()const{return !(lower < upper);};
			size_t size()const{return upper-lower;}
			bool is_divisible()const{return !empty() && size() > MIN_PARTITION; }
			sample_range(sample_range& sr,tbb::split)
			{
				
			}
			sample_range(int start,int ends,sampler_ptr smp,surface_integrator_ptr surf,volume_integrator_ptr v,
				SceneT* s)
			{
				lower=start;upper = ends;sampler_ = smp;
				surface_integrator = surf;
				volume_integrator = v;
				scene = s;
				cur_ = iterator(this,sample_t::make_sample(surface_integrator,volume_integrator,s));
			}
			~sample_range()
			{
				delete_ptr(sampler_);
				delete_ptr(cur_.cur_);
			}
			const_iterator cur()const{return cur_;}
			bool next(){
				++cur_;
				return cur_ == iterator();
			}
			iterator cur_;
			sampler_ptr sampler_;
			int lower;
			int upper;


			surface_integrator_ptr surface_integrator;
			volume_integrator_ptr volume_integrator;
			SceneT* scene;

		};
		template<typename SceneT>
		struct input_sample{
			//typename SceneT::scalar_t ray_weight;
			typename SceneT::sampler_ptr samplers[MAX_PARALLEL]; //these are managed by the scene sampler_ptr
			typename SceneT::sample_ptr samples[MAX_PARALLEL];
			input_sample(){
				::memset(samples,0,sizeof(samples));
			}
			~input_sample(){
				for (size_t i = 0;i < MAX_PARALLEL; ++i)
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

			typedef input_sample<S> input_sample_t;
			typedef output_info<S> output_info_t;
			//typedef const std::vector<input_ray_t>& input_seq_t;
			//typedef std::vector<output_info_t>& output_seq_t;
			typedef input_sample_t& input_seq_t;
			typedef output_info_t* output_seq_t;

			input_seq_t input_;
			output_seq_t output_;
			const S* scene;
			const camera_ptr camera;

			ray_tracing(
				input_seq_t in,output_seq_t out,
				const S* s,const camera_ptr c):input_(in),output_(out),scene(s),camera(c){}

			input_seq_t input()const{return input_;}
			bool run(size_t i )const //return bool to decide continue or break
			{
				int thread_id = get_thread_logic_id();
				sample_ptr sample = input_.samples[thread_id];
				if(input_.samplers[thread_id]->getNextSample(*sample))
				{
					output_info_t& o = output_[i];
					assert(!o.processed);
					scalar_t ray_weight = camera->generateRay(*sample,o.ray);
					o.camera_sample = *sample;
					if (ray_weight > 0)
						o.ls = ray_weight * scene->li(o.ray, sample,o.alpha);
					o.processed = true;
					return true;
				}
				return false;
			}
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

	
	//FILE* fp = fopen("ray.txt","wb");
	//while(getNextSample(sampler,sample))
	//parallel this
#ifdef TBB_PARALLEL
	size_t reserved_size = 256;
	typedef parallel::input_sample<class_type> input_sample_t;
	input_sample_t sampled_rays;
	MA_ASSERT( MAX_PARALLEL >= hardware_concurrency());
	unsigned concurrency = std::min<unsigned>(MAX_PARALLEL,hardware_concurrency());
	sampler_ptr sampler_divided = sampler->subdivide(concurrency);
	for (unsigned i = 0;i < concurrency; ++i)
	{
		sampled_rays.samples[i] = sample_t::make_sample(surface_integrator,volume_integrator,this);
		sampled_rays.samplers[i] = sampler_divided+i;
		reserved_size += sampled_rays.samplers[i]->totalSamples();
	}
	

	surface_integrator->preprocess(this);

	typedef parallel::output_info<class_type> output_info_t;
	output_info_t* outputs = new output_info_t[reserved_size];
	size_t sample_count = reserved_size;
	//while(sampler->getNextSample(*sample) && sample_count < reserved_size)
	//{
	//	sampled_rays[sample_count].ray_weight = camera->generateRay(*sample,outputs[sample_count].ray);
	//	outputs[sample_count].camera_sample = sample->cameraSample();
	//	sample_count++;
	//}
#endif

	//exit(0);
	clock_t before_parallel = clock()-tick;
#ifdef TBB_PARALLEL
	//do compute
	typedef parallel::ray_tracing<class_type> ray_tracing_func_t;
	ray_tracing_func_t tracing_f(sampled_rays,outputs,this,camera);
	parallel_for::run(tracing_f,sample_count);

	for (size_t i = 0;i < sample_count; ++i)
	{
		if(outputs[i].processed)
			camera->addSample( outputs[i].camera_sample,outputs[i].ray,outputs[i].ls,outputs[i].alpha);
		if (outputs[i].camera_sample.image_y > 25 && outputs[i].camera_sample.image_y < 28)
		{
			printf("%f \n",outputs[i].alpha);
		}
	}
	delete []outputs;
#else
	sample_ptr sample = sample_t::make_sample(surface_integrator,volume_integrator,this);

	surface_integrator->preprocess(this);
	//preprocess(surface_integrator,this);
	//preprocess(volume_integrator,this);
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
/**/
	//fflush(fp);
	//fclose(fp);
	delete_ptr(sample);
#endif

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
