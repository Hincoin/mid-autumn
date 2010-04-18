#ifndef _SHAPE_INCLUDED_HPP_
#define _SHAPE_INCLUDED_HPP_


#include "ScalarType.hpp"
#include "VectorType.hpp"
#include "SpaceSegment.hpp"
#include "Ray.hpp"
#include "DifferentialGeometry.hpp"
#include "Vector.hpp"
#include "Transform.hpp"
/// shape interface it's for rendering not for geometry operation
/// no geometry operation here
namespace ma
{

    template<typename Derived,typename Configure>
    class Shape
    {
    private:
        const Derived& derived()const
        {
            return static_cast<const Derived&>(*this);
        }
        Derived& derived()
        {
            return static_cast<Derived&>(*this);
        }
    public:
        //type compute
		typedef Configure config_t;
       // friend class Configure;
		typedef typename Configure::ScalarType ScalarType;
		
		static const int Dimension = Configure::Dimension ;
		static const bool CanIntersect = Configure::CanIntersect;
		typedef typename vector_type<ScalarType,Dimension>::type vector_t;
		typedef typename transform_type<ScalarType,Dimension>::type transform_t;
		typedef SpaceSegment<vector_t> BBox;
		typedef Ray<vector_t> ray_t; 
		typedef typename normal_type<ScalarType,Dimension>::type normal_t;

		typedef DifferentialGeometry<ScalarType,Dimension> differential_geometry;
		/// interfaces
		BBox objectBound()const{return derived().objectBoundImpl();}

		BBox worldBound()const {return Configure::worldBound(derived());}
		bool canIntersect()const {return CanIntersect;}

		template<typename S>
		void refine(std::vector<S >& refined)const{
		    //typedef typename S::config_t S_Conf;
		    ///// make sure it's refine compatible
		    //BOOST_STATIC_ASSERT((boost::is_same<typename S_Conf::ScalarType,ScalarType>::value &&
		    //S_Conf::Dimension == Dimension)
		    //);
			return derived().refineImpl(refined);
		}
		//predict intersect
		bool intersectP(const ray_t& r)const{return derived().intersectPImpl(r);}
		bool intersect(const ray_t& r,ScalarType& tHit,differential_geometry& dg)const{
			return derived().intersectImpl(r,tHit,dg);
		}
		void getShadingGeometry(const transform_t& obj2world,
			const differential_geometry& dg,
			differential_geometry& dgShading)const{
			    Configure::getShadingGeometry(derived(),obj2world,dg,dgShading);
			    }
		ScalarType area()const {return Configure::area(derived());}
///typedef typename Configure::
        bool isTransformSwapHandedness()const
        {
            return transform_swap_handedness_;
        }
        bool isReverseNormal()const{return reverse_normal_;}
        const transform_t& worldToObj()const{return world_to_obj_;}
    protected:
        const transform_t obj_to_world_,world_to_obj_;
        const bool reverse_normal_;
        bool transform_swap_handedness_;

        Shape(const transform_t& obj2world,bool reverse_normal):
                obj_to_world_(obj2world),world_to_obj_(obj2world.inverse()),reverse_normal_(reverse_normal),
                transform_swap_handedness_(transform_op::swap_handness(obj2world))
        {}
				~Shape(){}
    };
    /// is this nessary for compilers ?
    template<typename S,typename SR>
    void refine_shape(const S& shape,std::vector<SR>& refined)
    {
        shape.template refine<SR>(refined);
    }
    template<typename S>
    struct ShapeTraits{
        typedef typename  S::ScalarType ScalarType;
		static const int Dimension =  S::Dimension ;
		static const bool CanIntersect =  S::CanIntersect;

		typedef typename S::index_type index_type;
		typedef typename vector_type<ScalarType,Dimension>::type vector_t;
		typedef typename transform_type<ScalarType,Dimension>::type transform_t;
		typedef SpaceSegment<vector_t> BBox;
		typedef Ray<vector_t> ray_t;
		typedef DifferentialGeometry<ScalarType,Dimension> differential_geometry;
        };
    struct base_shape_configure{
        //overide this 2 methods if the shape has different implementation
        template<typename SHAPE>
            static void getShadingGeometry(const SHAPE& ,const typename ShapeTraits<SHAPE>::transform_t& /*obj2world*/,
			const typename ShapeTraits<SHAPE>::differential_geometry& dg,
			typename ShapeTraits<SHAPE>::differential_geometry& dgShading){
			    dgShading = dg;
			    }

            template<typename SHAPE>
			  static typename ShapeTraits<SHAPE>::ScalarType area(const SHAPE&){return 0;}
			  template<typename SHAPE>
			  static typename ShapeTraits<SHAPE>::BBox worldBound(const SHAPE& s) {return (s.obj_to_world_ * s.objectBound());}
        };
	struct default_config_3d:base_shape_configure{
		static const int Dimension = 3;
		typedef float ScalarType;
	};

}


#include "ParamSet.hpp"


namespace ma{
	namespace details{
			template<typename ShapeT>
			struct shape_creator;
	}
	template<typename ShapeT>
	ShapeT* create_shape(const typename ShapeT/*::config_t*/::transform_t &o2w,
		bool reverseOrientation, const ParamSet &params)
	{
		return details::shape_creator<ShapeT>()(o2w,reverseOrientation,params);
	}
}

#endif
