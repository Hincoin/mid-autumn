#ifndef __INCLUDE_MA_MESH_HPP__
#define __INCLUDE_MA_MESH_HPP__

#include <vector>
#include <list>
#include <cassert>

#include "Vector.hpp"

#include "MAVertex.hpp"
#include "MAPoly.hpp"

#include <boost/array.hpp>
#include <boost/mpl/if.hpp>
#include <boost/noncopyable.hpp>

#include "Move.hpp"

#include "MeshBuffer.hpp"
//#include <boost/enable_i>
namespace ma{

	//template<typename Vertex_Type,
	//	typename Face_Type,
	//	typename Topology_Type,
	//	typename Vertex_Container = std::vector<Vertex_Type>,
	//	typename Face_Container = std::vector<Face_Type>
	//>
	//struct MARenderMesh:private Topology_Type{
	//
	//	Vertex_Container vertices;
	//	Face_Container faces;
	//};

	////the default topology policy : 1 edge of a face at most connect to 1 face
	////for rendering
	//template<typename Vert_Type,typename Poly_Type,typename face_index_type = std::size_t,
	//	typename VarySize_FaceID_Container = std::list<face_index_type>
	//>
	//struct connect_face_by_edge{
	////	ma_traits::polygon_traits<Poly_Type>;
	////	ma_traits::vertex_traits<Vert_Type>;
	//	typedef face_index_type edge_index_type;
	//	static const face_index_type null_index = face_index_type(-1);
	//
	//	struct self_type:
	//		std::vector<typename boost::mpl::if_c<ma_traits::polygon_traits<Poly_Type>::is_fixed,
	//		boost::array<face_index_type,ma_traits::polygon_traits<Poly_Type>::vertex_count>,
	//		VarySize_FaceID_Container>::type>
	//	{};
	//	self_type self_;

	//	void add_face(face_index_type exist_faceid,edge_index_type which_edge,face_index_type face_id)
	//	{
	//		self_[exist_faceid][which_edge] = face_id;
	//	}
	//	void add_singular_face(){self_.push_back(self_type::value_type());}
	//};


	//render part refer to scene::IMeshBuffer
	//animation part refer to CSkinnedMesh
	namespace ma_traits{
			template<typename VertexType,typename FacesByIndex,typename MaterialType>
			struct MARenderMeshFixedTraits{
				typedef VertexType vertex_type;
				typedef FacesByIndex face_type;
				typedef MaterialType material_type;
			};

	}
	template<typename VertexType,typename FacesByIndex,typename MaterialType>
	struct MARenderMeshFixed/*:MeshBuffer<
		MARenderMeshFixed<VertexType,FacesByIndex,MaterialType>,
		ma_traits::MARenderMeshFixedTraits<VertexType,FacesByIndex,MaterialType> >*/{
		//tri-mesh or quad-mesh for storage efficiency
		typedef VertexType vertex_type;
		typedef MaterialType material_type;
		typedef FacesByIndex face_type;

		enum{vertex_per_face = FacesByIndex::vertex_count};
		typedef typename FacesByIndex::index_type index_type;

		struct SubMesh:MeshBuffer<SubMesh,
			ma_traits::MARenderMeshFixedTraits<VertexType,FacesByIndex,MaterialType> >,
			boost::noncopyable{
			typedef MaterialType material_type;
		private:
			std::vector<ma::vector3i> faces_indices_;// faces_indices in parent mesh
			index_type material_index_;
			//the topology is shared from parent mesh which can be accessed by faces_indices_

			template<typename VertexT,typename FacesByI,typename Material>
			friend struct MARenderMeshFixed;

			SubMesh(){}
			//cannot be copied and only can be constructed from parent mesh
			SubMesh(
				const std::vector<ma::vector3i>& faces_indices,
				index_type mat_idx
				):faces_indices_(faces_indices),material_index_(mat_idx){
			}
		public:
			SubMesh* clone()const{assert(false);}
			//access methods ...
		};
	private:
		std::vector<VertexType> verts_;

		// vertex_per_face vertices construct a face and it contains some structure as face normal
		std::vector<FacesByIndex> faces_;
		std::vector<MaterialType> materials_;

		std::vector<SubMesh> sub_meshes_;

		//the topology: adjacent list of faces
		typedef boost::array<index_type,vertex_per_face + 1> EdgeArray; //vertex_per_face == 3 can make this array aligned
		typedef std::vector<EdgeArray> TopologyMap;//map face index to its edges
		TopologyMap topology_; //fairly simple topology

		void buildTopology(const std::vector<VertexType>& v,const std::vector<FacesByIndex>& f){
			topology_.resize(f.size());//allocate space
			std::memset(&topology_[0],-1,sizeof(EdgeArray)*topology_.size());
			index_type l_idx,r_idx;
			for (std::size_t i = 0;i < f.size(); ++i)
			{
				if (topology_[i][vertex_per_face] != vertex_per_face + index_type(-1) )
				{
					for (std::size_t j = 0;j < i; ++j)
					{
						if(f[i].isAdjacent(f[j],l_idx,r_idx))
						{
							topology_[i][l_idx] = j;
							topology_[j][r_idx] = i;
							++topology_[i][vertex_per_face];
							++topology_[j][vertex_per_face];
						}
					}
					//skip i == j
					for (std::size_t j = i+1;j < f.size(); ++j)
					{
						if(f[i].isAdjacent(f[j],l_idx,r_idx))
						{
							topology_[i][l_idx] = j;
							topology_[j][r_idx] = i;
							++topology_[i][vertex_per_face];
							++topology_[j][vertex_per_face];
						}
					}
				}

			}
		}//building
	public:

		//construction
		MARenderMeshFixed(){}
		MARenderMeshFixed(
			const std::vector<VertexType>& verts,
			const std::vector<FacesByIndex>& faces,
			const std::vector<MaterialType>& mat
			):verts_(verts),faces_(faces),materials_(mat)
		{buildTopology(verts_,faces_);}

		//move from
		MARenderMeshFixed& operator = (ma::move_from<MARenderMeshFixed> other){
			//...
		}
		//
		void swap(MARenderMeshFixed& other){
			//...
		}

		//functions
		void setVertexArray(const std::vector<VertexType>& v){verts_=v; }
		void setVertexArray(ma::move_from<std::vector<VertexType> > v){verts_.swap(v);}
		void setFaceIndexArray(const std::vector<FacesByIndex>& f){faces_ = f;}
		void setFaceIndexArray(ma::move_from<std::vector<FacesByIndex> > f){faces_.swap(f);}

		void setMaterial(const std::vector<MaterialType>& mat){materials_ = mat;}

		void rebuildTopology(){buildTopology(verts_,faces_);}

		MARenderMeshFixed* clone()const{
			//...
		}

		//vertex_type& getVertex(index_type i){return verts_[i];}
		const vertex_type& getVertex(index_type i)const
		{
			return verts_[i];
		}
		const face_type& getFace(index_type i)const{return faces_[i];}

		const SubMesh& getIndexedSubMesh(index_type i)const{return sub_meshes_[i];}

		static void calculateFaceNormals(const std::vector<vertex_type>& v,std::vector<face_type>& faces){}
		static void calculateVertexNormals( std::vector<vertex_type>& v,const std::vector<face_type>& faces){}
	private:
	};

