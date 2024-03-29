#ifndef CORE_SOURCE
#define CORE_SOURCE
#endif

#include "MAAPI.hpp"



#include "Scene.hpp"
#include "Dynload.hpp"
#include "ErrorReporter.hpp"
#include "Vector.hpp"
#include "Transform.hpp"
#include "Light.hpp"
#include "Color.hpp"
#include "DefaultConfigurations.hpp"
//test
#include "TextureParam.hpp"
#include "modules.hpp"

#include <string>
#include <vector>
enum RENDER_MODE{IMMEDIATE=0,LAZY};
using std::string;
using std::vector;
using namespace ma;
struct RenderOptions{
	RenderOptions();
	scene_ptr makeScene();
	~RenderOptions(){clear();}
	//
	string filter_name;
	ParamSet filter_params;
	string film_name;
	ParamSet film_params;
	string sampler_name;
	ParamSet sampler_params;
	string accelerator_name;
	ParamSet accelerator_params;
	string surface_integrator_name,volume_integrator_name;
	ParamSet surface_integrator_params,volume_integrator_params;
	string camera_name;
	ParamSet camera_params;
	transform_t world_to_camera;
	vector<light_ptr> lights;
	vector <primitive_ref_t> primitives;
	RENDER_MODE mode;
	scene_ptr scene;
	//AssocVector<string,vector<SharedPrimitive> > instances;
	//vector<SharedPrimitive> *current_instance;
	//todo not clear everyting when world end 
	void clear()
	{
		filter_params.clear();
		film_params.clear();
		sampler_params.clear();
		accelerator_params.clear();
		surface_integrator_params.clear();
		volume_integrator_params.clear();
		camera_params.clear();
		lights.clear();
		primitives.clear();
		delete_ptr(scene);
	}
	void render()
	{
		//process other options here
		//preprocess or save accelerate data or render
		if (mode == LAZY)
		{
			makeScene();
			return ;
		}
		if (scene)
			scene->render();
		else
		{
			makeScene();
			scene->render();
		}

	}
};
RenderOptions::RenderOptions()
:filter_name("mitchell"),film_name("image"),
sampler_name("bestcandidate"),accelerator_name("kdtree"),
surface_integrator_name("directlighting"),camera_name("perspective")
,scene(0){
	mode = IMMEDIATE;
	//default setup
	//current_instance=0;
}


struct GraphicsState{
	GraphicsState():material("matte"),reverse_orientation(false){;}
	std::map<string, shared_float_texture_t > float_textures_;
	std::map<string, shared_spectrum_texture_t> spectrum_textures_;
	ParamSet material_params;
	string material;
	ParamSet area_light_params;
	string area_light;
	bool reverse_orientation;
	void clear()
	{
		float_textures_.clear();
		spectrum_textures_.clear();
		material_params.clear();
		area_light_params.clear();
	}
};

enum{STATE_UNINITIALIZED=0,STATE_OPTIONS_BLOCK,STATE_WORLD_BLOCK};
static int current_state = STATE_UNINITIALIZED;
static transform_t current_transform;
static AssocVector<string,transform_t> named_coordinate_sys;
static RenderOptions *render_options = 0;
static GraphicsState graphics_state;
static vector<GraphicsState> graphics_state_stack;
static vector<transform_t> transform_stack;
//state verifiers
void inline verify_initialized(const char* func)
{
	if (current_state == STATE_UNINITIALIZED)
	{
		report_error("Init function must be calling \"%s()\".",func);
		return;
	}
}
void inline verify_options(const char* func)
{
	verify_initialized(func);
	if (current_state == STATE_WORLD_BLOCK)
	{
		report_error("Options cannot be set inside world block; \"%s\"",func);
	}
}
void inline verify_world(const char* func)
{
	if (current_state == STATE_OPTIONS_BLOCK)
	{
		report_error("Scene description must be inside world block.\"%s\"",func);
	}
}


