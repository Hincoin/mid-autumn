#ifndef _MA_INCLUDED_DIFFERENTIAL_GEOMETRY_HPP_
#define _MA_INCLUDED_DIFFERENTIAL_GEOMETRY_HPP_

#include "Point.hpp"
#include "VectorType.hpp"

namespace ma
{
    template<typename S,int D>
    class DifferentialGeometry
    {
	public:
        typedef typename vector_type<S,D>::type vector_t;
        typedef Point<vector_t> point_t;
		typedef typename normal_type<S,D>::type normal_t;

        typedef S scalar_t;

        ///
		DifferentialGeometry(){}

        template<typename ShapePtr>
        DifferentialGeometry(const point_t& p,
                             const vector_t& dp_du,const vector_t& dp_dv,
                             const vector_t& dn_du,const vector_t& dn_dv,
                             scalar_t uu,scalar_t vv,const ShapePtr sh):
                point(p),normal(dp_du.cross(dp_dv).normalized()),dpdu(dp_du),dpdv(dp_dv),dndu(dn_du),dndv(dn_dv),u(uu),v(vv)
        {
            if (sh->isTransformSwapHandedness() ^ sh->isReverseNormal())
                normal *= -1;
        }
		template<typename RayDiff>
		void computeDifferentials(const RayDiff& ray)const{
			if (ray.has_differential) {
				// Estimate screen-space change in \pt and $(u,v)$
				// Compute auxiliary intersection points with plane
				scalar_t dir = -dot(normal, normal_t(point.x(), point.y(), point.z()));
				vector_t rxv(ray.rx.o.x(), ray.rx.o.y(), ray.rx.o.z());
				scalar_t tx = -(dot(normal, rxv) + dir) / dot(normal, ray.rx.dir);
				point_t px = ray.rx.o + tx * ray.rx.dir;
				vector_t ryv(ray.ry.o.x(), ray.ry.o.y(), ray.ry.o.z());
				scalar_t ty = -(dot(normal, ryv) + dir) / dot(normal, ray.ry.dir);
				point_t py = ray.ry.o + ty * ray.ry.dir;
				dpdx = px - point;
				dpdy = py - point;
				// Compute $(u,v)$ offsets at auxiliary points
				// Initialize _A_, _Bx_, and _By_ matrices for offset computation
				scalar_t A[2][2], Bx[2], By[2], x[2];
				int axes[2];
				if (std::abs(normal.x()) > std::abs(normal.y()) && std::abs(normal.x()) > std::abs(normal.z())) {
					axes[0] = 1; axes[1] = 2;
				}
				else if (std::abs(normal.y()) > std::abs(normal.z())) {
					axes[0] = 0; axes[1] = 2;
				}
				else {
					axes[0] = 0; axes[1] = 1;
				}
				// Initialize matrices for chosen projection plane
				A[0][0] = dpdu[axes[0]];
				A[0][1] = dpdv[axes[0]];
				A[1][0] = dpdu[axes[1]];
				A[1][1] = dpdv[axes[1]];
				Bx[0] = px[axes[0]] - point[axes[0]];
				Bx[1] = px[axes[1]] - point[axes[1]];
				By[0] = py[axes[0]] - point[axes[0]];
				By[1] = py[axes[1]] - point[axes[1]];
				if (SolveLinearSystem2x2(A, Bx, x)) {
					dudx = x[0]; dvdx = x[1];
				}
				else  {
					dudx = 1.; dvdx = 0.;
				}
				if (SolveLinearSystem2x2(A, By, x)) {
					dudy = x[0]; dvdy = x[1];
				}
				else {
					dudy = 0.; dvdy = 1.;
				}
			}
			else {
				dudx = dvdx = 0.;
				dudy = dvdy = 0.;
				dpdx = dpdy = vector_t(0,0,0);
			}
		};

        point_t point;
        normal_t normal;
        mutable vector_t dpdu,dpdv;
        mutable vector_t dndu,dndv;
        scalar_t u,v;

		mutable vector_t dpdx, dpdy;
		mutable scalar_t dudx, dvdx, dudy, dvdy;
    };
}
#endif
