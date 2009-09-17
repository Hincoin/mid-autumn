#ifndef INCLUDE_TYPE_DEFS_HPP
#define INCLUDE_TYPE_DEFS_HPP

typedef char int8;
typedef unsigned char uint8;

typedef short int16;
typedef unsigned short uint16;

typedef int int32;
typedef unsigned int uint32;



//! a more useful memset for pixel
inline void memset32 ( void * dest, const uint32 value, uint32 bytesize )
{
	uint32 * d = (uint32*) dest;

	uint32 i;

	i = bytesize >> ( 2 + 3 );
	while( i )
	{
		d[0] = value;
		d[1] = value;
		d[2] = value;
		d[3] = value;

		d[4] = value;
		d[5] = value;
		d[6] = value;
		d[7] = value;

		d += 8;
		i -= 1;
	}
	i = (bytesize >> 2 ) & 7;
	while( i )
	{
		d[0] = value;
		d += 1;
		i -= 1;
	}

}
#endif