#ifndef _MA_INCLUDED_BIDIRECTIONAL_INTEGRATOR_HPP_
#define _MA_INCLUDED_BIDIRECTIONAL_INTEGRATOR_HPP_


#include "Transport.hpp"
namespace ma
{
	namespace bidirectional
	{
		template<typename Conf>
			struct BidirVertex
			{
				ADD_SAME_TYPEDEF(Conf,bsdf_ptr);
				ADD_SAME_TYPEDEF(Conf,point_t);
				ADD_SAME_TYPEDEF(Conf,normal_t);
				ADD_SAME_TYPEDEF(Conf,vector_t);
				ADD_SAME_TYPEDEF(Conf,scalar_t);
				ADD_SAME_TYPEDEF(Conf,intersection_t);
				ADD_SAME_TYPEDEF(Conf,spectrum_t);
				BxDFType flags;
				bsdf_ptr bsdf;
				point_t p;
				normal_t ng,ns;
				vector_t wi,wo;
				scalar_t bsdfWeight,dAWeight,rrWeight;
				bool specularBounce;
				int nSpecularComponents;
				spectrum_t cumulative,add_cumulative;
				intersection_t isect;
				BidirVertex():cumulative(0.f),add_cumulative(1.f){bsdfWeight = dAWeight = 0.f;rrWeight = 1.f;flags = BxDFType(0);bsdf=bsdf_ptr();specularBounce=false;nSpecularComponents=0;}
			};
	}
	template<typename Conf>
		class BidirectionalIntegrator:public Integrator<BidirectionalIntegrator<Conf>,typename Conf::interface_config>
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
		typedef  bidirectional::BidirVertex<Conf> bidirvertex_t;
		public:
		void preprocessImpl(const scene_ptr){}
		spectrum_t liImpl(const scene_ptr scene,const ray_differential_t &ray,const sample_ptr sampl,scalar_t &alpha)const;
		void requestSamplesImpl(sample_t& sampl,const scene_ptr scene);

