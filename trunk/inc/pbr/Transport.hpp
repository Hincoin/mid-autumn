#ifndef _MA_INCLUDED_TRANSPORT_HPP_
#define _MA_INCLUDED_TRANSPORT_HPP_


#include "CRTPInterfaceMacro.hpp"
#include "ptr_var.hpp"
namespace ma{
	namespace integrator{
		DECL_FUNC_NEST(spectrum_t,li,4)
		DECL_FUNC(void,preprocess,1)
		DECL_FUNC(void,requestSamples,2)	
	}
	BEGIN_CRTP_INTERFACE(Integrator)
		ADD_CRTP_INTERFACE_TYPEDEF(spectrum_t);
		ADD_CRTP_INTERFACE_TYPEDEF(scene_ptr);
		ADD_CRTP_INTERFACE_TYPEDEF(ray_differential_t);
		ADD_CRTP_INTERFACE_TYPEDEF(sample_ptr);
		ADD_CRTP_INTERFACE_TYPEDEF(scalar_t);
		ADD_CRTP_INTERFACE_TYPEDEF(sample_t);

		CRTP_CONST_METHOD(spectrum_t,li,4,
			( I_(const scene_ptr,scene), I_(const ray_differential_t&,r), I_(const sample_ptr,sample), I_(scalar_t&,alpha)));

		CRTP_METHOD(void,preprocess,1,( I_(const scene_ptr,s)));
		CRTP_METHOD(void,requestSamples,2,( I_(sample_t&,s), I_(const scene_ptr,scene)));
		END_CRTP_INTERFACE
}

namespace ma{
	namespace details{
	template<typename I>
	struct integrator_creator;	
	}
	template <typename I>
		I* create_integrator(const ParamSet& param)
		{
			return details::integrator_creator<I>()(param);
		}
}

