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

        StencilFunctor(int r,int m,StencilFun sf):ref(r),mask(m),stencil_func(sf){}
        bool operator()(Z z,Z* z_b,S* s_b)const
        {
            return (*stencil_func)(z,z_b,ref,mask,s_b);
        }
        int ref;
        int mask;
        StencilFun stencil_func;
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
        static bool sort_triangle_vertex(const MARasterizerBase::Vertex& v1,
           const MARasterizerBase::Vertex& v2,
           const MARasterizerBase::Vertex& v3,
           const MARasterizerBase::Vertex*& tri_v1,
           const MARasterizerBase::Vertex*& tri_v2,
           const MARasterizerBase::Vertex*& tri_v3)
        {
            tri_v1 = (&v1);
            tri_v2 = (&v2);
            tri_v3 = (&v3);
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
			 assert( tri_v1->y <= tri_v2->y && tri_v2->y <= tri_v3->y );
            float ax = tri_v3->x - tri_v2->x;
            float ay = tri_v3->y - tri_v2->y;
            float bx = tri_v1->x - tri_v2->x;
            float by = tri_v1->y - tri_v2->y;
			//because left up is (0,0) , so reverse it
            return ax * by - ay * bx  < 0;
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
