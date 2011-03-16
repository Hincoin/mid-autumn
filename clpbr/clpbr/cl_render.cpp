#include <CL/cl.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <omp.h>

#include "camera.h"
#include "shape.h"
#include "primitive.h"
#include "light.h"

#include "display_func.h"

#include "spectrum.h"
#include "display_func.h"

#include "path_tracing.h"
#include "photon_map.h"

#ifndef M_PI
#define M_PI       3.14159265358979323846
#endif


/* Options */
static int useGPU = 0;
static int forceWorkSize = 0;

/* OpenCL variables */
static cl_context context;

static cl_mem colorBuffer;
static cl_mem pixelBuffer;
static cl_mem seedBuffer;
static cl_mem cameraBuffer;

static cl_mem sphereBuffer;

static cl_mem shape_data_buffer;
static cl_mem light_data_buffer;
static cl_mem material_data_buffer;
static cl_mem texture_data_buffer;
static cl_mem integrator_data_buffer;
static cl_mem accelerator_data_buffer;
static cl_mem primitives_buffer;
static cl_mem lights_buffer;

static float* shape_data;
static unsigned shape_buffer_size = 0;
static float* light_data;
static unsigned light_buffer_size = 0;
static float* material_data;
static unsigned material_buffer_size = 0;
static float* texture_data;
static unsigned texture_buffer_size = 0;
static float* integrator_data;
static unsigned integrator_buffer_size = 0;
static float* accelerator_data;
static unsigned accelerator_buffer_size = 0;

static primitive_info_t* primitives;
static unsigned int primitive_count;
static light_info_t* lights;
static unsigned int light_count;

static cl_command_queue commandQueue;
static cl_program program;

static cl_kernel kernel;


static unsigned int workGroupSize = 1;
static char *kernelFileName = "rendering_kernel.cl";

static spectrum_t *colors;
static unsigned int *seeds;
camera_t camera;
static int currentSample = 0;

sphere_t *spheres;
unsigned int sphereCount;

#ifdef _DEBUG
int debug_use_cpu = 1;
#else
int debug_use_cpu = 1;
#endif
static void SetUpScene()
{
#define WALL_RAD 1e4f
	//static sphere_t CornellSpheres[] = { /* Scene: radius, position, emission, color, material */
	//	{ WALL_RAD, {WALL_RAD + 1.f, 40.8f, 81.6f}, {0.f, 0.f, 0.f}, {.75f, .25f, .25f}, DIFF }, /* Left */
	//	{ WALL_RAD, {-WALL_RAD + 99.f, 40.8f, 81.6f}, {0.f, 0.f, 0.f}, {.25f, .25f, .75f}, DIFF }, /* Rght */
	//	{ WALL_RAD, {50.f, 40.8f, WALL_RAD}, {0.f, 0.f, 0.f}, {.75f, .75f, .75f}, DIFF }, /* Back */
	//	{ WALL_RAD, {50.f, 40.8f, -WALL_RAD + 270.f}, {0.f, 0.f, 0.f}, {0.f, 0.f, 0.f}, DIFF }, /* Frnt */
	//	{ WALL_RAD, {50.f, WALL_RAD, 81.6f}, {0.f, 0.f, 0.f}, {.75f, .75f, .75f}, DIFF }, /* Botm */
	//	{ WALL_RAD, {50.f, -WALL_RAD + 81.6f, 81.6f}, {0.f, 0.f, 0.f}, {.75f, .75f, .75f}, DIFF }, /* Top */
	//	{ 16.5f, {27.f, 16.5f, 47.f}, {0.f, 0.f, 0.f}, {.9f, .9f, .9f}, SPEC }, /* Mirr */
	//	{ 16.5f, {73.f, 16.5f, 78.f}, {0.f, 0.f, 0.f}, {.9f, .9f, .9f}, REFR }, /* Glas */
	//	{ 7.f, {50.f, 81.6f - 15.f, 81.6f}, {12.f, 12.f, 12.f}, {0.f, 0.f, 0.f}, DIFF } /* Lite */
	//};

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
	transform_translate(CornellSpheres[0].o2w,WALL_RAD + 1.f, 40.8f, 81.6f);
	transform_translate(CornellSpheres[1].o2w,-WALL_RAD + 99.f, 40.8f, 81.6f);
	transform_translate(CornellSpheres[2].o2w,50.f, 40.8f, WALL_RAD);
	transform_translate(CornellSpheres[3].o2w,50.f, 40.8f, -WALL_RAD + 270.f);
	transform_translate(CornellSpheres[4].o2w,50.f, WALL_RAD, 81.6f);
	transform_translate(CornellSpheres[5].o2w,50.f, -WALL_RAD + 81.6f, 81.6f);
	transform_translate(CornellSpheres[6].o2w,27.f, 16.5f, 47.f);//mirror
	transform_translate(CornellSpheres[7].o2w,73.f, 16.5f, 78.f);//glass
	transform_translate(CornellSpheres[8].o2w,50.f, 81.6f - 15.f, 81.6f);//light
	float ball_rad0,ball_rad1,light_ball_rad;
	ball_rad0 = 16.5f;
	ball_rad1 = 16.5f;
	light_ball_rad = 7.f;
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

	point3f_t unit_sphere_point = {-0.412557,-0.500631,-0.761029};
			point3f_t sp;
			transform_point(sp,CornellSpheres[8].o2w,unit_sphere_point);

	primitive_count = cornell_sphere_count;
	static primitive_info_t primitive_array[cornell_sphere_count];
	for (size_t i = 0;i < primitive_count; ++i)
	{
		primitive_array[i].shape_info.memory_start =  i * sizeof(sphere_t)/sizeof(float);
		primitive_array[i].shape_info.shape_type = 0;
		primitive_array[i].material_info.material_type = -1;
	}


	primitives = primitive_array;

	shape_data = (float*)(CornellSpheres);
	shape_buffer_size = sizeof(CornellSpheres)/sizeof(float);
	
	static area_light_t l;
	l.primitive_idx = 8;
	light_data = (float*)&l;
	light_buffer_size = sizeof(l)/sizeof(float);
	static float texture_mem[1024];
	texture_data=texture_mem;
	texture_buffer_size = sizeof(texture_mem)/sizeof(float);
	texture_data[0]= 1.f;
	texture_data[1]= 0.1f;
	texture_data[2] =0.1f;
	texture_data[3] = 0.f;
	
	texture_data[4] = 0.1f;
	texture_data[5]= 1.f;
	texture_data[6]= 0.1f;
	texture_data[7] =0.f;
	
	texture_data[8] = 0.1f;
	texture_data[9]= 0.1f;
	texture_data[10]= 1.6f;
	texture_data[11] =0.f;

	texture_data[12] = 0.f;
	texture_data[13]= 0.f;
	texture_data[14]= 0.f;
	texture_data[15] =0.f;

	texture_data[16] = 1.f;
	texture_data[17]= 1.f;
	texture_data[18]= 1.f;
	texture_data[19] =0.f;

	texture_data[20] = 1.f;
	texture_data[21]= 1.f;
	texture_data[22]= 1.f;
	texture_data[23] =0.f;

	//for glass
	texture_data[24] = 0.1f;
	texture_data[25] = 0.1f;
	texture_data[26] = 0.1f;
	texture_data[27] = 1.f;
	texture_data[28] = 1.f;
	texture_data[29] = 1.f;
	texture_data[30] = 1.33f;
	//for mirror
	texture_data[31] = 1.f;
	texture_data[32] = 1.f;
	texture_data[33] = 1.f;


	static light_material_t lm;
	vinit(lm.color,10.f,10.f,10.f);
	material_buffer_size = 1024;
	material_data = new float[material_buffer_size];
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

	static float dummy[1];
	integrator_data = dummy;
	integrator_buffer_size = sizeof(dummy)/sizeof(float);
	accelerator_data = dummy;
	accelerator_buffer_size = sizeof(dummy)/sizeof(float);

	static light_info_t dummy_light[1];
	dummy_light[0].light_type = 0;
	dummy_light[0].memory_start = 0;
	lights = dummy_light;
	light_count = sizeof(dummy_light)/sizeof(dummy_light[0]);

	spheres = CornellSpheres;
	sphereCount = sizeof(CornellSpheres) / sizeof(sphere_t);

	vinit(camera.eye, 50.f, 45.f, 205.6f);
	vinit(camera.center, 50.f, 45 - 0.042612f, 204.6);

}
static void FreeBuffers() {
	cl_int status = clReleaseMemObject(colorBuffer);
	if (status != CL_SUCCESS) {
		fprintf(stderr, "Failed to release OpenCL color buffer: %d\n", status);
		exit(-1);
	}

	status = clReleaseMemObject(pixelBuffer);
	if (status != CL_SUCCESS) {
		fprintf(stderr, "Failed to release OpenCL pixel buffer: %d\n", status);
		exit(-1);
	}

	status = clReleaseMemObject(seedBuffer);
	if (status != CL_SUCCESS) {
		fprintf(stderr, "Failed to release OpenCL seed buffer: %d\n", status);
		exit(-1);
	}

	free(seeds);
	free(colors);
	free(pixels);
}

