#ifndef _INCLUDED_COMMON_FUNCTIONS_HPP_
#define _INCLUDED_COMMON_FUNCTIONS_HPP_



#if defined(WIN32)
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef _WIN32_WINNT
#define _WIN32_WINNT WINVER
#endif
#include <windows.h>
#else
#include <pthread.h>
#include <sys/mman.h>
static pthread_mutex_t m_mutex = PTHREAD_RECURSIVE_MUTEX_INITIALIZER_NP;
#endif



namespace ma{
	namespace core{

		namespace details{
			struct memory_null_tag;

			template<class T> inline T round_down(T x, size_t a) {return x & -(int)a;}
			template<class T> inline T round_up(T x, size_t a) {return (x + (a-1)) & -(int)a;}
			template<class T> inline T* align_down(T* p, size_t a) {return (T*)((size_t)p & -(int)a);}
			template<class T> inline T* align_up(T* p, size_t a) {return (T*)(((size_t)p + (a-1)) & -(int)a);}
			//////////////////////////////////////////////////////////////////////////
			// on Win32 we use the virtual memory system to get large pages from the OS
			// virtual memory is not necessary for the correctness of the algorithm
#if defined(WIN32)
			const size_t VIRTUAL_PAGE_SIZE_LOG2 = 16;
			const size_t VIRTUAL_PAGE_SIZE  = (size_t)1 << VIRTUAL_PAGE_SIZE_LOG2;
			inline void* virtual_alloc(size_t size) {
				assert(size / VIRTUAL_PAGE_SIZE * VIRTUAL_PAGE_SIZE == size);
				return VirtualAlloc(NULL, size, MEM_COMMIT, PAGE_READWRITE);
				//return ::malloc(size);
			}
			inline void virtual_free(void* addr) {
				//return ::free(addr);
				BOOL ret = VirtualFree(addr, 0, MEM_RELEASE);
				assert(ret);
			}
#else
			const size_t VIRTUAL_PAGE_SIZE_LOG2 = 12;
			const size_t VIRTUAL_PAGE_SIZE  = (size_t)1 << VIRTUAL_PAGE_SIZE_LOG2;
			inline void* virtual_alloc(size_t size) {
				assert(size / VIRTUAL_PAGE_SIZE * VIRTUAL_PAGE_SIZE == size);
				unsigned int start = 0;
				void* ret = mmap(&start, size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
				return ret;
			}
			inline void virtual_free(void* addr, size_t size) {
				int ret = munmap(addr, size);
				(void)ret;
				assert(ret == 0);
			}
#endif
			static const size_t PAGE_SIZE  = 1 << VIRTUAL_PAGE_SIZE_LOG2;
		}
	}
}


#endif