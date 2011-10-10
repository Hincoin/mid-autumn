#include <cassert>
#include "sppm_hash_grid.h"


SPPMHashGrid::SPPMHashGrid(const std::vector<ray_hit_point_t>& ray_hits,
			 const std::vector<final_hit_point_t>& final_hits,float max_photon_r2,
			 unsigned w,unsigned h,bool is_first_pass,
			 std::vector<accum_hit_point_t>& accum_hits)
{
	assert(is_first_pass || !is_first_pass && ray_hits.size() == final_hits.size());
	assert(accum_hits.size() == ray_hits.size());
	//
	bbox_init(&hpbbox_);
	for(size_t i = 0;i < ray_hits.size(); ++i)
	{
		if(ray_hits[i].type == hp_constant_color)continue;
		bbox_union_with_point(&hpbbox_,&ray_hits[i].pos);
	}
	vector3f_t ssize;
	vsub(ssize,hpbbox_.pmax,hpbbox_.pmin);
	float irad = is_first_pass? 
		(((ssize.x + ssize.y + ssize.z)/3.f)/((w+h)/2.f)*2.f):(sqrtf(max_photon_r2));
	bbox_init(&hpbbox_);
	int vphoton = 0;
	vector3f_t vtmp;
	vinit(vtmp,irad,irad,irad);
	point3f_t ptmp;
	for (size_t i = 0;i < ray_hits.size(); ++i)
	{
		if(ray_hits[i].type == hp_constant_color)
			continue;
		accum_hit_point_t& hp = accum_hits[i];
		hp = accum_hit_point_t((is_first_pass || final_hits[i].r2 <= 0)?
			irad * irad : final_hits[i].r2);
		vphoton++;
		vsub(ptmp,ray_hits[i].pos,vtmp);
		bbox_union_with_point(&hpbbox_,&ptmp);
		vadd(ptmp,ray_hits[i].pos,vtmp)
		bbox_union_with_point(&hpbbox_,&ptmp);
	}
	hash_s_= 1.f/(irad*2.f);
	impl_.resize(vphoton);
	for (size_t i = 0;i < ray_hits.size();++i)
	{
		if(ray_hits[i].type == hp_constant_color)continue;
		accum_hit_point_t& hp = accum_hits[i];
		irad = sqrtf(hp.r2);
		point3f_t bmin,bmax;
		vinit(vtmp,irad,irad,irad)
		vsub(ptmp,ray_hits[i].pos,vtmp);
		vsub(ptmp,ptmp,hpbbox_.pmin);
		vsmul(bmin,hash_s_,ptmp);

		vadd(ptmp,ray_hits[i].pos,vtmp);
		vsub(ptmp,ptmp,hpbbox_.pmin);
		vsmul(bmax,hash_s_,ptmp);

		for (int iz = abs(int(bmin.z));iz <= abs(int(bmax.z));iz++)
		for (int iy = abs(int(bmin.y));iy <= abs(int(bmax.y));iy++)
		for (int ix = abs(int(bmin.x));ix <= abs(int(bmax.x));ix++)
		{
			int hv = hash(ix,iy,iz);impl_[hv].push_back(i);
		}
	}
}