static void AllocateBuffers() {
	const int pixelCount = width * height;
	int i;
	colors = (spectrum_t *)malloc(sizeof(spectrum_t) * pixelCount);

	seeds = (unsigned int *)malloc(sizeof(unsigned int) * pixelCount * 2);
	for (i = 0; i < pixelCount * 2; i++) {
		seeds[i] = rand();
		if (seeds[i] < 2)
			seeds[i] = 2;
	}

	pixels = (unsigned int *)malloc(sizeof(unsigned int) * pixelCount);
	// Test colors
	for(i = 0; i < pixelCount; ++i)
		pixels[i] = i;

	cl_int status;
	cl_uint sizeBytes = sizeof(spectrum_t) * width * height;
	colorBuffer = clCreateBuffer(
		context,
		CL_MEM_READ_WRITE,
		sizeBytes,
		NULL,
		&status);
	if (status != CL_SUCCESS) {
		fprintf(stderr, "Failed to create OpenCL output buffer: %d\n", status);
		exit(-1);
	}

	sizeBytes = sizeof(unsigned int) * width * height;
	pixelBuffer = clCreateBuffer(
		context,
		CL_MEM_WRITE_ONLY,
		sizeBytes,
		NULL,
		&status);
	if (status != CL_SUCCESS) {
		fprintf(stderr, "Failed to create OpenCL pixel buffer: %d\n", status);
		exit(-1);
	}

	sizeBytes = sizeof(unsigned int) * width * height * 2;
	seedBuffer = clCreateBuffer(
		context,
		CL_MEM_READ_WRITE,
		sizeBytes,
		NULL,
		&status);
	if (status != CL_SUCCESS) {
		fprintf(stderr, "Failed to create OpenCL seed buffer: %d\n", status);
		exit(-1);
	}
	status = clEnqueueWriteBuffer(
		commandQueue,
		seedBuffer,
		CL_TRUE,
		0,
		sizeBytes,
		seeds,
		0,
		NULL,
		NULL);
	if (status != CL_SUCCESS) {
		fprintf(stderr, "Failed to write the OpenCL seeds buffer: %d\n", status);
		exit(-1);
	}
}

