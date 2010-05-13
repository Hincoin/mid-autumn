#ifndef _INCLUDED_POOL_MUTEX_HPP_
#define _INCLUDED_POOL_MUTEX_HPP_

//#ifdef WIN32
//#  include <windows.h>
//#else
//#  include <pthread.h>
//#endif
#include "common_functions.hpp"

namespace ma{
	namespace core{
		namespace details{
#ifdef WIN32
			class win32_mutex
			{
			private:
				CRITICAL_SECTION mtx;

				win32_mutex(const win32_mutex &);
				void operator=(const win32_mutex &);

			public:
				win32_mutex()
				{ InitializeCriticalSection(&mtx); }

				~win32_mutex()
				{ DeleteCriticalSection(&mtx); }

				void lock()
				{ EnterCriticalSection(&mtx); }

				void unlock()
				{ LeaveCriticalSection(&mtx); }
			};
			typedef win32_mutex mutex_t;
#else
			class pthread_mutex
			{
			private:
				pthread_mutex_t mtx;

				pthread_mutex(const pthread_mutex &);
				void operator=(const pthread_mutex &);

			public:
				pthread_mutex()
				{ pthread_mutex_init(&mtx, 0); }

				~pthread_mutex()
				{ pthread_mutex_destroy(&mtx); }

				void lock()
				{ pthread_mutex_lock(&mtx); }

				void unlock()
				{ pthread_mutex_unlock(&mtx); }
			};
			typedef pthread_mutex mutex_t;
#endif
			class null_mutex
			{
			private:
				null_mutex(const null_mutex &);
				void operator=(const null_mutex &);

			public:
				null_mutex() { }

				static void lock() { }
				static void unlock() { }
			};
			template <typename Mutex>
			class scope_lock
			{
			private:
				Mutex & mtx;

				scope_lock(const scope_lock &);
				void operator=(const scope_lock &);

			public:
				explicit scope_lock(Mutex & nmtx)
					:mtx(nmtx) { mtx.lock(); }

				~scope_lock() { mtx.unlock(); }
			};
		}

	}
}

#endif
