#ifndef _INCLUDED_MA_TRIANGLE_MESH_HPP_
#define _INCLUDED_MA_TRIANGLE_MESH_HPP_

#include "Shape.hpp"
#include <vector>

namespace ma{
	struct default_config_3d{
		static const int Dimesion = 3;
		typedef float ScalarType;
	};

	template<typename Cfg = default_config_3d>
	struct MATriangleMesh:public Shape<MATriangleMesh<Cfg>,Cfg >{
		friend class Shape<MATriangleMesh<Cfg>,Cfg>;
		typedef Shape<MATriangleMesh<Cfg>,Cfg> parent_type;
		typedef typename parent_type::VectorType NormalType;
		typedef typename parent_type::VectorType VectorType;
		typedef typename parent_type::Transform Transform;
		typedef Point<VectorType> PointType;
		typedef typename parent_type::ScalarType ScalarType;
		typedef typename parent_type::BBox BBox;
		typedef ScalarType uv_type;
		typedef unsigned index_type;
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
			if (normals){
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
				uvs_ = new uv_type[2 * points_.size()];
				memcpy(uvs_,uv,sizeof(uv_type)*2 * points_.size());
			}
		}
		//override
		BBox worldBound()const{
			///
			BBox bbox;
			for (point_array::iterator it = points_.begin(); it != points_.end(); ++it)
			{
				bbox = (space_union(bbox,  (*it)));
			}
			return bbox;
		}
		bool canIntersect()const{
			return false;
		}
		///
		void swap(MATriangleMesh& other){
			//
		}
		~MATriangleMesh(){
			delete [] normals_;
			delete [] tangents_;
			delete [] uvs_;
		}
	private:
		BBox objectBoundImpl()const{
			///
			BBox bbox;
			for (point_array::iterator it = points_.begin(); it != points_.end(); ++it)
			{
				bbox.swap(space_union(bbox, world_to_obj_ * (*it)));
			}
		}
		template<typename Tri,typename Confg>
		void refineImpl(std::vector<Shape<Tri,Confg> >& refined)const{}

		//////////////////////////////////////////////////////////////////////////

		index_array vertex_indices_;
		point_array points_;
		NormalType* normals_;//one per vertex
		VectorType* tangents_;
		uv_type* uvs_;

	};
}
#endif