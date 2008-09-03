// $Id:   $

#ifndef MEMORYMGR_H
#define MEMORYMGR_H

#include <memory>
namespace ma{
	namespace core{

		template<class MemPool,class GC>
		class MemoryMgr:public MemPool,public GC
		{
		public:
			MemoryMgr& getInstance(){
				if (!mem_mgr_)
				{
					mem_mgr_ = new MemoryMgr;
				}
				return *mem_mgr_;
			}
		private:
			static std::auto_ptr<MemoryMgr> mem_mgr_;
		private:
			MemoryMgr(){}
			MemoryMgr(const MemoryMgr&);
			MemoryMgr& operator=(const MemoryMgr&);
		};
		template<class MemPool,class GC>
		std::auto_ptr<MemoryMgr<MemPool,GC> > MemoryMgr<MemPool,GC>::mem_mgr_;
	}
}


#endif