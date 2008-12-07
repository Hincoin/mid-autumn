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
	// don't use the full positive range (0x7fffffff) because it
	// could be possible to get overflows at the far plane.
	//depth_range(0, 0x3fffffff);
	depth_range(0, 1.f);
	cull_mode_ = CULL_CW;
	viewport_.ox = viewport_.oy = viewport_.px = viewport_.py = 0;
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
	viewport_.px = w / 2;
	viewport_.py = h / 2;

	// the origin is stored in fixed point so it does not need to be
	// converted later.
	viewport_.ox = (x + viewport_.px) ;
	viewport_.oy = (y + viewport_.py) ;
}

// the depth range to use. Normally from 0 to a value less than MAX_INT
template<typename Rasterizer>
void MAGeometryRenderer<Rasterizer>::
depth_range(int n, int f){ depth_range_.near = n;depth_range_.far=f;}

template<typename Rasterizer>
void MAGeometryRenderer<Rasterizer>::
vertex_attrib_pointer(int n, int stride, const void* buffer){
	Base::vertex_attrib_pointer(n, stride, buffer);
}

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
void MAGeometryRenderer<Rasterizer>::add_interp_vertex(float t, int out, int in){
    	//#define LINTERP(T, OUT, IN) (OUT) + fixmul<24>((IN) - (OUT), (T))
    	//#define LINTERP(T, OUT, IN) (OUT) + ((IN) - (OUT) * (T))

    assert(t>=0.f && t<= 1.f);
	vertices_.resize(vertices_.size() + 1);
	VertexOutput& v = vertices_.back();
	VertexOutput& a = vertices_[out];
	VertexOutput& b = vertices_[in];

	v.x = lerp(a.x,b.x,t);//LINTERP(t, a.x, b.x);
	v.y = lerp(a.y,b.y,t);//LINTERP(t, a.y, b.y);
	v.z = lerp(a.z,b.z,t);//LINTERP(t, a.z, b.z);
	v.w = lerp(a.w,b.w,t);//LINTERP(t, a.w, b.w);

	for (unsigned i = 0, n = varying_count_(); i < n; ++i)
		v.varyings[i] = lerp(a.varyings[i],b.varyings[i],t);//LINTERP(t, a.varyings[i], b.varyings[i]);

	//#undef LINTERP

    }


// perspective divide and viewport transform of vertices
template<typename Rasterizer>
void MAGeometryRenderer<Rasterizer>::pdiv_and_vt(){

	details::static_vector<bool, MAX_VERTICES_INDICES> already_processed ;
	already_processed.resize(vertices_.size(), false);

	for (size_t i = 0; i < indices_.size(); ++i) {
		// don't process triangles which are marked as unused by clipping
		if (indices_[i] == SKIP_FLAG) continue;

		if (!already_processed[indices_[i]]) {
			// perspective divide
			VertexOutput &v = vertices_[indices_[i]];

			v.x/=v.w;
			v.y/=v.w;
			v.z/=v.w;
			// triangle setup (x and y are converted from 16.16 to 28.4)
			v.x = (viewport_.px * v.x + viewport_.ox); /*>> 12;*/

			// y needs to be flipped since the viewport has (0,0) in the
			// upper left but vs output is like in OpenGL
			v.y = (viewport_.py * -v.y + viewport_.oy);

			//Ü³ËûÂè¸ö±ÆµÄ windef.h
#ifdef near
#undef near
#endif
#ifdef far
#undef far
#endif
			v.z = ((1 + v.z)*depth_range_.far+(1-v.z)*depth_range_.near)/2;

			already_processed[indices_[i]] = true;
		}
	}
}


#define DIFFERENT_SIGNS(x,y) ((x <= 0 && y > 0) || (x > 0 && y <= 0))

#define CLIP_DOTPROD(I, A, B, C, D) \
	(vertices_[I].x * A + vertices_[I].y * B + vertices_[I].z * C + vertices_[I].w * D)

#define POLY_CLIP( PLANE_BIT, A, B, C, D ) \
{  \
	if (mask & PLANE_BIT) {  \
		int idxPrev = inlist[0];  \
		float dpPrev = CLIP_DOTPROD(idxPrev, A, B, C, D );  \
		int outcount = 0;  \
		int i;  \
  \
		inlist[n] = inlist[0];  \
		for (i = 1; i <= n; i++) { 		  \
			int idx = inlist[i]; \
			float dp = CLIP_DOTPROD(idx, A, B, C, D ); \
			if (dpPrev >= 0) {  \
				outlist[outcount++] = idxPrev;  \
			}  \
   \
			if (DIFFERENT_SIGNS(dp, dpPrev)) {				   \
				if (dp < 0) {					   \
					/*int t = fixdiv<24>(dp,(dp - dpPrev));				   */ \
					float t = dp/(dp-dpPrev);/*fixmul<8>(dp, invert(dp - dpPrev));*/ \
					add_interp_vertex(t, idx, idxPrev);   \
				} else {							   \
					/*int t = fixdiv<24>(dpPrev,(dpPrev - dp));			   */\
					float t = dpPrev/(dpPrev-dp);/*fixmul<8>(dpPrev, invert(dpPrev - dp));*/ \
					add_interp_vertex(t, idxPrev, idx);   \
				}								   \
				outlist[outcount++] = (int)(vertices_.size() - 1); \
			}								   \
   \
			idxPrev = idx;							   \
			dpPrev = dp;							   \
		}									   \
   \
		if (outcount < 3)							   \
			continue;							   \
   \
	 	{									   \
			int *tmp = inlist;				  	 \
			inlist = outlist;				  	 \
			outlist = tmp;					  	 \
			n = outcount;					  	 \
		}									   \
	}									   \
}

