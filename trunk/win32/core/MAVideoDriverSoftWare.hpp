#ifndef MA_VIDEODRIVER_SOFTWARE_HPP
#define MA_VIDEODRIVER_SOFTWARE_HPP

#include "VideoDriver.hpp"

namespace ma{
	template<typename Configure>
	class MAVideoDriverSoftWare:public VideoDriver<MAVideoDriverSoftWare<Configure>,Configure >{
	public:
		typedef typename Configure::Color Color;
		typedef typename Configure::FileSystem FileSystem;
		typedef typename Configure::FileSystemPtr FileSystemPtr;
	public:
		MAVideoDriverSoftWare(const scalar2i& windowSize,bool fullscreen, FileSystemPtr fs){}
		bool beginScene(bool backBuffer,bool zBuffer,Color clr){return false;}
		bool endScene(int windowid,recti* sourceRect){return false;}
		template<typename PresenterPtr>
		void present(PresenterPtr presenter){return ;}
		
	};




}
#endif