#ifndef _MA_INCLUDED_REFLECTION_HPP_
#define _MA_INCLUDED_REFLECTION_HPP_

#include "CRTPInterfaceMacro.hpp"


#include "VectorType.hpp"

#include "ptr_var.hpp"
#include "pool.hpp"
#include "malloc.h"

namespace ma{

	// BSDF Inline Functions
	template<typename V>
	inline typename scalar_type<V>::type CosTheta(const V &w) { return w.z(); }
	template<typename V>
	inline typename scalar_type<V>::type SinTheta(const V &w) {
		return std::sqrt(std::max(0.f, 1.f - w.z()*w.z()));
	}
	template<typename V>
	inline typename scalar_type<V>::type SinTheta2(const V &w) {
		return 1.f - CosTheta(w)*CosTheta(w);
	}
	template<typename V>
	inline typename scalar_type<V>::type CosPhi(const V &w) {
		return w.x() / SinTheta(w);
	}
	template<typename V>
	inline typename scalar_type<V>::type SinPhi(const V &w) {
		return w.y() / SinTheta(w);
	}
	template<typename V>
	inline typename scalar_type<V>::type SameHemisphere(const V &w,
		const V &wp) {
			return w.z() * wp.z() > 0.f;
	}

	namespace bxdf
	{
		MAKE_VISITOR(type,0);
		MAKE_VISITOR(matchesFlags,1)
		MAKE_VISITOR(f,2)
		MAKE_VISITOR(sample_f,5)
		MAKE_VISITOR(rho,3)
		MAKE_VISITOR(pdf,2)
		
	}


enum BxDFType{BSDF_REFLECTION = 1<<0,
BSDF_TRANSMISSION = 1<<1,
BSDF_DIFFUSE = 1<<2,
BSDF_GLOSSY = 1<<3,
BSDF_SPECULAR = 1<<4,
BSDF_ALL_TYPES = BSDF_DIFFUSE | BSDF_GLOSSY | BSDF_SPECULAR,
BSDF_ALL_REFLECTION = BSDF_REFLECTION | BSDF_ALL_TYPES,
BSDF_ALL_TRANSMISSION = BSDF_TRANSMISSION | BSDF_ALL_TYPES,
BSDF_ALL = BSDF_ALL_REFLECTION | BSDF_ALL_TRANSMISSION};

template<typename Conf>
class BSDF{
	typedef BSDF<Conf> class_type;
	public:
		ADD_SAME_TYPEDEF(Conf,spectrum_t)
		ADD_SAME_TYPEDEF(Conf,vector_t)
		ADD_SAME_TYPEDEF(Conf,scalar_t);
		ADD_SAME_TYPEDEF(Conf,differential_geometry_t)
		ADD_SAME_TYPEDEF(Conf,normal_t)
		ADD_SAME_TYPEDEF(Conf,BxDF_ptr)
		spectrum_t sample_f(const vector_t& o,vector_t& wi,scalar_t u1,
				scalar_t u2,scalar_t u3,scalar_t &pdf,BxDFType flags = BSDF_ALL,BxDFType* sampleType=0)const;
		spectrum_t sample_f(const vector_t& wo,vector_t& wi,
				BxDFType flags=BSDF_ALL,BxDFType* sampleType=0)const;
		scalar_t pdf(const vector_t&wo,
				const vector_t& wi,
				BxDFType flags = BSDF_ALL)const;
		BSDF(const differential_geometry_t &dgs,
				const normal_t& ngeom,
				scalar_t eta=1);
		void add(BxDF_ptr bxdf){
			assert(nBxDFs < MAX_BxDFS);
			bxdfs[nBxDFs++] = bxdf;
		}
		int numComponents()const{return nBxDFs;}
		int numComponents(BxDFType flags)const{
			int num = 0;
			for (int i = 0; i < nBxDFs; ++i)
				if (bxdf::matchesFlags_ref<bool>(bxdfs[i],flags)) ++num;
			return num;}
		bool hasShadingGeometry()const{return (nn.x() != ng.x() || nn.y() != ng.y() || nn.z() != ng.z());}
		vector_t worldToLocal(const vector_t& v)const{return vector_t(dot(v, sn), dot(v, tn), dot(v, nn));}
		vector_t localToWorld(const vector_t& v)const{
			return vector_t(sn.x() * v.x() + tn.x() * v.y() + nn.x() * v.z(),
			sn.y() * v.x() + tn.y() * v.y() + nn.y() * v.z(),
			sn.z() * v.x() + tn.z() * v.y() + nn.z() * v.z());}
		spectrum_t f(const vector_t& woW,const vector_t& wiW,
				BxDFType flags = BSDF_ALL)const;
		spectrum_t rho(BxDFType flags = BSDF_ALL, int nSamples = 6*6)const;
		spectrum_t rho(const vector_t& wo,
				BxDFType flags = BSDF_ALL, int nSamples = 6*6)const;
		const differential_geometry_t dg_shading;
		const scalar_t eta;

