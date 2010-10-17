

    template<typename T, typename P, typename F>
inline void loop_action(T& v, P& p,F& f)
{
    while (!p(v))
    {
        f(v);
    }
}

#include <utility>
#include <algorithm>
#include <vector>
#include <stack>
#include <set>
#include <cassert>
//left,right,up,down
enum ConnectorType{
    L=0,R,U,D, 
    UR,RD,DL,LU,LR,UD, 
    LUR,DLU,RDL,URD,
    LURD,
    UnknownOrCount
};
//for print
static const char* ConnectTypeStr[] = 
{   "L    ",
    "R    ",
    "U    ",
    "D    ", 
    "UR   ",
    "RD   ",
    "DL   ",
    "LU   ",
    "LR   ",
    "UD   ", 
    "LUR  ",
    "DLU  ",
    "RDL  ",
    "URD  ",
    "LURD ",
    "UnknownOrCount"
};
    template<int _0, int _1, int _2, int _3, int _4, int _5, int _6, int _7>
bool is_one_of(ConnectorType c)
{
    return _0 == c || _1 == c || _2 == c || _3 == c || _4 == c || _5 == c || _6 == c || _7 == c;
}

struct check_connection
{
    template<typename Array2D>
        std::pair<typename Array2D::size_type, typename Array2D::size_type> go_left(const Array2D& array, const std::pair<typename Array2D::size_type, typename Array2D::size_type>& t)const
        {
            assert(t.second > 0);
            return (std::make_pair(t.first,t.second - 1));

        }
    template<typename Array2D>
        std::pair<typename Array2D::size_type, typename Array2D::size_type> go_up(const Array2D& array, const std::pair<typename Array2D::size_type, typename Array2D::size_type>& t)const
        {
            assert(t.first > 0);
            return (std::make_pair(t.first - 1, t.second));
        }
    template<typename Array2D>
        std::pair<typename Array2D::size_type, typename Array2D::size_type> go_right(const Array2D& array, const std::pair<typename Array2D::size_type, typename Array2D::size_type>& t)const
        {
            assert(t.second +1 < array[t.first].size());
            return (std::make_pair(t.first, t.second +1));

        }

    template<typename Array2D>
        std::pair<typename Array2D::size_type, typename Array2D::size_type> go_down(const Array2D& array, const std::pair<typename Array2D::size_type, typename Array2D::size_type>& t)const
        {
            assert(t.first + 1 < array.size());
            return (std::make_pair(t.first+1,t.second));

        }
    template<typename Array2D>
        bool operator()(const Array2D& array)const
        {
            if (array.empty()) return true;
            if (array[0].empty()) return true;
            typedef typename Array2D::size_type size_type;

            size_type i = 0;
            size_type j = 0;
            typedef std::pair<size_type,size_type> coord_type;

            std::stack< coord_type > coord_stack;
            std::set< coord_type > coords;
            coord_stack.push(std::make_pair(i,j));
            while(!coord_stack.empty())
            {
                coord_type t = coord_stack.top();
                coord_stack.pop();
                if (coords.find(std::make_pair(t.first,t.second)) != coords.end())
                    continue;
                coords.insert(std::make_pair(t.first,t.second));
                switch(array[t.first][t.second])
                {
                    case L:
                        {
                            coord_stack.push(go_left(array,t));
                        }
                        break;
                    case U:
                        {
                            coord_stack.push(go_up(array,t));
                        }
                        break;
                    case R:
                        {
                            coord_stack.push(go_right(array,t));
                        }
                        break;
                    case D:
                        {
                            coord_stack.push(go_down(array,t));
                        }
                        break;
                    case LU:
                        {
                            coord_stack.push(go_left(array,t));
                            coord_stack.push(go_up(array,t));
                        }
                        break;
                    case LR:
                        {
                            coord_stack.push(go_left(array,t));
                            coord_stack.push(go_right(array,t));
                        }
                        break;
                    case DL:
                        {
                            coord_stack.push(go_left(array,t));
                            coord_stack.push(go_down(array,t));
                        }
                        break;
                    case UR:
                        {
                            coord_stack.push(go_up(array,t));
                            coord_stack.push(go_right(array,t));
                        }
                        break;
                    case UD:
                        {
                            coord_stack.push(go_up(array,t));
                            coord_stack.push(go_down(array,t));
                        }
                        break;
                    case RD:
                        coord_stack.push(go_right(array,t));
                        coord_stack.push(go_down(array,t));
                        break;
                    case LUR:
                        coord_stack.push(go_left(array,t));
                        coord_stack.push(go_up(array,t));
                        coord_stack.push(go_right(array,t));
                        break;
                    case URD:
                        coord_stack.push(go_up(array,t));
                        coord_stack.push(go_right(array,t));
                        coord_stack.push(go_down(array,t));
                        break;
                    case RDL:
                        coord_stack.push(go_right(array,t));
                        coord_stack.push(go_down(array,t));
                        coord_stack.push(go_left(array,t));
                        break;
                    case DLU:
                        coord_stack.push(go_down(array,t));
                        coord_stack.push(go_left(array,t));
                        coord_stack.push(go_up(array,t));
                        break;
                    case LURD:
                        coord_stack.push(go_left(array,t));
                        coord_stack.push(go_up(array,t));
                        coord_stack.push(go_right(array,t));
                        coord_stack.push(go_down(array,t));
                        break;

                    case UnknownOrCount:
                        if (t.first > 0)
                        {
                            ConnectorType c = array[t.first - 1][t.second];
                            if(c == UnknownOrCount || 
                                    is_one_of<D,DL,UD,RD,URD,RDL,DLU,LURD>(c))
                            {
                                coord_stack.push(go_up(array,t));
                            }
                        }
                        if (t.first+1 < array.size())
                        {
                            ConnectorType c = array[t.first + 1][t.second];
                            if (c == UnknownOrCount || 
                                    is_one_of<U,LU,UR,UD,LUR,URD,DLU,LURD>(c))
                                coord_stack.push(go_down(array,t));
                        }
                        if (t.second > 0)
                        {
                            ConnectorType c = array[t.first][t.second - 1];
                            if (c == UnknownOrCount || 
                                    is_one_of<R,LR,UR,RD,LUR,URD,RDL,LURD>(c))
                                coord_stack.push(go_left(array,t));
                        }
                        if (t.second + 1 < array[t.first].size())
                        {
                            ConnectorType c = array[t.first][t.second + 1];
                            if (c == UnknownOrCount || 
                                    is_one_of<L,LR,LU,DL,LUR,RDL,DLU,LURD>(c))
                            {
                                coord_stack.push(go_right(array,t));
                            }
                        }
                        break;
                    default:assert(false);
                            break;
                }
            }
            for (i = 0;i < array.size(); ++i)
                for(j = 0;j < array[i].size(); ++j)
                {
                    if (coords.end() == coords.find(std::make_pair(i,j)))
                    {
                        return false;
                    }
                }
            return true;
        }
};
struct is_complete{
    mutable size_t count ;
    is_complete(size_t max_count = size_t(-1)):count(max_count){}
    template<typename Array2D>
        bool operator()(const Array2D& array)const
        {
            typedef typename Array2D::size_type size_type;
            if (count == 0) return true;
            //
            typedef typename Array2D::size_type size_type;
            for(size_type i = 0;i < array.size(); ++i)
                for(size_type j = 0;j < array[i].size(); ++j)
                {
                    if (array[i][j] == UnknownOrCount) return false;
                }
            return true;
        }
};

