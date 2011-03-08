#ifndef _REFLECTION_H_
#define _REFLECTION_H_

#include "fresnel.h"
#include "mc.h"


// BSDF Inline Functions
INLINE float cos_theta(vector3f_t w) { return w.z; }
INLINE float sin_theta(vector3f_t w) {
	return sqrt(max(0.f, (1.f - w.z*w.z)));
}
INLINE float sin_theta2(vector3f_t w) {
	return 1.f - cos_theta(w)*cos_theta(w);
}
INLINE float cos_phi(vector3f_t w) {
	return w.x / sin_theta(w);
}
INLINE float sin_phi(vector3f_t w) {
	return w.y / sin_theta(w);
}
INLINE bool same_hemisphere(vector3f_t w,
						   vector3f_t wp) {
							   return w.z * wp.z > 0.f;
}
// BSDF Declarations
typedef enum {
	BSDF_REFLECTION   = 1<<0,
	BSDF_TRANSMISSION = 1<<1,
	BSDF_DIFFUSE      = 1<<2,
	BSDF_GLOSSY       = 1<<3,
	BSDF_SPECULAR     = 1<<4,
	BSDF_ALL_TYPES        = BSDF_DIFFUSE |
	BSDF_GLOSSY |
	BSDF_SPECULAR,
	BSDF_ALL_REFLECTION   = BSDF_REFLECTION |
	BSDF_ALL_TYPES,
	BSDF_ALL_TRANSMISSION = BSDF_TRANSMISSION |
	BSDF_ALL_TYPES,
	BSDF_ALL              = BSDF_ALL_REFLECTION |
	BSDF_ALL_TRANSMISSION
}BxDFType ;

typedef enum{
	BRDF_SPECULAR_REFLECTION=0,
	BTDF_SPECULAR_TRANSMISSION,
	BRDF_LAMBERTIAN,
	BRDF_OREN_NAYAR,
	BRDF_MICROFACET,
	BRDF_LAFORTUNE,
	BRDF_FRESNEL_BLEND
}
BxDFModelType;

typedef struct  
{
	BxDFType type;
	BxDFModelType bxdf_model;
	union{
		struct  //specular reflection
		{
			spectrum_t r_specular_refl;
			fresnel_t fresnel_refl;
		};
		struct //specular transmission
		{
			spectrum_t t;
			float etai,etat;
			fresnel_t fresnel_trans;
		};
		struct  //lambertian
		{
			spectrum_t r_lambert,r_over_pi;
		};
		struct //oren_nayar
		{
			spectrum_t r_oren_nayar;
			float a,b;
		};
		//todo ...
	};
}bxdf_t;

#define bxdf_init_specular_reflection(_x,_r,_fr)\
{\
	_x.type =( BxDFType ) (BSDF_REFLECTION | BSDF_SPECULAR);\
	_x.bxdf_model = BRDF_SPECULAR_REFLECTION;\
	_x.r_specular_refl = _r;\
	_x.fresnel_refl = _fr;\
}\

#define bxdf_init_specular_transmission(_x,_t,_ei,_et,_f)\
{\
	_x.type =(BxDFType) (BSDF_SPECULAR | BSDF_TRANSMISSION);\
	_x.bxdf_model = BTDF_SPECULAR_TRANSMISSION;\
	_x.t = _t ;\
	_x.etai = _ei;\
	_x.etat = _et;\
	_x.fresnel_trans = _f;\
}\

#define bxdf_init_lambertian(_x,_r)\
{\
	_x.type =(BxDFType) (BSDF_REFLECTION | BSDF_DIFFUSE);\
	_x.bxdf_model = BRDF_LAMBERTIAN;\
	_x.r_lambert = _r;\
	vsmul(_x.r_over_pi,INV_PI, _r);\
}\


#define bxdf_init_oren_nayar(_x,_r,_sig)\
{\
	_x.type = (BxDFType)(BSDF_REFLECTION | BSDF_DIFFUSE);\
	_x.bxdf_model = BRDF_OREN_NAYAR;\
	float _sigma = radians(_sig);\
	float _sigma2 = _sigma*_sigma;\
	_x.a = 1.f - (_sigma2 / (2.f * (_sigma2 + 0.33f)));\
	_x.b = 0.45f * _sigma2 / (_sigma2 + 0.09f);\
	_x.r_oren_nayar = _r;\
}\


