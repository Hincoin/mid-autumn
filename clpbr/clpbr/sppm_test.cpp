
#include <math.h>
#include "perspective_camera.h"

#include "clscene.h"
#include "image_film.h"

#include "sppm_test.h"
#include "progressive_photon_map_renderer.h"
#include "random_sampler.h"


Scene* scene = NULL;
Camera* camera = NULL;
Film* film = NULL;
Sampler* sampler = NULL;


unsigned w = 256;
unsigned h = 256;

static primitive_info_t* primitives;
static unsigned int primitive_count;
static light_info_t* lights;
static unsigned int light_count;

cl_scene_info_t cl_scene_info;

static void setup_scene();

void sppm_test()
{
	setup_scene();
	photon_map_t* photon_map = new photon_map_t();
	photon_map->final_gather = true;
	photon_map->cos_gather_angle = 0.95f;
	photon_map->gather_samples = 32;
	
	photon_map->max_dist_squared = 50;
	photon_map->max_specular_depth = 5;
	photon_map->n_caustic_paths = 0;
	photon_map->n_caustic_photons = 10000;
	photon_map->n_indirect_paths = 0;
	photon_map->n_indirect_photons = 10000;
	photon_map->n_lookup = 30;
	photon_map->total_photons = 0;
	photon_map->rr_threshold = 0.125f;
	photon_map->progressive_iteration = 0;

	photon_map->alpha = 0.618;

	PPMRenderer* renderer = new PPMRenderer(camera,film,sampler,photon_map);
	renderer->Render(cl_scene_info);
	delete photon_map;
}

