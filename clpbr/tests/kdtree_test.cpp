#include "all_tests.h"

#include "config.h"
#include <vector>
#include <cmath>
#include "spectrum.h"
#include "geometry.h"
#include "kdtree.h"
#include "photon_map.h"
#include "random_number_generator_mt19937.h"
struct compare{
	point3f_t p;
	compare(point3f_t pp):p(pp){}
	bool operator()(const photon_t& p1,const photon_t& p2)const
	{
		return distance_squared(p1.p,p) > distance_squared(p2.p,p);
	}
};
void kdtree_test()
{
	std::vector<photon_t> photon_map;
	RandomNumberGeneratorMT19937 rng;
	photon_kd_tree_t kdtree;
	for(int i = 0;i < 100000;++i)
	{
		point3f_t p;
		p.x = i*rng.RandomFloat();
		p.y = i*rng.RandomFloat();
		p.z = i*rng.RandomFloat();
		photon_t photon;
		photon.p = p;
		photon_map.push_back(photon);
	}
	kd_tree_init(&kdtree, photon_map);
	//kd_tree_lookup(kdtree,
	point3f_t p;
	int N = rng.RandomUnsignedInt() % 1000;
	p.x =N * rng.RandomFloat();
	p.y =N * rng.RandomFloat();
	p.z =N * rng.RandomFloat();
	photon_process_data_t photon_map_data;
	photon_process_data_init(&photon_map_data,50);
	close_photon_t close_photon_data_store[MAX_CLOSE_PHOTON_LOOKUP];
	photon_map_data.photons = close_photon_data_store;
	float dist = 100.f;
	kd_tree_lookup(kdtree,p,&photon_map_data,photon_process,dist);
	//estimate reflected light from photons
	close_photon_t *photons = photon_map_data.photons;
	int n_found = photon_map_data.found_photons;

	std::make_heap(photon_map.begin(),photon_map.end(),compare(p));
	std::vector<photon_t> pmap;
	for (int i = 0;i<photon_map.size();++i)
	{
		if(distance_squared(photon_map[i].p,p) < dist)
			pmap.push_back(photon_map[i]);
	}
	int stop = 0;
}