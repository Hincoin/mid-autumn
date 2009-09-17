#ifndef __INCLUDE_MA_RASTERIZER_HPP__
#define __INCLUDE_MA_RASTERIZER_HPP__

#include "NullType.hpp"

namespace ma{

	//empty base
	template<typename ComponentType,int MaX_VARYING>
	struct MARasterizerBaseI{
		static const int MAX_VARYING =MaX_VARYING;

		// Type definitions
		struct Vertex {
			ComponentType x, y; // in 28.4 fixed point
			ComponentType z; // range from 0 to 0x7fffffff
			ComponentType w; // in 16.16 fixed point
			ComponentType inv_w; //can share with w but sepearate them can be more readable
			//float varyings[MAX_VARYING];
			ComponentType perspective_varyings[MAX_VARYING];//such as texture coordinate
			ComponentType linear_varyings[MAX_VARYING];//such as z,color
			static unsigned persp_var_cnt;
			static unsigned linear_var_cnt;
		};

		// This is the data necessary for each fragment. It is defined here
		// as probably all rasterizers will need this.
		struct FragmentData {
ComponentType z;
			//float varyings[MAX_VARYING];
			ComponentType perspective_varyings[MAX_VARYING];
			ComponentType linear_varyings[MAX_VARYING];
		};

		// Use for perspective spans. Defined here for convenience
		struct FragmentDataPerspective {
			int oow;
			FragmentData fd;
		};
	protected:
		MARasterizerBaseI(){}
		~MARasterizerBaseI(){}
	};
	template<typename ComponentType,int MaxVarying>
	unsigned MARasterizerBaseI<ComponentType,MaxVarying>::Vertex::persp_var_cnt=0;
	template<typename ComponentType,int MaxVarying>
	unsigned MARasterizerBaseI<ComponentType,MaxVarying>::Vertex::linear_var_cnt = 0;

    typedef MARasterizerBaseI<float,8> MARasterizerBase;

	template<typename Derived, typename Configure=EmptyType>
	class MARasterizer:public MARasterizerBase{
		Derived& derived(){return static_cast<Derived&>(*this);}
	public:
		void clip_rect(int x, int y, int w, int h){derived().clip_rect(x,y,w,h);}
		void drawTriangle(const Vertex &v1, const Vertex &v2, const Vertex &v3) {
			derived().drawTriangleImpl(v1,v2,v3);
		}
		void drawLine(const Vertex &v1, const Vertex &v2) {derived().drawLine(v1,v2);}
		void drawPoint(const Vertex &v1) {derived().drawPoint(v1);}
	};


	// Base rasterizer implementation for all rasterizers where the shader
	// can be set via a template function. Here only line and point functions
	// will be implemented. The triangle drawing function is implemented by
	// specialized shaders.
	template<typename Derived>
	class RasterizerTemplateShaderBase : public MARasterizer<RasterizerTemplateShaderBase<Derived> > {
	protected:
		struct { int x0, y0, x1, y1; } clip_rect_;
		struct { int offset; int mask; } interlace_;
		//unsigned varying_count_;

        typedef MARasterizer<RasterizerTemplateShaderBase<Derived> > Base;
		friend class MARasterizer<RasterizerTemplateShaderBase<Derived> >;


	public:
		typedef typename Base::Vertex Vertex;

		RasterizerTemplateShaderBase()
		{
			clip_rect(0, 0, 0, 0);
			interlace(0, 0);
		}



		//unsigned varying_count()
		//{ return varying_count_; }

		// set the fragment shader
		template <typename FragSpan>
		void fragment_shader()
		{
			//varying_count_ = FragSpan::varying_count;
			line_func_ = &RasterizerTemplateShaderBase<Derived>::template line_template<FragSpan>;
			point_func_ = &RasterizerTemplateShaderBase<Derived>::template point_template<FragSpan>;
		}



		// Controls interlacing. Uses the following formula to determine if a
		// row should be drawn:
		//
		// bool drawit = ((y + offset) & mask) == 0
		//
		// Thus to draw every line you can pass (0,0). To draw every line with
		// even y you pass (0,1). For odd lines you pass (1,1).
		//
		// This should give enough control. More control could only be achived
		// by doing (y + offset) % some_value == 0 which requires an expensive
		// mod.
		void interlace(int offset, int mask)
		{
			interlace_.offset = offset;
			interlace_.mask = mask;
		}

	protected:
		inline bool clip_test(int x, int y)
		{
			return (x >= clip_rect_.x0 && x < clip_rect_.x1 &&
				y >= clip_rect_.y0 && y < clip_rect_.y1);
		}

		inline bool ilace_drawit(int y)
		{
			return ((y + interlace_.offset) & interlace_.mask) == 0;
		}

	private:
		void (RasterizerTemplateShaderBase::*line_func_)(const Vertex &v1, const Vertex &v2);
		void (RasterizerTemplateShaderBase::*point_func_)(const Vertex &v1);

