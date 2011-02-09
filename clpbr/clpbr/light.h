#ifndef _LIGHT_H_
#define _LIGHT_H_

#include "geometry.h"
#include "spectrum.h"

typedef struct {
	unsigned light_type;
	unsigned memory_start;
}light_info_t;


typedef struct {
    unsigned primitive_idx;//the primitive which contain emissive material
}
area_light_t
;


#endif
