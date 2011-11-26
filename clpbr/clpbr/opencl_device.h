#ifndef _OPENCL_DEVICE_H_
#define _OPENCL_DEVICE_H_

#include <CL/cl.h>
#include <vector>
#include <string>

//todo: change this to OpenCLProgram
class OpenCLDevice
{
public:
	//device methods 
	void SetKernelFile(const char* file);
public:
	//program methods
	//All types are plain old data
	template<typename T>
	void SetReadOnlyArg(size_t arg_idx,const T& arg){SetArg(arg_idx,arg,CL_MEM_READ_ONLY);}
	template<typename T>
	void SetReadOnlyArg(size_t arg_idx,const std::vector<T>& arg){SetArg(arg_idx,arg,CL_MEM_READ_ONLY);}
	template<typename T>
	void SetReadWriteArg(size_t arg_idx,const T& arg){SetArg(arg_idx,arg,CL_MEM_READ_WRITE);}
	template<typename T>
	void SetReadWriteArg(size_t arg_idx,const std::vector<T>& arg){SetArg(arg_idx,arg,CL_MEM_READ_WRITE);}


	void Run();

	template<typename T>
	void ReadBuffer(unsigned arg_idx,T* output,unsigned count);
private:
	template<typename T>
	void SetArg(size_t arg_idx,const T& arg, cl_mem_flags flags);
	template<typename T>
	void SetArg(size_t arg_idx,const std::vector<T>& arg,cl_mem_flags flags);
	void ReadSource(const char* source_file,std::string *source_string);
public:
	OpenCLDevice();
	~OpenCLDevice();
private:
	std::vector<cl_mem> kernel_args_;

	cl_context context_;
	cl_device_id *devices_;
	cl_device_id selected_device_;
	cl_command_queue command_queue_;
	cl_program program_;
	cl_kernel kernel_;
	cl_uint max_compute_units_;
	cl_uint max_work_group_size_;
};

template<typename T>
void OpenCLDevice::SetArg(size_t arg_idx,const T& arg, cl_mem_flags flags)
{
	if (arg_idx >= kernel_args_.size())
	{
		kernel_args_.resize(arg_idx + 1,NULL);
	}
	cl_int status;
	if(NULL != kernel_args_[arg_idx])
	{
		clReleaseMemObject(kernel_args_[arg_idx]);
		kernel_args_[arg_idx] = NULL;
	}
	kernel_args_[arg_idx] = clCreateBuffer(context_,flags,
		sizeof(T),NULL,&status);
	status = clEnqueueWriteBuffer(
			command_queue_,
			kernel_args_[arg_idx],
			CL_TRUE,//todo: tobe async
			0,
			sizeof(T),
			&arg,
			0,
			NULL,
			NULL);
}
template<typename T>
void OpenCLDevice::SetArg(size_t arg_idx,const std::vector<T>& arg,cl_mem_flags flags)
{
	if (arg_idx >= kernel_args_.size())
	{
		kernel_args_.resize(arg_idx + 1,NULL);
	}
	cl_int status;

	if(NULL != kernel_args_[arg_idx])
	{
		clReleaseMemObject(kernel_args_[arg_idx]);
		kernel_args_[arg_idx] = NULL;
	}
	kernel_args_[arg_idx] = clCreateBuffer(context_,flags,
		sizeof(T) * arg.size(),NULL,&status);
	if(status != CL_SUCCESS)
	{
		exit(-1);
	}
	status = clEnqueueWriteBuffer(
			command_queue_,
			kernel_args_[arg_idx],
			CL_TRUE,//todo: tobe async
			0,
			sizeof(T) * arg.size(),
			&arg[0],
			0,
			NULL,
			NULL);

	if(status != CL_SUCCESS)
	{
		exit(-1);
	}
}


template<typename T>
void OpenCLDevice::ReadBuffer(unsigned arg_idx,T* output,unsigned count)
{
	/* Enqueue readBuffer */
	clFinish(command_queue_);
	cl_uint status = clEnqueueReadBuffer(
			command_queue_,
			kernel_args_[arg_idx],
			CL_TRUE,
			0,
			count * sizeof(T),
			output,
			0,
			NULL,
			NULL);
}
#endif