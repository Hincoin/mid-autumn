#include <stdlib.h>
#include <stdio.h>

#include "WriteTargaImage.hpp"
#include <cassert>
namespace ma{


/*	unsigned char ReadByte(FILE *file) {  
		unsigned char b;
		size_t success = fread((void*)&b,sizeof(unsigned char),1,file);
		assert (success == 1);
		return b;
	}
*/
	size_t WriteByte(FILE *file, unsigned char b) {
		size_t success = fwrite((void*)&b,sizeof(unsigned char),1,file);
		assert (success == 1);
		return success;
	}

	unsigned char ClampColorComponent(float c) {
		int tmp = int (c*255);
		if (tmp < 0) tmp = 0;
		if (tmp > 255) tmp = 255;
		return (unsigned char)tmp;
	}
	void WriteRGBAImage(std::string& filename,float* rgb,float* alhpa
		,int x_pixel_count,int y_pixel_count,
		int x_res,int y_res,
		int x_pixel_start,int y_pixel_start)
	{
		//write simple rgb targa file for test reason
		// must end in .tga

		FILE *file = fopen(filename.c_str(),"wb");
		int width= x_pixel_count;
		int height = y_pixel_count;
		// misc header information
		for (int i = 0; i < 18; i++) {
			//unsigned char tmp;
			if (i == 2) WriteByte(file,2);
			else if (i == 12) WriteByte(file,width%256);
			else if (i == 13) WriteByte(file,width/256);
			else if (i == 14) WriteByte(file,height%256);
			else if (i == 15) WriteByte(file,height/256);
			else if (i == 16) WriteByte(file,24);
			else if (i == 17) WriteByte(file,32);
			else WriteByte(file,0);
		}
		// the data
		// flip y so that (0,0) is bottom left corner
		int offset = 0;
		//FILE* tf  = fopen("rgb.txt","wb");
		for (int y = height-1; y >= 0; y--) {
			for (int x = 0; x < width; x++) {
				float* v = &rgb[offset*3];
				//fprintf(tf,"(%.5f,%.5f,%.5f)",v[0],v[1],v[2]);
				//if (x==40 && y == 26)
				//{
				//	if (ClampColorComponent(v[0])+ClampColorComponent(v[1])+ClampColorComponent(v[2]) != 0){
				//		printf("%d,%d (%d,%d,%d)\n",
				//			x,y,ClampColorComponent(v[0]),ClampColorComponent(v[1]),ClampColorComponent(v[2]));
				//	}
				//}
				// note reversed order: b, g, r

				WriteByte(file,ClampColorComponent(v[2]));
				WriteByte(file,ClampColorComponent(v[1]));
				WriteByte(file,ClampColorComponent(v[0]));
				offset++;
			}
			//fprintf(tf,"\n");
		}
		//fflush(tf);fclose(tf);
		fflush(file);
		fclose(file);
		
	}

}
