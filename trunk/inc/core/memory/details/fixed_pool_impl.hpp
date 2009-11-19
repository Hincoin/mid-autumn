#include <limits>
#include "intrusive_list.hpp"
#include "common_functions.hpp"

#include <boost/static_assert.hpp>

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
/*
template<unsigned Size,unsigned NextCount,typename UserAllocator>
struct fixed_pool_impl_rb_tree_segregate{
	typedef typename UserAllocator::size_type size_type;
	typedef typename UserAllocator::difference_type difference_type;
	
	void* malloc(){}
	void free(void* ptr){}
	bool release_memory(){}
	fixed_pool_impl_rb_tree_segregate(){}
	~fixed_pool_impl_rb_tree_segregate(){}
	private:
	struct FreeNode{
		FreeNode* next;
	};
	struct page:public intrusive_multi_rbtree<page>::node{
		size_type ref_cnt_;
	};
	struct page_node:public intrusive_list<page_node>::node{
	};
	FreeNode* free_list_;
	intrusive_rb_tree<page> page_tree_;
	intrusive_list<page_node> empty_nodes_;
};
*/
//Size < 4
template<unsigned Size,typename UserAllocator>
struct fixed_pool_impl_small{
	private:           	
	
		struct bucket_info{
			bucket_info(unsigned char bucket_sz):free_list_(0){
				if (bucket_sz == 0){free_list_=UCHAR_MAX;return;}
				unsigned char* f=get_first_free_mem();
				unsigned char* t = f;
				for(unsigned char i = 0;i < bucket_sz
					;)
				{
					f = t;
					*f = ++i;
					t += requested_size_; 
				}	
				*f = UCHAR_MAX;

			}
			unsigned char free_list_;
			unsigned char next_block_;
			//no free block
			bool empty()const{ return free_list_ == UCHAR_MAX;}
			bool is_free(){
				unsigned char free_count = 0;
				unsigned char next = free_list_;
				for(;next != UCHAR_MAX;)
				{
					next = *get_mem_by_idx(next);
					free_count ++;	
				}
				return free_count == bucket_size_;
			}
			void* alloc(){
				assert(!empty());
				unsigned char* f = get_first_free_mem();	
				free_list_ = *f;
				return f;
			}
			void free(void* ptr)
			{
				unsigned char* p =(unsigned char*)ptr;
				*p = free_list_;
				unsigned char idx = (p - reinterpret_cast<unsigned char*>(this) - sizeof(bucket_info))/requested_size_; 
				free_list_ = idx;
				assert(!dead_loop());
			}
			bool dead_loop()
			{
				unsigned char next = free_list_;
				for(;next != UCHAR_MAX;)
				{
					next = *get_mem_by_idx(next);
					if (free_list_ == next)return true;
				}
				return false;
			}
			unsigned char* get_first_free_mem()
			{
				return get_mem_by_idx(free_list_);
			}
			unsigned char* get_mem_by_idx(unsigned char idx)
			{
				return reinterpret_cast<unsigned char*>(
					reinterpret_cast<unsigned char*>(this)+ sizeof(bucket_info) +idx * requested_size_
					);

			}	
		};

	struct page:public intrusive_list<page>::node{
		unsigned char free_list_;	
		unsigned char ref_cnt_;
		page():free_list_(0){
			void* pData = get_first_free_bucket();
			bucket_info* bucket = 0;// = new () bucket_info(bucket_size_ );
			assert(sub_page_count_ * sub_page_size_ < details::PAGE_SIZE);
			for(unsigned char i = 0;i < sub_page_count_;)
			{
				bucket = new (pData) bucket_info(bucket_size_);
				bucket->free_list_ = 0;
				bucket->next_block_ = ++i;
				pData = (char*)pData + sub_page_size_;
			}
			unsigned char extra_room = (char*)(this) + details::PAGE_SIZE - (char*)pData;
			unsigned char extra_size = extra_room > sizeof(bucket_info)?(extra_room - sizeof(bucket_info))/requested_size_ : 0;
			if (extra_size > 0)
			{
				bucket = new (pData) bucket_info(extra_size);
				bucket->free_list_ = 0;
			}
			bucket->next_block_ = UCHAR_MAX;
		}
		//no free space
		bool empty()const{return free_list_ == UCHAR_MAX;}
		bool is_free(){
			bucket_info* bucket = get_first_free_bucket();
			for(size_t i = 0;i < sub_page_count_;i++)
			{
				if(!bucket->is_free())return false;
				bucket = reinterpret_cast<bucket_info*>(
						reinterpret_cast<char*>(bucket) + sub_page_size_
						);
			}		
			return true;
		}
		void* alloc(){
			assert(!empty());
			bucket_info* bucket = get_first_free_bucket();
			void* ptr = bucket->alloc();
			if (bucket->empty())
			{
				free_list_ = bucket->next_block_;
				ref_cnt_ ++;
			}
			return ptr;
		}
		void free(void* ptr){
			//bucket 
			unsigned char b_idx = ((char*)ptr-reinterpret_cast<char*>(this)-sizeof(page))/sub_page_size_;
			bucket_info* b = reinterpret_cast<bucket_info*>( reinterpret_cast<char*>(this) + sizeof(page)  + b_idx * sub_page_size_);
			if (b->empty())
			{
				b->free(ptr);
				b->next_block_ = free_list_;
				free_list_ = b_idx;
				ref_cnt_--;
			}
			else{
				b->free(ptr);
			}

		}
		unsigned char sub_page_count()const{return ref_cnt_;}
		bucket_info* get_first_free_bucket(){
			return reinterpret_cast<bucket_info*>(	reinterpret_cast<char*>(this)+sizeof(page) + free_list_ * sizeof(bucket_info));
		}

	};
	