		~BSDF(){
			while(nBxDFs--)delete_ptr(bxdfs[nBxDFs]);
		}
	private:

		friend class NoClass;
		normal_t nn,ng;
		vector_t sn,tn;
		int nBxDFs;
		static const int MAX_BxDFS = 8;
		BxDF_ptr bxdfs[MAX_BxDFS];

	MA_DECLARE_POOL_NEW_DELETE_MT(class_type)
};


// BSDF Method Definitions
template<typename Conf>
typename Conf::spectrum_t BSDF<Conf>::sample_f(const vector_t &wo, vector_t &wi, BxDFType flags,
						BxDFType *sampledType) const {
							scalar_t pdf;
							spectrum_t f = sample_f(wo, wi, RandomFloat(), RandomFloat(),
								RandomFloat(), pdf, flags, sampledType);
							if (!f.black() && pdf > 0.) f /= pdf;
							return f;
}
template<typename Conf>
typename Conf::spectrum_t BSDF<Conf>::sample_f(const vector_t &woW, vector_t &wiW,
						scalar_t u1, scalar_t u2, scalar_t u3, scalar_t &pdf,
						BxDFType flags, BxDFType *sampledType) const {
							// Choose which _BxDF_ to sample
							int matchingComps = numComponents(flags);
							if (matchingComps == 0) {
								pdf = 0.f;
								return spectrum_t(0.f);
							}
							int which = std::min(floor32(u3 * matchingComps),
								matchingComps-1);
							BxDF_ptr bxdf_;
							int count = which;
							for (int i = 0; i < nBxDFs; ++i)
								if (bxdf::matchesFlags_ref<bool>(bxdfs[i],flags)/*bxdfs[i]->matchesFlags(flags)*/)
									if (count-- == 0) {
										bxdf_ = bxdfs[i];
										break;
									}
									assert(bxdf_); // NOBOOK
									// Sample chosen _BxDF_
									vector_t wi;
									vector_t wo = worldToLocal(woW);
									pdf = 0.f;
									spectrum_t f = bxdf::sample_f_ref<spectrum_t>(bxdf_,wo, wi, u1, u2, pdf); //bxdf->sample_f(wo, wi, u1, u2, pdf);
									if (pdf == 0.f) return 0.f;
									if (sampledType) *sampledType = bxdf::type_ref<BxDFType>(bxdf_);//bxdf->type;
									wiW = localToWorld(wi);
									// Compute overall PDF with all matching _BxDF_s
									if (!(bxdf::type_ref<BxDFType>(bxdf_) /*bxdf->type*/ & BSDF_SPECULAR) && matchingComps > 1) {
										for (int i = 0; i < nBxDFs; ++i) {
											if (bxdfs[i] != bxdf_ &&
												bxdf::matchesFlags_ref<bool>(bxdfs[i],flags)
												/*bxdfs[i]->matchesFlags(flags)*/)
												pdf += bxdf::pdf_ref<scalar_t>(bxdfs[i],wo,wi);//bxdfs[i]->pdf(wo, wi);
										}
									}
									if (matchingComps > 1) pdf /= matchingComps;
									// Compute value of BSDF for sampled direction
									if (!(bxdf::type_ref<BxDFType>(bxdf_) /*bxdf->type*/ & BSDF_SPECULAR)) {
										f = 0.;
										if (dot(wiW, ng) * dot(woW, ng) > 0)
											// ignore BTDFs
											flags = BxDFType(flags & ~BSDF_TRANSMISSION);
										else
											// ignore BRDFs
											flags = BxDFType(flags & ~BSDF_REFLECTION);
										for (int i = 0; i < nBxDFs; ++i)
											if (bxdf::matchesFlags_ref<bool>(bxdfs[i],flags)/*bxdfs[i]->matchesFlags(flags)*/)
												f += bxdf::f_ref<spectrum_t>(bxdfs[i],wo,wi);//bxdfs[i]->f(wo, wi);
									}
									return f;
}

template<typename Conf>
typename Conf::scalar_t BSDF<Conf>::pdf(const vector_t &woW, const vector_t &wiW,
				BxDFType flags) const {
					if (nBxDFs == 0.) return 0.;
					vector_t wo = worldToLocal(woW), wi = worldToLocal(wiW);
					scalar_t pdf = scalar_t();
					int matchingComps = 0;
					for (int i = 0; i < nBxDFs; ++i)
						if (bxdf::matchesFlags_ref<bool>(bxdfs[i],flags)/*bxdfs[i]->MatchesFlags(flags)*/) {
							++matchingComps;
							pdf += bxdf::pdf_ref<scalar_t>(bxdfs[i],wo,wi);//bxdfs[i]->pdf(wo, wi);
						}
						return matchingComps > 0 ? pdf / matchingComps : 0.f;
}

template<typename Conf>
BSDF<Conf>::BSDF(const differential_geometry_t &dg,
		   const normal_t &ngeom, scalar_t e)
		   : dg_shading(dg), eta(e) {
			   ng = ngeom;
			   nn = dg_shading.normal;
			   sn = (dg_shading.dpdu.normalized());
			   tn = nn.cross(sn);
			   nBxDFs = 0;
}
template<typename Conf>
typename Conf::spectrum_t BSDF<Conf>::f(const vector_t &woW,
				 const vector_t &wiW, BxDFType flags) const {
					 vector_t wi = worldToLocal(wiW), wo = worldToLocal(woW);
					 if (dot(wiW, ng) * dot(woW, ng) > 0)
						 // ignore BTDFs
						 flags = BxDFType(flags & ~BSDF_TRANSMISSION);
					 else
						 // ignore BRDFs
						 flags = BxDFType(flags & ~BSDF_REFLECTION);
					 spectrum_t f = 0.;
					 for (int i = 0; i < nBxDFs; ++i)
						 if (bxdf::matchesFlags_ref<bool>(bxdfs[i],flags) /*bxdfs[i]->matchesFlags(flags)*/)
							 f += bxdf::f_ref<spectrum_t>(bxdfs[i],wo,wi);//bxdfs[i]->f(wo, wi);
					 return f;
}
template<typename Conf>
typename Conf::spectrum_t BSDF<Conf>::rho(BxDFType flags,int nSamples) const {
		scalar_t *s1 = (scalar_t*)alloca(sizeof(scalar_t) * nSamples * 2);
		LatinHypercube(s1, nSamples, 2);
		scalar_t *s2 = (scalar_t*)alloca(sizeof(scalar_t) * nSamples * 2);
		LatinHypercube(s2, nSamples, 2);
	spectrum_t ret(0.);
	
	for (int i = 0; i < nBxDFs; ++i)
		if (bxdf::matchesFlags_ref<bool>(bxdfs[i],flags)/*bxdfs[i]->MatchesFlags(flags)*/)
			ret += bxdf::rho<spectrum_t(int,scalar_t*,scalar_t*)>(bxdfs[i],nSamples,s1,s2);//bxdfs[i]->rho();
	return ret;
}
template<typename Conf>
typename Conf::spectrum_t BSDF<Conf>::rho(const vector_t &wo, BxDFType flags,int nSamples) const {
		scalar_t *s1 = (scalar_t*)(alloca(sizeof(scalar_t) * 2 * nSamples));
		LatinHypercube(s1, nSamples, 2);
	spectrum_t ret(0.);
	for (int i = 0; i < nBxDFs; ++i)
		if (bxdf::matchesFlags_ref<bool>(bxdfs[i],flags) /*bxdfs[i]->MatchesFlags(flags)*/)
			ret += bxdf::rho<spectrum_t(const vector_t&,int,scalar_t*)>(bxdfs[i],wo,nSamples, s1);//bxdfs[i]->rho(wo);
	return ret;
}
//////////////////////////////////////////////////////////////////////////

namespace bxdf{
	DECL_FUNC(bool,matchesFlags,1)
	DECL_FUNC_NEST(spectrum_t,sample_f,5)
	DECL_FUNC_NEST(spectrum_t,rho,3)
	DECL_FUNC_NEST(scalar_t,pdf,2)
	DECL_FUNC_NEST(spectrum_t,f,2)
	DECL_FUNC(BxDFType,type,0)
}
//more bxdf types
BEGIN_CRTP_INTERFACE(BxDF)
public:
ADD_CRTP_INTERFACE_TYPEDEF(spectrum_t)
ADD_CRTP_INTERFACE_TYPEDEF(vector_t)
ADD_CRTP_INTERFACE_TYPEDEF(scalar_t);
BxDF(BxDFType t):type_(t){}
bool matchesFlags(BxDFType flags)const{return (type_ & flags) == type_;}

CRTP_CONST_METHOD(spectrum_t,f,2,( I_(const vector_t& , wo), I_(const vector_t& ,wi)));
//can be overrided
DECLARE_CONST_METHOD(spectrum_t,sample_f,5,( I_(const vector_t&,wo),
				   I_(vector_t&, wi),
				   I_(scalar_t,u1), I_(scalar_t,u2), I_(scalar_t&,pdf)));
DECLARE_CONST_METHOD(spectrum_t,rho,3,( I_(const vector_t&,wo),
				   I_(int , nSamples), I_(scalar_t*,samples)));
DECLARE_CONST_METHOD(spectrum_t,rho,3,( I_(int , nSamples), I_(scalar_t* , s1), I_(scalar_t* ,samples2)))

DECLARE_CONST_METHOD(scalar_t,pdf,2,( I_(const vector_t& ,wi), I_(const vector_t&,wo)))

BxDFType type()const{return type_;}
private:

