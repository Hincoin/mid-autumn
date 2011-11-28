#include <fstream>
#include "opencl_device.h"


OpenCLDevice::OpenCLDevice(cl_device_type default_device_type)
{
	context_ = NULL;
	devices_ = NULL;
	command_queue_ = NULL;
	program_ = NULL;
	kernel_ = NULL;

	cl_int status = 0;
	size_t device_list_size;

	cl_uint number_platforms;
	cl_platform_id platform = NULL;
	status = clGetPlatformIDs(0, NULL, &number_platforms);
	if (status != CL_SUCCESS)
	{
		return;
	}
	if(number_platforms > 0)
	{
		cl_platform_id* platforms = new cl_platform_id[number_platforms];
		status = clGetPlatformIDs(number_platforms,platforms, NULL);
		if(status != CL_SUCCESS)
		{
			delete []platforms;
			return;
		}
		for (cl_uint i = 0;i < number_platforms; ++i)
		{
			char pbuffer[100];
			status = clGetPlatformInfo(platforms[i],
				CL_PLATFORM_VENDOR,
				sizeof(pbuffer),
				pbuffer,
				NULL);
			if(status != CL_SUCCESS)
			{
				delete []platforms;
				return;
			}
			platform = platforms[i];
			fprintf(stdout,"OpenCL Platform %d: %s\n",i,pbuffer);
		}
		delete []platforms;
	}
	if (NULL == platform)
	{
		return;
	}
	cl_context_properties cps[3] = {CL_CONTEXT_PLATFORM, (cl_context_properties)platform,0};

	cl_device_type device_type = default_device_type;
	context_ = clCreateContextFromType(cps,device_type,NULL,NULL,&status);
	if (status != CL_SUCCESS)
	{
		device_type = CL_DEVICE_TYPE_CPU;
		context_ = clCreateContextFromType(cps,device_type,NULL,NULL,&status);
	}
	if(status != CL_SUCCESS)
	{
		return ;
	}
	status = clGetContextInfo(context_,CL_CONTEXT_DEVICES,0,NULL,&device_list_size);
	if(status != CL_SUCCESS)
	{
		return;
	}
	devices_ = new cl_device_id[device_list_size];
	status = clGetContextInfo(context_,CL_CONTEXT_DEVICES,device_list_size,devices_,NULL);
	if(status != CL_SUCCESS)
	{
		delete []devices_;
		devices_ = NULL;
		return ;
	}
	for(size_t i = 0;i < device_list_size; ++i)
	{
		cl_device_type type = 0;
		status = clGetDeviceInfo(devices_[i],
			CL_DEVICE_TYPE,
			sizeof(cl_device_type),
			&type,
			NULL);
		if(type == device_type)
		{
			status = clGetDeviceInfo(devices_[i],
				CL_DEVICE_MAX_COMPUTE_UNITS,
				sizeof(cl_uint),
				&max_compute_units_,
				NULL);
			status = clGetDeviceInfo(devices_[i],
				CL_DEVICE_MAX_WORK_GROUP_SIZE,
				sizeof(cl_uint),
				&max_work_group_size_,
				NULL
				);
			selected_device_ = devices_[i];
			break;
		}
	}
	cl_command_queue_properties prop = CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE;
	command_queue_ = clCreateCommandQueue(context_,selected_device_,prop,&status);
	if(status != CL_SUCCESS)
	{
		fprintf(stderr,"Failed to create OpenCL command queue: %d\n",status);
	}
}
OpenCLDevice::~OpenCLDevice()
{
	cl_int status;
	if (kernel_)
	{
		status = clReleaseKernel(kernel_);
	}
	if (program_)
	{
		status = clReleaseProgram(program_);
	}
	for(size_t i = 0;i < kernel_args_.size(); ++i)
	{
		if(kernel_args_[i])
			status = clReleaseMemObject(kernel_args_[i]);
	}
	if(command_queue_)
	{
		status = clReleaseCommandQueue(command_queue_);
	}
	if(context_)
	{
		status = clReleaseContext(context_);
	}
	delete []devices_;
}
void OpenCLDevice::Run(size_t total_threads)
{
	//set args
	for(size_t i = 0;i < kernel_args_.size(); ++i)
	{
		/* Set kernel arguments */
		cl_int status = clSetKernelArg(
			kernel_,
			(cl_uint)i,
			sizeof(cl_mem),
			(void *)&kernel_args_[i]);
		if (status != CL_SUCCESS) {
			fprintf(stderr, "Failed to set OpenCL arg. #%d: %d\n", i,status);
			exit(-1);
		}
	}
	//run
	size_t global_threads[1];
	global_threads[0] = total_threads; //todo
	if (global_threads[0] % max_work_group_size_!= 0)
		global_threads[0] = (global_threads[0] / max_work_group_size_ + 1) * max_work_group_size_;
	size_t local_threads[1];
	local_threads[0] = max_work_group_size_;

	cl_int status = clEnqueueNDRangeKernel(
		command_queue_,
		kernel_,
		1,
		NULL,
		global_threads,
		local_threads,
		0,
		NULL,
		NULL);
	if (status != CL_SUCCESS) {
		fprintf(stderr, "Failed to enqueue OpenCL work: %d\n", status);
		exit(-1);
	}
}
void OpenCLDevice::SetKernelFile(const char* file, const char *kernel_name)
{
	/* Create the kernel program */
	cl_int status;
	std::string source_string;
	ReadSource(file,&source_string);
	const char* source_code = source_string.c_str();
	program_ = clCreateProgramWithSource(
		context_,
		1,
		&source_code,
		NULL,
		&status);
	if (status != CL_SUCCESS) {
		fprintf(stderr, "Failed to open OpenCL kernel sources: %d\n", status);
		exit(-1);
	}

#ifdef __APPLE__
	status = clBuildProgram(program_, 1, devices_, "-I. -D__APPLE__", NULL, NULL);
#else
	status = clBuildProgram(program_, 1, devices_, "-I. ", NULL, NULL);
#endif
	if (status != CL_SUCCESS) {
		fprintf(stderr, "Failed to build OpenCL kernel: %d\n", status);

		size_t kernel_info_size;
		status = clGetProgramBuildInfo(
			program_,
			selected_device_,
			CL_PROGRAM_BUILD_LOG,
			0,
			NULL,
			&kernel_info_size);
		if (status != CL_SUCCESS) {
			fprintf(stderr, "Failed to get OpenCL kernel info size: %d\n", status);
			exit(-1);
		}

		char *build_log = (char *)malloc(kernel_info_size+ 1);
		status = clGetProgramBuildInfo(
			program_,
			selected_device_,
			CL_PROGRAM_BUILD_LOG,
			kernel_info_size,
			build_log,
			NULL);
		if (status != CL_SUCCESS) {
			fprintf(stderr, "Failed to get OpenCL kernel info: %d\n", status);
			exit(-1);
		}
		build_log[kernel_info_size] = '\0';

		fprintf(stderr, "OpenCL Program Build Log: %s\n", build_log);
		free(build_log);
	}
		kernel_ = clCreateKernel(program_, kernel_name, &status);
		if (status != CL_SUCCESS) {
			fprintf(stderr, "Failed to create OpenCL kernel: %d\n", status);
			exit(-1);
		}
}


void OpenCLDevice::ReadSource(const char *filename, std::string *source_string)
{
	size_t size;
	char*  str;

	std::fstream f(filename, (std::fstream::in | std::fstream::binary));

	if(f.is_open())
	{
		std::streamsize fileSize;
		f.seekg(0, std::fstream::end);
		size = fileSize = f.tellg();
		f.seekg(0, std::fstream::beg);

		str = new char[size+1];
		if(!str)
		{
			f.close();
			return ;
		}

		f.read(str, fileSize);
		f.close();
		str[size] = '\0';
	
		*source_string = str;
		delete[] str;
	}
	else
	{
		//std::cout << "\nFile containg the kernel code(\".cl\") not found. Please copy the required file in the folder containg the executable.\n";
	}
}