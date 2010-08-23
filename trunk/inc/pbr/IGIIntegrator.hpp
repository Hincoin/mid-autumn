#ifndef _MA_INCLUDED_IGIINTEGRATOR_HPP_
#define _MA_INCLUDED_IGIINTEGRATOR_HPP_

#include "Transport.hpp"
#include "MonteCarlo.hpp"
#include "MAMath.hpp"

#include <vector>

namespace ma
{
namespace igi{
template <typename Conf>
		struct VirtualLight{
				ADD_SAME_TYPEDEF(Conf,point_t);
				ADD_SAME_TYPEDEF(Conf,normal_t);
				ADD_SAME_TYPEDEF(Conf,spectrum_t);

		VirtualLight(){}
		VirtualLight(const point_t& pp,const normal_t& nn,const spectrum_t& le)
				:p(pp),n(nn),Le(le)
		{}
		point_t p;
		normal_t n;
		spectrum_t Le;
		};
}
//instant global illumination
template<typename Conf>
class IGIIntegrator:public Integrator<IGIIntegrator<Conf>,typename Conf::interface_config>
{
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
		ADD_SAME_TYPEDEF(Conf,ray_t);
		typedef  igi::VirtualLight<Conf> virtual_light_t;
		public:
		IGIIntegrator(int nl,int ns,float rrt,float maxdepth,float gl,
						int ngather);
		void preprocessImpl(const scene_ptr);
		spectrum_t liImpl(const scene_ptr scene,const ray_differential_t &ray,const sample_ptr sampl,scalar_t &alpha)const;
		void requestSamplesImpl(sample_t& sampl,const scene_ptr scene);
		~IGIIntegrator()
		{
				delete []lightSampleOffset;
				delete []bsdfSampleOffset;
				delete []bsdfComponentOffset;
		}
		private:
		unsigned nLightPaths,nLightSets;

