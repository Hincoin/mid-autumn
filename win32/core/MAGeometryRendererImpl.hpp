#ifndef __INCLUDE_MA_GEOMETRY_RENDERER_IMPL_HPP__
#define __INCLUDE_MA_GEOMETRY_RENDERER_IMPL_HPP__

#include <cassert>

#include "MAGeometryRenderer.hpp"
namespace ma
{


template<typename Rasterizer>
MAGeometryRenderer<Rasterizer>::MAGeometryRenderer(const boost::shared_ptr<Rasterizer>& rasterizer)
:rasterizer_(rasterizer)
{
}

template<typename Rasterizer>
typename MAGeometryRenderer<Rasterizer>::VertexOutput*
MAGeometryRenderer<Rasterizer>::acquire_output_location()
{
	vertices_.resize(vertices_.size() + 1);
	return &vertices_.back();
}


// public interface

// upper left is (0,0)
template<typename Rasterizer>
void MAGeometryRenderer<Rasterizer>::
viewport(int x, int y, int w, int h)
{
}

// the depth range to use. Normally from 0 to a value less than MAX_INT
template<typename Rasterizer>
void MAGeometryRenderer<Rasterizer>::
depth_range(int n, int f){}

template<typename Rasterizer>
void MAGeometryRenderer<Rasterizer>::
vertex_attrib_pointer(int n, int stride, const void* buffer){}

// count gives the number of indices
template<typename Rasterizer>
void MAGeometryRenderer<Rasterizer>::
draw_triangles(unsigned count,const unsigned *indices)
{
	draw_mode_ = DM_TRIANGLES;
	Base::process(count, indices);
}
template<typename Rasterizer>
void MAGeometryRenderer<Rasterizer>::
draw_lines(unsigned count,const unsigned *indices){assert(false);}
template<typename Rasterizer>
void MAGeometryRenderer<Rasterizer>::
draw_points(unsigned count,const unsigned *indices){assert(false);}

template<typename Rasterizer>
void MAGeometryRenderer<Rasterizer>::
cull_mode(CullMode m){;}

//template<typename Rasterizer>
//template <typename VertexShader>
//void MAGeometryRenderer<Rasterizer>::vertex_shader()
//{
//	Base::vertex_shader<VertexShader>();
//	varying_count_ = &varying_count_template<VertexShader>;
//}

//template<typename Rasterizer>
//template <typename VertexShader>
//unsigned MAGeometryRenderer<Rasterizer>::varying_count_template()
//{
//	return VertexShader::varying_count;
//}
template<typename Rasterizer>
void MAGeometryRenderer<Rasterizer>::add_interp_vertex(int t, int out, int in){assert(false);}

template<typename Rasterizer>
void MAGeometryRenderer<Rasterizer>::pdiv_and_vt(){assert(false);}

template<typename Rasterizer>
void MAGeometryRenderer<Rasterizer>::clip_triangles(){assert(false);}
template<typename Rasterizer>
void MAGeometryRenderer<Rasterizer>::process_triangles(){

	//clip_triangles();
	//pdiv_and_vt();

	// compute facing and possibly cull and then draw the triangles		
	for (size_t i = 0; i + 3 <= indices_.size(); i += 3) {
		// don't process triangles which are marked as unused by clipping
		if (indices_[i] == SKIP_FLAG) continue;

		VertexOutput &v0 = vertices_[indices_[i]];
		VertexOutput &v1 = vertices_[indices_[i + 1]];
		VertexOutput &v2 = vertices_[indices_[i + 2]];

		// here x and y are in 28.4 fixed point. I don't use the fixmul<4>
		// here since these coordinates are clipped to the viewport and
		// therefore are sufficiently small to not overflow.
		int facing = (v0.x-v1.x)*(v2.y-v1.y)-(v2.x-v1.x)*(v0.y-v1.y);
		if (facing > 0) {
			if (cull_mode_ != CULL_CCW) 
				rasterizer_->drawTriangle(v0, v1, v2);
		}
		else {
			if (cull_mode_ != CULL_CW) 
				rasterizer_->drawTriangle(v2, v1, v0);
		}
	}

	vertices_.clear();
	indices_.clear();
}

template<typename Rasterizer>
void MAGeometryRenderer<Rasterizer>::clip_lines(){assert(false);}
template<typename Rasterizer>
void MAGeometryRenderer<Rasterizer>::process_lines(){assert(false);}

template<typename Rasterizer>
void MAGeometryRenderer<Rasterizer>::clip_points(){assert(false);}
template<typename Rasterizer>
void MAGeometryRenderer<Rasterizer>::process_points(){assert(false);}


// interface inherited from the vertex processor
template<typename Rasterizer>
void MAGeometryRenderer<Rasterizer>::process_begin()
{
	//do nothing here
	//assert(false);
}
//template<typename Rasterizer>
//typename MAGeometryRenderer<Rasterizer>::VertexOutput* MAGeometryRenderer<Rasterizer>::acquire_output_location()
//{;}
template<typename Rasterizer>
bool MAGeometryRenderer<Rasterizer>::push_vertex_index(unsigned i)
{
	indices_.push_back(i);

	switch (draw_mode_) {
	case DM_TRIANGLES:
		if (indices_.size() >= MAX_TRIANGLES * 3) {
			process_triangles();
			return true;
		}
		break;
	case DM_LINES:
		if (indices_.size() >= MAX_LINES * 2) {
			process_lines();
			return true;
		}
		break;
	case DM_POINTS:
		if (indices_.size() >= MAX_POINTS) {
			process_points();
			return true;
		}
		break;
	}

	return false;
}
template<typename Rasterizer>
void MAGeometryRenderer<Rasterizer>::process_end()
{
	switch (draw_mode_)
	{
	case DM_TRIANGLES: process_triangles();break;
	case DM_LINES: process_lines(); break;
	case DM_POINTS: process_points(); break;
	default:assert(false);
	}
}

}

#endif