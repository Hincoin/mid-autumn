#include <algorithm>
#include <vector>
#include <set>
#include <stack>
#include <utility>
#include <cassert>

template<size_t N>
struct log2_n
{
    enum{value = 1 + log2_n< (N >> 1) >::value};
};
template<>
struct log2_n<1>
{
    enum{value = 0};
};
template<>
struct log2_n<0>
{
    enum{value = 0};
};

typedef int ConnectorType;
static const ConnectorType UnknownConnector = -1;

enum DirectionType{L=1, U = L << 1, R = U << 1, D = R << 1};
enum PathType {UnknownPath = 0, 
    PL=0x01,PR = PL << 1,PU = PR << 1,PD = PU << 1};
template<int X>
struct dir_to_path{
    enum{value = X};
};
template<int X>
struct opposite_dir;
template<>
struct opposite_dir<L>{enum {value = R};};
template<>
struct opposite_dir<U>{enum {value = D};};
template<>
struct opposite_dir<R>{enum {value = L};};
template<>
struct opposite_dir<D>{enum {value = U};};

class Connector;

class Connector{
    typedef std::vector<Connector*> element_array;
    element_array connectors[4];//allowable neighbors
    int key_;//identifier of this element
    int path_;
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
    Connector(int key,int path):key_(key),path_(path){}
    template<int N>
        void set_connector_1d(const std::vector<Connector*>& v)
        {
            assert(N == L || N == U || N == R || N == D);//or static_assert
            connectors[log2_n<N>::value] = v;
            assert(check_connection<N>(v));
        }
    template<int N> 
        void add_connector_1d(Connector* v);
    template<int N> 
        void add_connector_1d(const std::vector<Connector*>& v)
        {
            for(size_t i = 0;i < v.size(); ++i)
                add_connector_1d<N>(v[i]);
        }

    template<int N>
        const std::vector<Connector*>& get_connector_1d()const
        {
            assert(N == L || N == U || N == R || N == D);//or static_assert
            return connectors[log2_n<N>::value];
        }
    ConnectorType get_key()const{return key_;};
    int get_path_type()const{return path_;}
};
struct unary_equal_by_key{
    typedef bool result_type;
    const Connector* c;
    unary_equal_by_key(const Connector* cc):c(cc){}
    bool operator()(const Connector* x)const
    {
        return c->get_key() == x->get_key();
    }
};

    template<int N>
void Connector::add_connector_1d(Connector* v)
{
    std::vector<Connector*>& a = connectors[log2_n<N>::value];
    if(std::find_if(a.begin(),a.end(),unary_equal_by_key(v)) != a.end())
        return;
    assert(N == L || N == U || N == R || N == D);//or static_assert
    connectors[log2_n<N>::value].push_back(v);
    assert(check_connection<N>(connectors[log2_n<N>::value]));
    v->add_connector_1d<opposite_dir<N>::value>(this);
}

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
        if(! (*it)->get_connector_1d<Dir>().empty())
            ret.push_back(*it);
    }
    return ret;
}
std::vector<Connector*> filter_by_key(int K, const std::vector<Connector*>& cs)
{
    if(K == 0) return cs;
    std::vector<Connector*> ret;
    for(std::vector<Connector*>::const_iterator it = cs.begin(); it != cs.end(); ++it)
    {
        if((*it)->get_key() & K)
            ret.push_back(*it);
    }
    return ret;
}
std::vector<Connector*> filter_by_not_key(int K, const std::vector<Connector*>& cs)
{
    if(K == 0) return cs;
    std::vector<Connector*> ret;
    for(std::vector<Connector*>::const_iterator it = cs.begin(); it != cs.end(); ++it)
    {
        if(!((*it)->get_key() & K))
            ret.push_back(*it);
    }
    return ret;
}

