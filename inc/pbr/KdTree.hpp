#ifndef KDTREE_HPP_INCLUDED
#define KDTREE_HPP_INCLUDED

#include <list>
#include "Primitive.hpp"
namespace ma{

	struct MemoryArena
	{
	private:

		typedef std::list<void*> pool_type;
		pool_type pool_;
	public:
		MemoryArena(){}
		void *Alloc(unsigned sz)
		{
			void* p = ::malloc(sz);
			pool_.push_back(p);
			return p;
		}
		~MemoryArena()
		{
			for (pool_type::iterator it = pool_.begin();it != pool_.end(); ++it)
				free(*it);
		}
	};
	using std::vector;
	namespace vop=vector_op;
	struct KdTreeAccel:MAAggregate<geometry_primitive_config<basic_config_t> >
	{
		typedef float S;
		static const int D = 3;
		typedef geometry_primitive_config<basic_config_t> config_t;
		typedef MAAggregate<config_t > parent_type;
		typedef KdTreeAccel class_type;
		typedef parent_type::virtual_table_type virtual_table_type;
		typedef parent_type::interface_type interface_type;
		typedef parent_type::ray_type Ray;
		typedef parent_type::intersection_t intersection_t;
		typedef parent_type::vector_t Vector;
		typedef parent_type::shared_primitive shared_primitive;

		static Vector vreciprocal(const Vector& v)
		{
			return Vector(reciprocal(vop::x(v)),reciprocal(vop::y(v)),reciprocal(vop::z(v)));
		}
		// KdAccelNode Declarations
		struct MailboxPrim
		{
			MailboxPrim(shared_primitive p):primitive(p)
			{
				//lastMailboxId = -1;
				::memset(lastMailboxId,0xFFFFFFFF,sizeof(lastMailboxId));
			}
			const shared_primitive primitive;
			int lastMailboxId[MAX_PARALLEL];
		};
		struct KdAccelNode
		{
			// KdAccelNode Methods
			void initLeaf(int *primNums, int np,
				MailboxPrim *mailboxPrims, MemoryArena &arena)
			{
				//		// Update kd leaf node allocation statistics
				//		static StatsCounter numLeafMade("Kd-Tree Accelerator",
				//		                                "Leaf kd-tree nodes made");
				//		//static StatsCounter maxDepth("Kd-Tree Accelerator",
				//		//                             "Maximum kd-tree depth");
				//		static StatsCounter maxLeafPrims("Kd-Tree Accelerator",
				//			"Maximum number of primitives in leaf node");
				//		++numLeafMade;
				//		//maxDepth.Max(depth);
				//		maxLeafPrims.Max(np);
				//		static StatsRatio leafPrims("Kd-Tree Accelerator",
				//			"Avg. number of primitives in leaf nodes");
				//		leafPrims.Add(np, 1);
				nPrims = np << 2;
				flags |= 3;
				// Store _MailboxPrim *_s for leaf node
				if (np == 0)
					onePrimitive = NULL;
				else if (np == 1)
					onePrimitive = &mailboxPrims[primNums[0]];
				else
				{
					primitives = (MailboxPrim **)arena.Alloc(np *
						sizeof(MailboxPrim *));
					for (int i = 0; i < np; ++i)
						primitives[i] = &mailboxPrims[primNums[i]];
				}
			}
			void initInterior(int axis, float s)
			{
				//		static StatsCounter nodesMade("Kd-Tree Accelerator", "Interior kd-tree nodes made"); // NOBOOK
				//		++nodesMade; // NOBOOK
				split = s;
				flags &= ~3;
				flags |= axis;
			}
			float SplitPos() const
			{
				return split;
			}
			int nPrimitives() const
			{
				return nPrims >> 2;
			}
			int SplitAxis() const
			{
				return flags & 3;
			}
			bool IsLeaf() const
			{
				return (flags & 3) == 3;
			}
			union
			{
				uint32 flags;   // Both
				float split;   // Interior
				uint32 nPrims;  // Leaf
			};
			union
			{
				uint32 aboveChild;           // Interior
				MailboxPrim *onePrimitive;  // Leaf
				MailboxPrim **primitives;   // Leaf
			};
		};
		struct BoundEdge
		{
			// BoundEdge Public Methods
			BoundEdge() { }
			BoundEdge(float tt, int pn, bool starting)
			{
				t = tt;
				primNum = pn;
				type = starting ? START : END;
			}
			bool operator<(const BoundEdge &e) const
			{
				if (t == e.t)
					return (int)type < (int)e.type;
				else return t < e.t;
			}
			float t;
			int primNum;
			enum { START, END } type;
		};
		struct KdToDo
		{
			const KdAccelNode *node;
			float tmin, tmax;
		};

	public:
		// KdTreeAccel Public Methods
		KdTreeAccel(const shared_primitive_array &p,
			int icost, int scost,
			float ebonus, int maxp, int maxDepth);
		~KdTreeAccel();
		void buildTree(int nodeNum, const BBox &bounds,
			const vector<BBox> &primBounds,
			int *primNums, int nprims, int depth,
			BoundEdge *edges[3],
			int *prims0, int *prims1, int badRefines = 0);

		static class_type& self(interface_type& x)
		{
			return static_cast<class_type&>(x);
		}
		static const class_type& self(const interface_type& x)
		{
			return static_cast<const class_type&>(x);
		}
		static void destruct(interface_type& x)
		{
			self(x).~KdTreeAccel();
		}
		static void swap(interface_type& x,interface_type& y)
		{
			MA_ASSERT(false);
		}

		static BBox worldBound(const interface_type& x)
		{
			return self(x).bounds;
		}
		static bool canIntersect(const interface_type& )
		{
			return true;
		}
		static bool intersect(const interface_type& x,const Ray &ray, intersection_t& isect) ;
		static bool intersectP(const interface_type& x,const Ray &ray) ;
	private:
		// KdTreeAccel Private Data
		int isectCost, traversalCost, maxPrims;
		float emptyBonus;
		size_t nMailboxes;
		MailboxPrim *mailboxPrims;
		mutable int curMailboxId[MAX_PARALLEL];
		KdAccelNode *nodes;
		int nAllocedNodes, nextFreeNode;
		BBox bounds;
		MemoryArena arena;
		///
		static const virtual_table_type kd_tree_vtable;
	};

}

namespace ma{
	MAKE_TYPE_STR_MAP(0,KdTreeAccel,kdtree)
namespace details{
template<>
	struct accelerator_creator<KdTreeAccel>{
		template<typename PrimRef>
	KdTreeAccel* operator()(
	const std::vector<PrimRef>& prims,const ParamSet& param
			)const
	{
	 //////////////////////////////////////////////////////////////////////////
	 int isectCost = param.as<int>("intersectcost",80);
	 int travCost = param.as<int>("traversalcost",1);
	 float emptyBonus =  param.as<float>("emptybonus",0.5f);
	 int maxPrims = param.as<int>("maxprims",1);
	 int maxDepth = param.as<int>("maxdepth",-1);
	 return new KdTreeAccel(prims, isectCost, travCost,
		 emptyBonus, maxPrims, maxDepth);
	}	
	};

}
}
#endif // KDTREE_HPP_INCLUDED
