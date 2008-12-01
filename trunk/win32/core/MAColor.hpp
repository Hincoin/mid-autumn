#ifndef MA_COLOR_HPP
#define MA_COLOR_HPP

#include "TypeDefs.hpp"

#include "MAMath.hpp"

#ifdef max
#undef max
#endif // max

#ifdef min
#undef min
#endif // min

namespace ma{
	namespace color_op{
		//! Creates a 16 bit A1R5G5B5 color
		inline uint16 A1R5G5B5(uint32 r, uint32 g, uint32 b, uint32 a=0xFF)
		{
			return ((a & 0x80) << 8 |
				(r & 0xF8) << 7 |
				(g & 0xF8) << 2 |
				(b & 0xF8) >> 3);
		}


		//! Creates a 16 bit A1R5G5B5 color
		inline uint16 A1R5G5B5(uint32 r, uint32 g, uint32 b)
		{
			return A1R5G5B5(r,g,b,0xFF);
		}


		//! Creates a 16bit A1R5G5B5 color, based on 16bit input values
		inline uint16 A1R5G5B5from16(uint16 r, uint16 g, uint16 b)
		{
			return (0x8000 |
				(r & 0x1F) << 10 |
				(g & 0x1F) << 5  |
				(b & 0x1F));
		}


		//! Converts a 32bit (X8R8G8B8) color to a 16bit A1R5G5B5 color
		inline uint16 X8R8G8B8toA1R5G5B5(uint32 color)
		{
			return (0x8000 |
				( color & 0x00F80000) >> 9 |
				( color & 0x0000F800) >> 6 |
				( color & 0x000000F8) >> 3);
		}


		//! Converts a 32bit (A8R8G8B8) color to a 16bit A1R5G5B5 color
		inline uint16 A8R8G8B8toA1R5G5B5(uint32 color)
		{
			return (( color & 0x80000000) >> 16|
				( color & 0x00F80000) >> 9 |
				( color & 0x0000F800) >> 6 |
				( color & 0x000000F8) >> 3);
		}


		//! Converts a 32bit (A8R8G8B8) color to a 16bit R5G6B5 color
		inline uint16 A8R8G8B8toR5G6B5(uint32 color)
		{
			return (( color & 0x00F80000) >> 8 |
				( color & 0x0000FC00) >> 5 |
				( color & 0x000000F8) >> 3);
		}


		//! Convert A8R8G8B8 Color from A1R5G5B5 color
		/** build a nicer 32bit Color by extending dest lower bits with source high bits. */
		inline uint32 A1R5G5B5toA8R8G8B8(uint16 color)
		{
			return ( (( -( (int32) color & 0x00008000 ) >> (int32) 31 ) & 0xFF000000 ) |
				(( color & 0x00007C00 ) << 9) | (( color & 0x00007000 ) << 4) |
				(( color & 0x000003E0 ) << 6) | (( color & 0x00000380 ) << 1) |
				(( color & 0x0000001F ) << 3) | (( color & 0x0000001C ) >> 2)
				);
		}


		//! Returns A8R8G8B8 Color from R5G6B5 color
		inline uint32 R5G6B5toA8R8G8B8(uint16 color)
		{
			return 0xFF000000 |
				((color & 0xF800) << 8)|
				((color & 0x07E0) << 5)|
				((color & 0x001F) << 3);
		}


		//! Returns A1R5G5B5 Color from R5G6B5 color
		inline uint16 R5G6B5toA1R5G5B5(uint16 color)
		{
			return 0x8000 | (((color & 0xFFC0) >> 1) | (color & 0x1F));
		}


		//! Returns R5G6B5 Color from A1R5G5B5 color
		inline uint16 A1R5G5B5toR5G6B5(uint16 color)
		{
			return (((color & 0x7FE0) << 1) | (color & 0x1F));
		}



		//! Returns the alpha component from A1R5G5B5 color
		inline uint32 alpha(uint16 color)
		{
			return ((color >> 15)&0x1);
		}


		//! Returns the red component from A1R5G5B5 color.
		/** Shift left by 3 to get 8 bit value. */
		inline uint32 red(uint16 color)
		{
			return ((color >> 10)&0x1F);
		}


