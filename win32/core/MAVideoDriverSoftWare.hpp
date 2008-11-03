#ifndef MA_VIDEODRIVER_SOFTWARE_HPP
#define MA_VIDEODRIVER_SOFTWARE_HPP

#include "VideoDriver.hpp"
#include "AddPointer.hpp"
namespace ma{
	template<typename Configure>
	class MAVideoDriverSoftWare:public VideoDriver<MAVideoDriverSoftWare<Configure>,Configure >{
		typedef typename Configure::Image Image;
		typedef typename Configure::Texture Texture;

		typedef typename Configure::ImagePtr ImagePtr;
		typedef typename Configure::Texture TexturePtr;
	public:
		typedef typename Configure::Color Color;
		typedef typename Configure::FileSystem FileSystem;
		typedef typename Configure::FileSystemPtr FileSystemPtr;
	public:
		MAVideoDriverSoftWare(const scalar2i& windowSize,bool fullscreen, FileSystemPtr fs);
		~MAVideoDriverSoftWare();
		bool beginScene(bool backBuffer,bool zBuffer,Color clr){
			if (backBuffer)
				BackBuffer->fill( clr );

			//if (DepthBuffer && zBuffer)
			//	DepthBuffer->clear();

			return true;
		}
		template<typename PresenterPtr>
		bool endScene(PresenterPtr presenter,int windowid,recti* sourceRect)
		{
			presenter->present(BackBuffer,windowid,sourceRect);
			return false;
		}
	private:
		ImagePtr BackBuffer;

		TexturePtr RenderTargetTexture;
		ImagePtr RenderTargetSurface;
		scalar2i RenderTargetSize;
		scalar2i ScreenSize;
		recti ViewPort;

		FileSystemPtr FileSystem_;
	};
}

#include "MAVideoDriverSoftWareImpl.hpp"

#endif