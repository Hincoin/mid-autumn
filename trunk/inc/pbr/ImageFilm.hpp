#ifndef _MA_INCLUDED_IMAGEFILM_HPP_
#define _MA_INCLUDED_IMAGEFILM_HPP_

#include "Film.hpp"
#include <string>
#include <malloc.h>

#include "WriteTargaImage.hpp"
namespace ma{



	template<class T, int logBlockSize = 2> class BlockedArray {
		inline void *AllocAligned(size_t size) {
			const int  L1_CACHE_LINE_SIZE = 64;
#if defined(WIN32) && defined(_MSC_VER)
			return _aligned_malloc(size, L1_CACHE_LINE_SIZE);
#elif defined(WIN32)
			return __mingw_aligned_malloc(size, L1_CACHE_LINE_SIZE);
#elif defined(__APPLE__)
			return valloc(size);
#else
			return memalign(L1_CACHE_LINE_SIZE, size);
#endif
		}
		inline void FreeAligned(void *ptr) {

#if defined(WIN32) && defined(_MSC_VER)
        _aligned_free(ptr);
#elif defined(WIN32)
		__mingw_aligned_free(ptr);
#else // NOBOOK
        free(ptr);
#endif // NOBOOK
		}
	public:
		// BlockedArray Public Methods
		BlockedArray(int nu, int nv, const T *d = NULL) {
			uRes = nu;
			vRes = nv;
			uBlocks = RoundUp(uRes) >> logBlockSize;
			int nAlloc = RoundUp(uRes) * RoundUp(vRes);
			data = (T *)AllocAligned(nAlloc * sizeof(T));
			for (int i = 0; i < nAlloc; ++i)
				new (&data[i]) T();
			if (d)
				for (int v = 0; v < nv; ++v)
					for (int u = 0; u < nu; ++u)
						(*this)(u, v) = d[v * uRes + u];
		}
		int BlockSize() const { return 1 << logBlockSize; }
		int RoundUp(int x) const {
			return (x + BlockSize() - 1) & ~(BlockSize() - 1);
		}
		int uSize() const { return uRes; }
		int vSize() const { return vRes; }
		~BlockedArray() {
			for (int i = 0; i < uRes * vRes; ++i)
				data[i].~T();
			FreeAligned(data);
		}
		int Block(int a) const { return a >> logBlockSize; }
		int Offset(int a) const { return (a & (BlockSize() - 1)); }
		T &operator()(int u, int v) {
			int bu = Block(u), bv = Block(v);
			int ou = Offset(u), ov = Offset(v);
			int offset = BlockSize() * BlockSize() *
				(uBlocks * bv + bu);
			offset += BlockSize() * ov + ou;
			return data[offset];
		}
		const T &operator()(int u, int v) const {
			int bu = Block(u), bv = Block(v);
			int ou = Offset(u), ov = Offset(v);
			int offset = BlockSize() * BlockSize() * (uBlocks * bv + bu);
			offset += BlockSize() * ov + ou;
			return data[offset];
		}
		void GetLinearArray(T *a) const {
			for (int v = 0; v < vRes; ++v)
				for (int u = 0; u < uRes; ++u)
					*a++ = (*this)(u, v);
		}
	private:
		// BlockedArray Private Data
		T *data;
		int uRes, vRes, uBlocks;
	};

