#ifndef __INCLUDE_MA_FRAGMENT_PROCESSOR_HPP__
#define __INCLUDE_MA_FRAGMENT_PROCESSOR_HPP__

#include "MARasterizer.hpp"
namespace ma{

	template <typename FragmentShader>
	struct SpanDrawerBase {
		static const int AFFINE_LENGTH = 24;

		static MARasterizerBase::FragmentData compute_step_al(
			const MARasterizerBase::FragmentData &fdl, 
			const MARasterizerBase::FragmentData &fdr)
		{
			MARasterizerBase::FragmentData r;

			if (FragmentShader::interpolate_z)
				r.z = (fdr.z - fdl.z) / AFFINE_LENGTH;

			int i = 0;
			DUFFS_DEVICE(8,int,(FragmentShader::varying_count),(r.varyings[i] = (fdr.varyings[i] - fdl.varyings[i]) / AFFINE_LENGTH; ++i;));
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
			using namespace detail;

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

	template <typename FragmentShader>
	struct GenericSpanDrawer : public SpanDrawerBase<FragmentShader> {
		static void affine_span(
			int x, 
			int y, 
			MARasterizerBase::FragmentData fd, 
			const MARasterizerBase::FragmentData &step, 
			unsigned n)
		{
			//DUFFS_DEVICE8(
			//	/**/,
			//	FragmentShader::single_fragment(x++, y, fd);
			//FRAGMENTDATA_APPLY(FragmentShader, fd, +=, step),
			//	n,
			//	/**/)
		}
	};

	template <typename FragmentShader>
	struct SpanDrawer16BitColorAndDepth : public SpanDrawerBase<FragmentShader> {
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