#ifndef _MA_INCLUDED_NETIMAGEFILM_HPP_
#define _MA_INCLUDED_NETIMAGEFILM_HPP_ 

#include "ImageFilm.hpp"
#include "dispatcher.hpp"
#include "net.hpp"

#include "DefaultConfigurations.hpp"
namespace ma{
	template<typename Conf=image_film_config<basic_config_t> >
	class NetImageFilm:public ImageFilm<Conf>
	{
	public:
		ADD_SAME_TYPEDEF(Conf,sample_t)
		ADD_SAME_TYPEDEF(Conf,spectrum_t)
		ADD_SAME_TYPEDEF(Conf,ray_t)
		ADD_SAME_TYPEDEF(Conf,scalar_t)
		ADD_SAME_TYPEDEF(Conf,filter_ptr);
	
		typedef ImageFilm<Conf> parent_type;
		NetImageFilm(int xr,int yr,
				filter_ptr filt, const scalar_t crop[4],
				const std::string &file_name, bool premult,
			int wf,rpc::conn_t c):parent_type(xr,yr,filt,crop,file_name,premult,wf),connection_(c){}

	void addSampleImpl(const sample_t &sample, const ray_t &ray,const spectrum_t& l,scalar_t alpha);
	void writeImageImpl();
		
	private:
		rpc::conn_t connection_;
	};

}

#include "TypeMap.hpp"
#include "render_client.hpp"
namespace ma{
	MAKE_TYPE_STR_MAP(1,NetImageFilm,netimage)
namespace details{
	template<typename Conf>
	struct film_creator<NetImageFilm<Conf> >
	{
		template<typename FP>
		NetImageFilm<Conf>*
		operator()(const ParamSet& param,FP filt)const
		{
			typedef NetImageFilm<Conf> film_t;
			//create film
			 std::string filename =  param.as<std::string>("filename","pbrt.tga");
			 bool premultiplyAlpha =  param.as<bool>("premultiplyalpha",true);
#ifdef NDEBUG
	 int xres = 800;
	 int yres = 600;
#else
	 int xres = 80;
	 int yres = 60;
#endif

		 xres = param.as<int>("xresolution",xres);
	 yres = param.as<int>("yresolution",yres);
	 float crop[4] = { 0, 1, 0, 1 };
	 std::vector<float> cropwindow ;
	cropwindow =  param.as<std::vector<float> >("cropwindow",cropwindow);	
	 if(cropwindow.size() == 4)
	 {
		 crop[0] = clamp(std::min(cropwindow[0],cropwindow[1]),0.f,1.f);
	  	 crop[1] = clamp(std::max(cropwindow[0],cropwindow[1]),0.f,1.f);
   		crop[2] = clamp(std::min(cropwindow[2],cropwindow[3]),0.f,1.f);
		crop[3] = clamp(std::max(cropwindow[2],cropwindow[3]),0.f,1.f);   
	 }

	 int write_frequency = param.as<int>("writefrequency",-1);
	 return new film_t(xres,yres,filt,crop,filename,premultiplyAlpha,write_frequency,render_client::get_client().get_connection());		
		}	
	};	
}
}

#include "pbr_rpc.hpp"
#include <time.h>
namespace ma{
		//call remote function
		template<typename Conf>
		void NetImageFilm<Conf>::addSampleImpl(const sample_t &sample, const ray_t &ray,
			const spectrum_t &L, scalar_t alpha)
		{
			assert(connection_);
			rpc::send_rpc<rpc::c2s::rpc_add_sample>(net::connection_write_handler_ptr(new rpc::rpc_null_handler()),connection_,sample,ray,L,alpha);
		}
		template<typename Conf>
		void NetImageFilm<Conf>::writeImageImpl()
		{
			assert(connection_);
			rpc::send_rpc<rpc::c2s::rpc_write_image>(net::connection_write_handler_ptr(new rpc::rpc_null_handler()),connection_);
		}
	
}


#endif
