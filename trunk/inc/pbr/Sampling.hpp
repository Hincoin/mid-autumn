#ifndef _MA_INCLUDED_SAMPLING_HPP_
#define _MA_INCLUDED_SAMPLING_HPP_

#include "MAMath.hpp"
#include "ptr_var.hpp"
namespace ma{

	namespace sampler{
		DECL_FUNC(bool,getNextSample,1)
		DECL_FUNC(int,totalSamples,0)
		DECL_FUNC(int,roundSize,1)
	}
	BEGIN_CRTP_INTERFACE(Sampler)
		ADD_CRTP_INTERFACE_TYPEDEF(sample_t)
protected:
	int x_pixel_start,x_pixel_end,y_pixel_start,y_pixel_end;
	int samples_per_pixel;
public:
		Sampler(int xs,int xe,int ys,int ye,int spp)
			:x_pixel_start(xs),x_pixel_end(xe),
			y_pixel_start(ys),y_pixel_end(ye),samples_per_pixel(spp){}
		CRTP_METHOD(bool,getNextSample,1,( I_(sample_t&,s)));
		int totalSamples()const{return samples_per_pixel * (x_pixel_end-x_pixel_start) * (y_pixel_end - y_pixel_start);}
		CRTP_CONST_METHOD(int,roundSize,1,( I_(int ,size)));
		CRTP_METHOD(derived_type*,subdivide,1,(I_(unsigned&,count)));
	END_CRTP_INTERFACE

 	template<typename Conf>
	class ISampler:public Sampler<ISampler<Conf>,typename Conf::interface_config>
	{
		public:
		typedef Sampler<ISampler<Conf>,typename Conf::interface_config> parent_type;
		ADD_SAME_TYPEDEF(Conf,sample_t);
		ISampler(int xs,int xe,int ys,int ye,int spp):parent_type(xs,xe,ys,ye,spp){}
		virtual bool getNextSampleImpl(sample_t& s) = 0;	
		virtual int totalSamples()const{return parent_type::totalSamples();}
		virtual int roundSize(int size) = 0;
		virtual ~ISampler(){}
	};
	template<typename Conf>
	struct CameraSample{
		ADD_SAME_TYPEDEF(Conf,scalar_t);
		typedef CameraSample<Conf> class_type;
		scalar_t image_x,image_y;
		scalar_t lens_u,lens_v;
		scalar_t time;
	};
	template<typename Conf>
	struct IntegratorSample{
		ADD_SAME_TYPEDEF(Conf,scalar_t);

		std::vector<unsigned> n1D,n2D;
		scalar_t **oneD,**twoD;

		unsigned add1D(unsigned n){n1D.push_back(n);return n1D.size()-1;}
		unsigned add2D(unsigned n){n2D.push_back(n);return n2D.size()-1;}
		~IntegratorSample(){
			if (oneD)
			{
				::free(oneD[0]);
				::free(oneD);
			}
		}

	};
	template<typename Conf>
	struct Sample:CameraSample<Conf>,IntegratorSample<Conf>{
		ADD_SAME_TYPEDEF(Conf,scalar_t);
		typedef Sample<Conf> class_type;
		typedef CameraSample<Conf> camera_sample_t;
		typedef IntegratorSample<Conf> integrator_sample_t;
	private:
		Sample(const Sample<Conf>&);
		Sample<Conf>& operator=(const Sample<Conf>&);
		Sample()
		{}
	public:
		template<typename SIPtr,typename VIPtr,typename SPtr>
		static class_type* make_sample(SIPtr surf,VIPtr vol,const SPtr s)
		{
			class_type* sample = new class_type;
			integrator::requestSamples(surf,ref(*sample), s);
			//vol->requestSamples(this, s);
			// Allocate storage for sample pointers
			size_t nPtrs = sample->n1D.size() + sample->n2D.size();
			if (!nPtrs) {
				sample->oneD = sample->twoD = 0;
				return sample;
			}
			sample->oneD = (scalar_t **)malloc(nPtrs * sizeof(scalar_t *));
			sample->twoD = sample->oneD + sample->n1D.size();
			// Compute total number of sample values needed
			size_t totSamples = 0;
			for (size_t i = 0; i < sample->n1D.size(); ++i)
				totSamples += sample->n1D[i];
			for (size_t i = 0; i < sample->n2D.size(); ++i)
				totSamples += 2 * sample->n2D[i];
			// Allocate storage for sample values
			scalar_t *mem = (scalar_t *)malloc(totSamples *
				sizeof(scalar_t));
			for (unsigned i = 0; i < sample->n1D.size(); ++i) {
				sample->oneD[i] = mem;
				mem += sample->n1D[i];
			}
			for (unsigned i = 0; i < sample->n2D.size(); ++i) {
				sample->twoD[i] = mem;
				mem += 2 * sample->n2D[i];
			}
			return sample;
		}
		const camera_sample_t& cameraSample()const{
			return *this;
		}
		const integrator_sample_t* integratorSample()const{
			return this;
		}
		//private:

	};
	namespace filter{
		DECL_FUNC_NEST(scalar_t,evaluate,2)
	}
	BEGIN_CRTP_INTERFACE(Filter)
public:
		ADD_CRTP_INTERFACE_TYPEDEF(scalar_t)
		// Filter Interface
		Filter(scalar_t xw, scalar_t yw)
			: xWidth(xw), yWidth(yw), invXWidth(reciprocal(xw)),
			invYWidth(reciprocal(yw)) {
		}
		CRTP_CONST_METHOD(scalar_t,evaluate,2,( I_(scalar_t,x), I_(scalar_t , y)));
		// Filter Public Data
		const scalar_t xWidth, yWidth;
		const scalar_t invXWidth, invYWidth;
	END_CRTP_INTERFACE
}