#include "MonteCarlo.hpp"
#include "Reflection.hpp"
#include "Light.hpp"
namespace ma{
//integrator utility
//
//

template<typename IntegratorCfg,typename ScenePtr,typename LightPtr,typename PointT,
	typename NormalT,typename VectorT,typename BSDFT,typename SampleT
	>
		typename IntegratorCfg::spectrum_t EstimateDirect(const ScenePtr scene,
				const LightPtr lght,const PointT& p,
				const NormalT &n,const VectorT &wo,
				BSDFT& bsdf,const SampleT &sample,int lightSamp,
				int bsdfSamp, int bsdfComponent,size_t sampleNum)
		{
			typedef typename IntegratorCfg::spectrum_t SpectrumT;
			SpectrumT Ld(0.);
			// 
			typedef typename SampleT::scalar_t scalar_t;
			scalar_t ls1,ls2,bs1,bs2,bcs;
			if (lightSamp != -1 && bsdfSamp != -1 &&
					sampleNum < sample.n2D[lightSamp] && 
					sampleNum < sample.n2D[bsdfSamp])
			{
				ls1 = sample.twoD[lightSamp][2*sampleNum];
				ls2 = sample.twoD[lightSamp][2*sampleNum + 1];
				bs1 = sample.twoD[bsdfSamp][2*sampleNum];
				bs2 = sample.twoD[bsdfSamp][2*sampleNum + 1];
				bcs = sample.oneD[bsdfComponent][sampleNum];
			}
			else
			{
				ls1 = RandomFloat();
				ls2 = RandomFloat();
				bs1 = RandomFloat();
				bs2 = RandomFloat();
				bcs = RandomFloat();
			}
			VectorT wi;
			scalar_t lightPdf=0.,bsdfPdf=0.;

			typedef typename IntegratorCfg::visibility_tester_t visibility_tester_t;
			visibility_tester_t visibility;
			SpectrumT Li = light::sample_l(lght,p,n,ls1,ls2,ref(wi),ref(lightPdf),ref(visibility));
			if(lightPdf > 0. && !Li.black())
			{
				SpectrumT f = bsdf.f(wo,wi);
				if(!f.black() && visibility.unOccluded(scene))
				{
					Li *= visibility.transmittance(scene);
					if(light::isDeltaLight(lght))
					{
						Ld += f * Li * abs_dot(wi,n) * reciprocal(lightPdf);
					}
					else
					{
						bsdfPdf = bsdf.pdf(wo,wi);
						scalar_t weight = PowerHeuristic(1,lightPdf,1,bsdfPdf);
						Ld += f * Li * abs_dot(wi,n) * weight *reciprocal(lightPdf);
					}
				}
			}
			if(light::isDeltaLight(lght))
			{
				BxDFType flags = BxDFType(BSDF_ALL & ~BSDF_SPECULAR);
			   SpectrumT f = bsdf.sample_f(wo,wi,bs1,bs2,bcs,bsdfPdf,flags);
		   		if(!f.black() && bsdfPdf > 0.)		   
				{
					lightPdf = light::pdf(lght,p,n,wi);
					if(lightPdf > 0.f)
					{
						scalar_t weight = PowerHeuristic(1,bsdfPdf,1,lightPdf);
						typename IntegratorCfg::intersection_t lightIsect;
						typename IntegratorCfg::ray_differential_t ray(p,wi);
						SpectrumT Li(0);
						if(scene->intersect(ray,lightIsect))
						{
							//area light not support
							//if(lightIsect.primitive->getAreaLight() == lght)
							//		Li = lightIsect.le(-wi);
						}
						else
							Li = light::le(lght,ray);
						if(!Li.black())
						{
							Li *= scene->transmittance(ray);
							Ld += f * Li * abs_dot(wi,n) * weight * reciprocal(bsdfPdf);
						}
					}	
				}
			}
			return Ld;	
		}
template<typename IntegratorCfg,typename ScenePtr,typename PointT,
	typename NormalT,typename VectorT,typename BSDFT,typename SampleT
	>
	typename IntegratorCfg::spectrum_t UniformSampleAllLights(const ScenePtr scene,
				const PointT& p,const NormalT &n,const VectorT &wo,
				BSDFT& bsdf,const SampleT& sample,
				int *lightSampleOffset,
				int *bsdfSampleOffset,
				int *bsdfComponentOffset)
		{
			typedef typename IntegratorCfg::spectrum_t SpectrumT;
			SpectrumT L(0.);
			for(size_t i = 0;i < scene->lights.size();++i)
			{
				int nSamples = sample.n2D[lightSampleOffset[i]];
				SpectrumT Ld(0.);
				for(int j = 0;j < nSamples; ++j)
					Ld += EstimateDirect<IntegratorCfg>(scene,scene->lights[i],p,n,wo,bsdf,
							sample,lightSampleOffset[i],bsdfSampleOffset[i],
							bsdfComponentOffset[i],j);
				L += Ld / nSamples;
			}	
			return L;
		}
template<typename IntegratorCfg,typename ScenePtr,typename PointT,
	typename NormalT,typename VectorT,typename BSDFT,typename SampleT
	>
	typename IntegratorCfg::spectrum_t
   	UniformSampleOneLight(const ScenePtr scene,const PointT& p,const NormalT& n,
	 const VectorT& wo,BSDFT& bsdf,const SampleT &sample,
	 int lightSampleOffset,int lightNumOffset,
	 int bsdfSampleOffset,int bsdfComponentOffset)
	{
		typedef typename IntegratorCfg::spectrum_t SpectrumT;
		typedef typename IntegratorCfg::scalar_t scalar_t;
		int nLights = int(scene->lights.size());	
		int lightNum;
		if (lightNumOffset != -1)
			lightNum = floor32(sample.oneD[lightNumOffset][0] * nLights);
		else
			lightNum = floor32(RandomFloat() * nLights);
		lightNum = std::min(lightNum ,nLights-1);
		return (((scalar_t)nLights * 
				EstimateDirect<IntegratorCfg>(
					scene,scene->lights[lightNum],p,n,wo,bsdf,sample,
					lightSampleOffset,bsdfSampleOffset,
					bsdfComponentOffset,0)));
	}
	
template<typename IntegratorCfg,typename ScenePtr,typename PointT,
	typename NormalT,typename VectorT,typename BSDFT,typename SampleT
	>
	typename IntegratorCfg::spectrum_t 
	WeightedSampleOneLight(const ScenePtr scene,const PointT& p,const NormalT& n,
	 const VectorT& wo,BSDFT& bsdf,
	 const SampleT& sample,int lightSampleOffset,
	 int lightNumOffset,int bsdfSampleOffset,
	 int bsdfComponentOffset,typename IntegratorCfg::scalar_t* &avgY,
	typename IntegratorCfg::scalar_t *&avgYsample,typename IntegratorCfg::scalar_t *&cdf,typename IntegratorCfg::scalar_t &overallAvgY
	 )
	{
		typedef typename IntegratorCfg::scalar_t scalar_t;
		typedef typename IntegratorCfg::spectrum_t SpectrumT;
		int nLights = int(scene->lights.size());	
		if(!avgY)
		{
			avgY = new scalar_t[nLights];
			avgYsample = new scalar_t[nLights];
			cdf = new scalar_t [nLights + 1];
			for (int i = 0;i < nLights;++i)
				avgY[i] = avgYsample[i] = 0.;
		}
		SpectrumT L(0.);
		if (overallAvgY == 0.)
		{
			L = UniformSampleOneLight<IntegratorCfg>(scene,p,n,
				wo,bsdf,sample,lightSampleOffset,
				lightNumOffset,bsdfSampleOffset,
				bsdfComponentOffset);	
			scalar_t luminance = L.y();
			overallAvgY = luminance;
			for(int i = 0;i < nLights; ++i)
				avgY[i] = luminance;
		}
		else
		{
			scalar_t c,lightSampleWeight;
			for(int i = 0;i < nLights; ++i)
				avgYsample[i] = std::max(avgY[i],0.1f*overallAvgY);
			ComputeStep1dCDF(avgYsample,nLights,c,cdf);
			scalar_t t = SampleStep1d(
					avgYsample,cdf,c,nLights,
					sample.oneD[lightNumOffset][0],lightSampleWeight);
			int lightNum = std::min(floor32(nLights * t),nLights-1);
			L = EstimateDirect<IntegratorCfg>(scene,scene->lights[lightNum],p,n,wo,bsdf,
					sample,lightSampleOffset,bsdfSampleOffset,
					bsdfComponentOffset,0);
			scalar_t luminance = L.y();
			avgY[lightNum]=
				ExponentialAverage(avgY[lightNum],luminance,.99f);
			overallAvgY = 
				ExponentialAverage(overallAvgY,luminance,0.999f);
			L /= lightSampleWeight;
		}

	}
	
}
#endif
