#ifndef MA_IMAGE_IMPL_HPP
#define MA_IMAGE_IMPL_HPP

#include "MAImage.hpp"

#include <cassert>

namespace ma{
	//! Returns width and height of image data.
	template<typename Configure>
	const scalar2i& MAImage<Configure>::getDimension() const{return Size;}

	//! Returns bits per pixel.
	template<typename Configure>
	unsigned int MAImage<Configure>::getBitsPerPixel() const {return BitsPerPixel;}

	//! Returns bytes per pixel
	template<typename Configure>
	unsigned int MAImage<Configure>::getBytesPerPixel() const {return BytesPerPixel;}

	//! Returns image data size in bytes
	template<typename Configure>
	unsigned int MAImage<Configure>::getImageDataSizeInBytes() const {	return Pitch * scalar2_op::height(Size);}

	//! Returns image data size in pixels
	template<typename Configure>
	unsigned int MAImage<Configure>::getImageDataSizeInPixels() const 
	{	return scalar2_op::width(Size) * scalar2_op::height(Size);}

	//! Returns a pixel
	template<typename Configure>
	typename Configure::Color MAImage<Configure>::getPixel(unsigned int x, unsigned int y) const
	{	
		if (x >= (unsigned int) scalar2_op::width(Size) ||
			y >= (unsigned int) scalar2_op::height(Size) )
			return Color(0);

		switch(Format)
		{
		case ECF_A1R5G5B5:
			return A1R5G5B5toA8R8G8B8(((unsigned short*)Data)[y * scalar2_op::width(Size)  + x]);
		case ECF_R5G6B5:
			return R5G6B5toA8R8G8B8(((unsigned short*)Data)[y* scalar2_op::width(Size) + x]);
		case ECF_A8R8G8B8:
			return ((unsigned int*)Data)[y* scalar2_op::width(Size) + x];
		case ECF_R8G8B8:
			{
				unsigned char* p = &((unsigned char*)Data)[(y*3)* scalar2_op::width(Size)  + (x*3)];
				return Color(255,p[0],p[1],p[2]);
			}
		}

		return Color(0);
	}

	//! Sets a pixel
	template<typename Configure>
	void MAImage<Configure>::setPixel(unsigned int x, unsigned int y, const Color &color ){
		if (x >= (unsigned int)scalar2_op::width(Size) || y >= (unsigned int)scalar2_op::height(Size))
			return;

		switch(Format)
		{
		case ECF_A1R5G5B5:
			//{
			//	unsigned short * dest = (unsigned short*) ((unsigned char*) Data + ( y * Pitch ) + ( x << 1 ));
			//	*dest = video::A8R8G8B8toA1R5G5B5 ( color.color );
			//} 
			break;

		case ECF_R5G6B5:
			//{
			//	unsigned short * dest = (unsigned short*) ((unsigned char*) Data + ( y * Pitch ) + ( x << 1 ));
			//	*dest = video::A8R8G8B8toR5G6B5 ( color.color );
			//}
			break;

		case ECF_R8G8B8:
			{
				unsigned char* dest = (unsigned char*) Data + ( y * Pitch ) + ( x * 3 );
				dest[0] = color.getRed();
				dest[1] = color.getGreen();
				dest[2] = color.getBlue();
			} break;

		case ECF_A8R8G8B8:
			{
				unsigned int * dest = (unsigned int*) ((unsigned char*) Data + ( y * Pitch ) + ( x << 2 ));
				*dest = color.color;
			} break;
		}
	}

	//! Returns the color format
	template<typename Configure>
	ECOLOR_FORMAT MAImage<Configure>::getColorFormat() const  {return ;}

	//! Returns mask for red value of a pixel
	template<typename Configure>
	unsigned int MAImage<Configure>::getRedMask() const {return RedMask;}

	//! Returns mask for green value of a pixel
	template<typename Configure>
	unsigned int MAImage<Configure>::getGreenMask() const {return GreenMask;}

	//! Returns mask for blue value of a pixel
	template<typename Configure>
	unsigned int MAImage<Configure>::getBlueMask() const {return BlueMask;}

	//! Returns mask for alpha value of a pixel
	template<typename Configure>
	unsigned int MAImage<Configure>::getAlphaMask() const {return AlphaMask;}

	//! Returns pitch of image
	template<typename Configure>
	unsigned int MAImage<Configure>::getPitch() const {return Pitch;}

	//! Copies the image into the target, scaling the image to fit
	template<typename Configure>
	void MAImage<Configure>::copyToScaling(void* target, int width, int height, ECOLOR_FORMAT format/*=ECF_A8R8G8B8*/, unsigned int pitch/*=0*/)
	{
	};

	//! Copies the image into the target, scaling the image to fit
	template<typename Configure>
	void MAImage<Configure>::copyToScaling(ImagePtr target) 
	{
	}

	template<typename Configure>
	void MAImage<Configure>::initData()
	{
		setBitMasks();
		BitsPerPixel = getBitsPerPixelFromFormat(Format);
		BytesPerPixel = BitsPerPixel / 8;

		// Pitch should be aligned...
		Pitch = BytesPerPixel * scalar2_op::width(Size);

		if (!Data)
			Data = new int8[scalar2_op::height(Size) * Pitch];
	}

	template<typename Configure>
	void MAImage<Configure>::setBitMasks()
	{
		switch(Format)
		{
		case ECF_A1R5G5B5:
			AlphaMask = 0x1<<15;
			RedMask = 0x1F<<10;
			GreenMask = 0x1F<<5;
			BlueMask = 0x1F;
			break;
		case ECF_R5G6B5:
			AlphaMask = 0x0;
			RedMask = 0x1F<<11;
			GreenMask = 0x3F<<5;
			BlueMask = 0x1F;
			break;
		case ECF_R8G8B8:
			AlphaMask = 0x0;
			RedMask   = 0x00FF0000;
			GreenMask = 0x0000FF00;
			BlueMask  = 0x000000FF;
			break;
		case ECF_A8R8G8B8:
			AlphaMask = 0xFF000000;
			RedMask   = 0x00FF0000;
			GreenMask = 0x0000FF00;
			BlueMask  = 0x000000FF;
			break;
		}
	}

	template<typename Configure>
	void MAImage<Configure>::fill(const Color &color)
	{
		uint32 c;

		switch ( Format )
		{
		case ECF_A1R5G5B5:
			c = color_op::A8R8G8B8toA1R5G5B5 ( color.color );
			c |= c << 16;
			break;
		case ECF_R5G6B5:
			c = color_op::A8R8G8B8toR5G6B5 ( color.color );
			c |= c << 16;
			break;
		case ECF_A8R8G8B8:
			c = color.color;
			break;
		default:
			//			os::Printer::log("CImage::Format not supported", ELL_ERROR);
			assert(false);
			return;
		}

		memset32 ( Data, c, getImageDataSizeInBytes () );
	}
}
#endif