namespace ma{
namespace details{
template<typename F> struct filter_creator;
template<typename S> struct sampler_creator;
}
template<typename F>
F* create_filter(const ParamSet& param)
{return details::filter_creator<F>()(param);}
template<typename S,typename FP>
S* create_sampler(const ParamSet& param,FP film)
{return details::sampler_creator<S>()(param,film);}
}

namespace ma{

	// Sampling Inline Functions

	inline float VanDerCorput(unsigned n, unsigned scramble) {
		n = (n << 16) | (n >> 16);
		n = ((n & 0x00ff00ff) << 8) | ((n & 0xff00ff00) >> 8);
		n = ((n & 0x0f0f0f0f) << 4) | ((n & 0xf0f0f0f0) >> 4);
		n = ((n & 0x33333333) << 2) | ((n & 0xcccccccc) >> 2);
		n = ((n & 0x55555555) << 1) | ((n & 0xaaaaaaaa) >> 1);
		n ^= scramble;
		return (float)n / (float)0x100000000LL;
	}
	inline float Sobol2(unsigned n, unsigned scramble) {
		for (unsigned v = 1 << 31; n != 0; n >>= 1, v ^= v >> 1)
			if (n & 0x1) scramble ^= v;
		return (float)scramble / (float)0x100000000LL;
	}
	inline void Sample02(unsigned n, unsigned scramble[2],
		float sample[2]) {
			sample[0] = VanDerCorput(n, scramble[0]);
			sample[1] = Sobol2(n, scramble[1]);
	}
	inline float
		LarcherPillichshammer2(unsigned n, unsigned scramble) {
			for (unsigned v = 1 << 31; n != 0; n >>= 1, v |= v >> 1)
				if (n & 0x1) scramble ^= v;
			return (float)scramble / (float)0x100000000LL;
	}
	inline void Shuffle(float *samp, size_t count, size_t dims) {
		using std::swap;
		for (size_t i = 0; i < count; ++i) {
			unsigned other = RandomUInt() % count;
			for (size_t j = 0; j < dims; ++j)
				swap(samp[dims*i + j], samp[dims*other + j]);
		}
	}
	inline void LDShuffleScrambled1D(size_t nSamples,
		size_t nPixel, float *samples) {
			unsigned scramble = RandomUInt();
			for (unsigned i = 0; i < nSamples * nPixel; ++i)
				samples[i] = VanDerCorput(i, scramble);
			for (unsigned i = 0; i < nPixel; ++i)
				Shuffle(samples + i * nSamples, nSamples, 1);
			Shuffle(samples, nPixel, nSamples);
	}
	inline void LDShuffleScrambled2D(size_t nSamples,
		size_t nPixel, float *samples) {
			unsigned scramble[2] = { RandomUInt(), RandomUInt() };
			for (unsigned i = 0; i < nSamples * nPixel; ++i)
				Sample02(i, scramble, &samples[2*i]);
			for (unsigned i = 0; i < nPixel; ++i)
				Shuffle(samples + 2 * i * nSamples, nSamples, 2);
			Shuffle(samples, nPixel, 2 * nSamples);
	}
}
#endif