	private:
		template <typename FragSpan>
		void line_template(const Vertex &v1, const Vertex &v2)
		{
			//using namespace detail;

			//int x0 = v1.x >> 4;
			//int y0 = v1.y >> 4;

			//int x1 = v2.x >> 4;
			//int y1 = v2.y >> 4;

			//int dx = x1 - x0;
			//int dy = y1 - y0;

			//int adx = detail::abs(dx);
			//int ady = detail::abs(dy);

			//if (dx == 0 && dy == 0)
			//	return;

			//FragmentData fragment_data;

			//struct step_info {
			//	int step;
			//	int remainder;
			//	int error_term;

			//	void init(int start, int end, int delta)
			//	{
			//		floor_divmod(end-start, delta, &step, &remainder);
			//		error_term = 0;
			//	}

			//	int do_step(int absdelta)
			//	{
			//		int r = step;
			//		error_term += remainder;
			//		if (error_term >= absdelta) {
			//			error_term -= absdelta;
			//			r++;
			//		}
			//		return r;
			//	}
			//} step_z, step_v[MAX_VARYING];

			//if (adx > ady) {
			//	int xstep = dx > 0 ? 1 : -1;
			//	//int ystep_extra = dy > 0 ? 1 : -1;
			//	int ystep, remainder, error_term = 0;
			//	floor_divmod(dy, adx, &ystep, &remainder);

			//	if (FragSpan::interpolate_z) {
			//		fragment_data.z = v1.z;
			//		step_z.init(v1.z, v2.z, adx);
			//	}
			//	for (unsigned i = 0; i < FragSpan::varying_count; ++i) {
			//		fragment_data.varyings[i] = v1.varyings[i];
			//		step_v[i].init(v1.varyings[i], v2.varyings[i], adx);
			//	}

			//	int x = x0;
			//	int y = y0;
			//	while (x != x1) {
			//		x += xstep;
			//		y += ystep;
			//		error_term += remainder;
			//		if (error_term >= adx) {
			//			error_term -= adx;
			//			y++;
			//		}

			//		if (FragSpan::interpolate_z)
			//			fragment_data.z += step_z.do_step(adx);

			//		for (unsigned i = 0; i < FragSpan::varying_count; ++i)
			//			fragment_data.varyings[i] += step_v[i].do_step(adx);

			//		if (ilace_drawit(y) && clip_test(x, y))
			//			FragSpan::affine_span(x, y, fragment_data, fragment_data, 1);
			//	}
			//}
			//else {
			//	int ystep = dy > 0 ? 1 : -1;
			//	//int xstep_extra = dx > 0 ? 1 : -1;
			//	int xstep, remainder, error_term = 0;
			//	floor_divmod(dx, ady, &xstep, &remainder);

			//	if (FragSpan::interpolate_z) {
			//		fragment_data.z = v1.z;
			//		step_z.init(v1.z, v2.z, ady);
			//	}
			//	for (unsigned i = 0; i < FragSpan::varying_count; ++i) {
			//		fragment_data.varyings[i] = v1.varyings[i];
			//		step_v[i].init(v1.varyings[i], v2.varyings[i], ady);
			//	}

			//	int x = x0;
			//	int y = y0;
			//	while (y != y1) {
			//		y += ystep;
			//		x += xstep;
			//		error_term += remainder;
			//		if (error_term >= ady) {
			//			error_term -= ady;
			//			x++;
			//		}

			//		if (FragSpan::interpolate_z)
			//			fragment_data.z += step_z.do_step(adx);

			//		for (unsigned i = 0; i < FragSpan::varying_count; ++i)
			//			fragment_data.varyings[i] += step_v[i].do_step(adx);

			//		if (ilace_drawit(y) && clip_test(x, y))
			//			FragSpan::affine_span(x, y, fragment_data, fragment_data, 1);
			//	}
			//}
		}

		template <typename FragSpan>
		void point_template(const Vertex &v1)
		{
			//FragmentData fd;

			//int x = v1.x >> 4;
			//int y = v1.y >> 4;

			//if (!clip_test(x, y) || !ilace_drawit(y))
			//	return;

			//if (FragSpan::interpolate_z)
			//	fd.z = v1.z;

			//for (unsigned i = 0; i < FragSpan::varying_count; ++i)
			//	fd.varyings[i] = v1.varyings[i];

			//FragSpan::affine_span(x, y, fd, fd, 1);
		}
		public:
			void draw_line(const Vertex &v1, const Vertex &v2)
			{
				if (line_func_)
					(this->*line_func_)(v1, v2);
			}

			void draw_point(const Vertex &v1)
			{
				if (point_func_)
					(this->*point_func_)(v1);
			}
			void drawTriangleImpl(const Vertex &v1, const Vertex &v2, const Vertex &v3){
				return static_cast<Derived&>(*this).drawTriangleImpl(v1,v2,v3);
			}

			// upper left is (0,0)
			void clip_rect(int x, int y, int w, int h)
			{
				clip_rect_.x0 = std::max(0, x);
				clip_rect_.y0 = std::max(0, y);
				clip_rect_.x1 = std::max(0, x + w);
				clip_rect_.y1 = std::max(0, y + h);
			}
	};
}
#endif
