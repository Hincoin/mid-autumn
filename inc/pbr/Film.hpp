#ifndef _MA_INCLUDED_FILM_HPP_
#define _MA_INCLUDED_FILM_HPP_

#include "CRTPInterfaceMacro.hpp"

namespace ma{
	namespace film
	{
		DECL_FUNC(void,addSample,4)
		DECL_FUNC(void,writeImage,0)
		DECL_FUNC(void,resetCropWindow,4)
		DECL_FUNC(void,getSampleExtent,4)
		DECL_FUNC(int,xResolution,0)
		DECL_FUNC(int,yResolution,0)
	}
	BEGIN_CRTP_INTERFACE(Film)
	ADD_CRTP_INTERFACE_TYPEDEF(sample_t);
	ADD_CRTP_INTERFACE_TYPEDEF(ray_t);
	ADD_CRTP_INTERFACE_TYPEDEF(spectrum_t);
	ADD_CRTP_INTERFACE_TYPEDEF(scalar_t);
public:
	Film(int xres,int yres):x_resolution(xres),y_resolution(yres){}
	CRTP_METHOD(void,addSample,4,( I_(const sample_t&,s), I_(const ray_t&,r), I_(const spectrum_t&,l), I_(scalar_t,alpha)));
	CRTP_METHOD(void,writeImage,0,());
	CRTP_METHOD(void,resetCropWindow,4,(I_(float,xmin),I_(float,xmax),I_(float,ymin),I_(float,ymax)))
	CRTP_CONST_VOID_METHOD(getSampleExtent,4,( I_(int&,xs), I_(int&,xe), I_(int&,ys), I_(int&,ye)));
	int xResolution()const{return x_resolution;}
	int yResolution()const{return y_resolution;}
protected:
	const int x_resolution,y_resolution;
	END_CRTP_INTERFACE
	//for runtime extension
	template<typename Conf>
	class IFilm:public Film<IFilm<Conf>,typename Conf::interface_config>
	{
		ADD_SAME_TYPEDEF(Conf,sample_t);
		ADD_SAME_TYPEDEF(Conf,ray_t);
		ADD_SAME_TYPEDEF(Conf,spectrum_t);
		ADD_SAME_TYPEDEF(Conf,scalar_t);

		public:
		typedef Film<IFilm<Conf>,typename Conf::interface_config> parent_type;
		IFilm(int xr,int yr):parent_type(xr,yr){}
		virtual void addSampleImpl(const sample_t&,const ray_t&,const spectrum_t& ,scalar_t) = 0;
		virtual void writeImageImpl() = 0;
		virtual void resetCropWindowImpl(scalar_t ,scalar_t,scalar_t,scalar_t) = 0;
		virtual void getSampleExtentImpl(int&,int&,int&,int&)const = 0;
		virtual ~IFilm(){}
	};
}
#include "ParamSet.hpp"
namespace ma{
	namespace details{
	template<typename F>
	struct film_creator;	
	}
	template<typename F,typename FP>
		F* create_film(const ParamSet &param,FP filt)
		{return details::film_creator<F>()(param,filt);}
}

#endif
