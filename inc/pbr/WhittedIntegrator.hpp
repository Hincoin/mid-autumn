#ifndef _MA_INCLUDED_WHITTEDINTEGRATOR_HPP_
#define _MA_INCLUDED_WHITTEDINTEGRATOR_HPP_

#include "Transport.hpp"
#include "parallel_compute.hpp"
namespace ma{

	// WhittedIntegrator Declarations
	template<typename Conf>
	class WhittedIntegrator : public Integrator<WhittedIntegrator<Conf>,Conf> {
	public:
		ADD_SAME_TYPEDEF(Conf,spectrum_t);
		ADD_SAME_TYPEDEF(Conf,scene_ptr);
		ADD_SAME_TYPEDEF(Conf,ray_differential_t);
		ADD_SAME_TYPEDEF(Conf,sample_ptr);
		ADD_SAME_TYPEDEF(Conf,scalar_t);
		ADD_SAME_TYPEDEF(Conf,point_t);
		ADD_SAME_TYPEDEF(Conf,vector_t);
		ADD_SAME_TYPEDEF(Conf,normal_t);
		ADD_SAME_TYPEDEF(Conf,intersection_t);
		ADD_SAME_TYPEDEF(Conf,bsdf_ptr);
		ADD_SAME_TYPEDEF(Conf,visibility_tester_t);
		ADD_SAME_TYPEDEF(Conf,sample_t);
	public:
		void preprocessImpl(const scene_ptr){}
		void requestSamplesImpl(sample_t& ,const scene_ptr){}
		// WhittedIntegrator Public Methods
		spectrum_t li(const scene_ptr scene, const ray_differential_t &ray,
			const sample_ptr sample, scalar_t &alpha) const;
		WhittedIntegrator(int md) {
			maxDepth = md;
			//rayDepth = 0;
			::memset(rayDepth,0,sizeof(rayDepth));
		}
	private:
		// WhittedIntegrator Private Data
		int maxDepth;
		mutable int rayDepth[MAX_PARALLEL];
	};
	// WhittedIntegrator Method Definitions
	template<typename Conf>
	typename Conf::spectrum_t WhittedIntegrator<Conf>::li(const scene_ptr scene, const ray_differential_t &ray,
		const sample_ptr sample, scalar_t &alpha) const {
			intersection_t isect;
			spectrum_t L(0.);
			bool hitSomething;
			// Search for ray-primitive intersection
			hitSomething = scene->intersect(ray, isect);
			if (!hitSomething) {
				// Handle ray with no intersection
				alpha = 0.;
				L += scene->le(ray);
				if (!L.black()) alpha = 1.;
				return L;
			}
			else {
				// Initialize _alpha_ for ray hit
				//printf("hit somthing \n");
				alpha = 1.;
				// Compute emitted and reflected light at ray intersection point_t
				// Evaluate BSDF at hit point_t
				bsdf_ptr bsdf = isect.getBSDF(ray);
				// Initialize common variables for Whitted integrator
				const point_t &p = bsdf->dg_shading.point;
				const normal_t &n = bsdf->dg_shading.normal;
				vector_t wo = -ray.dir;
				// Compute emitted light if ray hit an area light source
				L += isect.le(wo);
				// Add contribution of each light source
				vector_t wi;
				for (unsigned i = 0; i < scene->lights.size(); ++i) {
					visibility_tester_t visibility;
					spectrum_t Li = light::sample_l(scene->lights[i],p,ref( wi ), ref(visibility));
					if (Li.black()) continue;
					//printf("light not black\n");
					spectrum_t f = bsdf->f(wo, wi);
					//printf("bsdf->f is black %d",f.black());
					if (!f.black() && visibility.unOccluded(scene))
						L += f * Li * std::abs(wi.dot(n) ) * visibility.transmittance(scene);
				}
				//printf("is light black %d\n",L.black());
				if (rayDepth[get_thread_logic_id()]++ < maxDepth) {
					// Trace rays for specular reflection and refraction
					spectrum_t f = bsdf->sample_f(wo, wi,
						BxDFType(BSDF_REFLECTION | BSDF_SPECULAR));
					if (!f.black()) {
						// Compute ray differential _rd_ for specular reflection
						ray_differential_t rd(p, wi);
						rd.has_differential = true;
						rd.rx.o = p + isect.dg.dpdx;
						rd.ry.o = p + isect.dg.dpdy;
						// Compute differential reflected directions
						vector_t dndx = bsdf->dg_shading.dndu * bsdf->dg_shading.dudx +
							bsdf->dg_shading.dndv * bsdf->dg_shading.dvdx;
						vector_t dndy = bsdf->dg_shading.dndu * bsdf->dg_shading.dudy +
							bsdf->dg_shading.dndv * bsdf->dg_shading.dvdy;
						vector_t dwodx = -ray.rx.dir - wo, dwody = -ray.ry.dir - wo;
						scalar_t dDNdx = dwodx.dot(n)+ dot(wo, dndx);
						scalar_t dDNdy = dwody.dot(n) + dot(wo, dndy);
						rd.rx.dir = wi -
							dwodx + 2 * ( wo.dot( n) * dndx +
							vector_t(dDNdx * n));
						rd.ry.dir = wi -
							dwody + 2 * ( (wo.dot(n) )* dndy +
							vector_t(dDNdy * n));
						L += scene->li(rd, sample) * f * std::abs(wi.dot( n));
					}
					f = bsdf->sample_f(wo, wi,
						BxDFType(BSDF_TRANSMISSION | BSDF_SPECULAR));
					if (!f.black()) {
						// Compute ray differential _rd_ for specular transmission
						ray_differential_t rd(p, wi);
						rd.has_differential = true;
						rd.rx.o = p + isect.dg.dpdx;
						rd.ry.o = p + isect.dg.dpdy;

						scalar_t eta = bsdf->eta;
						vector_t w = -wo;
						if ( (wo.dot(n) ) < 0) eta = reciprocal(eta);

						vector_t dndx = bsdf->dg_shading.dndu * bsdf->dg_shading.dudx + bsdf->dg_shading.dndv * bsdf->dg_shading.dvdx;
						vector_t dndy = bsdf->dg_shading.dndu * bsdf->dg_shading.dudy + bsdf->dg_shading.dndv * bsdf->dg_shading.dvdy;

						vector_t dwodx = -ray.rx.dir - wo, dwody = -ray.ry.dir - wo;
						scalar_t dDNdx = dwodx.dot(n) + dot(wo, dndx);
						scalar_t dDNdy = dwody.dot( n) + dot(wo, dndy);

						scalar_t mu = eta * (w.dot( n)) -  (wi.dot(n) );
						scalar_t dmudx = (eta - (eta*eta*w.dot(n))/ (wi.dot(n) )) * dDNdx;
						scalar_t dmudy = (eta - (eta*eta* (w.dot(n)))/(wi.dot( n ))) * dDNdy;

						rd.rx.dir = wi + eta * dwodx - (mu * dndx + vector_t(dmudx * n));
						rd.ry.dir = wi + eta * dwody - (mu * dndy + vector_t(dmudy * n));
						L += scene->li(rd, sample) * f * std::abs(wi.dot(n) );
					}
				}
				--rayDepth[get_thread_logic_id()];
			}
			return L;
	}
}

namespace ma
{
	MAKE_TYPE_STR_MAP(1,WhittedIntegrator,whitted)
namespace details
{
	template<typename C>
		struct integrator_creator<WhittedIntegrator<C> >
		{
			typedef WhittedIntegrator<C> surface_integrator_t;	
			surface_integrator_t* 
				operator()(const ParamSet& param)const
				{
					int max_depth = param.as<float>("maxdepth",5);
					return new surface_integrator_t(max_depth);
				}
		};
}

}
#endif
