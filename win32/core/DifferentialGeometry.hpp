#ifndef _MA_INCLUDED_DIFFERENTIAL_GEOMETRY_HPP_
#define _MA_INCLUDED_DIFFERENTIAL_GEOMETRY_HPP_

#include "Point.hpp"
#include "VectorType.hpp"

namespace ma
{
    template<typename S>
    class DifferentialGeometry
    {
        typedef S shape_type;
        typedef S* shape_pointer;
        typedef typename S::VectorType vector_type;
        typedef Point<vector_type> point_type;
        typedef vector_type normal_type;

        typedef scalar_type<V>::type scalar_type;

        ///

        DifferentialGeometry(const point_type& p,
                             const vector_type& dp_du,const vector_type& dp_dv,
                             const vector_type& dn_du,const vector_type& dn_dv,
                             scalar_type uu,scalar_type vv,const shape_pointer sh):
                point(p),normal(dpdu.cross(dpdv).normalize()),dpdu(dp_du),dpdv(dp_dv),dndu(dn_du),dndv(dn_dv),u(uu),v(vv),shape(sh)
        {
            if ( shape->isTransformSwapHandedness() ^ shape->isReverseNormal())
                normal *= -1;
        }

        point_type point;
        normal_type normal;
        vector_type dpdu,dpdv;
        vector_type dndu,dndv;
        scalar_type u,v;
        const shape_pointer shape;
    };
}
#endif