//filter_by_path< (~R & ~D) & (L | U)>
std::vector<Connector*> filter_by_path(int P, const std::vector<Connector*>& cs)
{
    std::vector<Connector*> ret;
    for(std::vector<Connector*>::const_iterator it = cs.begin(); it != cs.end(); ++it)
    {
        if (( ((*it)->get_path_type()) & P)) ret.push_back(*it);
    }
    return ret;
}
std::vector<Connector*> filter_by_path_strict(int P, const std::vector<Connector*>& cs)
{
    std::vector<Connector*> ret;
    for(std::vector<Connector*>::const_iterator it = cs.begin(); it != cs.end(); ++it)
    {
        if (( ((*it)->get_path_type()) & P) == (*it)->get_path_type()) ret.push_back(*it);
    }
    return ret;
}

std::vector<Connector*> filter_by_path_not(int P, const std::vector<Connector*>& cs)
{
    std::vector<Connector*> ret;
    for(std::vector<Connector*>::const_iterator it = cs.begin(); it != cs.end(); ++it)
    {
        if (!( ((*it)->get_path_type()) & P)) ret.push_back(*it);
    }
    return ret;
}


    template<int CT1, int CT0>
bool is_connector_match(const Connector* a, const Connector* b)
{
    assert(CT0 == opposite_dir<CT1>::value);
    return std::find_if(a->get_connector_1d<CT0>().begin(),
            a->get_connector_1d<CT0>().end(),unary_equal_by_key(b)) != a->get_connector_1d<CT0>().end()
        && std::find_if(b->get_connector_1d<CT1>().begin(),
                b->get_connector_1d<CT1>().end(),unary_equal_by_key(a)) != b->get_connector_1d<CT1>().end();
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
        if((!lc || lc->get_key() == UnknownConnector ||  is_connector_match<L,R>(lc,normal_connectors[i]))
                && (!uc || uc->get_key() == UnknownConnector || is_connector_match<U,D>(uc,normal_connectors[i]))
                && (!dc || dc->get_key() == UnknownConnector || is_connector_match<D,U>(dc,normal_connectors[i]))
                && (!rc || rc->get_key() == UnknownConnector || is_connector_match<R,L>(rc,normal_connectors[i])))
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
            if( coord.first > 0 && 
                    ((m[coord.first-1][coord.second]->get_path_type() & PD) ||
                     m[coord.first-1][coord.second]->get_path_type() == UnknownPath))
            {
                connector_stack.push(std::make_pair(coord.first - 1, coord.second ));
            }
            if( coord.first < m.size() - 1 && 
                    ((m[coord.first+1][coord.second]->get_path_type() & PU) ||
                     m[coord.first+1][coord.second]->get_path_type() == UnknownPath
                    ))
            {
                connector_stack.push(std::make_pair(coord.first + 1, coord.second ));
            }
            if( coord.second > 0 && 
                    ((m[coord.first][coord.second-1]->get_path_type() & PR) ||
                     m[coord.first][coord.second-1]->get_path_type() == UnknownPath
                    ))
            {
                connector_stack.push(std::make_pair(coord.first, coord.second - 1));
            }
            if( coord.second < m[coord.first].size()-1&& 
                    ((m[coord.first][coord.second+1]->get_path_type() & PL) ||
                     m[coord.first][coord.second+1]->get_path_type() == UnknownPath 
                    ))
            {
                connector_stack.push(std::make_pair(coord.first, coord.second + 1));
            }
        }
    }
    for(size_t i= 0 ;i < m.size(); ++i)
    {
        for (size_t j = 0; j < m[i].size(); ++j)
        {
            if(visited.find(std::make_pair(i,j)) == visited.end())
                return false;
        }
    }
    return true;
}
bool is_solution_matrix(const ConnectorMatrix& m)
{
    for(size_t i= 0 ;i < m.size(); ++i)
    {
        for (size_t j = 0; j < m[i].size(); ++j)
        {
            if(m[i][j]->get_key() == UnknownConnector)
                return false;
        }
    }
    return is_connected(m);
}

