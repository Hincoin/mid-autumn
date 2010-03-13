#ifndef _INCLUDED_MA_TRIANGLE_MESH_HPP_
#define _INCLUDED_MA_TRIANGLE_MESH_HPP_

#include "Shape.hpp"
#include <vector>
#include "ErrorReporter.hpp"
namespace ma
{
    struct cfg_not_intersectable
    {
        static const bool CanIntersect = false;
    };
    struct cfg_intersectable
    {
        static const bool CanIntersect = true;
    };
    struct cfg_uv
    {
        static const int uv_dimension = 2;    /// 2 dimension uv
    };
    struct default_tri_mesh_cfg:default_config_3d,cfg_not_intersectable,cfg_uv{ };

    template<typename Cfg = default_tri_mesh_cfg>
    struct MATriangleMesh:public Shape<MATriangleMesh<Cfg>,Cfg >
    {
        friend class Shape<MATriangleMesh<Cfg>,Cfg>;

		typedef Cfg config_t;

        static const int uv_dimension = Cfg::uv_dimension;
        typedef Shape<MATriangleMesh<Cfg>,Cfg> parent_type;
        typedef typename parent_type::normal_t NormalType;
        //typedef typename parent_type::vector_t vector_t;
		typedef typename parent_type::vector_t vector_t;
		typedef typename parent_type::transform_t transform_t;
		typedef Point<vector_t> point_t;
		typedef Ray<vector_t>  ray_t;
		typedef typename parent_type::ScalarType scalar_t;

        typedef Point<vector_t> PointType;
        typedef typename parent_type::ScalarType ScalarType;
        typedef typename parent_type::BBox BBox;
        typedef ScalarType uv_type;
        typedef typename Cfg::index_type index_type;
        typedef std::vector<index_type> index_array;
        typedef std::vector<PointType> point_array;

