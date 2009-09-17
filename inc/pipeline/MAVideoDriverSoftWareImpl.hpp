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
		depth_buffer_->clear(std::numeric_limits<float>::max());
		stencil_buffer_ = new StencilBuffer(windowSize);
		stencil_buffer_->clear(0);
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

	//attribute start index is the attribute start position in buffer
	//vertex stride is that the size of the attribute
	template<typename Configure>
    template<typename VertexShader>
    void MAVideoDriverSoftWare<Configure>::vertexBuffer(unsigned attribute_start_idx,unsigned vert_stride,void* buffer)
    {
		geometry_renderer_->template vertex_shader<VertexShader>();
		geometry_renderer_->vertex_attrib_pointer(attribute_start_idx,vert_stride,buffer);
    }
	template<typename Configure>
    template<typename FragShader>
    void MAVideoDriverSoftWare<Configure>::drawIndexedTriangle(unsigned tri_count ,const unsigned *tri_idx){
		geometry_renderer_->template fragment_shader<FragShader>();
		geometry_renderer_->draw_triangles(tri_count, tri_idx);
        }
	template<typename Configure>
	void MAVideoDriverSoftWare<Configure>::viewport(int left,int top,int right,int bottom){
		rect_op::left(ViewPort) = left;
		rect_op::top(ViewPort) = top;
		rect_op::right(ViewPort) = right;
		rect_op::bottom(ViewPort) = bottom;
		geometry_renderer_->viewport(left,top,right-left,bottom-top );
		geometry_renderer_->rasterizer()->clip_rect(left,top,right-left,bottom - top);
	}
	//template<typename Configure>
	//template<typename VertexShader,typename FragShader>
	//void MAVideoDriverSoftWare<Configure>::drawIndexTriangleBuffer(unsigned vert_count,unsigned tri_count,unsigned vert_stride,const void* vertex_buffer,const unsigned* tri_index_buffer)
	//{
	//	
	//}
}


#endif
