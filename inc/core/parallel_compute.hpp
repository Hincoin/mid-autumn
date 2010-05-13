#ifndef _INCLUDED_PARALLEL_COMPUTE_HPP_
#define _INCLUDED_PARALLEL_COMPUTE_HPP_

#include "MAConfig.hpp"

#ifdef TBB_PARALLEL
#include <tbb/tbb.h>
#include <tbb/parallel_while.h>

#if USE_PTHREAD
#include <pthread.h>
#else /* assume USE_WINTHREAD */
#ifndef NOMINMAX
#define NOMINMAX
#endif
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

#else
#include <vector>
#endif
#include <boost/static_assert.hpp>
#include <boost/type_traits.hpp>
#include "Singleton.hpp"


namespace ma{

#ifdef TBB_PARALLEL
	namespace core{
	namespace details{
		struct tls_worker_id 
		{
			tls_worker_id(unsigned i=0):id(i){}
			unsigned id;
		};

		typedef void (*tls_dtor_t)(void*);
		//! Basic cross-platform wrapper class for TLS operations.
		template <typename T>
		class basic_tls {
#if USE_PTHREAD
			typedef pthread_key_t tls_key_t;
		public:
			int  create( tls_dtor_t dtor = NULL ) {
				return pthread_key_create(&my_key, dtor);
			}
			int  destroy()      { return pthread_key_delete(my_key); }
			void set( T value ) { pthread_setspecific(my_key, (void*)value); }
			T    get()          { return (T)pthread_getspecific(my_key); }
#else /* USE_WINTHREAD */
			typedef DWORD tls_key_t;
		public:
			int create() {
				tls_key_t tmp = TlsAlloc();
				if( tmp==TLS_OUT_OF_INDEXES )
					return TLS_OUT_OF_INDEXES;
				my_key = tmp;
				return 0;
			}
			int  destroy()      { TlsFree(my_key); my_key=0; return 0; }
			void set( T value ) { TlsSetValue(my_key, (LPVOID)value); }
			T    get()          { return (T)TlsGetValue(my_key); }
#endif
		private:
			tls_key_t my_key;
		};

		//! More advanced TLS support template class.
		/** It supports RAII and to some extent mimic __declspec(thread) variables. */
		template <typename T>
		class tls : public basic_tls<T> {
			typedef basic_tls<T> base;
		public:
			tls()  { base::create();  }
			~tls() { base::destroy(); }
			T operator=(T value) { base::set(value); return value; }
			operator T() { return base::get(); }
		};

		template <typename T>
		class tls<T*> : basic_tls<T*> {
			typedef basic_tls<T*> base;
			static void internal_dtor(void* ptr) {
				if (ptr) delete (T*)ptr;
			}
			T* internal_get() {
				T* result = base::get();
				if (!result) {
					result = new T;
					base::set(result);
				}
				return result;
			}
		public:
			tls()  {
#if USE_PTHREAD
				base::create( internal_dtor );
#else
				base::create();
#endif
			}
			~tls() { base::destroy(); }
			T* operator=(T* value) { base::set(value); return value; }
			operator T*()   { return  internal_get(); }
			T* operator->() { return  internal_get(); }
			T& operator*()  { return *internal_get(); }
		};
	}//detals
	}//core
#endif
	struct thread_id_map{
#ifdef TBB_PARALLEL
		typedef tbb::tbb_thread::id id_type;
		typedef core::details::tls_worker_id tls_worker_id_t;
		typedef core::details::tls<core::details::tls_worker_id*> tls_type;
		typedef tbb::atomic<unsigned> worker_count_t;
		tls_type id;
		worker_count_t worker_count;
#else
		typedef unsigned id_type;
#endif
		unsigned hardware_concurrency_;
		thread_id_map(){
#ifdef TBB_PARALLEL
			using namespace tbb;
			hardware_concurrency_ = tbb_thread::hardware_concurrency();
#else
			hardware_concurrency_ = 1;
#endif
		}
		unsigned get_hardware_concurrency()const{return hardware_concurrency_;}
		unsigned get_thread_logic_id(){
#ifdef TBB_PARALLEL
			assert((*id).id < MAX_PARALLEL);
			return std::min<unsigned>((*id).id,MAX_PARALLEL-1);
#else
			return 0;
#endif
		}
		void on_thread_entry(){
#ifdef TBB_PARALLEL
			tls_worker_id_t& worker_id = *id;
			worker_id.id = worker_count;
			++worker_count;
#endif

		}
		void on_thread_exit(){
#ifdef TBB_PARALLEL
			--worker_count;
#endif
		}
	};
#ifdef TBB_PARALLEL
	struct thread_observer:public tbb::task_scheduler_observer{

