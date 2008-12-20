#ifndef __INCLUDE_MA_FRAGMENT_PROCESSOR_HPP__
#define __INCLUDE_MA_FRAGMENT_PROCESSOR_HPP__

#include "MARasterizer.hpp"
#include "DuffsDevice.hpp"
namespace ma
{

    template <typename FragmentShader>
    struct SpanDrawerBase
    {
        static const int AFFINE_LENGTH = 24;

        static MARasterizerBase::FragmentData compute_step_al(
            const MARasterizerBase::FragmentData &fdl,
            const MARasterizerBase::FragmentData &fdr)
        {
            MARasterizerBase::FragmentData r;

            if (FragmentShader::interpolate_z)
                r.z = (fdr.z - fdl.z) / AFFINE_LENGTH;

            int i = 0;
            //DUFFS_DEVICE(8,int,(FragmentShader::varying_count),(r.varyings[i] = (fdr.varyings[i] - fdl.varyings[i]) / AFFINE_LENGTH; ++i;); );
            //DUFFS_DEVICE8(
            //	int i = 0,
            //	r.varyings[i] = (fdr.varyings[i] - fdl.varyings[i]) / AFFINE_LENGTH; ++i,
            //	FragmentShader::varying_count
            //	/**/)

            return r;
        }

        static MARasterizerBase::FragmentData compute_step(
            const MARasterizerBase::FragmentData &fdl,
            const MARasterizerBase::FragmentData &fdr,
            int inv_delta)
        {
            //using namespace detail;

            MARasterizerBase::FragmentData r;

            //if (FragmentShader::interpolate_z)
            //	r.z = fixmul<16>(fdr.z - fdl.z, inv_delta);

            //DUFFS_DEVICE8(
            //	int i = 0,
            //	r.varyings[i] = fixmul<16>(fdr.varyings[i] - fdl.varyings[i], inv_delta); ++i,
            //	FragmentShader::varying_count,
            //	/**/)

            return r;
        }

        static MARasterizerBase::FragmentData fd_from_fds(const MARasterizerBase::FragmentDataPerspective &fd)
        {
            using namespace details;

            MARasterizerBase::FragmentData r = MARasterizerBase::FragmentData();

            //if (FragmentShader::interpolate_z)
            //	r.z = fd.fd.z;

            //if (FragmentShader::varying_count) {
            //	int w = detail::invert(fd.oow);

            //	DUFFS_DEVICE8(
            //		int i = 0,
            //		r.varyings[i] = fixmul<16>(fd.fd.varyings[i], w); ++i,
            //		FragmentShader::varying_count,
            //		/**/)
            //}

            return r;
        }

