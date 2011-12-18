#ifndef _SCENE_DATA_H_
#define _SCENE_DATA_H_

#include "cl_scene.h"
#include "triangle_array.h"
#include "triangle_mesh.h"
#include "texture.h"
#include "material.h"
#include "material_funcs.h"

template<typename TextureScalar>
struct TextureInfoTraits;
template<>
struct TextureInfoTraits<float>{
	typedef float_texture_info_t texture_info_t;
};
template<>
struct TextureInfoTraits<spectrum_t>{
	typedef color_texture_info_t texture_info_t;
};

template<typename T>
class TextureData{
public:
	virtual ~TextureData(){}
	virtual void AppendDataTo(std::vector<float>* data, typename TextureInfoTraits<T>::texture_info_t* texture_info) = 0;
};
template<typename T>
class ConstantTextureData:public TextureData<T>{
	TextureType get_constant_texture_type(float ){return FLOAT_CONSTANT;}
	TextureType get_constant_texture_type(spectrum_t ){return COLOR_CONSTANT;}
	void AppendDataTo(std::vector<float>* data,float x)
	{
		data->push_back(x);
	}

	void AppendDataTo(std::vector<float>* data,const spectrum_t& c)
	{
		save_float3(*data,c);
	}
public:
	ConstantTextureData(const T& value):value_(value){}
	virtual void AppendDataTo(std::vector<float>* data, typename TextureInfoTraits<T>::texture_info_t* texture_info) 
	{
		texture_info->texture_type = get_constant_texture_type(T());
		texture_info->memory_start = (unsigned)data->size();
		AppendDataTo(data,value_);
	}
private:
	T value_;
};
//////////////////////////////////////////////////////////////////////////
class MaterialData
{
public:
	virtual ~MaterialData(){}
	virtual void AppendDataTo(std::vector<float> *material_data,std::vector<float>* texture_data, material_info_t* info) = 0;
protected:
private:
};
class LightMaterialData:public MaterialData
{
public:
	LightMaterialData(TextureData<spectrum_t> *emit):emit_(emit){}
	virtual void AppendDataTo(std::vector<float> *material_data,std::vector<float>* texture_data, material_info_t* info) ;
protected:
private:
	TextureData<spectrum_t> *emit_;
};
class GlassMaterialData:public MaterialData
{
public:
	GlassMaterialData(TextureData<spectrum_t> *kr,
		TextureData<spectrum_t> *kt,
		TextureData<float>* index){
			kr_ = kr;
			kt_ = kt;
			index_ = index;
	}
	virtual void AppendDataTo(std::vector<float> *material_data,std::vector<float>* texture_data, material_info_t* info) ;
private:
	TextureData<spectrum_t> *kr_,*kt_;
	TextureData<float> *index_;
};
class MatteMaterialData:public MaterialData
{
	//
public:
	MatteMaterialData(TextureData<spectrum_t> *kd,
		TextureData<float> *sigma):kd_(kd),sigma_(sigma){}
	virtual void AppendDataTo(std::vector<float> *material_data,std::vector<float>* texture_data, material_info_t* info) ;
private:
	TextureData<spectrum_t> *kd_;
	TextureData<float> *sigma_;
};