template<typename Array2D>
struct place_step
{
    private:
        typedef typename Array2D::size_type size_type;
        typedef place_step<Array2D> class_type;
        size_type x,z;

        enum NeighborRelation{NoPath, HasPath, NoNeighbor, RelationCount}; 
        typedef std::vector<ConnectorType> connector_array_t;
        connector_array_t condition_map[RelationCount][RelationCount][RelationCount][RelationCount];

        template<int _0,int _1,int _2,int _3,int _4,int _5,int _6,int _7>
            connector_array_t filter_by_conn(bool bHasPath, const connector_array_t& a)
            {
                connector_array_t ret;
                for (connector_array_t::const_iterator it = a.begin(); it != a.end();++it)
                {
                    if( is_one_of<_0,_1,_2,_3,_4,_5,_6,_7>(*it))
                    {
                        if (bHasPath)
                            ret.push_back(*it);
                    }
                    else if (!bHasPath)
                        ret.push_back(*it);
                }

                return ret;

            }
        connector_array_t filter_by_left_conn(bool bHasPath, const connector_array_t& a)
        {
            return filter_by_conn<L,LU,LR,DL,LUR,RDL,DLU,LURD>(bHasPath,a);
        }
        connector_array_t filter_by_up_conn(bool bHasPath, const connector_array_t& a)
        {
            return filter_by_conn<U,LU,UR,UD,LUR,URD,DLU,LURD>(bHasPath,a);
        }
        connector_array_t filter_by_down_conn(bool bHasPath, const connector_array_t& a)
        {
            return filter_by_conn<D,DL,UD,RD,URD,RDL,DLU,LURD>(bHasPath,a);
        }
        connector_array_t filter_by_right_conn(bool bHasPath, const connector_array_t& a)
        {
            return filter_by_conn<R,LR,UR,RD,LUR,URD,RDL,LURD>(bHasPath,a);
        }

        void init_condition_map()
        {
            ConnectorType all_cs[]={L,R,U,D, 
                UR,RD,DL,LU,LR,UD, 
                LUR,DLU,RDL,URD,
                LURD};
            connector_array_t all_connectors(all_cs,all_cs + sizeof(all_cs)/sizeof(ConnectorType));
            NeighborRelation nr[] = {NoPath, HasPath,  NoNeighbor};
            size_t count = sizeof(nr)/sizeof(NeighborRelation);
            for (size_t li = 0;li < count; ++li)
                for (size_t ui = 0; ui < count; ++ui)
                    for(size_t ri = 0; ri < count; ++ri)
                        for (size_t di = 0; di < count; ++di)
                        {
                            NeighborRelation l,u,r,d;
                            l = nr[li];
                            u = nr[ui];
                            r = nr[ri];
                            d = nr[di];

                            connector_array_t cs ( all_connectors ); 
                            if (l != NoNeighbor)
                                cs = (filter_by_left_conn( l == HasPath, cs));
                            if (u != NoNeighbor)
                                cs = (filter_by_up_conn( u == HasPath, cs));
                            if (r != NoNeighbor)
                                cs = (filter_by_right_conn( r == HasPath, cs));
                            if (d != NoNeighbor)
                                cs = (filter_by_down_conn( d == HasPath, cs));
                            condition_map[l][u][r][d].swap(cs);
                        }
        }

