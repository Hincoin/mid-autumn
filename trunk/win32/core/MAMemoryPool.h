#ifndef MAMEMORYPOOL_H
#define MAMEMORYPOOL_H


//designed by lzy for big object allocation for 32bit arch
namespace ma
{
namespace core
{
	//template<typename ContainerAllocator>
	struct MAMemoryPool{

		static void* malloc(size_t sz);
		static void free(void* p);//mark as freed
		static bool clean_unused();
	};
}
}
#endif