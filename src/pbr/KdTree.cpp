#include "MAConfig.hpp"

#include "Primitive.hpp"



#include "Intersect.hpp"

#include "DefaultConfigurations.hpp"
#include "TypeDefs.hpp"
#include "KdTree.hpp"
#include "parallel_compute.hpp"

#if !defined(__APPLE__) && !defined(__OpenBSD__)
#include <malloc.h> // for _alloca, memalign
#endif
#if !defined(WIN32) && !defined(__APPLE__) && !defined(__OpenBSD__)
#include <alloca.h>
#endif

namespace ma
{

#define _doublemagicroundeps	      (.5-1.4e-11)
    //almost .5f = .5f - 1e^(number of exp bit)
    inline int round2Int(double val)
    {
#ifdef FAST_INT
#define _doublemagic			double (6755399441055744.0)
        //2^52 * 1.5,  uses limited precision to floor
        val		= val + _doublemagic;
        return (reinterpret_cast<long*>(&val))[0];
#else
        return int (val+_doublemagicroundeps);
#endif
    }


// Platform-specific definitions
#if defined(WIN32) && defined(_MSC_VER)
#define memalign(a,b) _aligned_malloc(b, a)
#elif defined(WIN32)
#define memalign(a,b) __mingw_aligned_malloc(b,a) 
#elif defined(__APPLE__)
#define memalign(a,b) valloc(b)
#elif defined(__OpenBSD__)
#define memalign(a,b) malloc(b)
#endif

// Memory Allocation Functions
    void *AllocAligned(size_t size)
    {
#ifndef L1_CACHE_LINE_SIZE
#define L1_CACHE_LINE_SIZE 64
#endif
        return memalign(L1_CACHE_LINE_SIZE, size);
    }
    void FreeAligned(void *ptr)
    {
#if defined(WIN32) && defined(_MSC_VER)
        _aligned_free(ptr);
#elif defined(WIN32)
		__mingw_aligned_free(ptr);
#else // NOBOOK
        free(ptr);
#endif // NOBOOK
    }

//template<int SZ>



    using namespace std;

   

