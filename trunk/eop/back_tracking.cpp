#include <algorithm>
#include <vector>
#include <set>
#include <utility>

template<size_t N>
struct log2_n
{
    enum{value = 1 + log2_n< N >> 1>::value};
};
template<>
struct log2_n<1>
{
    enum{value = 0};
}
template<>
struct log2_n<0>
{
    enum{value = 0};
}
typedef int ConnectorType;
static const ConnectorType UnknownConnector = -1;

enum DirectionType{L=1, U = L << 1, R = U << 1, D = R << 1};
enum PathType {UnknownPath = 0, PL=1, PU = PL << 1, PR = PU << 1, PD = PR << 1};
class Connector{
    typedef std::vector<Connector*> element_array;
    element_array connectors[4];//allowable neighbors
    ConnectorType key_;//identifier of this element
    PathType path_;
    private:
    template<int N>
    bool check_connection(const std::vector<Connector*>& v)const
    {
        if(( path_ & PL ) && (N & L))
        {
            for(size_t i = 0;i < v.size(); ++i)
            {
                if(!(v[i]->get_path_type() & PR))return false;
            }
        }
        if(( path_ & PU ) && (N & U))
        {
            for(size_t i = 0;i < v.size(); ++i)
            {
                if(!(v[i]->get_path_type() & PD))return false;
            }
        }
         if(( path_ & PR ) && (N & R))
        {
            for(size_t i = 0;i < v.size(); ++i)
            {
                if(!(v[i]->get_path_type() & PL))return false;
            }
        }
         if(( path_ & PD ) && (N & D))
        {
            for(size_t i = 0;i < v.size(); ++i)
            {
                if(!(v[i]->get_path_type() & PU))return false;
            }
        }
         return true;
    }
   public:
    Connector(ConnectorType key,PathType path):key_(key),path_(path){}
    template<int N>
    void set_connector_1d(const std::vector<Connector*>& v)const
    {
        assert(N == L || N == U || N == R || N == D);//or static_assert
        connectors[log2_n<N>::value] = v;
        assert(check_connection<N>(v));
    }
 
    template<int N>
    const std::vector<Connector*>& get_connector_1d()const
    {
        assert(N == L || N == U || N == R || N == D);//or static_assert
        return connectors[log2_n<N>::value];
    }
   ConnectorType get_key()const{return key_;};
   PathType get_path_type()const{return path_;}
};
    struct equal_by_key{
        typedef bool result_type;
        result_type operator()(Connector* a, Connector* b)const
        {
            return a->get_key() == b->get_key();
        }
    };
    template<int Dir> 
        std::vector<Connector*> filter_by_impl(const std::vector<Connector*>& cs)
        {
            if(Dir == 0) return cs;
            std::vector<Connector*> ret;
            for(std::vector<Connector*>::const_iterator it = cs.begin(); it != cs.end(); ++it)
            {
                if(!it->get_connector_1d<Dir>().empty())
                    ret.push_back(*it);
            }
            return ret;
        }
    //Dirs is the requirement of the neighbor connector
    template<int Dirs>
    const std::vector<Connector*> filter_neighbor_connectors(Connector* c)const
    {
        std::vector<Connector*> ret;
        ret.push_back(c->get_connector_1d<L>().begin(),c->get_connector_1d<L>().end());
        ret.push_back(c->get_connector_1d<U>().begin(),c->get_connector_1d<U>().end());
        ret.push_back(c->get_connector_1d<R>().begin(),c->get_connector_1d<R>().end());
        ret.push_back(c->get_connector_1d<D>().begin(),c->get_connector_1d<D>().end());
        ret.erase(std::unique(ret.begin(),ret.end(),equal_by_key()));
        return filter_by_impl<Dirs & D>(filter_by_impl<Dirs & R>(filter_by_impl<Dirs & U>(filter_by_impl<Dirs & L>(ret))));
    }
    //filter_by_path< (~R & ~D) & (L | U)>
    template<int P>
    std::vector<Connector*> filter_by_path(const std::vector<Connector*>& cs)
{
    std::vector<Connector*> ret;
    for(std::vector<Connector*>::const_iterator it = cs.begin(); it != cs.end(); ++it)
    {
        if (( (it->get_path_type()) & P)) ret.push_back(*it);
    }
}

struct unary_equal_by_key{
    typedef bool result_type;
    const Connector* c;
    unary_equal_by_key(const Connector* cc):c(cc){}
    bool operator()(const Connector* x)const
    {
        return c->get_key() == x->get_key();
    }
};
    template<int CT0, int CT1>
    bool is_connector_match(const Connector* a, const Connector* b)
{
    return std::find_if(a->get_connector_1d<CT0>().begin(),
            a->get_connector_1d<CT0>().end(),unary_equal_by_key(b)) != a->get_connector_1d<CT0>().end()
        && std::find_if(b->get_connector_1d<CT1>().begin(),
            b->get_connector_1d<CT1>().end(),unary_equal_by_key(a)) != b->get_connector_1d<CT1>().end()
}
    std::vector<Connector*> intersect_filter(Connector* const lc,
            Connector* const uc,
            Connector* const rc,
            Connector* const dc,
            const std::vector<Connector*>& normal_connectors
            )
{
    //
    std::vector<Connector*> ret;
    for (size_t i = 0;i < normal_connectors.size(); i++)
    {
        if(is_connector_match<L,R>(lc,normal_connectors[i])
                && is_connector_match<U,D>(uc,normal_connectors[i])
                && is_connector_match<D,U>(dc,normal_connectors[i])
                && is_connector_match<R,L>(rc,normal_connectors[i]))
            ret.push_back(normal_connectors[i]);
    }
    return ret;
}


