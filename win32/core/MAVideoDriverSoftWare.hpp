#ifndef MA_VIDEODRIVER_SOFTWARE_HPP
#define MA_VIDEODRIVER_SOFTWARE_HPP

#include "VideoDriver.hpp"
#include "AddPointer.hpp"
namespace ma{
	template<typename Configure>
	class MAVideoDriverSoftWare:public VideoDriver<MAVideoDriverSoftWare<Configure>,Configure >{
		public:
		typedef typename Configure::Image Image;
		typedef typename Configure::Texture Texture;

		typedef typename Configure::ImagePtr ImagePtr;
		typedef typename Configure::Texture TexturePtr;

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
		template<typename Vertex_PTR> void drawIndexedTriangleList(const Vertex_PTR verts,unsigned int vert_cnt,const unsigned int* idx_list,unsigned int tri_cnt);
		template<typename Point_T> void draw3DLine(const Point_T& start,const Point_T& end,Color clr);
		template<typename Triangle_T> void draw3DTriangle(const Triangle_T& tri,Color clr);

		template<typename VertexShader,typename FragShader>
		void drawIndexTriangleBuffer(
			std::size_t vert_count,
			std::size_t tri_count,
			std::size_t vert_stride,
			const void* vertex_buffer,const unsigned* tri_index_buffer);

		ImagePtr getBackBuffer(){return BackBuffer;}
	private:
		ImagePtr BackBuffer;

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