static char *ReadSources(const char *fileName) {
	FILE *file = fopen(fileName, "r");
	if (!file) {
		fprintf(stderr, "Failed to open file '%s'\n", fileName);
		exit(-1);
	}

	if (fseek(file, 0, SEEK_END)) {
		fprintf(stderr, "Failed to seek file '%s'\n", fileName);
		exit(-1);
	}

	long size = ftell(file);
	if (size == 0) {
		fprintf(stderr, "Failed to check position on file '%s'\n", fileName);
		exit(-1);
	}

	rewind(file);

	char *src = (char *)malloc(sizeof(char) * size + 1);
	if (!src) {
		fprintf(stderr, "Failed to allocate memory for file '%s'\n", fileName);
		exit(-1);
	}

	fprintf(stderr, "Reading file '%s' (size %ld bytes)\n", fileName, size);
	unsigned res = fread(src, 1, sizeof(char) * size, file);
	if (res != sizeof(char) * size) {
		fprintf(stderr, "Failed to read file '%s' (read %ld)\n", fileName, res);
		exit(-1);
	}
	src[size] = '\0'; /* NULL terminated */

	fclose(file);

	return src;

}

 void setup_opencl() {
	cl_device_type dType;

	if (useGPU)
		dType = CL_DEVICE_TYPE_GPU;
	else
		dType = CL_DEVICE_TYPE_CPU;

	// Select the platform

	cl_uint numPlatforms;
	cl_platform_id platform = NULL;
	cl_int status = clGetPlatformIDs(0, NULL, &numPlatforms);
	if (status != CL_SUCCESS) {
		fprintf(stderr, "Failed to get OpenCL platforms\n");
		exit(-1);
	}

	if (numPlatforms > 0) {
		cl_platform_id *platforms = (cl_platform_id *)malloc(sizeof(cl_platform_id) * numPlatforms);
		status = clGetPlatformIDs(numPlatforms, platforms, NULL);
		if (status != CL_SUCCESS) {
			fprintf(stderr, "Failed to get OpenCL platform IDs\n");
			exit(-1);
		}

		unsigned int i;
		for (i = 0; i < numPlatforms; ++i) {
			char pbuf[100];
			status = clGetPlatformInfo(platforms[i],
				CL_PLATFORM_VENDOR,
				sizeof(pbuf),
				pbuf,
				NULL);

			status = clGetPlatformIDs(numPlatforms, platforms, NULL);
			if (status != CL_SUCCESS) {
				fprintf(stderr, "Failed to get OpenCL platform IDs\n");
				exit(-1);
			}

			fprintf(stderr, "OpenCL Platform %d: %s\n", i, pbuf);
		}

		platform = platforms[0];
		free(platforms);
	}

	// Select the device

	cl_device_id devices[32];
	cl_uint deviceCount;
	status = clGetDeviceIDs(platform, CL_DEVICE_TYPE_ALL, 32, devices, &deviceCount);
	if (status != CL_SUCCESS) {
		fprintf(stderr, "Failed to get OpenCL device IDs\n");
		exit(-1);
	}

	int deviceFound = 0;
	cl_device_id selectedDevice;
	unsigned int i;
	for (i = 0; i < deviceCount; ++i) {
		cl_device_type type = 0;
		status = clGetDeviceInfo(devices[i],
			CL_DEVICE_TYPE,
			sizeof(cl_device_type),
			&type,
			NULL);
		if (status != CL_SUCCESS) {
			fprintf(stderr, "Failed to get OpenCL device info: %d\n", status);
			exit(-1);
		}

		char *stype;
		switch (type) {
			case CL_DEVICE_TYPE_ALL:
				stype = "TYPE_ALL";
				break;
			case CL_DEVICE_TYPE_DEFAULT:
				stype = "TYPE_DEFAULT";
				break;
			case CL_DEVICE_TYPE_CPU:
				stype = "TYPE_CPU";
				if (!useGPU && !deviceFound) {
					selectedDevice = devices[i];
					deviceFound = 1;
				}
				break;
			case CL_DEVICE_TYPE_GPU:
				stype = "TYPE_GPU";
				if (useGPU && !deviceFound) {
					selectedDevice = devices[i];
					deviceFound = 1;
				}
				break;
			default:
				stype = "TYPE_UNKNOWN";
				break;
		}
		fprintf(stderr, "OpenCL Device %d: Type = %s\n", i, stype);

		char buf[256];
		status = clGetDeviceInfo(devices[i],
			CL_DEVICE_NAME,
			sizeof(char[256]),
			&buf,
			NULL);
		if (status != CL_SUCCESS) {
			fprintf(stderr, "Failed to get OpenCL device info: %d\n", status);
			exit(-1);
		}

		fprintf(stderr, "OpenCL Device %d: Name = %s\n", i, buf);

		cl_uint units = 0;
		status = clGetDeviceInfo(devices[i],
			CL_DEVICE_MAX_COMPUTE_UNITS,
			sizeof(cl_uint),
			&units,
			NULL);
		if (status != CL_SUCCESS) {
			fprintf(stderr, "Failed to get OpenCL device info: %d\n", status);
			exit(-1);
		}

		fprintf(stderr, "OpenCL Device %d: Compute units = %u\n", i, units);

		unsigned gsize = 0;
		status = clGetDeviceInfo(devices[i],
			CL_DEVICE_MAX_WORK_GROUP_SIZE,
			sizeof(unsigned ),
			&gsize,
			NULL);
		if (status != CL_SUCCESS) {
			fprintf(stderr, "Failed to get OpenCL device info: %d\n", status);
			exit(-1);
		}

		fprintf(stderr, "OpenCL Device %d: Max. work group size = %d\n", i, (unsigned int)gsize);
	}

	if (!deviceFound) {
		fprintf(stderr, "Unable to select an appropriate device\n");
		exit(-1);
	}

	// Create the context

	cl_context_properties cps[3] = {
		CL_CONTEXT_PLATFORM,
		(cl_context_properties) platform,
		0
	};

	cl_context_properties *cprops = (NULL == platform) ? NULL : cps;
	context = clCreateContext(
		cprops,
		1,
		&selectedDevice,
		NULL,
		NULL,
		&status);
	if (status != CL_SUCCESS) {
		fprintf(stderr, "Failed to open OpenCL context\n");
		exit(-1);
	}

	/* Get the device list data */
	unsigned deviceListSize;
	status = clGetContextInfo(
		context,
		CL_CONTEXT_DEVICES,
		32,
		devices,
		&deviceListSize);
	if (status != CL_SUCCESS) {
		fprintf(stderr, "Failed to get OpenCL context info: %d\n", status);
		exit(-1);
	}

	/* Print devices list */
	for (i = 0; i < deviceListSize / sizeof(cl_device_id); ++i) {
		cl_device_type type = 0;
		status = clGetDeviceInfo(devices[i],
			CL_DEVICE_TYPE,
			sizeof(cl_device_type),
			&type,
			NULL);
		if (status != CL_SUCCESS) {
			fprintf(stderr, "Failed to get OpenCL device info: %d\n", status);
			exit(-1);
		}

		char *stype;
		switch (type) {
			case CL_DEVICE_TYPE_ALL:
				stype = "TYPE_ALL";
				break;
			case CL_DEVICE_TYPE_DEFAULT:
				stype = "TYPE_DEFAULT";
				break;
			case CL_DEVICE_TYPE_CPU:
				stype = "TYPE_CPU";
				break;
			case CL_DEVICE_TYPE_GPU:
				stype = "TYPE_GPU";
				break;
			default:
				stype = "TYPE_UNKNOWN";
				break;
		}
		fprintf(stderr, "[SELECTED] OpenCL Device %d: Type = %s\n", i, stype);

		char buf[256];
		status = clGetDeviceInfo(devices[i],
			CL_DEVICE_NAME,
			sizeof(char[256]),
			&buf,
			NULL);
		if (status != CL_SUCCESS) {
			fprintf(stderr, "Failed to get OpenCL device info: %d\n", status);
			exit(-1);
		}

		fprintf(stderr, "[SELECTED] OpenCL Device %d: Name = %s\n", i, buf);

		cl_uint units = 0;
		status = clGetDeviceInfo(devices[i],
			CL_DEVICE_MAX_COMPUTE_UNITS,
			sizeof(cl_uint),
			&units,
			NULL);
		if (status != CL_SUCCESS) {
			fprintf(stderr, "Failed to get OpenCL device info: %d\n", status);
			exit(-1);
		}

		fprintf(stderr, "[SELECTED] OpenCL Device %d: Compute units = %u\n", i, units);

		unsigned gsize = 0;
		status = clGetDeviceInfo(devices[i],
			CL_DEVICE_MAX_WORK_GROUP_SIZE,
			sizeof(unsigned ),
			&gsize,
			NULL);
		if (status != CL_SUCCESS) {
			fprintf(stderr, "Failed to get OpenCL device info: %d\n", status);
			exit(-1);
		}

		fprintf(stderr, "[SELECTED] OpenCL Device %d: Max. work group size = %d\n", i, (unsigned int)gsize);
	}

	cl_command_queue_properties prop = 0;
	commandQueue = clCreateCommandQueue(
		context,
		devices[0],
		prop,
		&status);
	if (status != CL_SUCCESS) {
		fprintf(stderr, "Failed to create OpenCL command queue: %d\n", status);
		exit(-1);
	}

	/*------------------------------------------------------------------------*/

	SetUpScene();
	primitives_buffer= clCreateBuffer(
			context,
#ifdef __APPLE__
			CL_MEM_READ_WRITE,
#else
			CL_MEM_READ_ONLY,
#endif
			sizeof(primitive_info_t) * primitive_count,
			NULL,
			&status
			);
	if (status != CL_SUCCESS) {
		fprintf(stderr, "Failed to create OpenCL primitive data buffer: %d\n", status);
		exit(-1);
	}
	status = clEnqueueWriteBuffer(
		commandQueue,
		primitives_buffer,
		CL_TRUE,
		0,
		sizeof(primitive_info_t) * primitive_count,
		primitives,
		0,
		NULL,
		NULL);
	if (status != CL_SUCCESS) {
		fprintf(stderr, "Failed to write the OpenCL primitive buffer: %d\n", status);
		exit(-1);
	}

	lights_buffer = clCreateBuffer(
			context,
#ifdef __APPLE__
			CL_MEM_READ_WRITE,
#else
			CL_MEM_READ_ONLY,
#endif
			sizeof(light_info_t) * light_count,
			NULL,
			&status
			);
	if (status != CL_SUCCESS) {
		fprintf(stderr, "Failed to create OpenCL lights buffer: %d\n", status);
		exit(-1);
	}
	status = clEnqueueWriteBuffer(
		commandQueue,
		lights_buffer,
		CL_TRUE,
		0,
		sizeof(light_info_t) * light_count,
		lights,
		0,
		NULL,
		NULL);
	if (status != CL_SUCCESS) {
		fprintf(stderr, "Failed to write the OpenCL light buffer: %d\n", status);
		exit(-1);
	}

	shape_data_buffer = clCreateBuffer(
			context,
#ifdef __APPLE__
			CL_MEM_READ_WRITE,
#else
			CL_MEM_READ_ONLY,
#endif
			sizeof(float) * shape_buffer_size,
			NULL,
			&status
			);
	if (status != CL_SUCCESS) {
		fprintf(stderr, "Failed to create OpenCL shape data buffer: %d\n", status);
		exit(-1);
	}
	status = clEnqueueWriteBuffer(
		commandQueue,
		shape_data_buffer,
		CL_TRUE,
		0,
		sizeof(float) * shape_buffer_size,
		shape_data,
		0,
		NULL,
		NULL);
	if (status != CL_SUCCESS) {
		fprintf(stderr, "Failed to write the OpenCL shape buffer: %d\n", status);
		exit(-1);
	}

	light_data_buffer = clCreateBuffer(
			context,
#ifdef __APPLE__
			CL_MEM_READ_WRITE,
#else
			CL_MEM_READ_ONLY,
#endif
			sizeof(float) * light_buffer_size,
			NULL,
			&status);
	if (status != CL_SUCCESS)
	{
		fprintf(stderr, "Failed to create OpenCL light data buffer: %d\n", status);
		exit(-1);
	}
	status = clEnqueueWriteBuffer(
		commandQueue,
		light_data_buffer,
		CL_TRUE,
		0,
		sizeof(float) * light_buffer_size,
		light_data,
		0,
		NULL,
		NULL);
	if (status != CL_SUCCESS) {
		fprintf(stderr, "Failed to write the OpenCL light buffer: %d\n", status);
		exit(-1);
	}

	material_data_buffer = clCreateBuffer(
			context,
#ifdef __APPLE__
			CL_MEM_READ_WRITE,
#else
			CL_MEM_READ_ONLY,
#endif
			sizeof(float) * material_buffer_size,
			NULL,
			&status);
	if (status != CL_SUCCESS)
	{
		fprintf(stderr, "Failed to create OpenCL material data buffer: %d\n", status);
		exit(-1);
	}
	status = clEnqueueWriteBuffer(
		commandQueue,
		material_data_buffer,
		CL_TRUE,
		0,
		sizeof(float) * material_buffer_size,
		material_data,
		0,
		NULL,
		NULL);
	if (status != CL_SUCCESS) {
		fprintf(stderr, "Failed to write the OpenCL material buffer: %d\n", status);
		exit(-1);
	}

	accelerator_data_buffer= clCreateBuffer(
			context,
#ifdef __APPLE__
			CL_MEM_READ_WRITE,
#else
			CL_MEM_READ_ONLY,
#endif
			sizeof(float) * accelerator_buffer_size,
			NULL,
			&status);
	if (status != CL_SUCCESS)
	{
		fprintf(stderr, "Failed to create OpenCL accelerator data buffer: %d\n", status);
		exit(-1);
	}
	status = clEnqueueWriteBuffer(
		commandQueue,
		accelerator_data_buffer,
		CL_TRUE,
		0,
		sizeof(float) * accelerator_buffer_size,
		accelerator_data,
		0,
		NULL,
		NULL);
	if (status != CL_SUCCESS) {
		fprintf(stderr, "Failed to write the OpenCL accelerator buffer: %d\n", status);
		exit(-1);
	}

	integrator_data_buffer = clCreateBuffer(
			context,
#ifdef __APPLE__
			CL_MEM_READ_WRITE,
#else
			CL_MEM_READ_ONLY,
#endif
			sizeof(float) * integrator_buffer_size,
			NULL,
			&status);
	if (status != CL_SUCCESS)
	{
		fprintf(stderr, "Failed to create OpenCL integrator data buffer: %d\n", status);
		exit(-1);
	}
	status = clEnqueueWriteBuffer(
		commandQueue,
		integrator_data_buffer,
		CL_TRUE,
		0,
		sizeof(float) * integrator_buffer_size,
		integrator_data,
		0,
		NULL,
		NULL);
	if (status != CL_SUCCESS) {
		fprintf(stderr, "Failed to write the OpenCL integrator buffer: %d\n", status);
		exit(-1);
	}


	texture_data_buffer = clCreateBuffer(
			context,
#ifdef __APPLE__
			CL_MEM_READ_WRITE,
#else
			CL_MEM_READ_ONLY,
#endif
			sizeof(float) * texture_buffer_size,
			NULL,
			&status);
	if (status != CL_SUCCESS)
	{
		fprintf(stderr, "Failed to create OpenCL texture data buffer: %d\n", status);
		exit(-1);
	}
	status = clEnqueueWriteBuffer(
		commandQueue,
		texture_data_buffer,
		CL_TRUE,
		0,
		sizeof(float) * texture_buffer_size,
		texture_data,
		0,
		NULL,
		NULL);
	if (status != CL_SUCCESS) {
		fprintf(stderr, "Failed to write the OpenCL texture buffer: %d\n", status);
		exit(-1);
	}
	//


	sphereBuffer = clCreateBuffer(
		context,
#ifdef __APPLE__
		CL_MEM_READ_WRITE, // NOTE: not READ_ONLY because of Apple's OpenCL bug
#else
		CL_MEM_READ_ONLY,
#endif
		sizeof(sphere_t) * sphereCount,
		NULL,
		&status);
	if (status != CL_SUCCESS) {
		fprintf(stderr, "Failed to create OpenCL scene buffer: %d\n", status);
		exit(-1);
	}
	status = clEnqueueWriteBuffer(
		commandQueue,
		sphereBuffer,
		CL_TRUE,
		0,
		sizeof(sphere_t) * sphereCount,
		spheres,
		0,
		NULL,
		NULL);
	if (status != CL_SUCCESS) {
		fprintf(stderr, "Failed to write the OpenCL scene buffer: %d\n", status);
		exit(-1);
	}

	cameraBuffer = clCreateBuffer(
		context,
#ifdef __APPLE__
		CL_MEM_READ_WRITE, // NOTE: not READ_ONLY because of Apple's OpenCL bug
#else
		CL_MEM_READ_ONLY,
#endif
		sizeof(camera_t),
		NULL,
		&status);
	if (status != CL_SUCCESS) {
		fprintf(stderr, "Failed to create OpenCL camera buffer: %d\n", status);
		exit(-1);
	}
	status = clEnqueueWriteBuffer(
		commandQueue,
		cameraBuffer,
		CL_TRUE,
		0,
		sizeof(camera_t),
		&camera,
		0,
		NULL,
		NULL);
	if (status != CL_SUCCESS) {
		fprintf(stderr, "Failed to write the OpenCL camera buffer: %d\n", status);
		exit(-1);
	}

	AllocateBuffers();

	/*------------------------------------------------------------------------*/

	/* Create the kernel program */
	const char *sources = ReadSources(kernelFileName);
	program = clCreateProgramWithSource(
		context,
		1,
		&sources,
		NULL,
		&status);
	if (status != CL_SUCCESS) {
		fprintf(stderr, "Failed to open OpenCL kernel sources: %d\n", status);
		exit(-1);
	}

#ifdef __APPLE__
	status = clBuildProgram(program, 1, devices, "-I. -D__APPLE__", NULL, NULL);
#else
	status = clBuildProgram(program, 1, devices, "-I. ", NULL, NULL);
#endif
	if (status != CL_SUCCESS) {
		fprintf(stderr, "Failed to build OpenCL kernel: %d\n", status);

		unsigned retValSize;
		status = clGetProgramBuildInfo(
			program,
			devices[0],
			CL_PROGRAM_BUILD_LOG,
			0,
			NULL,
			&retValSize);
		if (status != CL_SUCCESS) {
			fprintf(stderr, "Failed to get OpenCL kernel info size: %d\n", status);
			exit(-1);
		}

		char *buildLog = (char *)malloc(retValSize + 1);
		status = clGetProgramBuildInfo(
			program,
			devices[0],
			CL_PROGRAM_BUILD_LOG,
			retValSize,
			buildLog,
			NULL);
		if (status != CL_SUCCESS) {
			fprintf(stderr, "Failed to get OpenCL kernel info: %d\n", status);
			exit(-1);
		}
		buildLog[retValSize] = '\0';

		fprintf(stderr, "OpenCL Programm Build Log: %s\n", buildLog);
		exit(-1);
	}

	//kernel = clCreateKernel(program, "RadianceGPU", &status);
	kernel = clCreateKernel(program, "render", &status);
	if (status != CL_SUCCESS) {
		fprintf(stderr, "Failed to create OpenCL kernel: %d\n", status);
		exit(-1);
	}

	// LordCRC's patch for better workGroupSize
	unsigned gsize = 0;
	status = clGetKernelWorkGroupInfo(kernel,
		devices[0],
		CL_KERNEL_WORK_GROUP_SIZE,
		sizeof(unsigned ),
		&gsize,
		NULL);
	if (status != CL_SUCCESS) {
		fprintf(stderr, "Failed to get OpenCL kernel work group size info: %d\n", status);
		exit(-1);
	}

	workGroupSize = (unsigned int) gsize;
	fprintf(stderr, "OpenCL Device 0: kernel work group size = %d\n", workGroupSize);

	if (forceWorkSize > 0) {
		fprintf(stderr, "OpenCL Device 0: forced kernel work group size = %d\n", forceWorkSize);
		workGroupSize = forceWorkSize;
	}
}
photon_map_t *pm = NULL;
static void InitializePhotonMapping()
{
	pm = new photon_map_t;
	//////////////////////////////////////////////////////////////////////////
	photon_map_t &photon_map = *pm;
	unsigned n_caustic_photons, n_indirect_photons;
	unsigned  n_lookup;
	int specular_depth;
	int max_specular_depth;
	float max_dist_squared, rr_threshold;
	float cos_gather_angle;
	int gather_samples;
	// Declare sample parameters for light source sampling
	int n_caustic_paths, n_indirect_paths;
	photon_map.final_gather = true;//true;
	photon_map.n_caustic_paths = 0;
	photon_map.n_indirect_paths = 0;

	photon_map.n_caustic_photons = 20000;//20000;//20000;
	photon_map.n_indirect_photons = 100000;//100000;//100000;

	photon_map.n_lookup = 50;
	photon_map.max_specular_depth = 4;
	photon_map.max_dist_squared = 4.f;
	photon_map.rr_threshold = 0.01f;
	photon_map.cos_gather_angle = 0.9;//0.984f;

	photon_map.gather_samples = 32;

	photon_map.caustic_map.nodes = NULL;
	photon_map.caustic_map.node_data = NULL;
	photon_map.caustic_map.n_nodes = 0;

	photon_map.indirect_map.nodes = NULL;
	photon_map.indirect_map.node_data = NULL;
	photon_map.indirect_map.n_nodes = 0;

	photon_map.radiance_map.nodes = NULL;
	photon_map.radiance_map.node_data = NULL;
	photon_map.radiance_map.n_nodes = 0;
	Seed ps;
	init_rng(seeds[0],&ps);
	printf("start shooting photons\n");
	photon_map_init(&photon_map,light_data,material_data,shape_data,texture_data,integrator_data,accelerator_data,primitives,primitive_count,lights,light_count,&ps);
	printf("initializ photon mapping done!\n");
}
static void ExecuteKernelCPUPhotonMapping()
{
	if(!pm)
	{
		InitializePhotonMapping();
	}
	const int pixel_count = width * height;
	cl_scene_info_t scene_info;
	scene_info.light_data = light_data;
	scene_info.material_data = material_data;
	scene_info.shape_data = shape_data;
	scene_info.texture_data = texture_data;
	scene_info.integrator_data = integrator_data;
	scene_info.accelerator_data = accelerator_data;
	scene_info.primitives = primitives;
	scene_info.primitive_count = primitive_count;
	scene_info.lghts = lights;
	scene_info.lght_count = light_count;

	const int print_step = 1000;
	//#pragma omp parallel for
	for (int ii = 0;ii < pixel_count ; ++ii)
	{

		const int gid = ii;
		const int gid2 = 2 * gid;
		const int x = gid % width;
		const int y = gid / width;

		if (x == width/2 && y == height /2)
		{
			int debug_break = 0;
		}
		/* Check if we have to do something */
		if (y >= height)
			continue;

		/* LordCRC: move seed to local store */
		unsigned int seed0 = seeds[gid];
		Seed s;
		init_rng(seed0,&s);

		ray_t ray;
		GenerateCameraRay(&camera, &s, width, height, x, y, &ray);

		spectrum_t r;

		if(x == 41 && height - y - 1 == 34)
		{
			//printf("debug break;");
		}
		photon_map_li(pm,&ray,scene_info,&s,&r);
		if(color_is_black(r))
		{
			//printf("debug break %d,%d",x,y);
		}
		const int i = (height - y - 1) * width + x;
		if (currentSample == 0) {
			// Jens's patch for MacOS
			vassign(colors[i], r);
		} else {
			const float k1 = currentSample;
			const float k2 = 1.f / (currentSample + 1.f);
			colors[i].x = (colors[i].x * k1  + r.x) * k2;
			colors[i].y = (colors[i].y * k1  + r.y) * k2;
			colors[i].z = (colors[i].z * k1  + r.z) * k2;
		}

		spectrum_t c;
		c.x = colors[i].x;
		c.y = colors[i].y;
		c.z = colors[i].z;
		pixels[y * width + x] = convert_to_rgb(&c);

		seeds[gid] = random_uint(&s);
		if(ii % print_step == 0)
		{
			printf("rendering %.2f%% \t",(100*ii/float(pixel_count)));
		}
	}
	printf("frame complete \n");
}
static void ExecuteKernelCPUPathTracing()
{
	const int pixel_count = width * height;

	//#pragma omp parallel for
	for (int ii = 0;ii < pixel_count ; ++ii)
	{
	
		const int gid = ii;
		const int gid2 = 2 * gid;
		const int x = gid % width;
		const int y = gid / width;

		if (x == width/2 && y == height /2)
		{
			int debug_break = 0;
		}
		/* Check if we have to do something */
		if (y >= height)
			continue;

		/* LordCRC: move seed to local store */
		unsigned int seed0 = seeds[gid];
		Seed s;
		init_rng(seed0,&s);

		ray_t ray;
		GenerateCameraRay(&camera, &s, width, height, x, y, &ray);

		vector3f_t r;
		PathTracing(light_data,material_data,shape_data,texture_data,integrator_data,accelerator_data,primitives,primitive_count,lights,light_count,
			&ray, &s, &r);

		const int i = (height - y - 1) * width + x;
		if (currentSample == 0) {
			// Jens's patch for MacOS
			vassign(colors[i], r);
		} else {
			const float k1 = currentSample;
			const float k2 = 1.f / (currentSample + 1.f);
			colors[i].x = (colors[i].x * k1  + r.x) * k2;
			colors[i].y = (colors[i].y * k1  + r.y) * k2;
			colors[i].z = (colors[i].z * k1  + r.z) * k2;
		}

		spectrum_t c;
		c.x = colors[i].x;
		c.y = colors[i].y;
		c.z = colors[i].z;
		pixels[y * width + x] = convert_to_rgb(&c);

		seeds[gid] = random_uint(&s);
	}

}
static void GetPixelsCPU()
{
	FILE *f = fopen("image.ppm", "w"); // Write image to PPM file.
	if (!f) {
		fprintf(stderr, "Failed to open image file: image.ppm\n");
	} else {
		fprintf(f, "P3\n%d %d\n%d\n", width, height, 255);

		int x, y;
		for (y = height - 1; y >= 0; --y) {
			unsigned char *p = (unsigned char *)(&pixels[y * width]);
			for (x = 0; x < width; ++x, p += 4)
				fprintf(f, "%d %d %d ", p[0], p[1], p[2]);
		}

		fclose(f);
	}
}
static void GetPixels()
{
	
	if (debug_use_cpu)
	{
		GetPixelsCPU();
		return;
	}
	//--------------------------------------------------------------------------
	/* Enqueue readBuffer */
	cl_int status = clEnqueueReadBuffer(
		commandQueue,
		pixelBuffer,
		CL_TRUE,
		0,
		width * height * sizeof(unsigned int),
		pixels,
		0,
		NULL,
		NULL);
	if (status != CL_SUCCESS) {
		fprintf(stderr, "Failed to read the OpenCL pixel buffer: %d\n", status);
		exit(-1);
	}

	/*------------------------------------------------------------------------*/
}
static void ExecuteKernel() {
	if(debug_use_cpu) {
		ExecuteKernelCPUPhotonMapping();
		//ExecuteKernelCPUPathTracing();
		return;
	}
	/* Enqueue a kernel run call */
	unsigned globalThreads[1];
	globalThreads[0] = width * height;
	if (globalThreads[0] % workGroupSize != 0)
		globalThreads[0] = (globalThreads[0] / workGroupSize + 1) * workGroupSize;
	unsigned localThreads[1];
	localThreads[0] = workGroupSize;

	cl_int status = clEnqueueNDRangeKernel(
		commandQueue,
		kernel,
		1,
		NULL,
		globalThreads,
		localThreads,
		0,
		NULL,
		NULL);
	if (status != CL_SUCCESS) {
		fprintf(stderr, "Failed to enqueue OpenCL work: %d\n", status);
		exit(-1);
	}
}

