#ifndef INCLUDE_IMAGE_HPP
#define INCLUDE_IMAGE_HPP

#include "AddPointer.hpp"
namespace ma{

	//! An enum for the color format of textures used by the Irrlicht Engine.
	/** A color format specifies how color information is stored. */
	enum ECOLOR_FORMAT
	{
		//! 16 bit color format used by the software driver.
		/** It is thus preferred by all other irrlicht engine video drivers.
		There are 5 bits for every color component, and a single bit is left
		for alpha information. */
		ECF_A1R5G5B5 = 0,

		//! Standard 16 bit color format.
		ECF_R5G6B5,

		//! 24 bit color, no alpha channel, but 8 bit for red, green and blue.
		ECF_R8G8B8,

		//! Default 32 bit color format. 8 bits are used for every component: red, green, blue and alpha.
		ECF_A8R8G8B8
	};


	//! Interface for software image data.
	/** Image loaders create these images from files. IVideoDrivers convert
	these images into their (hardware) textures.
	*/
	template<typename Derived,typename Config>
	class Image
	{
		Derived& derived(){return static_cast<Derived&>(*this);}
		const Derived& derived()const{return static_cast<const Derived&>(*this);}
	protected:
		//! Destructor
		 ~Image() {}
	public:
		typedef typename Config::Color Color;
		typedef typename add_ptr<Image<Derived , Config> >::type ImagePtr; 

		//! Lock function. Use this to get a pointer to the image data.
		/** After you don't need the pointer anymore, you must call unlock().
		\return Pointer to the image data. What type of data is pointed to
		depends on the color format of the image. For example if the color
		format is ECF_A8R8G8B8, it is of unsigned int. Be sure to call unlock() after
		you don't need the pointer any more. */
		void* lock(){return derived().lock();}

		//! Unlock function.
		/** Should be called after the pointer received by lock() is not
		needed anymore. */
		void unlock(){return derived().unlock();}

		//! Returns width and height of image data.
		const scalar2i& getDimension() const{return derived().getDimension();}

		//! Returns bits per pixel.
		 unsigned int getBitsPerPixel() const {return derived().getBitsPerPixel();}

		//! Returns bytes per pixel
		 unsigned int getBytesPerPixel() const {return derived().getBytesPerPixel();}

		//! Returns image data size in bytes
		 unsigned int getImageDataSizeInBytes() const {return derived().getImageDataSizeInBytes();}

		//! Returns image data size in pixels
		 unsigned int getImageDataSizeInPixels() const {return derived().getImageDataSizeInPixels();}

		//! Returns a pixel
		 Color getPixel(unsigned int x, unsigned int y) const {return derived().getPixel(x,y);}

		//! Sets a pixel
		 void setPixel(unsigned int x, unsigned int y, const Color &color ){return derived().setPixel(x,y,color);}

		//! Returns the color format
		 ECOLOR_FORMAT getColorFormat() const  {return derived().getColorFormat();}

		//! Returns mask for red value of a pixel
		 unsigned int getRedMask() const {return derived().getRedMask();}

		//! Returns mask for green value of a pixel
		 unsigned int getGreenMask() const {return derived().getGreenMask();}

		//! Returns mask for blue value of a pixel
		 unsigned int getBlueMask() const {return derived().getBlueMask();}

		//! Returns mask for alpha value of a pixel
		 unsigned int getAlphaMask() const {return derived().getAlphaMask();}

		//! Returns pitch of image
		 unsigned int getPitch() const {return derived().getPitch();}

		//! Copies the image into the target, scaling the image to fit
		 void copyToScaling(void* target, int width, int height, ECOLOR_FORMAT format/*=ECF_A8R8G8B8*/, unsigned int pitch/*=0*/)
		 {
			return derived().copyToScaling(target,width,height,format,pitch);
		 };

		//! Copies the image into the target, scaling the image to fit
		 void copyToScaling(ImagePtr target) 
		 {
			return derived().copyToScaling(target);
		 }
		 //! fills the surface with black or white
		 void fill(const Color &color){derived().fill(color);}


	};
}
#endif