		private:
		int generatePath(const scene_ptr scene,const ray_t&r,const sample_ptr sampl,
			const int *bsdfOffset,const int *bsdfCompOffset,
		bidirvertex_t *vertices,int maxVerts)const;
		scalar_t weightPath(bidirvertex_t *eye,int nEye,bidirvertex_t *lght,int nLight)const;
		spectrum_t evalPath(const scene_ptr scene,bidirvertex_t *eye,int nEye,
				bidirvertex_t *lght,int nLight)const;
		static scalar_t G(const bidirvertex_t &v0,const bidirvertex_t& v1);
		static bool visible(const scene_ptr scene,const point_t &P0,const point_t &P1);
		static const int MAX_VERTS = 8;
		int eyeBSDFOffset[MAX_VERTS],eyeBSDFCompOffset[MAX_VERTS];
		int lightBSDFOffset[MAX_VERTS],lightBSDFCompOffset[MAX_VERTS];
		int directLightOffset[MAX_VERTS],directLightNumOffset[MAX_VERTS];
		int directBSDFOffset[MAX_VERTS], directBSDFCompOffset[MAX_VERTS];
		int lightNumOffset ,lightPosOffset,lightDirOffset;
	};
	template<typename Conf>
		void BidirectionalIntegrator<Conf>::requestSamplesImpl(typename Conf::sample_t& sample,
				const typename Conf::scene_ptr scene)
		{
			for(int i = 0 ;i < MAX_VERTS; ++i)
			{
				eyeBSDFOffset[i] = sample.add2D(1);
				eyeBSDFCompOffset[i] = sample.add1D(1);
				lightBSDFOffset[i] = sample.add2D(1);
				lightBSDFCompOffset[i] = sample.add1D(1);
				directLightOffset[i] = sample.add2D(1);
				directLightNumOffset[i] = sample.add1D(1);
				directBSDFOffset[i] = sample.add2D(1);
				directBSDFCompOffset[i]  = sample.add1D(1);
			}
			lightNumOffset = sample.add1D(1);
			lightPosOffset = sample.add2D(1);
			lightDirOffset = sample.add2D(1);
		}
	template<typename Conf>
			typename Conf::spectrum_t BidirectionalIntegrator<Conf>::liImpl(const typename Conf::scene_ptr scene,
							const typename Conf::ray_differential_t &ray,const typename Conf::sample_ptr sample,
							typename Conf::scalar_t &alpha)const
			{
					spectrum_t L(0.);
					// Generate eye and light sub-paths
					bidirvertex_t eyePath[MAX_VERTS], lightPath[MAX_VERTS];
					int nEye = generatePath(scene, ray, sample, eyeBSDFOffset,
									eyeBSDFCompOffset, eyePath, MAX_VERTS);
					if (nEye == 0) {
							alpha = 0.;
					}
					else
							alpha = 1;
					// Choose light for bidirectional path
					int lightNum = floor32(sample->oneD[lightNumOffset][0] *
									scene->lights.size());
					lightNum = std::min(lightNum, (int)scene->lights.size() - 1);
					float lightWeight = float(scene->lights.size());
					// Sample ray from light source to start light path
					ray_t lightRay;
					float lightPdf;
					float u[4];
					u[0] = sample->twoD[lightPosOffset][0];
					u[1] = sample->twoD[lightPosOffset][1];
					u[2] = sample->twoD[lightDirOffset][0];
					u[3] = sample->twoD[lightDirOffset][1];
					normal_t Nl;
					spectrum_t Le = light::sample_l(scene->lights[lightNum],scene,u[0],u[1],u[2],u[3],ref(lightRay),ref(lightPdf));
					int nLight = 0;
					if (Le.black() || lightPdf == 0.); 
					else
					{
							Le *=/*abs_dot(normalize(Nl),normalize(lightRay.dir)) * */lightWeight / lightPdf;
							nLight = generatePath(scene, lightRay, sample, lightBSDFOffset,
											lightBSDFCompOffset, lightPath, MAX_VERTS);
					}
					int nVerts = nEye + nLight +2;
					int *nSpecularVertices=(int*)alloca(nVerts * sizeof(int));
					memset(nSpecularVertices,0,nVerts * sizeof(int));
					for (int i = 0;i < nEye;++i)
							for(int j = 0;j < nLight;++j)
									if(eyePath[i].specularBounce || 
													lightPath[j].specularBounce)
											++nSpecularVertices[i+j+2];

					spectrum_t directWt(1.0);
					for (int i = 1; i <= nEye; ++i) {
							// Handle direct lighting for bidirectional integrator
							directWt /= eyePath[i-1].rrWeight; 
							spectrum_t localLe = directWt *
									UniformSampleOneLight<Conf>(scene, eyePath[i-1].p, eyePath[i-1].ng, eyePath[i-1].wi,
													*eyePath[i-1].bsdf, *sample, directLightOffset[i-1], directLightNumOffset[i-1],
													directBSDFOffset[i-1], directBSDFCompOffset[i-1]);
							if(i>1)
									localLe *= eyePath[i-2].cumulative;
							float pw =  1.f/(i-nSpecularVertices[i]);
							L +=/*	weightPath(eyePath, i,nEye, lightPath, 0,nLight)*/pw * (localLe + eyePath[i-1].add_cumulative);
							/*L += localLe   * eyePath[i-1].bsdf->f(eyePath[i-1].wi, eyePath[i-1].wo) *
							  AbsDot(eyePath[i-1].wo, eyePath[i-1].ng) /
							  eyePath[i-1].bsdfWeight*/;;

							/*directWt *= eyePath[i-1].bsdf->f(eyePath[i-1].wi, eyePath[i-1].wo) *
							  AbsDot(eyePath[i-1].wo, eyePath[i-1].ng) /
							  eyePath[i-1].bsdfWeight;*/
							for (int j = 1; j <= nLight; ++j)
							{
									L += Le * evalPath(scene, eyePath, i, lightPath, j) *
											1.f/(i+j - nSpecularVertices[i+j])	;
									//	weightPath(eyePath, i,nEye, lightPath, j,nLight);
							}
					}
					return L;
		}
	template<typename Conf>
		int 
		BidirectionalIntegrator<Conf>::generatePath(const typename Conf::scene_ptr scene,const typename Conf::ray_t &r,const typename Conf::sample_ptr sample,
			const int *bsdfOffset,const int *bsdfCompOffset,
		bidirvertex_t *vertices,int maxVerts)const
		{
				int nVerts = 0;
				ray_differential_t ray(r.o, r.dir);
				spectrum_t cumulative(1.f);// = scene->Transmittance(ray);
				//for eye
				bool specularBounce = false;
				spectrum_t add_cumulative(0.f);
				while (nVerts < maxVerts) {
						// Find next vertex in path and initialize _vertices_
						intersection_t isect;
						if (!scene->intersect(ray, isect))
								break;
						bidirvertex_t &v = vertices[nVerts];
						v.bsdf = isect.getBSDF(ray); // do before Ns is set!
						v.p = isect.dg.point;
						v.ng = isect.dg.normal;
						v.ns = v.bsdf->dg_shading.normal;
						v.wi = -ray.dir;
						v.isect = isect;
						//eye path
						if(nVerts == 0 || specularBounce)
						{
								add_cumulative += cumulative * isect.le(-ray.dir);
						}
						if(nVerts > 0)
								cumulative *= scene->transmittance(ray);
						v.add_cumulative = add_cumulative;
						//////////////////////////////

						// Possibly terminate bidirectional path sampling
						if (nVerts > 2) {
								float rrProb = .5f;
								if (RandomFloat() > rrProb)
										break;
								v.rrWeight =rrProb;// 1.f / rrProb;
						}
						++nVerts;
						// Initialize _ray_ for next segment of path
						float u1 = sample->twoD[bsdfOffset[nVerts-1]][0];
						float u2 = sample->twoD[bsdfOffset[nVerts-1]][1];
						float u3 = sample->oneD[bsdfCompOffset[nVerts-1]][0];

						cumulative *= v.bsdf->sample_f(v.wi, v.wo, u1, u2, u3,
										v.bsdfWeight, BSDF_ALL, &v.flags);
						v.specularBounce = (v.flags & BSDF_SPECULAR) != 0;
						v.nSpecularComponents = v.bsdf->numComponents(BxDFType(BSDF_SPECULAR | BSDF_TRANSMISSION|BSDF_REFLECTION));
						if (cumulative.black() && v.bsdfWeight == 0.f)
								break;
						cumulative *= abs_dot(v.wo,v.ng) / (v.bsdfWeight);
						v.cumulative = cumulative;
						specularBounce = (v.flags & BSDF_SPECULAR) != 0;
						ray = ray_differential_t(v.p, v.wo);
				}
				// Initialize additional values in _vertices_
				for (int i = 0; i < nVerts-1; ++i)
						vertices[i].dAWeight = vertices[i].bsdfWeight *
								abs_dot(vector_t(-vertices[i].wo), static_cast<vector_t&>(vertices[i+1].ng)) /
								distanceSquared(vertices[i].p, vertices[i+1].p);
				return nVerts;

			}
	template<typename Conf>
		typename Conf::scalar_t 
		BidirectionalIntegrator<Conf>::weightPath(typename BidirectionalIntegrator<Conf>::bidirvertex_t *eye,int nEye,typename BidirectionalIntegrator<Conf>::bidirvertex_t *lght,int nLight)const
		{
			return float(nEye + nLight);
		}
	template<typename Conf>
		typename Conf::spectrum_t 
		BidirectionalIntegrator<Conf>::evalPath(const typename Conf::scene_ptr scene,typename BidirectionalIntegrator<Conf>::bidirvertex_t *eye,int nEye,
				typename BidirectionalIntegrator<Conf>::bidirvertex_t *lght,int nLight)const
		{
				bidirvertex_t& ev=eye[nEye-1];
				bidirvertex_t& lv=lght[nLight-1];

				if (!visible(scene, ev.p, lv.p))
						return 0.;

				spectrum_t L(1.);
				if (nEye > 1)
						L *= eye[nEye-2].cumulative;
				if(L.black())return L;
				if (nLight> 1)
						L *= lght[nLight-2].cumulative;
				if(L.black())return L;
				vector_t w = lv.p - ev.p;
				vector_t we = w;
				vector_t wl = w;
				spectrum_t ef = ev.bsdf->f(ev.wi,we);
				if(ef.black())return 0.f;
				spectrum_t lf = lv.bsdf->f(-wl,lv.wi);
				if(lf.black())return 0.f;

				L *= (ev.nSpecularComponents + 1) * (lv.nSpecularComponents+1) * G(ev,lv) * ef * lf/(ev.rrWeight * lv.rrWeight);
				return L;
		}
	template<typename Conf>
		typename Conf::scalar_t 
		BidirectionalIntegrator<Conf>::G(const typename BidirectionalIntegrator<Conf>::bidirvertex_t &v0,const typename BidirectionalIntegrator<Conf>::bidirvertex_t& v1)
		{
				vector_t w = (v1.p - v0.p);
				normalize(w);
			return abs_dot(v0.ng, w) * abs_dot(v1.ng, -w) /
				distanceSquared(v0.p, v1.p);
		}
	template<typename Conf>
		bool 
		BidirectionalIntegrator<Conf>::visible(const typename Conf::scene_ptr scene,const typename Conf::point_t &P0,const typename Conf::point_t &P1)
		{
			ray_t ray(P0, P1-P0, ray_t::epsilon, 1.f - ray_t::epsilon);
			return !scene->intersect_p(ray);
		}
		
}



namespace ma
{
	MAKE_TYPE_STR_MAP(1,BidirectionalIntegrator,bidirectional)
namespace details
{
	template<typename C>
		struct integrator_creator<BidirectionalIntegrator<C> >
		{
			typedef BidirectionalIntegrator<C> surface_integrator_t;	
			surface_integrator_t* 
				operator()(const ParamSet& param)const
				{
					return new surface_integrator_t();
				}
		};
}

}
#endif