ConnectorMatrix construct_matrix(size_t z, size_t x, const ConnectorMatrix& m, const std::vector<Connector*> normal_connectors)
{
    if( !(z < m.size() && x < m[z].size()) ) return m;
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
    std::vector<Connector*> cs = filter_by_path_strict(pt, intersect_filter(lc,uc,rc,dc,normal_connectors));
    std::random_shuffle(cs.begin(),cs.end());
    ConnectorMatrix cur = m;
    //filter by neibor 
    for(size_t i = 0;i < cs.size();++i)
    {
        cur[z][x] = cs[i];
        if(is_connected(cur))
        {
            ConnectorMatrix ret;
            construct_matrix(next_z,next_x, cur, normal_connectors).swap(ret);
            if(is_solution_matrix(ret))
                return ret;
        }
    }
    return m;
}

    struct connector_matrix_state{
        private:
        ConnectorMatrix m;
        size_t x,z;//2d coord visitor
        Connector* unknown_connector;
        public:
        connector_matrix_state(const ConnectorMatrix& cm, Connector* u):m(cm),x(0),z(0),unknown_connector(u){}
        void next(Connector* c){
            assert(z < m.size() && x < m[z].size());
            m[z][x] = c;
            if ( x == m[z].size() -1)
            {
                z++;
                x = 0;
            }
            else
                x++;
        }
        void prev(){
            assert(x > 0 || z > 0);
            if(z < m.size() && x < m[z].size())
                m[z][x] = unknown_connector;
            if(x > 0) x--;
            else
            {
                z--;
                assert(!m[z].empty());
                x = m[z].size() - 1;
            }
        }
        const ConnectorMatrix& get_matrix()const{return m;}
        size_t get_x()const{return x;}
        size_t get_z()const{return z;}
    };

namespace back_tracking{
    bool acceptable(connector_matrix_state& state, Connector* c)
    {
        size_t x = state.get_x();
        size_t z = state.get_z();
        ConnectorMatrix& m = const_cast<ConnectorMatrix&>(state.get_matrix());

        if( !(z < m.size() && x < m[z].size()) ) return false;
        Connector *lc,*uc,*rc,*dc;
        lc = uc = rc = dc = 0;
        int pt = PL | PU | PD | PR;
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
        if ( x == m[z].size() -1)
        {
            pt = pt & ~PR;
        }
        else rc = m[z][x+1];
        if ((c->get_path_type() & pt) != c->get_path_type()) 
        {
            return false;
        }

        if((!lc || lc->get_key() == UnknownConnector ||  is_connector_match<L,R>(lc,c))
                && (!uc || uc->get_key() == UnknownConnector || is_connector_match<U,D>(uc,c))
                && (!dc || dc->get_key() == UnknownConnector || is_connector_match<D,U>(dc,c))
                && (!rc || rc->get_key() == UnknownConnector || is_connector_match<R,L>(rc,c)))
        {
            //test connectivity 
            state.next(c);
            bool ret = is_connected(m);
            state.prev();
            return ret;
        } 
        return false;
    }
    void successor_action(connector_matrix_state& state, Connector* c)
    {
        state.next(c);
    }
    void predecessor_action(connector_matrix_state& state)
    {
        state.prev();
    }
    bool is_solution(const connector_matrix_state& state)
    {
        return is_solution_matrix(state.get_matrix());
    }
}
namespace back_tracking{
	struct no_shuffle{
		template<typename I>
			void operator()(I , I )const
			{}
	};
	struct default_shuffle{
		template<typename I>
			void operator()(I f, I l)const
			{
                std::random_shuffle(f,l);
            }
	};


	template<typename R, typename TI>
	inline void back_tracking_step(R& state, TI& f, TI tf, TI tl, std::stack<TI>& s)
	{
		if (acceptable(state, *f))//change this to try_next
		{
			successor_action(state, *f);
            s.push(++f);//store next start pos
			f = tf;
			return;
		}
        ++f;
        while (!s.empty() && f == tl)
        {
            f = s.top();
            s.pop();
            predecessor_action(state);
        }
	}
	template<typename R, typename TI, typename S>
		bool back_tracking_iterate(R& state, TI tf, TI tl, S shuffle)
		{
			TI f = tf;
            std::stack<TI> s;
			while(f != tl)
			{
				back_tracking_step(state, f, tf, tl, s);
				if (f == tf && is_solution(state)) return true;
				if ( f == tf) shuffle(f, tl);
			}
			return false;
		}
	//EOP:this is an action that will change the input value
	//bidirectional state required(reversable)
template<typename R,typename TI, typename S>
bool back_tracking_recursive(R& state,TI tf, TI tl,S shuffle)
{
	if(is_solution(state)) return true;
	TI f = tf;
	shuffle(f,tl);
	while(f != tl )
	{
        assert(!is_solution(state));
		if(acceptable(state, *f)) //change this to try_next
		{
			successor_action(state,*f);
			if(back_tracking_recursive(state,tf,tl,shuffle))
				return true;
			predecessor_action(state);
		}
		++f;	
	}
	return false;//empty result
}
	//this is an action change the input value
template<typename R,typename TI>
inline bool back_tracking_recursive(R& state,TI tf, TI tl)
{
	return back_tracking_recursive(state, tf, tl, no_shuffle());
}
}


