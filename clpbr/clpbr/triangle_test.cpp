#include <math.h>
#include "triangle_mesh_ocl.h"
#include "triangle_mesh.h"
#include "material_funcs.h"
#include "scene_data.h"
#include "random_sampler.h"
#include "image_film.h"
#include "perspective_camera.h"
#include "progressive_photon_map_renderer.h"

/*
typedef struct{
	cl_uint entity_size;
	cl_uint *entity_index_array;
	float *memory;
}cl_memory_t;

class CLMemory{
private:
	std::vector<cl_uint> entity_index_array;
	std::vector<float> memory;
public:
	template<typename IndexType>
	void AddTriangleMesh(const TriangleMesh<IndexType>& triangle_mesh);
};
template<typename IndexType>
void add_triangle_mesh(cl_memory_t *memory, const TriangleMesh<IndexType>& triangle_mesh)
{
	//
}

*/

TriangleMesh<cl_uchar>* build_triangle_mesh(int *index_buffer,int index_count,
											vector3f_t *point_array,
											int point_count,
											normal3f_t *normals,
											float uvs[][2],
											vector3f_t *tangent)
{
	TriangleMesh<cl_uchar>* mesh = new TriangleMesh<cl_uchar>();
	transform_identity(mesh->object_to_world);
	mesh->normal_buffer = new float[3*point_count];
	mesh->number_triangle = index_count / 3;
	mesh->number_vertex = point_count;
	mesh->reverse_orientation = 0;
	mesh->point_buffer = new float[3*point_count];
	mesh->tangent_buffer = new float[3*point_count];
	mesh->uvs_buffer = new float [2*point_count];
	mesh->vertex_index_buffer= new cl_uchar[index_count];

	for(int i = 0;i < point_count;i++)
	{
		mesh->normal_buffer[3*i]=normals[i].x;
		mesh->normal_buffer[3*i+1]=normals[i].y;
		mesh->normal_buffer[3*i+2]=normals[i].z;

		mesh->point_buffer[3*i] = point_array[i].x;
		mesh->point_buffer[3*i+1] = point_array[i].y;
		mesh->point_buffer[3*i + 2] = point_array[i].z;

		mesh->uvs_buffer[2*i]=uvs[i][0];
		mesh->uvs_buffer[2*i + 1]=uvs[i][1];

		mesh->tangent_buffer[3*i] = tangent[i].x;
		mesh->tangent_buffer[3*i + 1] = tangent[i].y;
		mesh->tangent_buffer[3*i + 2] = tangent[i].z;

	}
	for (int i = 0;i < index_count; ++i)
	{
		mesh->vertex_index_buffer[i] = (cl_uchar)index_buffer[i];
	}
	return mesh;
}
void get_triangle_mesh_shapes(std::vector<TriangleMesh<cl_uchar>* > *mesh_shape)
{
	int ceil_light_index_buffer[] = {0,1,2,2,3,0};
	vector3f_t ceil_light_points[] = {{256,508.8f,200},{256,508.8f,259.2f},{200,508.8f,259.2f},{200,508.8f,200}};
	normal3f_t ceil_light_normals[] = {{0,-1,0},{0,-1,0},{0,-1,0},{0,-1,0}};
	float uvs[][2] = {{0,0},{0,1},{1,1},{1,0}};
	vector3f_t ceil_light_tangent[] ={{0,0,0},{0,0,0},{0,0,0},{0,0,0}};
	mesh_shape->push_back(build_triangle_mesh(ceil_light_index_buffer,
		sizeof(ceil_light_index_buffer)/sizeof(ceil_light_index_buffer[0]),
		ceil_light_points,
		sizeof(ceil_light_points)/sizeof(sizeof(ceil_light_points[0])),
		ceil_light_normals,
		uvs,
		ceil_light_tangent
		));
	//Cornell box
	int floor_index_buffer[] = {0,1,2,2,3,0};
	vector3f_t floor_points[] = {{552.8f,0,0},{0,0,0},{0,0,559.2f},{552.8f,0,559.2f},{549.6f,0,559.2f}};
	normal3f_t floor_normals[] = {{0,1,0},{0,1,0},{0,1,0},{0,1,0}};
	float floor_uvs[][2] = {{0,0},{0,1},{1,1},{1,0}};
	vector3f_t floor_tangent[] = {{0,0,0},{0,0,0},{0,0,0},{0,0,0}};
	mesh_shape->push_back(build_triangle_mesh(
		floor_index_buffer,
		sizeof(floor_index_buffer)/sizeof(floor_index_buffer[0]),
		floor_points,
		sizeof(floor_points)/sizeof(floor_points[0]),
		floor_normals,
		uvs,
		floor_tangent
		));

	int ceil_index_buffer[] = {0,1,2,2,3,0};
	vector3f_t ceil_points[] = {{556,548.8f,0},{556,548.8f,559.2f},{0,548.8f,559.2f},{0,548.8f,0}};
	normal3f_t ceil_normals[] = {{0,-1,0},{0,-1,0},{0,-1,0},{0,-1,0}};
	float ceil_uvs[][2] = {{0,0,},{0,1},{1,1},{1,0}};
	vector3f_t ceil_tangent[] = {{0,0,0},{0,0,0},{0,0,0},{0,0,0}};
	mesh_shape->push_back(build_triangle_mesh(
		ceil_index_buffer,
		sizeof(ceil_index_buffer)/sizeof(ceil_index_buffer[0]),
		ceil_points,
		sizeof(ceil_points)/sizeof(ceil_points[0]),
		ceil_normals,
		ceil_uvs,
		ceil_tangent
		));


	int back_index_buffer[] = {0,1,2,2,3,0};
	vector3f_t back_points[] = {{549.6f,0,559.2f},{0,0,559.2f},{0,548.8f,559.2f},{556.0f,548.8f,559.2f}};
	normal3f_t back_normals[]= {{0,0,-1},{0,0,-1},{0,0,-1},{0,0,-1}};
	float back_uvs[][2] = {{0,0},{0,1},{1,1},{1,0}};
	vector3f_t back_tangent[]={{0,0,0},{0,0,0},{0,0,0},{0,0,0}};
	mesh_shape->push_back(build_triangle_mesh(
		back_index_buffer,
		sizeof(back_index_buffer)/sizeof(back_index_buffer[0]),
		back_points,
		sizeof(back_points)/sizeof(back_points[0]),
		back_normals,
		back_uvs,
		back_tangent
		));

	int right_index_buffer[] = {0,1,2,2,3,0};
	vector3f_t right_points[] = {{0,0,559.2f},{0,0,0},{0,548.8f,0},{0,548.8f,559.2f}};
	normal3f_t right_normals[] = {{1,0,0},{1,0,0},{1,0,0},{1,0,0}};
	float right_uvs[][2] = {{0,0},{0,1},{1,1},{1,0}};
	vector3f_t right_tangent[] = {{0,0,0},{0,0,0},{0,0,0},{0,0,0}};
	mesh_shape->push_back(
		build_triangle_mesh(right_index_buffer,sizeof(right_index_buffer)/sizeof(right_index_buffer[0]),
		right_points,sizeof(right_points)/sizeof(right_points[0]),
		right_normals,
		right_uvs,right_tangent)
		);

	int left_index_buffer[] = {0,1,2,2,3,0};
	vector3f_t left_points[] = {{552.8f,0,0},{549.6f,0,559.2f},{556.0f,548.8f,559.2f},{556.0f,548.8f,0}};
	normal3f_t left_normals[] = {{-1,0,0},{-1,0,0},{-1,0,0},{-1,0,0}};
	float left_uvs[][2] = {
		{0,0},{0,1},{1,1},{1,0}
	};
	vector3f_t left_tangent[] = {
		{0,0,0},{0,0,0},{0,0,0},{0,0,0}
	};
	mesh_shape->push_back(
		build_triangle_mesh(left_index_buffer,sizeof(left_index_buffer)/sizeof(left_index_buffer[0]),
		left_points,sizeof(left_points)/sizeof(left_points[0]),
		left_normals,
		left_uvs,left_tangent)
		);


	int short_block_index_buffer[] ={
		0,1,2, 2,3,0,
		4,5,6, 6,7,4,
		8,9,10,10,11,8,
		12,13,14,14,15,12,
		16,17,18,18,19,16,
	};
	vector3f_t short_block_points[]={
		{130.0,165.0, 65.0}, 
		{ 82.0,165.0,225.0},
		{240.0,165.0,272.0},
		{290.0,165.0,114.0},

		{290.0,  0.0,114.0},
		{290.0,165.0,114.0},
		{240.0,165.0,272.0},
		{240.0,  0.0,272.0},

		{130.0,  0.0, 65.0},
		{130.0,165.0, 65.0},
		{290.0,165.0,114.0},
		{290.0,  0.0,114.0},

		{ 82.0,  0.0,225.0},
		{ 82.0,165.0,225.0},
		{130.0,165.0, 65.0},
		{130.0,  0.0, 65.0},

		{240.0,  0.0,272.0},
		{240.0,165.0,272.0},
		{ 82.0,165.0,225.0},
		{ 82.0,  0.0,225.0},
	};
	normal3f_t vs0 = {0,1,0};
	normal3f_t v0, v1;
	vsub(v0, short_block_points[5],short_block_points[4]);
	vsub(v1, short_block_points[6],short_block_points[4]);
	normal3f_t vs1;
	vxcross(vs1,v0,v1);
	vnorm(vs1);
	vsub(v0, short_block_points[9],short_block_points[8]);
	vsub(v1, short_block_points[10],short_block_points[8]);
	normal3f_t vs2;
	vxcross(vs2,v0,v1);
	vnorm(vs2);
	vsub(v0, short_block_points[13],short_block_points[12]);
	vsub(v1, short_block_points[14],short_block_points[12]);
	normal3f_t vs3;
	vxcross(vs3,v0,v1);
	vnorm(vs3);
	vsub(v0, short_block_points[17],short_block_points[16]);
	vsub(v1, short_block_points[18],short_block_points[16]);
	normal3f_t vs4;
	vxcross(vs4,v0,v1);
	vnorm(vs4);
	normal3f_t short_block_normals[] = {
		vs0,vs0,vs0,vs0,
		vs1,vs1,vs1,vs1,
		vs2,vs2,vs2,vs2,
		vs3,vs3,vs3,vs3,
		vs4,vs4,vs4,vs4,
	};
	float short_block_uvs[][2] = {
		{0,0},{0,1},{1,1},{1,0},
		{0,0},{0,1},{1,1},{1,0},
		{0,0},{0,1},{1,1},{1,0},
		{0,0},{0,1},{1,1},{1,0},
		{0,0},{0,1},{1,1},{1,0}
	};
	vector3f_t short_block_tangent[]={
		{0,0,0},{0,0,0},{0,0,0},{0,0,0},
		{0,0,0},{0,0,0},{0,0,0},{0,0,0},
		{0,0,0},{0,0,0},{0,0,0},{0,0,0},
		{0,0,0},{0,0,0},{0,0,0},{0,0,0},
		{0,0,0},{0,0,0},{0,0,0},{0,0,0}
	};
	mesh_shape->push_back(build_triangle_mesh(short_block_index_buffer,sizeof(short_block_index_buffer)/sizeof(short_block_index_buffer[0]),
		short_block_points,sizeof(short_block_points)/sizeof(short_block_points[0]),
		short_block_normals,
		short_block_uvs,
		short_block_tangent));

	int tall_block_index_buffer[]={
		0,1,2, 2,3,0,
		4,5,6, 6,7,4,
		8,9,10,10,11,8,
		12,13,14,14,15,12,
		16,17,18,18,19,16,
	};
	vector3f_t tall_block_points[]={
		{423.0, 330.0, 247.0},
		{265.0, 330.0, 296.0},
		{314.0, 330.0, 456.0},
		{472.0, 330.0, 406.0},

		{423.0,   0.0, 247.0},
		{423.0, 330.0, 247.0},
		{472.0, 330.0, 406.0},
		{472.0,   0.0, 406.0},

		{472.0,   0.0, 406.0},
		{472.0, 330.0, 406.0},
		{314.0, 330.0, 456.0},
		{314.0,   0.0, 456.0},

		{314.0,   0.0, 456.0},
		{314.0, 330.0, 456.0},
		{265.0, 330.0, 296.0},
		{265.0,   0.0, 296.0},

		{265.0,   0.0, 296.0},
		{265.0, 330.0, 296.0},
		{423.0, 330.0, 247.0},
		{423.0,   0.0, 247.0},
	};

	vsub(v0,tall_block_points[5],tall_block_points[4]);
	vsub(v1,tall_block_points[6],tall_block_points[4]);
	vxcross(vs1,v0,v1);
	vnorm(vs1);
	vsub(v0,tall_block_points[9],tall_block_points[8]);
	vsub(v1,tall_block_points[10],tall_block_points[8]);
	vxcross(vs2,v0,v1);
	vnorm(vs2);
	vsub(v0,tall_block_points[13],tall_block_points[12]);
	vsub(v1,tall_block_points[14],tall_block_points[12]);
	vxcross(vs3,v0,v1);
	vnorm(vs3);
	vsub(v0,tall_block_points[17],tall_block_points[16]);
	vsub(v1,tall_block_points[18],tall_block_points[16]);
	vxcross(vs4,v0,v1);
	vnorm(vs4);
	normal3f_t tall_block_normals[] = {
		vs0,vs0,vs0,vs0,
		vs1,vs1,vs1,vs1,
		vs2,vs2,vs2,vs2,
		vs3,vs3,vs3,vs3,
		vs4,vs4,vs4,vs4,
	};
	float tall_block_uvs[][2] = {
		{0,0},{0,1},{1,1},{1,0},
		{0,0},{0,1},{1,1},{1,0},
		{0,0},{0,1},{1,1},{1,0},
		{0,0},{0,1},{1,1},{1,0},
		{0,0},{0,1},{1,1},{1,0}
	};
	vector3f_t tall_block_tangent[]={
		{0,0,0},{0,0,0},{0,0,0},{0,0,0},
		{0,0,0},{0,0,0},{0,0,0},{0,0,0},
		{0,0,0},{0,0,0},{0,0,0},{0,0,0},
		{0,0,0},{0,0,0},{0,0,0},{0,0,0},
		{0,0,0},{0,0,0},{0,0,0},{0,0,0},
	};
	mesh_shape->push_back(build_triangle_mesh(tall_block_index_buffer,sizeof(tall_block_index_buffer)/sizeof(tall_block_index_buffer[0]),
		tall_block_points,sizeof(tall_block_points)/sizeof(tall_block_points[0]),
		tall_block_normals,
		tall_block_uvs,
		tall_block_tangent));
}
void triangle_test()
{

	MaterialData* matte_white = new MatteMaterialData(new ConstantTextureData<spectrum_t>(spectrum_t(1.f,1.f,1.f)),new ConstantTextureData<float>(0.f));
	MaterialData* matte_blue = new MatteMaterialData(new ConstantTextureData<spectrum_t>(spectrum_t(0.f,0.f,1.f)),new ConstantTextureData<float>(0.f));
	MaterialData* matte_red = new MatteMaterialData(new ConstantTextureData<spectrum_t>(spectrum_t(1.f,0.f,0.f)),new ConstantTextureData<float>(0.f));
	MaterialData* matte_green = new MatteMaterialData(new ConstantTextureData<spectrum_t>(spectrum_t(0.f,1.f,0.f)),new ConstantTextureData<float>(0.f));
	MaterialData* matte_gray = new MatteMaterialData(new ConstantTextureData<spectrum_t>(spectrum_t(.4f,0.5f,0.7f)),new ConstantTextureData<float>(0.f));
	MaterialData* glass = new GlassMaterialData(new ConstantTextureData<spectrum_t>(spectrum_t(1.f,1.f,1.f)),new ConstantTextureData<spectrum_t>(spectrum_t(1.f,1.f,1.f)),new ConstantTextureData<float>(1.7f));
	MaterialData* mirror= new MirrorMaterialData(new ConstantTextureData<spectrum_t>(spectrum_t(1.f,1.f,1.f)));
	MaterialData* light_material = new LightMaterialData(new ConstantTextureData<spectrum_t>(spectrum_t(250.f,250.f,250.f)));

	std::vector<TriangleMesh<cl_uchar>* > meshes;
	get_triangle_mesh_shapes(&meshes);
	PrimitiveData* light = new PrimitiveData(light_material,new TriangleMeshShapeData<cl_uchar>(meshes[0]));
	PrimitiveData* floor_wall = new PrimitiveData(matte_white,new TriangleMeshShapeData<cl_uchar>(meshes[1]));
	PrimitiveData* ceil_wall = new PrimitiveData(matte_blue,new TriangleMeshShapeData<cl_uchar>(meshes[2]));
	PrimitiveData* back_wall = new PrimitiveData(matte_red,new TriangleMeshShapeData<cl_uchar>(meshes[3]));
	PrimitiveData* right_wall = new PrimitiveData(matte_green,new TriangleMeshShapeData<cl_uchar>(meshes[4]));
	PrimitiveData* left_wall = new PrimitiveData(matte_gray,new TriangleMeshShapeData<cl_uchar>(meshes[5]));
	PrimitiveData* short_block = new PrimitiveData(glass,new TriangleMeshShapeData<cl_uchar>(meshes[6]));
	PrimitiveData* tall_block = new PrimitiveData(mirror,new TriangleMeshShapeData<cl_uchar>(meshes[7]));

	std::vector<PrimitiveData*> primitives;
	primitives.push_back(light);
	primitives.push_back(floor_wall);
	primitives.push_back(ceil_wall);
	primitives.push_back(back_wall);
	primitives.push_back(right_wall);
	primitives.push_back(left_wall);
	primitives.push_back(short_block);
	primitives.push_back(tall_block);
	SceneData* scene_data = new SceneData(primitives);

	transform_t camera_to_world,world_to_camera;
	lookat(278,273,-800,278,273,400,0,1,0,&world_to_camera);
	transform_inverse(camera_to_world,world_to_camera);

	int w,h;
	w=h = 256;
	Film* film = new ImageFilm(w,h);
	PerspectiveCamera* camera = new PerspectiveCamera(camera_to_world,screen_window_t(1.f),degree_to_radian(degree_t(40)),film);
	RandomSampler* sampler = new RandomSampler(0,w,0,h,4,4);
	
	photon_map_t* photon_map = new photon_map_t();
	photon_map->final_gather = false;
	photon_map->cos_gather_angle = 0.95f;
	photon_map->gather_samples = 32;
	
	photon_map->max_dist_squared = 30;
	photon_map->max_specular_depth = 5;
	photon_map->n_caustic_paths = 0;
	photon_map->n_caustic_photons = 0;
	photon_map->n_indirect_paths = 0;
	photon_map->n_indirect_photons = 100000;
	photon_map->n_lookup = 100;
	photon_map->total_photons = 0;
	photon_map->rr_threshold = 0.125f;
	photon_map->progressive_iteration = 0;

	photon_map->alpha = 0.618f;

	PPMRenderer* renderer = new PPMRenderer(camera,film,sampler,photon_map);
	renderer->Render(scene_data->ConvertToCLSceneInfo());
	delete photon_map;
}