        MATriangleMesh(const transform_t& obj2world,bool reverse_normal,
                       const index_array& indices,const point_array& p,
                       const NormalType* normals,const vector_t* tangent,
                       const uv_type* uv):parent_type(obj2world,reverse_normal),
                vertex_indices_(indices),normals_(0),tangents_(0),uvs_(0)
        {
            points_.resize(p.size());
            for (index_type i = 0 ;i < p.size(); ++i )
            {
                points_[i].p = obj2world * p[i].p;
            }
            if (normals)
            {
                normals_ = new NormalType[points_.size()];
                memcpy(normals_,normals,sizeof(NormalType)*points_.size());
            }
            if (tangent)
            {
                tangents_  = new vector_t[points_.size()];
                memcpy(tangents_,tangent,sizeof(vector_t) * points_.size());
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
            for (typename point_array::const_iterator it = points_.begin(); it != points_.end(); ++it)
            {
                bbox = (space_union(bbox,  (*it)));
            }
            return bbox;
        }
		bool intersectPImpl(const ray_t& )const{MA_ASSERT(false);return false;}
		bool intersectImpl(const ray_t& ,ScalarType& ,typename parent_type::differential_geometry& )const{
			MA_ASSERT(false);
			return false;
		}
        ///
        void swap(MATriangleMesh& other)
        {
            MA_ASSERT(false);
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
        const vector_t* tangents()const{return tangents_;}
private:
        BBox objectBoundImpl()const
        {
            ///
            BBox bbox;
            for (typename point_array::iterator it = points_.begin(); it != points_.end(); ++it)
            {
                bbox.swap(space_union(bbox, parent_type::world_to_obj_ * (*it)));
            }
        }
        template<typename Tri_Ref>
        void refineImpl(std::vector<Tri_Ref >& refined)const
        {
			typedef typename ptr_traits<Tri_Ref>::value_type Tri;
            BOOST_STATIC_ASSERT((boost::is_same<typename Tri::index_type,index_type>::value));
            size_t nTris = vertex_indices_.size()/3;
            for (size_t i = 0;i < nTris; ++i)
            {
				//todo : Tri_Ref should be a raw ptr
                refined.push_back(Tri_Ref(new Tri(parent_type::obj_to_world_,parent_type::reverse_normal_,*this,i)));
            }
        }

        //////////////////////////////////////////////////////////////////////////

        index_array vertex_indices_;
        point_array points_;
        NormalType* normals_;//one per vertex
        vector_t* tangents_;
        uv_type* uvs_;

    };
    //reimplement
    struct default_tri_cfg:default_config_3d,cfg_intersectable,cfg_uv
    {
        typedef MATriangleMesh<> MeshType;
        template<typename SHAPE>
        static void getShadingGeometry(const SHAPE& s,const typename ShapeTraits<SHAPE>::transform_t&  obj2world,
                                       const typename ShapeTraits<SHAPE>::differential_geometry& dg,
                                       typename ShapeTraits<SHAPE>::differential_geometry& dgShading)
        {
            const MeshType* mesh = s.mesh_;
            const typename SHAPE::index_type* v = s.v_;
			typedef typename SHAPE::vector_t vector_t;
			typedef typename SHAPE::NormalType NormalType;
			typedef ScalarType scalar_t;
            if (!mesh->normals() && !mesh->tangents()){
                dgShading = dg;return;
                }
                ///
                ScalarType uv[3][2];
                s.getUVs(uv);
                typename matrix_type<ScalarType,2,2>::type A ;
                typename vector_type<ScalarType,2>::type C(dg.u - uv[0][0],dg.v - uv[0][1]),B;
                ScalarType b;
                A<<(uv[1][0]-uv[0][0]),(uv[2][0]-uv[0][0]),(uv[1][1] - uv[0][1]),(uv[2][1] - uv[0][1]);
                if (std::abs(A.determinant()) < std::numeric_limits<ScalarType>::epsilon() )
                b=B[0]=B[1] = ScalarType(1)/ScalarType(3);
                else B = A.inverse() * C;
                NormalType ns;
                vector_t ss,ts;
                if (mesh->normals()) ns = (b * mesh->normals()[v[0]] +
                                                B[0] * mesh->normals()[v[1]] +
                                                B[1] * mesh->normals()[v[2]]).normalized();
                else ns = dg.normal;
                if (mesh->tangents()) ss = (b * mesh->tangents()[v[0]] +
                B[0] * mesh->tangents()[v[1]] + B[1] * mesh->tangents()[v[2]]).normalized();
                else ss = normalize(dg.dpdu);
                ts = ss.cross(ns);// cross(ss,ns);
                ss = ts.cross(ns);//cross(ts,ns);
                ts = (obj2world * ts).normalized();
                ss = (obj2world * ss).normalized();
//////////////////////////////////////////////////////////////////////////
				vector_t dndu, dndv;
				// Compute \dndu and \dndv for triangle shading geometry
				scalar_t uvs[3][2];
				s.getUVs(uvs);
				// Compute deltas for triangle partial derivatives of normal
				scalar_t du1 = uvs[0][0] - uvs[2][0];
				scalar_t du2 = uvs[1][0] - uvs[2][0];
				scalar_t dv1 = uvs[0][1] - uvs[2][1];
				scalar_t dv2 = uvs[1][1] - uvs[2][1];
				vector_t dn1 = vector_t(mesh->normals()[v[0]] - mesh->normals()[v[2]]);
				vector_t dn2 = vector_t(mesh->normals()[v[1]] - mesh->normals()[v[2]]);
				scalar_t determinant = du1 * dv2 - dv1 * du2;
				if (determinant == 0)
					dndu = dndv = vector_t(0,0,0);
				else {
					scalar_t invdet = reciprocal( determinant );
					dndu = ( dv2 * dn1 - dv1 * dn2) * invdet;
					dndv = (-du2 * dn1 + du1 * dn2) * invdet;
				}
				//////////////////////////////////////////////////////////////////////////

                dgShading =  typename ShapeTraits<SHAPE>::differential_geometry(dg.point,ss,ts,
                dndu,dndv,dg.u,dg.v,&s);
				dgShading.dudx = dg.dudx;  dgShading.dvdx = dg.dvdx;
				dgShading.dudy = dg.dudy;  dgShading.dvdy = dg.dvdy;
				dgShading.dpdx = dg.dpdx;  dgShading.dpdy = dg.dpdy;
        }

        template<typename SHAPE>
        static typename ShapeTraits<SHAPE>::ScalarType area(const SHAPE& s)
        {
			typedef typename SHAPE::point_t point_t;
			const typename SHAPE::index_type* v = s.v_;
            const point_t& p0 = s.mesh_->pointArray()[v[0]];
            const point_t& p1 = s.mesh_->pointArray()[v[1]];
            const point_t& p2 = s.mesh_->pointArray()[v[2]];

            return length(cross(p1 - p0,p2-p0)) * 0.5f;
        }
		template<typename SHAPE>
        static typename ShapeTraits<SHAPE>::BBox worldBound(const SHAPE& s)
        {
			typedef typename SHAPE::point_t point_t;
			const typename SHAPE::index_type* v = s.v_;
            const point_t& p0 = s.mesh_->pointArray()[v[0]];
            const point_t& p1 = s.mesh_->pointArray()[v[1]];
            const point_t& p2 = s.mesh_->pointArray()[v[2]];

            return space_union(space_union( p0,p1),p2);
        }
    };

    template<typename CFG = default_tri_cfg>
    struct MATriangle:Shape<MATriangle<CFG>,CFG>
    {
        typedef Shape<MATriangle<CFG>,CFG> parent_type;
        typedef MATriangle<CFG> self_type;
		typedef self_type shape_type;
        typedef CFG Configure;
        friend class Shape<MATriangle<CFG>,CFG>;
        //friend class CFG;

        static const int uv_dimension = CFG::uv_dimension;
        typedef typename parent_type::normal_t NormalType;
		typedef typename parent_type::vector_t vector_t;
		typedef typename parent_type::normal_t normal_t;
        typedef typename parent_type::transform_t transform_t;
		typedef Point<vector_t> point_t;

        typedef Point<vector_t> PointType;
        typedef Ray<vector_t> ray;
		typedef ray ray_t;
        typedef typename parent_type::ScalarType ScalarType;
        typedef typename parent_type::BBox BBox;
        typedef ScalarType uv_type;
        typedef typename CFG::index_type index_type;
        typedef typename CFG::MeshType MeshType;
        ///
        MATriangle(const transform_t& o2w,bool rev_n,const MeshType& mesh,index_type idx):parent_type(o2w,rev_n),
                mesh_(&mesh)
        {
            v_ = & mesh.indexArray()[idx * 3];
        }
		MATriangle& operator=(MATriangle rhs)
		{swap(rhs);return *this;}
		void swap(MATriangle& rhs){
			const MeshType* tmp = this->mesh_;
			this->mesh_ = rhs.mesh_;
			rhs.mesh_ = tmp;
			const index_type* tmpi = this->v_;
			this->v_ = rhs.v_;
			rhs.v_ = tmpi;
		}
private:
		template<typename S>
		void refineImpl(std::vector<S >& refined)const{
			MA_ASSERT(false);
		}
        BBox objectBoundImpl()
        {
            const PointType& p0 = mesh_->pointArray()[v_[0]];
            const PointType& p1 = mesh_->pointArray()[v_[1]];
            const PointType& p2 = mesh_->pointArray()[v_[2]];
            return space_union(space_union(parent_type::world_to_obj_* p0,parent_type::world_to_obj_ * p1),parent_type::world_to_obj_ * p2);
        }
        bool intersectImpl(const ray& r, ScalarType& tHit,
                           typename parent_type::differential_geometry& dg)const
        {
            const PointType& p0 = mesh_->pointArray()[v_[0]];
            const PointType& p1 = mesh_->pointArray()[v_[1]];
            const PointType& p2 = mesh_->pointArray()[v_[2]];
            vector_t e0 = p1 - p0;
            vector_t e1 = p2 - p0;
            vector_t s1 = cross(r.dir,e1);
            ScalarType divisor = dot(s1,e0);
            if (divisor == 0)return false;
            ScalarType inv_divisor = reciprocal(divisor);

            vector_t d = r.o - p0;
            ScalarType b1 = dot(d,s1) * inv_divisor;
            if (b1 < 0 || b1 > 1) return false;

            vector_t s2 = cross(d,e0);
            ScalarType b2 = dot(r.dir, s2) * inv_divisor;
            if (b2 < 0 || b1 + b2 > 1) return false;

            ScalarType t = dot(e1,s2) * inv_divisor;
            if (t < r.mint || t > r.maxt)return false;

/// compute partial derivatives
            vector_t dpdu,dpdv;
            ScalarType uvs[3][2];
            getUVs(uvs);

/// compute deltas for triangle partial derivatives
            ScalarType du1 = uvs[0][0] - uvs[2][0];
            ScalarType du2 = uvs[1][0] - uvs[2][0];
            ScalarType dv1 = uvs[0][1] - uvs[2][1];
            ScalarType dv2 = uvs[1][1] - uvs[2][1];
            vector_t dp1 = p0 - p2,dp2 = p1 - p2;

            ScalarType det = du1 * dv2 - dv1 * du2;
            if (det == 0)
            {
                coordinate_system<vector_t>((cross(e1,e0).normalized()),dpdu,dpdv);

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
            dg = typename parent_type::differential_geometry(r(t), dpdu,dpdv,vector_t(0,0,0),
                                       vector_t(0,0,0),tu,tv,this);
			tHit = t;
			return true;
        }
		bool intersectPImpl(const ray &r) const {
			// Update triangle tests count
			// Compute $\VEC{s}_1$
			// Get triangle vertices in _p1_, _p2_, and _p3_
			const point_t &p1 = mesh_->pointArray()[v_[0]];
			const point_t &p2 = mesh_->pointArray()[v_[1]];
			const point_t &p3 = mesh_->pointArray()[v_[2]];
			vector_t e1 = p2 - p1;
			vector_t e2 = p3 - p1;
			vector_t s1 = cross(r.dir, e2);
			typedef ScalarType scalar_t;
			scalar_t divisor = dot(s1, e1);
			if (divisor == 0.)
				return false;
			scalar_t invDivisor = 1.f / divisor;
			// Compute first barycentric coordinate
			vector_t d = r.o - p1;
			scalar_t b1 = dot(d, s1) * invDivisor;
			if (b1 < 0. || b1 > 1.)
				return false;
			// Compute second barycentric coordinate
			vector_t s2 = cross(d, e1);
			scalar_t b2 = dot(r.dir, s2) * invDivisor;
			if (b2 < 0. || b1 + b2 > 1.)
				return false;
			// Compute _t_ to intersection point
			scalar_t t = dot(e2, s2) * invDivisor;
			if (t < r.mint || t > r.maxt)
				return false;

			return true;
		}
public:
        void getUVs(ScalarType uvs[3][2])const
        {
            if (mesh_->UVs())
            {
                uvs[0][0] = mesh_->UVs()[2*v_[0]];
                uvs[0][1] = mesh_->UVs()[2*v_[0]+1];

                uvs[1][0] = mesh_->UVs()[2*v_[1]];
                uvs[1][1] = mesh_->UVs()[2*v_[1]+1];

                uvs[2][0] = mesh_->UVs()[2*v_[2]];
                uvs[2][1] = mesh_->UVs()[2*v_[2]+1];
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

        const MeshType* mesh_;
        const index_type* v_;
    };
}


namespace ma{
	namespace details{
		template<typename CFG>
		struct shape_creator<MATriangleMesh<CFG> >{
			MATriangleMesh<CFG>*
				operator()(const typename MATriangleMesh<CFG>::transform_t &o2w,
				bool reverseOrientation, const ParamSet &params)const
			{
				size_t nvi=0, npi=0, nuvi=0, nsi=0, nni=0;
				typedef MATriangleMesh<CFG> mesh_t;
				typedef typename mesh_t::point_t point_t;
				typedef typename mesh_t::vector_t vector_t;
				typedef typename mesh_t::scalar_t scalar_t;
				typedef typename mesh_t::normal_t normal_t;
				typedef mesh_t triangle_mesh;
				//const int *vi = params.as<int*>("indices", &nvi);
				//const point_t *P = params.as<point_t*>("P", &npi);
				//const float *uvs = params.as<scalar_t*>("uv", &nuvi);
				//if (!uvs) uvs = params.as<scalar_t*>("st", &nuvi);
				using std::vector;
				std::vector<unsigned>  vi = params.as< std::vector<unsigned> >("indices");
				 vector<point_t>  P = params.as< vector<point_t> >("P");
				vector<scalar_t>  uvs = params.as<vector<scalar_t> >("uv");
				if(uvs.empty()) uvs = params.as<vector<scalar_t> >("st");
				// XXX should complain if uvs aren't an array of 2...
				if (vi.empty() || P.empty()) return NULL;
				//const vector_t *S = params.as<vector_t*>("S", &nsi);
				 vector<vector_t>  S = params.as< vector<vector_t> >("S");
				nvi = vi.size();
				npi = P.size();
				nuvi = uvs.size();
				nsi = S.size();
				if (!S.empty() && nsi != npi) {
					//Error("Number of \"S\"s for triangle mesh must match \"P\"s");
					//S = NULL;
					S.clear();
				}
				//const normal_t *N = params.as<normal_t*>("N", &nni)
				 vector<normal_t> N = params.as< vector<normal_t> >("N");
				nni = N.size();
				if (!N.empty() && nni != npi) {
					//Error("Number of \"N\"s for triangle mesh must match \"P\"s");
					//N = NULL;
					N.clear();
				}
				if (!uvs.empty() && !N.empty()) {
					// if there are normals, check for bad uv's that
					// give degenerate mappings; discard them if so
					const unsigned *vp = &vi[0];
					for (unsigned i = 0; i < nvi; i += 3, vp += 3) {
						scalar_t area = .5f * cross(P[vi[0]]-P[vi[1]], P[vi[2]]-P[vi[1]]).norm();
						if (area < 1e-7) continue; // ignore degenerate tris.
						if ((uvs[2*vi[0]] == uvs[2*vi[1]] &&
							uvs[2*vi[0]+1] == uvs[2*vi[1]+1]) ||
							(uvs[2*vi[1]] == uvs[2*vi[2]] &&
							uvs[2*vi[1]+1] == uvs[2*vi[2]+1]) ||
							(uvs[2*vi[2]] == uvs[2*vi[0]] &&
							uvs[2*vi[2]+1] == uvs[2*vi[0]+1])) {
								//Warning("Degenerate uv coordinates in triangle mesh.  Discarding all uvs.");
								//uvs = NULL;
								uvs.clear();
								break;
						}
					}
				}
				for (unsigned i = 0; i < nvi; ++i)
					if (vi[i] >= npi) {
						//Error("trianglemesh has out of-bounds vertex index %d (%d \"P\" values were given",
						//	vi[i], npi);
						report_warning("trianglemesh has out of-bounds vertex index %d (%d \"P\" values were given",
								vi[i], npi);
						return NULL;
					}
					return new triangle_mesh(o2w, reverseOrientation, /*nvi/3, npi,*/ vi, P,
						N.empty()? 0:&N[0],S.empty()? 0: &S[0],uvs.empty()?0: &uvs[0]);
			}

		};
	}
}
#endif