	BxDFType type_;
template<typename Conf> friend  class BSDF;
END_CRTP_INTERFACE


template<typename D,typename Conf>
typename Conf::spectrum_t BxDF<D,Conf>::sample_f(const typename Conf::vector_t &wo, typename Conf::vector_t &wi,
						scalar_t u1, scalar_t u2, scalar_t &ppdf) const {
							// Cosine-sample the hemisphere, flipping the direction if necessary
							wi = CosineSampleHemisphere(u1, u2);
							if (wo.z() < 0.) wi.z() *= -1.f;
							ppdf = derived().pdf(wo, wi);
							return derived().f(wo, wi);
}
template<typename D,typename Conf>
typename Conf::scalar_t BxDF<D,Conf>::pdf(const typename Conf::vector_t &wo, const typename Conf::vector_t &wi) const {
	return
		SameHemisphere(wo, wi) ? fabsf(wi.z()) * INV_PI : 0.f;
}

template<typename D,typename Conf>
typename Conf::spectrum_t BxDF<D,Conf>::rho(const typename Conf::vector_t &w, int nSamples,
				   scalar_t *samples) const {
					   if (!samples) {
						   samples =
							   (scalar_t *)alloca(2 * nSamples * sizeof(scalar_t));
						   LatinHypercube(samples, nSamples, 2);
					   }
					   spectrum_t r = 0.;
					   for (int i = 0; i < nSamples; ++i) {
						   // Estimate one term of $\rho_{dh}$
						   vector_t wi;
						   scalar_t pdf = 0.f;
						   spectrum_t f =
							   derived().sample_f(w, wi, samples[2*i], samples[2*i+1], pdf);
						   if (pdf > 0.) r += f * fabsf(vector_op::z(wi)) / pdf;
					   }
					   return r / ( nSamples );
}
template<typename D,typename Conf>
typename Conf::spectrum_t BxDF<D,Conf>::rho(int nSamples,scalar_t* s1, scalar_t *s2) const {
	if (!s1) {
		s1 =
			(scalar_t *)alloca(2 * nSamples * sizeof(scalar_t));
		LatinHypercube(s1, nSamples, 2);
	}
	if (!s2) {
		s2 =
			(scalar_t *)alloca(2 * nSamples * sizeof(scalar_t));
		LatinHypercube(s2, nSamples, 2);
	}
	spectrum_t r = 0.;
	for (int i = 0; i < nSamples; ++i) {
		// Estimate one term of $\rho_{hh}$
		vector_t wo, wi;
		wo = UniformSampleHemisphere(s1[2*i], s1[2*i+1]);
		const float INV_TWOPI = INV_PI/2;
		scalar_t pdf_o = INV_TWOPI, pdf_i = 0.f;
		spectrum_t f =
			derived().sample_f(wo,  wi, s2[2*i], s2[2*i+1],
			 pdf_i);
		if (pdf_i > 0.)
			r += f * fabsf(wi.z * wo.z) / (pdf_o * pdf_i);
	}
	return r / (M_PI*nSamples);
}

//////////////////////////////////////////////////////////////////////////
namespace fresnel{
	DECL_FUNC_NEST(spectrum_t,evaluate,1)

}
BEGIN_CRTP_INTERFACE(Fresnel)
ADD_CRTP_INTERFACE_TYPEDEF(spectrum_t)
ADD_CRTP_INTERFACE_TYPEDEF(scalar_t)
public:
	CRTP_CONST_METHOD(spectrum_t,evaluate,1,( I_(scalar_t,cosi)))
END_CRTP_INTERFACE

template<typename C>
class FresnelNoOp:public Fresnel<FresnelNoOp<C>,typename C::interface_config>
{
	public:
	ADD_SAME_TYPEDEF(C,spectrum_t)
	ADD_SAME_TYPEDEF(C,scalar_t)
	public:
	spectrum_t evaluateImpl(scalar_t)const
	{return spectrum_t(1);}
};
template <typename C>
class FresnelConductor: public Fresnel<FresnelConductor<C>,typename C::interface_config>
{
	public:
	ADD_SAME_TYPEDEF(C,scalar_t)
	ADD_SAME_TYPEDEF(C,spectrum_t)
	public:
	FresnelConductor(const spectrum_t& e,const spectrum_t& kk)
		:eta_(e),k_(kk){}
	spectrum_t evaluateImpl(scalar_t cosi)const;
	private:
	spectrum_t eta_,k_;
};
template<typename C>
class FresnelDielectric:public Fresnel<FresnelDielectric<C>,typename C::interface_config>
{
	public:
	ADD_SAME_TYPEDEF(C,scalar_t)
	ADD_SAME_TYPEDEF(C,spectrum_t)
	public:
	FresnelDielectric(const scalar_t& ei,const scalar_t& et)
		:eta_i_(ei),eta_t_(et){}
	spectrum_t evaluateImpl(scalar_t cosi)const;
	private:
	scalar_t eta_i_,eta_t_;
};
template<typename C> 
class SpecularReflection:public BxDF<SpecularReflection<C>,typename C::interface_config>
{
	public:
		ADD_SAME_TYPEDEF(C,spectrum_t);
		ADD_SAME_TYPEDEF(C,scalar_t);
		ADD_SAME_TYPEDEF(C,vector_t);
		ADD_SAME_TYPEDEF(C,fresnel_ptr);
		typedef BxDF<SpecularReflection<C>,typename C::interface_config> parent_type;
		typedef SpecularReflection<C> class_type;
		MA_DECLARE_POOL_NEW_DELETE_MT(class_type)
	public:
		SpecularReflection(const spectrum_t& r,fresnel_ptr f)
			:parent_type(BxDFType(BSDF_REFLECTION | BSDF_SPECULAR)),
			r_(r),fresnel_(f){}
		spectrum_t fImpl(const vector_t&,const vector_t&)const
		{ return spectrum_t(0);}
		//override
		spectrum_t sample_f(const vector_t &wo,vector_t &wi,
				scalar_t u1,scalar_t u2,scalar_t& pdf)const;
		scalar_t pdfImpl(const vector_t& wo,const vector_t& wi)const{return scalar_t(0);}
		~SpecularReflection(){delete_ptr(fresnel_);}
	private:
		spectrum_t r_;
		fresnel_ptr fresnel_;
};
template<typename C>
class SpecularTransmission:public BxDF<SpecularTransmission<C>,typename C::interface_config>
{
	public:
		ADD_SAME_TYPEDEF(C,spectrum_t);
		ADD_SAME_TYPEDEF(C,scalar_t);
		ADD_SAME_TYPEDEF(C,vector_t);
		ADD_SAME_TYPEDEF(C,fresnel_dielectric_t);
		typedef BxDF<SpecularTransmission<C>,typename C::interface_config> parent_type;
		typedef SpecularTransmission<C> class_type;
		MA_DECLARE_POOL_NEW_DELETE_MT(class_type)
	public:
		SpecularTransmission(const spectrum_t& t,scalar_t ei,scalar_t et)
			:parent_type(BxDFType(BSDF_TRANSMISSION | BSDF_SPECULAR)),
			fresnel_(ei,et),t_(t),etai_(ei),etat_(et){}
		spectrum_t fImpl(const vector_t&,const vector_t&)const
		{ return spectrum_t(0);}
		//override
		spectrum_t sample_f(const vector_t &wo,vector_t &wi,
				scalar_t u1,scalar_t u2,scalar_t& pdf)const;
		scalar_t pdfImpl(const vector_t& wo,const vector_t& wi)const{return scalar_t(0);}
	
