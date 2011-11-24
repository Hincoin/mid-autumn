#include <cassert>
#include "any.h"
#include "test.h"
#include "parameter_map.h"
#include "opencl_device.h"

void any_test()
{
	Any a;
	assert(a.empty());
	assert(a.type_id() == Any::void_type_id);

	Any b;
	int inta=100;
	a = inta;
	assert(*any_cast<int>(&a) == 100);
	b = a;
	assert(*any_cast<int>(&b) == 100);

	assert(any_cast<unsigned>(&b) == NULL);

	int *bptr = new int(1000);
	Any bptr_any = bptr;

	int *aptr=*any_cast<int*>(&bptr_any);
	assert(*aptr == 1000);
}

void parameter_test()
{
	ParameterMap m;
	int a = 100;
	m.insert("help",a);
	int b;
	assert(m.get("help",&b));
	assert(b == a);
	assert(!m.get("help1",&b));
}

void opencl_compile_test()
{
	OpenCLDevice device;
	device.SetKernelFile("rendering_kernel.cl");
//	device.Run();
}