ConnectorMatrix test(int width,int height, int seed, const std::vector<Connector*>& normal_connectors)
{
    std::vector<Connector*> input_connectors = normal_connectors;
    srand(seed);
    std::random_shuffle(input_connectors.begin(), input_connectors.end());
    Connector* unknown = new Connector(UnknownConnector, UnknownPath);
    unknown->set_connector_1d<L>(normal_connectors);
    unknown->set_connector_1d<U>(normal_connectors);
    unknown->set_connector_1d<R>(normal_connectors);
    unknown->set_connector_1d<D>(normal_connectors);

    std::vector<Connector*> row(width, unknown);
    ConnectorMatrix inited_matrix(height,row);
    //
    connector_matrix_state s(inited_matrix,unknown); 
    //back_tracking::back_tracking_recursive(s, input_connectors.begin(), input_connectors.end(),back_tracking::default_shuffle());
    back_tracking::back_tracking_iterate(s, input_connectors.begin(), input_connectors.end(), back_tracking::default_shuffle());
    return s.get_matrix();
}
//left,right,up,down
enum TestConnectorType{
    TUnknownConnector=0x0,
    TL=0x01,TR = TL << 1,TU = TR << 1,TD = TU << 1, 
    TUR = TU | TR, TRD = TR | TD, TDL = TD|TL, TLU = TL|TU , TLR = TL|TR, TUD = TU|TD, 
    TLUR = TL|TUR,TDLU = TD|TLU ,TRDL = TR|TDL,TURD=TU|TRD,
    TLURD = TLUR | TD
};

//for print
static const char* TestConnectTypeStr[] = 
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

