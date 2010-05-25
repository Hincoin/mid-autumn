#ifndef _MA_INCLUDED_PBR_SVR_HPP_
#define _MA_INCLUDED_PBR_SVR_HPP_

#include <deque>
#include <sstream>
#include "net.hpp"
#include "dispatcher.hpp"
#include <set>
#include "rpc_svr.hpp"

#include "oolua.h"
namespace ma{


	class render_node;
	typedef boost::shared_ptr<render_node> render_node_ptr;

	struct job_desc{
		job_desc(const std::string& f,int b,int e):file(f),begin_frame(b),end_frame(e)
		{}
		std::string file;
		int begin_frame,end_frame;
		std::vector<film_ptr> films;
		void initFilms(film_ptr film)
		{
			films.resize(std::max(end_frame-begin_frame,1));
			for(int i = begin_frame ;i < end_frame; ++i)
			{
				std::stringstream ss;
				ss<<file<<i;
				films[i-begin_frame] = static_cast<film_ptr>(film->clone(ss.str()));
			}
		}
		void clearFilms()
		{
			for(size_t i = 0;i < films.size(); ++i)
				delete_ptr(films[i]);
			films.clear();
		}
	};
class pbr_svr
{
	private:
		boost::asio::ip::tcp::acceptor acceptor_;
	public:
	pbr_svr(OOLUA::Script& lua,boost::asio::io_service& io_service,unsigned short port);
	///handle accept
	void handle_accept(const boost::system::error_code& e,
			rpc::conn_t conn);
	void add_render_node(render_node_ptr r){ connections_.insert(r);}
	void disconnect_render_node(render_node_ptr r){connections_.erase(r);}
	//svr logic here
	bool getNextCropWindow(render_node& r,crop_window* &w);	
	void end_frame();//end current frame
	void render_scene(const std::string& f,int begin,int end);
	void end_scene();
	void write_image(int frame);
	film_ptr get_film(int frame);
	private:
	void do_render_scene();
	private:
	typedef std::set<render_node_ptr> connection_set_t;
	std::vector<crop_window> crop_windows_;//
	connection_set_t connections_;
	int cur_frame_;
	int next_frame_;
	OOLUA::Script& lua_;
	std::deque<job_desc> job_files_;
};


}


#endif
