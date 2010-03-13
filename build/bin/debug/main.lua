

function main()

	maInit();
	local pixel_filter={
	["name"]="mitchell",
	["sig"]=1,
	["aaa"]={
	["bbb"]=1,
	["ccc"]=2,
	["ddd"]={
	["eee"] = 3,
	["fff"] = "aaaaa",
	}
	}
	};
	maPixelFilter("mitchell",pixel_filter);
	maCleanUp();
end

