// $Id:   $

#ifndef MEMORYMGR_H
#define MEMORYMGR_H

#include <memory>

#include "NullType.h"

namespace ma{
	namespace core{

		template<class MemPool,class GC = EmptyType>
		class MemoryMgr:public MemPool//,public GC
		{
		private:
			//singleton stuff
			struct object_creator{
				object_creator(){MemoryMgr::getInstance();}
				inline void do_nothing()const{}
			};
		public:
			static MemoryMgr& getInstance(){
				static MemoryMgr mgr_;
				create_mgr_.do_nothing();
				return mgr_;
				}
		private:
			static object_creator create_mgr_;
		private:
			MemoryMgr(){}
			MemoryMgr(const MemoryMgr&);
			MemoryMgr& operator=(const MemoryMgr&);
		};
		template<class MemPool,class GC>
		typename MemoryMgr<MemPool,GC>::object_creator MemoryMgr<MemPool,GC>::create_mgr_;
	}
}

#define REGISTER_RELEASE_FUN(ClASSNAME)\
	namespace ma{\
	namespace core{\
	namespace{\
}\
}\
}

#endif