COREDLL void maInit(int mode)
{
	// System-wide initialization
	// Make sure floating point unit's rounding stuff is set
	// as is expected by the fast FP-conversion routines.  In particular,
	// we want double precision on Linux, not extended precision!
#ifdef FAST_INT
#if defined(__linux__) && defined(__i386__)
	int cword = _FPU_MASK_DM | _FPU_MASK_ZM | _FPU_MASK_OM | _FPU_MASK_PM |
		_FPU_MASK_UM | _FPU_MASK_IM | _FPU_DOUBLE | _FPU_RC_NEAREST;
	_FPU_SETCW(cword);
#endif
#if defined(WIN32)
	_control87(_PC_53, MCW_PC);
#endif
#endif // FAST_INT
	// API Initialization
	if (current_state != STATE_UNINITIALIZED)
		report_error("Init() has already been called.");
	current_state = STATE_OPTIONS_BLOCK;
	render_options = new RenderOptions;
	render_options->mode = (RENDER_MODE)mode;
	graphics_state = GraphicsState();
	//
	//
	//
	register_all_creators();
}
COREDLL void maCleanUp(){
	if (current_state == STATE_UNINITIALIZED)
		report_error("cleanUp called without Init().");
	else if(current_state == STATE_WORLD_BLOCK)
		report_error("cleanUp called while inside world block.");
	current_state = STATE_UNINITIALIZED;
	delete render_options;
	render_options = 0;
	//clear memory pool
	release_memory();
}



// API Function Declarations
COREDLL void maIdentity(){
	verify_initialized("Identity");
	current_transform.identity();
}
COREDLL void maTranslate(float dx, float dy, float dz){
	verify_initialized("Translate");
	current_transform.translate(vector3f(dx,dy,dz));
}
 COREDLL void maRotate(float angle_in_radian,
							 float ax,
							 float ay,
							 float az)
 {
	 verify_initialized("Rotate");
	 current_transform.rotate(angle_in_radian,vector3f(ax,ay,az));
 }
 COREDLL void maScale(float sx,
							float sy,
							float sz)
 {
	 verify_initialized("Scale");
	current_transform.scale(vector3f(sx,sy,sz));
 }
 COREDLL void maLookAt(float ex,
							 float ey,
							 float ez,
							 float lx,
							 float ly,
							 float lz,
							 float ux,
							 float uy,
							 float uz)
 {
	 verify_initialized("LookAt");
	 current_transform*=transform_op::lookat(ex,ey,ez,lx,ly,lz,ux,uy,uz);
 }

COREDLL void maConcatTransform(float transform[4][4])
{
	verify_initialized("ConcatTransform");
	transform_t t(transform);
	current_transform*=t;
}

COREDLL void maTransform(float transform[4][4])
{
	verify_initialized("transform");
	transform_t t(transform);
	current_transform.swap(t);
}
 COREDLL void maCoordinateSystem(const std::string &n)
 {
	 named_coordinate_sys.insert(make_pair(n,current_transform));
 }
 COREDLL void maCoordSysTransform(const std::string &name){
	 verify_initialized("CoordSysTransform");
	 if (named_coordinate_sys.find(name) != named_coordinate_sys.end())
	 {
		 current_transform = named_coordinate_sys[name];
	 }

 }
 COREDLL void maPixelFilter(const std::string &name, const ParamSet &params){
	 verify_options("PixelFilter");
	 render_options->filter_name = name;
	 render_options->filter_params = params;
 }
 COREDLL void maFilm(const std::string &type,
	 const ParamSet &params){
		 verify_options("Film");
		 render_options->film_name = type;
		 render_options->film_params = params;
 }
 COREDLL void maSampler(const std::string &name,
	 const ParamSet &params){
		 verify_options("Sampler");
		 render_options->sampler_name = name;
		 render_options->sampler_params = params;
 }
 COREDLL void maAccelerator(const std::string &name,
	 const ParamSet &params){
		 verify_options("Accelerator");
		 render_options->accelerator_name= name;
		 render_options->accelerator_params= params;
 }

COREDLL void maSurfaceIntegrator(const std::string &name,
						 const ParamSet &params){
		verify_options("SurfaceIntegrator");
		render_options->surface_integrator_name = name;
		render_options->surface_integrator_params = params;
}