#define LINE_CLIP(PLANE_BIT, A, B, C, D ) \
{ \
	if (mask & PLANE_BIT) { \
		const float dp0 = CLIP_DOTPROD( v0, A, B, C, D ); \
		const float dp1 = CLIP_DOTPROD( v1, A, B, C, D ); \
		const bool neg_dp0 = dp0 < 0; \
		const bool neg_dp1 = dp1 < 0; \
\
		if (neg_dp0 && neg_dp1) \
			continue; \
\
		if (neg_dp1) { \
			/*int t = fixdiv<24>(dp1, (dp1 - dp0));*/ \
			float t = dp1 / (dp1-dp0);/*fixmul<8>(dp1, invert(dp1 - dp0));*/ \
			if (t > t1) t1 = t; \
		} else if (neg_dp0) { \
			/*int t = fixdiv<24>(dp0, (dp0 - dp1));*/ \
			float t = dp0 / (dp0 - dp1);/*fixmul<8>(dp0, invert(dp0 - dp1));*/ \
			if (t > t0) t0 = t; \
		} \
		if (t0 + t1 >= 1) \
			continue; \
	} \
}

enum {
	CLIP_POS_X_BIT = 0x01,
	CLIP_NEG_X_BIT = 0x02,
	CLIP_POS_Y_BIT = 0x04,
	CLIP_NEG_Y_BIT = 0x08,
	CLIP_POS_Z_BIT = 0x10,
	CLIP_NEG_Z_BIT = 0x20
};

template<typename GeometryRenderer>
inline int calc_clip_mask(const typename GeometryRenderer::VertexOutput& v)
{
	int cmask = 0;
	if (v.w - v.x < 0) cmask |= CLIP_POS_X_BIT;
	if (v.x + v.w < 0) cmask |= CLIP_NEG_X_BIT;
	if (v.w - v.y < 0) cmask |= CLIP_POS_Y_BIT;
	if (v.y + v.w < 0) cmask |= CLIP_NEG_Y_BIT;
	if (v.w - v.z < 0) cmask |= CLIP_POS_Z_BIT;
	if (v.z + v.w < 0) cmask |= CLIP_NEG_Z_BIT;
	return cmask;
}

template<typename Rasterizer>
void MAGeometryRenderer<Rasterizer>::clip_triangles(){
      int mask = 0;
      for(size_t i = 0; i < vertices_.size(); ++i)
      mask |= calc_clip_mask<MAGeometryRenderer<Rasterizer> >(vertices_[i]);
      if( mask != 0)
      {
          for (size_t idx = 0, count = indices_.size(); idx + 3 <= count; idx += 3) {
			int vlist[2][2*6+1];
			int *inlist = vlist[0], *outlist = vlist[1];
			int n = 3;

			inlist[0] = indices_[idx];
			inlist[1] = indices_[idx + 1];
			inlist[2] = indices_[idx + 2];

			// mark this triangle as unused in case it should be completely
			// clipped
			indices_[idx] = SKIP_FLAG;
			indices_[idx + 1] = SKIP_FLAG;
			indices_[idx + 2] = SKIP_FLAG;

			POLY_CLIP(CLIP_POS_X_BIT, -1,  0,  0, 1);
			POLY_CLIP(CLIP_NEG_X_BIT,  1,  0,  0, 1);
			POLY_CLIP(CLIP_POS_Y_BIT,  0, -1,  0, 1);
			POLY_CLIP(CLIP_NEG_Y_BIT,  0,  1,  0, 1);
			POLY_CLIP(CLIP_POS_Z_BIT,  0,  0, -1, 1);
			POLY_CLIP(CLIP_NEG_Z_BIT,  0,  0,  1, 1);

			// transform the poly in inlist into triangles
			indices_[idx] = inlist[0];
			indices_[idx + 1] = inlist[1];
			indices_[idx + 2] = inlist[2];
			for (int i = 3; i < n; ++i) {
				indices_.push_back(inlist[0]);
				indices_.push_back(inlist[i - 1]);
				indices_.push_back(inlist[i]);
			}
		}
       }
    }
template<typename Rasterizer>
void MAGeometryRenderer<Rasterizer>::process_triangles(){

	clip_triangles();
	pdiv_and_vt();

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
