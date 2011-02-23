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

INLINE void kd_node_init(kd_node_t* kd_node,float p,unsigned a)
{
	kd_node->split_pos = p;
	kd_node_set_split_axis(*kd_node,a);
	kd_node_set_has_left_child(*kd_node,0);
	kd_node_set_right_child(*kd_node,((1<<29)-1))
}
INLINE void kd_node_init_leaf(kd_node_t* kd_node)
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
#define KD_TREE_MAX_DEPTH 64

#define distance_squared(_v0,_v1)\
	vdot(_v0,_v1)

//stack
#define kd_tree_lookup(_kd_tree,_p,_proc_data,_proc,_max_dist_sqr)\
{\
	unsigned node_stack[KD_TREE_MAX_DEPTH];\
	int stack_top=0;\
	int node_num = 0;\
	node_stack[stack_top++] = (node_num);\
\
	while (stack_top != 0)\
	{\
		node_num = node_stack[stack_top-1];\
		kd_node_t* node = (_kd_tree).nodes + node_num;\
		unsigned axis = kd_node_get_split_axis(*node);\
		if(axis == 3)\
		{\
			float dist2 = distance_squared((_kd_tree).node_data[node_num].p, (_p));\
			if (dist2 < (_max_dist_sqr))\
				(_proc)((_proc_data), &((_kd_tree).node_data[node_num]), dist2, &(_max_dist_sqr));\
			--stack_top;\
		}\
		else{\
			--stack_top;\
			float project_to_axis = (_p).x;\
			if(axis == 1)project_to_axis = (_p).y;\
			if(axis == 2)project_to_axis = (_p).z;\
			float dist2 = (project_to_axis - node->split_pos) * (project_to_axis - node->split_pos);\
			if( project_to_axis <= node->split_pos)\
			{\
				if (kd_node_get_has_left_child(*node))\
				{\
					node_stack[stack_top++] = node_num+1;\
				}\
				if (dist2 < (_max_dist_sqr) && kd_node_get_right_child(*node) < (_kd_tree).n_nodes)\
				{\
					node_stack[stack_top++] = kd_node_get_right_child(*node);\
				}\
			}\
			else\
			{\
				if(kd_node_get_right_child(*node) < (_kd_tree).n_nodes)\
				{\
					node_stack[stack_top++] =  kd_node_get_right_child(*node);\
				}\
				if(dist2 < (_max_dist_sqr) && (kd_node_get_has_left_child(*node)))\
				{\
					node_stack[stack_top++] = node_num+1;\
				}\
			}\
		}\
	}\
}\


#ifndef CL_KERNEL
//cpu code to build kd_tree
#include <vector>
#include <algorithm>

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
	// Create leaf node of kd-tree if we've reached the bottom
	if (start + 1 == end) {
		kd_node_init_leaf(kd_tree->nodes+ node_num);
		kd_tree->node_data[node_num] = *build_nodes[start];
		return;
	}

	// Choose split direction and partition data

	// Compute bounds of data from _start_ to _end_
	bbox_t bound;
	bbox_init(&bound);
	for (int i = start; i < end; ++i)
		bbox_union_with_point(&bound,&(build_nodes[i]->p))	;
	int splitAxis = bbox_max_extent(&bound);
	int splitPos = (start+end)/2;
	std::nth_element(&build_nodes[start], &build_nodes[splitPos],
		&build_nodes[end], CompareNode<NodeData>(splitAxis));

	// Allocate kd-tree node and continue recursively
	kd_node_init(kd_tree->nodes+node_num,build_nodes[splitPos]->p[splitAxis],splitAxis);
	kd_tree->node_data[node_num] = *build_nodes[splitPos];
	if (start < splitPos) {
		kd_node_set_has_left_child(kd_tree->nodes[node_num],1);
		unsigned int childNum = kd_tree->next_free_node++;
		_kd_tree_recursive_build(kd_tree,childNum, start, splitPos, build_nodes);
	}
	if (splitPos+1 < end) {
		kd_node_set_right_child(kd_tree->nodes[node_num],kd_tree->next_free_node);
		kd_tree->next_free_node ++;
		_kd_tree_recursive_build(kd_tree,kd_node_get_right_child(kd_tree->nodes[node_num]),
			splitPos+1,end,build_nodes);
	}

}
template<typename KDTree,typename NodeData> 
void kd_tree_init(KDTree* kd_tree,
				  const std::vector<NodeData>& d)
{
	kd_tree->n_nodes = d.size();
	kd_tree->nodes = alloc_aligned<kd_node_t>(kd_tree->n_nodes);
	kd_tree->node_data = alloc_aligned<NodeData>(kd_tree->n_nodes);
	kd_tree->next_free_node = 1;
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