		std::vector<std::vector<virtual_light_t> > virtualLights;
		mutable int specularDepth;
		int maxSpecularDepth;
		int nGatherSamples ;
		float rrThreshold;
		float gLimit;
		int vlSetOffset;
		int *lightSampleOffset,lightNumOffset;
		int *bsdfSampleOffset,*bsdfComponentOffset;
		int gatherSampleOffset,gatherComponentOffset;
};
template<typename Conf>
IGIIntegrator<Conf>::IGIIntegrator(int nl,int ns,float rrt,float maxdepth,float gl,
						int ngather)
{
		nGatherSamples = ngather;
		nLightPaths = RoundUpPow2((unsigned )nl);
		nLightSets = RoundUpPow2((unsigned)ns);
		virtualLights.resize(nLightSets);
		rrThreshold = rrt;
		maxSpecularDepth=maxdepth;
		specularDepth = 0;
		lightSampleOffset=NULL;
		bsdfSampleOffset=NULL;
		bsdfComponentOffset = NULL;
		gatherSampleOffset=gatherComponentOffset = 0;
		gLimit = gl;

}
template<typename Conf>
void IGIIntegrator<Conf>::requestSamplesImpl(typename Conf::sample_t& sampl,const typename Conf::scene_ptr scene)
{
		unsigned nLights = scene->lights.size();
		lightSampleOffset = new int[nLights];
		bsdfSampleOffset=new int [nLights];
		bsdfComponentOffset = new int[nLights]; 
		for(unsigned i = 0;i < nLights;++i)
		{
				const int lightSamples = 
					    sampler::roundSize(scene->getSampler(),light::numSamples(
											   scene->lights[i]
											   ));	
				lightSampleOffset[i] = sampl.add2D(lightSamples);
				bsdfSampleOffset[i] = sampl.add2D(lightSamples);
				bsdfComponentOffset[i] = sampl.add1D(lightSamples);

		}
		lightNumOffset = -1;
		vlSetOffset = sampl.add1D(1);
		nGatherSamples = sampler::roundSize(scene->getSampler(),nGatherSamples);
		gatherSampleOffset = sampl.add2D(nGatherSamples);
		gatherComponentOffset = sampl.add1D(nGatherSamples);
}

template<typename Conf>
void IGIIntegrator<Conf>::preprocessImpl(const scene_ptr scene)
{
	if (scene->lights.size() == 0) return;
	printf("lights,nLightPaths,nLightSets:%d,%d,%d",scene->lights.size(),nLightPaths,nLightSets);
	float *lightNum = new float[nLightPaths * nLightSets];
	float *lightSamp0 = new float [2 * nLightPaths * nLightSets];
	float *lightSamp1 = new float [2 * nLightPaths * nLightSets];
	LDShuffleScrambled1D(nLightPaths, nLightSets, lightNum);
	LDShuffleScrambled2D(nLightPaths, nLightSets, lightSamp0);
	LDShuffleScrambled2D(nLightPaths, nLightSets, lightSamp1);
	// precompute information for light sampling densities
	int nLights = int(scene->lights.size());
	float *lightPower = (float*)alloca(nLights*sizeof(float));
	float *lightCDF = (float*)alloca((nLights+1) * sizeof(float));
	for(int i = 0;i < nLights;++i)
			lightPower[i] = light::power(scene->lights[i],scene).y();
	float totalPower;
	ComputeStep1dCDF(lightPower,nLights,(&totalPower),lightCDF);
	for(unsigned s = 0;s < nLightSets; ++s)
	{
			for(unsigned i = 0;i < nLightPaths; ++i)
			{
					int sampOffset = s * nLightPaths + i;
					float lightPdf;
					int lNum = floor32(SampleStep1d<float>(lightPower,lightCDF,
											totalPower,nLights,
											lightNum[sampOffset],
											(lightPdf)) * nLights);
					ray_differential_t ray;
					float pdf;
					spectrum_t alpha = 
							light::sample_l(
											scene->lights[lNum],
											scene,
											lightSamp0[2*sampOffset],
											lightSamp0[2*sampOffset+1],
											lightSamp1[2*sampOffset],
											lightSamp1[2*sampOffset+1],
											ref(ray),
											ref(pdf));
					if(pdf == 0.f || alpha.black())continue;
					alpha /= pdf * lightPdf;
					printf("pdf,lightPdf,alpha:%f,%f,%f\n",pdf,lightPdf,alpha.y());
					intersection_t isect;
					int nIntersections=0;
					while(scene->intersect(ray,isect) && !alpha.black())
					{
							++nIntersections;
							alpha *= scene->transmittance(ray);
							vector_t wo = -ray.dir;
							bsdf_ptr bsdf = isect.getBSDF(ray);
							//todo
							spectrum_t Le = alpha * bsdf->rho(wo)/M_PI;
							virtualLights[s].push_back(
											virtual_light_t(isect.dg.point,isect.dg.normal,Le)
											);
							printf("virtual light[%d].size: %d,\nilu:%f,pos(%f,%f,%f)\t",s,virtualLights[s].size(),Le.y(),
											isect.dg.point[0],isect.dg.point[1],isect.dg.point[2]);
							vector_t wi;
							float pdf;
							BxDFType flags;
							spectrum_t fr = bsdf->
									sample_f(wo,(wi),
													RandomFloat(),
													RandomFloat(),
													RandomFloat(),
													(pdf),
													BSDF_ALL,
													(&flags));
							if(fr.black() || pdf == 0.f)
									break;
							spectrum_t alpha_scale = fr * 
									abs_dot(wi,bsdf->dg_shading.normal)/pdf;
							float r = std::min(1.f,alpha_scale.y());
							if(RandomFloat() > r)
									break;
							alpha *= alpha_scale /r;
							ray = ray_differential_t(isect.dg.point,wi);
					}
					printf("nIntersections : %d\n",nIntersections);
			}
	}
	delete[] lightNum;
	delete[] lightSamp0;
	delete[] lightSamp1;
}

template <typename Conf>
typename Conf::spectrum_t 
IGIIntegrator<Conf>::liImpl(const scene_ptr scene,const ray_differential_t &ray,const sample_ptr sampl,scalar_t &alpha)const
{
		spectrum_t L(0.f);
		intersection_t isect;
		if(scene->intersect(ray,ref(isect)))
		{
				alpha = 1.f;
				vector_t wo = -ray.dir;
				L += isect.le(wo);
				bsdf_ptr bsdf = isect.getBSDF(ray);
				const point_t &p = bsdf->dg_shading.point;
				const normal_t &n = bsdf->dg_shading.normal;

				L += UniformSampleAllLights<Conf>(
								scene,
								p,n,wo,*bsdf,*sampl,
								lightSampleOffset,bsdfSampleOffset,
								bsdfComponentOffset);
				//copute indirect illumination
				unsigned lSet = 
						std::min(unsigned(
												sampl->oneD[vlSetOffset][0]
												*
												nLightSets),
										nLightSets-1);
				//printf("lSet %d,%d",lSet,virtualLights[lSet].size());
				for(unsigned i = 0;i < virtualLights[lSet].size();++i)
				{
						const virtual_light_t &vl = virtualLights[lSet][i];
						float d2 = distanceSquared(p,vl.p);
						vector_t wi = (vl.p - p);
						normalize(wi);
						spectrum_t f = bsdf->f(wo,wi);
						
						if(f.black()){continue;}
						float G = abs_dot(wi,n) * abs_dot(wi,vl.n)/d2;
						G = std::min(G,gLimit);
						if (G == 0.f ){ continue;}
						spectrum_t Llight=
								f * G * vl.Le/
								virtualLights[lSet].size();
						Llight *= scene->transmittance(ray_t(p,vl.p-p));
						if(Llight.y() < rrThreshold)
						{
								float continueProbability = 0.1f;
								if(RandomFloat() > continueProbability)
										continue;
								Llight /= continueProbability;
						}
						//printf("Llight:%f,!intersect_p:%d\n",Llight.y(),!scene->intersect_p(ray_t(p,vl.p-p,ray_t::epsilon,
						//								1.f-ray_t::epsilon)));

						if(!scene->intersect_p(ray_t(p,vl.p-p,ray_t::epsilon,
														1.f-ray_t::epsilon)))
						{
								L += Llight;
								//if(specularDepth>0)
							//	printf("f,G,vl.Le,Llight:%f,%f,%f,%f\n",f.y(),G,vl.Le.y(),Llight.y());
						}

				}
				if ( specularDepth+1 < maxSpecularDepth )
				{
						//do bias compensation for bounding geometry term
						int nSamples = specularDepth == 0 ? 
								nGatherSamples : 1;
						for(int i = 0; i < nSamples; ++i)
						{
								vector_t wi;
								float pdf;
								float sampl0,sampl1,sampl2;
								if(specularDepth == 0)
								{
										sampl0 = sampl->twoD[ gatherSampleOffset ][0];
										sampl1 = sampl->twoD[ gatherSampleOffset ][1];
										sampl2 = sampl->oneD[ gatherComponentOffset ][0];
								}
								else
								{
										sampl0 = RandomFloat();
										sampl1 = RandomFloat();
										sampl2 = RandomFloat();
								}
								spectrum_t f = bsdf->sample_f										(wo,wi,sampl0,sampl1,sampl2,pdf,										 BxDFType(BSDF_ALL & ~BSDF_SPECULAR));
								if( !f.black() && pdf > 0.f)
								{
										//trace ray for bias compensation
										float maxDist = std::sqrt(abs_dot(wi,n) / gLimit);
										ray_differential_t gatherRay(p, wi,
														ray_t::epsilon,
														maxDist
														);
										intersection_t gatherIsect;
										if( scene->intersect(gatherRay,gatherIsect) )
										{

											// add bias compensation
												float Ggather = 
														abs_dot(wi, n) * abs_dot(vector_t(-wi), gatherIsect.dg.normal) / distanceSquared(p, gatherIsect.dg.point);
												if(Ggather - gLimit > 0.f && !isinf(Ggather))
												{
														specularDepth++;
														spectrum_t Li = 
																scene->li(gatherRay,sampl);

														specularDepth--;
														if(Li.black())continue;

														float gs = 
																(Ggather - gLimit) / Ggather * abs_dot(vector_t(-wi), gatherIsect.dg.normal);
														L += f * Li * (abs_dot(wi, n) * gs / (nSamples * pdf));
												}


										}
								}

						}
				}
				if (specularDepth++ < maxSpecularDepth)
				{
						vector_t wi;
						spectrum_t f = 
								bsdf->sample_f(wo,ref(wi),
												BxDFType(BSDF_REFLECTION |
														BSDF_SPECULAR));
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
								L += scene->li(rd, sampl) * f * std::abs(wi.dot( n));
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
								L += scene->li(rd, sampl) * f * std::abs(wi.dot(n) );
						}

				}
				--specularDepth;

		}
		else
		{
				alpha = 0.f;
				for(size_t i = 0;i < scene->lights.size();++i)
						L += light::le(scene->lights[i],ray);
				if(!L.black()) alpha = 1.f;
				return L;
		}
		return L;
}

}


namespace ma
{
	MAKE_TYPE_STR_MAP(1,IGIIntegrator,igi)
namespace details
{
	template<typename C>
		struct integrator_creator<IGIIntegrator<C> >
		{
			typedef IGIIntegrator<C> surface_integrator_t;	
			surface_integrator_t* 
				operator()(const ParamSet& param)const
				{

					int nLights= param.as<int>("nlights",64);
					int nLightSets = param.as<int>("nsets",4);
					int maxDepth = param.as<int>("maxdepth",5);
					float glimit = param.as<float>("glimit",10.f);
					float rrThresh = param.as<float>("rrthreshold",.0001f);
					int gatherSamples = param.as<int>("gathersamples",16);
					return new surface_integrator_t(nLights,nLightSets,rrThresh,maxDepth,
									glimit, gatherSamples);
				}
		};
}

}

#endif
