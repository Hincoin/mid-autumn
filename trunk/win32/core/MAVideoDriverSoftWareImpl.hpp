#ifndef MA_VIDEODRIVER_SOFTWARE_IMPL_HPP
#define MA_VIDEODRIVER_SOFTWARE_IMPL_HPP

#include "MAVideoDriverSoftWare.hpp"
#include "Image.hpp"
namespace ma{
	template<typename Configure>
	MAVideoDriverSoftWare<Configure>::MAVideoDriverSoftWare
		(const scalar2i& windowSize,bool fullscreen, FileSystemPtr fs):
	ScreenSize(windowSize),FileSystem_(fs),geometry_renderer_(new GeometryRenderer(RasterizerPtr(new Rasterizer)))
	{
		BackBuffer = new Image(ECF_A8R8G8B8, windowSize);
		BackBuffer->fill(Color(0));
		depth_buffer_ = new DepthBuffer(windowSize);  //32 bit depth buffer
		depth_buffer_->clear();
	}
	template<typename Configure>
	MAVideoDriverSoftWare<Configure>::~MAVideoDriverSoftWare()
	{
		//delete pointer free memory
		delete BackBuffer;
		delete depth_buffer_;
	}


	template<typename Configure>
	template<typename Vertex_PTR>
	void MAVideoDriverSoftWare<Configure>::drawIndexedTriangleList(const Vertex_PTR verts,unsigned int vert_cnt,const unsigned int* idx_list,unsigned int tri_cnt)
	{
				//
	}


	template<typename Configure>
	template<typename Point_T>
	void MAVideoDriverSoftWare<Configure>::draw3DLine(const Point_T& start,const Point_T& end,Color clr)
	{

	}


	template<typename Configure>
		template<typename Triangle_T>
	void MAVideoDriverSoftWare<Configure>::draw3DTriangle(const Triangle_T& tri,Color clr)
	{

	}


	template<typename Configure>
	template<typename VertexShader,typename FragShader>
	void MAVideoDriverSoftWare<Configure>::drawIndexTriangleBuffer(std::size_t vert_count,std::size_t tri_count,std::size_t vert_stride,const void* vertex_buffer,const unsigned* tri_index_buffer)
	{
		//geometry_renderer_->viewport(0,0,BackBuffer->getWidth(),BackBuffer->getHeight());
		geometry_renderer_->viewport(0,0,320,240);
		geometry_renderer_->rasterizer()->clip_rect(0, 0, 320, 240);
		geometry_renderer_->template vertex_shader<VertexShader>();
		geometry_renderer_->template fragment_shader<FragShader>();
		geometry_renderer_->vertex_attrib_pointer(0,vert_stride,vertex_buffer);
		geometry_renderer_->draw_triangles(tri_count, tri_index_buffer);
	}
}


#endif