		//! Returns the green component from A1R5G5B5 color
		/** Shift left by 3 to get 8 bit value. */
		inline uint32 green(uint16 color)
		{
			return ((color >> 5)&0x1F);
		}


		//! Returns the blue component from A1R5G5B5 color
		/** Shift left by 3 to get 8 bit value. */
		inline uint32 blue(uint16 color)
		{
			return (color & 0x1F);
		}

		//! Returns the red component from A1R5G5B5 color.
		/** Shift left by 3 to get 8 bit value. */
		inline int32 redSigned(uint16 color)
		{
			return ((color >> 10)&0x1F);
		}


		//! Returns the green component from A1R5G5B5 color
		/** Shift left by 3 to get 8 bit value. */
		inline int32 greenSigned(uint16 color)
		{
			return ((color >> 5)&0x1F);
		}


		//! Returns the blue component from A1R5G5B5 color
		/** Shift left by 3 to get 8 bit value. */
		inline int32 blueSigned(uint16 color)
		{
			return (color & 0x1F);
		}

		//! Returns the average from a 16 bit A1R5G5B5 color
		inline int32 average(int16 color)
		{
			return ((red(color)<<3) + (green(color)<<3) + (blue(color)<<3)) / 3;
		}
	}
	//32 bit integer color (0-255) each component
	struct Colori{
	    typedef int32 value_type;//

		int32 color;

		//! Constructor of the Color. Does nothing.
		/** The color value is not initialized to save time. */
		Colori() {}

		//! Constructs the color from 4 values representing the alpha, red, green and blue component.
		/** Must be values between 0 and 255. */
		Colori (uint32 a, uint32 r, uint32 g, uint32 b)
			: color(((a & 0xff)<<24) | ((r & 0xff)<<16) | ((g & 0xff)<<8) | (b & 0xff)) {}

		//! Constructs the color from a 32 bit value. Could be another color.
		Colori(uint32 clr)
			: color(clr) {}

		//! Returns the alpha component of the color.
		/** The alpha component defines how transparent a color should
		be. 255 means not transparent (opaque), 0 means fully
		transparent. */
		uint32 getAlpha() const { return color>>24; }

		//! Returns the red component of the color.
		/** \return Value between 0 and 255, specifying how red the color is.
		0 means no red, 255 means full red. */
		uint32 getRed() const { return (color>>16) & 0xff; }

		//! Returns the green component of the color.
		/** \return Value between 0 and 255, specifying how green the color is.
		0 means no green, 255 means full green. */
		uint32 getGreen() const { return (color>>8) & 0xff; }

		//! Returns the blue component of the color.
		/** \return Value between 0 and 255, specifying how blue the color is.
		0 means no blue, 255 means full blue. */
		uint32 getBlue() const { return color & 0xff; }

		//! Returns the luminance of the color.
		float getLuminance() const
		{
			return 0.3f*getRed() + 0.59f*getGreen() + 0.11f*getBlue();
		}

		//! Returns the average intensity of the color.
		uint32 getAverage() const
		{
			return ( getRed() + getGreen() + getBlue() ) / 3;
		}

		//! Sets the alpha component of the Color.
		/** The alpha component defines how transparent a color should
		be.
		\param a: Has to be a value between 0 and 255.
		255 means not transparent (opaque), 0 means fully transparent. */
		void setAlpha(uint32 a) { color = ((a & 0xff)<<24) | (color & 0x00ffffff); }

		//! Sets the red component of the Color.
		/** \param r: Has to be a value between 0 and 255.
		0 means no red, 255 means full red. */
		void setRed(uint32 r) { color = ((r & 0xff)<<16) | (color & 0xff00ffff); }

		//! Sets the green component of the Color.
		/** \param g: Has to be a value between 0 and 255.
		0 means no green, 255 means full green. */
		void setGreen(uint32 g) { color = ((g & 0xff)<<8) | (color & 0xffff00ff); }

		//! Sets the blue component of the Color.
		/** \param b: Has to be a value between 0 and 255.
		0 means no blue, 255 means full blue. */
		void setBlue(uint32 b) { color = (b & 0xff) | (color & 0xffffff00); }

		//! Calculates a 16 bit A1R5G5B5 value of this color.
		/** \return 16 bit A1R5G5B5 value of this color. */
		uint16 toA1R5G5B5() const { return color_op::A8R8G8B8toA1R5G5B5(color); };

