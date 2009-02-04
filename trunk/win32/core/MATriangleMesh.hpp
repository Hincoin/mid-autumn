#ifndef _INCLUDED_MA_TRIANGLE_MESH_HPP_
#define _INCLUDED_MA_TRIANGLE_MESH_HPP_

#include "Shape.hpp"
#include <vector>

namespace ma
{
    struct cfg_not_intersectable
    {
        static const bool CanIntersect = false;
    };
    struct cfg_intersectable
    {
        static const bool CanIntersect = false;
    };
    struct cfg_uv
    {
        static const int uv_dimension = 2;    /// 2 dimension uv
    }
    struct default_tri_mesh_cfg:default_config_3d,cfg_not_intersectable,cfg_uv{ };

    template<typename Cfg = default_tri_mesh_cfg>
    struct MATriangleMesh:public Shape<MATriangleMesh<Cfg>,Cfg >
    {
        friend class Shape<MATriangleMesh<Cfg>,Cfg>;

        static const int uv_dimension = Cfg::uv_dimension;
        typedef Shape<MATriangleMesh<Cfg>,Cfg> parent_type;
        typedef typename parent_type::VectorType NormalType;
        typedef typename parent_type::VectorType VectorType;
        typedef typename parent_type::Transform Transform;
        typedef Point<VectorType> PointType;
        typedef typename parent_type::ScalarType ScalarType;
        typedef typename parent_type::BBox BBox;
        typedef ScalarType uv_type;
        typedef typename Cfg::index_type index_type;
        typedef std::vector<index_type> index_array;
        typedef std::vector<PointType> point_array;

        MATriangleMesh(const Transform& obj2world,bool reverse_normal,
                       const index_array& indices,const point_array& p,
                       const NormalType* normals,const VectorType* tangent,
                       const uv_type* uv):parent_type(obj2world,reverse_normal),
                vertex_indices_(indices),normals_(0),tangents_(0),uvs_(0)
        {
            points_.resize(p.size());
            for (index_type i = 0 ;i < p.size(); ++i )
            {
                points_[i] = obj2world * p[i];
            }
            if (normals)
            {
                normals_ = new NormalType[points_.size()];
                memcpy(normals_,normals,sizeof(NormalType)*points_.size());
            }
            if (tangent)
            {
                tangents_  = new VectorType[points_.size()];
                memcpy(tangents_,tangent,sizeof(VectorType) * points_.size());
            }
            if (uv)
            {
                uvs_ = new uv_type[uv_dimension * points_.size()];
                memcpy(uvs_,uv,sizeof(uv_type)* uv_dimension * points_.size());
            }
        }
        //override
        BBox worldBound()const
        {
            ///
            BBox bbox;
            for (point_array::iterator it = points_.begin(); it != points_.end(); ++it)
            {
                bbox = (space_union(bbox,  (*it)));
            }
            return bbox;
        }
        ///
        void swap(MATriangleMesh& other)
        {
            //
        }
        ~MATriangleMesh()
        {
            delete [] normals_;
            delete [] tangents_;
            delete [] uvs_;
        }
        const index_array& indexArray()const
        {
            return vertex_indices_;
        }
        index_array& indexArray()
        {
            return vertex_indices_;
        }

        const point_array& pointArray()const
        {
            return points_;
        }
        point_array& pointArray()
        {
            return points_;
        }

        const uv_type* UVs()const
        {
            return uvs_;
        }
        const NormalType* normals()const{return normals_;}
        const VectorType* tangents()const{return tangents;}
private:
        BBox objectBoundImpl()const
        {
            ///
            BBox bbox;
            for (point_array::iterator it = points_.begin(); it != points_.end(); ++it)
            {
                bbox.swap(space_union(bbox, world_to_obj_ * (*it)));
            }
        }
        template<typename Tri>
        void refineImpl(std::vector<Tri >& refined)const
        {
            BOOST_STATIC_ASSERT(boost::is_same<typename Tri::index_type,index_type>::value);
            index_type nTris = vertex_indices_.size()/3;
            for (index_type i = 0;i < nTris; ++i)
            {
                refined.push_back(Tri(object_to_world_,reverse_normal_,*this,i));
            }
        }

        //////////////////////////////////////////////////////////////////////////

        index_array vertex_indices_;
        point_array points_;
        NormalType* normals_;//one per vertex
        VectorType* tangents_;
        uv_type* uvs_;

    };
    //reimplement
    struct default_tri_cfg:default_config_3d,cfg_intersectable,cfg_uv
    {
        typedef MATriangleMesh<> MeshType;
        template<typename SHAPE>
        static void getShadingGeometry(const SHAPE& s,const typename ShapeTraits<SHAPE>::Transform&  obj2world,
                                       const typename ShapeTraits<SHAPE>::differential_geometry& dg,
                                       typename ShapeTraits<SHAPE>::differential_geometry& dgShading)
        {
            MeshType& mesh = s.mesh_;
            typename SHAPE::index_type* v = s.v_;
            if (!mesh.normals() && !mesh.tangents()){
                dgShading = dg;return;
                }
                ///
        }

        template<typename SHAPE>
        static ShapeTraits<SHAPE>::ScalarType area(const SHAPE& s)
        {
            const PointType& p0 = s.mesh_.pointArray()[v_[0]];
            const PointType& p1 = s.mesh_.pointArray()[v_[1]];
            const PointType& p2 = s.mesh_.pointArray()[v_[2]];

            return length(cross(p2 - p1,p3-p1)) * 0.5f;
        }