COREDLL void maVolumeIntegrator(const std::string &name,
						const ParamSet &params){
		verify_options("VolumeIntegrator");
		render_options->volume_integrator_name = name;
		render_options->volume_integrator_params = params;
}
COREDLL void maCamera(const std::string &name, const ParamSet &cameraParams){
	verify_options("Camera");
		render_options->camera_name = name;
	render_options->camera_params = cameraParams;
	render_options->world_to_camera = current_transform;
	named_coordinate_sys["camera"] = current_transform.inverse();
}
COREDLL void maSearchPath(const std::string &path){
	//todo
}
COREDLL void maFrameBegin(const std::string&,const ParamSet& frameParams){
	//todo
}
COREDLL void maWorldBegin(){
	if(render_options)
	{
		render_options->primitives.clear();
		render_options->lights.clear();
	}
	verify_options("WorldBegin");
	current_state = STATE_WORLD_BLOCK;
	current_transform = transform_t().identity();
	named_coordinate_sys["world"] = current_transform;
}
COREDLL void maAttributeBegin(){
	verify_world("AttributeBegin");
	graphics_state_stack.push_back(graphics_state);
	transform_stack.push_back(current_transform);
}
COREDLL void maAttributeEnd(){
	verify_world("AttributeEnd");
	if (graphics_state_stack.empty() || transform_stack.empty())
	{
		report_error("Unmatched AttributeEnd() encountered.");
		return;
	}
	graphics_state = graphics_state_stack.back();
	current_transform = transform_stack.back();
	graphics_state_stack.pop_back();
	transform_stack.pop_back();
}
COREDLL void maTransformBegin(){
	verify_world("TransformBegin");
	transform_stack.push_back(current_transform);
}
COREDLL void maTransformEnd(){
	if (transform_stack.empty())
	{
		report_error("Unmatched TransformEnd encountered.");
		return;
	}
	current_transform = transform_stack.back();
	transform_stack.pop_back();

}
 COREDLL void maTexture(const std::string &name, const std::string &type,
	 const std::string &texname, const ParamSet &params){
		 //todo
		 verify_world("Texture");
		 TextureParam tp(params,params,graphics_state.float_textures_,graphics_state.spectrum_textures_);
		 if(type == "float")
		 {
			 if(graphics_state.float_textures_.find(name) != graphics_state.float_textures_.end())
				 report_warning("Texture \"%s\" being redefined.",name.c_str());
			 shared_float_texture_t ft = make_float_texture(texname,current_transform,tp);
			 assert(ft);
			 if (ft) graphics_state.float_textures_[name] = ft;
		 }
		 else if(type == "color")
		 {
			 if(graphics_state.spectrum_textures_.find(name) != graphics_state.spectrum_textures_.end())
				 report_warning("Texture \"%s\" being redefined",name.c_str());
			 shared_spectrum_texture_t st = make_spectrum_texture(texname,
					 current_transform,tp);
			 assert(st);
			 printf("add color texture %s,%d \n",name.c_str(),st.which_type());
			 if(st) graphics_state.spectrum_textures_[name] = st;
		 }
		 else 
			 report_error("Texture type \"%s\" unknow.",type.c_str());
		if(type == "float")
		 {
			 assert(graphics_state.float_textures_[name]);
		 }
		 else if(type == "color")
		 {
			 assert(graphics_state.spectrum_textures_[name]);
		 }
			
 }
 COREDLL void maMaterial(const std::string &name,
	 const ParamSet &params){
		 verify_world("Material");
		 graphics_state.material = name;
		 graphics_state.material_params = params;
 }
 COREDLL void maLightSource(const std::string &name, const ParamSet &params){
	 verify_world("LightSource");
	 //todo
	 render_options->lights.push_back(make_light(name,current_transform,params));
	// render_options->lights.push_back(create_light<light_t>(current_transform,params));
 }
 COREDLL void maAreaLightSource(const std::string &name, const ParamSet &params){
	 verify_world("AreaLightSource");
	 graphics_state.area_light = name;
	 graphics_state.area_light_params = params;
 }
 COREDLL void maShape(const std::string &name, const ParamSet &params){
	verify_world("Shape");
	//todo
	//Reference<Shape> shape = MakeShape(name,
	//	curTransform, graphicsState.reverseOrientation,
	//	params);
	shape_ref_t shape = make_shape(name,current_transform,graphics_state.reverse_orientation,
		params);	
	//shape_t* shape = create_shape<shape_t>(current_transform,graphics_state.reverse_orientation,
	//		params);
	if (!shape) return;
	//params.ReportUnused();
	//// Initialize area light for shape
	//AreaLight *area = NULL;
	//if (graphicsState.areaLight != "")
	//	area = MakeAreaLight(graphicsState.areaLight,
	//	curTransform, graphicsState.areaLightParams, shape);
	//// Initialize material for shape
	//TextureParam mp(params,
	//	graphicsState.materialParams,
	//	graphicsState.floatTextures,
	//	graphicsState.spectrumTextures);
	//Reference<Texture<float> > bump = NULL;
	//Reference<Material> mtl =
	//	MakeMaterial(graphicsState.material,
	//	curTransform, mp);
	//material_t* mtl = create_material<material_t>(current_transform,graphics_state.material_params);
	TextureParam mp(params,graphics_state.material_params,graphics_state.float_textures_,graphics_state.spectrum_textures_);
	material_ref_t mtl = make_material(graphics_state.material,current_transform,mp);
	//if (!mtl)
	//	mtl = MakeMaterial("matte", curTransform, mp);
	//if (!mtl)
	//	Severe("Unable to create \"matte\" material?!");
	// Create primitive and add to scene or current instance
	primitive_ref_t prim (  new geometry_primitive_t((shape),(mtl)));
	//Reference<Primitive> prim =
	//	new GeometricPrimitive(shape, mtl, area);
	//if (renderOptions->currentInstance) {
	//	if (area)
	//		Warning("Area lights not supported "
	//		"with object instancing");
	//	renderOptions->currentInstance->push_back(prim);
	//}
	//else {
	//	renderOptions->primitives.push_back(prim);
	//	if (area != NULL) {
	//		// Add area light for primitive to light vector
	//		renderOptions->lights.push_back(area);
	//	}
	//}
	render_options->primitives.push_back(prim);

 }
 COREDLL void maReverseOrientation(){
	 graphics_state.reverse_orientation = !graphics_state.reverse_orientation;
 }
 COREDLL void maVolume(const std::string &name, const ParamSet &params){
	 verify_world("Volume");
 }
 COREDLL void maObjectBegin(const std::string &name){
	 verify_world("ObjectBegin");
	 maAttributeBegin();
	 //todo
 }
 COREDLL void maObjectEnd(){
	 verify_world("ObjectEnd");
	 maAttributeEnd();
	 //todo
 }
 COREDLL void maObjectInstance(const std::string &name){
	verify_world("ObjectInstance");
	//todo
 }
 COREDLL void maWorldEnd(){
	verify_world("WorldEnd");
	while (!graphics_state_stack.empty() && !transform_stack.empty())
	{
		report_warning("Missing end to AttributeBegin");
		graphics_state_stack.pop_back();
		transform_stack.pop_back();
	}

	if(render_options )
	{
		render_options->render();
	}
	current_state = STATE_OPTIONS_BLOCK;
	current_transform = transform_t();
	named_coordinate_sys.clear();
	graphics_state.clear();
 }
 COREDLL void maFrameEnd(){;}



 scene_ptr RenderOptions::makeScene() 
{

	 typedef Scene<scene_config<basic_config_t> > scene_t;
	filter_ptr filter = make_filter(filter_name,filter_params);
	film_ptr film = make_film(film_name,film_params,filter);
	camera_ptr camera = make_camera(camera_name,camera_params,world_to_camera,film);
	sampler_ptr sampler = make_sampler(sampler_name,sampler_params,film);
	surface_integrator_ptr si = make_surface_integrator(surface_integrator_name,
			surface_integrator_params);
	primitive_ptr accelerator = make_accelerator(accelerator_name,primitives,accelerator_params);
	if(!filter || !film || !camera || !sampler || !si || !accelerator)
	{
		report_error("Cannot create Scene!\n");
		return 0;
	}
	scene = new scene_t(camera,si,NULL,sampler,accelerator,lights,NULL);
	return scene;

 }
namespace ma{
scene_ptr get_renderer()
{
	if(render_options)
	{
		if (!render_options->scene)
			return render_options->makeScene();
		return render_options->scene;
	}
	else return NULL;
}
film_ptr get_film()
{
	scene_ptr s = get_renderer();
	if (s)
		return camera::getFilm(s->camera_);
	return NULL;
}

}