	public:

	typedef typename UserAllocator::size_type size_type;
	typedef typename UserAllocator::difference_type difference_type;
	
	void* malloc(){
		//no free space
		if ( page_list_.empty() || page_list_.front().empty())
		{
			page_list_.push_front(new (system_alloc()) page);
		}	
		return alloc_from_page_list();
	}
	void free(void* ptr){
		page* p = get_page(ptr);	
		p->free(ptr);
		if (p->sub_page_count() == 0)
		{
			p->unlink();
			page_list_.push_front(p);
		}
	}
	bool release_memory(){
		bool ret = false;
		page* p = &page_list_.front();
		while(p)
		{
			if(p->is_free())
			{
				page* next = p->next();
				p->unlink();
				system_free(p);
				p = next;
				ret = true;
			}
			else p = p->next();
		}
		return ret;
	
	}
	private:
	void* system_alloc()
	{
		void* ptr = details::virtual_alloc(details::PAGE_SIZE);
		assert(((size_t)ptr & (details::PAGE_SIZE-1)) == 0);
		return ptr;
	}
	void system_free(void* ptr)
	{
		details::virtual_free(ptr,details::PAGE_SIZE);
	}
	void* alloc_from_page_list()
	{
		page& p = page_list_.front();
		void *ptr = p.alloc();
		if (p.empty())
		{
			p.unlink();
			page_list_.push_back(&p);
		}
		return ptr;
	}

	page* get_page(void* ptr){
		return (page*)(details::align_down(ptr,details::PAGE_SIZE));
	
	}

	public:

template<unsigned Start,unsigned Depth,unsigned PrevDepth,typename MetaFunc>
struct MinimumImpl;

template<unsigned Depth,unsigned PrevDepth,typename MetaFunc>
struct MinimumImpl<Depth,Depth,PrevDepth,MetaFunc>
{
    static const unsigned value = MetaFunc::template apply<Depth>::value < MetaFunc::template apply<PrevDepth>::value ?
    Depth : PrevDepth;
};
template<unsigned CurDepth,unsigned Depth,unsigned PrevDepth,typename MetaFunc>
struct MinimumImpl
{
    static const unsigned prev_value = MetaFunc::template apply<PrevDepth>::value;
    static const unsigned cur_value = MetaFunc::template apply<CurDepth>::value;
    static const unsigned passed_depth = cur_value < prev_value ? CurDepth : PrevDepth;
    static const unsigned value =
    MinimumImpl<CurDepth-1,Depth,passed_depth,MetaFunc>::value;
};
template<unsigned Start,unsigned Depth,typename MetaFunc>
struct Minimum:MinimumImpl<Start,Depth,Start,MetaFunc>{};
template<unsigned A,unsigned B >
struct mini_cost_meta_func{
        template<unsigned K>
        struct apply{
			static const unsigned value = (B-sizeof(page))%(sizeof(bucket_info) + K * A); 
        };
};
static const unsigned char requested_size_ = Size;
static const unsigned char min_stop_ = (UCHAR_MAX+1 - sizeof(bucket_info))/requested_size_ > 128 ?
(UCHAR_MAX + 1 - sizeof(bucket_info))/requested_size_ : 128;
static const unsigned char bucket_size_ = Minimum<254,min_stop_,mini_cost_meta_func<requested_size_,details::PAGE_SIZE> >::value;

	static const unsigned sub_page_size_ = sizeof(bucket_info) + requested_size_ * bucket_size_;

static const unsigned sub_page_count_ = (details::PAGE_SIZE - sizeof(page))/(sub_page_size_);
BOOST_STATIC_ASSERT((sub_page_count_ > 0 ));
BOOST_STATIC_ASSERT((sub_page_size_ > 0));
	BOOST_STATIC_ASSERT((sub_page_count_ * sub_page_size_ <=  (details::PAGE_SIZE - sizeof(page)) ));
	intrusive_list<page> page_list_;	
	};
	}

}