        static ShapeTraits<SHAPE>::BBox worldBound(const SHAPE& s)
        {
            const PointType& p0 = s.mesh_.pointArray()[v_[0]];
            const PointType& p1 = s.mesh_.pointArray()[v_[1]];
            const PointType& p2 = s.mesh_.pointArray()[v_[2]];

            return space_union(space_union( p0,p1),p2);
        }
    };

    template<typename CFG = default_tri_cfg>
    struct MATriangle:Shape<MATriangle<CFG>,CFG>
    {
        typedef Shape<MATriangle<CFG>,CFG> parent_type;
        typedef MATriangle<CFG> self_type;
        typedef CFG Configure;
        friend struct Shape<MATriangle<CFG>,CFG>;
        friend class CFG;

        static const int uv_dimension = CFG::uv_dimension;
        typedef typename parent_type::VectorType NormalType;
        typedef typename parent_type::VectorType VectorType;
        typedef typename parent_type::Transform Transform;
        typedef Point<VectorType> PointType;
        typedef Ray<VectorType> ray;
        typedef typename parent_type::ScalarType ScalarType;
        typedef typename parent_type::BBox BBox;
        typedef ScalarType uv_type;
        typedef typename CFG::index_type index_type;
        typedef typename CFG::MeshType MeshType;
        ///
        MATriangle(const Transform& o2w,bool rev_n,MeshType& mesh,index_type idx):parent_type(o2w,rev_n),
                mesh_(mesh)
        {
            v_ = & mesh.indexArray()[idx * 3];
        }

private:
        BBox objectBoundImpl()
        {
            const PointType& p0 = mesh_.pointArray()[v_[0]];
            const PointType& p1 = mesh_.pointArray()[v_[1]];
            const PointType& p2 = mesh_.pointArray()[v_[2]];
            return space_union(space_union(world_to_obj_* p0,world_to_obj_ * p1),world_to_obj_ * p2);
        }
        bool intersectImpl(const ray& r, ScalarType& tHit,
                           differential_geometry& dg)const
        {
            const PointType& p0 = mesh_.pointArray()[v_[0]];
            const PointType& p1 = mesh_.pointArray()[v_[1]];
            const PointType& p2 = mesh_.pointArray()[v_[2]];
            VectorType e0 = p1 - p0;
            VectorType e1 = p2 - p0;
            VectorType s1 = cross(r.dir,e1);
            ScalarType divisor = dot(s1,e1);
            if (divisor == 0)return false;
            ScalarType inv_divisor = reciprocal(divisor);

            VectorType d = ray.o - p0;
            ScalarType b1 = dot(d,s1) * inv_divisor;
            if (b1 < 0 || b1 > 1) return false;

            VectorType s2 = cross(d,e0);
            ScalarType b2 = dot(ray.dir, s2) * inv_divisor;
            if (b2 < 0 || b1 + b2 > 1) return false;

            ScalarType t = dot(e1,s2) * inv_divisor;
            if (t < ray.mint || t > ray.maxt)return false;

/// compute partial derivatives
            VectorType dpdu,dpdv;
            ScalarType uvs[3][2];
            getUVs(uvs);

/// compute deltas for triangle partial derivatives
            ScalarType du1 = uvs[0][0] - uvs[2][0];
            ScalarType du2 = uvs[1][0] - uvs[2][0];
            ScalarType dv1 = uvs[0][1] - uvs[2][1];
            ScalarType dv2 = uvs[1][1] - uvs[2][1];
            VectorType dp1 = p1 - p3,dp2 = p2 - p3;

            ScalarType det = du1 * dv2 - dv1 * du2;
            if (det == 0)
            {
                coordinate_system(nomalize(cross(e1,e0)),dpdu,dpdv);

            }
            else
            {
                ScalarType inv_det = reciprocal(det);
                dpdu  = (dv2 * dp1 - dv1 * dp2) * inv_det;
                dpdv = (-du2 * dp1 + du1 * dp2) * inv_det;
            }
///interpolate (u,v )
            ScalarType b0 = 1- b1 - b2;
            ScalarType tu = b0*uvs[0][0] + b1*uvs[1][0] + b2*uvs[2][0];
            ScalarType tv = b0*uvs[0][1] + b1 * uvs[1][1]+ b2*uvs[2][1];
            dg = differential_geometry(r(t), dpdu,dpdv,VectorType(0,0,0),
                                       VectorType(0,0,0),tu,tv,this);
        }
        void getUVs(ScalarType uvs[3][2])const
        {
            if (mesh_.UVs())
            {
                uvs[0][0] = mesh_.UVs()[2*v_[0]];
                uvs[0][1] = mesh_.UVs()[2*v_[0]+1];

                uvs[1][0] = mesh_.UVs()[2*v_[1]];
                uvs[1][1] = mesh_.UVs()[2*v_[1]+1];

                uvs[2][0] = mesh_.UVs()[2*v_[2]];
                uvs[2][1] = mesh_.UVs()[2*v_[2]+1];
            }
            else
            {
                uvs[0][0] = 0;
                uvs[0][1] = 0;

                uvs[1][0] = 1;
                uvs[1][1] = 0;

                uvs[2][0] = 1;
                uvs[2][1] = 1;
            }
        }
private:
        MeshType& mesh_;
        index_type* v_;
    };
}
#endif
