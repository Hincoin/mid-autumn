#ifndef MA_INTERSECT_HPP
#define MA_INTERSECT_HPP

#include "MAMath.hpp"
#include "SpaceSegment.hpp"
#include "Ray.hpp"

#include <utility>
namespace ma
{

    template<typename V>
    bool intersectP(const SpaceSegment<V>& seg,
                    const Ray<V>& ray,
                    typename scalar_type<V>::type& t_hit0,
                    typename scalar_type<V>::type& t_hit1)
    {
        typedef typename scalar_type<V>::type s_type;
        static const int D = dimensions<V>::value;
        s_type t0 = ray.mint,t1 = ray.maxt;
        for (int i = 0;i < D; ++i)
        {
            s_type inv_dir = reciprocal(ray.dir[i]);
            s_type tNear = (seg.smin[i] - ray.o[i]) * inv_dir;
            s_type tFar = (seg.smax[i] - ray.o[i]) * inv_dir;
            using namespace std;
            if (tNear > tFar) std::swap(tNear,tFar);
            t0 = tNear > t0 ? tNear:t0;
            t1 = tFar < t1 ? tFar : t1;
            if (t0 > t1) return false;
        }
        t_hit0 = t0;
        t_hit1 = t1;
        return true;
    }

}
#endif
