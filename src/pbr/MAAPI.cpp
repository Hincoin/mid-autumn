#define CORE_SOURCE


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
#include "KdTree.hpp"

#include <string>
#include <vector>
using std::string;
using std::vector;
using namespace ma;

typedef transform3f transform_t;

struct RenderOptions{
	RenderOptions();
	scene_ptr makeScene()const;
	~RenderOptions(){lights.clear();primitives.clear();}
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
	mutable vector<light_ptr> lights;
	vector <primitive_ref_t> primitives;
	//AssocVector<string,vector<SharedPrimitive> > instances;
	//vector<SharedPrimitive> *current_instance;
};
RenderOptions::RenderOptions()
:filter_name("mitchell"),film_name("image"),
sampler_name("bestcandidate"),accelerator_name("kdtree"),
surface_integrator_name("directlighting"),camera_name("perspective")
{
	//default setup
	//current_instance=0;
}

struct GraphicsState{
	GraphicsState():material("matte"),reverse_orientation(false){;}
	//AssocVector<string,add_shared_ptr<Texture<float> >::type> float_textures;
	//AssocVector<string,add_shared_ptr<Texture<Spectrumf> >::type> spectrum_textures;
	ParamSet material_params;
	string material;
	ParamSet area_light_params;
	string area_light;
	bool reverse_orientation;
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


COREDLL void maInit()
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
	graphics_state = GraphicsState();
}
COREDLL void maCleanUp(){
	if (current_state == STATE_UNINITIALIZED)
		report_error("cleanUp called without Init().");
	else if(current_state == STATE_WORLD_BLOCK)
		report_error("cleanUp called while inside world block.");
	current_state = STATE_UNINITIALIZED;
	delete render_options;
	render_options = 0;
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
		 render_options->filter_params = params;
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
		 render_options->sampler_name = name;
		 render_options->sampler_params = params;
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
	 render_options->lights.push_back(create_light<light_t>(current_transform,params));
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
	shape_t* shape = create_shape<shape_t>(current_transform,graphics_state.reverse_orientation,
		params);
	if (!shape) return;
	//params.ReportUnused();
	//// Initialize area light for shape
	//AreaLight *area = NULL;
	//if (graphicsState.areaLight != "")
	//	area = MakeAreaLight(graphicsState.areaLight,
	//	curTransform, graphicsState.areaLightParams, shape);
	//// Initialize material for shape
	//TextureParams mp(params,
	//	graphicsState.materialParams,
	//	graphicsState.floatTextures,
	//	graphicsState.spectrumTextures);
	//Reference<Texture<float> > bump = NULL;
	//Reference<Material> mtl =
	//	MakeMaterial(graphicsState.material,
	//	curTransform, mp);
	material_t* mtl = create_material<material_t>(current_transform,graphics_state.material_params);
	//if (!mtl)
	//	mtl = MakeMaterial("matte", curTransform, mp);
	//if (!mtl)
	//	Severe("Unable to create \"matte\" material?!");
	// Create primitive and add to scene or current instance
	primitive_ref_t prim (  new geometry_primitive_t(const_shape_ref_t(shape),const_material_ref_t(mtl)));
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
	scene_ptr scene = render_options->makeScene();
	if (scene)
	{
		scene->render();
	}
	delete scene;
	render_options->primitives.clear();
	render_options->lights.clear();
	current_state = STATE_OPTIONS_BLOCK;
	current_transform = transform_t();
	named_coordinate_sys.clear();
 }
 COREDLL void maFrameEnd(){;}



 scene_ptr RenderOptions::makeScene() const {

	 // Create scene objects from API settings
	 //Filter *filter = MakeFilter(FilterName, FilterParams);
	 //Film *film = MakeFilm(FilmName, FilmParams, filter);
	 //Camera *camera = MakeCamera(CameraName, CameraParams,
		// WorldToCamera, film);
	 //Sampler *sampler = MakeSampler(SamplerName, SamplerParams, film);
	 //SurfaceIntegrator *surfaceIntegrator = MakeSurfaceIntegrator(SurfIntegratorName,
		// SurfIntegratorParams);
	 //VolumeIntegrator *volumeIntegrator = MakeVolumeIntegrator(VolIntegratorName,
		// VolIntegratorParams);
	 //Primitive *accelerator = MakeAccelerator(AcceleratorName,
		// primitives, AcceleratorParams);
	 //if (!accelerator) {
		// ParamSet ps;
		// accelerator = MakeAccelerator("kdtree", primitives, ps);
	 //}
	 //if (!accelerator)
		// Severe("Unable to find \"kdtree\" accelerator");
	 //// Initialize _volumeRegion_ from volume region(s)
	 //VolumeRegion *volumeRegion;
	 //if (volumeRegions.size() == 0)
		// volumeRegion = NULL;
	 //else if (volumeRegions.size() == 1)
		// volumeRegion = volumeRegions[0];
	 //else
		// volumeRegion = new AggregateVolume(volumeRegions);
	 //// Make sure all plugins initialized properly
	 //if (!camera || !sampler || !film || !accelerator ||
		// !filter || !surfaceIntegrator || !volumeIntegrator) {
		//	 Severe("Unable to create scene due "
		//		 "to missing plug-ins");
		//	 return NULL;
	 //}
	 //Scene *ret = new Scene(camera,
		// surfaceIntegrator, volumeIntegrator,
		// sampler, accelerator, lights, volumeRegion);
	 //// Erase primitives, lights, and volume regions from _RenderOptions_
	 //primitives.erase(primitives.begin(),
		// primitives.end());
	 //lights.erase(lights.begin(),
		// lights.end());
	 //volumeRegions.erase(volumeRegions.begin(),
		// volumeRegions.end());
	 //return ret;


	 //////////////////////////////////////////////////////////////////////////
	 //test
	 typedef ImageFilm<film_config<basic_config_t> > image_film_t;
	 typedef PointLight<light_config<basic_config_t> > light_t;
	 typedef PerspectiveCamera<camera_config<basic_config_t> > camera_t;
	 typedef Sample<sample_config<basic_config_t> > sample_t;
	 typedef LDSampler<sampler_config<basic_config_t> > sampler_t;
	 typedef MAPrimitive<primitive_interface_config<basic_config_t> > primitive_t;
	 typedef WhittedIntegrator<surface_integrator_config<basic_config_t> > surface_integrator_t;
	 typedef Scene<scene_config<basic_config_t> > scene_t;
	 typedef MitchellFilter<filter_config<basic_config_t> > filter_t;
	 filter_t* filter = new filter_t(0.33f,0.33f,2,2);

	 //////////////////////////////////////////////////////////////////////////
	//create film
	 std::string filename =  "pbrt.tga";
	 bool premultiplyAlpha =  true;
#ifdef NDEBUG
	 int xres = 800;
	 int yres = 600;
#else
	 int xres = 80;
	 int yres = 60;
#endif

	 float crop[4] = { 0, 1, 0, 1 };

	 int write_frequency = -1;
	 image_film_t* film = new image_film_t(xres,yres,filter,crop,filename,premultiplyAlpha,write_frequency);
	 //////////////////////////////////////////////////////////////////////////
	 // Extract common camera parameters
	 float hither = 1e-3f;
	 float yon =  1e30f;
	 float shutteropen =  0.f;
	 float shutterclose = 1.f;
	 float lensradius =  0.f;
	 float focaldistance =  1e30f;
	 float frame = xres/float(yres);
	 float screen[4];
	 if (frame > 1.f) {
		 screen[0] = -frame;
		 screen[1] =  frame;
		 screen[2] = -1.f;
		 screen[3] =  1.f;
	 }
	 else {
		 screen[0] = -1.f;
		 screen[1] =  1.f;
		 screen[2] = -1.f / frame;
		 screen[3] =  1.f / frame;
	 }
	 float fov =  90;
	 camera_t* camera = new camera_t(world_to_camera,screen, hither, yon,
		 shutteropen, shutterclose, lensradius, focaldistance,
		 fov, film);

	 ////////////////////////////////////////////////////////////////////////////
	 // Initialize common sampler parameters
	 int xstart, xend, ystart, yend;
	 film->getSampleExtent(xstart, xend, ystart, yend);
	 int nsamp = 4;
	 sampler_t* sampler = new sampler_t(xstart, xend, ystart, yend, nsamp);

	 //////////////////////////////////////////////////////////////////////////
	 int isectCost = 80;
	 int travCost = 1;
	 float emptyBonus =  0.5f;
	 int maxPrims = 1;
	 int maxDepth = -1;
	 primitive_t* accel = new KdTreeAccel(primitives, isectCost, travCost,
		 emptyBonus, maxPrims, maxDepth);
	 
	 ////////////////////////////////////////////////////////////////////////////
	 surface_integrator_t* surface_integrator = new surface_integrator_t(5);

	 //////////////////////////////////////////////////////////////////////////
	 scene_ptr ret = new scene_t(camera,
		surface_integrator, NULL,
		 sampler, accel, lights, NULL);
	 return ret;
 }
