#ifndef MA_VIDEODRIVER_SOFTWARE_HPP
#define MA_VIDEODRIVER_SOFTWARE_HPP

#include "VideoDriver.hpp"
#include "AddPointer.hpp"
#ifdef max
#undef max
#endif
#ifdef min
#undef min
#endif

namespace ma{
	template<typename Configure>
	class MAVideoDriverSoftWare:public VideoDriver<MAVideoDriverSoftWare<Configure>,Configure >{
		public:
		typedef typename Configure::Image Image;
		typedef typename Configure::Texture Texture;

		typedef typename Configure::ImagePtr ImagePtr;
		typedef typename Configure::Texture TexturePtr;
		typedef typename Configure::DepthBuffer DepthBuffer;
		typedef typename Configure::DepthBufferPtr DepthBufferPtr ;
		typedef typename Configure::StencilBuffer StencilBuffer;
		typedef typename Configure::StencilBufferPtr StencilBufferPtr;

		typedef typename Configure::GeometryRenderer GeometryRenderer;
		typedef typename Configure::GeometryRendererPtr GeometryRendererPtr;

		typedef typename Configure::Rasterizer Rasterizer;
		typedef typename Configure::RasterizerPtr RasterizerPtr;

		typedef typename Configure::Color Color;
		typedef typename Configure::FileSystem FileSystem;
		typedef typename Configure::FileSystemPtr FileSystemPtr;
	public:
		MAVideoDriverSoftWare(const scalar2i& windowSize,bool fullscreen, FileSystemPtr fs);
		~MAVideoDriverSoftWare();
		bool beginScene(bool backBuffer,bool zBuffer,Color clr){
			clearFPUException();

			if (backBuffer)
				BackBuffer->fill( clr );

			if (depth_buffer_ && zBuffer)
				depth_buffer_->clear(std::numeric_limits<float>::max());
            if (stencil_buffer_)
                stencil_buffer_->clear(0);
			return true;
		}
		template<typename PresenterPtr>
		bool endScene(PresenterPtr presenter,int windowid,recti* sourceRect)
		{
			presenter->present(BackBuffer,windowid,sourceRect);
			return false;
		}
		template<typename Vertex_PTR> void drawIndexedTriangleList(const Vertex_PTR verts,unsigned int vert_cnt,const unsigned int* idx_list,unsigned int tri_cnt);
		template<typename Point_T> void draw3DLine(const Point_T& start,const Point_T& end,Color clr);
		template<typename Triangle_T> void draw3DTriangle(const Triangle_T& tri,Color clr);

		ImagePtr getBackBuffer(){return BackBuffer;}
		DepthBufferPtr getDepthBuffer(){return depth_buffer_;}
		StencilBufferPtr getStencilBuffer(){return stencil_buffer_;}
		void viewport(int left,int top,int right,int bottom);
		template<typename VertexShader>
		void vertexBuffer(unsigned attribute_start_idx,unsigned vert_stride,void* buffer);
		template<typename FragShader>
		void drawIndexedTriangle(unsigned tri_count ,const unsigned *tri_idx);
	private:
		ImagePtr BackBuffer;
		DepthBufferPtr depth_buffer_;
		StencilBufferPtr stencil_buffer_;

		TexturePtr RenderTargetTexture;
		ImagePtr RenderTargetSurface;
		scalar2i RenderTargetSize;
		scalar2i ScreenSize;
		recti ViewPort;
		matrix44f TransformationMatrices[ETS_COUNT];

		FileSystemPtr FileSystem_;

		GeometryRendererPtr geometry_renderer_;

	};
}

#include "MAVideoDriverSoftWareImpl.hpp"

#endif