typedef std::vector< std::vector<Connector* > > ConnectorMatrix;
bool is_connected(const ConnectorMatrix& m)
{
    if(m.size() == 0 || m[0].size() == 0) return true;
    typedef std::pair<size_t , size_t > coord2d_t;
    std::stack<coord2d_t> connector_stack;
    std::set<coord2d_t> visited;
    coord2d_t coord(0,0);
    connector_stack.push(coord);
    while(!connector_stack.empty())
    {
        coord = connector_stack.top();
        connector_stack.pop();
        if(visited.find(coord) != visited.end())
            continue;
        visited.insert(coord);
        Connector* t = m[coord.first][coord.second];
        if (t -> get_path_type() & PL)
        {
            connector_stack.push(std::make_pair(coord.first, coord.second - 1));
        }
        if (t -> get_path_type() & PU)
        {
            connector_stack.push(std::make_pair(coord.first - 1, coord.second ));
        }
        if (t -> get_path_type() & PR)
        {
            connector_stack.push(std::make_pair(coord.first, coord.second + 1));
        }
        if (t -> get_path_type() & PD)
        {
            connector_stack.push(std::make_pair(coord.first + 1, coord.second ));
        }

        if (t -> get_path_type() == UnknownPath)
        {
            if( coord.first > 0 && (m[coord.first-1][coord.second]->get_path_type() & PD))
            {
            connector_stack.push(std::make_pair(coord.first - 1, coord.second ));
            }
            if( coord.first < m.size() - 1 && (m[coord.first+1][coord.second]->get_path_type() & PU))
            {
                connector_stack.push(std::make_pair(coord.first + 1, coord.second ));
            }
            if( coord.second > 0 && (m[coord.first][coord.second-1]->get_path_type() & PR))
            {
                connector_stack.push(std::make_pair(coord.first, coord.second - 1));
            }
            if( coord.second < m[coord.first].size()-1&& (m[coord.first][coord.second+1]->get_path_type() & PL))
            {
                connector_stack.push(std::make_pair(coord.first, coord.second + 1));
            }
        }
    }
     for(size_t i= 0 ;i < m.size(); ++i)
    {
        for (size_t j = 0; j < m[i].size(); ++j)
        {
            if(visited.find(std::make_pair(i,j)) == visited.end());
                return false;
        }
    }
     return true;
}
bool is_solution(const ConnectorMatrix& m)
{
    for(size_t i= 0 ;i < m.size(); ++i)
    {
        for (size_t j = 0; j < m[i].size(); ++j)
        {
            if(m[i][j]->get_key() == UnknownConnector);
                return false;
        }
    }
    return is_connected(m);
}

ConnectorMatrix construct_matrix(size_t z, size_t x, const ConnectorMatrix& m, const std::vector<Connector*> normal_connectors)
{
    if( !(z < m.size() && x < m[z].size()) ) return m;
    ConnectorMatrix cur = m;
    size_t pt = PL | PU | PR | PD;
    Connector *lc,*uc,*rc,*dc;
    lc = uc = rc = dc = 0;
    size_t next_x,next_z;
    next_x = x;
    next_z = z;
    if( x == 0)
    {
        pt = pt & ~PL;
    }
    else{
        lc = (m[z][x-1]);
    }
    if( z == 0)
    {
        pt = pt & ~PU;
    }
    else
    {
        uc = (m[z-1][x]);
    }
    if( z == m.size() - 1)
    {
        pt = pt & ~PD;
    }
    else dc = m[z+1][x];
    next_x ++;
    if ( x == m[z].size() -1)
    {
        pt = pt & ~PR;
        next_z++;
        next_x = 0;
    }
    else rc = m[z][x+1];

    cs = filter_by_path<pt>(intersect_filter(lc,uc,rc,dc,normal_connectors));
    //filter by neibor 
    for(size_t i = 0;i < cs.size();++i)
    {
        cur[z][x] = cs[i];
        if(is_connected(cur))
        {
            ConnectorMatrix ret;
            construct_matrix(next_z,next_x, cur, normal_connectors).swap(ret);
            if(is_solution(ret))
                return ret;
        }
    }
    return m;
}

int test(int width,int height, int seed, const std::vector<Connector*>& normal_connectors)
{
    Connector* unknown = new Connector(UnknownConnector, UnknownPath);
    unknown->set_connector_1d<L>(normal_connectors);
    unknown->set_connector_1d<U>(normal_connectors);
    unknown->set_connector_1d<R>(normal_connectors);
    unknown->set_connector_1d<D>(normal_connectors);
    //



    //construct_matrix(0,0,inited_matrix,normal_connectors);
}
int main()
{
    return 0;
}