		//! Converts color to OpenGL color format
		/** From ARGB to RGBA in 4 byte components for endian aware
		passing to OpenGL
		\param dest: address where the 4x8 bit OpenGL color is stored. */
		void toOpenGLColor(uint8* dest) const
		{
			*dest =   getRed();
			*++dest = getGreen();
			*++dest = getBlue();
			*++dest = getAlpha();
		};

		//! Sets all four components of the color at once.
		/** Constructs the color from 4 values representing the alpha,
		red, green and blue components of the color. Must be values
		between 0 and 255.
		\param a: Alpha component of the color. The alpha component
		defines how transparent a color should be. Has to be a value
		between 0 and 255. 255 means not transparent (opaque), 0 means
		fully transparent.
		\param r: Sets the red component of the Color. Has to be a
		value between 0 and 255. 0 means no red, 255 means full red.
		\param g: Sets the green component of the Color. Has to be a
		value between 0 and 255. 0 means no green, 255 means full
		green.
		\param b: Sets the blue component of the Color. Has to be a
		value between 0 and 255. 0 means no blue, 255 means full blue. */
		void set(uint32 a, uint32 r, uint32 g, uint32 b)
		{
			color = (((a & 0xff)<<24) | ((r & 0xff)<<16) | ((g & 0xff)<<8) | (b & 0xff));
		}
		void set(uint32 col) { color = col; }

		//! Compares the color to another color.
		/** \return True if the colors are the same, and false if not. */
		bool operator==(const Colori& other) const { return other.color == color; }

		//! Compares the color to another color.
		/** \return True if the colors are different, and false if they are the same. */
		bool operator!=(const Colori& other) const { return other.color != color; }

		//! comparison operator
		/** \return True if this color is smaller than the other one */
		bool operator<(const Colori& other) const { return (color < other.color); }

		//! Adds two colors, result is clamped to 0..255 values
		/** \param other Color to add to this color
		\return Addition of the two colors, clamped to 0..255 values */
		Colori operator+(const Colori& other) const
		{

			return Colori(std::min(getAlpha() + other.getAlpha(), 255u),
				std::min(getRed() + other.getRed(), 255u),
				std::min(getGreen() + other.getGreen(), 255u),
				std::min(getBlue() + other.getBlue(), 255u));
		}

		//! Interpolates the color with a float value to another color
		/** \param other: Other color
		\param d: value between 0.0f and 1.0f
		\return Interpolated color. */
		Colori getInterpolated(const Colori &other, float d) const
		{
			d = clamp(d, 0.f, 1.f);
			const float inv = 1.0f - d;
			return Colori((uint32)(other.getAlpha()*inv + getAlpha()*d),
				(uint32)(other.getRed()*inv + getRed()*d),
				(uint32)(other.getGreen()*inv + getGreen()*d),
				(uint32)(other.getBlue()*inv + getBlue()*d));
		}

		//! Returns interpolated color. ( quadratic )
		/** \param c1: first color to interpolate with
		\param c2: second color to interpolate with
		\param d: value between 0.0f and 1.0f. */
		Colori getInterpolated_quadratic(const Colori& c1, const Colori& c2, float d) const
		{
			// this*(1-d)*(1-d) + 2 * c1 * (1-d) + c2 * d * d;
			d = clamp(d, 0.f, 1.f);
			const float inv = 1.f - d;
			const float mul0 = inv * inv;
			const float mul1 = 2.f * d * inv;
			const float mul2 = d * d;

			return Colori(
				clamp( floor32(
				getAlpha() * mul0 + c1.getAlpha() * mul1 + c2.getAlpha() * mul2 ), 0, 255 ),
				clamp( floor32(
				getRed()   * mul0 + c1.getRed()   * mul1 + c2.getRed()   * mul2 ), 0, 255 ),
				clamp ( floor32(
				getGreen() * mul0 + c1.getGreen() * mul1 + c2.getGreen() * mul2 ), 0, 255 ),
				clamp ( floor32(
				getBlue()  * mul0 + c1.getBlue()  * mul1 + c2.getBlue()  * mul2 ), 0, 255 ));
		}
	};

	//32 bit float color (0-1) each component
	struct Colorf{
	    typedef float value_type;
	    };

}
#endif
