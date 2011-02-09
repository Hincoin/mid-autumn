#include "display_func.h"

//shape_kernels --include intersect code
//material_kernels
//light_kernels

//integrator_kernel
//accelerator_kernel

//data
//light param data
//material param data
//shape param data
//texture param data

//integrator param data
//accelerator param data
int main(int argc, char* argv[])
{
	init_display(argc, argv, "clpbr");
	display();
	return 0;
}