std::vector<Connector*> init_connectors()
{
    std::vector<Connector*> ret;
    //Connector* l = new Connector(TL,PL);ret.push_back(l);
    //Connector* r = new Connector(TR,PR);ret.push_back(r);
    Connector* lr = new Connector(TLR,PL|PR);ret.push_back(lr);
    //Connector* u = new Connector(TU,PU);ret.push_back(u);
    Connector* lu = new Connector(TLU, PL|PU);ret.push_back(lu);
    Connector* ur = new Connector(TUR, PR|PU);ret.push_back(ur);
    Connector* lur = new Connector(TLUR, PL|PU|PR);ret.push_back(lur);
    //Connector* d = new Connector(TD, PD);ret.push_back(d);
    Connector* dl = new Connector(TDL, PD|PL);ret.push_back(dl);
    Connector* rd = new Connector(TRD, PR|PD);ret.push_back(rd);
    Connector* rdl = new Connector(TRDL, PR|PD|PL);ret.push_back(rdl);
    Connector* ud = new Connector(TUD, PU|PD);ret.push_back(ud);
    Connector* dlu = new Connector(TDLU, PD|PL|PU);ret.push_back(dlu);
    Connector* urd = new Connector(TURD, PU|PR|PD);ret.push_back(urd);
    //Connector* lurd = new Connector(TLURD, PL|PU|PR|PD);ret.push_back(lurd);

    std::vector<Connector*> ls,nls,us,nus,rs,nrs,ds,nds;
    ls = filter_by_key(TL,ret);nls = filter_by_not_key(TL,ret);
    us = filter_by_key(TU,ret);nus = filter_by_not_key(TU,ret);
    rs = filter_by_key(TR,ret);nrs = filter_by_not_key(TR,ret);
    ds = filter_by_key(TD,ret);nds = filter_by_not_key(TD,ret);
/*
    l->add_connector_1d<L>(rs);
    l->add_connector_1d<U>(nds);
    l->add_connector_1d<R>(nls);
    l->add_connector_1d<D>(nus);

    u->add_connector_1d<L>(nrs);
    u->add_connector_1d<U>(ds);
    u->add_connector_1d<R>(nls);
    u->add_connector_1d<D>(nus);


    r->add_connector_1d<L>(nrs);
    r->add_connector_1d<U>(nds);
    r->add_connector_1d<R>(ls);
    r->add_connector_1d<D>(nus);

    d->add_connector_1d<L>(nrs);
    d->add_connector_1d<U>(nds);
    d->add_connector_1d<R>(nls);
    d->add_connector_1d<D>(us);


    assert((!is_connector_match<L,R>(r,d)));
    */
    lr->add_connector_1d<L>(rs);
    lr->add_connector_1d<U>(nds);
    lr->add_connector_1d<R>(ls);
    lr->add_connector_1d<D>(nus);

    lu->add_connector_1d<L>(rs);
    lu->add_connector_1d<U>(ds);
    lu->add_connector_1d<R>(nls);
    lu->add_connector_1d<D>(nus);

    dl->add_connector_1d<L>(rs);
    dl->add_connector_1d<U>(nds);
    dl->add_connector_1d<R>(nls);
    dl->add_connector_1d<D>(us);

    ur->add_connector_1d<L>(nrs);
    ur->add_connector_1d<U>(ds);
    ur->add_connector_1d<R>(ls);
    ur->add_connector_1d<D>(nus);

    rd->add_connector_1d<L>(nrs);
    rd->add_connector_1d<U>(nds);
    rd->add_connector_1d<R>(ls);
    rd->add_connector_1d<D>(us);

    ud->add_connector_1d<L>(nrs);
    ud->add_connector_1d<U>(ds);
    ud->add_connector_1d<R>(nls);
    ud->add_connector_1d<D>(us);

    lur->add_connector_1d<L>(rs);
    lur->add_connector_1d<U>(ds);
    lur->add_connector_1d<R>(ls);
    lur->add_connector_1d<D>(nus);

    rdl->add_connector_1d<L>(rs);
    rdl->add_connector_1d<U>(nds);
    rdl->add_connector_1d<R>(ls);
    rdl->add_connector_1d<D>(us);

    dlu->add_connector_1d<L>(rs);
    dlu->add_connector_1d<U>(ds);
    dlu->add_connector_1d<R>(nls);
    dlu->add_connector_1d<D>(us);

    urd->add_connector_1d<L>(nrs);
    urd->add_connector_1d<U>(ds);
    urd->add_connector_1d<R>(ls);
    urd->add_connector_1d<D>(us);

   // lurd->add_connector_1d<L>(rs);
   // lurd->add_connector_1d<U>(ds);
   // lurd->add_connector_1d<R>(ls);
   // lurd->add_connector_1d<D>(us);

    return ret;
}

#include <iostream>
#include <cstdlib>
#include <cstdio>

void output_matrix(const ConnectorMatrix& m)
{
                for(size_t z = 0; z < m.size(); ++z)
                {
                    for(size_t x = 0; x < m[z].size(); ++x)
                    {
                        printf("%s",TestConnectTypeStr[m[z][x]->get_key()]);
                    }
                    printf("\n");
                }
                printf("\n");
 
}
void test_case()
{
    std::vector<Connector*> cs = init_connectors();
    int z = 14;
    int x = 3;
    ConnectorMatrix m = test(z,x,z*20 + x,cs);
    assert(is_solution_matrix(m));
    output_matrix(m); 
    for(int i = 2;i < 10; ++i)
        for(int j = 2;j < 10; ++j)
        {
            ConnectorMatrix m = test(i,j,i*20 + j,cs);
            assert(is_solution_matrix(m));
            if(is_solution_matrix(m))
            {   //output 
                output_matrix(m);
            }
        }

}
int main()
{
    test_case();
    return 0;
}