	template<typename Conf>
	class ImageFilm:public IFilm<Conf>//public Film<ImageFilm<Conf>,typename Conf::interface_config>
	{
		ADD_SAME_TYPEDEF(Conf,sample_t)
		ADD_SAME_TYPEDEF(Conf,spectrum_t)
		ADD_SAME_TYPEDEF(Conf,ray_t)
		ADD_SAME_TYPEDEF(Conf,scalar_t)
		ADD_SAME_TYPEDEF(Conf,filter_ptr);
		//typedef Film<ImageFilm<Conf>,typename Conf::interface_config> parent_type;
		typedef IFilm<Conf> parent_type;
	public:
		typedef ImageFilm<Conf> class_type;
		class_type*
			cloneImpl(const std::string& file)const
			{
				return new class_type(this->xResolution(),this->yResolution(),
						filter::clone(filter),cropWindow,file,premultiplyAlpha,writeFrequency);
			}
		// ImageFilm Public Methods
		ImageFilm(int xres, int yres,
			filter_ptr filt, const scalar_t crop[4],
			const std::string &file_name, bool premult,
			int wf):parent_type(xres,yres){
				filter = filt;
				memcpy(cropWindow, crop, 4 * sizeof(scalar_t));
				filename = file_name;
				premultiplyAlpha = premult;
				writeFrequency = sampleCount = wf;
				// Compute film image extent
				xPixelStart = ceil32(parent_type::x_resolution * cropWindow[0]);
				xPixelCount =
					ceil32(parent_type::x_resolution * cropWindow[1]) - xPixelStart;
				yPixelStart =
					ceil32(parent_type::y_resolution * cropWindow[2]);
				yPixelCount =
					ceil32(parent_type::y_resolution * cropWindow[3]) - yPixelStart;
				// Allocate film image storage
				pixels = new BlockedArray<Pixel>(xPixelCount, yPixelCount);
				// Precompute filter weight table

				filterTable =
					new scalar_t[FILTER_TABLE_SIZE * FILTER_TABLE_SIZE];
				scalar_t *ftp = filterTable;
				for (int y = 0; y < FILTER_TABLE_SIZE; ++y) {
					scalar_t fy = ((scalar_t)y + .5f) * filter->yWidth /
						FILTER_TABLE_SIZE;
					for (int x = 0; x < FILTER_TABLE_SIZE; ++x) {
						scalar_t fx = ((scalar_t)x + .5f) * filter->xWidth /
							FILTER_TABLE_SIZE;
						*ftp++ = filter->evaluate(fx, fy);
					}
				}
		}
		void addSampleImpl(const sample_t &sample, const ray_t &ray,
			const spectrum_t &L, scalar_t alpha){
				// Compute sample's raster extent
				scalar_t dImageX = sample.image_x - 0.5f;
				scalar_t dImageY = sample.image_y - 0.5f;
				int x0 = ceil32 (dImageX - filter->xWidth);
				int x1 = floor32(dImageX + filter->xWidth);
				int y0 = ceil32 (dImageY - filter->yWidth);
				int y1 = floor32(dImageY + filter->yWidth);

				x0 = std::min(x0,x1);
				x1 = std::max(x0,x1);
				x0 = std::max(x0, xPixelStart);
				x1 = std::min(x1, xPixelStart + xPixelCount - 1);
				x0 = std::min(x0, xPixelStart + xPixelCount - 1);
				x1 = std::max(x1, xPixelStart);

				y0 = std::min(y0,y1);
				y1 = std::max(y0,y1);
				y0 = std::max(y0, yPixelStart);
				y1 = std::min(y1, yPixelStart + yPixelCount - 1);
				y1 = std::max(y1, yPixelStart);
				y0 = std::min(y0, yPixelStart + yPixelCount - 1);

				// Loop over filter support and add sample to pixel arrays
				// Precompute $x$ and $y$ filter table offsets
				int *ifx = (int *)alloca((x1-x0+1) * sizeof(int));
				for (int x = x0; x <= x1; ++x) {
					scalar_t fx = fabsf((x - dImageX) *
						filter->invXWidth * FILTER_TABLE_SIZE);
					ifx[x-x0] = std::min(floor32(fx), FILTER_TABLE_SIZE-1);
				}
				int *ify = (int *)alloca((y1-y0+1) * sizeof(int));
				for (int y = y0; y <= y1; ++y) {
					scalar_t fy = fabsf((y - dImageY) *
						filter->invYWidth * FILTER_TABLE_SIZE);
					ify[y-y0] = std::min(floor32(fy), FILTER_TABLE_SIZE-1);
				}
				for (int y = y0; y <= y1; ++y)
					for (int x = x0; x <= x1; ++x) {
						// Evaluate filter value at $(x,y)$ pixel
						int offset = ify[y-y0]*FILTER_TABLE_SIZE + ifx[x-x0];
						scalar_t filterWt = filterTable[offset];
						// Update pixel values with filtered sample contribution
						Pixel &pixel = (*pixels)(x - xPixelStart, y - yPixelStart);
						pixel.L.AddWeighted(filterWt, L);
						pixel.alpha += alpha * filterWt;
						pixel.weightSum += filterWt;
					}

					// Possibly write out in-progress image
					if (--sampleCount == 0) {
						parent_type::writeImage();
						sampleCount = writeFrequency;
					}
		}
		void getSampleExtentImpl(int &xstart, int &xend,
			int &ystart, int &yend) const{
				xstart = floor32(xPixelStart + .5f - filter->xWidth);
				xend   = floor32(xPixelStart + .5f + xPixelCount  +
					filter->xWidth);
				ystart = floor32(yPixelStart + .5f - filter->yWidth);
				yend   = floor32(yPixelStart + .5f + yPixelCount +
					filter->yWidth);
		}
		void writeImageImpl(){
			// Convert image to RGB and compute final pixel values
			int nPix = xPixelCount * yPixelCount;
			scalar_t *rgb = new scalar_t[3*nPix], *alpha = new scalar_t[nPix];
			int offset = 0;
			for (int y = 0; y < yPixelCount; ++y) {
				for (int x = 0; x < xPixelCount; ++x) {
					// Convert pixel spectral radiance to RGB
					scalar_t xyz[3];
					(*pixels)(x, y).L.XYZ(xyz);
					const scalar_t
						rWeight[3] = { 3.240479f, -1.537150f, -0.498535f };
					const scalar_t
						gWeight[3] = {-0.969256f,  1.875991f,  0.041556f };
					const scalar_t
						bWeight[3] = { 0.055648f, -0.204043f,  1.057311f };
					rgb[3*offset  ] = rWeight[0]*xyz[0] +
						rWeight[1]*xyz[1] +
						rWeight[2]*xyz[2];
					rgb[3*offset+1] = gWeight[0]*xyz[0] +
						gWeight[1]*xyz[1] +
						gWeight[2]*xyz[2];
					rgb[3*offset+2] = bWeight[0]*xyz[0] +
						bWeight[1]*xyz[1] +
						bWeight[2]*xyz[2];
					alpha[offset] = (*pixels)(x, y).alpha;

					// Normalize pixel with weight sum
					scalar_t weightSum = (*pixels)(x, y).weightSum;
					if (weightSum != 0.f) {
						scalar_t invWt = 1.f / weightSum;
						rgb[3*offset  ] =
							ma::clamp(rgb[3*offset  ] * invWt, 0.f, std::numeric_limits<scalar_t>::max());
						rgb[3*offset+1] =
							ma::clamp(rgb[3*offset+1] * invWt, 0.f, std::numeric_limits<scalar_t>::max());
						rgb[3*offset+2] =
							ma::clamp(rgb[3*offset+2] * invWt, 0.f, std::numeric_limits<scalar_t>::max());
						alpha[offset] = ma::clamp(alpha[offset] * invWt, 0.f, 1.f);
					}
					// Compute premultiplied alpha color
					if (premultiplyAlpha) {
						rgb[3*offset  ] *= alpha[offset];
						rgb[3*offset+1] *= alpha[offset];
						rgb[3*offset+2] *= alpha[offset];
					}
#ifdef _DEBUG
					if(alpha[offset] !=0)
					{
						//Pixel &pixel = (*pixels)(x - xPixelStart, y - yPixelStart);
						printf("pixel (%d,%d): (%.6f,%.6f,%.6f,%.6f)\n",x,y,rgb[3*offset  ],rgb[3*offset+1],rgb[3*offset+2],
							alpha[offset] );
					}

#endif
					++offset;
				}
			}
			//// Write RGBA image
			WriteRGBAImage(filename, rgb, alpha,
				xPixelCount, yPixelCount,
				parent_type::x_resolution, parent_type::y_resolution,
				xPixelStart, yPixelStart);
			// Release temporary image memory
			delete[] alpha;
			delete[] rgb;

		}
		void resetCropWindowImpl(scalar_t xmin,scalar_t xmax,scalar_t ymin,scalar_t ymax)
		{
			//parent_type::writeImage();//flush previous image
			cropWindow[0] = xmin;
			cropWindow[1] = xmax;
			cropWindow[2] = ymin;
			cropWindow[3] = ymax;
			xPixelStart = ceil32(parent_type::x_resolution * cropWindow[0]);
			xPixelCount =
					ceil32(parent_type::x_resolution * cropWindow[1]) - xPixelStart;
			yPixelStart =
					ceil32(parent_type::y_resolution * cropWindow[2]);
			yPixelCount =
					ceil32(parent_type::y_resolution * cropWindow[3]) - yPixelStart;
			if(pixels) delete pixels;
			// Allocate film image storage
			pixels = new BlockedArray<Pixel>(xPixelCount, yPixelCount);
			
		}

