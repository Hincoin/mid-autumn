#ifndef _FRESNEL_H_
#define _FRESNEL_H_

#include "config.h"
#include "spectrum.h"
#include "geometry.h"


INLINE void fr_diel(float cosi,float cost,spectrum_t etai,spectrum_t etat,spectrum_t *ret)
{
	//spectrum_t Rparl = ((etat*cosi)-(etai * cost))/((etat * cosi)+(etai * cosi));
	//spectrum_t Rperp = ((etai * cosi)-(etat * cost))/((etai * cosi)+(etat * cost));
	//return (Rparl * Rparl + Rperp * Rperp) / 2.f;
	spectrum_t Rparl,Rperp;
	spectrum_t tmp0,tmp1,tmp_u,tmp_d;

	vsmul(tmp0,cosi,etat);
	vsmul(tmp1,cost,etai);
	vsub(tmp_u,tmp0,tmp1);
	vadd(tmp_d,tmp0,tmp1);
	vdiv(Rparl,tmp_u,tmp_d);

	vsmul(tmp0,cosi,etai);
	vsmul(tmp1,cost,etat);
	vsub(tmp_u,tmp0,tmp1);
	vadd(tmp_d,tmp0,tmp1);
	vdiv(Rperp,tmp_u,tmp_d);


	vmul(tmp0,Rparl,Rparl);
	vmul(tmp1,Rperp,Rperp);
	vadd(*ret,tmp0,tmp1);
	vsmul(*ret,0.5f,*ret);
}

INLINE void fr_cond(float cosi, spectrum_t eta,spectrum_t k,spectrum_t* ret)
{
	//spectrum_t tmp = (eta * eta + k * k) * cosi * cosi;
	//spectrum_t Rparl2 = (tmp - (2.f * eta * cosi) +1) /
	//					(tmp + (2.f * eta * cosi) +1);
	//spectrum_t tmp_f = eta * eta + k * k;
	//spectrum_t Rperp2 = (tmp_f - (2.f * eta * cosi) + cosi * cosi)/
	//	                (tmp_f + (2.f * eta * cosi) + cosi * cosi);
	//return (Rparl2 + Rperp2)/2.f;
	spectrum_t tmp0,tmp1,tmp_u,tmp_d,tmp;
	spectrum_t Rparl2,Rperp2;
	float cosi2 = cosi*cosi;
	float cosi_2 = 2 * cosi;
	vmul(tmp0,eta,eta);
	vmul(tmp1,k,k);
	vadd(tmp,tmp0,tmp1);
	vsmul(tmp0,cosi2,tmp);
	vsmul(tmp1,cosi_2,eta);
	vsub(tmp,tmp0,tmp1);
	vsadd(tmp_u,1.f,tmp);
	vadd(tmp,tmp0,tmp1);
	vsadd(tmp_d,1.f,tmp);
	vdiv(Rparl2,tmp_u,tmp_d);

	vmul(tmp0,eta,eta);
	vmul(tmp1,k,k);
	vadd(tmp,tmp0,tmp1);
	
	vsmul(tmp0,cosi_2,eta);
	vsub(tmp1,tmp,tmp0);
	vsadd(tmp_u,cosi2,tmp1);

	vadd(tmp1,tmp,tmp0);
	vsadd(tmp_d,cosi2,tmp1);

	vdiv(Rperp2,tmp_u,tmp_d);
	vadd(*ret,Rparl2,Rperp2);
	vsmul(*ret,0.5f,*ret);
}

INLINE void fresnel_approx_eta(spectrum_t fr,spectrum_t *eta)
{
	spectrum_t reflectance;
	reflectance.x = clamp(fr.x,0.f,0.999f);
	reflectance.y = clamp(fr.y,0.f,0.999f);
	reflectance.z = clamp(fr.z,0.f,0.999f);
	vinit(reflectance,sqrt(reflectance.x),sqrt(reflectance.y),sqrt(reflectance.z));
	spectrum_t tmp_u,tmp_d;
	vinit(tmp_u,1.f,1.f,1.f);
	vadd(tmp_u,tmp_u,reflectance);
	vinit(tmp_d,1.f,1.f,1.f);
	vsub(tmp_d,tmp_d,reflectance);

	vdiv(*eta,tmp_u,tmp_d);
}
INLINE void fresnel_approx_k(spectrum_t fr,spectrum_t *k)
{
	//spectrum_t reflectance = fr.clamp(0.f,0.999f);
	//return 2.f * (reflectance / (spectrum_t(1.f) - reflectance)).sqrt();
	spectrum_t reflectance,reflectance_c,tmp;
	reflectance.x = clamp(fr.x,0.f,0.999f);
	reflectance.y = clamp(fr.y,0.f,0.999f);
	reflectance.z = clamp(fr.z,0.f,0.999f);
	vinit(reflectance_c, 1-reflectance.x,1.f-reflectance.y,1.f-reflectance.z);
	vdiv(tmp,reflectance,reflectance_c);
	vinit(*k, sqrt(tmp.x),sqrt(tmp.y),sqrt(tmp.z));
	vsmul(*k,2.f,*k);
}

typedef enum {FR_CONDUCTOR=0,FR_DIELECTRIC,FR_NOOP}FresnelType;

typedef struct
{
	FresnelType t;
	union{
		struct{spectrum_t eta,k;};
		struct{float eta_i,eta_t;};
	};
}
fresnel_t;

#define fresnel_conductor_init(fr,ee,kk) \
{\
	fr.t = FR_CONDUCTOR;vassign(fr.eta,ee);vassign(fr.k,kk);\
}\


#define fresnel_dielectric_init(fr,ei,et) \
{\
	fr.t = FR_DIELECTRIC;fr.eta_i = ei;fr.eta_t = et;\
}\

#define fresnel_noop_init(fr)\
{\
	fr.t = FR_NOOP;\
}\


INLINE void fresnel_evaluate(fresnel_t f,float cosi,spectrum_t *ret)
{
	switch(f.t)
	{
	case FR_CONDUCTOR:
		{
			fr_cond(cosi,f.eta,f.k,ret);
		}
		break;
	case FR_DIELECTRIC:
		{
			cosi = clamp(cosi,-1.f,1.f);
			bool enter = cosi > 0.f;
			float ei = f.eta_i;
			float et = f.eta_t;
			if(!enter)
			{
				float t = ei;
				ei = et;
				et = t;
			}
			float sint = ei/et * sqrt(max(0.f,(1.f-cosi*cosi)));
			if(sint > 1.f)
			{
				//total internal reflection
				vinit(*ret,1.f,1.f,1.f);
			}
			else
			{
				float cost = sqrt(max(0.f,(1.f-sint*sint)));
				spectrum_t sei,set;
				vinit(sei,ei,ei,ei);
				vinit(set,et,et,et);
				fr_diel(fabs(cosi),cost,sei,set,ret);
			}
		}break;
	case FR_NOOP:
		vinit(*ret,1.f,1.f,1.f);
		break;
	default:
		break;
	}
}

#endif