//bxdf interface
INLINE void bxdf_f(bxdf_t *self,vector3f_t wo,vector3f_t wi,spectrum_t *f)
{
	//todo
	switch(self->bxdf_model)
	{
	case BRDF_LAMBERTIAN:
		{ *f = self->r_over_pi;}
		break;
	case BRDF_OREN_NAYAR:
		{
			//
			float sinthetai = sin_theta(wi);
			float sinthetao = sin_theta(wo);
			// Compute cosine term of Oren-Nayar model
			float maxcos = 0.f;
			if (sinthetai > 1e-4f && sinthetao > 1e-4f) {
				float sinphii = sin_phi(wi), cosphii = cos_phi(wi);
				float sinphio = sin_phi(wo), cosphio = cos_phi(wo);
				float dcos = cosphii * cosphio + sinphii * sinphio;
				maxcos = max(0.f, dcos);
			}

			// Compute sine and tangent terms of Oren-Nayar model
			float sinalpha, tanbeta;
			if (fabs(cos_theta(wi)) > fabs(cos_theta(wo))) {
				sinalpha = sinthetao;
				tanbeta = sinthetai / fabs(cos_theta(wi));
			}
			else {
				sinalpha = sinthetai;
				tanbeta = sinthetao / fabs(cos_theta(wo));
			}
			vsmul(*f,INV_PI * (self->a + self->b * maxcos * sinalpha * tanbeta),self->r_oren_nayar);
		}
		break;
	case BRDF_SPECULAR_REFLECTION:
	case BTDF_SPECULAR_TRANSMISSION:
		vclr(*f);
		break;
	default:
		break;
	}
}


INLINE float bxdf_pdf(bxdf_t *self, vector3f_t wi,vector3f_t wo)
{
	switch(self->bxdf_model)
	{
	case BRDF_SPECULAR_REFLECTION:
	case BTDF_SPECULAR_TRANSMISSION:
		return 0.f;
		break;
	case BRDF_OREN_NAYAR:
	case BRDF_LAMBERTIAN:
	default:
		return
			same_hemisphere(wo, wi) ? fabs(wi.z) * INV_PI : 0.f;
	}
	return 0.f;
}

INLINE void bxdf_sample_f(bxdf_t *self, vector3f_t wo,vector3f_t* wi,
						  float u1,float u2,float *pdf,spectrum_t *f)
{
	//todo
	switch(self->bxdf_model)
	{
	case BRDF_SPECULAR_REFLECTION:
		{
			vinit(*wi,-wo.x,-wo.y,wo.z);
			*pdf = 1.f;
			fresnel_evaluate(self->fresnel_refl,cos_theta(wo),f);
			vmul(*f,*f,self->r_specular_refl);
			vsmul(*f,1.f/fabs(cos_theta(*wi)),*f);
		}
		break;
	case BTDF_SPECULAR_TRANSMISSION:
		{
			bool entering = cos_theta(wo) > 0.f;
			float ei = self->etai;
			float et = self->etat;
			if(!entering)
			{
				float t = ei;
				ei = et;
				et = t;
			}

			//compute transmitted ray dir
			float sini2 = sin_theta2(wo);
			float eta = ei/et;
			float sint2 = eta * eta * sini2;

			//handle total internal reflection for transmission

			if(sint2 >= 1.f) {vclr(*f);return;}
			float cost = sqrt(max(0.f,(1.f-sint2)));
			if(entering) cost = -cost;
			float sint_over_sini = eta;
			vinit(*wi,sint_over_sini* -wo.x,sint_over_sini* -wo.y,cost);
			*pdf = 1.f;
			spectrum_t ft;
			fresnel_evaluate(self->fresnel_trans,cos_theta(wo),&ft);

			vinit(ft,1.f-ft.x,1.f-ft.y,1.f-ft.z);
			vmul(ft,ft,self->t);
			vsmul(ft,1.f/fabs(cos_theta(*wi)),ft);
			*f = ft;
		}
		break;
	case BRDF_OREN_NAYAR:
	case BRDF_LAMBERTIAN:
	default:
		// Cosine-sample the hemisphere, flipping the direction if necessary
		cosine_sample_hemisphere(u1, u2,wi);
		if (wo.z < 0.) wi->z *= -1.f;
		*pdf = bxdf_pdf(self,wo, *wi);
		bxdf_f(self,wo, *wi,f);
	}
}
INLINE bool bxdf_matches(bxdf_t* bxdf,int flags)
{
	return (bxdf->type & flags) == bxdf->type;
}
//rho todo


typedef struct  
{
	#define MAX_BXDF_NUM 8
	bxdf_t bxdfs[MAX_BXDF_NUM];
	normal3f_t nn,ng;
	vector3f_t sn,tn;
	//const
	differential_geometry_t dg_shading;
	float eta;

	unsigned char n_bxdfs;	
}bsdf_t;

