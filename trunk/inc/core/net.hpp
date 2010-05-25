#ifndef _MA_INCLUDED_NET_HPP_
#define _MA_INCLUDED_NET_HPP_

#include <iomanip>
#include <string>
#include <sstream>
#include <iostream>
#include <vector>
#include <deque>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/tuple/tuple.hpp>
#include "Move.hpp"
namespace ma
{
	namespace net
	{
		using boost::asio::ip::tcp;
		struct connection_write_handler_base
		{
				
			typedef void (*handler_t)(const boost::system::error_code& e,std::size_t);
			typedef void result_type;
			connection_write_handler_base(handler_t func = 0):f(func){}
			virtual result_type operator()(const boost::system::error_code& e,std::size_t trans=0)
			{
				if(f)(*f)(e,trans);
			}
			virtual ~connection_write_handler_base(){}
			private:
			handler_t f;

		};
		typedef boost::shared_ptr<connection_write_handler_base> connection_write_handler_ptr;
		class Connection
		{
			public:
			Connection(boost::asio::io_service& io_service):socket_(io_service),context_(0)
			{}
			tcp::socket& socket(){return socket_;}
			//the memory of buffer should never be freed before the write is completed
			//the handle should responsible for queueing the data stream 
				void async_write(move_from<std::string> buffer,connection_write_handler_ptr handler)
				{
					bool is_write_in_progress = !msg_queue_.empty();
					msg_queue_.push_back(msg_t());
					msg_queue_.back().data.swap(buffer.source);
					std::ostringstream header_stream;
					header_stream << std::setw(header_length)
						<< std::hex << msg_queue_.back().data.size();
					if(!header_stream || header_stream.str().size() != header_length)
					{
						boost::system::error_code error(boost::asio::error::invalid_argument);
						socket_.io_service().post(boost::bind(*handler,error));
						return;
					}
					msg_queue_.back().header = header_stream.str();
					msg_queue_.back().handler = handler;
					if(!is_write_in_progress)
					{
						std::vector<boost::asio::const_buffer> buffers;
						buffers.push_back(boost::asio::buffer(msg_queue_.front().header));	
						buffers.push_back(boost::asio::buffer(msg_queue_.front().data));
						boost::asio::async_write(socket_,buffers,boost::bind(&Connection::async_write_handler,this,boost::asio::placeholders::error,handler));
					}
				}
			void async_write_handler(const boost::system::error_code& e,
						connection_write_handler_ptr handler)
				{
					if(!e)
					{
						msg_queue_.pop_front();
						if(!msg_queue_.empty())
						{
						std::vector<boost::asio::const_buffer> buffers;
						buffers.push_back(boost::asio::buffer(msg_queue_.front().header));	
						buffers.push_back(boost::asio::buffer(msg_queue_.front().data));
						boost::asio::async_write(socket_,buffers,boost::bind(&Connection::async_write_handler,this,boost::asio::placeholders::error,msg_queue_.front().handler));
						}
					}
					else
					{
						(*handler)(e);
					}
				}
			template<typename Handler>
				void async_read(size_t& size,void* &buffer,Handler handler)
				{
					//read header
					void (Connection::*f)(const boost::system::error_code&,
							size_t& ,void*&	,boost::tuple<Handler>)
						= &Connection::handle_read_header<Handler>;
						//read data
					boost::asio::async_read(socket_,boost::asio::buffer(inbound_header_),
							boost::bind(f,
								this,boost::asio::placeholders::error,
								boost::ref(size),boost::ref(buffer),
								boost::make_tuple(handler)));
				}
			template<typename Handler>
			void handle_read_header(const boost::system::error_code& e,
					size_t& size,void* &buffer,boost::tuple<Handler> handler)
			{
				if(e)
				{
					boost::get<0>(handler)(e);
				}
				else
				{
					std::istringstream is(std::string(inbound_header_,header_length));
					std::size_t inbound_data_size = 0;
					if(!(is>>std::hex >> inbound_data_size))
					{
						boost::system::error_code error(boost::asio::error::invalid_argument);
						boost::get<0>(handler)(e);
						return;
					}
					//async read data
					inbound_data_.resize(inbound_data_size);
					void (Connection::*f)(const boost::system::error_code&,
							size_t& ,void* &,boost::tuple<Handler>)
						= &Connection::handle_read_data<Handler>;
					boost::asio::async_read(socket_,boost::asio::buffer(inbound_data_),
							boost::bind(f,
								this,boost::asio::placeholders::error,
								boost::ref(size),boost::ref(buffer),
								(handler)));
				}

			}
			template<typename Handler>
				void handle_read_data(const boost::system::error_code& e,
						size_t& size,void* &buffer,boost::tuple<Handler> handler)
				{
					if(e)
					{
						boost::get<0>(handler)(e);
					}
					else
					{
						size = inbound_data_.size();
						buffer = &inbound_data_[0];
						boost::get<0>(handler)(e);
					}
				}
			//
			void set_context(void* c){context_ = c;}
			void* get_context()const{return context_;}
			private:
			tcp::socket socket_;
			enum{header_length = 8};
			struct msg_t{
				std::string header;
				std::string data;
				connection_write_handler_ptr handler;
			};
			std::deque<msg_t> msg_queue_;

			char inbound_header_[header_length];
			std::vector<char> inbound_data_;

			//context , a pointer point to any specific data needed
			void* context_;
		}; 
		typedef boost::shared_ptr<Connection> connection_ptr;


	}

}


#endif
