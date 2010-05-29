
local function light_param(posx,posy,posz)
	maBeginParam()
	maAddPointParam("from",{posx,posy,posz});
	maAddSpectrumParam("I",{1000});
	maLightSource("point",{})
	maEndParam()
end

local function create_triangle_mesh(indices,points,uvs)
	maBeginParam();
	if indices then
		maAddUnsignedArrayParam("indices",indices);
	end
	if points then
		maAddPointArrayParam("P",points)
	end
	--todo uvs
	
	maShape("trianglemesh",{})
	maEndParam()
end
--simplfied 
local function material_param(name,texname)
	if texname  then
		if name == "matte" then
			maMaterial(name,{["Kd"]=texname})
		elseif name == "mirror" then
			maMaterial(name,{["Kr"] = texname})
		elseif name == "glass" then
			maMaterial(name,{["Kt"] = texname})
		end
	else
		maMaterial(name,{})
	end
end


local function shape_param()
--[[	local indices={0,1,2}
	local P = {{-1,-1,0}, {1,1,0}, {0,1,0}}
	maBeginParam()
	maAddUnsignedArrayParam("indices",indices)
	maAddPointArrayParam("P",P);
	maShape("trianglemesh",{})
	maEndParam()
	]]
	----------------------------------------
	local floor_idx={0,1,2,2,3,0}
	local floor_points = {{552.8,0,0},{0,0,0},{0,0,559.2},{549.6,0,559.2}};
	material_param("matte","test_checker");
	create_triangle_mesh(floor_idx,floor_points)
	-----------------------------------------------
	local ceil_idx={0,1,2,2,3,0};
	local ceil_points = {{556,548.8,0},{556,548.8,559.2},{0,548.8,559.2},{0,548.8,0},}

	material_param("matte","test_white")
	create_triangle_mesh(ceil_idx,ceil_points)
	---------------------------------------------
	local back_idx={0,1,2,2,3,0}
	local back_points = {{549.6,0,559.2},{0,0,559.2},{0,548.8,559.2},{556.0,548.8,559.2}}
	create_triangle_mesh(back_idx,back_points)
	--------------------------------------------
	local right_idx = {0,1,2,2,3,0}
	local right_points = {{0,0,559.2},{0,0,0},{0,548.8,0},{0,548.8,559.2}}
	material_param("matte","test_green")
	create_triangle_mesh(right_idx,right_points)
	-----------------------------------------------
	local left_idx ={0,1,2,2,3,0}
	local left_points = {{552.8,0,0},{549.6,0,559.2},{556.0,548.8,559.2},{556.0,548.8,0}}
	material_param("matte","test_red")
	create_triangle_mesh(left_idx,left_points)
	-----------------------------------------------
	--short block
	local short_block_idx={
		0,1,2, 2,3,0,
		4,5,6, 6,7,4,
		8,9,10,10,11,8,
		12,13,14,14,15,12,
		16,17,18,18,19,16,
	};
	local short_block_points={
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
material_param("glass","test_gray");
create_triangle_mesh(short_block_idx,short_block_points)

--------------------------------------
	local tall_block_idx={
		0,1,2, 2,3,0,
		4,5,6, 6,7,4,
		8,9,10,10,11,8,
		12,13,14,14,15,12,
		16,17,18,18,19,16,
	}
	local tall_block_points={
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
}
material_param("mirror","test_white")
create_triangle_mesh(tall_block_idx,tall_block_points)
end
local function camera_param()
	local params = {
	["hither"]=1e-3,
	["yon"] = 1e30,
	["shutteropen"] = 0,
	["shutterclose"]=1,
	["lensradius"]= 0,
	["focaldistance"]= 1200,
	["frameaspectratio"]=4/3,
	["fov"] = 40,
	}
	maCamera("perspective",params);
end
function main(frame,id,wcropx_min,wcropx_max,wcropy_min,wcropy_max)

	maInit();
	maIdentity();
	--eye,center,updir
	maLookAt(278,273,-800,278,273,400,0,1,0);
	local xres,yres = 320,240
	--local xres,yres = 640,480
	
	maBeginParam();
	maAddFloatArrayParam("cropwindow",{wcropx_min or 0,wcropx_max or 1,wcropy_min or 0,wcropy_max or 1});
	maFilm("image",{
	["filename"] = "cornell_box_" .. tostring(frame) .. "_" .. tostring(id) .. ".tga",	
	["xresolution"]=xres,
	["yresolution"]=yres
	});
	maEndParam()

	maSampler("lowdiscrepancy",{["pixelsamples"]=1});
	camera_param();
	maAccelerator("kdtree",{});
	maSurfaceIntegrator("whitted",{});
-----------------------------------------------------------------------------------------
	maWorldBegin();

	local const_texture = function(name,color)
	maBeginParam()
	maAddSpectrumParam("value",color)
	maTexture(name,"color","constant",{})
	maEndParam()
	end
	local checker_spectrum_texture = function(name,tex1,tex2)
		maBeginParam()
		--maTexture(name,"color","checker",{["mapping"]="planar",["tex1"]=tex1,["tex2"] = tex2});
		maTexture(name,"color","checker",{
			["mapping"]="uv",
			["uscale"]=10,
			["vscale"]=10,
			["tex1"]=tex1,["tex2"] = tex2});
		maEndParam()
	end
	const_texture("test_gray",{0.5,0.5,0.5})
	const_texture("test_red",{1,0,0})
	const_texture("test_green",{0,1,0})
	const_texture("test_white",{1,1,1});
	checker_spectrum_texture("test_checker","test_red","test_white")
	maAttributeBegin()
	maScale(100,100,100)
	maTexture("test_constant_color","float","constant",{["value"]=2})
	maAttributeEnd()
	

	
	maAttributeBegin();
	--simulate area light 
	local add_i = 20 
	local add_j= 20 
	local light_cnt = 0;
	for i=213,343,add_i do
		for j=227,332,add_j do
			light_cnt = light_cnt + 1
			light_param(i,548.*0.9,j);
		end
	end
	print("light count: ",light_cnt)
	--light_param(343,548.*0.9,227);
	--light_param(343,548.*0.9,332);
	--light_param(213,548.*0.9,332);
	--light_param(213,548.*0.9,227);
	maAttributeEnd();
	--maRotate(3.14/4,1,0,0);
	material_param("matte");
	shape_param();
	maWorldEnd();
	maCleanUp();
end
--start
function startup(is_client)
	maInit(1);--lazy
	maIdentity();
	--setup options
	--eye,center,updir
	maLookAt(278,273,-800,278,273,400,0,1,0);
	--local xres,yres = 160/2,120/2--320,240
	--debug configuration
	local xres,yres = 80,60--640,480
	local spp = 1--128 
	maBeginParam();
	--maAddFloatArrayParam("cropwindow",{wcropx_min or 0,wcropx_max or 1,wcropy_min or 0,wcropy_max or 1});
	local image_type= "image"
	if is_client then image_type = "netimage" end
	maFilm(image_type,{
	["filename"] = "cornell_box.tga",	
	["xresolution"]=xres,
	["yresolution"]=yres
	});
	maEndParam()

	maSampler("lowdiscrepancy",{["pixelsamples"]=spp});
	camera_param();
	maAccelerator("kdtree",{});
	--maSurfaceIntegrator("whitted",{});
	maSurfaceIntegrator("bidirectional",{});

end
--exit
function cleanup()
	maCleanUp();
end
--frames
function frame(n)
	maWorldBegin();

	local const_texture = function(name,color)
	maBeginParam()
	maAddSpectrumParam("value",color)
	maTexture(name,"color","constant",{})
	maEndParam()
	end
	local checker_spectrum_texture = function(name,tex1,tex2)
		maBeginParam()
		--maTexture(name,"color","checker",{["mapping"]="planar",["tex1"]=tex1,["tex2"] = tex2});
		maTexture(name,"color","checker",{
			["mapping"]="uv",
			["uscale"]=10,
			["vscale"]=10,
			["tex1"]=tex1,["tex2"] = tex2});
		maEndParam()
	end
	const_texture("test_gray",{0.5,0.5,0.5})
	const_texture("test_red",{1,0,0})
	const_texture("test_green",{0,1,0})
	const_texture("test_white",{1,1,1});
	checker_spectrum_texture("test_checker","test_red","test_white")
	maAttributeBegin()
	maScale(100,100,100)
	maTexture("test_constant_color","float","constant",{["value"]=2})
	maAttributeEnd()
	

	
	maAttributeBegin();
	--simulate area light 
	local add_i = 10 
	local add_j= 10 
	local light_cnt = 0;
	for i=213,343,add_i do
		for j=227,332,add_j do
			light_cnt = light_cnt + 1
			light_param(i,548.*0.9,j);
		end
	end
	print("light count: ",light_cnt)
	--light_param(343,548.*0.9,227);
	--light_param(343,548.*0.9,332);
	--light_param(213,548.*0.9,332);
	--light_param(213,548.*0.9,227);
	maAttributeEnd();
	--maRotate(3.14/4,1,0,0);
	material_param("matte");
	shape_param();
	maWorldEnd();
end

