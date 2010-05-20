#ifndef _MA_INCLUDED_COLOR_HPP_
#define _MA_INCLUDED_COLOR_HPP_

#include <iostream>
#include <limits>
#include <cmath>
#include "MAMath.hpp"
#include "serialization.hpp"
namespace ma{
	template<typename S = float,int C_S = 3>
	class Spectrum;
	template<typename S,int SN>
	Spectrum<S,SN> FromXYZ(float x, float y, float z); 
	template<typename S,int SN>
	std::ostream &operator<<(std::ostream &, const Spectrum<S,SN>&);

	// Spectrum Declarations
	template<typename S ,int C_S>
	class Spectrum:public serialization::serializable<Spectrum<S,C_S> > {
		typedef Spectrum<S,C_S> class_type;
		public:
		void serializeImpl(std::ostream& out)const
		{
			for(int i = 0;i < COLOR_SAMPLES;++i)
				serialization::serialize(c[i],out);
		}
		void deserializeImpl(std::istream& in)
		{
			for(int i = 0;i < COLOR_SAMPLES;++i)
				serialization::deserialize(c[i],in);
		}	
	public:
		static const int COLOR_SAMPLES = C_S;
	
		typedef S sample_type;
		// Spectrum Public Methods
		Spectrum(sample_type v = 0.f) {
			for (int i = 0; i < COLOR_SAMPLES; ++i)
				c[i] = v;
		}
		Spectrum(sample_type cs[COLOR_SAMPLES]) {
			for (int i = 0; i < COLOR_SAMPLES; ++i)
				c[i] = cs[i];
		}
		template<typename RealType>
		Spectrum(RealType v){
			for (int i = 0; i < COLOR_SAMPLES; ++i)
				c[i] = static_cast<sample_type>(v);
		};
		// move operation
		//////////////////////////////////////////////////////////////////////////
		//////////////////////////////////////////////////////////////////////////
		friend std::ostream &operator<< <>(std::ostream &, const class_type &);
		Spectrum &operator+=(const class_type &s2) {
			for (int i = 0; i < COLOR_SAMPLES; ++i)
				c[i] += s2.c[i];
			return *this;
		}
		Spectrum operator+(const class_type &s2) const {
			Spectrum ret = *this;
			for (int i = 0; i < COLOR_SAMPLES; ++i)
				ret.c[i] += s2.c[i];
			return ret;
		}
		Spectrum operator-(const Spectrum &s2) const {
			Spectrum ret = *this;
			for (int i = 0; i < COLOR_SAMPLES; ++i)
				ret.c[i] -= s2.c[i];
			return ret;
		}
		Spectrum operator/(const Spectrum &s2) const {
			Spectrum ret = *this;
			for (int i = 0; i < COLOR_SAMPLES; ++i)
				ret.c[i] /= s2.c[i];
			return ret;
		}
		Spectrum operator*(const Spectrum &sp) const {
			Spectrum ret = *this;
			for (int i = 0; i < COLOR_SAMPLES; ++i)
				ret.c[i] *= sp.c[i];
			return ret;
		}
		Spectrum &operator*=(const Spectrum &sp) {
			for (int i = 0; i < COLOR_SAMPLES; ++i)
				c[i] *= sp.c[i];
			return *this;
		}
		Spectrum operator*(sample_type a) const {
			Spectrum ret = *this;
			for (int i = 0; i < COLOR_SAMPLES; ++i)
				ret.c[i] *= a;
			return ret;
		}
		Spectrum &operator*=(sample_type a) {
			for (int i = 0; i < COLOR_SAMPLES; ++i)
				c[i] *= a;
			return *this;
		}
		friend inline
			Spectrum operator*(sample_type a, const Spectrum &s) {
				return s * a;
		}
		Spectrum operator/(sample_type a) const {
			return *this * reciprocal(a);
		}
		Spectrum &operator/=(sample_type a) {
			float inv = reciprocal(a);
			for (int i = 0; i < COLOR_SAMPLES; ++i)
				c[i] *= inv;
			return *this;
		}
		void AddWeighted(sample_type w, const Spectrum &s) {
			for (int i = 0; i < COLOR_SAMPLES; ++i)
				c[i] += w * s.c[i];
		}
		bool operator==(const Spectrum &sp) const {
			for (int i = 0; i < COLOR_SAMPLES; ++i)
				if (c[i] != sp.c[i]) return false;
			return true;
		}
		bool operator!=(const Spectrum &sp) const {
			return !(*this == sp);
		}
		bool black() const {
			for (int i = 0; i < COLOR_SAMPLES; ++i)
				if (c[i] != sample_type(0)) return false;
			return true;
		}
		Spectrum sqrt() const {
			Spectrum ret;
			for (int i = 0; i < COLOR_SAMPLES; ++i)
				ret.c[i] = std::sqrt(c[i]);
			return ret;
		}
		Spectrum pow(const Spectrum &e) const {
			Spectrum ret;
			for (int i = 0; i < COLOR_SAMPLES; ++i)
				ret.c[i] = c[i] > 0 ? powf(c[i], e.c[i]) : 0.f;
			return ret;
		}
		Spectrum operator-() const {
			Spectrum ret;
			for (int i = 0; i < COLOR_SAMPLES; ++i)
				ret.c[i] = -c[i];
			return ret;
		}
		friend Spectrum Exp(const Spectrum &s) {
			Spectrum ret;
			for (int i = 0; i < COLOR_SAMPLES; ++i)
				ret.c[i] = expf(s.c[i]);
			return ret;
		}
		Spectrum clamp(sample_type low = sample_type() ,
			sample_type high = std::numeric_limits<sample_type>::max()) const {
				Spectrum ret;
				for (int i = 0; i < COLOR_SAMPLES; ++i)
					ret.c[i] = ma::clamp(c[i], low, high);
				return ret;
		}
		bool IsNaN() const {
			for (int i = 0; i < COLOR_SAMPLES; ++i)
				if (isnan(c[i])) return true;
			return false;
		}
		void XYZ(sample_type xyz[3]) const {
			xyz[0] = xyz[1] = xyz[2] = 0.;
			for (int i = 0; i < COLOR_SAMPLES; ++i) {
				xyz[0] += XWeight[i] * c[i];
				xyz[1] += YWeight[i] * c[i];
				xyz[2] += ZWeight[i] * c[i];
			}
		}
		float y() const {
			sample_type v = 0.;
			for (int i = 0; i < COLOR_SAMPLES; ++i)
				v += YWeight[i] * c[i];
			return v;
		}
		bool operator<(const Spectrum &s2) const {
			return y() < s2.y();
		}

		// Spectrum Public Data
		static const int CIEstart = 360;
		static const int CIEend = 830;
		static const int nCIE = CIEend-CIEstart+1;
		static const float CIE_X[nCIE];
		static const float CIE_Y[nCIE];
		static const float CIE_Z[nCIE];
	private:
		// Spectrum Private Data
		sample_type c[COLOR_SAMPLES];
		static float XWeight[COLOR_SAMPLES];
		static float YWeight[COLOR_SAMPLES];
		static float ZWeight[COLOR_SAMPLES];

		template<typename ST,int CS> friend class_type FromXYZ(ST x, ST y, ST z);
	};

}
#include "ColorImpl.hpp"
namespace ma{

typedef Spectrum<> Spectrumf;
}


#endif