        static void perspective_span(
            int x,
            int y,
            const MARasterizerBase::FragmentDataPerspective &fd_in,
            const MARasterizerBase::FragmentDataPerspective &step,
            unsigned n)
        {
            //using namespace detail;

            //MARasterizerBase::FragmentDataPerspective fds[2];
            //FRAGMENTDATA_PERSPECTIVE_APPLY(FragmentShader, fds[0], = , fd_in);

            //MARasterizerBase::FragmentData fd[2];
            //fd[0] = fd_from_fds(fds[0]);

            //while (AFFINE_LENGTH <= static_cast<int>(n)) {
            //	FRAGMENTDATA_PERSPECTIVE_APPLY(FragmentShader, fds[1], = , fds[0])
            //		FRAGMENTDATA_PERSPECTIVE_APPLY(FragmentShader, fds[1], += AFFINE_LENGTH *, step)

            //		fd[1] = fd_from_fds(fds[1]);

            //	const MARasterizerBase::FragmentData step = compute_step_al(fd[0], fd[1]);

            //	FragmentShader::affine_span(x, y, fd[0], step, AFFINE_LENGTH);
            //	x += AFFINE_LENGTH; n -= AFFINE_LENGTH;

            //	FRAGMENTDATA_PERSPECTIVE_APPLY(FragmentShader, fds[0], =, fds[1])
            //		FRAGMENTDATA_APPLY(FragmentShader, fd[0], =, fd[1])
            //}

            //if (n) {
            //	const int inv_n = detail::invert(n << 16);

            //	FRAGMENTDATA_PERSPECTIVE_APPLY(FragmentShader, fds[1], = , fds[0])
            //		FRAGMENTDATA_PERSPECTIVE_APPLY(FragmentShader, fds[1], += n *, step)

            //		fd[1] = fd_from_fds(fds[1]);

            //	const MARasterizerBase::FragmentData step = compute_step(fd[0], fd[1], inv_n);

            //	FragmentShader::affine_span(x, y, fd[0], step, n);
            //}
        }
    };
    namespace details
    {
        struct only_z_tag{};
    }
    template<typename Z,typename S>
    struct StencilFunctor
    {
        typedef bool (*StencilFun)(Z,Z*,int,int,S*);
        StencilFun stencil_func;
        int ref;
        int mask;
        StencilFunctor(int r,int m,StencilFun sf):ref(r),mask(m),stencil_func(sf){}
        bool operator()(Z z,Z* z_b,S* s_b)const
        {
            return (*stencil_func)(z,z_b,ref,mask,s_b);
        }
    };
    template<>
    struct StencilFunctor<details::only_z_tag,details::only_z_tag>
    {
        template<typename Z,typename S>
        bool operator()(Z z,Z* z_b,S* s_b)const//only z test
        {
            return z < *z_b;
        }
    };


    template <typename FragmentShader>
    struct GenericSpanDrawer : public SpanDrawerBase<FragmentShader>
    {