	public:
		fresnel_dielectric_t fresnel_;
		spectrum_t t_;
		scalar_t etai_,etat_;
};
template<typename R,typename C>
R FrDiel(R cosi,R cost,const C& etai,const C& etat)
{
	C Rparl = ((etat*cosi)-(etai * cost)) * reciprocal((etat * cosi)+(etai * cost));
	C Rperp = ((etai * cosi) - (etat * cost)) * 
		reciprocal((etai * cosi) + (etat * cost));
	return (Rparl*Rparl + Rperp * Rperp)* reciprocal(R(2.f));
}
template<typename RealType,typename ColorType>
ColorType FrCond(RealType cosi,
		const ColorType & eta,
		const ColorType &k)
{
	ColorType tmp = (eta*eta + k*k)*cosi*cosi;
	ColorType Rparl2 = (tmp -(RealType(2) * eta * cosi)+1) *
		reciprocal(tmp + (RealType(2) * eta * cosi) + 1);
	ColorType tmp_f = eta * eta + k * k;
	ColorType Rperp2 =
		(tmp_f - (RealType(2) * eta *cosi) + cosi*cosi) *
		reciprocal(tmp_f + (RealType(2)*eta*cosi) + cosi*cosi);
	return (Rparl2 + Rperp2) * reciprocal(RealType(2));
}

template<typename ColorType>
ColorType FresnelApproxEta(const ColorType& Fr)
{
	ColorType reflectance = Fr.clamp(0.f,0.9999f);
	return (ColorType(1) + reflectance.sqrt()) *
		reciprocal(ColorType(1) - reflectance.sqrt());
}
template<typename ColorType>
ColorType FresnelApproxK(const ColorType& Fr)
{
	ColorType reflectance = Fr.clamp(0.f,0.9999f);
	return 2.f * (reflectance * reciprocal(ColorType(1)-reflectance)).sqrt();
}

template<typename C>
typename C::spectrum_t 
FresnelConductor<C>::evaluateImpl(typename C::scalar_t cosi)const
{
	return FrCond(std::abs(cosi),eta_,k_);
}
template<typename C>
typename C::spectrum_t 
FresnelDielectric<C>::evaluateImpl(typename C::scalar_t cosi)const
{
	typedef typename C::scalar_t scalar_t;
	cosi = clamp(cosi,scalar_t(-1.f),scalar_t(1.f));
	bool entering = cosi > 0;
	scalar_t ei = eta_i_,et = eta_t_;
	if(!entering)
		std::swap(ei,et);
	scalar_t sint = ei * reciprocal(et) * std::sqrt(std::max<scalar_t>(0.,1.-cosi*cosi));
	if (sint > 1)
	{return 1;}//total internal reflection
	else
	{
		scalar_t cost = std::sqrt(std::max<scalar_t>(0.,1.-sint*sint));
		return FrDiel(std::abs(cosi),cost,ei,et);	
	}
}
template<typename C>
typename C::spectrum_t 
SpecularReflection<C>::sample_f(const typename C::vector_t& wo,typename C::vector_t& wi, typename C::scalar_t u1,typename C::scalar_t u2,typename C::scalar_t& pdf)const
{
	//perfect specular reflection
	wi = vector_t(-wo[0],-wo[1],wo[2]);
	pdf = 1;
//	spectrum_t  sp = 
	return fresnel::evaluate(fresnel_,CosTheta(wo)) * r_ *
		reciprocal(std::abs(CosTheta(wi)));
//	printf("relfection sample_f: %f",sp.y());
//	return sp;
}
template<typename C>
typename C::spectrum_t 
SpecularTransmission<C>::sample_f(const typename C::vector_t& wo,typename C::vector_t& wi, typename C::scalar_t u1,typename C::scalar_t u2,typename C::scalar_t& pdf)const
{
	bool entering = CosTheta(wo) > 0;
	scalar_t ei = etai_,et = etat_;
	if(!entering)std::swap(ei,et);
	scalar_t sini2 = SinTheta2(wo);
	scalar_t eta = ei * reciprocal(et);
	scalar_t sint2 = eta * eta * sini2;
	if( sint2 > 1 ) return 0;//total internal reflection
	scalar_t cost = std::sqrt(std::max<scalar_t>(0.,1. - sint2));
	if (entering) cost = - cost;
	scalar_t sintOverSini = eta;
	wi = vector_t(sintOverSini * -wo[0],
			sintOverSini * -wo[1],
			cost);
	pdf = 1;
	spectrum_t F = fresnel::evaluate(fresnel_,
			CosTheta(wo));

	return (et*et) * reciprocal(ei*ei) * (spectrum_t(1)-F) * t_
		* reciprocal(std::abs(CosTheta(wi)));
}

namespace microfacetdistribution{
	DECL_FUNC_NEST(scalar_t,d,1)
	DECL_FUNC(void,sample_f,5)
	DECL_FUNC_NEST(scalar_t,pdf,2)
}
BEGIN_CRTP_INTERFACE(MicrofacetDistribution)
	ADD_CRTP_INTERFACE_TYPEDEF(vector_t)
	ADD_CRTP_INTERFACE_TYPEDEF(scalar_t)
public:
CRTP_CONST_METHOD(scalar_t,d,1,( I_(const vector_t&,wh)))
CRTP_CONST_VOID_METHOD(sample_f,5,(
					    I_(const vector_t&,wo),
					    I_(vector_t&,wi),
					    I_(scalar_t,u1),
					    I_(scalar_t,u2),
					    I_(scalar_t&,pdf)))
CRTP_CONST_METHOD(scalar_t,pdf,2,( I_(const vector_t&,wo), I_(const vector_t&,wi)))
END_CRTP_INTERFACE

template <typename Conf>
class Microfacet: public BxDF<Microfacet<Conf>,typename Conf::interface_config>
{
	public:
		ADD_SAME_TYPEDEF(Conf,scalar_t);
		ADD_SAME_TYPEDEF(Conf,spectrum_t);
		ADD_SAME_TYPEDEF(Conf,microfacet_distribution_ptr);
		ADD_SAME_TYPEDEF(Conf,fresnel_ptr);
		ADD_SAME_TYPEDEF(Conf,vector_t);
		typedef Microfacet<Conf> class_type;
		MA_DECLARE_POOL_NEW_DELETE_MT(class_type);
	public:
		Microfacet(const spectrum_t &reflectance,fresnel_ptr f,
				microfacet_distribution_ptr d):
			r_(reflectance),distribution_(d),fresnel_(f){}
		spectrum_t fImpl(const vector_t &wo,const vector_t wi)const
		{
			scalar_t cos_thetaO = std::abs(CosTheta(wo));
			scalar_t cos_thetaI = std::abs(CosTheta(wi));
			vector_t wh = normalize(wi + wo);
			scalar_t cos_thetaH = dot(wi,wh);
			spectrum_t F = fresnel::evaluate(fresnel_,cos_thetaH);
			return r_ * microfacetdistribution::d(distribution_,wh) * G(wo,wi,wh) * F
				* reciprocal(4 * cos_thetaI * cos_thetaO);
		}

