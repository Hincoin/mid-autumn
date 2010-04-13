
local function light_param(posx,posy,posz)
	maBeginParam()
	maAddPointParam("from",{posx,posy,posz});
	maAddSpectrumParam("I",{200});
	maLightSource("point",{})
	maEndParam()
end
local function shape_param()
	local indices={0,1,2}
	local P = {{-1,-1,0}, {1,1,0}, {0,1,0}}
	maBeginParam()
	maAddUnsignedArrayParam("indices",indices)
	maAddPointArrayParam("P",P);
	maShape("trianglemesh",{})
	maEndParam()
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
	maLookAt(0,0,2,0,0,0,0,1,0);
	maSampler("lowdiscrepancy",{});
	camera_param();
	maAccelerator("kdtree",{});
	maSurfaceIntegrator("whitted",{});
	maWorldBegin();
	maAttributeBegin();
	light_param(10,20,10);
	light_param(10,20,10);
	light_param(-10,-20,-10);
	light_param(-10,-20,10);
	light_param(-10,20,10);
	light_param(-10,20,-10);
	light_param(10,20,-10);
	light_param(10,-20,-10);
	maAttributeEnd();
	maRotate(3.14/4,1,0,0);
	material_param();
	shape_param();
	maWorldEnd();
	maCleanUp();
end