void UpdateRendering() {
	double startTime = WallClockTime();
	int startSampleCount = currentSample;

	cl_int status = clSetKernelArg(
		kernel,
		0,
		sizeof(cl_mem),
		(void *)&colorBuffer);
	if (status != CL_SUCCESS) {
		fprintf(stderr, "Failed to set OpenCL arg. #1: %d\n", status);
		exit(-1);
	}

	status = clSetKernelArg(
		kernel,
		1,
		sizeof(cl_mem),
		(void *)&seedBuffer);
	if (status != CL_SUCCESS) {
		fprintf(stderr, "Failed to set OpenCL arg. #2: %d\n", status);
		exit(-1);
	}

	status = clSetKernelArg(
		kernel,
		2,
		sizeof(cl_mem),
		(void *)&light_data_buffer);
	if (status != CL_SUCCESS) {
		fprintf(stderr, "Failed to set OpenCL arg. #3: %d\n", status);
		exit(-1);
	}
	status = clSetKernelArg(
		kernel,
		3,
		sizeof(cl_mem),
		(void *)&material_data_buffer);
	if (status != CL_SUCCESS) {
		fprintf(stderr, "Failed to set OpenCL arg. #4: %d\n", status);
		exit(-1);
	}
	status = clSetKernelArg(
		kernel,
		4,
		sizeof(cl_mem),
		(void *)&shape_data_buffer);
	if (status != CL_SUCCESS) {
		fprintf(stderr, "Failed to set OpenCL arg. #5: %d\n", status);
		exit(-1);
	}
	status = clSetKernelArg(
		kernel,
		5,
		sizeof(cl_mem),
		(void *)&texture_data_buffer);
	if (status != CL_SUCCESS) {
		fprintf(stderr, "Failed to set OpenCL arg. #6: %d\n", status);
		exit(-1);
	}
		status = clSetKernelArg(
		kernel,
		6,
		sizeof(cl_mem),
		(void *)&integrator_data_buffer);
	if (status != CL_SUCCESS) {
		fprintf(stderr, "Failed to set OpenCL arg. #7: %d\n", status);
		exit(-1);
	}
			status = clSetKernelArg(
		kernel,
		7,
		sizeof(cl_mem),
		(void *)&accelerator_data_buffer);
	if (status != CL_SUCCESS) {
		fprintf(stderr, "Failed to set OpenCL arg. #7: %d\n", status);
		exit(-1);
	}
			status = clSetKernelArg(
		kernel,
		8,
		sizeof(cl_mem),
		(void *)&primitives_buffer);
	if (status != CL_SUCCESS) {
		fprintf(stderr, "Failed to set OpenCL arg. #9: %d\n", status);
		exit(-1);
	}
			status = clSetKernelArg(
		kernel,
		9,
		sizeof(unsigned),
		(void *)&primitive_count);
	if (status != CL_SUCCESS) {
		fprintf(stderr, "Failed to set OpenCL arg. #10: %d\n", status);
		exit(-1);
	}



			status = clSetKernelArg(
		kernel,
		10,
		sizeof(cl_mem),
		(void *)&lights_buffer);
	if (status != CL_SUCCESS) {
		fprintf(stderr, "Failed to set OpenCL arg. #11: %d\n", status);
		exit(-1);
	}

			status = clSetKernelArg(
		kernel,
		11,
		sizeof(unsigned),
		(void *)&light_count);
	if (status != CL_SUCCESS) {
		fprintf(stderr, "Failed to set OpenCL arg. #12: %d\n", status);
		exit(-1);
	}



	status = clSetKernelArg(
		kernel,
		12,
		sizeof(cl_mem),
		(void *)&cameraBuffer);
	if (status != CL_SUCCESS) {
		fprintf(stderr, "Failed to set OpenCL arg. #13: %d\n", status);
		exit(-1);
	}

	status = clSetKernelArg(
		kernel,
		13,
		sizeof(int),
		(void *)&width);
	if (status != CL_SUCCESS) {
		fprintf(stderr, "Failed to set OpenCL arg. #14: %d\n", status);
		exit(-1);
	}

	status = clSetKernelArg(
		kernel,
		14,
		sizeof(int),
		(void *)&height);
	if (status != CL_SUCCESS) {
		fprintf(stderr, "Failed to set OpenCL arg. #15: %d\n", status);
		exit(-1);
	}

	status = clSetKernelArg(
		kernel,
		15,
		sizeof(int),
		(void *)&currentSample);
	if (status != CL_SUCCESS) {
		fprintf(stderr, "Failed to set OpenCL arg. #16: %d\n", status);
		exit(-1);
	}

	status = clSetKernelArg(
		kernel,
		16,
		sizeof(cl_mem),
		(void *)&pixelBuffer);
	if (status != CL_SUCCESS) {
		fprintf(stderr, "Failed to set OpenCL arg. #17: %d\n", status);
		exit(-1);
	}

	/* Set kernel arguments */
	/*
	cl_int status = clSetKernelArg(
		kernel,
		0,
		sizeof(cl_mem),
		(void *)&colorBuffer);
	if (status != CL_SUCCESS) {
		fprintf(stderr, "Failed to set OpenCL arg. #1: %d\n", status);
		exit(-1);
	}

	status = clSetKernelArg(
		kernel,
		1,
		sizeof(cl_mem),
		(void *)&seedBuffer);
	if (status != CL_SUCCESS) {
		fprintf(stderr, "Failed to set OpenCL arg. #2: %d\n", status);
		exit(-1);
	}

	status = clSetKernelArg(
		kernel,
		2,
		sizeof(cl_mem),
		(void *)&sphereBuffer);
	if (status != CL_SUCCESS) {
		fprintf(stderr, "Failed to set OpenCL arg. #3: %d\n", status);
		exit(-1);
	}

	status = clSetKernelArg(
		kernel,
		3,
		sizeof(cl_mem),
		(void *)&cameraBuffer);
	if (status != CL_SUCCESS) {
		fprintf(stderr, "Failed to set OpenCL arg. #4: %d\n", status);
		exit(-1);
	}

	status = clSetKernelArg(
		kernel,
		4,
		sizeof(unsigned int),
		(void *)&sphereCount);
	if (status != CL_SUCCESS) {
		fprintf(stderr, "Failed to set OpenCL arg. #5: %d\n", status);
		exit(-1);
	}

	status = clSetKernelArg(
		kernel,
		5,
		sizeof(int),
		(void *)&width);
	if (status != CL_SUCCESS) {
		fprintf(stderr, "Failed to set OpenCL arg. #6: %d\n", status);
		exit(-1);
	}

	status = clSetKernelArg(
		kernel,
		6,
		sizeof(int),
		(void *)&height);
	if (status != CL_SUCCESS) {
		fprintf(stderr, "Failed to set OpenCL arg. #7: %d\n", status);
		exit(-1);
	}

	status = clSetKernelArg(
		kernel,
		7,
		sizeof(int),
		(void *)&currentSample);
	if (status != CL_SUCCESS) {
		fprintf(stderr, "Failed to set OpenCL arg. #8: %d\n", status);
		exit(-1);
	}

	status = clSetKernelArg(
		kernel,
		8,
		sizeof(cl_mem),
		(void *)&pixelBuffer);
	if (status != CL_SUCCESS) {
		fprintf(stderr, "Failed to set OpenCL arg. #9: %d\n", status);
		exit(-1);
	}
	*/

	//--------------------------------------------------------------------------

	if (currentSample < 20) {
		ExecuteKernel();
		currentSample++;
	} else {
		/* After first 20 samples, continue to execute kernels for more and more time */
		const float k = min(currentSample - 20, 100) / 100.f;
		const float tresholdTime = 0.5f * k;
		for (;;) {
			ExecuteKernel();
			clFinish(commandQueue);
			currentSample++;

			const float elapsedTime = WallClockTime() - startTime;
			if (elapsedTime > tresholdTime)
				break;
		}
	}
	GetPixels();

	const double elapsedTime = WallClockTime() - startTime;
	const int samples = currentSample - startSampleCount;
	const double sampleSec = samples * height * width / elapsedTime;
	sprintf(captionBuffer, "Rendering time %.3f sec (pass %d)  Sample/sec  %.1fK\n",
		elapsedTime, currentSample, sampleSec / 1000.f);
}

void ReInitScene() {
	if (pm)
	{
		//delete pm;
		//pm = NULL;
	}
	currentSample = 0;

	// Redownload the scene

	cl_int status = clEnqueueWriteBuffer(
		commandQueue,
		sphereBuffer,
		CL_TRUE,
		0,
		sizeof(sphere_t) * sphereCount,
		spheres,
		0,
		NULL,
		NULL);
	if (status != CL_SUCCESS) {
		fprintf(stderr, "Failed to write the OpenCL scene buffer: %d\n", status);
		exit(-1);
	}
}

void ReInit(const int reallocBuffers) {
	// Check if I have to reallocate buffers
	if (reallocBuffers) {
		FreeBuffers();
		update_camera();
		AllocateBuffers();
	} else
		update_camera();

	cl_int status = clEnqueueWriteBuffer(
		commandQueue,
		cameraBuffer,
		CL_TRUE,
		0,
		sizeof(camera_t),
		&camera,
		0,
		NULL,
		NULL);
	if (status != CL_SUCCESS) {
		fprintf(stderr, "Failed to write the OpenCL camera buffer: %d\n", status);
		exit(-1);
	}

	currentSample = 0;
}
