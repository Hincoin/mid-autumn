#ifndef MAPRIMITIVE_HPP_INCLUDED
#define MAPRIMITIVE_HPP_INCLUDED

#include "MAConfig.hpp"

#include "VectorType.hpp"
#include "SpaceSegment.hpp"
#include "DifferentialGeometry.hpp"
#include "Ray.hpp"
#include "SharedPointer.hpp"
#include <vector>

#include "VirtualInterface.hpp"

#include "Shape.hpp"
#include "Vector.hpp"
namespace ma
{

template<typename S,int D,typename Tag = NullType>
struct PrimitiveConfigure:MAConfigure<S,D,Tag>{};


template<typename Conf>
class MAPrimitive;

        //template<typename Conf>
        //struct MAIntersection
        //{
        //    typedef typename Conf::ScalarType ScalarType;
        //    static const int Dimension = Conf::Dimension;
        //    typedef DifferentialGeometry<ScalarType,Dimension> differential_geometry;
        //    typedef MAPrimitive<PrimitiveConfigure<ScalarType,Dimension> > primitive_type;
        //    typedef typename  transform_type<ScalarType,Dimension>::type TransformType;

        //    differential_geometry dg;
        //    const primitive_type* primitive;
        //    TransformType world2object;
        //};

///interface constraint
template<typename Derived,typename Conf>
class Primitive{
public:
            typedef typename Conf::ScalarType ScalarType;
            static const int Dimension = Conf::Dimension;
            typedef typename vector_type<ScalarType,Dimension>::type vector_t;
            typedef SpaceSegment<vector_t> BBox;
            typedef  Point<vector_t> point_type;
            typedef Ray<vector_t> ray_type;
			typedef typename Conf::intersection_t intersection_t;
            typedef MAPrimitive<Conf>  interface_type;
            typedef typename shared_pointer<interface_type>::type shared_primitive;
            typedef std::vector<shared_primitive> shared_primitive_array;
            typedef Primitive<Derived,Conf> class_type;

