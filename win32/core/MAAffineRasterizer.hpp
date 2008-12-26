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

        template<typename FragSpan>
        void triangle_template_fixed_point(const Vertex &v1, const Vertex &v2, const Vertex &v3)
        {
            const Vertex  *tri_v1,*tri_v2,*tri_v3;
            bool is_middle_left = false;
            /////////////////////////////////////////////////////////////////////////////////////
            //sort triangle vertex
            is_middle_left = FragSpan::sort_triangle_vertex(v1, v2, v3,tri_v1,tri_v2,tri_v3);
            /////////////////////////////////////////////////////////////////////////////////////

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
            if ( !clip_test(minx,miny) && !clip_test(maxx,maxy)
                    && !clip_test(minx,maxy) && !clip_test(maxx,miny))
                return;
            minx = std::max(minx,clip_rect_.x0);
            miny = std::max(miny,clip_rect_.y0);
            maxx = std::min(maxx,clip_rect_.x1);
            maxy = std::min(maxy,clip_rect_.y1);

            // Block size, standard 8x8 (must be power of two)
            static const int q = 8;

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
                //////////////////////////////////////////////////////////////////////////////
                //prepare for gradient
                float left_ty[q];
                float right_ty[q];
                float z_left[q];
                float z_right[q];
                float linear_varyings_left[q][MARasterizerBase::MAX_VARYING];
                float linear_varyings_right[q][MARasterizerBase::MAX_VARYING];

                float x_step[q];
                float x_left[q];
                bool computed_flag = false;
                //float left_x[q];
                //float right_x[q];

////////////////////////////////////////////////////////////////////////////////////////////////////////

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

                    //computedf only once for this line
                    if (! computed_flag)
                    {
                        if (is_middle_left)
                        {
                            for (int k = 0;k < q; ++k)
                            {
                                if (y+k < tri_v2->y) // betwenn v1,v2
                                {
                                    left_ty[k] = linearStep(tri_v1->y,tri_v2->y,(float)(y+k));
                                    float left_x = lerp(tri_v1->x,tri_v2->x,left_ty[k]);
                                    right_ty[k] = linearStep(tri_v1->y,tri_v3->y,(float)(y+k));
                                    float right_x = lerp(tri_v1->x,tri_v3->x,right_ty[k]);

                                    //same pixel ?
                                    float rx = round(right_x);
                                    float lx = round(left_x);
                                    if (lx > x1)
                                        continue;
                                    x_step[k] = rx - lx >= 1.f ? (1.f)/(rx-lx):0;
                                    //assert(x_step[k] > 0);
                                    x_left[k] = lx;

                                    assert(x_step[k] < 1.1f);

                                    z_left[k] = lerp(tri_v1->z,tri_v2->z,left_ty[k]);
                                    z_right[k] = lerp(tri_v1->z,tri_v3->z,right_ty[k]);

                                    int i = 0;
                                    DUFFS_DEVICE(8,int,MARasterizerBase::Vertex::linear_var_cnt,
                                                 linear_varyings_left[k][i]=lerp(tri_v1->linear_varyings[i],tri_v2->linear_varyings[i],left_ty[k]);++i;
                                                );
                                    i = 0;
                                    DUFFS_DEVICE(8,int,MARasterizerBase::Vertex::linear_var_cnt,
                                                 linear_varyings_right[k][i]=lerp(tri_v1->linear_varyings[i],tri_v3->linear_varyings[i],right_ty[k]);++i;
                                                );
                                }
                                else
                                {
                                    left_ty[k] = linearStep(tri_v2->y,tri_v3->y,(float)(y+k));
                                    float left_x = lerp(tri_v2->x,tri_v3->x,left_ty[k]);
                                    right_ty[k] = linearStep(tri_v1->y,tri_v3->y,(float)(y+k));
                                    float right_x = lerp(tri_v1->x,tri_v3->x,right_ty[k]);

                                    //same pixel ?
                                    float rx = round(right_x);
                                    float lx = round(left_x);
                                    if (lx > x1)
                                        continue;
                                    x_step[k] = rx - lx >= 1.f ?  (1.f)/(rx-lx):0;
                                    //assert(x_step[k] > 0);
                                    x_left[k] = lx;

                                    assert(x_step[k] < 1.1f);


                                    z_left[k] = lerp(tri_v2->z,tri_v3->z,left_ty[k]);
                                    z_right[k] = lerp(tri_v1->z,tri_v3->z,right_ty[k]);

                                    int i = 0;
                                    DUFFS_DEVICE(8,int,MARasterizerBase::Vertex::linear_var_cnt,
                                                 linear_varyings_left[k][i]=lerp(tri_v2->linear_varyings[i],tri_v3->linear_varyings[i],left_ty[k]);++i;
                                                );
                                    i = 0;
                                    DUFFS_DEVICE(8,int,MARasterizerBase::Vertex::linear_var_cnt,
                                                 linear_varyings_right[k][i]=lerp(tri_v1->linear_varyings[i],tri_v3->linear_varyings[i],right_ty[k]);++i;
                                                );
                                }
                            }
                        }
                        else
                        {
                            for (int k = 0;k < q; ++k)
                            {
                                if (y+k < tri_v2->y) // betwenn v1,v2
                                {
                                    right_ty[k] = linearStep(tri_v1->y,tri_v2->y,(float)(y+k));
                                    float right_x = lerp(tri_v1->x,tri_v2->x,right_ty[k]);
                                    left_ty[k] = linearStep(tri_v1->y,tri_v3->y,(float)(y+k));
                                    float left_x = lerp(tri_v1->x,tri_v3->x,left_ty[k]);

                                    //same pixel ?
                                    float rx = round(right_x);
                                    float lx = round(left_x);
                                    if (lx > x1)
                                        continue;
                                    x_step[k] = rx - lx >= 1.f ? (1.f)/(rx-lx):0;
                                    //assert(x_step[k] > 0);
                                    x_left[k] = lx;

                                    assert(x_step[k] < 1.1f);



                                    z_left[k] =lerp(tri_v1->z,tri_v3->z,left_ty[k]);
                                    z_right[k] = lerp(tri_v1->z,tri_v2->z,right_ty[k]);

                                    int i = 0;
                                    DUFFS_DEVICE(8,int,MARasterizerBase::Vertex::linear_var_cnt,
                                                 linear_varyings_left[k][i]=lerp(tri_v1->linear_varyings[i],tri_v3->linear_varyings[i],left_ty[k]);++i;
                                                );
                                    i = 0;
                                    DUFFS_DEVICE(8,int,MARasterizerBase::Vertex::linear_var_cnt,
                                                 linear_varyings_right[k][i]=lerp(tri_v1->linear_varyings[i],tri_v2->linear_varyings[i],right_ty[k]);++i;
                                                );
                                }
                                else
                                {
                                    right_ty[k] = linearStep(tri_v2->y,tri_v3->y,(float)(y+k));
                                    float right_x = lerp(tri_v2->x,tri_v3->x,right_ty[k]);
                                    left_ty[k] = linearStep(tri_v1->y,tri_v3->y,(float)(y+k));
                                    float left_x = lerp(tri_v1->x,tri_v3->x,left_ty[k]);

                                    //same pixel ?
                                    float rx = round(right_x);
                                    float lx = round(left_x);
                                    if (lx > x1)
                                        continue;
                                    x_step[k] = rx - lx >= 1.f ? (1.f)/(rx-lx):0;
                                    //assert(x_step[k] > 0);
                                    x_left[k] = lx;
                                    assert(x_step[k] < 1.1f);



                                    z_left[k] = lerp(tri_v1->z,tri_v3->z,left_ty[k]);
                                    z_right[k] = lerp(tri_v2->z,tri_v3->z,right_ty[k]);


                                    int i = 0;
                                    DUFFS_DEVICE(8,int,MARasterizerBase::Vertex::linear_var_cnt,
                                                 linear_varyings_left[k][i]=lerp(tri_v1->linear_varyings[i],tri_v3->linear_varyings[i],left_ty[k]);++i;
                                                );
                                    i = 0;
                                    DUFFS_DEVICE(8,int,MARasterizerBase::Vertex::linear_var_cnt,
                                                 linear_varyings_right[k][i]=lerp(tri_v2->linear_varyings[i],tri_v3->linear_varyings[i],right_ty[k]);++i;
                                                );
                                }
                            }
                        }
                        computed_flag = true;
                    }

                    // Accept whole block when totally covered
                    if (a == 0xF && b == 0xF && c == 0xF)
                    {
                        float x_fraction ;
                        float z_start ;
                        float z_step ;

                        float linear_varyings_start[MARasterizerBase::MAX_VARYING];
                        float linear_varyings_step[MARasterizerBase::MAX_VARYING];

                        for (int iy = 0; iy < q; iy++)
                        {
                            x_fraction = (x-x_left[iy]) * x_step[iy];/// x_fraction (- [0,1]

                            z_start = lerp(z_left[iy],z_right[iy],x_fraction);
                            z_step= x_step[iy] *(z_right[iy] - z_left[iy]);

                            int i_ = 0;
                            DUFFS_DEVICE(8,int,MARasterizerBase::Vertex::linear_var_cnt,
                                         linear_varyings_start[i_]=
                                             lerp(linear_varyings_left[iy][i_],linear_varyings_right[iy][i_],x_fraction);
                                         ++i_;
                                        );
                            i_ = 0;
                            DUFFS_DEVICE(8,int,MARasterizerBase::Vertex::linear_var_cnt,
                                         linear_varyings_step[i_]=
                                             (linear_varyings_right[iy][i_] - linear_varyings_left[iy][i_] )* x_step[iy];
                                         ++i_;
                                        );

                            ///
                            for (int ix = x; ix < x + q; ix++)
                            {
                                //buffer[ix] = 0x00007F00;<< // Green
                                {

                                    FragmentData fragment;
                                    fragment.z = z_start;
                                    i_ = 0 ;
                                    DUFFS_DEVICE(8,int,MARasterizerBase::Vertex::linear_var_cnt,
                                                 fragment.linear_varyings[i_] =linear_varyings_start[i_];
                                                 ++i_;
                                                );
                                    FragSpan::single_fragment(ix,y + iy,fragment);
                                }
                                ///
                                {
                                    //x_fraction += x_step[iy];
                                    z_start += z_step;
                                    i_ = 0;
                                    DUFFS_DEVICE(8,int,MARasterizerBase::Vertex::linear_var_cnt,
                                                 linear_varyings_start[i_]+=linear_varyings_step[i_];
                                                 ++i_;
                                                );
                                }
                                ///
                            }

                            //(char*&)buffer += stride;
                        }
                    }
                    else // Partially covered block
                    {
                        int CY1 = C1 + DX12 * y0 - DY12 * x0;
                        int CY2 = C2 + DX23 * y0 - DY23 * x0;
                        int CY3 = C3 + DX31 * y0 - DY31 * x0;

                        ///
                        float x_fraction ;
                        float z_start ;
                        float z_step ;
                        float linear_varyings_start[MARasterizerBase::MAX_VARYING];
                        float linear_varyings_step[MARasterizerBase::MAX_VARYING];

                        int i_ = 0;
                        ///
                        for (int iy = 0; iy <  q; iy++)
                        {
                            int CX1 = CY1;
                            int CX2 = CY2;
                            int CX3 = CY3;

                            ///
                            //if (CX1 > 0 && CX2 > 0 && CX3 > 0)

                            ///
                            ///for (int ix = x; ix < x + q; ix++)
                            //////////////////////////////////////////////////////////////////////
                            int xl = int(x_left[iy]);
                            if (xl >= x+q)
                            {
                                CY1 += FDX12;
                                CY2 += FDX23;
                                CY3 += FDX31;
                                continue;
                            }
                            xl = xl >= x && xl < x+q? xl : x;
                            int ix=x;
                            CX1 -= FDY12 * (xl-ix);
                            CX2 -= FDY23 * (xl-ix);
                            CX3 -= FDY31 * (xl-ix);
                            ix = xl;
                            //for (;ix < xl;++ix)
                            //{
                            //    CX1 -= FDY12;
                            //    CX2 -= FDY23;
                            //    CX3 -= FDY31;
                            //}

                            {
                                x_fraction = (xl-x_left[iy]) * x_step[iy];/// x_fraction (- [0,1]
                                //x_fraction = clamp(x_fraction,0.f,1.f);//x_fraction > 1.f ? 1.f:x_fraction;
                                x_fraction = x_fraction>1.f ? 1.f:x_fraction;
                                assert(! (x_fraction > 1.1f || x_fraction < -0.1f));
                                if (x_fraction > 1.1f || x_fraction < -0.1f)
                                    assert(false);
                                z_start = lerp(z_left[iy],z_right[iy],x_fraction);
                                z_step= x_step[iy] *(z_right[iy] - z_left[iy]);

                                i_= 0;
                                DUFFS_DEVICE(8,int,MARasterizerBase::Vertex::linear_var_cnt,
                                             linear_varyings_start[i_]=
                                                 lerp(linear_varyings_left[iy][i_],linear_varyings_right[iy][i_],x_fraction);
                                             ++i_;
                                            );
                                i_ = 0;
                                DUFFS_DEVICE(8,int,MARasterizerBase::Vertex::linear_var_cnt,
                                             linear_varyings_step[i_]=
                                                 (linear_varyings_right[iy][i_] - linear_varyings_left[iy][i_])*x_step[iy];
                                             ++i_;
                                            );
                            }
                            ////////////////////////////////////////////////////////////////////
                            for (int ix = xl;ix < x + q; ix++)
                            {
                                if (CX1 > 0 && CX2 > 0 && CX3 > 0)
                                {
                                    //buffer[ix] = 0x0000007F;<< // Blue
                                    ///
                                    {

                                    }
                                    ///
                                    {
                                        FragmentData fragment;
                                        fragment.z = z_start;
                                        i_ = 0 ;
                                        DUFFS_DEVICE(8,int,MARasterizerBase::Vertex::linear_var_cnt,
                                                     fragment.linear_varyings[i_] =linear_varyings_start[i_];
                                                     ++i_;
                                                    );
                                        FragSpan::single_fragment(ix,iy + y,fragment);
                                    }
                                    {
                                        z_start += z_step;
                                        i_ = 0;
                                        DUFFS_DEVICE(8,int,MARasterizerBase::Vertex::linear_var_cnt,
                                                     linear_varyings_start[i_]+=linear_varyings_step[i_];
                                                     ++i_;
                                                    );
                                    }
                                }

                                CX1 -= FDY12;
                                CX2 -= FDY23;
                                CX3 -= FDY31;
                            }

                            CY1 += FDX12;
                            CY2 += FDX23;
                            CY3 += FDX31;
                        }
                    }
                }
            }

        }
        // The triangle must be counter clockwise in screen space in order to be
        // drawn.
        template <typename FragSpan>
        void triangle_template_fixed_point_persp(const Vertex &v1, const Vertex &v2, const Vertex &v3)
        {
            const Vertex  *tri_v1,*tri_v2,*tri_v3;
            bool is_middle_left = false;
            /////////////////////////////////////////////////////////////////////////////////////
            //sort triangle vertex
            is_middle_left = FragSpan::sort_triangle_vertex(v1, v2, v3,tri_v1,tri_v2,tri_v3);
            /////////////////////////////////////////////////////////////////////////////////////

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
            if ( !clip_test(minx,miny) && !clip_test(maxx,maxy)
                    && !clip_test(minx,maxy) && !clip_test(maxx,miny))
                return;
            minx = std::max(minx,clip_rect_.x0);
            miny = std::max(miny,clip_rect_.y0);
            maxx = std::min(maxx,clip_rect_.x1);
            maxy = std::min(maxy,clip_rect_.y1);

            // Block size, standard 8x8 (must be power of two)
            static const int q = 8;

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
                //////////////////////////////////////////////////////////////////////////////
                //prepare for gradient
                float left_ty[q];
                float right_ty[q];
                float z_left[q];
                float z_right[q];
                float linear_varyings_left[q][MARasterizerBase::MAX_VARYING];
                float linear_varyings_right[q][MARasterizerBase::MAX_VARYING];
                /// perspective correction
                float inv_w_left[q];
                float inv_w_right[q];
                float persp_varyings_left[q][MARasterizerBase::MAX_VARYING];
                float persp_varyings_right[q][MARasterizerBase::MAX_VARYING];
                ///
                float x_step[q];
                float x_left[q];
                bool computed_flag = false;
                //float left_x[q];
                //float right_x[q];

////////////////////////////////////////////////////////////////////////////////////////////////////////

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

                    //computedf only once for this line
                    if (! computed_flag)
                    {
                        if (is_middle_left)
                        {
                            for (int k = 0;k < q; ++k)
                            {
                                if (y+k < tri_v2->y) // betwenn v1,v2
                                {
                                    left_ty[k] = linearStep(tri_v1->y,tri_v2->y,(float)(y+k));
                                    float left_x = lerp(tri_v1->x,tri_v2->x,left_ty[k]);
                                    right_ty[k] = linearStep(tri_v1->y,tri_v3->y,(float)(y+k));
                                    float right_x = lerp(tri_v1->x,tri_v3->x,right_ty[k]);

                                    //same pixel ?
                                    float rx = round(right_x);
                                    float lx = round(left_x);
                                    x_step[k] = rx - lx >= 1.f ? (1.f)/(rx-lx):0;
                                    //assert(x_step[k] > 0);
                                    x_left[k] = lx;

                                    assert(x_step[k] < 1.1f);



                                    z_left[k] = lerp(tri_v1->z,tri_v2->z,left_ty[k]);
                                    z_right[k] = lerp(tri_v1->z,tri_v3->z,right_ty[k]);

                                    inv_w_left[k] = 16.f * lerp(tri_v1->inv_w,tri_v2->inv_w,left_ty[k]);
                                    inv_w_right[k] = 16.f * lerp(tri_v1->inv_w,tri_v3->inv_w,right_ty[k]);

                                    int i = 0;
                                    DUFFS_DEVICE(8,int,MARasterizerBase::Vertex::linear_var_cnt,
                                                 linear_varyings_left[k][i]=lerp(tri_v1->linear_varyings[i],tri_v2->linear_varyings[i],left_ty[k]);++i;
                                                );
                                    i = 0;
                                    DUFFS_DEVICE(8,int,MARasterizerBase::Vertex::linear_var_cnt,
                                                 linear_varyings_right[k][i]=lerp(tri_v1->linear_varyings[i],tri_v3->linear_varyings[i],right_ty[k]);++i;
                                                );
                                    i = 0;
                                    DUFFS_DEVICE(8,int,MARasterizerBase::Vertex::persp_var_cnt ,
                                                 persp_varyings_left[k][i]=lerp(tri_v1->perspective_varyings[i],tri_v2->perspective_varyings[i],left_ty[k]);++i;
                                                );
                                    i = 0;
                                    DUFFS_DEVICE(8,int,MARasterizerBase::Vertex::persp_var_cnt,
                                                 persp_varyings_right[k][i]=lerp(tri_v1->perspective_varyings[i],tri_v3->perspective_varyings[i],right_ty[k]);++i;
                                                );
                                }
                                else
                                {
                                    left_ty[k] = linearStep(tri_v2->y,tri_v3->y,(float)(y+k));
                                    float left_x = lerp(tri_v2->x,tri_v3->x,left_ty[k]);
                                    right_ty[k] = linearStep(tri_v1->y,tri_v3->y,(float)(y+k));
                                    float right_x = lerp(tri_v1->x,tri_v3->x,right_ty[k]);

                                    //same pixel ?
                                    float rx = round(right_x);
                                    float lx = round(left_x);
                                    x_step[k] = rx - lx >= 1.f ?  (1.f)/(rx-lx):0;
                                    //assert(x_step[k] > 0);
                                    x_left[k] = lx;

                                    assert(x_step[k] < 1.1f);


                                    z_left[k] = lerp(tri_v2->z,tri_v3->z,left_ty[k]);
                                    z_right[k] = lerp(tri_v1->z,tri_v3->z,right_ty[k]);

                                    inv_w_left[k] =16.f * lerp(tri_v2->inv_w,tri_v3->inv_w,left_ty[k]);
                                    inv_w_right[k] =16.f * lerp(tri_v1->inv_w,tri_v3->inv_w,right_ty[k]);

                                    int i = 0;
                                    DUFFS_DEVICE(8,int,MARasterizerBase::Vertex::linear_var_cnt,
                                                 linear_varyings_left[k][i]=lerp(tri_v2->linear_varyings[i],tri_v3->linear_varyings[i],left_ty[k]);++i;
                                                );
                                    i = 0;
                                    DUFFS_DEVICE(8,int,MARasterizerBase::Vertex::linear_var_cnt,
                                                 linear_varyings_right[k][i]=lerp(tri_v1->linear_varyings[i],tri_v3->linear_varyings[i],right_ty[k]);++i;
                                                );
                                    i = 0;
                                    DUFFS_DEVICE(8,int,MARasterizerBase::Vertex::persp_var_cnt ,
                                                 persp_varyings_left[k][i]=lerp(tri_v2->perspective_varyings[i],tri_v3->perspective_varyings[i],left_ty[k]);++i;
                                                );
                                    i = 0;
                                    DUFFS_DEVICE(8,int,MARasterizerBase::Vertex::persp_var_cnt,
                                                 persp_varyings_right[k][i]=lerp(tri_v1->perspective_varyings[i],tri_v3->perspective_varyings[i],right_ty[k]);++i;
                                                );
                                }
                            }
                        }
                        else
                        {
                            for (int k = 0;k < q; ++k)
                            {
                                if (y+k < tri_v2->y) // betwenn v1,v2
                                {
                                    right_ty[k] = linearStep(tri_v1->y,tri_v2->y,(float)(y+k));
                                    float right_x = lerp(tri_v1->x,tri_v2->x,right_ty[k]);
                                    left_ty[k] = linearStep(tri_v1->y,tri_v3->y,(float)(y+k));
                                    float left_x = lerp(tri_v1->x,tri_v3->x,left_ty[k]);

                                    //same pixel ?
                                    float rx = round(right_x);
                                    float lx = round(left_x);
                                    x_step[k] = rx - lx >= 1.f ? (1.f)/(rx-lx):0;
                                    //assert(x_step[k] > 0);
                                    x_left[k] = lx;

                                    assert(x_step[k] < 1.1f);



                                    z_left[k] =lerp(tri_v1->z,tri_v3->z,left_ty[k]);
                                    z_right[k] = lerp(tri_v1->z,tri_v2->z,right_ty[k]);

                                    inv_w_left[k] =16.f * lerp(tri_v1->inv_w,tri_v3->inv_w,left_ty[k]);
                                    inv_w_right[k] =16.f * lerp(tri_v1->inv_w,tri_v2->inv_w,right_ty[k]);

                                    int i = 0;
                                    DUFFS_DEVICE(8,int,MARasterizerBase::Vertex::linear_var_cnt,
                                                 linear_varyings_left[k][i]=lerp(tri_v1->linear_varyings[i],tri_v3->linear_varyings[i],left_ty[k]);++i;
                                                );
                                    i = 0;
                                    DUFFS_DEVICE(8,int,MARasterizerBase::Vertex::linear_var_cnt,
                                                 linear_varyings_right[k][i]=lerp(tri_v1->linear_varyings[i],tri_v2->linear_varyings[i],right_ty[k]);++i;
                                                );

                                    i = 0;
                                    DUFFS_DEVICE(8,int,MARasterizerBase::Vertex::persp_var_cnt ,
                                                 persp_varyings_left[k][i]=lerp(tri_v1->perspective_varyings[i],tri_v3->perspective_varyings[i],left_ty[k]);++i;
                                                );
                                    i = 0;
                                    DUFFS_DEVICE(8,int,MARasterizerBase::Vertex::persp_var_cnt,
                                                 persp_varyings_right[k][i]=lerp(tri_v1->perspective_varyings[i],tri_v2->perspective_varyings[i],right_ty[k]);++i;
                                                );
                                }
                                else
                                {
                                    right_ty[k] = linearStep(tri_v2->y,tri_v3->y,(float)(y+k));
                                    float right_x = lerp(tri_v2->x,tri_v3->x,right_ty[k]);
                                    left_ty[k] = linearStep(tri_v1->y,tri_v3->y,(float)(y+k));
                                    float left_x = lerp(tri_v1->x,tri_v3->x,left_ty[k]);

                                    //same pixel ?
                                    float rx = round(right_x);
                                    float lx = round(left_x);
                                    x_step[k] = rx - lx >= 1.f ? (1.f)/(rx-lx):0;
                                    //assert(x_step[k] > 0);
                                    x_left[k] = lx;
                                    assert(x_step[k] < 1.1f);



                                    z_left[k] = lerp(tri_v1->z,tri_v3->z,left_ty[k]);
                                    z_right[k] = lerp(tri_v2->z,tri_v3->z,right_ty[k]);

                                    inv_w_left[k] =16.f * lerp(tri_v1->inv_w,tri_v3->inv_w,left_ty[k]);
                                    inv_w_right[k] =16.f * lerp(tri_v2->inv_w,tri_v3->inv_w,right_ty[k]);

                                    int i = 0;
                                    DUFFS_DEVICE(8,int,MARasterizerBase::Vertex::linear_var_cnt,
                                                 linear_varyings_left[k][i]=lerp(tri_v1->linear_varyings[i],tri_v3->linear_varyings[i],left_ty[k]);++i;
                                                );
                                    i = 0;
                                    DUFFS_DEVICE(8,int,MARasterizerBase::Vertex::linear_var_cnt,
                                                 linear_varyings_right[k][i]=lerp(tri_v2->linear_varyings[i],tri_v3->linear_varyings[i],right_ty[k]);++i;
                                                );

                                    i = 0;
                                    DUFFS_DEVICE(8,int,MARasterizerBase::Vertex::persp_var_cnt ,
                                                 persp_varyings_left[k][i]=lerp(tri_v1->perspective_varyings[i],tri_v3->perspective_varyings[i],left_ty[k]);++i;
                                                );
                                    i = 0;
                                    DUFFS_DEVICE(8,int,MARasterizerBase::Vertex::persp_var_cnt,
                                                 persp_varyings_right[k][i]=lerp(tri_v2->perspective_varyings[i],tri_v3->perspective_varyings[i],right_ty[k]);++i;
                                                );
                                }
                            }
                        }
                        computed_flag = true;
                    }

                    // Accept whole block when totally covered
                    if (a == 0xF && b == 0xF && c == 0xF)
                    {
                        float x_fraction ;
                        float z_start ;
                        float z_step ;
                        float inv_w_start;
                        float inv_w_step;

                        float linear_varyings_start[MARasterizerBase::MAX_VARYING];
                        float linear_varyings_step[MARasterizerBase::MAX_VARYING];

                        float persp_varyings_start[MARasterizerBase::MAX_VARYING];
                        float persp_varyings_step[MARasterizerBase::MAX_VARYING];
                        for (int iy = 0; iy < q; iy++)
                        {
                            x_fraction = (x-x_left[iy]) * x_step[iy];/// x_fraction (- [0,1]

                            z_start = lerp(z_left[iy],z_right[iy],x_fraction);
                            z_step= x_step[iy] *(z_right[iy] - z_left[iy]);
                            inv_w_start = lerp(inv_w_left[iy],inv_w_right[iy],x_fraction);
                            inv_w_step = x_step[iy] * (inv_w_right[iy] - inv_w_left[iy]);

                            int i_ = 0;
                            DUFFS_DEVICE(8,int,MARasterizerBase::Vertex::linear_var_cnt,
                                         linear_varyings_start[i_]=
                                             lerp(linear_varyings_left[iy][i_],linear_varyings_right[iy][i_],x_fraction);
                                         ++i_;
                                        );
                            i_ = 0;
                            DUFFS_DEVICE(8,int,MARasterizerBase::Vertex::linear_var_cnt,
                                         linear_varyings_step[i_]=
                                             (linear_varyings_right[iy][i_] - linear_varyings_left[iy][i_] )* x_step[iy];
                                         ++i_;
                                        );

                            i_ = 0;
                            DUFFS_DEVICE(8,int,MARasterizerBase::Vertex::persp_var_cnt,
                                         persp_varyings_start[i_]=
                                             lerp(persp_varyings_left[iy][i_],persp_varyings_right[iy][i_],x_fraction);
                                         ++i_;
                                        );
                            i_ = 0;
                            DUFFS_DEVICE(8,int,MARasterizerBase::Vertex::persp_var_cnt,
                                         persp_varyings_step[i_]=
                                             (persp_varyings_right[iy][i_] - persp_varyings_left[iy][i_])*x_step[iy];
                                         ++i_;
                                        );
                            ///
                            for (int ix = x; ix < x + q; ix++)
                            {
                                //buffer[ix] = 0x00007F00;<< // Green
                                {

                                    FragmentData fragment;
                                    fragment.z = z_start;
                                    i_ = 0 ;
                                    DUFFS_DEVICE(8,int,MARasterizerBase::Vertex::linear_var_cnt,
                                                 fragment.linear_varyings[i_] =linear_varyings_start[i_];
                                                 ++i_;
                                                );
                                    i_=0;
                                    DUFFS_DEVICE(8,int,MARasterizerBase::Vertex::persp_var_cnt,
                                                 fragment.perspective_varyings[i_] =16.f * persp_varyings_start[i_] / inv_w_start;
                                                 ++i_;
                                                );
                                    FragSpan::single_fragment(ix,y + iy,fragment);
                                }
                                ///
                                {
                                    //x_fraction += x_step[iy];
                                    z_start += z_step;
                                    inv_w_start += inv_w_step;
                                    i_ = 0;
                                    DUFFS_DEVICE(8,int,MARasterizerBase::Vertex::linear_var_cnt,
                                                 linear_varyings_start[i_]+=linear_varyings_step[i_];
                                                 ++i_;
                                                );
                                    i_= 0 ;
                                    DUFFS_DEVICE(8,int,MARasterizerBase::Vertex::persp_var_cnt,
                                                 persp_varyings_start[i_] += persp_varyings_step[i_];
                                                 ++i_;
                                                );
                                }
                                ///
                            }

                            //(char*&)buffer += stride;
                        }
                    }
                    else // Partially covered block
                    {
                        int CY1 = C1 + DX12 * y0 - DY12 * x0;
                        int CY2 = C2 + DX23 * y0 - DY23 * x0;
                        int CY3 = C3 + DX31 * y0 - DY31 * x0;

                        ///
                        float x_fraction ;
                        float z_start ;
                        float z_step ;
                        float linear_varyings_start[MARasterizerBase::MAX_VARYING];
                        float linear_varyings_step[MARasterizerBase::MAX_VARYING];

                        float inv_w_start;
                        float inv_w_step;
                        float persp_varyings_start[MARasterizerBase::MAX_VARYING];
                        float persp_varyings_step[MARasterizerBase::MAX_VARYING];
                        int i_ = 0;
                        ///
                        for (int iy = 0; iy <  q; iy++)
                        {
                            int CX1 = CY1;
                            int CX2 = CY2;
                            int CX3 = CY3;

                            ///
                            //if (CX1 > 0 && CX2 > 0 && CX3 > 0)

                            ///
                            ///for (int ix = x; ix < x + q; ix++)
                            //////////////////////////////////////////////////////////////////////
                            int xl = int(x_left[iy]);
                            if (xl >= x+q)
                            {
                                CY1 += FDX12;
                                CY2 += FDX23;
                                CY3 += FDX31;
                                continue;
                            }
                            xl = xl >= x && xl < x+q? xl : x;
                            int ix=x;
                            CX1 -= FDY12 * (xl-ix);
                            CX2 -= FDY23 * (xl-ix);
                            CX3 -= FDY31 * (xl-ix);
                            ix = xl;
                            //for (;ix < xl;++ix)
                            //{
                            //    CX1 -= FDY12;
                            //    CX2 -= FDY23;
                            //    CX3 -= FDY31;
                            //}

                            {
                                x_fraction = (xl-x_left[iy]) * x_step[iy];/// x_fraction (- [0,1]
                                //x_fraction = clamp(x_fraction,0.f,1.f);//x_fraction > 1.f ? 1.f:x_fraction;
                                x_fraction = x_fraction>1.f ? 1.f:x_fraction;
                                assert(! (x_fraction > 1.1f || x_fraction < -0.1f));
                                if (x_fraction > 1.1f || x_fraction < -0.1f)
                                    assert(false);
                                z_start = lerp(z_left[iy],z_right[iy],x_fraction);
                                z_step= x_step[iy] *(z_right[iy] - z_left[iy]);
                                inv_w_start = lerp(inv_w_left[iy],inv_w_right[iy],x_fraction);//x_fraction * (inv_w_right[iy] - inv_w_left[iy]);
                                inv_w_step = x_step[iy] * (inv_w_right[iy] - inv_w_left[iy]);

                                i_= 0;
                                DUFFS_DEVICE(8,int,MARasterizerBase::Vertex::linear_var_cnt,
                                             linear_varyings_start[i_]=
                                                 lerp(linear_varyings_left[iy][i_],linear_varyings_right[iy][i_],x_fraction);
                                             ++i_;
                                            );
                                i_ = 0;
                                DUFFS_DEVICE(8,int,MARasterizerBase::Vertex::linear_var_cnt,
                                             linear_varyings_step[i_]=
                                                 (linear_varyings_right[iy][i_] - linear_varyings_left[iy][i_])*x_step[iy];
                                             ++i_;
                                            );
                                i_ = 0;
                                DUFFS_DEVICE(8,int,MARasterizerBase::Vertex::persp_var_cnt,
                                             persp_varyings_start[i_]=
                                                 lerp(persp_varyings_left[iy][i_],persp_varyings_right[iy][i_],x_fraction);
                                             ++i_;
                                            );
                                i_ = 0;
                                DUFFS_DEVICE(8,int,MARasterizerBase::Vertex::persp_var_cnt,
                                             persp_varyings_step[i_]=
                                                 (persp_varyings_right[iy][i_] - persp_varyings_left[iy][i_])*x_step[iy];
                                             ++i_;
                                            );
                            }
                            ////////////////////////////////////////////////////////////////////
                            for (int ix = xl;ix < x + q; ix++)
                            {
                                if (CX1 > 0 && CX2 > 0 && CX3 > 0)
                                {
                                    //buffer[ix] = 0x0000007F;<< // Blue
                                    ///
                                    {

                                    }
                                    ///
                                    {
                                        FragmentData fragment;
                                        fragment.z = z_start;
                                        i_ = 0 ;
                                        DUFFS_DEVICE(8,int,MARasterizerBase::Vertex::linear_var_cnt,
                                                     fragment.linear_varyings[i_] =linear_varyings_start[i_];
                                                     ++i_;
                                                    );
                                        i_=0;
                                        DUFFS_DEVICE(8,int,MARasterizerBase::Vertex::persp_var_cnt,
                                                     fragment.perspective_varyings[i_] =16.f * persp_varyings_start[i_] / (inv_w_start);
                                                     ++i_;
                                                    );
                                        FragSpan::single_fragment(ix,iy + y,fragment);
                                    }
                                    {
                                        z_start += z_step;
                                        inv_w_start += inv_w_step;
                                        i_ = 0;
                                        DUFFS_DEVICE(8,int,MARasterizerBase::Vertex::linear_var_cnt,
                                                     linear_varyings_start[i_]+=linear_varyings_step[i_];
                                                     ++i_;
                                                    );

                                        i_= 0 ;
                                        DUFFS_DEVICE(8,int,MARasterizerBase::Vertex::persp_var_cnt,
                                                     persp_varyings_start[i_] += persp_varyings_step[i_];
                                                     ++i_;
                                                    );

                                    }
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
            if (Vertex::persp_var_cnt > 0)
                return triangle_template_fixed_point_persp<FragSpan>(v1,v2,v3);
            else
            {
                return triangle_template_fixed_point<FragSpan>(v1,v2,v3);
            }
        }
    };
}
#endif
