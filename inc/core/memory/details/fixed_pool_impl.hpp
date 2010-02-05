#include <cstdlib>
#include <climits>
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
				static void* virtual_alloc(size_type sz){return details::virtual_alloc(sz);}
				static void virtual_free(void* addr, size_t size){return details::virtual_free(addr,size);}
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

			template<bool Flag,typename A,typename B>
			struct Select_{typedef B type;};
			template<typename A,typename B>
			struct Select_<true,A,B>{typedef A type;};


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
				FreeNode* t = FreeNode::as_free_node(ptr);
				t->next = free_list_;
				free_list_ = t;	
				page* cur_page = t->get_page();
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
					while(free_list_ && (free_list_->get_page())->count()==0)
					{
						free_list_ = free_list_->next;
						ret = true;
					}
					FreeNode* cur = free_list_;
					FreeNode* prev = free_list_;
					while(cur)
					{
						page* p = cur->get_page();
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
			static const size_type MinSize = 32;
			struct page;
			struct FreeNode;
			struct FreeNodeBase_Empty{
				void set_page(page*){}
				page* get_page(){
					BOOST_STATIC_ASSERT(page_request_size_ == details::PAGE_SIZE);
					return (page*)details::align_down(this,page_request_size_);
				}	
				static page* get_page(void* ptr){return ((FreeNodeBase_Empty*)ptr)->get_page();}
				void* get_memory(){return this;}
				static FreeNode* as_free_node(void* ptr){return (FreeNode*)ptr;}
				static const size_type extra_space = 0;
			};
			struct FreeNodeBase_Header{
				page* page_header;
				void set_page(page* p){
					page_header = p;
					assert(((size_t)p & (details::PAGE_SIZE-1))==0);
				}
				page* get_page(){
					assert(((size_t)page_header &(details::PAGE_SIZE-1))==0);
					return page_header;
				}

				static page* get_page(void* ptr){
					void* p = ((FreeNodeBase_Header*)(ptr)-1)->page_header;
					assert(((size_t)p &(details::PAGE_SIZE-1))==0);
					return ((FreeNodeBase_Header*)(ptr)-1)->page_header;}
				void* get_memory(){
					assert(((size_t)page_header &(details::PAGE_SIZE-1))==0);
					return this+1;
				}
				static 	FreeNode* as_free_node(void* ptr){return (FreeNode*)((FreeNodeBase_Header*)(ptr)-1);}
				static const size_type extra_space = sizeof(page*);
			};
			struct FreeNode:details::Select_< (Size>MinSize),
			FreeNodeBase_Header,FreeNodeBase_Empty>::type{
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
				return FreeNode::get_page(ptr);
			}
			void* system_alloc()
			{
				assert(!free_list_);
				void* ptr = UserAllocator::virtual_alloc(page_request_size_);
				// page address must be naturally aligned
				assert(((size_t)ptr & (details::PAGE_SIZE-1)) == 0);
				void* p_last = (void*)((char*)ptr+page_request_size_-requested_size_);
				((FreeNode*) (p_last))->next = free_list_;
				free_list_ = (FreeNode*)(p_last);
				free_list_->set_page((page*)ptr);
				return ptr;
			}
			void system_free(void* ptr)
			{
				UserAllocator::virtual_free(ptr,page_request_size_);
			}

			void* alloc_from_free_list()
			{
				assert(free_list_);
				void* ptr = free_list_->get_memory();
				page* cur_page = free_list_->get_page();
				cur_page->inc_ref();
				if (!free_list_->next && !cur_page->empty())
				{
					//difference_type dist = ptr - details::align_down(ptr,page_request_size_);	
					//if (dist >= (sizeof(page)+requested_size_))
					{
						free_list_->next = 
							reinterpret_cast<FreeNode*>((char*)free_list_- requested_size_);
						free_list_->next->set_page(cur_page);
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
				assert(((size_t)(FreeNode::as_free_node(ptr)->get_page())  & (details::PAGE_SIZE -1)) == 0);
				assert(((size_t)(	get_page(ptr)) & (details::PAGE_SIZE-1)) == 0);
				return ptr;
			}
			static const size_type requested_size_ = Size + FreeNode::extra_space;
			//make sure it is mutiple of PAGE_SIZE
			static const size_type MAX_PAGES = 32;
			static const size_type computed_size =  (NextCount * requested_size_< details::PAGE_SIZE ?
				details::MinimumK<requested_size_,details::PAGE_SIZE,sizeof(page),MAX_PAGES>::value : 
			details::MinimumK<
				NextCount * requested_size_,details::PAGE_SIZE,sizeof(page),MAX_PAGES
			>::value)*details::PAGE_SIZE;
			static const size_type page_request_size_ = requested_size_ > MinSize? computed_size:details::PAGE_SIZE/**/;

			BOOST_STATIC_ASSERT((requested_size_ > sizeof(FreeNode*)));
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
		// Size > 1
		template<unsigned short Size,typename UserAllocator>
		struct fixed_pool_impl_small_segregate{
		private:
			struct page:intrusive_list<page>::node{
				unsigned short free_list_;
				unsigned short ref_cnt_;
				page():free_list_(0),ref_cnt_(0){
					char* first = (char*)get_block_by_idx(free_list_);
					for(unsigned short i = 0;i < max_elem_count_;){
						*((unsigned short*) first ) = ++i;
						first += requested_size_;
					}
				}
				void free(void* ptr){
					ref_cnt_--;
					*((unsigned short*)ptr) = free_list_;
					free_list_ =(unsigned short)(((char*)ptr - (char*)this - sizeof(page)) / requested_size_) ;
				}
				bool empty(){
					return ref_cnt_ == max_elem_count_;
				}
				unsigned short count(){return ref_cnt_;};
				void* alloc(){
					assert(ref_cnt_ < max_elem_count_);
					ref_cnt_ ++ ;
					unsigned short* cur = get_block_by_idx(free_list_);
					free_list_ = *cur;
					return cur;
				}
				unsigned short* get_block_by_idx(unsigned short idx){
					void * p = ((char*)(this) + sizeof(page) + requested_size_ * idx);
					return (unsigned short*)p;
				}
			};
			intrusive_list<page> page_list_;
			void* system_alloc(){return UserAllocator::virtual_alloc(details::PAGE_SIZE);};
			void system_free(void* ptr){UserAllocator::virtual_free(ptr,details::PAGE_SIZE);}
			void* alloc_from_list(){
				page* p = &page_list_.front();
				assert(!p->empty());
				return p->alloc();
			}
			static const unsigned short requested_size_ = Size;
			static const unsigned short max_elem_count_ = (details::PAGE_SIZE - sizeof(page)) /requested_size_;
			BOOST_STATIC_ASSERT((requested_size_ > (details::PAGE_SIZE - sizeof(page))/USHRT_MAX));
		public:
			void* malloc()
			{
				if(page_list_.empty() || page_list_.front().empty()){
					page_list_.push_front(new (system_alloc()) page);
				}
				return alloc_from_list();
			}
			void free(void* ptr)
			{
				page* p = (page*)details::align_down(ptr,details::PAGE_SIZE);
				p->free(ptr);
				if (p->count() == 0)
				{
					p->unlink();
					page_list_.push_front(p);
				}
			}
			bool release_memory()
			{
				bool ret = false;
				while (!page_list_.empty() && page_list_.front().count() == 0)
				{
					page& p = page_list_.front();
					page_list_.pop_front();
					system_free(&p);
					ret = true;
				}
				return ret;
			}
			fixed_pool_impl_small_segregate(){}
			~fixed_pool_impl_small_segregate(){release_memory();assert(page_list_.empty());}
		};
		//Size == 1
		template<unsigned Size,typename UserAllocator>
		struct fixed_pool_impl_small_list{
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
					assert(!dead_loop());
					return f;
				}
				void free(void* ptr)
				{
					unsigned char* p =(unsigned char*)ptr;
					*p = free_list_;
					unsigned char idx = (unsigned char)((p - reinterpret_cast<unsigned char*>(this) - sizeof(bucket_info))/requested_size_); 
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
					bucket->next_block_ = UCHAR_MAX;
				}
				//no free space
				bool empty()const{return free_list_ == UCHAR_MAX;}
				bool is_free(){
					unsigned char next = free_list_;
					unsigned char free_count = 0;
					while(next != UCHAR_MAX)
					{
						bucket_info* bucket = get_bucket_by_idx(next);
						if(!bucket->is_free())return false;
						next = bucket->next_block_;
						++free_count;
					}		
					return free_count == sub_page_count_;
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
					unsigned char b_idx = (unsigned char)(((char*)ptr-reinterpret_cast<char*>(this)-sizeof(page))/sub_page_size_);
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
					return get_bucket_by_idx(free_list_);
				}
				bucket_info* get_bucket_by_idx(unsigned char idx)
				{
					return reinterpret_cast<bucket_info*>(reinterpret_cast<char*>(this)+sizeof(page) + idx * sub_page_size_);
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
				typename intrusive_list<page>::iterator it = page_list_.begin();
				while (it != page_list_.end())
				{
					if (it->is_free())
					{
						page* free_node = &(*it);
						it = page_list_.erase(it);
						system_free(free_node);
						ret = true;
					}
					else ++it;
				}
				return ret;
			}
			~fixed_pool_impl_small_list(){
				release_memory();
				assert(page_list_.empty());
			}
		private:
			void* system_alloc()
			{
				void* ptr = UserAllocator::virtual_alloc(details::PAGE_SIZE);
				assert(((size_t)ptr & (details::PAGE_SIZE-1)) == 0);
				return ptr;
			}
			void system_free(void* ptr)
			{
				UserAllocator::virtual_free(ptr,details::PAGE_SIZE);
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
		template<unsigned Size,typename UserAllocator>
		struct fixed_pool_impl_small:
			details::Select_< (Size >= sizeof(short)),
			fixed_pool_impl_small_segregate<Size,UserAllocator>, 
			fixed_pool_impl_small_list<Size,UserAllocator>
			>::type
		{};
		template<unsigned Size,unsigned NextCount, typename UserAllocator>
		struct fixed_pool_impl:
			details::Select_<Size <= sizeof(void*),
			fixed_pool_impl_small<Size,UserAllocator>, 
			fixed_pool_impl_fast_segregate<Size,NextCount,UserAllocator> >::type
		{};
	}

}