		/*override*/ void on_scheduler_entry( bool /*is_worker*/ ){core::simple_singleton<thread_id_map>::instance().on_thread_entry();}
		/*override*/ void on_scheduler_exit( bool /*is_worker*/ ){core::simple_singleton<thread_id_map>::instance().on_thread_exit();}
	public:
		thread_observer( ) {
			observe(true);
		}
	};
#endif
	//hashed to [0,MAX_PARALLEL)
	//for lock free data accessing 
	inline unsigned get_thread_logic_id()
	{
		return core::simple_singleton<thread_id_map>::instance().get_thread_logic_id();
	}
	inline unsigned hardware_concurrency()
	{
		return core::simple_singleton<thread_id_map>::instance().get_hardware_concurrency();
	}
#ifndef TBB_PARALLEL
	template<typename T>
	struct simple_range{
		typedef T value_type;
		typedef value_type iterator;
		typedef value_type const_iterator;

		simple_range(value_type b,value_type e):begin_(b),end_(e){}
		iterator begin(){return begin_;}
		iterator end(){return end_;}
		const_iterator begin()const{return begin_;}
		const_iterator end()const{return end_;}
	private:
		value_type begin_;
		value_type end_;
	};
#endif
	//take range as input
	template<typename D>
	struct parallel_range_processor{
	private:
		D& derived(){return static_cast<D&>(*this);}
		const D& derived()const{return static_cast<const D&>(*this);}
	public:
		//evaluate the context
		template<typename RangeT>
		void operator()(
			const RangeT& r
			)const{
			for(typename RangeT::const_iterator i = r.begin();i != r.end() && derived().run(i); ++i);
			return;
		}
	};
	struct parallel_for{
		template<typename FuncT>
		static void run(const FuncT& f,size_t N)
		{
#ifdef TBB_PARALLEL
			tbb::parallel_for(tbb::blocked_range<size_t>(0,N),f,tbb::auto_partitioner());
#else
			return f(simple_range<size_t>(0,N));
#endif
			
		}
	};
	template<typename ProducerT>
	struct parallel_stream{
		typedef typename ProducerT::argument_type argument_type;
		typedef typename ProducerT::argument_value_type argument_value_type;
	private:
		ProducerT& f;
	public:
		parallel_stream(ProducerT& pf):f(pf){}
		bool pop_if_present(argument_type& v){
			return f(v);
		}
		argument_type& start(){return f.start();}
	};
	//take one item as input
	//this is paralleled by tbb , specify lock type if you have global shared data
	template<typename D>
	struct parallel_item_processor{
	private:
		D& derived(){return static_cast<D&>(*this);}
		const D& derived()const{return static_cast<const D&>(*this);}
	public:
		template<typename T>
		void operator()(T item)const{
			return derived().run(item);
		}
	};
	struct parallel_while{
		template<typename ProducerT,
		typename ConsumerT>
		static void run(parallel_stream<ProducerT>& stream,
		ConsumerT& func	
		)
		{
#ifndef TBB_PARALLEL
			while (stream.pop_if_present(stream.start()))
			{
				func(stream.start());
			}
#else
			tbb::parallel_while<ConsumerT> pw;
			return pw.run(stream,func);
#endif
			BOOST_STATIC_ASSERT((boost::is_same<typename ProducerT::argument_type,typename ConsumerT::argument_type>::value));
		}
	};

	namespace details{
#ifdef TBB_PARALLEL
		typedef tbb::filter pipeline_filter_base;
#else
		struct pipeline_filter_base{
			pipeline_filter_base(bool){}
			virtual void* operator()(void* item)=0;
			virtual ~pipeline_filter_base(){}
		};
#endif
	}
	template<typename D>
	struct pipeline_filter:details::pipeline_filter_base
	{
	private:
		D& derived(){return static_cast<D&>(*this);}
		const D& derived()const{return static_cast<const D&>(*this);}
	public:
		pipeline_filter(bool is_serial):details::pipeline_filter_base(is_serial){}
		void* operator()(void* item){
			return derived().run(item);
		};
	};
	struct pipeline{
		typedef details::pipeline_filter_base pipeline_filter_t;
#ifdef TBB_PARALLEL
		tbb::pipeline pipe;
#else
		std::vector<pipeline_filter_t*> pipe;
#endif
		void add_filter( pipeline_filter_t& filter_ ){
#ifdef TBB_PARALLEL
			pipe.add_filter(filter_);
#else
			pipe.push_back(&filter_);
#endif
		}
		 void run( size_t max_number_of_live_tokens )
		 {
#ifdef TBB_PARALLEL
			 pipe.run(max_number_of_live_tokens);
#else
			 
			if (pipe.empty())return;
			std::vector<pipeline_filter_t*>::iterator it = pipe.begin();
			void* item = 0;
			while ((item = (*(*it))(0)))
			{
				while (item && (++it) != pipe.end())
				{
					item = (*(*it))(item);
				}
				it = pipe.begin();
			}

#endif
		 }
		 void clear()
		 {
			 pipe.clear();
		 }
	};
}


#endif