        static void flat_shading();
        static void prepare_gauround_triangle(
           const MARasterizerBase::Vertex& v1,
           const MARasterizerBase::Vertex& v2,
           const MARasterizerBase::Vertex& v3,
            MARasterizerBase::Vertex*& tri_v1,
            MARasterizerBase::Vertex*& tri_v2,
            MARasterizerBase::Vertex*& tri_v3
        )
        {
            typedef MARasterizerBase::Vertex* VertexPtr;
             tri_v1 = const_cast<VertexPtr>(&v1);
            tri_v2 = const_cast<VertexPtr>(&v2);
            tri_v3 = const_cast<VertexPtr>(&v3);
            //sort by y axis
            if (tri_v1->y < tri_v2->y) //v1 < v2
            {
                if (tri_v1->y < tri_v3->y)
                {
                    if (tri_v2->y < tri_v3->y);
                    else std::swap(tri_v2,tri_v3);
                }
                else//tri_v2->y > tri_v1->y > tri_v3->y
                {
                    std::swap(tri_v1,tri_v3);//tri_v1 be smallest
                    std::swap(tri_v2,tri_v3);//tri_v3 be biggest
                }
            }
            else if (tri_v1->y < tri_v3->y) // v2 < v1 < v3
            {
                std::swap(tri_v1,tri_v2);
            }
            else//v1 > v2, v1 > v3
            {
                if (tri_v2->y > tri_v3->y)//v1> v2 > v3
                {
                    std::swap(tri_v3,tri_v1);
                }
                else //v1 > v3 > v2
                {
                    std::swap(tri_v1,tri_v2);
                    std::swap(tri_v2,tri_v3);
                }

            }
            /*
            int i = 0;
            DUFFS_DEVICE(8,int,MARasterizerBase::Vertex::persp_var_cnt,
            tri_v1->perspective_varyings[i]/=tri_v1->w;++i;
            );
            i=0;
            DUFFS_DEVICE(8,int,MARasterizerBase::Vertex::persp_var_cnt,
            tri_v2->perspective_varyings[i]/=tri_v2->w;++i;
            );
            i=0;
            DUFFS_DEVICE(8,int,MARasterizerBase::Vertex::persp_var_cnt,
            tri_v3->perspective_varyings[i]/=tri_v3->w;++i;
            );
            */
            assert( tri_v1->y <= tri_v2->y && tri_v2->y <= tri_v3->y );
        }
        struct perspective_correct_tag{};
        struct linear_tag{};
        template<typename Z,typename S,typename SF>
        static bool gauround_shading(int x,int y,
                                     const MARasterizerBase::Vertex& tri_v1,
                                     const MARasterizerBase::Vertex& tri_v2,
                                     const MARasterizerBase::Vertex& tri_v3,
                                     Z* z_b,
                                     S* s_b,
                                     const SF& stencil_func,
                                     MARasterizerBase::FragmentData& fragment,
                                     const linear_tag&
                                    )
        {
            Z interpolated_z = 0;

            float linear_varyings12[MARasterizerBase::MAX_VARYING];//such as z,color
            float linear_varyings13[MARasterizerBase::MAX_VARYING];
            float linear_varyings23[MARasterizerBase::MAX_VARYING];
            //gauraud shading
            //in y axis
            if (y < tri_v2.y) // betwenn v1,v2
            {
                float t12 = linearStep(tri_v1.y,tri_v2.y,(float)y);
                float t13 = linearStep(tri_v1.y,tri_v3.y,(float)y);

                float z12 = lerp(tri_v1.z,tri_v2.z,t12);
                float z13 = lerp(tri_v1.z,tri_v3.z,t13);
                int i = 0;
                DUFFS_DEVICE(8,int,MARasterizerBase::Vertex::linear_var_cnt,
                             linear_varyings12[i]=lerp(tri_v1.linear_varyings[i],tri_v2.linear_varyings[i],t12);++i;
                            );
                float posx12 = lerp(tri_v1.x,tri_v2.x,t12);


                i = 0;
                DUFFS_DEVICE(8,int,MARasterizerBase::Vertex::linear_var_cnt,
                             linear_varyings13[i]=lerp(tri_v1.linear_varyings[i],tri_v3.linear_varyings[i],t13);++i;
                            );
                float posx13 = lerp(tri_v1.x,tri_v3.x,t13);


                if (posx12 < posx13)
                {
                    float tx = linearStep(posx12,posx13,(float)x);
                    {
                        interpolated_z = lerp(z12,z13,tx);
                        assert(interpolated_z >=0 );
                        if (interpolated_z>0 &&
                                //*z_b > interpolated_z
                                (stencil_func)(interpolated_z,z_b,s_b)
                           )
                            *(z_b) = interpolated_z;
                        else return false;
                    }
                    fragment.z = interpolated_z;
                    i = 0;
                    DUFFS_DEVICE(8,int,MARasterizerBase::Vertex::linear_var_cnt,
                                 fragment.linear_varyings[i]=lerp(linear_varyings12[i],linear_varyings13[i],tx);++i;
                                );
                }
                else
                {
                    float tx = linearStep(posx13,posx12,(float)x);
                    {
                        interpolated_z =lerp(z13,z12,tx);
                        assert(interpolated_z >=0 );
                        if (interpolated_z>0 &&
                                (stencil_func)(interpolated_z,z_b,s_b)
                                //*z_b > interpolated_z
                           )
                            *(z_b) = interpolated_z;
                        else return false;
                    }
                    fragment.z = interpolated_z;
                    i = 0;
                    DUFFS_DEVICE(8,int,MARasterizerBase::Vertex::linear_var_cnt,
                                 fragment.linear_varyings[i]=lerp(linear_varyings13[i],linear_varyings12[i],tx);++i;
                                );
                }
            }
            else
            {
                float t23 = linearStep(tri_v2.y,tri_v3.y,(float)y);
                float t13 = linearStep(tri_v1.y,tri_v3.y,(float)y);

                float z23 = lerp(tri_v2.z,tri_v3.z, t23);
                float z13 = lerp(tri_v1.z,tri_v3.z, t13);
                int i = 0;
                DUFFS_DEVICE(8,int,MARasterizerBase::Vertex::linear_var_cnt,
                             linear_varyings23[i]=lerp(tri_v2.linear_varyings[i],tri_v3.linear_varyings[i],t23);++i;
                            );
                float posx23 = lerp(tri_v2.x,tri_v3.x,t23);

                i = 0;
                DUFFS_DEVICE(8,int,MARasterizerBase::Vertex::linear_var_cnt,
                             linear_varyings13[i]=lerp(tri_v1.linear_varyings[i],tri_v3.linear_varyings[i],t13);++i;
                            );
                float posx13 = lerp(tri_v1.x,tri_v3.x,t13);

                if (posx23 < posx13)
                {

                    float tx = linearStep(posx23,posx13,(float)x);

                    {
                        interpolated_z = lerp(z23,z13, tx);
                        assert(interpolated_z >=0 );
                        if (interpolated_z>0 && //*z_b > interpolated_z
                                (stencil_func)(interpolated_z,z_b,s_b)
                           )
                            *(z_b) = interpolated_z;
                        else return false;
                    }
                    fragment.z = interpolated_z;
                    i = 0;
                    DUFFS_DEVICE(8,int,MARasterizerBase::Vertex::linear_var_cnt,
                                 fragment.linear_varyings[i]=lerp(linear_varyings23[i],linear_varyings13[i],tx);++i;
                                );
                }
                else
                {
                    float tx = linearStep(posx13,posx23,(float)x);
                    {
                        interpolated_z = lerp(z13,z23,tx);
                        assert(interpolated_z >=0 );
                        if (interpolated_z>0 &&
                                (stencil_func)(interpolated_z,z_b,s_b)
                           )
                            *(z_b) = interpolated_z;
                        else return false;
                    }
                    fragment.z = interpolated_z;
                    i = 0;
                    DUFFS_DEVICE(8,int,MARasterizerBase::Vertex::linear_var_cnt,
                                 fragment.linear_varyings[i]=lerp(linear_varyings13[i],linear_varyings23[i],tx);++i;
                                );
                }
            }
            return true;
        }
        template<typename Z,typename S,typename SF>
        static bool gauround_shading(int x,int y,
                                     const MARasterizerBase::Vertex& tri_v1,
                                     const MARasterizerBase::Vertex& tri_v2,
                                     const MARasterizerBase::Vertex& tri_v3,
                                     Z* z_b,
                                     S* s_b,
                                     const SF& stencil_func,
                                     MARasterizerBase::FragmentData& fragment,
                                     const perspective_correct_tag&
                                    )
        {
            Z interpolated_z = 0;
            float perspective_varyings12[MARasterizerBase::MAX_VARYING];//such as texture coordinate
            float perspective_varyings13[MARasterizerBase::MAX_VARYING];
            float perspective_varyings23[MARasterizerBase::MAX_VARYING];
            float linear_varyings12[MARasterizerBase::MAX_VARYING];//such as z,color
            float linear_varyings13[MARasterizerBase::MAX_VARYING];
            float linear_varyings23[MARasterizerBase::MAX_VARYING];
            //gauraud shading
            //in y axis
            if (y < tri_v2.y) // betwenn v1,v2
            {
                float t12 = linearStep(tri_v1.y,tri_v2.y,(float)y);
                float t13 = linearStep(tri_v1.y,tri_v3.y,(float)y);

                float z12 = lerp(tri_v1.z,tri_v2.z,t12);
                float z13 = lerp(tri_v1.z,tri_v3.z,t13);
                int i = 0;
                DUFFS_DEVICE(8,int,MARasterizerBase::Vertex::linear_var_cnt,
                             linear_varyings12[i]=lerp(tri_v1.linear_varyings[i],tri_v2.linear_varyings[i],t12);++i;
                            );
                i = 0;
                DUFFS_DEVICE(8,int,MARasterizerBase::Vertex::persp_var_cnt,
                             perspective_varyings12[i]=lerp(tri_v1.perspective_varyings[i],tri_v2.perspective_varyings[i],t12);++i;
                            );
                float w12 = lerp(tri_v1.inv_w,tri_v2.inv_w,t12);

                float posx12 = lerp(tri_v1.x,tri_v2.x,t12);


                i = 0;
                DUFFS_DEVICE(8,int,MARasterizerBase::Vertex::linear_var_cnt,
                             linear_varyings13[i]=lerp(tri_v1.linear_varyings[i],tri_v3.linear_varyings[i],t13);++i;
                            );
                i = 0;
                DUFFS_DEVICE(8,int,MARasterizerBase::Vertex::persp_var_cnt,
                             perspective_varyings13[i]=lerp(tri_v1.perspective_varyings[i],tri_v3.perspective_varyings[i],t13);++i;
                            );
                float w13 = lerp( tri_v1.inv_w, tri_v3.inv_w,t13);

                float posx13 = lerp(tri_v1.x,tri_v3.x,t13);


                if (posx12 < posx13)
                {
                    float tx = linearStep(posx12,posx13,(float)x);
                    {
                        interpolated_z = lerp(z12,z13,tx);
                        assert(interpolated_z >=0 );
                        if (interpolated_z>0 &&
                                //*z_b > interpolated_z
                                (stencil_func)(interpolated_z,z_b,s_b)
                           )
                            *(z_b) = interpolated_z;
                        else return false;
                    }
                    fragment.z = interpolated_z;
                    i = 0;
                    DUFFS_DEVICE(8,int,MARasterizerBase::Vertex::linear_var_cnt,
                                 fragment.linear_varyings[i]=lerp(linear_varyings12[i],linear_varyings13[i],tx);++i;
                                );
                    float w =lerp(w12,w13,tx);
                    i = 0;
                    DUFFS_DEVICE(8,int,MARasterizerBase::Vertex::persp_var_cnt,
                                 fragment.perspective_varyings[i]=lerp(perspective_varyings12[i],perspective_varyings13[i],tx)/w;++i;
                                );
                }
                else
                {
                    float tx = linearStep(posx13,posx12,(float)x);
                    {
                        interpolated_z =lerp(z13,z12,tx);
                        assert(interpolated_z >=0 );
                        if (interpolated_z>0 &&
                                (stencil_func)(interpolated_z,z_b,s_b)
                                //*z_b > interpolated_z
                           )
                            *(z_b) = interpolated_z;
                        else return false;
                    }
                    fragment.z = interpolated_z;
                    i = 0;
                    DUFFS_DEVICE(8,int,MARasterizerBase::Vertex::linear_var_cnt,
                                 fragment.linear_varyings[i]=lerp(linear_varyings13[i],linear_varyings12[i],tx);++i;
                                );
                    float w =lerp(w13,w12,tx);
                    i = 0;
                    DUFFS_DEVICE(8,int,MARasterizerBase::Vertex::persp_var_cnt,
                                 fragment.perspective_varyings[i]=lerp(perspective_varyings13[i],perspective_varyings12[i],tx)/w;++i;
                                 );
                }
            }
            else
            {
                float t23 = linearStep(tri_v2.y,tri_v3.y,(float)y);
                float t13 = linearStep(tri_v1.y,tri_v3.y,(float)y);

                float z23 = lerp(tri_v2.z,tri_v3.z, t23);
                float z13 = lerp(tri_v1.z,tri_v3.z, t13);
                int i = 0;
                DUFFS_DEVICE(8,int,MARasterizerBase::Vertex::linear_var_cnt,
                             linear_varyings23[i]=lerp(tri_v2.linear_varyings[i],tri_v3.linear_varyings[i],t23);++i;
                            );
                i = 0;
                DUFFS_DEVICE(8,int,MARasterizerBase::Vertex::persp_var_cnt,
                             perspective_varyings23[i]=lerp(tri_v2.perspective_varyings[i],tri_v3.perspective_varyings[i],t23);++i;
                            );
                float w23 = lerp(tri_v2.inv_w,tri_v3.inv_w,t23);
                float posx23 = lerp(tri_v2.x,tri_v3.x,t23);

                i = 0;
                DUFFS_DEVICE(8,int,MARasterizerBase::Vertex::linear_var_cnt,
                             linear_varyings13[i]=lerp(tri_v1.linear_varyings[i],tri_v3.linear_varyings[i],t13);++i;
                            );
                i =  0;
                DUFFS_DEVICE(8,int,MARasterizerBase::Vertex::persp_var_cnt,
                             perspective_varyings13[i]=lerp(tri_v1.perspective_varyings[i],tri_v3.perspective_varyings[i],t13);++i;
                            );
                float w13 = lerp( tri_v1.inv_w, tri_v3.inv_w,t13);
                float posx13 = lerp(tri_v1.x,tri_v3.x,t13);

                if (posx23 < posx13)
                {

                    float tx = linearStep(posx23,posx13,(float)x);

                    {
                        interpolated_z = lerp(z23,z13, tx);
                        assert(interpolated_z >=0 );
                        if (interpolated_z>0 && //*z_b > interpolated_z
                                (stencil_func)(interpolated_z,z_b,s_b)
                           )
                            *(z_b) = interpolated_z;
                        else return false;
                    }
                    fragment.z = interpolated_z;
                    i = 0;
                    DUFFS_DEVICE(8,int,MARasterizerBase::Vertex::linear_var_cnt,
                                 fragment.linear_varyings[i]=lerp(linear_varyings23[i],linear_varyings13[i],tx);++i;
                                );
                    float inv_w = lerp(w23,w13,tx);
                    i = 0;
                    DUFFS_DEVICE(8,int,MARasterizerBase::Vertex::persp_var_cnt,
                                fragment.perspective_varyings[i]=lerp(perspective_varyings23[i],perspective_varyings13[i],tx)/inv_w;++i;
                                );
                }
                else
                {
                    float tx = linearStep(posx13,posx23,(float)x);
                    {
                        interpolated_z = lerp(z13,z23,tx);
                        assert(interpolated_z >=0 );
                        if (interpolated_z>0 &&
                                (stencil_func)(interpolated_z,z_b,s_b)
                           )
                            *(z_b) = interpolated_z;
                        else return false;
                    }
                    fragment.z = interpolated_z;
                    i = 0;
                    DUFFS_DEVICE(8,int,MARasterizerBase::Vertex::linear_var_cnt,
                                 fragment.linear_varyings[i]=lerp(linear_varyings13[i],linear_varyings23[i],tx);++i;
                                );
                    float inv_w = lerp(w13,w23,tx);
                    i = 0;
                    DUFFS_DEVICE(8,int,MARasterizerBase::Vertex::persp_var_cnt,
                                 fragment.perspective_varyings[i]=lerp(perspective_varyings13[i],perspective_varyings23[i],tx)/inv_w;++i;
                                );
                }
            }
            return true;
        }
    };

    template <typename FragmentShader>
    struct SpanDrawer16BitColorAndDepth : public SpanDrawerBase<FragmentShader>
    {
        static void affine_span(
            int x,
            int y,
            MARasterizerBase::FragmentData fd,
            const MARasterizerBase::FragmentData &step,
            unsigned n)
        {
            //unsigned short *color16_pointer = static_cast<unsigned short*>(FragmentShader::color_pointer(x, y));
            //unsigned short *depth16_pointer = static_cast<unsigned short*>(FragmentShader::depth_pointer(x, y));

            //// using duffs device for loop unrolling can improve performance
            //// and seems useful even when using -funroll-loops in GCC.
            //// In a few tests the above was 5% faster than a normal while loop with -funroll-loops.

            //DUFFS_DEVICE16(
            //	/**/,
            //{
            //	FragmentShader::single_fragment(fd, *color16_pointer, *depth16_pointer);
            //	FRAGMENTDATA_APPLY(FragmentShader, fd, += , step);

            //	color16_pointer++;
            //	depth16_pointer++;
            //},
            //	n,
            //	/**/)
        }
    };
}

#endif