        struct advance_it{
            template<typename I>
                void operator()(I& it)const
                {
                    ++it ;
                }
        };
        struct check_end{
            const Array2D& array;
            connector_array_t& cs;
            size_type x,z;
            check_end(size_type x, size_type z,const Array2D& a, connector_array_t& c):
                x(x),z(z),array(a),cs(c){}
            bool operator()(connector_array_t::iterator& it)const
            {
                if (it == cs.end())
                {
                    return true;
                }
                Array2D tmp ( array );
                tmp[z][x] = *it;
                return *it == LURD || check_connection()(tmp);
            }
        };
        ConnectorType pick_current_connector(const Array2D& array, 
                NeighborRelation left, NeighborRelation up, NeighborRelation right, NeighborRelation down
                ) const
        {
            connector_array_t& a = const_cast<connector_array_t&>(condition_map[left][up][right][down]) ;
            if (a.empty() ){ assert(false); return UnknownOrCount;}   

            //shuffle a
            std::random_shuffle(a.begin(), a.end());

            connector_array_t::iterator it = a.begin(); 
            check_end p(x,z,array,a);
            advance_it step;
            loop_action(it ,p,step);
            if (it == a.end()) { assert(false); return UnknownOrCount;}
            ConnectorType ret = *it;
            return ret;
        }

        NeighborRelation get_left_relation(const Array2D& array)const
        {
            if (x == 0) return class_type::NoPath;
            ConnectorType left = array[z][x-1];
            if (left == UnknownOrCount) return class_type::NoNeighbor;
            if (is_one_of<R,UR,LR,RD,LUR,URD,RDL,LURD>(left))
                return class_type::HasPath;
            return class_type::NoPath;
        }
        NeighborRelation get_up_relation(const Array2D& array)const
        {
            if ( z == 0) return class_type::NoPath;
            ConnectorType up = array[z-1][x];
            if (up == UnknownOrCount) return class_type::NoNeighbor;
            if (is_one_of<D,DL,UD,RD,URD,RDL,DLU,LURD>(up)
               )
                return class_type::HasPath;
            return class_type::NoPath;
        }
        NeighborRelation get_right_relation(const Array2D& array)const
        {
            if (x == array[z].size() - 1) return class_type::NoPath;
            ConnectorType right = array[z][x+1];
            if(right == UnknownOrCount) return class_type::NoNeighbor;
            if (is_one_of<L,LU,DL,LR,LUR,RDL,DLU,LURD>(right))
                return class_type::HasPath;
            return class_type::NoPath;

        }
        NeighborRelation get_down_relation(const Array2D& array)const
        {
            if ( z == array.size() - 1) return class_type::NoPath;
            ConnectorType down = array[z+1][x];
            if (down == UnknownOrCount) return class_type::NoNeighbor;
            if (is_one_of<U,LU,UR,UD,LUR,URD,DLU,LURD>(down))
                return class_type::HasPath;
            return class_type::NoPath;
        }

        void try_place(Array2D& array)const
        {
            if (array.empty()) return ;
            array[z][x] = pick_current_connector(array, get_left_relation(array),get_up_relation(array),get_right_relation(array),get_down_relation(array));
            return;
        }
    public:
        place_step():x(0),z(0){
            init_condition_map();
        }


        void operator()(Array2D& array)
        {
            if( x < array[z].size() )
            {
                //keep trying place at z x
                if ( array[z][x] == UnknownOrCount)
                    try_place(array);
                x += 1;
            }
            else if( z < array.size() )
            {
                //keep trying place at z x
                if (x < array[z].size() &&  array[z][x] == UnknownOrCount)
                    try_place(array);
                z += 1; 
                x = 0;
            }
        }
};


#include <iostream>
#include <cstdio>
void test(int width,int height)
{
    typedef std::vector<std::vector<ConnectorType> > array2d_t;
    array2d_t array;
    std::vector<ConnectorType> elem;
    elem.resize(width,UnknownOrCount);
    array.resize(height, elem);
    place_step<array2d_t> step;
    is_complete p;
    loop_action(array,p,step);
    for(size_t i = 0;i < array.size(); ++i)
    {
        for(size_t j = 0; j < array[i].size(); ++j)
        {
            printf("%s",ConnectTypeStr[array[i][j]] );
        }
        printf("\n");
    }
}
int main()
{
    for(size_t i = 2;i < 20;++i)
    {
        for(size_t j = 2; j < 20; ++j)
        {
            test(i,j);
            printf("\n\n");
        }
    }
    return 0;

}
