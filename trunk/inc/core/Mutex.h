#ifndef MUTEX_H
#define MUTEX_H

namespace ma
{
	namespace core
	{
		struct NullMutex{void lock(){} void unlock(){}};

		template<typename Mutex>
		class ScopeLock{
			Mutex & mtx;

			ScopeLock(const ScopeLock &);
			void operator=(const ScopeLock &);
		public:
			explicit ScopeLock(Mutex &m):mtx(m){	mtx.lock();}
			~ScopeLock(){ mtx.unlock();}
		};
	}
}
#endif