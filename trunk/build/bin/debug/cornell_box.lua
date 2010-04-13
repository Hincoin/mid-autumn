
local function light_param(posx,posy,posz)
	maBeginParam()
	maAddPointParam("from",{posx,posy,posz});
	maAddSpectrumParam("I",{20000});
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
	create_triangle_mesh(floor_idx,floor_points)
	-----------------------------------------------
	--local ceil_idx={0,1,2,2,3,0};
	local ceil_idx = {0,3,2,2,1,0}
	local ceil_points = {{556,548.8,0},{556,548.8,559.2},{0,548.8,559.2},{0,548.8,0},}
	create_triangle_mesh(ceil_idx,ceil_points)
	---------------------------------------------
	local back_idx={0,1,2,2,3,0}
	local back_points = {{549.6,0,559,2},{0,0,559.2},{0,548.8,559.2},{556.0,548.8,559.2}}
	create_triangle_mesh(back_idx,back_points)
	--------------------------------------------
	local right_idx = {0,1,2,2,3,0}
	local right_points = {{0,0,559.2},{0,0,0},{0,548.8,0},{0,548.8,559.2}}
	create_triangle_mesh(right_idx,right_points)
	-----------------------------------------------
	local left_idx ={0,1,2,2,3,0}
	local left_points = {{552.8,0,0},{549.6,0,559.2},{556.0,548.8,559.2},{556.0,548.8,0}}
	create_triangle_mesh(left_idx,left_points)
	-----------------------------------------------
	
end
local function camera_param()
	local params = {
	["hither"]=1e-3,
	["yon"] = 1e30,
	["shutteropen"] = 0,
	["shutterclose"]=1,
	["lensradius"]= 0,
	["focaldistance"]=1e30,
	["frameaspectratio"]=4/3,
	}
	maCamera("perspective",params);
end
local function material_param()
	maMaterial("matte",{})
end

function main()

	maInit();
	maIdentity();
	--eye,center,updir
	maLookAt(278,273,-800,278,273,559,0,1,0);
	maSampler("lowdiscrepancy",{});
	camera_param();
	maAccelerator("kdtree",{});
	maSurfaceIntegrator("whitted",{});
	maWorldBegin();
	maAttributeBegin();
	light_param(343,548.,227);
	light_param(343,548.,332);
	light_param(213,548.,332);
	light_param(213,548.,227);
	maAttributeEnd();
	--maRotate(3.14/4,1,0,0);
	material_param();
	shape_param();
	maWorldEnd();
	maCleanUp();
end