		scalar_t G(const vector_t &wo,const vector_t &wi,
				const vector_t &wh)const
		{
			scalar_t NdotWh = std::abs(CosTheta(wh));
			scalar_t NdotWo = std::abs(CosTheta(wo));
			scalar_t NdotWi = std::abs(CosTheta(wi));
			scalar_t WodotWh = abs_dot(wo,wh);
			scalar_t WodotWh_Inv = reciprocal(WodotWh);
			return std::min(scalar_t(1),std::min((2 * NdotWh * NdotWo * WodotWh_Inv),(2 * NdotWh * NdotWi * WodotWh_Inv)));
		}
		spectrum_t sample_f(const vector_t &wo,vector_t& wi,
				scalar_t u1,scalar_t u2,scalar_t& pdf)const
		{
			//
			microfacetdistribution::sample_f(distribution_,wo,ref(wi),u1,u2,ref(pdf));
			if (!SameHemisphere(wo,wi))return spectrum_t(0);
			return f(wo,wi);
		}
		scalar_t pdf(const vector_t& wo,const vector_t& wi)const
		{
			if(!SameHemisphere(wo,wi))return scalar_t(0);
			return microfacetdistribution::pdf(distribution_,wo,wi);
		}
		~Microfacet()
		{
			delete_ptr(distribution_);
			delete_ptr(fresnel_);
		}
	private:
		spectrum_t r_;
		microfacet_distribution_ptr distribution_;
		fresnel_ptr fresnel_;
};

template<typename Conf>
class Lambertian : public BxDF<Lambertian<Conf>,typename Conf::interface_config> {
	typedef Lambertian<Conf> class_type;
public:
	ADD_SAME_TYPEDEF(Conf,vector_t);
	ADD_SAME_TYPEDEF(Conf,scalar_t);
	ADD_SAME_TYPEDEF(Conf,spectrum_t);
	typedef BxDF<Lambertian<Conf>,typename Conf::interface_config> parent_type;
	// Lambertian Public Methods
	Lambertian(const spectrum_t &reflectance)
		: parent_type(BxDFType(BSDF_REFLECTION | BSDF_DIFFUSE)),
		R(reflectance), RoverPI(reflectance * INV_PI) {
	}
	spectrum_t fImpl(const vector_t &wo, const vector_t &wi) const
	{	return RoverPI;	}
	spectrum_t rhoImpl(const vector_t &, int, scalar_t *) const {
		return R;
	}
	spectrum_t rhoImpl(int, scalar_t *) const { return R; }
private:
	// Lambertian Private Data
	spectrum_t R, RoverPI;
	MA_DECLARE_POOL_NEW_DELETE_MT(class_type)
};
template<typename Conf>
class OrenNayar : public BxDF<OrenNayar<Conf>,typename Conf::interface_config> {
	ADD_SAME_TYPEDEF(Conf,vector_t);
	ADD_SAME_TYPEDEF(Conf,scalar_t);
	ADD_SAME_TYPEDEF(Conf,spectrum_t);
	typedef BxDF<OrenNayar<Conf>,typename Conf::interface_config> parent_type;
	typedef OrenNayar<Conf> class_type;
public:
	// OrenNayar Public Methods
	spectrum_t fImpl(const vector_t &wo, const vector_t &wi) const;
	OrenNayar(const spectrum_t &reflectance, scalar_t sig)
		: parent_type(BxDFType(BSDF_REFLECTION | BSDF_DIFFUSE)),
		R(reflectance) {
			scalar_t sigma = Radians(sig);
			scalar_t sigma2 = sigma*sigma;
			A = 1.f - (sigma2 / (2.f * (sigma2 + 0.33f)));
			B = 0.45f * sigma2 / (sigma2 + 0.09f);
	}
private:
	// OrenNayar Private Data
	spectrum_t R;
	scalar_t A, B;
	MA_DECLARE_POOL_NEW_DELETE_MT(class_type)
};
template<typename Conf>
typename Conf::spectrum_t OrenNayar<Conf>::fImpl(const vector_t &wo,
											 const vector_t &wi) const {
	 scalar_t sinthetai = SinTheta(wi);
	 scalar_t sinthetao = SinTheta(wo);
	 // Compute cosine term of Oren--Nayar model
	 scalar_t sinphii = SinPhi(wi), cosphii = CosPhi(wi);
	 scalar_t sinphio = SinPhi(wo), cosphio = CosPhi(wo);
	 scalar_t dcos = cosphii * cosphio + sinphii * sinphio;
	 scalar_t maxcos = std::max(0.f, dcos);
	 // Compute sine and tangent terms of Oren--Nayar model
	 scalar_t sinalpha, tanbeta;
	 if (std::abs(CosTheta(wi)) > std::abs(CosTheta(wo))) {
		 sinalpha = sinthetao;
		 tanbeta = sinthetai / std::abs(CosTheta(wi));
	 }
	 else {
		 sinalpha = sinthetai;
		 tanbeta = sinthetao / std::abs(CosTheta(wo));
	 }
	 return R * INV_PI *
		 (A + B * maxcos * sinalpha * tanbeta);
}

}
#endif
