// $Id:   $

#ifndef MEMORYMGR_H
#define MEMORYMGR_H

#include <memory>

#include "NullType.h"
#include "HasMember.h"

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


#define MA_RELEASE_FUNCTION(CLASS_NAME_STR,CLASS_TYPE)											\
static bool MA_RELEASE_FUNCTION_##CLASS_NAME_STR()												\
{																								\
	typedef MostDerivedType<CLASS_TYPE >::type AllocType;										\
	typedef ::ma::core::MemoryPolicyType::MemoryPoolType MemPool;								\
	return MemPool::template SingletonPool<AllocType>::release_memory();						\
}

//this function is register a release all memory function for class CLASSNAME
//Place it in cpp

#define MA_REGISTER_RELEASE_FUN(ClASS_NAME_STR,CLASS_TYPE)												\
	namespace ma{																		\
	namespace core{																		\
	namespace{																			\
	HAS_MEMFUN(registerReleaseFunc)														\
	template<int>																		\
	struct MA_STATIC_OBJECT_TYPE_##ClASS_NAME_STR{};											\
	typedef bool (*FunPtr)();															\
																						\
	template<>																			\
	struct MA_STATIC_OBJECT_TYPE_##ClASS_NAME_STR											\
		<																				\
		true																				\
		>																				\
{																					\
		typedef MemoryPolicyType::MemoryPoolType MemPool;								\
		MA_RELEASE_FUNCTION(ClASS_NAME_STR,CLASS_TYPE)													\
		MA_STATIC_OBJECT_TYPE_##ClASS_NAME_STR(){											\
			MemPool::registerReleaseFunc(&MA_RELEASE_FUNCTION_##ClASS_NAME_STR);				\
		}																				\
	};																					\
	static MA_STATIC_OBJECT_TYPE_##ClASS_NAME_STR<has_memfun_registerReleaseFunc<MemoryPolicyType::MemoryPoolType,FunPtr>::value> \
		MA_STATIC_OBJECT_TYPE_##CLASSNAME_object;	\
}\
}\
}

#endif