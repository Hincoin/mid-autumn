
#include "api.h"

namespace clpbr{
	using std::map;
	using std::string;
	struct GraphicsState{
		GraphicsState();
		//map<string
	};
	void begin(const char* name)
	{}
	void end()
	{}
	void frame_begin(unsigned int frame)
	{
		//
	}
	void frame_end()
	{
		//
	}
	void world_begin()
	{
		//
	}
	void world_end()
	{
		//
	}
	void attribute_begin()
	{
		//
	}
	void attribute_end()
	{
		//
	}
	void transform_begin()
	{
		//
	}
	void transform_end()
	{
		//
	}
	void object_begin(const char* name)
	{
		//
	}
	void object_end()
	{
		//
	}

	void film(const char* name, const ParameterMap& param)
	{
		//
	}
	void shape(const char* name ,const ParameterMap& param)
	{
		//
	}
	void light_source(const char* name , const ParameterMap& param)
	{

	}
	void material(const char* name, const ParameterMap& param)
	{

	}
	void texture(const char *name, const ParameterMap& param)
	{

	}

	void identity()
	{

	}
	void translate(float tx,float ty,float tz)
	{

	}
	void rotate(float angle,float ax,float ay,float az)
	{

	}
	void scale(float sx,float sy,float sz)
	{
		//
	}
	void look_at(float ex,float ey,float ez,
		float lx,float ly,float lz,
		float ux,float uy,float uz)
	{

	}
	void coordinate_system(const char* name)
	{

	}
	void coordinate_system_transform(const char* name)
	{
		//
	}
	void transform(float tr[16])
	{
		//
	}
	void concate_transform(float tr[16])
	{
		//
	}
}
