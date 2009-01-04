#ifndef _MA_INCLUDED_DIFFERENTIAL_GEOMETRY_HPP_
#define _MA_INCLUDED_DIFFERENTIAL_GEOMETRY_HPP_

namespace ma{
template<typename P,typename V,typename UV>
class DifferentialGeometry{
	P point;
	V normal;
	V dpdu,dpdv;
	V dndu,dndv;
	UV u,v;
};
}
#endif