INLINE void bsdf_init(bsdf_t* bsdf,const differential_geometry_t* dg,
					  const normal3f_t* ngeom,float e)
{
	//
	bsdf->dg_shading = *dg;
	bsdf->nn = dg->nn;
	bsdf->eta = e;
	bsdf->ng = *ngeom;
	bsdf->sn = dg->dpdu;
	vnorm(bsdf->sn);
	vxcross(bsdf->tn,bsdf->nn,bsdf->sn);
	bsdf->n_bxdfs = 0;
}
//bsdf interfaces
INLINE void bsdf_add(bsdf_t* bsdf,bxdf_t* bxdf)
{
	bsdf->bxdfs[bsdf->n_bxdfs] = *bxdf;
	bsdf->n_bxdfs++;
}
INLINE void bsdf_world_to_local(const bsdf_t *bsdf,const vector3f_t *v,vector3f_t *vo)
{
	vo->x = vdot(*v,bsdf->sn);
	vo->y = vdot(*v,bsdf->tn);
	vo->z = vdot(*v,bsdf->nn);
}
INLINE void bsdf_local_to_world(const bsdf_t *bsdf,const vector3f_t *v,vector3f_t* vo) 
{
	vinit(*vo, (bsdf->sn.x * v->x + bsdf->tn.x * v->y + bsdf->nn.x * v->z),
		(bsdf->sn.y * v->x + bsdf->tn.y * v->y + bsdf->nn.y * v->z),
		(bsdf->sn.z * v->x + bsdf->tn.z * v->y + bsdf->nn.z * v->z));
}
INLINE void bsdf_f(bsdf_t* bsdf,
				   const vector3f_t* woW,const vector3f_t* wiW,BxDFType flags,
				   spectrum_t* f)
{
	//todo
	vector3f_t wi,wo;
	bsdf_world_to_local(bsdf,wiW,&wi);
	bsdf_world_to_local(bsdf,woW,&wo);
	if (vdot(*wiW, bsdf->ng) * vdot(*woW, bsdf->ng) > 0)
		// ignore BTDFs
		flags = (BxDFType)(flags & ~BSDF_TRANSMISSION);
	else
		// ignore BRDFs
		flags = (BxDFType)(flags & ~BSDF_REFLECTION);
	vclr(*f);
	spectrum_t tf;
	for (int i = 0; i < bsdf->n_bxdfs; ++i)
		if (bxdf_matches(&bsdf->bxdfs[i], flags))
		{
			bxdf_f(&bsdf->bxdfs[i],wo,wi,&tf);
			vadd(*f,*f,tf);
		}
}

INLINE float bsdf_pdf(bsdf_t* bsdf,const vector3f_t *woW,const vector3f_t* wiW,BxDFType flags)
{
	//todo
	if (bsdf->n_bxdfs == 0) return 0.;
	vector3f_t wo,wi;
	bsdf_world_to_local(bsdf,woW,&wo);
	bsdf_world_to_local(bsdf,wiW,&wi);
	float pdf = 0.f;
	int matchingComps = 0;
	for (int i = 0; i < bsdf->n_bxdfs; ++i)
		if (bxdf_matches(&bsdf->bxdfs[i], flags)) {
			++matchingComps;
			pdf += bxdf_pdf(&bsdf->bxdfs[i],wo,wi);
		}
	return matchingComps > 0 ? pdf / matchingComps : 0.f;
}

INLINE int bsdf_num_components(bsdf_t* bsdf,int flags)
{
	int matching_comps = 0;
	for (int i = 0;i < bsdf->n_bxdfs; ++i)
	{
		if ((flags & bsdf->bxdfs[i].type) == bsdf->bxdfs[i].type)
		{
			matching_comps ++;
		}
	}
	return matching_comps;
}
INLINE void bsdf_sample_f(bsdf_t* bsdf,const vector3f_t *woW,vector3f_t *wiW,
						  float u1,float u2,float u3,float *pdf,BxDFType flags,
						  BxDFType *sampled_type, spectrum_t *f)
{//todo
	// Choose which _BxDF_ to sample
	int matching_comps = bsdf_num_components(bsdf,flags);
	if (matching_comps == 0) {
		*pdf = 0.f;
		vclr(*f);
	}
	int which = min((u3 * matching_comps),
		matching_comps-1);
	//BxDF *bxdf = NULL;
	int bxdf_idx = -1;
	int count = which;
	for (int i = 0; i < bsdf->n_bxdfs; ++i)
		if (bxdf_matches(&bsdf->bxdfs[i], flags))
			if (count-- == 0) {
				bxdf_idx = i;
				break;
			}
			// Sample chosen _BxDF_
			vector3f_t wi;
			vector3f_t wo ;
			bsdf_world_to_local(bsdf,woW,&wo);
			*pdf = 0.f;
			bxdf_sample_f(&bsdf->bxdfs[bxdf_idx],wo,&wi,u1,u2,pdf,f);
			if (*pdf == 0.f){ vclr(*f);return ;}
			*sampled_type = bsdf->bxdfs[bxdf_idx].type;
			bsdf_local_to_world(bsdf,&wi,wiW);
			// Compute overall PDF with all matching _BxDF_s
			if (!(bsdf->bxdfs[bxdf_idx].type & BSDF_SPECULAR) && matching_comps> 1) {
				for (int i = 0; i < bsdf->n_bxdfs; ++i) {
					if (i != bxdf_idx  &&
						(bxdf_matches(&bsdf->bxdfs[i], (flags))))
						*pdf += bxdf_pdf(&bsdf->bxdfs[i],wo,wi);// bxdfs[i]->Pdf(wo, wi);
				}
			}
			if (matching_comps > 1) *pdf /= matching_comps;
			// Compute value of BSDF for sampled direction
			if (!(bsdf->bxdfs[bxdf_idx].type & BSDF_SPECULAR)) {
				vclr(*f);
				if (vdot(*wiW, bsdf->ng) * vdot(*woW, bsdf->ng) > 0)
					// ignore BTDFs
					flags =(BxDFType) (flags & ~BSDF_TRANSMISSION);
				else
					// ignore BRDFs
					flags = (BxDFType)(flags & ~BSDF_REFLECTION);
				spectrum_t tf;
				for (int i = 0; i < bsdf->n_bxdfs; ++i)
					if (bxdf_matches(&bsdf->bxdfs[i], flags))
					{
						bxdf_f(&bsdf->bxdfs[i],wo,wi,&tf);
						vadd(*f,*f,tf);
					}
			}
}

