#ifndef _MA_INCLUDED_REFLECTION_HPP_
#define _MA_INCLUDED_REFLECTION_HPP_

#include "CRTPInterfaceMacro.hpp"


#include "VectorType.hpp"

#include "ptr_var.hpp"
#include "pool.hpp"
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
		MAKE_VISITOR(rho,2)
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
		spectrum_t rho(BxDFType flags = BSDF_ALL)const;
		spectrum_t rho(const vector_t& wo,
				BxDFType flags = BSDF_ALL)const;
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
					vector_t wo = WorldToLocal(woW), wi = WorldToLocal(wiW);
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
typename Conf::spectrum_t BSDF<Conf>::rho(BxDFType flags) const {
	spectrum_t ret(0.);
	for (int i = 0; i < nBxDFs; ++i)
		if (bxdf::matchesFlags_ref<bool>(bxdfs[i],flags)/*bxdfs[i]->MatchesFlags(flags)*/)
			ret += bxdf::rho_ref<spectrum_t>(bxdfs[i]);//bxdfs[i]->rho();
	return ret;
}
template<typename Conf>
typename Conf::spectrum_t BSDF<Conf>::rho(const vector_t &wo, BxDFType flags) const {
	spectrum_t ret(0.);
	for (int i = 0; i < nBxDFs; ++i)
		if (bxdf::matchesFlags_ref<bool>(bxdfs[i],flags) /*bxdfs[i]->MatchesFlags(flags)*/)
			ret += bxdf::rho_ref<spectrum_t>(bxdfs[i],wo);//bxdfs[i]->rho(wo);
	return ret;
}
//////////////////////////////////////////////////////////////////////////

//more bxdf types
BEGIN_CRTP_INTERFACE(BxDF)
ADD_CRTP_INTERFACE_TYPEDEF(spectrum_t)
ADD_CRTP_INTERFACE_TYPEDEF(vector_t)
ADD_CRTP_INTERFACE_TYPEDEF(scalar_t);
public:
BxDF(BxDFType t):type_(t){}
bool matchesFlags(BxDFType flags)const{return (type_ & flags) == type_;}

CRTP_CONST_METHOD(spectrum_t,f,2,( I_(const vector_t& , wo), I_(const vector_t& ,wi)));
DECLARE_CONST_METHOD(spectrum_t,sample_f,5,( I_(const vector_t&,wo),
				   I_(vector_t&, wi),
				   I_(scalar_t,u1), I_(scalar_t,u2), I_(scalar_t&,pdf)));
DECLARE_CONST_METHOD(spectrum_t,rho,3,( I_(const vector_t&,wo),
				   I_(int , nSamples), I_(scalar_t&,samples)));
DECLARE_CONST_METHOD(spectrum_t,rho,2,( I_(int , nSamples), I_(scalar_t* ,samples)))

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
							ppdf = pdf(wo, wi);
							return f(wo, wi);
}
const float INV_PI = 0.318309886f;
template<typename D,typename Conf>
typename Conf::scalar_t BxDF<D,Conf>::pdf(const typename Conf::vector_t &wo, const typename Conf::vector_t &wi) const {
	return
		SameHemisphere(wo, wi) ? fabsf(wi.z()) * INV_PI : 0.f;
}

template<typename D,typename Conf>
typename Conf::spectrum_t BxDF<D,Conf>::rho(const typename Conf::vector_t &w, int nSamples,
				   scalar_t &samples) const {
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
							   sample_f(w, wi, samples[2*i], samples[2*i+1], pdf);
						   if (pdf > 0.) r += f * fabsf(wi.z) / pdf;
					   }
					   return r / (M_PI * nSamples);
}
template<typename D,typename Conf>
typename Conf::spectrum_t BxDF<D,Conf>::rho(int nSamples, scalar_t *samples) const {
	if (!samples) {
		samples =
			(scalar_t *)alloca(4 * nSamples * sizeof(scalar_t));
		LatinHypercube(samples, nSamples, 4);
	}
	spectrum_t r = 0.;
	for (int i = 0; i < nSamples; ++i) {
		// Estimate one term of $\rho_{hh}$
		vector_t wo, wi;
		wo = UniformSampleHemisphere(samples[4*i], samples[4*i+1]);
		const float INV_TWOPI = INV_PI/2;
		scalar_t pdf_o = INV_TWOPI, pdf_i = 0.f;
		spectrum_t f =
			sample_f(wo,  wi, samples[4*i+2], samples[4*i+3],
			 pdf_i);
		if (pdf_i > 0.)
			r += f * fabsf(wi.z * wo.z) / (pdf_o * pdf_i);
	}
	return r / (M_PI*nSamples);
}

//////////////////////////////////////////////////////////////////////////
BEGIN_CRTP_INTERFACE(Fresnel)
ADD_CRTP_INTERFACE_TYPEDEF(spectrum_t)
ADD_CRTP_INTERFACE_TYPEDEF(scalar_t)
public:
	CRTP_CONST_METHOD(spectrum_t,evaluate,1,( I_(scalar_t,cosi)))
END_CRTP_INTERFACE

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


template<typename Conf>
class Lambertian : public BxDF<Lambertian<Conf>,Conf> {
	typedef Lambertian<Conf> class_type;
public:
	ADD_SAME_TYPEDEF(Conf,vector_t);
	ADD_SAME_TYPEDEF(Conf,scalar_t);
	ADD_SAME_TYPEDEF(Conf,spectrum_t);
	typedef BxDF<Lambertian<Conf>,Conf> parent_type;
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
class OrenNayar : public BxDF<OrenNayar<Conf>,Conf> {
	ADD_SAME_TYPEDEF(Conf,vector_t);
	ADD_SAME_TYPEDEF(Conf,scalar_t);
	ADD_SAME_TYPEDEF(Conf,spectrum_t);
	typedef BxDF<OrenNayar<Conf>,Conf> parent_type;
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