#include "pbr_svr.hpp"
#include "render_node.hpp"
namespace ma{

using namespace net;
using namespace rpc;
//represet a client
pbr_svr::pbr_svr(OOLUA::Script& lua,boost::asio::io_service& io_service,unsigned short port):acceptor_(io_service,
			boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(),port)),cur_frame_(0),next_frame_(0),lua_(lua)
	{
		//start an accept operation for a new connection
		conn_t new_conn (new Connection(acceptor_.io_service()));
		acceptor_.async_accept(new_conn->socket(),
				boost::bind(&pbr_svr::handle_accept,this,
					boost::asio::placeholders::error,new_conn));
	}
	///handle accept
	void pbr_svr::handle_accept(const boost::system::error_code& e,
			conn_t conn)
	{
		if(!e)
		{
			//add to all connection set, and handle read data from this connection
			render_node_ptr r( new render_node(conn,*this,next_frame_));
			r->start();
			//start an accept operation for new connection
			conn_t new_conn(new Connection(acceptor_.io_service()));
			acceptor_.async_accept(new_conn->socket(),
					boost::bind(&pbr_svr::handle_accept,this,
						boost::asio::placeholders::error,new_conn)
					);
		}
	}


	bool pbr_svr::getNextCropWindow(render_node& node ,crop_window* &win)
	{
		//take the w,h resolution of the image
		//todo test client code	
		render_node* r = &node;
		float w = (float)get_film(r->get_frame())->xResolution();
		float h = (float)get_film(r->get_frame())->yResolution();
		const float min_w = 256.f;
		const float min_h = 256.f;
		const float min_fraction = 1/128.f;
		float crop_stepw = std::max((min_w)/(float)w,min_fraction);	
		float crop_steph = std::max((min_h)/(float)h,min_fraction);	
		crop_stepw = std::min(1.f,crop_stepw);
		crop_steph = std::min(1.f,crop_steph);
		if(crop_windows_.empty())
		{
			crop_windows_.push_back(crop_window(next_frame_,0,0,crop_stepw,0,crop_steph));
		}
		else
		{
			const crop_window& last = crop_windows_.back();
			float ymin = last.xmax >= 1.f ? last.ymax:last.ymin;
			float ymax = std::min(1.f,ymin+crop_steph);
			
			float xmin = last.xmax >= 1.f && ymin < ymax ? 0:last.xmax;
			float xmax = std::min(1.f,xmin+crop_stepw) ;
			int id = last.id + 1;
			if(xmin >= xmax && ymin >= ymax)
			{
				assert(r->get_frame() <= next_frame_);
				printf("get status : %d\n",r->get_status());
				if (r->get_frame() == next_frame_ && r->get_status() == START_FRAME)
				{
					//request crop
					{
						id = 0;
						xmin = 0;
						ymin = 0;
						xmax = std::min(1.f,xmin + crop_stepw);
						ymax = std::min(1.f,ymin + crop_steph);
						printf("get status : %d\n",r->get_status());
					}
				}
				else if (r->get_status() == RENDER_CROP)
				{
					r->end_current_frame();
					return false;
				}
				else if (r->get_status() == END_FRAME)
				{
						printf("cur_frame_ next_Frame_ %d ,%d\n",cur_frame_,next_frame_);
					if (r->get_frame() == next_frame_) next_frame_++;
					r->set_frame(next_frame_);
					end_frame();
					//if not exceeds max frame
					const job_desc& cur_job= job_files_.front();
					printf("cur frame:%d end_frame: %d",r->get_frame(),cur_job.end_frame);
					if (r->get_frame() < cur_job.end_frame)
						r->start_current_frame();
					else
					{
						r->idle();
						end_scene();
					}
					return false;
				}
				
			}
			//	ymin=xmin=0.f;
			//		id = 0;
			//	next_frame_++;
			//	start_current_frame(r->get_conn());
			crop_windows_.push_back(crop_window(
						next_frame_,
						id,
						xmin,
						xmax,
						ymin,
						ymax));
		}
		win = &crop_windows_.back();
		r->set_status(RENDER_CROP);
		return true;
	}
	void pbr_svr::end_frame()
	{
		//end current frame
		bool frame_all_done = true;
		for(connection_set_t::iterator it = connections_.begin();
				it != connections_.end();++it)
		{
			if ((*it) ->get_type() == RENDERER &&  (*it)->get_frame() <= cur_frame_)
				frame_all_done = false;
		}
		printf("frame_all_done : %d\n",frame_all_done);
		if (frame_all_done)
		{
			std::vector<crop_window> finished_crops;
			//todo: checking missing piece and re-render it
			for(size_t i = 0;i <  crop_windows_.size();++i)
			{
				if(crop_windows_[i].frame != cur_frame_)
					finished_crops.push_back(crop_windows_[i]);
			}
			finished_crops.swap(crop_windows_);

			//todo get current film write image
			write_image(cur_frame_);
	
			cur_frame_++;		
			if (next_frame_ < cur_frame_)next_frame_ = cur_frame_;
		}
		printf("crop_windows_ size: %d",crop_windows_.size());
	}

	void pbr_svr::end_scene()
	{
		//try to end cur_scene
		printf("try to end scene\n");
		for(connection_set_t::iterator it = connections_.begin();
				it != connections_.end();++it)
		{
			if ((*it)->get_type() == RENDERER && (*it)->get_status() != IDLE)
			{
				return;
			}
		}
		if(!(lua_ && lua_.call("cleanup")))
		{
			OOLUA::lua_stack_dump(lua_);
			return ;
		}
		printf("end scene \n");
		for(connection_set_t::iterator it = connections_.begin();
				it != connections_.end();++it)
		{
			if ((*it)->get_type() != CONTROLLER )
			{
				(*it)->end_scene();
			}
		}
		job_files_.front().clearFilms();
		job_files_.pop_front();
		do_render_scene();
	}

	void pbr_svr::do_render_scene()
	{
		if (job_files_.empty())return;
		const std::string& f = job_files_.front().file;
		const job_desc& job = job_files_.front();
		next_frame_ = cur_frame_ = job_files_.front().begin_frame;
		if (!lua_.run_file(f))
		{
			OOLUA::lua_stack_dump(lua_);
			return;
		}
		if(!(lua_ && lua_.call("startup",false)))
		{
			OOLUA::lua_stack_dump(lua_);
			return ;
		}
		if(!	get_renderer())
		{
			printf("create renderer failed!\n");return;//create renderer
		}
		job_files_.front().initFilms(ma::get_film());
		printf("start render scene : %s,%d,%d \n",job_files_.front().file.c_str(),job_files_.front().begin_frame,job_files_.front().end_frame);
		for(connection_set_t::iterator it = connections_.begin();
				it != connections_.end();++it)
		{
			if ((*it)->get_type() != CONTROLLER && (*it)->get_status() == IDLE)
			{
					(*it)->set_frame(job.begin_frame);
				(*it)->render_scene(f);
			}
		}
	

	}
	void pbr_svr::render_scene(const std::string& f,int begin,int end)
	{
		if ( begin < end)
		{
			job_files_.push_back(job_desc(f,begin,end));
			if(job_files_.size() > 1) return;
			do_render_scene();
		}
	}
	void pbr_svr::write_image(int frame)
	{
		get_film(frame)->writeImage();
		//todo:
		//send image back to controller
	}
	film_ptr pbr_svr::get_film(int frame)
	{
		if(job_files_.empty())return 0;
		const job_desc& cur_job = job_files_.front();
		return cur_job.films[frame - cur_job.begin_frame];
	}
}
