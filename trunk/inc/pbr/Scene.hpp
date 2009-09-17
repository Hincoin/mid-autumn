#ifndef _MA_INCLUDED_SCENE_HPP_
#define _MA_INCLUDED_SCENE_HPP_

#include "CRTPInterfaceMacro.hpp"
#include <vector>

namespace ma
{
	template<typename Conf>
	class Scene
	{
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
namespace ma{
template<typename Conf>
void Scene<Conf>::render()	
{
sample_ptr sample = new sample_t(surface_integrator,volume_integrator,this);
surface_integrator->preprocess(this);
//preprocess(surface_integrator,this);
//preprocess(volume_integrator,this);
	FILE* fp = fopen("ray.txt","wb");
	//while(getNextSample(sampler,sample))
	while(sampler->getNextSample(*sample))
	{
		ray_differential_t ray;
		scalar_t ray_weight = camera->generateRay(*sample,ray);//generateRay(camera,*sample,ray);
		//test ray;
		typedef typename Conf::point_t point_t;
		typedef typename Conf::vector_t vector_t;
		ray_differential_t test_ray;
		test_ray.o = point_t(3,3,3);
		test_ray.dir = vector_t(0.f-3,0.2f-3,0.f-3);
		test_ray.dir.normalize();
		test_ray.maxt = 1000000000.f;
		test_ray.mint = 0.001f;
		if(dot (ray.dir,test_ray.dir) > 0){ 
			fprintf(fp,"ray shoot:(%.3f,%.3f,%.3f)->(%.3f,%.3f,%.3f) \n",ray.o.x(),ray.o.y(),ray.o.z(),ray.dir.x(),ray.dir.y(),ray.dir.z());
		}

		scalar_t alpha=0;
		spectrum_t ls;
		if (ray_weight > 0)
			ls = ray_weight * li(ray, sample,alpha);
		if (alpha != 0)
		{
			printf("intersect %.2f,%.2f ! \n",sample->image_x,sample->image_y);
		}
		camera->addSample( *sample,ray,ls,alpha);
	}
	fflush(fp);
	fclose(fp);
	delete_ptr(sample);
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
