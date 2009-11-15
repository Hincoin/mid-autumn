#include "intrusive_list.hpp"
#include "common_functions.hpp"



//the wheel is recreated by luozhiyuan
namespace ma{
	namespace core{
		namespace details{
struct default_user_allocator_malloc_free
{
  typedef std::size_t size_type;
  typedef std::ptrdiff_t difference_type;

  static char * malloc(const size_type bytes)
  { return reinterpret_cast<char *>(std::malloc(bytes)); }
  static void free(char * const block)
  { std::free(block); }
};
			
			//to eliminate dependent from boost
			//this compile time gcd/lcm code is copy from boost.pool lib
		template<unsigned A,unsigned B,bool Bis0>
			struct gcd_helper;
		template<unsigned A, unsigned B>
			struct gcd_helper<A,B,false>
			{
				static const unsigned A_mod_B_ = A % B;
				static const unsigned value = 
					gcd_helper<B,A_mod_B_,A_mod_B_ == 0>::value;
			};
		template<unsigned A, unsigned B>
			struct gcd_helper<A,B,true>
			{
				static const unsigned value = A;
			};
		template<unsigned A,unsigned B>
			struct gcd{
				static const unsigned value = 
					gcd_helper<A,B,B == 0>::value;
			};
		template<unsigned A,unsigned B>
			struct lcm{
				static const unsigned value = 
					A / gcd<A,B>::value * B;
			};

		//get K to make (K*B)%A - M is minimum
		template<unsigned A,unsigned B,unsigned M,unsigned MinDepth,unsigned Depth,unsigned MaxDepth>
			struct MinimumKImpl;
		template<unsigned A,unsigned B,unsigned M,unsigned MinDepth,unsigned MaxDepth>
			struct MinimumKImpl<A,B,M,MinDepth,MaxDepth,MaxDepth>
			{
				static const unsigned value = MinDepth;
			};
		template<unsigned A,unsigned B,unsigned M,unsigned MinDepth,unsigned Depth,unsigned MaxDepth>
			struct MinimumKImpl
			{
				static const unsigned helper_value = (Depth * B - M) % A;
				static const unsigned pre_helper_value = (MinDepth * B - M) % A;
				static const unsigned cur_value= helper_value == 0 ?
					Depth :
					(pre_helper_value < helper_value  ?
					 MinDepth : Depth)
					;
				static const unsigned value =  helper_value == 0 ?
					Depth : (
				cur_value < MinimumKImpl<A,B,M,cur_value,Depth + 1,MaxDepth>::value ?
			cur_value : MinimumKImpl<A,B,M,cur_value,Depth + 1,MaxDepth>::value);
			};
			template<unsigned A,unsigned B,unsigned M,unsigned Depth=498>
			struct MinimumK:MinimumKImpl<A,B,M,Depth+1,M/B + 1,Depth>{};
	
		
		}
template<size_t Size,size_t NextCount,typename UserAllocator>
	struct fixed_pool_impl_fast_segregate{
	typedef typename UserAllocator::size_type size_type;
	typedef typename UserAllocator::difference_type difference_type;

	void* malloc(){
		if (!free_list_)
		{
			page_list_.push_front(new (system_alloc()) page);
		}
		return alloc_from_free_list();
	}
	void free(void *ptr)	
	{
		FreeNode* t = (FreeNode*)(ptr); 
		t->next = free_list_;
		free_list_ = t;	
		page* cur_page = get_page(ptr);
		cur_page->dec_ref();
		//push_front
		if (cur_page->count() == 0)
		{
			cur_page->unlink();
			page_list_.push_front(cur_page);
		}

	}
	size_type get_requested_size(){return requested_size_;}
	bool release_memory(){
		bool ret = false;
		//remove from free list
		if (free_list_)
		{
			while(free_list_ && get_page(free_list_)->count()==0)
			{
				free_list_ = free_list_->next;
				ret = true;
			}
			FreeNode* cur = free_list_;
			FreeNode* prev = free_list_;
			while(cur)
			{
				page* p = get_page(cur);
				if (p->count() == 0)
				{
					//delete cur node
					prev->next = cur->next;
					cur = cur->next;
					ret = true;
				}
				else{
					prev = cur;
					cur = cur->next;
				}
			}
		}	
		while((!page_list_.empty()) && page_list_.front().count()==0)
		{
			page* p = &page_list_.front();
			page_list_.pop_front();
			system_free(p);
		}
		return ret;
	}
		private:
		private:
		struct FreeNode{
			FreeNode* next;
		};
				struct page : intrusive_list<page>::node {
					page():ref_cnt_(0){}
					size_type ref_cnt_;
					size_t count() const {return ref_cnt_;}
					//empty free space
					bool empty() const {return ref_cnt_ == (page_request_size_-sizeof(page))/requested_size_;}

					void inc_ref() {ref_cnt_++;assert(ref_cnt_ <= (page_request_size_-sizeof(page))/requested_size_);}
					void dec_ref() {assert(ref_cnt_> 0); ref_cnt_--;}
				};
		intrusive_list<page> page_list_;
		FreeNode* free_list_;
		page* get_page(void* ptr){
			return (page*)details::align_down(ptr,page_request_size_);
		
		}
		void* system_alloc()
		{
			assert(!free_list_);
			void* ptr = details::virtual_alloc(page_request_size_);
			// page address must be naturally aligned
			assert(((size_t)ptr & (details::PAGE_SIZE-1)) == 0);
			void* p_last = (void*)((char*)ptr+page_request_size_-requested_size_);
			((FreeNode*) (p_last))->next = free_list_;
			free_list_ = (FreeNode*)(p_last);
			return ptr;
		}
		void system_free(void* ptr)
		{
			details::virtual_free(ptr,page_request_size_);
		}

		void* alloc_from_free_list()
		{
			assert(free_list_);
			void *ptr = free_list_;
			page* cur_page = get_page(ptr);
			cur_page->inc_ref();
			if (!free_list_->next && !cur_page->empty())
			{
				//difference_type dist = ptr - details::align_down(ptr,page_request_size_);	
				//if (dist >= (sizeof(page)+requested_size_))
				{
					free_list_->next = 
					 reinterpret_cast<FreeNode*>((char*)ptr - requested_size_);
					free_list_->next->next = 0;
				}
			}
			free_list_ = free_list_->next;
			//push_to_back
			if(cur_page->empty())
			{
				cur_page->unlink();
				page_list_.push_back(cur_page);
			}

			return ptr;
		}
		static const size_type requested_size_ = Size ;
		//make sure it is mutiple of PAGE_SIZE
		static const size_type MAX_PAGES = 32;
		static const size_type page_request_size_ = (NextCount * requested_size_< details::PAGE_SIZE ?
		   	details::MinimumK<requested_size_,details::PAGE_SIZE,sizeof(page),MAX_PAGES>::value : 
			details::MinimumK<
			NextCount * requested_size_,details::PAGE_SIZE,sizeof(page),MAX_PAGES
			>::value)*details::PAGE_SIZE;
		static const size_type page_count = page_request_size_/requested_size_;	
		public:
	fixed_pool_impl_fast_segregate()
	{
		free_list_ = 0;
	}
	~fixed_pool_impl_fast_segregate()
	{
		release_memory();
		//memory leak
		assert(page_list_.empty() && !free_list_);
	}

};


	
	}

}
