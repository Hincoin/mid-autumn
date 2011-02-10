#ifndef _KDTREE_H_
#define _KDTREE_H_

typedef struct  
{
	float split_pos;
	unsigned data;
/*	unsigned split_axis:2;
	unsigned has_left_child:1;
	unsigned right_child:29;
*/
}kd_node_t;

#define kd_node_set_split_axis(_kd_node,_split_axis)\
{\
	(_kd_node).data = ( ((_kd_node).data & (0x3FFFFFFF)) | ( ( ((unsigned)_split_axis) & 0x03) << 30 ));\
}\

#define kd_node_get_split_axis(_kd_node)\
	((_kd_node).data>>30)

#define kd_node_set_has_left_child(_kd_node,_has_left)\
{\
	(_kd_node).data = (((_kd_node).data & (0xdfffffff))) | ((((unsigned)_has_left)&0x01)<<29);\
}\

#define kd_node_get_has_left_child(_kd_node)\
	(((_kd_node).data >> 29) & 0x01)

#define kd_node_set_right_child(_kd_node,_right_child)\
{\
(_kd_node).data = ((_kd_node).data&0xe0000000) | (((unsigned)_right_child) & 0x1fffffff );\
}\

#define kd_node_get_right_child(_kd_node)\
	((_kd_node).data & 0x1fffffff)

void kd_node_init(kd_node_t* kd_node,float p,unsigned a)
{
	kd_node->split_pos = p;
	kd_node_set_split_axis(*kd_node,a);
	kd_node_set_has_left_child(*kd_node,0);
	kd_node_set_right_child(*kd_node,((1<<29)-1))
}
void kd_node_init_leaf(kd_node_t* kd_node)
{
	kd_node_set_split_axis(*kd_node,3);
	kd_node_set_has_left_child(*kd_node,0);
	kd_node_set_right_child(*kd_node,((1<<29)-1))
}

#define kd_tree_t(NodeData)\
	struct  \
	{\
		kd_node_t *nodes;\
		NodeData *node_data;\
		unsigned n_nodes,next_free_node;\
	}\

//todo

#define kd_tree_lookup(_p,_proc_data,_proc,_max_dist_sqr)\


#ifndef CL_KERNEL
//cpu code to build kd_tree
#include <vector>
#include "memory.h"

template <typename NodeData> struct CompareNode {
	CompareNode(int a) { axis = a; }
	int axis;
	bool operator()(const NodeData *d1, const NodeData *d2) const {
		return d1->p[axis] == d2->p[axis] ? (d1 < d2) :
			d1->p[axis] < d2->p[axis];
	}
};


template<typename KDTree,typename NodeData> 
void _kd_tree_recursive_build(KDTree* kd_tree,
					 unsigned node_num,
					 unsigned start,
					 unsigned end,
					 const NodeData** build_nodes)
{
//todo
}
template<typename KDTree,typename NodeData> 
void kd_tree_init(KDTree* kd_tree,
				  const std::vector<NodeData>& d)
{
	kd_tree->n_nodes = d.size();
	kd_tree->nodes = alloc_aligned<kd_node_t>(kd_tree->n_nodes);
	kd_tree->node_data = alloc_aligned<NodeData>(kd_tree->n_nodes);
	std::vector<const NodeData*> build_nodes(kd_tree->n_nodes,NULL);
	for (unsigned i = 0;i < kd_tree->n_nodes; ++i)
	{
		build_nodes[i] = &d[i];
	}
	_kd_tree_recursive_build(kd_tree,0,0,kd_tree->n_nodes,&build_nodes[0]);
}

template<typename KDTree,typename NodeData> 
void kd_tree_destroy(KDTree* kd_tree)
{
	free_aligned(kd_tree->nodes);
	free_aligned(kd_tree->node_data);
}


#endif



#endif