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
		////! constructor from another image with format change
		//MAImage(ECOLOR_FORMAT format, IImage* imageToCopy);

		////! constructor from raw image data
		////! \param useForeignMemory: If true, the image will use the data pointer
		////! directly and own it from now on, which means it will also try to delete [] the
		////! data when the image will be destructed. If false, the memory will by copied.
		MAImage(ECOLOR_FORMAT format, const scalar2i& size,
			void* data, bool ownForeignMemory, bool deleteMemory )
			:Data(0), Size(size),Format(format),DeleteMemory(deleteMemory)
		{
			if (ownForeignMemory)
			{
				Data = (void*)0xbadf00d;//bad food
				initData();
				Data = data;
			}
			else
			{
				Data = 0;
				initData();
				memcpy(Data,data,scalar2_op::height(size) * Pitch);
			}
		}

		////! constructor for empty image
		MAImage(ECOLOR_FORMAT format, const scalar2i& size):Data(0),Size(size),Format(format),DeleteMemory(true)
		{initData();}

		////! constructor using a part from another image
		//MAImage(IImage* imageToCopy,
		//	const core::position2d<s32>& pos, const core::dimension2d<s32>& size);

		//! Lock function. Use this to get a pointer to the image data.
		/** After you don't need the pointer anymore, you must call unlock().
		\return Pointer to the image data. What type of data is pointed to
		depends on the color format of the image. For example if the color
		format is ECF_A8R8G8B8, it is of unsigned int. Be sure to call unlock() after
		you don't need the pointer any more. */
		void* lock(){return Data;}

		//! Unlock function.
		/** Should be called after the pointer received by lock() is not
		needed anymore. */
		void unlock(){}

		//! Returns width and height of image data.
		const scalar2i& getDimension() const;

		//! Returns bits per pixel.
		unsigned int getBitsPerPixel() const ;

		//! Returns bytes per pixel
		unsigned int getBytesPerPixel() const ;

		//! Returns image data size in bytes
		unsigned int getImageDataSizeInBytes() const ;

		//! Returns image data size in pixels
		unsigned int getImageDataSizeInPixels() const;

		//! Returns a pixel
		Color getPixel(unsigned int x, unsigned int y) const;

		//! Sets a pixel
		void setPixel(unsigned int x, unsigned int y, const Color &color );

		//! Returns the color format
		ECOLOR_FORMAT getColorFormat() const ;

		//! Returns mask for red value of a pixel
		unsigned int getRedMask() const;

		//! Returns mask for green value of a pixel
		unsigned int getGreenMask() const;

		//! Returns mask for blue value of a pixel
		unsigned int getBlueMask() const;

		//! Returns mask for alpha value of a pixel
		unsigned int getAlphaMask() const;

		//! Returns pitch of image
		unsigned int getPitch() const;

		//! Copies the image into the target, scaling the image to fit
		void copyToScaling(void* target, int width, int height, ECOLOR_FORMAT format/*=ECF_A8R8G8B8*/, unsigned int pitch/*=0*/);

		//! Copies the image into the target, scaling the image to fit
		void copyToScaling(ImagePtr target) ;

		~MAImage(){if(DeleteMemory) delete[](char*)Data;}

		static unsigned int getBitsPerPixelFromFormat(ECOLOR_FORMAT format)
		{
			switch(format)
			{
			case ECF_A1R5G5B5:
				return 16;
			case ECF_R5G6B5:
				return 16;
			case ECF_R8G8B8:
				return 24;
			case ECF_A8R8G8B8:
				return 32;
			}

			return 0;
		}
	private:
		void initData();
		void setBitMasks();
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

#include "MAImageImpl.hpp"
#endif
