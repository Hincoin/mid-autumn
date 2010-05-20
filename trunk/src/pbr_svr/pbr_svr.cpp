#include "DefaultConfigurations.hpp"
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
		float w = (float)get_film()->xResolution();
		float h = (float)get_film()->yResolution();
		float crop_stepw = std::max((float)(32)/(float)w,std::min(0.1f,1.f/float(128)));	
		float crop_steph = std::max((float)(32)/(float)h,std::min(0.1f,1.f/float(128)));	
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
				else if (r->get_frame() < next_frame_ || r->get_status() != START_FRAME)
				{
					if (r->get_frame() == next_frame_) next_frame_++;
					r->set_frame(next_frame_);
					//if not exceeds max frame
					const int max_frame = 1;
					if (r->get_frame() < max_frame)
						r->start_current_frame();
					end_frame();
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
			if ( (*it)->get_frame() <= cur_frame_)
				frame_all_done = false;
		}
		if (frame_all_done)
		{
			std::vector<crop_window> finished_crops;
			for(size_t i = 0;i <  crop_windows_.size();++i)
			{
				if(crop_windows_[i].frame != cur_frame_)
					finished_crops.push_back(crop_windows_[i]);
			}
			finished_crops.swap(crop_windows_);

			//todo get current film write image
			get_film()->writeImage();
	
			cur_frame_++;		
			if (next_frame_ < cur_frame_)next_frame_ = cur_frame_;
		}
	}
}
