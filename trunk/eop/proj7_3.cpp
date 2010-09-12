//tree coordinate structure 
//

#include <vector>
#include <cassert>
//readable,weight type,empty,has_nth_successor,nth_successor
//index_of_nth_successor
template<typename T>
struct bidir_tree_coord{
    bidir_tree_coord<T>* predecessor;
    std::vector<bidir_tree_coord<T>*> successors;
    T value;
    explicit bidir_tree_coord(T x):value(x){
        predecessor = 0;
    }
};
//readable
template<typename T>
T& source (bidir_tree_coord<T>* t)
{
    return t->value;
}
    template<typename T>
const T& source (const bidir_tree_coord<T>* t)
{
    return t->value;
}
template<typename T>
bool empty(const bidir_tree_coord<T>* t)
{
    return t->successors.empty();
}

template<typename T>
size_t successor_size(const bidir_tree_coord<T>* t)
{
    return t->successors.size();
}
template<typename T>
bool has_nth_successor(const bidir_tree_coord<T>* t, size_t n)
{
    return successor_size(t) > n;
}

template<typename T>
bidir_tree_coord<T>* nth_successor(bidir_tree_coord<T>* t, size_t n)
{
    assert(has_nth_successor(t,n));
    return t->successors[n];
}
template<typename T>
const bidir_tree_coord<T>* nth_successor(const bidir_tree_coord<T>* t, size_t n)
{
    assert(has_nth_successor(t,n));
    return t->successors[n];
}

template<typename T>
bool has_predecessor(const bidir_tree_coord<T>* t)
{
    return t->predecessor != 0;
}
    template<typename T>
bidir_tree_coord<T>* predecessor(bidir_tree_coord<T>* t)
{
    return t->predecessor;
}

template<typename T>
const bidir_tree_coord<T>* predecessor(const bidir_tree_coord<T>* t)
{
    return t->predecessor;
}
template<typename T>
size_t index_of_nth_successor(const bidir_tree_coord<T>* t)
{
    assert(has_predecessor(t));
    const bidir_tree_coord<T>* pre = predecessor(t);
    for(size_t i = 0; i < successor_size(pre); ++i)
    {
        if( pre->successors[i] == t) return i;
    }
}

//traverse function
template<typename T>
int traverse_step(size_t &v, bidir_tree_coord<T>* &t)
{
    assert( has_predecessor(t) || v < successor_size( t ) );
    if(has_nth_successor(t,v))
    {
        t = nth_successor(t,v);
        v = 0;
        return 1;
    }
    v = index_of_nth_successor(t) + 1;
    t = predecessor(t) ;
    return -1;
}
///
template<typename T>
bool tree_isomorphic(bidir_tree_coord<T>* t0, bidir_tree_coord<T>* t1)
{
    if(empty(t0)) return empty(t1);
    if(empty(t1)) return false;
    const bidir_tree_coord<T>* r0 = t0;
    size_t v0 = 0;
    size_t v1 = 0;
    while(true)
    {
        traverse_step(v0,t0);
        traverse_step(v1,t1);
        if (v0 != v1) return false;
        if (t0 == r0 && v0 >= successor_size(t0)) return true;
    }
}
template<typename T, typename R>
bool tree_equivalent(bidir_tree_coord<T>* t0,bidir_tree_coord<T>* t1, R r)
{
    if(empty(t0)) return empty(t1);
    if(empty(t1)) return false;
    const bidir_tree_coord<T>* r0 = t0;
    size_t v0 = 0;
    size_t v1 = 0;
    while(true)
    {
        if(v0 == 0 && !r(source(t0),source(t1))) return false;
        traverse_step(v0,t0);
        traverse_step(v1,t1);
        if (v0 != v1) return false;
        if (t0 == r0 && v0 >= successor_size(t0)) return true;
    }
}
template<typename T, typename R>
bool tree_compare(bidir_tree_coord<T>* t0,bidir_tree_coord<T>* t1, R r)
{
    if(empty(t0)) return empty(t1);
    if(empty(t1)) return false;
    const bidir_tree_coord<T>* r0 = t0;
    size_t v0 = 0;
    size_t v1 = 0;
    while(true)
    {
        if(v0 == 0 ) 
        {
            if (r(source(t0),source(t1))) return true;
            if (r(source(t1),source(t0))) return false;
        }
        traverse_step(v0,t0);
        traverse_step(v1,t1);
        if (v0 != v1) return v0 > v1;
        if (t0 == r0 && v0 >= successor_size(t0)) return true;
    }
}


#include <iostream>

template<typename T>
void tree_print(bidir_tree_coord<T>* t)
{
    if(empty(t)) return ;
    const bidir_tree_coord<T>* r = t;
    size_t v = 0;
    while(true)
    {
        if(v == 0 ) 
        {
            std::cout<< source(t) <<",";
        }
        traverse_step(v,t);
        if (t == r && v >= successor_size(t)) return ;
    }

}
int main()
{
    //simple test
    typedef bidir_tree_coord<int> bidir_int_tree;
    bidir_int_tree* a0 = new bidir_int_tree(0);
    bidir_int_tree* a1 = new bidir_int_tree(1);
    bidir_int_tree* a2 = new bidir_int_tree(2);
    bidir_int_tree* a3 = new bidir_int_tree(3);
    bidir_int_tree* a4 = new bidir_int_tree(4);
    bidir_int_tree* a5 = new bidir_int_tree(5);
    bidir_int_tree* a6 = new bidir_int_tree(6);
    bidir_int_tree* a7 = new bidir_int_tree(7);
    bidir_int_tree* a8 = new bidir_int_tree(8);
    bidir_int_tree* a9 = new bidir_int_tree(9);
    bidir_int_tree* a10 = new bidir_int_tree(10);
    bidir_int_tree* a11 = new bidir_int_tree(11);
    bidir_int_tree* a12 = new bidir_int_tree(12);
    bidir_int_tree* a13 = new bidir_int_tree(13);
    bidir_int_tree* a14 = new bidir_int_tree(14);

    a0->successors.push_back(a1);a0->successors.push_back(a2);
    a1->successors.push_back(a3);
    a2->successors.push_back(a4);a2->successors.push_back(a5);a2->successors.push_back(a6);
    a3->successors.push_back(a7);a3->successors.push_back(a8);a3->successors.push_back(a9);
    a5->successors.push_back(a10);a5->successors.push_back(a11);
    a6->successors.push_back(a12);a6->successors.push_back(a13);a6->successors.push_back(a14);

    a1->predecessor = a2->predecessor = a0;
    a3->predecessor = a1;
    a4->predecessor = a5->predecessor = a6->predecessor = a2;
    a7->predecessor = a8->predecessor = a9->predecessor = a3;
    a10->predecessor = a11->predecessor = a5;
    a12->predecessor = a13->predecessor = a14->predecessor = a6;
    assert(tree_isomorphic(a0,a0));
    assert(!tree_isomorphic(a0,a1));
    tree_print(a0);
    std::cout<<std::endl;
    tree_print(a2);
}