    const KdTreeAccel::virtual_table_type
    KdTreeAccel::kd_tree_vtable=
    {
        &KdTreeAccel::destruct,
        &KdTreeAccel::swap,
        &KdTreeAccel::worldBound,
        &KdTreeAccel::canIntersect,
        &KdTreeAccel::intersect,
        &KdTreeAccel::intersectP,
        0,0
    };
// KdTreeAccel Method Definitions
    KdTreeAccel::
    KdTreeAccel(const shared_primitive_array &p,
                int icost, int tcost,
                float ebonus, int maxp, int maxDepth)
            :parent_type(kd_tree_vtable), isectCost(icost), traversalCost(tcost),
            maxPrims(maxp), emptyBonus(ebonus),
			mailboxPrims(0),
			nodes(0)
    {
        shared_primitive_array prims;
        for (uint32 i = 0; i < p.size(); ++i)
            ma::fullyRefine(p[i],prims);
        // Initialize mailboxes for _KdTreeAccel_
        //curMailboxId = 0;
		::memset(curMailboxId,0,sizeof(curMailboxId));
        nMailboxes = prims.size();
        mailboxPrims = (MailboxPrim *)AllocAligned(nMailboxes *
                       sizeof(MailboxPrim));
        for (uint32 i = 0; i < nMailboxes; ++i)
            new (&mailboxPrims[i]) MailboxPrim(prims[i]);
        // Build kd-tree for accelerator
        nextFreeNode = nAllocedNodes = 0;
        if (maxDepth <= 0)
            maxDepth =
                round2Int(8 + 1.3f * log2Int(float(prims.size())));
        // Compute bounds for kd-tree construction
        vector<BBox> primBounds;
        primBounds.reserve(prims.size());
        for (uint32 i = 0; i < prims.size(); ++i)
        {
            BBox b = prims[i]->worldBound();
            bounds = space_union(bounds, b);
            primBounds.push_back(b);
        }
        // Allocate working memory for kd-tree construction
        BoundEdge *edges[3];
        for (int i = 0; i < 3; ++i)
            edges[i] = new BoundEdge[2*prims.size()];
        int *prims0 = new int[prims.size()];
        int *prims1 = new int[(maxDepth+1) * prims.size()];
        // Initialize _primNums_ for kd-tree construction
        int *primNums = new int[prims.size()];
        for (uint32 i = 0; i < prims.size(); ++i)
            primNums[i] = i;
        // Start recursive construction of kd-tree
        buildTree(0, bounds, primBounds, primNums,
                 int( prims.size()), maxDepth, edges,
                  prims0, prims1);
        // Free working memory for kd-tree construction
        delete[] primNums;
        for (int i = 0; i < 3; ++i)
            delete[] edges[i];
        delete[] prims0;
        delete[] prims1;
    }
    KdTreeAccel::~KdTreeAccel()
    {
        for (uint32 i = 0; i < nMailboxes; ++i)
            mailboxPrims[i].~MailboxPrim();
        FreeAligned(mailboxPrims);
        FreeAligned(nodes);
    }
    void KdTreeAccel::buildTree(int nodeNum,
                                const BBox &nodeBounds,
                                const std::vector<BBox> &allPrimBounds, int *primNums,
                                int nPrims, int depth, BoundEdge *edges[3],
                                int *prims0, int *prims1, int badRefines)
    {
        assert(nodeNum == nextFreeNode); // NOBOOK
        // Get next free node from _nodes_ array
        if (nextFreeNode == nAllocedNodes)
        {
            int nAlloc = max(2 * nAllocedNodes, 512);
            KdAccelNode *n = (KdAccelNode *)AllocAligned(nAlloc *
                             sizeof(KdAccelNode));
            if (nAllocedNodes > 0)
            {
                memcpy(n, nodes,
                       nAllocedNodes * sizeof(KdAccelNode));
                FreeAligned(nodes);
            }
            nodes = n;
            nAllocedNodes = nAlloc;
        }
        ++nextFreeNode;
        // Initialize leaf node if termination criteria met
        if (nPrims <= maxPrims || depth == 0)
        {
            nodes[nodeNum].initLeaf(primNums, nPrims,
                                    mailboxPrims, arena);
            return;
        }
        // Initialize interior node and continue recursion
        // Choose split axis position for interior node
        int bestAxis = -1, bestOffset = -1;
		float bestCost = std::numeric_limits<float>::max();
        float oldCost = isectCost * float(nPrims);
        Vector d = nodeBounds.smax - nodeBounds.smin;
        //float totalSA = (2.f * (d.x*d.y + d.x*d.z + d.y*d.z));
        float totalSA = (2.f * (vop::x(d)*vop::y(d) + vop::x(d)*vop::z(d) + vop::y(d)*vop::z(d)));
        float invTotalSA = 1.f / totalSA;
        // Choose which axis to split along
        int axis;
        //if (d.x > d.y && d.x > d.z) axis = 0;
        if (vop::x(d) > vop::y(d) && vop::x(d) > vop::z(d)) axis = 0;
        //else axis = (d.y > d.z) ? 1 : 2;
        else axis = ( vop::y(d) > vop::z(d)) ? 1 : 2;
        int retries = 0;
retrySplit:
        // Initialize edges for _axis_
        for (int i = 0; i < nPrims; ++i)
        {
            int pn = primNums[i];
            const BBox &bbox = allPrimBounds[pn];
            edges[axis][2*i] =
                BoundEdge(bbox.smin[axis], pn, true);
            edges[axis][2*i+1] =
                BoundEdge(bbox.smax[axis], pn, false);
        }
        sort(&edges[axis][0], &edges[axis][2*nPrims]);
        // Compute cost of all splits for _axis_ to find best
        int nBelow = 0, nAbove = nPrims;
        for (int i = 0; i < 2*nPrims; ++i)
        {
            if (edges[axis][i].type == BoundEdge::END) --nAbove;
            float edget = edges[axis][i].t;
            if (edget > nodeBounds.smin[axis] &&
                    edget < nodeBounds.smax[axis])
            {
                // Compute cost for split at _i_th edge
                int otherAxis[3][2] = { {1,2}, {0,2}, {0,1} };
                int otherAxis0 = otherAxis[axis][0];
                int otherAxis1 = otherAxis[axis][1];
                float belowSA = 2 * (d[otherAxis0] * d[otherAxis1] +
                                     (edget - nodeBounds.smin[axis]) *
                                     (d[otherAxis0] + d[otherAxis1]));
                float aboveSA = 2 * (d[otherAxis0] * d[otherAxis1] +
                                     (nodeBounds.smax[axis] - edget) *
                                     (d[otherAxis0] + d[otherAxis1]));
                float pBelow = belowSA * invTotalSA;
                float pAbove = aboveSA * invTotalSA;
                float eb = (nAbove == 0 || nBelow == 0) ? emptyBonus : 0.f;
                float cost = traversalCost + isectCost * (1.f - eb) *
                             (pBelow * nBelow + pAbove * nAbove);
                // Update best split if this is lowest cost so far
                if (cost < bestCost)
                {
                    bestCost = cost;
                    bestAxis = axis;
                    bestOffset = i;
                }
            }
            if (edges[axis][i].type == BoundEdge::START) ++nBelow;
        }
        assert(nBelow == nPrims && nAbove == 0); // NOBOOK
        // Create leaf if no good splits were found
        if (bestAxis == -1 && retries < 2)
        {
            ++retries;
            axis = (axis+1) % 3;
            goto retrySplit;
        }
        if (bestCost > oldCost) ++badRefines;
        if ((bestCost > 4.f * oldCost && nPrims < 16) ||
                bestAxis == -1 || badRefines == 3)
        {
            nodes[nodeNum].initLeaf(primNums, nPrims,
                                    mailboxPrims, arena);
            return;
        }
        // Classify primitives with respect to split
        int n0 = 0, n1 = 0;
        for (int i = 0; i < bestOffset; ++i)
            if (edges[bestAxis][i].type == BoundEdge::START)
                prims0[n0++] = edges[bestAxis][i].primNum;
        for (int i = bestOffset+1; i < 2*nPrims; ++i)
            if (edges[bestAxis][i].type == BoundEdge::END)
                prims1[n1++] = edges[bestAxis][i].primNum;
        // Recursively initialize children nodes
        float tsplit = edges[bestAxis][bestOffset].t;
        nodes[nodeNum].initInterior(bestAxis, tsplit);
        BBox bounds0 = nodeBounds, bounds1 = nodeBounds;
        bounds0.smax[bestAxis] = bounds1.smin[bestAxis] = tsplit;
        buildTree(nodeNum+1, bounds0,
                  allPrimBounds, prims0, n0, depth-1, edges,
                  prims0, prims1 + nPrims, badRefines);
        nodes[nodeNum].aboveChild = nextFreeNode;
        buildTree(nodes[nodeNum].aboveChild, bounds1, allPrimBounds,
                  prims1, n1, depth-1, edges,
                  prims0, prims1 + nPrims, badRefines);
    }

