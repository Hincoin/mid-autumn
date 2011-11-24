#include <math.h>
#include "scene_data.h"

static std::vector<float> shape_data;
static std::vector<float> material_data;
static std::vector<float> texture_data;
static std::vector<float> light_data;
static std::vector<float> accelerator_data;
static std::vector<float> integrator_data;
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
cl_scene_info_t as_cl_scene_info(scene_info_memory_t& scene_mem)
{
	cl_scene_info_t cl_scene_info;
	cl_scene_info.lghts = &scene_mem.lghts[0];
	cl_scene_info.material_data  = &scene_mem.material_data[0];
	cl_scene_info.shape_data = &scene_mem.shape_data[0];
	cl_scene_info.texture_data = &scene_mem.texture_data[0];
	cl_scene_info.light_data = &scene_mem.light_data[0];
	cl_scene_info.primitives = &scene_mem.primitives[0];

	cl_scene_info.lght_count = (unsigned)scene_mem.lghts.size();
	cl_scene_info.primitive_count = (unsigned)scene_mem.primitives.size();

	cl_scene_info.accelerator_data = scene_mem.accelerator_data.empty()? NULL : &scene_mem.accelerator_data[0];
	cl_scene_info.integrator_data = scene_mem.integrator_data.empty()? NULL : &scene_mem.integrator_data[0];
	return cl_scene_info;
}
scene_info_memory_t SceneData::ConvertToCLSceneInfo()
{
	scene_info_memory_t cl_scene_info;

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
			light_info.light_type = 0;	
			light_info.memory_start = unsigned(light_data.size());
			light_info_array.push_back(light_info);
			light_data.push_back(as_float(j));
		}
		primitive_info_array.push_back(primitive_info[j]);
	}

	cl_scene_info.lghts = light_info_array;
	cl_scene_info.material_data  = material_data;
	cl_scene_info.shape_data = shape_data;
	cl_scene_info.texture_data = texture_data;
	cl_scene_info.light_data = light_data;
	cl_scene_info.primitives = primitive_info_array;

	accelerator_data.push_back(0.f);
	cl_scene_info.accelerator_data = accelerator_data;
	cl_scene_info.integrator_data = integrator_data;

	return cl_scene_info;
}

void LightMaterialData::AppendDataTo(std::vector<float> *material_data,std::vector<float>* texture_data, material_info_t* info) 
{
	color_texture_info_t emit;
	emit_->AppendDataTo(texture_data,&emit);
	info->memory_start = unsigned(material_data->size());
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

	info->memory_start =unsigned( material_data->size() );
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
	info->memory_start = unsigned(material_data->size());
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
	info->memory_start = unsigned( material_data->size() );
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