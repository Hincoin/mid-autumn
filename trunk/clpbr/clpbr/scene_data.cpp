#include <math.h>
#include "scene_data.h"

static std::vector<float> shape_data;
static std::vector<float> material_data;
static std::vector<float> texture_data;
static std::vector<float> light_data;
static std::vector<light_info_t> light_info_array;
static std::vector<primitive_info_t> primitive_info_array;

static void clear_scene_info()
{
	shape_data.clear();
	material_data.clear();
	texture_data.clear();
	light_data.clear();
	light_info_array.clear();
	primitive_info_array.clear();
}
cl_scene_info_t SceneData::ConvertToCLSceneInfo()
{
	cl_scene_info_t cl_scene_info;

	cl_scene_info.accelerator_data = NULL;
	cl_scene_info.integrator_data = NULL;
	cl_scene_info.light_data = NULL;

	cl_scene_info.primitive_count = 0;
	cl_scene_info.lght_count = 0;

	clear_scene_info();

	std::vector<primitive_info_t> primitive_info;
	light_info_t light_info;
	for (size_t i = 0;i < primitives_.size(); ++i)
	{
		primitives_[i]->AppendRefinedDataTo(&shape_data, &material_data, &texture_data, &primitive_info);
	}
	for(size_t j = 0;j < primitive_info.size(); ++j)
	{
		if (primitive_info[j].material_info.material_type == LIGHT_MATERIAL)
		{
			cl_scene_info.lght_count ++;
			light_info.light_type = 0;	
			light_info.memory_start = light_data.size();
			light_info_array.push_back(light_info);
			light_data.push_back(as_float(cl_scene_info.primitive_count));
		}
		cl_scene_info.primitive_count ++;
		primitive_info_array.push_back(primitive_info[j]);
	}

	cl_scene_info.lghts = &light_info_array[0];
	cl_scene_info.material_data  = &material_data[0];
	cl_scene_info.shape_data = &shape_data[0];
	cl_scene_info.texture_data = &texture_data[0];
	cl_scene_info.light_data = &light_data[0];
	cl_scene_info.primitives = &primitive_info_array[0];
	return cl_scene_info;
}

void LightMaterialData::AppendDataTo(std::vector<float> *material_data,std::vector<float>* texture_data, material_info_t* info) 
{
	color_texture_info_t emit;
	emit_->AppendDataTo(texture_data,&emit);
	info->memory_start = material_data->size();
	info->material_type = LIGHT_MATERIAL;
	material_data->push_back(as_float(emit.texture_type));
	material_data->push_back(as_float(emit.memory_start));
}

void GlassMaterialData::AppendDataTo(std::vector<float> *material_data,std::vector<float>* texture_data, material_info_t* info)
{
	//
	color_texture_info_t kr,kt;
	float_texture_info_t index;
	kr_->AppendDataTo(texture_data,&kr);
	kt_->AppendDataTo(texture_data,&kt);
	index_->AppendDataTo(texture_data,&index);

	info->memory_start = material_data->size();
	info->material_type = GLASS_MATERIAL;
	material_data->push_back(as_float(kr.texture_type));
	material_data->push_back(as_float(kr.memory_start));
	material_data->push_back(as_float(kt.texture_type));
	material_data->push_back(as_float(kt.memory_start));
	material_data->push_back(as_float(index.texture_type));
	material_data->push_back(as_float(index.memory_start));
}

void MirrorMaterialData::AppendDataTo(std::vector<float> *material_data,std::vector<float>* texture_data, material_info_t* info)
{
	//
	color_texture_info_t kr;
	kr_->AppendDataTo(texture_data,&kr);
	info->memory_start = material_data->size();
	info->material_type = MIRROR_MATERIAL;
	material_data->push_back(as_float(kr.texture_type));
	material_data->push_back(as_float(kr.memory_start));
}

void MatteMaterialData::AppendDataTo(std::vector<float> *material_data,std::vector<float>* texture_data, material_info_t* info)
{
	//
	color_texture_info_t kd;
	float_texture_info_t sigma;
	kd_->AppendDataTo(texture_data, &kd);
	sigma_->AppendDataTo(texture_data,&sigma);
	info->memory_start = material_data->size();
	info->material_type = MATTE_MATERIAL;
	material_data->push_back(as_float(kd.texture_type));
	material_data->push_back(as_float(kd.memory_start));
	material_data->push_back(as_float(sigma.texture_type));
	material_data->push_back(as_float(sigma.memory_start));
}

void PrimitiveData::AppendRefinedDataTo(std::vector<float> *shape_data, 
								 std::vector<float> *material_data, 
								 std::vector<float> *texture_data, 
								 std::vector<primitive_info_t> *primitive_info_array)
{
	std::vector<shape_info_t> refined_shape;
	material_info_t material_info;
	material_data_->AppendDataTo(material_data,texture_data,&material_info);
	shape_data_->AppendRefinedDataTo(shape_data,&refined_shape);
	primitive_info_t primitive_info;

	for (size_t i = 0;i < refined_shape.size(); ++i)
	{
		primitive_info.material_info = material_info;
		primitive_info.shape_info = refined_shape[i];
		primitive_info_array->push_back(primitive_info);
	}
}