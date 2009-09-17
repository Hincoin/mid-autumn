#include "SceneParser.hpp"

#include <cstdlib>
#include <cstdio>

#include "Vector.hpp"
#include "Point.hpp"
namespace ma{

ParamSet parseObjTriangleMesh(const std::string& file_name)
{
	// read it once, get counts
	ParamSet p;
	typedef Point<vector3f> point_t;

	FILE *file = fopen(file_name.c_str(),"r");
	assert (file != NULL);
	int vcount = 0; int fcount = 0;
	while (1) {
		int c = fgetc(file);
		if (c == EOF) { break;
		} else if (c == 'v') { 
			assert(fcount == 0); float v0,v1,v2;
			fscanf (file,"%f %f %f",&v0,&v1,&v2);
			vcount++; 
		} else if (c == 'f') {
			int f0,f1,f2;
			fscanf (file,"%d %d %d",&f0,&f1,&f2);
			fcount++; 
		} // otherwise, must be whitespace

	}
	printf ("verts %d faces %d\n", vcount,fcount);
	fclose(file);

	// make arrays
	std::vector<point_t> verts(vcount);
	std::vector<unsigned> vi(fcount*3);
 
	//Group *answer = new Group(fcount);

	// read it again, save it
	file = fopen(file_name.c_str(),"r");
	assert (file != NULL);
	int new_vcount = 0; int new_fcount = 0;
	while (1) {
		int c = fgetc(file);
		if (c == EOF) { break;
		} else if (c == 'v') { 
			assert(new_fcount == 0); float v0,v1,v2;
			fscanf (file,"%f %f %f",&v0,&v1,&v2);
			verts[new_vcount] = point_t(v0*5,v1*5,v2*5);
			new_vcount++; 
		} else if (c == 'f') {
			assert (vcount == new_vcount);
			int f0,f1,f2;
			fscanf (file,"%d %d %d",&f0,&f1,&f2);
			// indexed starting at 1...
			assert (f0 > 0 && f0 <= vcount);
			assert (f1 > 0 && f1 <= vcount);
			assert (f2 > 0 && f2 <= vcount);
			vi[new_fcount*3]=f0-1;
			vi[new_fcount*3+1]=f1-1;
			vi[new_fcount*3+2]=f2-1;

			// ++++++++++++++++++++++++++++++++++++++++++++++++++++
			// CALLING ASSIGNMENT 2 CODE
			// create a new triangle object and return it
			//Triangle *t = new Triangle(verts[f0-1],verts[f1-1],verts[f2-1],
			//	current_object_color);
			// ++++++++++++++++++++++++++++++++++++++++++++++++++++

			//answer->addObject(new_fcount,t);
			new_fcount++; 
		} // otherwise, must be whitespace
	}
	//delete [] verts;
	assert (fcount == new_fcount);
	assert (vcount == new_vcount);
	fclose(file);

	p.add("indices",vi);
	p.add("P",verts);
	return p;

}
}