			typedef typename Conf::bsdf_ptr bsdf_ptr;
			typedef typename Conf::differential_geometry_t differential_geometry_t;
			typedef typename Conf::transform_t transform_t;
protected:
Primitive(){}
~Primitive(){}
private:
Derived& derived() {return static_cast<Derived&>(*this);}
const Derived& derived()const{return static_cast<const Derived&>(*this);}
public:
        BBox  worldBound ()const
            {
                return derived().worldBoundImpl();
            }
            bool  canIntersect ()const
            {
                return derived().canIntersectImpl();
            }
            bool  intersect (const ray_type& r,intersection_t& isect)const
            {
                return derived().intersectImpl(r,isect);
            }
            bool intersectP ( const ray_type& r)const
            {
               return derived().intersectPImpl(r);
            }
			bsdf_ptr getBSDF(const differential_geometry_t &dg,
				const transform_t &WorldToObject) const
			{
				return derived().getBSDFImpl(dg,WorldToObject);
			}
            void refine (shared_primitive_array& ps)const
            {
                return derived().refineImpl(ps);
            }
            void fullyRefine(shared_primitive_array& ps)const
            {
                MA_ASSERT(!canIntersect())
                shared_primitive_array todo;
                refine(todo);
                while (!todo.empty())
                {
                    shared_primitive p = todo.back();
                    todo.pop_back();
                    if (p->canIntersect())
                        ps.push_back(p);
                    else
                        p->refine(todo);
                }
            }
    };

template<typename SharedP>
inline void fullyRefine(SharedP p,std::vector<SharedP>& refined)
{
    if (p->canIntersect())
        refined.push_back(p);
    else
        p->fullyRefine(refined);
}

MA_INTERFACE_BEGIN(MAPrimitive,Primitive)
MA_VTABLE_BEGIN
MA_VTABLE_FUNC(void ,destruct, (interface_type&))
MA_VTABLE_FUNC(void ,swap, (interface_type&,interface_type&))
MA_VTABLE_FUNC(typename parent_type::BBox,worldBound, (const interface_type&))
MA_VTABLE_FUNC(bool, canIntersect, (const interface_type&))
MA_VTABLE_FUNC(bool, intersect,(const interface_type&,const typename parent_type::ray_type& ,typename parent_type::intersection_t&))
MA_VTABLE_FUNC(bool, intersectP,(const interface_type&, const typename parent_type::ray_type& ))
MA_VTABLE_FUNC(void, refine,(const interface_type&,typename parent_type::shared_primitive_array&))
MA_VTABLE_FUNC(typename parent_type::bsdf_ptr, getBSDF,(const interface_type&,const typename parent_type::differential_geometry_t&,const typename parent_type::transform_t&))
MA_VTABLE_END

MA_VTABLE_IMPL(void ,destruct,(),(*this))
MA_VTABLE_IMPL(void ,swap,(interface_type& other),(*this,other))
MA_VTABLE_IMPL(typename parent_type::BBox,worldBound,()const,(*this))
MA_VTABLE_IMPL(bool, canIntersect, ()const,(*this))
MA_VTABLE_IMPL(bool, intersect,
(const typename parent_type::ray_type& r,typename parent_type::intersection_t& isect)const,(*this,r,isect))
MA_VTABLE_IMPL(bool, intersectP,(const typename parent_type::ray_type& r)const,(*this,r))
MA_VTABLE_IMPL(void, refine,(typename parent_type::shared_primitive_array& ps)const,(*this,ps))
MA_VTABLE_IMPL(typename parent_type::bsdf_ptr, getBSDF,(const typename parent_type::differential_geometry_t &dg,const typename parent_type::transform_t &WorldToObject)const,(*this,dg,WorldToObject))

MA_FORWARD_TYPEDEF(shared_primitive)
MA_FORWARD_TYPEDEF(shared_primitive_array)
MA_FORWARD_TYPEDEF(ray_type)
MA_FORWARD_TYPEDEF(ScalarType)
MA_FORWARD_TYPEDEF(intersection_t )
MA_FORWARD_TYPEDEF(transform_t)
MA_FORWARD_TYPEDEF(bsdf_ptr)
MA_FORWARD_TYPEDEF(differential_geometry_t)
MA_INTERFACE_END

    template<typename Conf>
struct MAGeometryPrimitive:MAPrimitive<typename Conf::interface_config/*PrimitiveConfigure<typename Conf::ScalarType,Conf::Dimension> */>
    {
        typedef typename Conf::shape_t shape_type;
        typedef typename Conf::refined_primitive_type refined_primitive_type;
		typedef typename Conf::refined_shape_t refined_shape_t;
		typedef typename Conf::refined_shape_ref_t refined_shape_ref_t;

		typedef typename Conf::const_shape_ref_t const_shape_ref_t;
        typedef typename Conf::material_t material_type;

        typedef typename Conf::const_material_ref_t const_material_ref_t;

		typedef typename Conf::bsdf_ptr bsdf_ptr;
		typedef typename Conf::differential_geometry_t differential_geometry_t;
		typedef typename Conf::transform_t transform_t;

        typedef MAPrimitive<typename Conf::interface_config/*PrimitiveConfigure<typename Conf::ScalarType,Conf::Dimension>*/ > parent_type;


MA_FORWARD_TYPEDEF(shared_primitive)
MA_FORWARD_TYPEDEF(shared_primitive_array)
MA_FORWARD_TYPEDEF(ray_type)
MA_FORWARD_TYPEDEF(ScalarType)
MA_FORWARD_TYPEDEF(intersection_t )
MA_FORWARD_TYPEDEF(virtual_table_type )
private:
        static const typename parent_type::virtual_table_type primitive_vtable;
private:
        typedef parent_type interface_type;
        typedef MAGeometryPrimitive<Conf> self_type;

        const_shape_ref_t shape_;
        const_material_ref_t material_;

