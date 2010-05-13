
#include "MAAPI.hpp"
#include "SceneParser.hpp"
#include "Point.hpp"
#include "Vector.hpp"
#include "Color.hpp"

using namespace ma;
namespace{
	ParamSet camera_param();
	ParamSet light_param(float x,float y,float z);
	ParamSet shape_param();
	ParamSet material_param();
}



typedef Point<vector3f> point3f;

void triangle_test()
{

	maInit();
	maIdentity();
	maLookAt(0.f, 0.f, 10.f  ,  0.f ,0.f, 0 , 0, 1, 0);


	maCamera("perspective",camera_param());
	maWorldBegin();
	maAttributeBegin();	
	//maCoordinateSystem("camera");
	maLightSource("point",light_param(10,20,10));
	maLightSource("point",light_param(10,20,10));
	maLightSource("point",light_param(-10,-20,-10));
	maLightSource("point",light_param(-10,-20,-10));
	maLightSource("point",light_param(-10,20,10));
	maLightSource("point",light_param(-10,20,10));
	maLightSource("point",light_param(-10,-20,10));
	maLightSource("point",light_param(-10,-20,10));
	maLightSource("point",light_param(10,-20,10));
	maLightSource("point",light_param(10,-20,10));
	maLightSource("point",light_param(10,-20,-10));
	maLightSource("point",light_param(10,-20,-10));
	maAttributeEnd();
	maRotate(3.14f/4.f,1,0,0);

	maMaterial("matte",material_param());
	maShape("trianglemesh",shape_param());

	maWorldEnd();
	maCleanUp();
}

namespace{
	ParamSet camera_param()
	{
		ParamSet camera_p ;
		camera_p.add("hither",1e-3f);
		camera_p.add("yon",1e30f);
		camera_p.add("shutteropen",0.f);
		camera_p.add("shutterclose",1.f);
		camera_p.add("lensradius",0.f);
		camera_p.add("focaldistance",1e30f);
		camera_p.add("frameaspectratio",3/4.f);
		return camera_p;
	}

	ParamSet light_param(float x,float y,float z)
	{
		ParamSet light_p;
		light_p.add("from", point3f(x,y,z));
		light_p.add("I",Spectrumf(100.f));
		return light_p;
	}
	ParamSet shape_param()
	{
		//ParamSet shape_p;

		//return shape_p;
		return parseObjTriangleMesh("triangle_test.obj");
	}
	ParamSet material_param()
	{
		ParamSet material_p;


		return material_p;
	}
}
