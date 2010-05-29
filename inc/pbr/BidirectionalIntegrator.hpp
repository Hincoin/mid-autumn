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
				BxDFType flags;
				bsdf_ptr bsdf;
				point_t p;
				normal_t ng,ns;
				vector_t wi,wo;
				scalar_t bsdfWeight,dAWeight,rrWeight;
				BidirVertex(){bsdfWeight = dAWeight = 0.f;rrWeight = 1.f;flags = BxDFType(0);bsdf=bsdf_ptr();}
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
		return L;
	}
	alpha = 1;
	// Choose light for bidirectional path
	int lightNum = floor32(sample->oneD[lightNumOffset][0] *
		scene->lights.size());
	lightNum = std::min(lightNum, (int)scene->lights.size() - 1);
	//Light *light = scene->lights[lightNum];
	scalar_t lightWeight = float(scene->lights.size());
	// Sample ray from light source to start light path
	ray_t lightRay;
	float lightPdf;
	float u[4];
	u[0] = sample->twoD[lightPosOffset][0];
	u[1] = sample->twoD[lightPosOffset][1];
	u[2] = sample->twoD[lightDirOffset][0];
	u[3] = sample->twoD[lightDirOffset][1];
	spectrum_t Le = light::sample_l(scene->lights[lightNum],scene, u[0], u[1], u[2], u[3],
		ref(lightRay), ref(lightPdf));
	if (lightPdf == 0.) return 0.f;
	Le = lightWeight / lightPdf;
	int nLight = generatePath(scene, lightRay, sample, lightBSDFOffset,
		lightBSDFCompOffset, lightPath, MAX_VERTS);
	// Connect bidirectional path prefixes and evaluate throughput
	spectrum_t directWt(1.0);
	for (int i = 1; i <= nEye; ++i) {
		// Handle direct lighting for bidirectional integrator
		directWt /= eyePath[i-1].rrWeight;
		L += directWt *
			UniformSampleOneLight<Conf>(scene, eyePath[i-1].p, eyePath[i-1].ng, eyePath[i-1].wi,
			*(eyePath[i-1].bsdf), *sample, directLightOffset[i-1], directLightNumOffset[i-1],
			directBSDFOffset[i-1], directBSDFCompOffset[i-1]) /
				weightPath(eyePath, i, lightPath, 0);
		directWt *= eyePath[i-1].bsdf->f(eyePath[i-1].wi, eyePath[i-1].wo) *
			abs_dot(eyePath[i-1].wo, eyePath[i-1].ng) /
			eyePath[i-1].bsdfWeight;
		for (int j = 1; j <= nLight; ++j)
			L += Le * evalPath(scene, eyePath, i, lightPath, j) /
				weightPath(eyePath, i, lightPath, j);
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
	while (nVerts < maxVerts) {
		// Find next vertex in path and initialize _vertices_
		intersection_t isect;
		if (!scene->intersect(ray, ref(isect)))
			break;
		bidirvertex_t &v = vertices[nVerts];
		v.bsdf = isect.getBSDF(ray); // do before Ns is set!
		v.p = isect.dg.point;
		v.ng = isect.dg.normal;
		v.ns = v.bsdf->dg_shading.normal;
		v.wi = -ray.dir;
		++nVerts;
		// Possibly terminate bidirectional path sampling
		if (nVerts > 2) {
			float rrProb = .2f;
			if (RandomFloat() > rrProb)
				break;
			v.rrWeight = 1.f / rrProb;
		}
		// Initialize _ray_ for next segment of path
		float u1 = sample->twoD[bsdfOffset[nVerts-1]][0];
		float u2 = sample->twoD[bsdfOffset[nVerts-1]][1];
		float u3 = sample->oneD[bsdfCompOffset[nVerts-1]][0];
		spectrum_t fr = v.bsdf->sample_f(v.wi, ref(v.wo), u1, u2, u3,
			 ref(v.bsdfWeight), BSDF_ALL, &v.flags);
		printf("is relfection %d,%d",v.bsdf->type()&(BSDF_REFLECTION | BSDF_SPECULAR),fr.black());
		if (fr.black() && v.bsdfWeight == 0.f)
			break;
		ray = ray_differential_t(v.p, v.wo);
	}
	// Initialize additional values in _vertices_
	for (int i = 0; i < nVerts-1; ++i)
		vertices[i].dAWeight = vertices[i].bsdfWeight *
			abs_dot(vector_t(-vertices[i].wo), static_cast<vector_t&>( vertices[i+1].ng )) /
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
			spectrum_t L(1.);
			for (int i = 0; i < nEye-1; ++i)
				L *= eye[i].bsdf->f(eye[i].wi, eye[i].wo) *
					abs_dot(eye[i].wo, eye[i].ng) /
						(eye[i].bsdfWeight * eye[i].rrWeight);
			vector_t w = lght[nLight-1].p - eye[nEye-1].p;
			L *= eye[nEye-1].bsdf->f(eye[nEye-1].wi, w) *
				G(eye[nEye-1], lght[nLight-1]) *
				lght[nLight-1].bsdf->f(-w, lght[nLight-1].wi) /
				(eye[nEye-1].rrWeight * lght[nLight-1].rrWeight);
			for (int i = nLight-2; i >= 0; --i)
				L *= lght[i].bsdf->f(lght[i].wi, lght[i].wo) *
				abs_dot(lght[i].wo, lght[i].ng) /
				(lght[i].bsdfWeight * lght[i].rrWeight);
			if (L.black())
				return L;
			if (!visible(scene, eye[nEye-1].p, lght[nLight-1].p))
				return spectrum_t(0.);
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
