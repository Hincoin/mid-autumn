#ifndef _MA_INCLUDED_LDSAMPLER_HPP_
#define _MA_INCLUDED_LDSAMPLER_HPP_

#include "Sampling.hpp"

#include "Sampling.hpp"
//#include "ParamSet.hpp"
#include "Film.hpp"
namespace ma{



	// LDSampler Declarations
	template<typename Conf>
	class LDSampler : public Sampler<LDSampler<Conf>,typename Conf::interface_config> {
		friend class Sampler<LDSampler<Conf>,typename Conf::interface_config>;
		typedef Sampler<LDSampler<Conf>,typename Conf::interface_config> parent_type;
		typedef LDSampler<Conf> class_type;
	public:
		ADD_SAME_TYPEDEF(Conf,sample_t);
		ADD_SAME_TYPEDEF(sample_t,scalar_t);

		// LDSampler Public Methods
		LDSampler(int xstart, int xend,
			int ystart, int yend,
			int nsamp);
		~LDSampler() {
			delete[] imageSamples;
			for (size_t i = 0 ;i < n1D;++i)
				delete [] oneDSamples[i];
			for (size_t i = 0;i < n2D; ++i)
				delete [] twoDSamples[i];
			delete[] oneDSamples;
			delete[] twoDSamples;
			if (subdivided_)
			{
				for(int i = 0 ;i < subdivide_count_; ++i)
				{
					(static_cast<class_type*>(subdivided_)+i)->~class_type();
				}
				subdivide_count_ = 0;
				::free(subdivided_);
			}
		}
	private:
		int roundSizeImpl(int size) const {
			return RoundUpPow2(size);
		}
		bool getNextSampleImpl(sample_t &sample);
		class_type* subdivideImpl(unsigned& count);
	private:
		// LDSampler Private Data
		int xPos, yPos, pixelSamples;
		int samplePos;
		scalar_t *imageSamples, *lensSamples, *timeSamples;
		scalar_t **oneDSamples, **twoDSamples;
		size_t n1D,n2D;

		void* subdivided_;
		int subdivide_count_;
	};
	// LDSampler Method Definitions
	template<typename Conf>
	LDSampler<Conf>::LDSampler(int xstart, int xend,
		int ystart, int yend, int ps)
		: parent_type(xstart, xend, ystart, yend, RoundUpPow2(ps))
		,subdivided_(0),subdivide_count_(0) {
			xPos = parent_type::x_pixel_start - 1;
			yPos = parent_type::y_pixel_start;
			if (!isPowerOf2(ps)) {
				/*Warning("Pixel samples being"
					"rounded up to power of 2");*/
				pixelSamples = RoundUpPow2(ps);
			}
			else
				pixelSamples = ps;
			samplePos = pixelSamples;
			oneDSamples = twoDSamples = NULL;
			n1D = n2D = 0;
			imageSamples = new scalar_t[5*pixelSamples];
			lensSamples = imageSamples + 2*pixelSamples;
			timeSamples = imageSamples + 4*pixelSamples;
	}
	template<typename Conf>
	bool LDSampler<Conf>::getNextSampleImpl(typename Conf::sample_t& sample) {
		if (!oneDSamples) {
			// Allocate space for pixel's low-discrepancy sample tables
			oneDSamples = new scalar_t *[sample.n1D.size()];
			n1D = sample.n1D.size();
			for (unsigned i = 0; i < sample.n1D.size(); ++i)
				oneDSamples[i] = new scalar_t[sample.n1D[i] *	pixelSamples];
			twoDSamples = new scalar_t *[sample.n2D.size()];
			n2D = sample.n2D.size();
			for (unsigned i = 0; i < sample.n2D.size(); ++i)
				twoDSamples[i] = new scalar_t[2 * sample.n2D[i] *
				pixelSamples];
		}
		if (samplePos == pixelSamples) {
			// Advance to next pixel for low-discrepancy sampling
			if (++xPos == parent_type::x_pixel_end) {
				xPos = parent_type::x_pixel_start;
				++yPos;
			}
			if (yPos == parent_type::y_pixel_end)
				return false;
			samplePos = 0;
			// Generate low-discrepancy samples for pixel
			LDShuffleScrambled2D(1, pixelSamples, imageSamples);
			LDShuffleScrambled2D(1, pixelSamples, lensSamples);
			LDShuffleScrambled1D(1, pixelSamples, timeSamples);
			for (unsigned i = 0; i < sample.n1D.size(); ++i)
				LDShuffleScrambled1D(sample.n1D[i], pixelSamples,
				oneDSamples[i]);
			for (unsigned i = 0; i < sample.n2D.size(); ++i)
				LDShuffleScrambled2D(sample.n2D[i], pixelSamples,
				twoDSamples[i]);
		}
		
		// Copy low-discrepancy samples from tables
		sample.image_x = xPos + imageSamples[2*samplePos];
		sample.image_y = yPos + imageSamples[2*samplePos+1];
		sample.time = timeSamples[samplePos];
		sample.lens_u = lensSamples[2*samplePos];
		sample.lens_v = lensSamples[2*samplePos+1];
		for (unsigned i = 0; i < sample.n1D.size(); ++i) {
			size_t startSamp = sample.n1D[i] * samplePos;
			for (unsigned j = 0; j < sample.n1D[i]; ++j)
				sample.oneD[i][j] = oneDSamples[i][startSamp+j];
		}
		for (unsigned i = 0; i < sample.n2D.size(); ++i) {
			size_t startSamp = 2 * sample.n2D[i] * samplePos;
			for (unsigned j = 0; j < 2*sample.n2D[i]; ++j)
				sample.twoD[i][j] = twoDSamples[i][startSamp+j];
		}
		++samplePos;
		return true;
	}

	template<typename Conf>
	typename LDSampler<Conf>::class_type* 
		LDSampler<Conf>::subdivideImpl(unsigned& count)
	{
		//free up first
		if (subdivided_)
		{
			for(int i = 0 ;i < subdivide_count_; ++i)
			{
				(static_cast<class_type*>(subdivided_)+i)->~class_type();
			}
			::free(subdivided_);
			subdivided_ = 0;
			subdivide_count_ = 0;
		}
		int y_step = (parent_type::y_pixel_end - parent_type::y_pixel_start + count-1)/count;
		int y_s = parent_type::y_pixel_start;
		subdivide_count_ = count;
		subdivided_ = ::malloc(subdivide_count_ * sizeof(class_type));
		int i = 0;
		for(;y_s < parent_type::y_pixel_end; y_s += y_step)
		{
			new ((static_cast<class_type*>(subdivided_)+(i++))) 
				class_type(parent_type::x_pixel_start,parent_type::x_pixel_end,y_s,y_s+y_step,parent_type::samples_per_pixel) ;
		}
		count=i;
		subdivide_count_ = count;
		return (class_type*)subdivided_;
	}
}
namespace ma{
	MAKE_TYPE_STR_MAP(1,LDSampler,lowdiscrepancy)
namespace details
{
template<typename C>
	struct sampler_creator<LDSampler<C> >
	{
		template<typename FP>
LDSampler<C>* operator()(const ParamSet& param,FP film )const
{

	typedef LDSampler<C> sampler_t;
	 ////////////////////////////////////////////////////////////////////////////
	 // Initialize common sampler parameters
	 int xstart, xend, ystart, yend;
	 film->getSampleExtent(xstart, xend, ystart, yend);
	 int nsamp = param.as<int>("pixelsamples",4);
	 return new sampler_t(xstart, xend, ystart, yend, nsamp);

	 //////////////////////////////////////////////////////////////////////////
	
}	
	};
}

}
#endif
