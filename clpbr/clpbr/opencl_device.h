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
	void SetKernelFile(const char* file, const char *kernel_name);
public:
	//program methods
	//All types are plain old data
	template<typename T>
	void SetReadOnlyArg(cl_uint arg_idx,const T& arg){SetArg(arg_idx,arg);}
	template<typename T>
	void SetReadOnlyArg(size_t arg_idx,const std::vector<T>& arg){SetArg(arg_idx,&arg[0],arg.size(),CL_MEM_READ_ONLY);}
	template<typename T>
	void SetReadWriteArg(size_t arg_idx,const std::vector<T>& arg){SetArg(arg_idx,&arg[0],arg.size(),CL_MEM_READ_WRITE);}

	template<typename T>
	void SetReadOnlyArg(size_t arg_idx,const T* arg,size_t arg_size){SetArg(arg_idx,arg,arg_size,CL_MEM_READ_ONLY);}
	template<typename T>
	void SetReadWriteArg(size_t arg_idx,const T* arg, size_t arg_size){SetArg(arg_idx,arg,arg_size,CL_MEM_READ_WRITE);}

	cl_mem GetMemoryObject(size_t arg_idx){return kernel_args_[arg_idx];}

	void SetArgFromMemoryObject(size_t arg_idx, cl_mem mem);

	void Run(size_t total_threads);
	void Wait();

	template<typename T>
	void ReadBuffer(unsigned arg_idx,T* output,unsigned count);
private:
	template<typename T>
	void SetArg(cl_uint arg_idx,const T& arg);
	template<typename T>
	void SetArg(size_t arg_idx,const T* arg,size_t arg_size, cl_mem_flags flags);
	void ReadSource(const char* source_file,std::string *source_string);
public:
	explicit OpenCLDevice(cl_device_type default_device_type);
	~OpenCLDevice();
private:
private:
	std::vector<cl_mem> kernel_args_;

	cl_context context_;
	cl_device_id *devices_;
	cl_device_id selected_device_;
	cl_command_queue command_queue_;
	cl_program program_;
	cl_kernel kernel_;
	size_t max_compute_units_;
	size_t max_work_group_size_;
	size_t work_group_size_;
};

template<typename T>
void OpenCLDevice::SetArg(cl_uint arg_idx,const T& arg)
{
	cl_int status = clSetKernelArg(kernel_,
		arg_idx,
		sizeof(T),
		(void*)&arg);
	if (status != CL_SUCCESS)
	{
		fprintf(stderr,"Failed to set OpenCL arg. %d:%d\n",arg_idx,status);
		exit(-1);
	}
	if(status != CL_SUCCESS)
	{
		exit(-1);
	}
}

inline void OpenCLDevice::SetArgFromMemoryObject(size_t arg_idx, cl_mem mem)
{
	kernel_args_[arg_idx] = mem;
}
template<typename T>
void OpenCLDevice::SetArg(size_t arg_idx,const T* arg, size_t arg_size,cl_mem_flags flags)
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
	if(NULL == kernel_args_[arg_idx])
	{
		kernel_args_[arg_idx] = clCreateBuffer(context_,flags,
			sizeof(T) * arg_size,NULL,&status);
		if(status != CL_SUCCESS)
		{
			exit(-1);
		}
	}
	status = clEnqueueWriteBuffer(
			command_queue_,
			kernel_args_[arg_idx],
			CL_TRUE,//todo: tobe async
			0,
			sizeof(T) * arg_size,
			arg,
			0,
			NULL,
			NULL);

	if(status != CL_SUCCESS)
	{
		exit(-1);
	}
	clFlush(command_queue_);
}


template<typename T>
void OpenCLDevice::ReadBuffer(unsigned arg_idx,T* output,unsigned count)
{
	/* Enqueue readBuffer */
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
	if (status != CL_SUCCESS)
	{
		fprintf(stderr,"OpenCL ReadBuffer Error: %d\n",status);
		exit(-1);
	}
}
#endif