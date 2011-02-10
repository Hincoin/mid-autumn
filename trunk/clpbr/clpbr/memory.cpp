#include "memory.h"

#ifndef POINTER_SIZE
#if defined(__amd64__) || defined(_M_X64)
#define POINTER_SIZE 8
#elif defined(__i386__) || defined(_M_IX86)
#define POINTER_SIZE 4
#endif
#endif


#if !defined(__APPLE__) && !defined(__OpenBSD__)
#include <malloc.h> // for _alloca, memalign
#endif

// Memory Allocation Functions
void *alloc_aligned(size_t size) {
#define L1_CACHE_LINE_SIZE 64

#if defined(_WIN32) || defined(_WIN64)
	return _aligned_malloc(size, L1_CACHE_LINE_SIZE);
#elif defined (__OpenBSD__) || defined(__APPLE__)
	// Allocate excess memory to ensure an aligned pointer can be returned
	void *mem = malloc(size + (L1_CACHE_LINE_SIZE-1) + sizeof(void*));
	char *amem = ((char*)mem) + sizeof(void*);
#if (POINTER_SIZE == 8)
	amem += L1_CACHE_LINE_SIZE - (reinterpret_cast<uint64_t>(amem) &
		(L1_CACHE_LINE_SIZE - 1));
#else
	amem += L1_CACHE_LINE_SIZE - (reinterpret_cast<uint32_t>(amem) &
		(L1_CACHE_LINE_SIZE - 1));
#endif
	((void**)amem)[-1] = mem;
	return amem;
#else
	return memalign(L1_CACHE_LINE_SIZE, size);
#endif
}


void free_aligned(void *ptr) {
	if (!ptr) return;
#if defined(_WIN32) || defined(_WIN64)
	_aligned_free(ptr);
#elif defined (__OpenBSD__) || defined(__APPLE__)
	free(((void**)ptr)[-1]);
#else
	free(ptr);
#endif
}

