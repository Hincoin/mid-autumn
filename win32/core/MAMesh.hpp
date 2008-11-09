#ifndef __INCLUDE_MA_MESH_HPP__
#define __INCLUDE_MA_MESH_HPP__

#include <vector>
#include <list>

#include "Vector.hpp"

#include "MAVertex.hpp"
#include "MAPoly.hpp"

#include <boost/array.hpp>
#include <boost/mpl/if.hpp>

#include "Move.hpp"
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

	//editable mesh can borrow some concept from blender's bmesh or cgal's algorithm

	template<typename VertexType,typename FacesByIndex,typename MaterialType>
	struct MARenderMeshFixed{
		//tri-mesh or quad-mesh for storage efficiency
		typedef VertexType vertex_type;
		typedef MaterialType material_type;
		enum{vertex_per_face = FacesByIndex::vertex_count};
		typedef std::size_t index_type;
	private:	
		std::vector<VertexType> verts;
		std::vector<FacesByIndex> faces; // N vertices construct a face
		MaterialType material;
		//the topology: adjacent list of faces
		std::vector<boost::array<index_type,vertex_per_face> > topology;

		void buildTopology(const std::vector<VertexType>& v,const std::vector<FacesByIndex>& f){
			//
		}//building
	public:

		//construction
		MARenderMeshFixed(){}
		MARenderMeshFixed(
			const std::vector<VertexType>& verts,
			const std::vector<FacesByIndex>& faces,
			const material_type& mat
			):verts(verts),faces(faces),material(mat)
		{}

		//move from
		MARenderMeshFixed& operator = (ma::move_from<MARenderMeshFixed> other){}
		//
		void swap(MARenderMeshFixed& other){}

		//functions
		void setVertexArray(const std::vector<VertexType>& v){verts=v;}
		void setVertexArray(ma::move_from<std::vector<VertexType> > v){verts.swap(v);}
		void setFaceIndexArray(const std::vector<FacesByIndex>& f){faces = f;}
		void setFaceIndexArray(ma::move_from<std::vector<FacesByIndex> > f){faces.swap(f);}
		
		void setMaterial(const material_type& mat){material = mat}

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
		vector3i indices_;
		Face_Property property_;//face property such as normal
	public:
		typedef Face_Property face_property;
		enum{vertex_count = 3};
	public:
		Face_Property& property(){return property_;}
		const Face_Property& property()const {return property_;}

		vector3i& index(){return indices_;}
		const vector3i& index()const{return indices_;}
	};

}

#endif