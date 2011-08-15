#ifndef _IGI_H_
#define _IGI_H_


typedef struct {
    point3f_t p;
    normal3f_t n;
    spectrum_t le;
}
virtual_light_t;

INLINE void virtual_light_init(virtual_light_t *l,
        const point3f_t* p,
        const normal3f_t *nn,
        const spectrum_t *le)
{
    vassign(l->p,*p);
    vassign(l->nn, *nn);
    vassign(l->le, *le);
}

//instant global illumination
typedef struct {
    unsigned n_light_paths;//cpu side
    unsigned n_light_sets;
    virtual_light_t *virtual_lights;//each size is n_virtual_lights_paths[i]
    unsigned *n_virtual_lights_paths;//size is n_light_sets
    int max_specular_depth;
    float rr_threshold,glimit;
    unsigned n_gather_samples;
}
igi_t;


void igi_init(
        igi_t *igi,
        int nl,int ns,float md,float rrt,float gl,unsigned gs
        );
void igi_destroy(igi_t *igi);

INLINE void igi_preprocess(
        igi_t *igi,
        cl_scene_info_t scene_info,
        Seed *seed
        );
INLINE void igi_li(igi_t *igi,
        cl_scene_info_t scene_info,
        Seed *seed,
        spectrum_t *l)
{
}


#endif