void setup_scene()
{
#define WALL_RAD 1e4f
	const int cornell_sphere_count = 9;
	static  sphere_t CornellSpheres[cornell_sphere_count];
	for (unsigned i = 0;i < sizeof(CornellSpheres)/sizeof(CornellSpheres[0]); ++i)
	{
		float zmin,zmax,rad ;
		zmin = -WALL_RAD;
		zmax = WALL_RAD;
		rad = WALL_RAD;
		float pm = 360.f;
		transform_identity(CornellSpheres[i].o2w);
		sphere_init(CornellSpheres+i,&CornellSpheres[i].o2w,rad,zmin,zmax,pm,1);
	}

	transform_translate(CornellSpheres[0].o2w,WALL_RAD + 1.f, 40.8f, 81.6f);//left
	transform_translate(CornellSpheres[1].o2w,-WALL_RAD + 99.f, 40.8f, 81.6f);//right
	transform_translate(CornellSpheres[2].o2w,50.f, 40.8f, WALL_RAD);//back
	transform_translate(CornellSpheres[3].o2w,50.f, 40.8f, -WALL_RAD + 570.f);//front
	transform_translate(CornellSpheres[4].o2w,50.f, WALL_RAD, 81.6f);//bottom
	transform_translate(CornellSpheres[5].o2w,50.f, -WALL_RAD + 81.6f, 81.6f);//top
	transform_translate(CornellSpheres[6].o2w,27.f, 16.5f, 47.f);//mirror
	transform_translate(CornellSpheres[7].o2w,73.f, 16.5f, 88.f);//glass
	transform_translate(CornellSpheres[8].o2w,50.f, 81.6f - 21.f, 81.6f);//light
	float ball_rad0,ball_rad1,light_ball_rad;
	ball_rad0 = 16.5f;
	ball_rad1 = 16.5f;
	light_ball_rad = .5f;
	sphere_init(CornellSpheres + 6,&CornellSpheres[6].o2w,
		ball_rad0,-ball_rad0,ball_rad0,360.f,0);
	sphere_init(CornellSpheres + 7,&CornellSpheres[7].o2w,
		ball_rad1,-ball_rad1,ball_rad1,360.f,0);
	sphere_init(CornellSpheres + 8,&CornellSpheres[8].o2w,
		light_ball_rad,-light_ball_rad,light_ball_rad,360.f,0);

	for (unsigned i = 0;i < sizeof(CornellSpheres)/sizeof(CornellSpheres[0]); ++i)
	{
		point3f_t tmp;
		vinit(tmp,0,0,0);
		transform_point(CornellSpheres[i].p,CornellSpheres[i].o2w,tmp);
	}

	primitive_count = cornell_sphere_count;
	static primitive_info_t primitive_array[cornell_sphere_count];
	for (size_t i = 0;i < primitive_count; ++i)
	{
		primitive_array[i].shape_info.memory_start =  i * sizeof(sphere_t)/sizeof(float);
		primitive_array[i].shape_info.shape_type = 0;
		primitive_array[i].material_info.material_type = -1;
	}
	primitives = primitive_array;


	cl_scene_info.shape_data = (float*)(CornellSpheres);
	unsigned shape_buffer_size = sizeof(CornellSpheres)/sizeof(float);

	static area_light_t l;
	l.primitive_idx = 8;
	cl_scene_info.light_data = (float*)&l;
	unsigned light_buffer_size = sizeof(l)/sizeof(float);
	static float texture_mem[1024];
	cl_scene_info.texture_data = texture_mem;

	unsigned texture_buffer_size = sizeof(texture_mem)/sizeof(float);
	texture_mem[0]= 1.f;
	texture_mem[1]= 0.1f;
	texture_mem[2] =0.1f;
	texture_mem[3] = 0.f;

	texture_mem[4] = 0.1f;
	texture_mem[5]= 1.f;
	texture_mem[6]= 0.1f;
	texture_mem[7] =0.f;

	texture_mem[8] = 0.1f;
	texture_mem[9]= 0.1f;
	texture_mem[10]= 1.f;
	texture_mem[11] =0.f;

	texture_mem[12] = 1.f;
	texture_mem[13]= 1.f;
	texture_mem[14]= 1.f;
	texture_mem[15] =0.f;

	texture_mem[16] = 1.f;
	texture_mem[17]= 1.f;
	texture_mem[18]= 1.f;
	texture_mem[19] =0.f;

	texture_mem[20] = 1.f;
	texture_mem[21]= 1.f;
	texture_mem[22]= 1.f;
	texture_mem[23] =0.f;

	//for glass
	texture_mem[24] = 1.0f;
	texture_mem[25] = 1.0f;
	texture_mem[26] = 1.0f;
	texture_mem[27] = 1.f;
	texture_mem[28] = 1.f;
	texture_mem[29] = 1.f;
	texture_mem[30] = 1.7f;
	//for mirror
	texture_mem[31] = 1.f;
	texture_mem[32] = 1.f;
	texture_mem[33] = 1.f;


	static light_material_t lm;
	vinit(lm.color,1.f,1.f,1.f);
	vsmul(lm.color,2500,lm.color);
	unsigned material_buffer_size = 1024;
	float *material_data = new float[material_buffer_size];
	material_data[0] = lm.color.x;
	material_data[1] = lm.color.y;
	material_data[2] = lm.color.z;
	matte_t m0;
	m0.kd.texture_type=COLOR_CONSTANT;
	m0.kd.memory_start = 0;
	m0.sig.texture_type = FLOAT_CONSTANT;
	m0.sig.memory_start = 3;
	matte_t m1;
	m1.kd.texture_type=COLOR_CONSTANT;
	m1.kd.memory_start = 4;
	m1.sig.texture_type = FLOAT_CONSTANT;
	m1.sig.memory_start = 7;

	matte_t m2,m3,m4,m5;
	m2.kd.texture_type = COLOR_CONSTANT;
	m2.kd.memory_start = 8;
	m2.sig.texture_type = FLOAT_CONSTANT;
	m2.sig.memory_start = 11;

	m3.kd.texture_type = COLOR_CONSTANT;
	m3.kd.memory_start = 12;
	m3.sig.texture_type = FLOAT_CONSTANT;
	m3.sig.memory_start = 15;


	m4.kd.texture_type = COLOR_CONSTANT;
	m4.kd.memory_start = 16;
	m4.sig.texture_type = FLOAT_CONSTANT;
	m4.sig.memory_start = 19;

	m5.kd.texture_type = COLOR_CONSTANT;
	m5.kd.memory_start = 20;
	m5.sig.texture_type = FLOAT_CONSTANT;
	m5.sig.memory_start = 23;


	glass_t gls;
	gls.kr.texture_type = COLOR_CONSTANT;
	gls.kr.memory_start= 24;
	gls.kt.texture_type = COLOR_CONSTANT;
	gls.kt.memory_start = 27;
	gls.index.texture_type = FLOAT_CONSTANT ;
	gls.index.memory_start = 30;
	mirror_t mirror;
	mirror.kr.texture_type = COLOR_CONSTANT;
	mirror.kr.memory_start = 31;

	material_data[3] = as_float(m0.kd.texture_type);
	material_data[4] = as_float(m0.kd.memory_start);
	material_data[5] = as_float(m0.sig.texture_type);
	material_data[6] = as_float(m0.sig.memory_start);

	material_data[7] = as_float(m1.kd.texture_type);
	material_data[8] = as_float(m1.kd.memory_start);
	material_data[9] = as_float(m1.sig.texture_type);
	material_data[10] = as_float(m1.sig.memory_start);


	material_data[11] = as_float(m2.kd.texture_type);
	material_data[12] = as_float(m2.kd.memory_start);
	material_data[13] = as_float(m2.sig.texture_type);
	material_data[14] = as_float(m2.sig.memory_start);


	material_data[15] = as_float(m3.kd.texture_type);
	material_data[16] = as_float(m3.kd.memory_start);
	material_data[17] = as_float(m3.sig.texture_type);
	material_data[18] = as_float(m3.sig.memory_start);


	material_data[19] = as_float(m4.kd.texture_type);
	material_data[20] = as_float(m4.kd.memory_start);
	material_data[21] = as_float(m4.sig.texture_type);
	material_data[22] = as_float(m4.sig.memory_start);


	material_data[23] = as_float(m5.kd.texture_type);
	material_data[24] = as_float(m5.kd.memory_start);
	material_data[25] = as_float(m5.sig.texture_type);
	material_data[26] = as_float(m5.sig.memory_start);

	//gls
	material_data[27] = as_float(gls.kr.texture_type);
	material_data[28] = as_float(gls.kr.memory_start);
	material_data[29] = as_float(gls.kt.texture_type);
	material_data[30] = as_float(gls.kt.memory_start);
	material_data[31] = as_float(gls.index.texture_type);
	material_data[32] = as_float(gls.index.memory_start);

	//mirror
	material_data[33] = as_float(mirror.kr.texture_type);
	material_data[34] = as_float(mirror.kr.memory_start);
//////////////////////////////////////////////////////////////////////////
	cl_scene_info.material_data = material_data;
	//////////////////////////////////////////////////////////////////////////

	primitives[0].material_info.material_type = MATTE_MATERIAL;
	primitives[0].material_info.memory_start = 3;
	primitives[1].material_info.material_type = MATTE_MATERIAL;
	primitives[1].material_info.memory_start = 7;

	primitives[2].material_info.material_type = MATTE_MATERIAL;
	primitives[2].material_info.memory_start = 11;

	primitives[3].material_info.material_type = MATTE_MATERIAL;
	primitives[3].material_info.memory_start = 15;

	primitives[4].material_info.material_type = MATTE_MATERIAL;
	primitives[4].material_info.memory_start = 19;

	primitives[5].material_info.material_type = MATTE_MATERIAL;
	primitives[5].material_info.memory_start = 23;

	primitives[6].material_info.material_type = MIRROR_MATERIAL;
	primitives[6].material_info.memory_start = 33;

	//primitives[6].material_info.material_type = MATTE_MATERIAL;
	//primitives[6].material_info.memory_start = 3;

	primitives[7].material_info.material_type = GLASS_MATERIAL;
	primitives[7].material_info.memory_start = 27;

	//primitives[7].material_info.material_type = MATTE_MATERIAL;
	//primitives[7].material_info.memory_start = 23;

	primitives[8].material_info.material_type = LIGHT_MATERIAL;
	primitives[8].material_info.memory_start= 0;

	cl_scene_info.primitives = primitives;
	static float dummy[1];
	cl_scene_info.integrator_data = dummy;
	unsigned integrator_buffer_size = sizeof(dummy)/sizeof(float);
	cl_scene_info.accelerator_data = dummy;
	unsigned accelerator_buffer_size = sizeof(dummy)/sizeof(float);

	static light_info_t dummy_light[1];
	dummy_light[0].light_type = 0;
	dummy_light[0].memory_start = 0;
	lights = dummy_light;
	light_count = sizeof(dummy_light)/sizeof(dummy_light[0]);
	
	cl_scene_info.lght_count = light_count;
	cl_scene_info.lghts = lights;
	cl_scene_info.primitive_count = primitive_count;


	point3f_t eye_pos,eye_center;
	
	vinit(eye_pos, 50.f, 48.f, 365);
	vinit(eye_center, 50.f, 48.f - 0.042612f, 365 - 1.f);

	transform_t camera_to_world,world_to_camera;
	lookat( eye_pos.x,eye_pos.y,eye_pos.z, eye_center.x, eye_center.y, eye_center.z , 0,1,0,&world_to_camera);
	transform_inverse(camera_to_world,world_to_camera);
	film = new ImageFilm(w,h);
	camera = new PerspectiveCamera(camera_to_world,screen_window_t(1.f),degree_to_radian(degree_t(30)),film);
	sampler = new RandomSampler(0,w,0,h,1,1);
	scene = new CLScene(cl_scene_info);
}