#include "igi.h"



void igi_init(
        igi_t *igi,
        int nl,int ns,int md,float rrt,float gl,unsigned gs
        )
{
    igi->n_light_paths = nl;
    igi->max_specular_depth = md;
    igi->n_light_sets = ns;
    igi->virtual_lights = 0;
    igi->n_virtual_lights_paths = (unsigned*)calloc(sizeof(unsigned) * ns);
    igi->rr_threshold = rrt;
    igi->glimit = gl;
    igi->n_gather_samples = gs;
}
void igi_destroy(igi_t *igi)
{
    ::free(igi->virtual_lights);
    ::free(igi->n_virtual_lights_paths);
}

INLINE void igi_preprocess(
        igi_t *igi,
        cl_scene_info_t scene_info,
        Seed *seed
        )
{

}

