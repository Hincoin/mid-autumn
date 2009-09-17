#ifndef VIRTUAL_CONTAINER_H
#define VIRTUAL_CONTAINER_H

#include "common.h"
class VShape
{
public:
	virtual void draw() = 0;
	virtual bool intersect(int seed) = 0;
	virtual ~VShape(){}
};

class vsimple_shape0:public VShape{
	vector<void*> stored;
	void draw(){draw_function(stored);}
	bool intersect(int seed){return intersect_function(stored,seed);}
	~vsimple_shape0(){destruct(stored);}
};

class vsimple_shape1:public VShape{
	vector<void*> stored;
	void draw(){draw_function(stored);}
	bool intersect(int seed){return intersect_function(stored,seed);}
	~vsimple_shape1(){destruct(stored);}
};
class vsimple_shape2:public VShape{
	vector<void*> stored;
	void draw(){draw_function(stored);}
	bool intersect(int seed){return intersect_function(stored,seed);}
	~vsimple_shape2(){destruct(stored);}
};
class vsimple_shape3:public VShape{
	vector<void*> stored;
	void draw(){draw_function(stored);}
	bool intersect(int seed){return intersect_function(stored,seed);}
	~vsimple_shape3(){destruct(stored);}
};
class vsimple_shape4:public VShape{
	vector<void*> stored;
	void draw(){draw_function(stored);}
	bool intersect(int seed){return intersect_function(stored,seed);}
	~vsimple_shape4(){destruct(stored);}
};
class vsimple_shape5:public VShape{
	vector<void*> stored;
	void draw(){draw_function(stored);}
	bool intersect(int seed){return intersect_function(stored,seed);}
	~vsimple_shape5(){destruct(stored);}
};
class vsimple_shape6:public VShape{
	vector<void*> stored;
	void draw(){draw_function(stored);}
	bool intersect(int seed){return intersect_function(stored,seed);}
	~vsimple_shape6(){destruct(stored);}
};
class vsimple_shape7:public VShape{
	vector<void*> stored;
	void draw(){draw_function(stored);}
	bool intersect(int seed){return intersect_function(stored,seed);}
	~vsimple_shape7(){destruct(stored);}
};
class vsimple_shape8:public VShape{
	vector<void*> stored;
	void draw(){draw_function(stored);}
	bool intersect(int seed){return intersect_function(stored,seed);}
	~vsimple_shape8(){destruct(stored);}
};
class vsimple_shape9:public VShape{
	vector<void*> stored;
	void draw(){draw_function(stored);}
	bool intersect(int seed){return intersect_function(stored,seed);}
	~vsimple_shape9(){destruct(stored);}
};
#endif