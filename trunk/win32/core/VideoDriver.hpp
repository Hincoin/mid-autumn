#ifndef INCLUDE_VIDEO_DRIVER_HPP
#define INCLUDE_VIDEO_DRIVER_HPP

#include "CRTPInterfaceMacro.hpp"
#include "SpaceSegment.hpp"

namespace ma{

	//! enumeration for geometry transformation states
	enum E_TRANSFORMATION_STATE
	{
		//! View transformation
		ETS_VIEW = 0,
		//! World transformation
		ETS_WORLD,
		//! Projection transformation
		ETS_PROJECTION,
		//! Texture transformation
		ETS_TEXTURE_0,
		//! Texture transformation
		ETS_TEXTURE_1,
		//! Texture transformation
		ETS_TEXTURE_2,
		//! Texture transformation
		ETS_TEXTURE_3,
		//! Not used
		ETS_COUNT
	};

	BEGIN_CRTP_INTERFACE(VideoDriver)
		ADD_CRTP_INTERFACE_TYPEDEF(Color)
		ADD_CRTP_INTERFACE_FUNC(bool,beginScene,(bool backBuffer,bool zBuffer,Color clr),(backBuffer,zBuffer,clr),)
		ADD_CRTP_INTERFACE_FUNC(template<typename PresenterPtr> bool,endScene,(PresenterPtr presenter,int windowid,recti* sourceRect),(presenter,windowid, sourceRect), )
		ADD_CRTP_INTERFACE_FUNC(template<typename Vertex_PTR> void,drawIndexedTriangleList,(const Vertex_PTR verts,unsigned int vert_cnt,const unsigned int* idx_list,unsigned int tri_cnt),(verts,vert_cnt,idx_list,tri_cnt),)
		ADD_CRTP_INTERFACE_FUNC(template<typename Point_T> void , draw3DLine,(const Point_T& start,const Point_T& end,Color clr),(start,end,clr),)
		ADD_CRTP_INTERFACE_FUNC(template<typename Triangle_T> void,draw3DTriangle,(const Triangle_T& tri,Color clr),(tri,clr),)
		//ADD_CRTP_INTERFACE_FUNC( template<typename VertexShader,typename FragShader> void, drawIndexTriangleBuffer,(size_t vert_count,size_t tri_count,size_t vert_stride,const void* vertex_buffer,const unsigned* tri_index_buffer),(vert_count,tri_count,vert_stride,vertex_buffer,tri_index_buffer), )
		//ADD_CRTP_TEMPLATE_INTERFACE_FUNC( ( typename VertexShader,typename FragShader ), void , drawIndexTriangleBuffer,(size_t vert_count,size_t tri_count,size_t vert_stride,const void* vertex_buffer,const unsigned* tri_index_buffer),(vert_count,tri_count,vert_stride,vertex_buffer,tri_index_buffer), )
		template<typename VertexShader,typename FragShader> void drawIndexTriangleBuffer(size_t vert_count,size_t tri_count,size_t vert_stride,const void* vertex_buffer,const unsigned* tri_index_buffer){ return derived().template drawIndexTriangleBuffer<VertexShader,FragShader>(vert_count,tri_count,vert_stride,vertex_buffer,tri_index_buffer);}
		END_CRTP_INTERFACE()


	template<typename CreateConfig>
	struct CreateDriver;
}
#endif
