

#include "MAAPI.hpp"
#include "SceneParser.hpp"
#include "Point.hpp"
#include "Vector.hpp"

using namespace ma;
//
//Film "image"
//#	"integer xresolution" [128] "integer yresolution" [128] 
//LookAt 0 .2 .2    -.02 .1 0  0 1 0
//Camera "perspective" "float fov" [60]
//
//WorldBegin
//
//AttributeBegin
//CoordSysTransform "camera"
//#LightSource "point" "color I" [ .3 .3 .3 ]
//LightSource "point" "color I" [ .7 .7 .7 ]
//AttributeEnd
//
//AttributeBegin
//#AreaLightSource "area" "color L" [60 60 60 ] "integer nsamples"  [3]
//AreaLightSource "area" "color L" [15 15 15 ] "integer nsamples"  [8]
//Translate 0 2 0
//Rotate 90 1 0 0
//Shape "disk" "float radius" [.25]
//AttributeEnd
//
//Material "matte" "color Kd" [.4 .42 .4]
//Shape "trianglemesh" "point P" [ -1 0 -1 1 0 -1 1 0 1 -1 0 1 ]
//"integer indices" [ 0 1 2 2 3 0]
//
//Material "bluepaint"
//
//Include "geometry/bunny.pbrt"
//
//WorldEnd

//key words : 
//point
//indices
//st
//normal
extern float bunny_vertices[];
extern float bunny_indices[];
extern float bunny_uvs[];

ParamSet camera_param();
ParamSet light_param(float x,float y,float z);
ParamSet shape_param();
ParamSet material_param();
using namespace ma;


typedef Point<vector3f> point3f;

void bunny_test()
{

	maInit();
	maIdentity();
	maLookAt(0, 1.4f, 1.5f  ,  -0.2f ,0.8f, 0 , 0, 1, 0);


	maCamera("perspective",camera_param());
	maWorldBegin();
	maAttributeBegin();	
	//maCoordinateSystem("camera");
	maLightSource("point",light_param(1,2,1));
	maLightSource("point",light_param(10,20,10));
	maLightSource("point",light_param(-1,-2,-1));
	maLightSource("point",light_param(-10,-20,-10));
	maLightSource("point",light_param(-1,2,1));
	maLightSource("point",light_param(-10,20,10));
	maLightSource("point",light_param(-1,-2,1));
	maLightSource("point",light_param(-10,-20,10));
	maLightSource("point",light_param(1,-2,1));
	maLightSource("point",light_param(10,-20,10));
	maLightSource("point",light_param(1,-2,-1));
	maLightSource("point",light_param(10,-20,-10));
	maAttributeEnd();
	maRotate(3.14f/4.f,0,0,1);

	maMaterial("matte",material_param());
	maShape("trianglemesh",shape_param());
	
	maWorldEnd();
	maCleanUp();

	//maFrameBegin();
	//maWorldBegin();
	//maLookAt(0, .2, .2  ,  -.02, .1, 0 , 0, 1, 0);
	//maIdentity();
	//maWorldEnd();
	//maFrameEnd();
}


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
	return light_p;
}
ParamSet shape_param()
{
	//ParamSet shape_p;

	//return shape_p;
	return parseObjTriangleMesh("bunny_1k.obj");
}
ParamSet material_param()
{
	ParamSet material_p;


	return material_p;
}
