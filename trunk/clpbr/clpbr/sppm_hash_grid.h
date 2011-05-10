#ifndef _SPPM_HASH_GRID_H_
#define _SPPM_HASH_GRID_H_

#include <vector>
#include "sppm_hit_point.h"

class SPPMHashGrid
{
public:
	typedef std::vector<std::vector<unsigned> > container_type;;
	typedef std::vector<unsigned>::const_iterator range_iterator;
public:
	SPPMHashGrid(const std::vector<ray_hit_point_t>& ray_hits,
		const std::vector<final_hit_point_t>& final_hits,float max_photon_r2,
		unsigned w,unsigned h,bool is_first_pass,
		std::vector<accum_hit_point_t>& accum_hits);

	std::pair<range_iterator,range_iterator> EqualRange(const point3f_t& p)const
	{
		//
		vector3f_t hh;
		vsub(hh,p,hpbbox_.pmin);
		vsmul(hh, hash_s_, hh);
		int ix = abs(int(hh.x)), iy = abs(int(hh.y)), iz = abs(int(hh.z));
		unsigned hash_idx = hash(ix,iy,iz);
		return std::make_pair(impl_[hash_idx].begin(),impl_[hash_idx].end());
	}
	void Insert(unsigned int hash_idx,unsigned value_idx){impl_[hash_idx].push_back(value_idx);}
	
	unsigned int hash(const int ix, const int iy, const int iz)const {
		return (unsigned int)((ix*73856093)^(iy*19349663)^(iz*83492791))%impl_.size();
	}
protected:
private:
	container_type impl_;
	bbox_t hpbbox_;
	float hash_s_;
};

#endif