INLINE void bxdf_rho_hh(bxdf_t* bxdf,int n_samples,
					 float *samples,spectrum_t *c)
{
	
	switch(bxdf->bxdf_model)
	{
	case BRDF_LAMBERTIAN:
		vassign(*c,bxdf->r_lambert);
		return ; 
	default:
		{
			vclr(*c);
			spectrum_t f;
			vector3f_t wo,wi;
			for(int i =0;i < n_samples; ++i)
			{
				uniform_sample_hemisphere(samples[4*i],samples[4*i+1],&wo);
				float pdf_o = INV_PI,pdf_i = 0.f;
				bxdf_sample_f(bxdf,
					wo,&wi,samples[4*i+2],samples[4*i+3],
					&pdf_i,&f);
				if(pdf_i > 0)
				{
					vsmul(f,fabs(wi.z*wo.z)/(pdf_o*pdf_i),f);
					vadd(*c,*c,f);
				}
			}
			vsmul(*c,1.f/(FLOAT_PI*n_samples),*c);
			return;
		}
		break;
		
	}
}
INLINE void bxdf_rho_hd(bxdf_t* bxdf,const vector3f_t *w,int n_samples,
					 float *samples,spectrum_t *c)
{
	switch(bxdf->bxdf_model)
	{
	case BRDF_LAMBERTIAN:
		vassign(*c,bxdf->r_lambert);
		return ; 
	default:
		{
			vector3f_t wi;
			spectrum_t f;
			vclr(*c);
			for(int i = 0;i < n_samples; ++i)
			{
				float pdf = 0.f;
				bxdf_sample_f(bxdf,
					*w,&wi,samples[2*i],samples[2*i+1],
					&pdf,&f);
				if (pdf > 0.f)
				{
					vsmul(f,fabs(wi.z)/pdf,f);
					vadd(*c,*c,f);
				}
			}
			vsmul(*c,1.f/n_samples,*c);
			return;
		}
		break;
		
	}
}
#include "sampling.h"
INLINE void bsdf_rho_hh(bsdf_t* bsdf,Seed* seed,int flags,spectrum_t *rho)
{
	//
	vclr(*rho);
	spectrum_t tmp;
	float samples[64];
	int nsamples = 16;
	for (int i = 0; i < bsdf->n_bxdfs; ++i)
		if (bxdf_matches(&bsdf->bxdfs[i], (flags)))
		{	
			latin_hypercube(samples,nsamples,4,seed);
			bxdf_rho_hh(&bsdf->bxdfs[i],nsamples,samples,&tmp);
			vadd(*rho,*rho,tmp);
		}
}

INLINE void bsdf_rho_hd(bsdf_t* bsdf,const vector3f_t* w,Seed* seed,int flags,spectrum_t *rho)
{
	//
	vclr(*rho);
	spectrum_t tmp;
	float samples[32];
	int nsamples = 16;
	for (int i = 0; i < bsdf->n_bxdfs; ++i)
		if (bxdf_matches(&bsdf->bxdfs[i] ,(flags)))
		{	
			latin_hypercube(samples,nsamples,2,seed);
			bxdf_rho_hd(&bsdf->bxdfs[i],w,nsamples,samples,&tmp);
			vadd(*rho,*rho,tmp);
		}
}
INLINE void compile_test()
{
}
#endif
