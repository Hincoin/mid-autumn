#ifndef __INCLUDE_MA_MESH_BUFFER_HPP__
#define __INCLUDE_MA_MESH_BUFFER_HPP__

namespace ma{
	//editable mesh can borrow some concept from blender's bmesh or cgal's algorithm

	//this mesh type can be animated, rendered but the vertex number cannot be changed

	//a mesh interface
	template<typename Derived,typename Configure>
	class MeshBuffer{
		Derived& derived(){return static_cast<Derived&>(*this);}
		const Derived& derived()const{return static_cast<const Derived&>(*this);}
	public:
		//make a clone of self
		Derived* clone()const{derived().clone();} 

		////! Get the material of this meshbuffer
		///** \return Material of this buffer. */
		//typename Configure::material_type& getMaterial(){return derived().getMaterial();}

		//////! Get the material of this meshbuffer
		/////** \return Material of this buffer. */
		//const Configure::material_type& getMaterial() const {return derived().getMaterial();}

		//////! Get access to vertex data. The data is an array of vertices.
		/////** Which vertex type is used can be determined by getVertexType().
		////\return Pointer to array of vertices. */
		//const std::vector<typename Configure::vertex_type>& getVertices() const {return derived().getVertices();}

		//////! Get access to vertex data. The data is an array of vertices.
		/////** Which vertex type is used can be determined by getVertexType().
		////\return Pointer to array of vertices. */
		//std::vector<typename Configure::vertex_type>& getVertices() {return derived().getVertices();}

		//////! Get amount of vertices in meshbuffer.
		/////** \return Number of vertices in this buffer. */
		//std::size_t getVertexCount() const {return derived().getVertexCount();}

		//////! Get access to Indices.
		/////** \return Pointer to indices array. */
		//const std::vector<typename Configure::face_type>& getIndexFaces() const {return derived().getIndexFaces();}

		//////! Get access to Indices.
		/////** \return Pointer to indices array. */
		//std::vector<typename Configure::face_type>& getIndexFaces() {return derived().getIndexFaces();}


		//////! Get amount of indices in this meshbuffer.
		/////** \return Number of indices in this buffer. */
		//std::size_t getIndexFaceCount() const {return derived().getIndexFaceCount();}

		////! Get the axis aligned bounding box of this meshbuffer.
		///** \return Axis aligned bounding box of this buffer. */
		//virtual const core::aabbox3df& getBoundingBox() const = 0;

		////! Set axis aligned bounding box
		///** \param box User defined axis aligned bounding box to use
		//for this buffer. */
		//virtual void setBoundingBox(const core::aabbox3df& box) = 0;

		////! Recalculates the bounding box. Should be called if the mesh changed.
		//virtual void recalculateBoundingBox() = 0;
	};
}
#endif