////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2008 by luozhiyuan (luozhiyuan@gmail.com)
// Permission to use, copy, modify, distribute and sell this software for any 
//     purpose is hereby granted without fee, provided that the above copyright 
//     notice appear in all copies and that both that copyright notice and this 
//     permission notice appear in supporting documentation.
// The author makes no representations about the 
//     suitability of this software for any purpose. It is provided "as is" 
//     without express or implied warranty.
////////////////////////////////////////////////////////////////////////////////

#ifndef MUTEX_HPP
#define MUTEX_HPP

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