		~ImageFilm(){
			delete pixels;
			delete filter;
			delete[] filterTable;
		}
	private:
		// ImageFilm Private Data
		filter_ptr filter;
		int writeFrequency, sampleCount;
		std::string filename;
		bool premultiplyAlpha;
		scalar_t cropWindow[4];
		int xPixelStart, yPixelStart, xPixelCount, yPixelCount;
		struct Pixel {
			Pixel() : L(0.f) {
				alpha = 0.f;
				weightSum = 0.f;
			}
			spectrum_t L;
			scalar_t alpha, weightSum;
		};
		BlockedArray<Pixel> *pixels;
		scalar_t *filterTable;
		static const int  FILTER_TABLE_SIZE = 16;
	};
}

#include "TypeMap.hpp"
namespace ma{
	MAKE_TYPE_STR_MAP(1,ImageFilm,image)
namespace details{
	template<typename Conf>
	struct film_creator<ImageFilm<Conf> >
	{
		template<typename FP>
		ImageFilm<Conf>*
		operator()(const ParamSet& param,FP filt)const
		{
			typedef ImageFilm<Conf> film_t;
			//create film
			 std::string filename =  param.as<std::string>("filename","pbrt.tga");
			 bool premultiplyAlpha =  param.as<bool>("premultiplyalpha",true);
#ifdef NDEBUG
	 int xres = 800;
	 int yres = 600;
#else
	 int xres = 80;
	 int yres = 60;
#endif

		 xres = param.as<int>("xresolution",xres);
	 yres = param.as<int>("yresolution",yres);
	 float crop[4] = { 0, 1, 0, 1 };
	 std::vector<float> cropwindow ;
	cropwindow =  param.as<std::vector<float> >("cropwindow",cropwindow);	
	 if(cropwindow.size() == 4)
	 {
		crop[0] = clamp(std::min(cropwindow[0],cropwindow[1]),0.f,1.f);
	   crop[1] = clamp(std::max(cropwindow[0],cropwindow[1]),0.f,1.f);
   crop[2] = clamp(std::min(cropwindow[2],cropwindow[3]),0.f,1.f);
crop[3] = clamp(std::max(cropwindow[2],cropwindow[3]),0.f,1.f);   
	 }

	 int write_frequency = param.as<int>("writefrequency",-1);
	 return new film_t(xres,yres,filt,crop,filename,premultiplyAlpha,write_frequency);		
		}	
	};	
}

}

#endif