        static self_type& self(interface_type& x)
        {
            return static_cast<self_type&>(x);
        }
        static const self_type& self(const interface_type& x)
        {
            return static_cast<const self_type&>(x);
        }

        static void destruct(/*const*/ interface_type& x)
        {
            return self(x).~MAGeometryPrimitive();
        }
        static void swap(interface_type& x,interface_type& t)
        {
			MA_ASSERT(false);
        }
        static void refine(const interface_type& x,shared_primitive_array& refined)
        {
            //typedef typename refined_primitive_type::shape_type refined_shape_type;
			//change smart-ptr to be raw ptr and manage it properly
            std::vector<refined_shape_ref_t> refined_shapes;
            refine_shape(*self(x).shape_,refined_shapes);
			for (size_t i = 0;i < refined_shapes.size();++i)
			{
				shared_primitive p (new refined_primitive_type(refined_shapes[i],self(x).material_));
				refined.push_back(p);
			}

           // MA_ASSERT(false);
        }
        static typename interface_type::BBox worldBound(const interface_type& x)
        {
            return self(x).shape_->worldBound();
        }
        static bool  canIntersect (const interface_type& x)
        {
            return self(x).shape_->canIntersect();
        }
        static bool  intersect (const interface_type& x,const ray_type& r,intersection_t& isect)
        {
            ScalarType thit;
            if (!self(x).shape_->intersect(r,thit,isect.dg))return false;
            isect.primitive = const_cast<const self_type*>(&self(x));
            isect.world2obj = self(x).shape_->worldToObj();
            r.maxt = thit;
            return true;
        }
        static bool intersectP (const interface_type& x, const ray_type& r)
        {
            return self(x).shape_->intersectP(r);
        }
		static bsdf_ptr getBSDF(const interface_type& x,const differential_geometry_t &dg,
			const transform_t &WorldToObject)
		{
			return self(x).getBSDFImpl(dg,WorldToObject);
		}
		bsdf_ptr getBSDFImpl(const differential_geometry_t &dg,
		const transform_t &WorldToObject) const
			{
				differential_geometry_t dgs;
				shape_->getShadingGeometry(WorldToObject.inverse(),
					dg, dgs);
				return material_->getBSDF(dg, dgs);
			}
public:
        MAGeometryPrimitive(const_shape_ref_t s,const_material_ref_t m)
                :parent_type(primitive_vtable),shape_(s),material_(m){};



    };

    template <typename Conf>
    const typename MAGeometryPrimitive<Conf>::virtual_table_type
    MAGeometryPrimitive<Conf>::primitive_vtable =
    {
        &MAGeometryPrimitive<Conf>::destruct,
        &MAGeometryPrimitive<Conf>::swap,
        &MAGeometryPrimitive<Conf>::worldBound,
        &MAGeometryPrimitive<Conf>::canIntersect,
        &MAGeometryPrimitive<Conf>::intersect,
        &MAGeometryPrimitive<Conf>::intersectP,
        &MAGeometryPrimitive<Conf>::refine,
		&MAGeometryPrimitive<Conf>::getBSDF
    };


    template<typename Conf>
    struct MAAggregate:public MAPrimitive<typename Conf::interface_config >
    {
        protected:
        typedef MAPrimitive<typename Conf::interface_config > parent_type;

        typedef parent_type interface_type;
        typedef typename interface_type::virtual_table_type virtual_table_type;
        MAAggregate(const virtual_table_type& v):interface_type(v){}
        ~MAAggregate(){}

MA_FORWARD_TYPEDEF(shared_primitive)
MA_FORWARD_TYPEDEF(shared_primitive_array)
MA_FORWARD_TYPEDEF(ray_type)
MA_FORWARD_TYPEDEF(ScalarType)
MA_FORWARD_TYPEDEF(intersection_t )
    };

////template<typename S,int D>
////details::ma_primitive_interface<S,D>* createAccel(const std::vector<details::ma_primitive_interface<S,D>::shared_primitive >& ps,const ParameterType& p)
////{}

}
#endif // MAPRIMITIVE_HPP_INCLUDED