	template<typename NormalType>//other properties
	struct FaceProperty:NormalType{
		typedef NormalType normal_type;

		normal_type& normal(){return static_cast<normal_type&>(*this);}
		const normal_type& normal()const{return static_cast<const normal_type&>(*this);}
	};

	template<typename Face_Property = FaceProperty<vector3f> > //default to be a normal
	class TriFaceByIndex{
		vector3si indices_;
		Face_Property property_;//face property such as normal

		struct compare_other{
			typedef scalar_type<vector3si>::type index_type;
			static bool check(const vector3i& indices_,const TriFaceByIndex& other,index_type current_i,index_type current_j,
				index_type& l,index_type& r){
				static const int mod[]={0,1,2,0,1};
					if (indices_[mod[current_i+1]] == other.indices_[mod[current_j+1]])
					{
						l = current_i;
						r = current_j;
						return true;
					}
					else if (indices_[mod[current_i+1]] == other.indices_[mod[current_j+ 2]])
					{
						l = current_i;
						r = mod[current_j+2];
						return true;
					}
					else if (indices_[mod[current_i+2]] == other.indices_[mod[current_j+2]])
					{
						l = mod[current_i+2];
						r = mod[current_j+2];
						return true;
					}
					else if (indices_[mod[current_i+2]] == other.indices_[mod[current_j+1]])
					{
						l = mod[current_i+2];
						r = mod[current_j+1];
						return true;
					}
					return false;
			}
		};
	public:
		typedef scalar_type<vector3si>::type index_type;
		typedef Face_Property face_property;
		enum{vertex_count = 3};
	public:
		TriFaceByIndex(index_type v0,index_type v1,index_type v2):indices_(v0,v1,v2){}
		Face_Property& property(){return property_;}
		const Face_Property& property()const {return property_;}

		vector3i& index(){return indices_;}
		const vector3i& index()const{return indices_;}

		bool isAdjacent(const TriFaceByIndex& other,index_type& l_idx,index_type& r_idx)const
		{
			assert(!isSameVertices(other));
			//holy shit comes ------loop unrolled



			for (int i = 0;i < 3; i++)
			{
				if (indices_[i] == other.indices_[0] && compare_other::check(indices_,other,i,0,l_idx,r_idx))
				{
					return true;
				}
				else if (indices_[i] == other.indices_[1] && compare_other::check(indices_,other,i,1,l_idx,r_idx))
				{
					return true;
				}
				else if (indices_[i] == other.indices_[2] && compare_other::check(indices_,other,i,2,l_idx,r_idx))
				{
					return true;
				}
			}
			return false;
		}

		bool isSameVertices(const TriFaceByIndex& other)const{
			return
				(indices_[0] == other.indices_[0] &&
				((indices_[1] == other.indices_[1] && indices_[2] == other.indices_[2])||(indices_[1] == other.indices_[2] && indices_[2] == other.indices_[1]) ))

				|| (indices_[0] == other.indices_[1] &&
				((indices_[1] == other.indices_[2] && indices_[2] == other.indices_[0])|| (indices_[1] == other.indices_[0] && indices_[2] == other.indices_[2])) )

				|| (indices_[0] == other.indices_[2] &&
				((indices_[1] == other.indices_[0] && indices_[2] == other.indices_[1])|| (indices_[1] == other.indices_[1] && indices_[2] == other.indices_[0]) ));
		}
	};

	namespace ma_traits{
		template<typename FaceType>
		struct face_normal_policy{
			static  void calculate(){}
		};
	}
	//typedef MARenderMeshFixed<MAVertex>
	namespace mesh_op{
		//refer to CMeshManipulator
	}
}

#endif
