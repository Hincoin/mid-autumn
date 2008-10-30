#ifndef _MA_IMAGE_HPP
#define _MA_IMAGE_HPP

#include "Image.hpp"
#include "Vector.hpp"

namespace ma{
	template<typename Configure>
	class MAImage:public Image<MAImage<Configure>,Configure>{
	public:
		typedef typename Configure::Color Color;
	public:
	private:
		void* Data;
		scalar2i Size;
		unsigned int BitsPerPixel;
		unsigned int BytesPerPixel;
		unsigned int Pitch;
		ECOLOR_FORMAT Format;

		bool DeleteMemory;

		unsigned int RedMask;
		unsigned int GreenMask;
		unsigned int BlueMask;
		unsigned int AlphaMask;
	};
}
#endif