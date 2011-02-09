#ifndef _MATERIAL_H_
#define _MATERIAL_H_

#include "spectrum.h"

typedef struct  
{
	unsigned material_type;
	unsigned memory_start;
}material_info_t;

typedef struct {
    spectrum_t color;
//    unsigned n_samples;
}light_material_t;

#endif
