#ifndef __INCLUDE_MA_AFFINE_RASTERIZER_HPP__
#define __INCLUDE_MA_AFFINE_RASTERIZER_HPP__

#include "MARasterizer.hpp"

#include "MAMath.hpp"

#include <omp.h>
namespace ma
{
    class MAAffineRasterizer:
                public RasterizerTemplateShaderBase<MAAffineRasterizer>
    {
        friend class RasterizerTemplateShaderBase<MAAffineRasterizer>;
        typedef RasterizerTemplateShaderBase<MAAffineRasterizer> Base;
    public:
        // constructor
        MAAffineRasterizer() :
                triangle_func_(0),
                perspective_correction_(true)
        {
            perspective_threshold(0, 0);
        }

    protected:
        void drawTriangleImpl(const Vertex &v1, const Vertex &v2, const Vertex &v3)
        {
            if (triangle_func_)
                (this->*triangle_func_)(v1, v2, v3);
        }
    public:
        // main interface

        void perspective_correction(bool enable)
        {
            perspective_correction_ = enable;
        }

        void perspective_threshold(int w, int h)
        {
            perspective_threshold_.w = w;
            perspective_threshold_.h = h;
        }

        // set the fragment shader
        template <typename FragSpan>
        void fragment_shader()
        {
            Base::template fragment_shader<FragSpan>();
            triangle_func_ = &MAAffineRasterizer::template triangle_template<FragSpan>;
        }



    private:
        void (MAAffineRasterizer::*triangle_func_)(const Vertex &v1, const Vertex &v2,
                const Vertex &v3);

        bool perspective_correction_;

        struct
        {
            int w;
            int h;
        } perspective_threshold_;

        // The triangle must be counter clockwise in screen space in order to be
        // drawn.
        template <typename FragSpan>
        void triangle_template_fixed_point(const Vertex &v1, const Vertex &v2, const Vertex &v3)
        {
            // 28.4 fixed-point coordinates
            const int Y1 = round32(16.0f * v1.y);
            const int Y2 = round32(16.0f * v2.y);
            const int Y3 = round32(16.0f * v3.y);

            const int X1 = round32(16.0f * v1.x);
            const int X2 = round32(16.0f * v2.x);
            const int X3 = round32(16.0f * v3.x);


            // Deltas
            const int DX12 = X1 - X2;
            const int DX23 = X2 - X3;
            const int DX31 = X3 - X1;

            const int DY12 = Y1 - Y2;
            const int DY23 = Y2 - Y3;
            const int DY31 = Y3 - Y1;

            // Fixed-point deltas
            const int FDX12 = DX12 << 4;
            const int FDX23 = DX23 << 4;
            const int FDX31 = DX31 << 4;

            const int FDY12 = DY12 << 4;
            const int FDY23 = DY23 << 4;
            const int FDY31 = DY31 << 4;

            // Bounding rectangle
            int minx = (std::min(std::min(X1, X2), X3) + 0xF) >> 4;
            int maxx = (std::max(std::max(X1, X2), X3) + 0xF) >> 4;
            int miny = (std::min(std::min(Y1, Y2), Y3) + 0xF) >> 4;
            int maxy = (std::max(std::max(Y1, Y2), Y3) + 0xF) >> 4;

            //Bonding rectangle intersect with the clip rect
            if( !clip_test(minx,miny) && !clip_test(maxx,maxy)
            && !clip_test(minx,maxy) && !clip_test(maxx,miny))
                return;
            minx = std::max(minx,clip_rect_.x0);
            miny = std::max(miny,clip_rect_.y0);
            maxx = std::min(maxx,clip_rect_.x1);
            maxy = std::min(maxy,clip_rect_.y1);

            // Block size, standard 8x8 (must be power of two)
            const int q = 8;

            // Start in corner of 8x8 block
            minx &= ~(q - 1);
            miny &= ~(q - 1);

            //(char*&)colorBuffer += miny * stride;

            // Half-edge constants
            int C1 = DY12 * X1 - DX12 * Y1;
            int C2 = DY23 * X2 - DX23 * Y2;
            int C3 = DY31 * X3 - DX31 * Y3;

            // Correct for fill convention
            if (DY12 < 0 || (DY12 == 0 && DX12 > 0)) C1++;
            if (DY23 < 0 || (DY23 == 0 && DX23 > 0)) C2++;
            if (DY31 < 0 || (DY31 == 0 && DX31 > 0)) C3++;

            // Loop through blocks
            //use openmp to do the parallel job to fill the pixels
            #pragma omp parallel for
            for (int y = miny; y < maxy; y += q)
            {
                for (int x = minx; x < maxx; x += q)
                {
                    // Corners of block
                    int x0 = x << 4;
                    int x1 = (x + q - 1) << 4;
                    int y0 = y << 4;
                    int y1 = (y + q - 1) << 4;

                    // Evaluate half-space functions
                    bool a00 = C1 + DX12 * y0 - DY12 * x0 > 0;
                    bool a10 = C1 + DX12 * y0 - DY12 * x1 > 0;
                    bool a01 = C1 + DX12 * y1 - DY12 * x0 > 0;
                    bool a11 = C1 + DX12 * y1 - DY12 * x1 > 0;
                    int a = (a00 << 0) | (a10 << 1) | (a01 << 2) | (a11 << 3);

                    bool b00 = C2 + DX23 * y0 - DY23 * x0 > 0;
                    bool b10 = C2 + DX23 * y0 - DY23 * x1 > 0;
                    bool b01 = C2 + DX23 * y1 - DY23 * x0 > 0;
                    bool b11 = C2 + DX23 * y1 - DY23 * x1 > 0;
                    int b = (b00 << 0) | (b10 << 1) | (b01 << 2) | (b11 << 3);

                    bool c00 = C3 + DX31 * y0 - DY31 * x0 > 0;
                    bool c10 = C3 + DX31 * y0 - DY31 * x1 > 0;
                    bool c01 = C3 + DX31 * y1 - DY31 * x0 > 0;
                    bool c11 = C3 + DX31 * y1 - DY31 * x1 > 0;
                    int c = (c00 << 0) | (c10 << 1) | (c01 << 2) | (c11 << 3);

                    // Skip block when outside an edge
                    if (a == 0x0 || b == 0x0 || c == 0x0) continue;

                    //color_type *buffer = colorBuffer;

                    // Accept whole block when totally covered
                    if (a == 0xF && b == 0xF && c == 0xF)
                    {
                        for (int iy = 0; iy < q; iy++)
                        {
                            for (int ix = x; ix < x + q; ix++)
                            {
                                //buffer[ix] = 0x00007F00;<< // Green
                                FragSpan::single_fragment(ix,y + iy);
                            }

                            //(char*&)buffer += stride;
                        }
                    }
                    else // Partially covered block
                    {
                        int CY1 = C1 + DX12 * y0 - DY12 * x0;
                        int CY2 = C2 + DX23 * y0 - DY23 * x0;
                        int CY3 = C3 + DX31 * y0 - DY31 * x0;

                        for (int iy = y; iy < y + q; iy++)
                        {
                            int CX1 = CY1;
                            int CX2 = CY2;
                            int CX3 = CY3;

                            for (int ix = x; ix < x + q; ix++)
                            {
                                if (CX1 > 0 && CX2 > 0 && CX3 > 0)
                                {
                                    //buffer[ix] = 0x0000007F;<< // Blue
                                    FragSpan::single_fragment(ix,iy);
                                }

                                CX1 -= FDY12;
                                CX2 -= FDY23;
                                CX3 -= FDY31;
                            }

                            CY1 += FDX12;
                            CY2 += FDX23;
                            CY3 += FDX31;

                            //(char*&)buffer += stride;
                        }
                    }
                }

                //(char*&)colorBuffer += q * stride;
            }

        }
        template <typename FragSpan>
        void triangle_template(const Vertex &v1, const Vertex &v2, const Vertex &v3)
        {
           FragSpan::begin_triangle(v1, v2, v3);
            return triangle_template_fixed_point<FragSpan>(v1,v2,v3);
/////////////////////////////////////////////////////////////////////////////////////////////

//			//using namespace detail;
//
//			// Early bounds test. Skip triangle if outside clip rect.
//			// This is intended for the special case when the clip_rect is smaller
//			// than the whole screen as is the case in dual cpu rendering, when each
//			// cpu renders only part of the screen.
//			int minx = std::min(std::min(v1.x, v2.x), v3.x) >> 4;
//			int miny = std::min(std::min(v1.y, v2.y), v3.y) >> 4;
//			int maxx = std::max(std::max(v1.x, v2.x), v3.x) >> 4;
//			int maxy = std::max(std::max(v1.y, v2.y), v3.y) >> 4;
//
//			if (minx >= clip_rect_.x1 || miny >= clip_rect_.y1 ||
//				maxx < clip_rect_.x0 || maxy < clip_rect_.y0)
//				return;
//
//			// Deltas
//			const int DX12 = v1.x - v2.x;
//			const int DX31 = v3.x - v1.x;
//
//			const int DY12 = v1.y - v2.y;
//			const int DY31 = v3.y - v1.y;
//
//			// this is actually twice the area
//			const int area = DX12 * DY31 - DX31 * DY12;
//
//			if (area <= 0xf)
//				return;
//
//			// Execute per triangle function. This can be used to compute the mipmap
//			// level per primitive. To support mipmap per pixel at least one varying
//			// attributes would need to be written by this function but this isn't
//			// possible for now as the vertices are declared const.
//			FragSpan::begin_triangle(v1, v2, v3, area);
//
//			// inv_area in 8.24
//			const int inv_area = 0;//invert(area);
//
//			// sort vertices in y and determine on which side the middle vertex lies
//			const Vertex *top, *middle, *bottom;
//			bool middle_is_left;
//
//			if (v1.y < v2.y) {
//				if (v1.y < v3.y) {
//					top = &v1;
//					if (v2.y < v3.y) {
//						middle = &v2;
//						bottom = &v3;
//						middle_is_left = true;
//					}
//					else {
//						middle = &v3;
//						bottom = &v2;
//						middle_is_left = false;
//					}
//				}
//				else {
//					top = &v3;
//					middle = &v1;
//					bottom = &v2;
//					middle_is_left = true;
//				}
//			}
//			else {
//				if (v2.y < v3.y) {
//					top = &v2;
//					if (v1.y < v3.y) {
//						middle = &v1;
//						bottom = &v3;
//						middle_is_left = false;
//					}
//					else {
//						middle = &v3;
//						bottom = &v1;
//						middle_is_left = true;
//					}
//				}
//				else {
//					top = &v3;
//					middle = &v2;
//					bottom = &v1;
//					middle_is_left = false;
//				}
//			}
//
//			if (perspective_correction_ && (
//				(maxx - minx) > perspective_threshold_.w ||
//				(maxy - miny) > perspective_threshold_.h ))
//			{
//				// computes the gradients of the varyings to be used for stepping
//				struct Gradients {
//					FragmentDataPerspective dx;
//					FragmentDataPerspective dy;
//
//					Gradients(const Vertex &v1, const Vertex &v2, const Vertex &v3,
//						int DX12, int DY12, int DX31, int DY31, int inv_area)
//					{
//						if (FragSpan::interpolate_z)
//							compute_gradients(DX12, DY12, DX31, DY31,
//							inv_area, v1.z, v2.z, v3.z, dx.fd.z, dy.fd.z);
//
//						if (FragSpan::varying_count) {
//							int invw1 = invert(v1.w);
//							int invw2 = invert(v2.w);
//							int invw3 = invert(v3.w);
//
//							compute_gradients(DX12, DY12, DX31, DY31,
//								inv_area, invw1, invw2, invw3, dx.oow, dy.oow);
//
//							for (unsigned i = 0; i < FragSpan::varying_count; ++i) {
//								int var1 = fixmul<16>(v1.varyings[i], invw1);
//								int var2 = fixmul<16>(v2.varyings[i], invw2);
//								int var3 = fixmul<16>(v3.varyings[i], invw3);
//
//								compute_gradients(DX12, DY12, DX31, DY31,
//									inv_area, var1, var2, var3,
//									dx.fd.varyings[i], dy.fd.varyings[i]);
//							}
//						}
//					}
//				};
//
//				// Edge structure used to walk the edge of the triangle and fill the
//				// scanlines
//				struct Edge {
//					int x, x_step, numerator, denominator, error_term; // DDA info for x
//					int y, height;
//
//					FragmentDataPerspective fragment_data;
//					FragmentDataPerspective fragment_step;
//
//					const Gradients& grad;
//
//					Edge(const Gradients &grad_, const Vertex* top, const Vertex *bottom)
//						: grad(grad_)
//					{
//						y = ceil28_4(top->y);
//						int yend = ceil28_4(bottom->y);
//						height = yend - y;
//
//						if (height) {
//							int dn = bottom->y - top->y;
//							int dm = bottom->x - top->x;
//
//							int initial_numerator = dm*16*y - dm*top->y +
//								dn*top->x - 1 + dn*16;
//							floor_divmod(initial_numerator, dn*16, &x, &error_term);
//							floor_divmod(dm*16, dn*16, &x_step, &numerator);
//							denominator = dn*16;
//
//							int y_prestep = y*16 - top->y;
//							int x_prestep = x*16 - top->x;
//
//#define PRESTEP(VAR) \
//	(((y_prestep * grad.dy.VAR) >> 4) + \
//	((x_prestep * grad.dx.VAR) >> 4))
//
//#define STEP(VAR) \
//	(x_step * grad.dx.VAR + grad.dy.VAR)
//
//							if (FragSpan::interpolate_z) {
//								fragment_data.fd.z = top->z + PRESTEP(fd.z);
//								fragment_step.fd.z = STEP(fd.z);
//							}
//
//							if (FragSpan::varying_count) {
//								int invw = invert(top->w);
//
//								fragment_data.oow = invw + PRESTEP(oow);
//								fragment_step.oow = STEP(oow);
//
//								for (unsigned i = 0; i < FragSpan::varying_count; ++i) {
//									fragment_data.fd.varyings[i] =
//										fixmul<16>(top->varyings[i], invw) + PRESTEP(fd.varyings[i]);
//									fragment_step.fd.varyings[i] = STEP(fd.varyings[i]);
//								}
//							}
//
//#undef STEP
//#undef PRESTEP
//						}
//					}
//
//					void step(bool step_varyings)
//					{
//						x += x_step; y++; height--;
//
//						if (step_varyings)
//							FRAGMENTDATA_PERSPECTIVE_APPLY(FragSpan, fragment_data, +=, fragment_step);
//
//						error_term += numerator;
//						if (error_term >= denominator) {
//							x++;
//							if (step_varyings)
//								FRAGMENTDATA_PERSPECTIVE_APPLY(FragSpan, fragment_data, +=, grad.dx);
//							error_term -= denominator;
//						}
//					}
//				};
//
//				Gradients grad(v1, v2, v3, DX12, DY12, DX31, DY31, inv_area);
//				Edge top_middle(grad, top, middle);
//				Edge top_bottom(grad, top, bottom);
//				Edge middle_bottom(grad, middle, bottom);
//
//				Edge *left, *right;
//				if (middle_is_left) {
//					left = &top_middle;
//					right = &top_bottom;
//				}
//				else {
//					left = &top_bottom;
//					right = &top_middle;
//				}
//
//				struct Scanline {
//					static void draw(const Edge *left, const Edge *right, int cl, int cr)
//					{
//						int y = left->y;
//						int l = left->x;
//						int r = std::min(right->x, cr);
//						const Gradients &grad = left->grad;
//
//						if (r - l <= 0) return;
//
//						// suppress possible GCC warnings by doing copy construction on fdp.
//						// should not harm performance.
//						FragmentDataPerspective fdp = FragmentDataPerspective();
//						FRAGMENTDATA_PERSPECTIVE_APPLY(FragSpan, fdp, =, left->fragment_data);
//
//						// skip pixels left of the clipping rectangle
//						if (l < cl) {
//							int d = cl - l;
//							FRAGMENTDATA_PERSPECTIVE_APPLY(FragSpan, fdp, += d *, grad.dx);
//							l = cl;
//						}
//
//						FragSpan::perspective_span(l, y, fdp, grad.dx, r - l);
//					}
//				};
//
//				int height = middle_is_left ? left->height : right->height;
//
//				// draw top triangle
//				while (height) {
//					int y = left->y;
//					if (ilace_drawit(y) && y >= clip_rect_.y0 && y < clip_rect_.y1)
//						Scanline::draw(left, right, clip_rect_.x0, clip_rect_.x1);
//					left->step(true);
//					right->step(false);
//					height--;
//				}
//
//				if (middle_is_left) {
//					left = &middle_bottom;
//					height = left->height;
//				}
//				else {
//					right = &middle_bottom;
//					height = right->height;
//				}
//
//				// draw bottom triangle
//				while (height) {
//					int y = left->y;
//					if (ilace_drawit(y) && y >= clip_rect_.y0 && y < clip_rect_.y1)
//						Scanline::draw(left, right, clip_rect_.x0, clip_rect_.x1);
//					left->step(true);
//					right->step(false);
//					height--;
//				}
//			}
//			// affine interpolation and rendering
//			else {
//				// computes the gradients of the varyings to be used for stepping
//				struct Gradients {
//					FragmentData dx;
//					FragmentData dy;
//
//					Gradients(const Vertex &v1, const Vertex &v2, const Vertex &v3,
//						int DX12, int DY12, int DX31, int DY31, int inv_area)
//					{
//						if (FragSpan::interpolate_z)
//							compute_gradients(DX12, DY12, DX31, DY31,
//							inv_area, v1.z, v2.z, v3.z, dx.z, dy.z);
//
//						for (unsigned i = 0; i < FragSpan::varying_count; ++i)
//							compute_gradients(DX12, DY12, DX31, DY31,
//							inv_area, v1.varyings[i], v2.varyings[i], v3.varyings[i],
//							dx.varyings[i], dy.varyings[i]);
//					}
//				};
//
//				// Edge structure used to walk the edge of the triangle and fill the
//				// scanlines
//				struct Edge {
//					int x, x_step, numerator, denominator, error_term; // DDA info for x
//					int y, height;
//
//					FragmentData fragment_data;
//					FragmentData fragment_step;
//
//					const Gradients& grad;
//
//					Edge(const Gradients &grad_, const Vertex* top, const Vertex *bottom)
//						: grad(grad_)
//					{
//						y = ceil28_4(top->y);
//						int yend = ceil28_4(bottom->y);
//						height = yend - y;
//
//						if (height) {
//							int dn = bottom->y - top->y;
//							int dm = bottom->x - top->x;
//
//							int initial_numerator = dm*16*y - dm*top->y +
//								dn*top->x - 1 + dn*16;
//							floor_divmod(initial_numerator, dn*16, &x, &error_term);
//							floor_divmod(dm*16, dn*16, &x_step, &numerator);
//							denominator = dn*16;
//
//							int y_prestep = y*16 - top->y;
//							int x_prestep = x*16 - top->x;
//

//#define PRESTEP(VAR) \
//	(((y_prestep * grad.dy.VAR) >> 4) + \
//	((x_prestep * grad.dx.VAR) >> 4))
//

//#define STEP(VAR) \
//	(x_step * grad.dx.VAR + grad.dy.VAR)
//
//							if (FragSpan::interpolate_z) {
//								fragment_data.z = top->z + PRESTEP(z);
//								fragment_step.z = STEP(z);
//							}
//
//							for (unsigned i = 0; i < FragSpan::varying_count; ++i) {
//								fragment_data.varyings[i] = top->varyings[i] + PRESTEP(varyings[i]);
//								fragment_step.varyings[i] = STEP(varyings[i]);
//							}
//
//#undef PRESTEP
//#undef STEP
//						}
//					}
//
//					void step(bool step_varyings)
//					{
//						x += x_step; y++; height--;
//
//						if (step_varyings)
//							FRAGMENTDATA_APPLY(FragSpan, fragment_data, +=, fragment_step);
//
//						error_term += numerator;
//						if (error_term >= denominator) {
//							x++;
//							if (step_varyings)
//								FRAGMENTDATA_APPLY(FragSpan, fragment_data, +=, grad.dx);
//							error_term -= denominator;
//						}
//					}
//				};
//
//				Gradients grad(v1, v2, v3, DX12, DY12, DX31, DY31, inv_area);
//				Edge top_middle(grad, top, middle);
//				Edge top_bottom(grad, top, bottom);
//				Edge middle_bottom(grad, middle, bottom);
//
//				Edge *left, *right;
//				if (middle_is_left) {
//					left = &top_middle;
//					right = &top_bottom;
//				}
//				else {
//					left = &top_bottom;
//					right = &top_middle;
//				}
//
//				struct Scanline {
//					static void draw(const Edge *left, const Edge *right, int cl, int cr)
//					{
//						int y = left->y;
//						int l = left->x;
//						int r = std::min(right->x, cr);
//						const Gradients &grad = left->grad;
//
//						if (r - l <= 0) return;
//
//						// suppress possible GCC warnings by doing copy construction on fd.
//						// should not harm performance.
//						FragmentData fd = FragmentData();
//						FRAGMENTDATA_APPLY(FragSpan, fd, =, left->fragment_data);
//
//						// skip pixels left of the clipping rectangle
//						if (l < cl) {
//							int d = cl - l;
//							FRAGMENTDATA_APPLY(FragSpan, fd, += d *, grad.dx);
//							l = cl;
//						}
//
//						// draw the scanline up until the right side
//						FragSpan::affine_span(l, y, fd, grad.dx, r - l);
//					}
//				};
//
//				int height = middle_is_left ? left->height : right->height;
//
//				// draw top triangle
//				while (height) {
//					int y = left->y;
//					if (ilace_drawit(y) && y >= clip_rect_.y0 && y < clip_rect_.y1)
//						Scanline::draw(left, right, clip_rect_.x0, clip_rect_.x1);
//					left->step(true);
//					right->step(false);
//					height--;
//				}
//
//				if (middle_is_left) {
//					left = &middle_bottom;
//					height = left->height;
//				}
//				else {
//					right = &middle_bottom;
//					height = right->height;
//				}
//
//				// draw bottom triangle
//				while (height) {
//					int y = left->y;
//					if (ilace_drawit(y) && y >= clip_rect_.y0 && y < clip_rect_.y1)
//						Scanline::draw(left, right, clip_rect_.x0, clip_rect_.x1);
//					left->step(true);
//					right->step(false);
//					height--;
//				}
//			}
        }
    };
}
#endif