class MirrorMaterialData:public MaterialData
{
public:
	MirrorMaterialData(TextureData<spectrum_t> *kr):kr_(kr){}
	virtual void AppendDataTo(std::vector<float> *material_data,std::vector<float>* texture_data, material_info_t* info) ;
protected:
private:
	TextureData<spectrum_t> *kr_;
};
//////////////////////////////////////////////////////////////////////////
class ShapeData
{
public:
	virtual void AppendRefinedDataTo(std::vector<float>* shape_data,
		std::vector<shape_info_t> *refined_shape) = 0;
	virtual ~ShapeData(){}
protected:
private:
};
template<typename IndexType>
class TriangleMeshShapeData:public ShapeData 
{
public:
	TriangleMeshShapeData(TriangleMesh<IndexType> *mesh):mesh_data_(mesh){}
	virtual void AppendRefinedDataTo(std::vector<float>* shape_data,
		std::vector<shape_info_t> *refined_shape);
protected:
private:
	TriangleMesh<IndexType> *mesh_data_;
};
template<typename IndexType>
void TriangleMeshShapeData<IndexType>::AppendRefinedDataTo(std::vector<float>* shape_data, std::vector<shape_info_t> *refined_shape)
{
	unsigned mesh_start = (unsigned)shape_data->size();
	shape_data->push_back(as_float(mesh_data_->number_triangle));
	shape_data->push_back(as_float(mesh_data_->number_vertex));
	shape_data->push_back(as_float(mesh_data_->reverse_orientation));
	for(int i = 0;i < 16;++i)
		shape_data->push_back(mesh_data_->object_to_world.m[i]);
	for(size_t i = 0;i < mesh_data_->number_vertex;++i)
	{
		point3f_t p;
		p.x = mesh_data_->point_buffer[3*i];
		p.y = mesh_data_->point_buffer[3*i+1];
		p.z = mesh_data_->point_buffer[3*i+2];
		save_float3(*shape_data,p,FLOAT3_96BIT);
	}
	for(size_t i = 0;i < mesh_data_->number_vertex;++i)
	{
		normal3f_t n;
		n.x = mesh_data_->normal_buffer[3*i];
		n.y = mesh_data_->normal_buffer[3*i+1];
		n.z = mesh_data_->normal_buffer[3*i+2];
		save_float3(*shape_data,n,FLOAT3_96BIT);
	}
	for(size_t i = 0;i < mesh_data_->number_vertex;++i)
	{
		vector3f_t v;
		v.x = mesh_data_->tangent_buffer[3*i];
		v.y = mesh_data_->tangent_buffer[3*i+1];
		v.z = mesh_data_->tangent_buffer[3*i+2];
		save_float3(*shape_data,v,FLOAT3_96BIT);
	}

	for(size_t i = 0;i < mesh_data_->number_vertex;++i)
	{
		shape_data->push_back(mesh_data_->uvs_buffer[2*i]);
		shape_data->push_back(mesh_data_->uvs_buffer[2*i + 1]);
	}
	static const int index_per_float = sizeof(float)/sizeof(IndexType);
	union{
		unsigned int a;
		IndexType b[index_per_float];
	}value;
	shape_info_t triangle;
	triangle.shape_type = TriangleTraits<IndexType>::triangle_type;

	std::vector<unsigned> triangle_index;
	size_t index_count = mesh_data_->number_triangle * 3;
	for(size_t i = 0;i < index_count;)
	{
		value.a = 0;
		for(int k= 0;k < index_per_float;++k)
		{
			if (i < index_count)
			{
				if(i % 3 == 0) {
					triangle_index.push_back(unsigned(i)/3);
				}
				value.b[k] = mesh_data_->vertex_index_buffer[i];
				i++;
			}
		}
		shape_data->push_back(as_float(value.a));
	}
	for(size_t i = 0;i < triangle_index.size(); ++i)
	{
		triangle.memory_start = (unsigned)shape_data->size();
		shape_data->push_back(as_float(mesh_start));
		shape_data->push_back(as_float(triangle_index[i]));
		refined_shape->push_back(triangle);
	}
}
//////////////////////////////////////////////////////////////////////////
class PrimitiveData
{
public:
	PrimitiveData(MaterialData* material, ShapeData* shape):material_data_(material),
		shape_data_(shape){}
	void AppendRefinedDataTo(std::vector<float> *shape_data,
		std::vector<float> *material_data,
		std::vector<float> *texture_data,
		std::vector<primitive_info_t> *primitive_info);
protected:
private:
	MaterialData* material_data_;
	ShapeData* shape_data_;		
};
struct scene_info_memory_t
{
	std::vector<float> light_data;
	std::vector<float> material_data;
	std::vector<float> shape_data;
	std::vector<float> texture_data;
	std::vector<float> integrator_data;
	std::vector<float> accelerator_data;

	std::vector<primitive_info_t> primitives;
	std::vector<light_info_t> lghts; 
};
class SceneData{
public:
	SceneData(const std::vector<PrimitiveData*>& primitives)
		:primitives_(primitives)
	{}
	scene_info_memory_t ConvertToCLSceneInfo();
private:
	std::vector<PrimitiveData*> primitives_;
	bbox_t world_bound_;
};

cl_scene_info_t as_cl_scene_info(scene_info_memory_t& scene_mem);

#endif