    bool KdTreeAccel::intersect(const interface_type& x,const Ray &ray,
                                intersection_t& isect)
    {
        // Compute initial parametric range of ray inside kd-tree extent

        const BBox& bounds = self(x).bounds;
        int& curMailboxId = self(x).curMailboxId[get_thread_logic_id()];

        const KdAccelNode *nodes = self(x).nodes;

        float tmin, tmax;
        if (!ma::intersectP(bounds,ray, tmin, tmax))
            return false;
        // Prepare to traverse kd-tree for ray
        int rayId = curMailboxId++;
        //Vector invDir(1.f/ray.d.x, 1.f/ray.d.y, 1.f/ray.d.z);
        Vector invDir(vreciprocal(ray.dir));
#define MAX_TODO 64
        KdToDo todo[MAX_TODO];
        int todoPos = 0;
        // Traverse kd-tree nodes in order for ray
        bool hit = false;
        const KdAccelNode *node = &nodes[0];
        while (node != NULL)
        {
            // Bail out if we found a hit closer than the current node
            if (ray.maxt < tmin) break;
            //static StatsCounter nodesTraversed("Kd-Tree Accelerator", //NOBOOK
            //	"Number of kd-tree nodes traversed by normal rays"); //NOBOOK
            //++nodesTraversed; //NOBOOK
            if (!node->IsLeaf())
            {
                // Process kd-tree interior node
                // Compute parametric distance along ray to split plane
                int axis = node->SplitAxis();
                float tplane = (node->SplitPos() - ray.o[axis]) *
                               invDir[axis];
                // Get node children pointers for ray
                const KdAccelNode *firstChild, *secondChild;
                int belowFirst = ray.o[axis] <= node->SplitPos();
                if (belowFirst)
                {
                    firstChild = node + 1;
                    secondChild = &nodes[node->aboveChild];
                }
                else
                {
                    firstChild = &nodes[node->aboveChild];
                    secondChild = node + 1;
                }
                // Advance to next child node, possibly enqueue other child
                if (tplane > tmax || tplane < 0)
                    node = firstChild;
                else if (tplane < tmin)
                    node = secondChild;
                else
                {
                    // Enqueue _secondChild_ in todo list
                    todo[todoPos].node = secondChild;
                    todo[todoPos].tmin = tplane;
                    todo[todoPos].tmax = tmax;
                    ++todoPos;
                    node = firstChild;
                    tmax = tplane;
                }
            }
            else
            {
                // Check for intersections inside leaf node
                uint32 nPrimitives = node->nPrimitives();
                if (nPrimitives == 1)
                {
                    MailboxPrim *mp = node->onePrimitive;
                    // Check one primitive inside leaf node
                    if (mp->lastMailboxId[get_thread_logic_id()] != rayId)
                    {
                        mp->lastMailboxId[get_thread_logic_id()] = rayId;
                        if (mp->primitive->intersect(ray, isect))
                            hit = true;
                    }
                }
                else
                {
                    MailboxPrim **prims = node->primitives;
                    for (uint32 i = 0; i < nPrimitives; ++i)
                    {
                        MailboxPrim *mp = prims[i];
                        // Check one primitive inside leaf node
                        if (mp->lastMailboxId[get_thread_logic_id()] != rayId)
                        {
                            mp->lastMailboxId[get_thread_logic_id()] = rayId;
                            if (mp->primitive->intersect(ray, isect))
                                hit = true;
                        }
                    }
                }
                // Grab next node to process from todo list
                if (todoPos > 0)
                {
                    --todoPos;
                    node = todo[todoPos].node;
                    tmin = todo[todoPos].tmin;
                    tmax = todo[todoPos].tmax;
                }
                else
                    break;
            }
        }
        return hit;
    }
    bool KdTreeAccel::intersectP(const interface_type& x, const Ray &ray)
    {
        // Compute initial parametric range of ray inside kd-tree extent
        const BBox& bounds = self(x).bounds;
        int& curMailboxId = self(x).curMailboxId[get_thread_logic_id()];
        const KdAccelNode *nodes = self(x).nodes;

        float tmin, tmax;
        if (!ma::intersectP(bounds,ray, tmin, tmax))
            return false;
        // Prepare to traverse kd-tree for ray
        int rayId = curMailboxId++;
        //Vector invDir(1.f/ray.d.x, 1.f/ray.d.y, 1.f/ray.d.z);
        Vector invDir(vreciprocal(ray.dir));
#define MAX_TODO 64
        KdToDo todo[MAX_TODO];
        int todoPos = 0;
        const KdAccelNode *node = &nodes[0];
        while (node != NULL)
        {
            // Update kd-tree shadow ray traversal statistics
            //static StatsCounter nodesTraversed("Kd-Tree Accelerator",
            //	"Number of kd-tree nodes traversed by shadow rays");
            //++nodesTraversed;
            if (node->IsLeaf())
            {
                // Check for shadow ray intersections inside leaf node
                uint32 nPrimitives = node->nPrimitives();
                if (nPrimitives == 1)
                {
                    MailboxPrim *mp = node->onePrimitive;
                    if (mp->lastMailboxId[get_thread_logic_id()] != rayId)
                    {
                        mp->lastMailboxId[get_thread_logic_id()] = rayId;
                        if (mp->primitive->intersectP(ray))
                            return true;
                    }
                }
                else
                {
                    MailboxPrim **prims = node->primitives;
                    for (uint32 i = 0; i < nPrimitives; ++i)
                    {
                        MailboxPrim *mp = prims[i];
                        if (mp->lastMailboxId[get_thread_logic_id()] != rayId)
                        {
                            mp->lastMailboxId[get_thread_logic_id()] = rayId;
                            if (mp->primitive->intersectP(ray))
                                return true;
                        }
                    }
                }
                // Grab next node to process from todo list
                if (todoPos > 0)
                {
                    --todoPos;
                    node = todo[todoPos].node;
                    tmin = todo[todoPos].tmin;
                    tmax = todo[todoPos].tmax;
                }
                else
                    break;
            }
            else
            {
                // Process kd-tree interior node
                // Compute parametric distance along ray to split plane
                int axis = node->SplitAxis();
                float tplane = (node->SplitPos() - ray.o[axis]) *
                               invDir[axis];
                // Get node children pointers for ray
                const KdAccelNode *firstChild, *secondChild;
                int belowFirst = ray.o[axis] <= node->SplitPos();
                if (belowFirst)
                {
                    firstChild = node + 1;
                    secondChild = &nodes[node->aboveChild];
                }
                else
                {
                    firstChild = &nodes[node->aboveChild];
                    secondChild = node + 1;
                }
                // Advance to next child node, possibly enqueue other child
                if (tplane > tmax || tplane < 0)
                    node = firstChild;
                else if (tplane < tmin)
                    node = secondChild;
                else
                {
                    // Enqueue _secondChild_ in todo list
                    todo[todoPos].node = secondChild;
                    todo[todoPos].tmin = tplane;
                    todo[todoPos].tmax = tmax;
                    ++todoPos;
                    node = firstChild;
                    tmax = tplane;
                }
            }
        }
        return false;
    }
}



