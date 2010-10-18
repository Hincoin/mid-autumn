#include <utility>
#include <algorithm>
#include <vector>
#include <stack>
#include <set>
#include <cassert>

#include <cstdlib>


    template<typename T, typename P, typename F>
inline void loop_action(T& v, P& p,F& f)
{
    while (!p(v))
    {
        f(v);
    }
}


//left,right,up,down
enum ConnectorType{
    UnknownConnector=0x0,
    L=0x01,R = L << 1,U = R << 1,D = U << 1, 
    UR = U | R, RD = R | D, DL = D|L, LU = L|U , LR = L|R, UD = U|D, 
    LUR = L|UR,DLU = D|LU ,RDL = R|DL,URD=U|RD,
    LURD = LUR | D
};

//for print
static const char* ConnectTypeStr[] = 
{ 
    "UnknownOrCount",
    "L    ",//01
    "R    ",//02
    "LR   ",//03
    "U    ",//04
    "LU   ",//05
    "UR   ",//06
    "LUR  ",//07
    "D    ",//08
    "DL   ",//09
    "RD   ",//10
    "RDL  ",//11
    "UD   ",//12 
    "DLU  ",//13
    "URD  ",//14
    "LURD ",//15
};
    template<int C>
bool is_one_of(ConnectorType c)
{
    return (C & c) != 0;
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
                ConnectorType conn = array[t.first][t.second];
                if (is_one_of<L>(conn))
                    coord_stack.push(go_left(array,t));
                if (is_one_of<U>(conn))
                    coord_stack.push(go_up(array,t));
                if (is_one_of<R>(conn))
                    coord_stack.push(go_right(array,t));
                if (is_one_of<D>(conn))
                    coord_stack.push(go_down(array,t));
                if (conn == UnknownConnector)
                {
                    if (t.first > 0)
                    {
                        ConnectorType c = array[t.first - 1][t.second];
                        if(c == UnknownConnector|| 
                                is_one_of<D>(c))
                        {
                            coord_stack.push(go_up(array,t));
                        }
                    }
                    if (t.first+1 < array.size())
                    {
                        ConnectorType c = array[t.first + 1][t.second];
                        if (c == UnknownConnector|| 
                                is_one_of<U>(c))
                            coord_stack.push(go_down(array,t));
                    }
                    if (t.second > 0)
                    {
                        ConnectorType c = array[t.first][t.second - 1];
                        if (c == UnknownConnector|| 
                                is_one_of<R>(c))
                            coord_stack.push(go_left(array,t));
                    }
                    if (t.second + 1 < array[t.first].size())
                    {
                        ConnectorType c = array[t.first][t.second + 1];
                        if (c == UnknownConnector || 
                                is_one_of<L>(c))
                        {
                            coord_stack.push(go_right(array,t));
                        }
                    }

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
                    if (array[i][j] == UnknownConnector) return false;
                }
            return check_connection()(array);
        }
};
struct default_random_policy{
    typedef std::ptrdiff_t value_type;
    //returns a value belongs to [0,m)
    value_type operator()(value_type m)
    {
        double tmp ;
        tmp = static_cast<double>(rand()) / static_cast<double>(RAND_MAX);
        return static_cast<value_type>(tmp * m);
    }
};
template<typename Array2D, typename RNG = default_random_policy>
struct place_step
{
    enum NeighborRelation{NoPath, HasPath, NoNeighbor, RelationCount}; 
    typedef typename Array2D::size_type size_type;
    typedef std::vector<ConnectorType> connector_array_t;
    typedef place_step<Array2D, RNG> class_type;
    public:
    explicit place_step(RNG* r = 0):x(0),z(0),rng(r){
        init_condition_map();
    }
    void operator()(Array2D& array)
    {
        assert(z < array.size());
        if( x < array[z].size() )
        {
            //keep trying place at z x
            if ( array[z][x] == UnknownConnector)
                try_place(array);
            x += 1;
        }
        else if( z < array.size() )
        {
            //keep trying place at z x
            if (x < array[z].size() &&  array[z][x] == UnknownConnector)
                try_place(array);
            z += 1; 
            x = 0;
        }
    }
    private:
    void init_condition_map()
    {
        ConnectorType all_cs[]=
        {
            L,R,U,D, 
            UR,RD,DL,LU,LR,UD, 
            LUR,DLU,RDL,URD,
            LURD
        };
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
                            filter_by_conn<L>(l == HasPath, cs).swap(cs);
                        if (u != NoNeighbor)
                            filter_by_conn<U>(u == HasPath, cs).swap(cs);
                        if (r != NoNeighbor)
                            filter_by_conn<R>(r == HasPath, cs).swap(cs);
                        if (d != NoNeighbor)
                            filter_by_conn<D>(d ==HasPath, cs).swap(cs);
                        condition_map[l][u][r][d].swap(cs);
                    }
    }
    template<int _0>
        connector_array_t filter_by_conn(bool bHasPath, const connector_array_t& a)
        {
            connector_array_t ret;
            for (connector_array_t::const_iterator it = a.begin(); it != a.end();++it)
            {
                if( is_one_of<_0>(*it))
                {
                    if (bHasPath)
                        ret.push_back(*it);
                }
                else if (!bHasPath)
                    ret.push_back(*it);
            }
            return ret;
        }
    private:
    void try_place(Array2D& array)const
    {
        if (array.empty()) return ;
        array[z][x] = pick_current_connector(array, get_left_relation(array),get_up_relation(array),get_right_relation(array),get_down_relation(array));
        return;
    }
    NeighborRelation get_left_relation(const Array2D& array)const
    {
        if (x == 0) return class_type::NoPath;
        ConnectorType left = array[z][x-1];
        if (left == UnknownConnector) return class_type::NoNeighbor;
        if (is_one_of<R>(left))
            return class_type::HasPath;
        return class_type::NoPath;
    }
    NeighborRelation get_up_relation(const Array2D& array)const
    {
        if ( z == 0) return class_type::NoPath;
        ConnectorType up = array[z-1][x];
        if (up == UnknownConnector) return class_type::NoNeighbor;
        if (is_one_of<D>(up)
           )
            return class_type::HasPath;
        return class_type::NoPath;
    }
    NeighborRelation get_right_relation(const Array2D& array)const
    {
        if (x == array[z].size() - 1) return class_type::NoPath;
        ConnectorType right = array[z][x+1];
        if(right == UnknownConnector) return class_type::NoNeighbor;
        if (is_one_of<L>(right))
            return class_type::HasPath;
        return class_type::NoPath;

    }
    NeighborRelation get_down_relation(const Array2D& array)const
    {
        if ( z == array.size() - 1) return class_type::NoPath;
        ConnectorType down = array[z+1][x];
        if (down == UnknownConnector) return class_type::NoNeighbor;
        if (is_one_of<U>(down))
            return class_type::HasPath;
        return class_type::NoPath;
    }
    private:
    ConnectorType pick_current_connector(const Array2D& array, 
            NeighborRelation left, NeighborRelation up, NeighborRelation right, NeighborRelation down
            ) const
    {
        connector_array_t& a = const_cast<connector_array_t&>(condition_map[left][up][right][down]) ;
        if (a.empty() ){ assert(false); return UnknownConnector;}   

        //shuffle a
        if(rng)
            std::random_shuffle(a.begin(), a.end(), *rng);
        else
            std::random_shuffle(a.begin(), a.end());

        connector_array_t::iterator it = a.begin(); 
        check_end p(x,z,array,a);
        advance_it step;
        loop_action(it, p, step);
        if (it == a.end()) { assert(false); return UnknownConnector;}
        return *it;
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
        const connector_array_t& cs;
        size_type x,z;
        check_end(size_type x, size_type z,const Array2D& a,const connector_array_t& c):
            x(x),z(z),array(a),cs(c){}
        bool operator()(const connector_array_t::iterator& it)const
        {
            if (it == cs.end())
            {
                return true;
            }
            Array2D& tmp = const_cast<Array2D&> ( array );
            ConnectorType t = array[z][x];
            tmp[z][x] = *it;
            bool ret = *it == LURD || check_connection()(tmp);
            //restore back
            tmp[z][x] = t;
            return ret;
        }
    };
    private:
    size_type x,z;
    RNG* rng;
    connector_array_t condition_map[RelationCount][RelationCount][RelationCount][RelationCount];
};


#include <iostream>
#include <cstdio>
void test(int width,int height)
{
    typedef std::vector<std::vector<ConnectorType> > array2d_t;
    array2d_t array;
    std::vector<ConnectorType> elem;
    elem.resize(width,UnknownConnector);
    array.resize(height, elem);
    default_random_policy rng;
    place_step<array2d_t> step(&rng);
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
void exaust_test()
{
    for(size_t i = 2;i < 20;++i)
    {
        for(size_t j = 2; j < 20; ++j)
        {
            test(i,j);
            printf("\n\n");
        }
    }
    printf("\n");
}
int main()
{
    for(int i = 0;i < 10000; i++)
    {
        srand(i);
        exaust_test();
    }
    return 0;

}
