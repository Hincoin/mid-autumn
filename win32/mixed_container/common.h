#ifndef COMMON_H_INCLUDED
#define COMMON_H_INCLUDED
#include <vector>

#include <stdlib.h>
#include <stdio.h>
using namespace std;

static const int malloc_count = 100 + rand()%100;
static const int fibo_test = 2 + rand()%4;
inline int fibo(int i)
{
	if (i<1)return 1;
	return fibo(i-1) + fibo(i-2);
}

inline void destruct(vector<void*>& stored)
{	for (size_t i = 0;i< stored.size();++i)
free(stored[i]);
}

inline void draw_function(vector<void*>& stored){
	if (stored.size() < malloc_count)stored.push_back(malloc(fibo(fibo_test)));
	else
	{
		for (int i = 0;i < fibo(fibo_test);++i)
			if(! stored.empty() ) {free(stored.back());stored.pop_back();};
	}
}
inline bool intersect_function(vector<void*>& stored,int seed)
{
	srand(seed);
	bool ret = rand() % 256 > 128;
	if (stored.size() < malloc_count)stored.push_back(malloc(fibo(fibo_test)));
	else
	{
		for (int i = 0;i < fibo(fibo_test);++i)
			if(! stored.empty() ) {free(stored.back());stored.pop_back();};
		return false;
	}
	return ret;
}

#endif // COMMON_H_INCLUDED
