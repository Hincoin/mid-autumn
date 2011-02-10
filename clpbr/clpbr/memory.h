#ifndef _MEMORY_H_
#define _MEMORY_H_


void *alloc_aligned(size_t size);
template <typename T> T *alloc_aligned(unsigned count) {
	return (T *)alloc_aligned(count * sizeof(T));
}


void free_aligned(void *);


#endif