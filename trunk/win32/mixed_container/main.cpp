#include "mixed_container.h"
#include <iostream>

#include <time.h>

clock_t run_baseline(int SZ,int N)
{
	srand(0);
	int seed = rand();
	std::vector<simple_shape> c;
	SZ *= 10;clock_t t_start = clock();
	for (int j = 0;j<SZ;++j)
	{
		c.push_back(simple_shape());
	}
	
	for (int i = 0 ;i<N;++i){
		for (int k = 0;k < c.size(); ++k)
		{
			c[k].draw();
			
		}
		for (int k = 0;k < c.size(); ++k)
			if(c[k].intersect(seed))break;
	}
	return clock() - t_start;
}
template<typename C>
clock_t run_test(int SZ,int N)
{
	srand(0);
	int seed = rand();
	C c;clock_t t_start = clock();
	for (int j = 0;j<SZ;++j)
	{
		c.push_back(simple_shape());
		c.push_back(simple_shape2());
		c.push_back(simple_shape3());
		c.push_back(simple_shape4());
		c.push_back(simple_shape5());
		c.push_back(simple_shape6());
		c.push_back(simple_shape7());
		c.push_back(simple_shape8());
		c.push_back(simple_shape9());
		c.push_back(simple_shape0());
	}
	
	for (int i = 0 ;i<N;++i){
		c.draw();
		c.intersect(seed);
	}
	return clock() - t_start;
}
template<typename C>
clock_t run_pointer_test(int SZ,int N)
{
	srand(0);
	int seed = rand();
	clock_t t_start = clock();
	{
	C c;
	for (int j = 0;j<SZ;++j)
	{
		c.push_back(new simple_shape());
		c.push_back(new simple_shape2());
		c.push_back(new simple_shape3());
		c.push_back(new simple_shape4());
		c.push_back(new simple_shape5());
		c.push_back(new simple_shape6());
		c.push_back(new simple_shape7());
		c.push_back(new simple_shape8());
		c.push_back(new simple_shape9());
		c.push_back(new simple_shape0());
	}

	for (int i = 0 ;i<N;++i){
		c.draw();
		c.intersect(seed);
	}
	}
 
	clock_t t = clock() - t_start;
	return t;
}

template<typename C>
clock_t run_shared_pointer_test(int SZ,int N)
{
	srand(0);
	int seed = rand();	
	clock_t t_start = clock();
	{
		C c;
		for (int j = 0;j<SZ;++j)
		{
			c.push_back(boost::shared_ptr<simple_shape>(new simple_shape()));
			c.push_back(boost::shared_ptr<simple_shape2>(new simple_shape2()));
			c.push_back(boost::shared_ptr<simple_shape3>(new simple_shape3()));
			c.push_back(boost::shared_ptr<simple_shape4>(new simple_shape4()));
			c.push_back(boost::shared_ptr<simple_shape5>(new simple_shape5()));
			c.push_back(boost::shared_ptr<simple_shape6>(new simple_shape6()));
			c.push_back(boost::shared_ptr<simple_shape7>(new simple_shape7()));
			c.push_back(boost::shared_ptr<simple_shape8>(new simple_shape8()));
			c.push_back(boost::shared_ptr<simple_shape9>(new simple_shape9()));
			c.push_back(boost::shared_ptr<simple_shape0>(new simple_shape0()));
		}

		for (int i = 0 ;i<N;++i){
			c.draw();
			c.intersect(seed);
		}
	}
	clock_t t = clock() - t_start;
	return t;
}

#include "virtual_container.h"
clock_t run_virtual_test(int SZ,int N)
{
	srand(0);
	int seed = rand();	
	clock_t t_start = clock();
	{
		std::vector<VShape*> c;
		for (int j = 0;j<SZ;++j)
		{
			c.push_back(new vsimple_shape1());
			c.push_back(new vsimple_shape2());
			c.push_back(new vsimple_shape3());
			c.push_back(new vsimple_shape4());
			c.push_back(new vsimple_shape5());
			c.push_back(new vsimple_shape6());
			c.push_back(new vsimple_shape7());
			c.push_back(new vsimple_shape8());
			c.push_back(new vsimple_shape9());
			c.push_back(new vsimple_shape0());
		}

		for (int i = 0 ;i<N;++i){
			for (int k = 0;k < c.size(); ++k)
			{
				c[k]->draw();
			}
			for (int k = 0;k < c.size(); ++k)
				if(c[k]->intersect(seed))break;
		}
		for (int k = 0;k < c.size(); ++k)
			delete c[k];
	}


	clock_t t = clock() - t_start;
	return t;
}
int main()
{
	int N_ELEMENT = 256;
	int N_ITERATION = 8 * 256;
	
	//printf("std::vector<simple_shape>: %ld \n",(long)(run_baseline(N_ELEMENT,N_ITERATION)));
	//typedef MixedShapeVector<> MSV;
	//printf("MixedShapeVector: %ld \n",(long)(run_test<MSV>(N_ELEMENT,N_ITERATION)));
	//typedef ShapeVariantVector<> SVV;
	//printf("ShapeVariantVector: %ld \n",(long)(run_test<SVV>(N_ELEMENT,N_ITERATION)));



	typedef ShapeVariantVector<generic_shape_ptr> SVVP;
	typedef ShapeVariantVector<generic_shared_shape_ptr> SVVGSSP;
	printf("ShapeVariantVector ptr: %ld \n",(long)(run_pointer_test<SVVP>(N_ELEMENT,N_ITERATION)));
	printf("shared ptr array : %ld \n",(long)(run_shared_pointer_test<SVVGSSP>(N_ELEMENT,N_ITERATION)));
	printf("virtual Shape test :%ld\n",(long)(run_virtual_test(N_ELEMENT,N_ITERATION)));
	std::cout<<sizeof(generic_shape)<<"\n"<<sizeof(boost::variant<double>)<< "\n" << sizeof(boost::variant<std::vector<void*> >) <<std::endl;
	return 0;
}