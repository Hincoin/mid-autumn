#ifndef _CONSTRUCT_BY_CONNECTION_HPP_
#define _CONSTRUCT_BY_CONNECTION_HPP_

#include <algorithm>
#include <vector>
#include <set>
#include <stack>
#include <queue>
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

struct less_than_by_key{
	bool operator()(const Connector* a, const Connector* b)const;
};
class Connector{
	//typedef std::vector<Connector*> element_array;
	typedef std::set<Connector*, less_than_by_key> element_set;
	element_set connectors[4];//allowable neighbors
	int key_;//identifier of this element
	int path_;
private:

	template< int N, typename I >
	bool check_connection(I f, I l)const
	{
		if(( path_ & PL ) && (N & L))
		{
			I i = f;
			while(i != l)
			{
				if(!((*i)->get_path_type() & PR))return false;
				++i;
			}
		}
		if(( path_ & PU ) && (N & U))
		{
			I i = f;
			while(i != l)
			{
				if(!((*i)->get_path_type() & PD))return false;
				++i;
			}
		}
		if(( path_ & PR ) && (N & R))
		{
			I i = f;
			while(i != l)
			{
				if(!((*i)->get_path_type() & PL))return false;
				++i;
			}
		}
		if(( path_ & PD ) && (N & D))
		{
			I i = f;
			while(i != l)
			{
				if(!((*i)->get_path_type() & PU))return false;
				++i;
			}
		}
		return true;
	}
public:
	Connector(int key,int path):key_(key),path_(path){}
	template<int N, typename I>
	void set_connector_1d(I f, I l)
	{
		assert(N == L || N == U || N == R || N == D);//or static_assert
		element_set(f,l).swap(connectors[log2_n<N>::value]) ;
		assert(check_connection<N>(f,l));
	}
	template<int N> 
	void add_connector_1d(Connector* v);
	template<int N, typename I > 
	void add_connector_1d(I f, I l)
	{
		while(f != l)
		{
			add_connector_1d<N>(*f);
			++f;
		}
	}

	template<int N>
	const element_set& get_connector_1d()const
	{
		assert(N == L || N == U || N == R || N == D);//or static_assert
		return connectors[log2_n<N>::value];
	}
	ConnectorType get_key()const{return key_;};
	int get_path_type()const{return path_;}
};

bool less_than_by_key::operator()(const Connector* a, const Connector* b)const
{
	return a->get_key() < b->get_key();
}
struct unary_equal_by_key{
	typedef bool result_type;
	const Connector* c;
	explicit unary_equal_by_key(const Connector* cc):c(cc){}
	bool operator()(const Connector* x)const
	{
		return c->get_key() == x->get_key();
	}
};

template<int N>
void Connector::add_connector_1d(Connector* v)
{
	element_set& a = connectors[log2_n<N>::value];
	if(std::find_if(a.begin(),a.end(),unary_equal_by_key(v)) != a.end())
		return;
	assert(N == L || N == U || N == R || N == D);//or static_assert
	a.insert(v);
	assert(check_connection<N>(a.begin(),a.end()));
	v->add_connector_1d<opposite_dir<N>::value>(this);
}

struct equal_by_key{
	typedef bool result_type;
	result_type operator()(Connector* a, Connector* b)const
	{
		return a->get_key() == b->get_key();
	}
};

std::vector<Connector*> filter_by_path_strict(int P, const std::vector<Connector*>& cs)
{
	std::vector<Connector*> ret;
	for(std::vector<Connector*>::const_iterator it = cs.begin(); it != cs.end(); ++it)
	{
		if (( ((*it)->get_path_type()) & P) == (*it)->get_path_type()) ret.push_back(*it);
	}
	return ret;
}


template<int CT1, int CT0>
bool is_connector_match(const Connector* a, const Connector* b)
{
	assert(CT0 == opposite_dir<CT1>::value);
	return a->get_connector_1d<CT0>().find(const_cast<Connector*>(b)) != a->get_connector_1d<CT0>().end();
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


inline bool is_connected(const ConnectorMatrix& m)	
{		
	if(m.size() == 0 || m[0].size() == 0) return true;		
	typedef std::pair<size_t , size_t > coord2d_t;		
	std::stack<coord2d_t> connector_stack;		
	std::vector<std::vector<bool> > visited;		
	//init visited flags		
	std::vector<bool> visited_row(m[0].size(),false);		
	visited.resize(m.size(),visited_row);		
	//std::set<coord2d_t> visited;		
	coord2d_t coord(0,0);		
	for(size_t i = 0;i < visited.size();++i)
		for(size_t j = 0;j < visited[i].size(); ++j)
		{
			if(!m[i][j])
			{
				visited[i][j] = true;//ignore empty
			}
			else
			{
				if(connector_stack.size() == 0)
				{
					coord.first = (size_t)i;
					coord.second= (size_t)j;
					connector_stack.push(coord);		
				}

			}
		}
		if (connector_stack.size() == 0) connector_stack.push(coord);

		while(!connector_stack.empty())		
		{		
			coord = connector_stack.top();		
			connector_stack.pop();		
			if(visited[coord.first][coord.second])		
				continue;		
			visited[coord.first][coord.second] = true;		
			Connector* t = m[coord.first][coord.second];		
			if (t -> get_path_type() == UnknownPath)		
			{		
				if( coord.first > 0 &&		
					m[coord.first-1][coord.second] &&
					((m[coord.first-1][coord.second]->get_path_type() & PD) ||		
					m[coord.first-1][coord.second]->get_path_type() == UnknownPath))		
				{		
					connector_stack.push(std::make_pair(coord.first - 1, coord.second ));		
				}		
				if( coord.first < m.size() - 1 &&		
					m[coord.first+1][coord.second] && 
					((m[coord.first+1][coord.second]->get_path_type() & PU) ||		
					m[coord.first+1][coord.second]->get_path_type() == UnknownPath		
					))		
				{		
					connector_stack.push(std::make_pair(coord.first + 1, coord.second ));		
				}		
				if( coord.second > 0 &&		
					m[coord.first][coord.second-1] &&
					((m[coord.first][coord.second-1]->get_path_type() & PR) ||		
					m[coord.first][coord.second-1]->get_path_type() == UnknownPath		
					))		
				{		
					connector_stack.push(std::make_pair(coord.first, coord.second - 1));		
				}		
				if( coord.second < m[coord.first].size()-1&&		
					m[coord.first][coord.second+1] &&
					((m[coord.first][coord.second+1]->get_path_type() & PL) ||		
					m[coord.first][coord.second+1]->get_path_type() == UnknownPath		
					))		
				{		
					connector_stack.push(std::make_pair(coord.first, coord.second + 1));		
				}		
				continue;		
			}		
			if (t -> get_path_type() & PL)		
			{		
				connector_stack.push(std::make_pair(coord.first, coord.second - 1));		
			}		
			if (t -> get_path_type() & PU)		
			{		
				connector_stack.push(std::make_pair(coord.first - 1, coord.second ));		
			}		
			if ( t -> get_path_type() & PR)		
			{		
				connector_stack.push(std::make_pair(coord.first, coord.second + 1));		
			}		
			if ( t -> get_path_type() & PD)		
			{		
				connector_stack.push(std::make_pair(coord.first + 1, coord.second ));		
			}		

		}		
		for(size_t i= 0 ;i < visited.size(); ++i)		
		{		
			for (size_t j = 0; j < visited[i].size(); ++j)		
			{		
				if(!visited[i][j])		
					return false;		
			}		
		}		
		return true;		
}

inline bool is_solution(const ConnectorMatrix& m)
{
	if(m.empty()) return false;
	for(size_t i= 0 ;i < m.size(); ++i)
	{
		for (size_t j = 0; j < m[i].size(); ++j)
		{
			if(m[i][j] && m[i][j]->get_key() == UnknownConnector)
				return false;
		}
	}
	return true;
}


template<typename ShuffleRand>
inline ConnectorMatrix construct_matrix(size_t z, size_t x, const ConnectorMatrix& m, const std::vector<Connector*> normal_connectors,ShuffleRand rand_func)
{

	if( !(z < m.size() && x < m[z].size()) ) return m;
	int pt = PL | PU | PR | PD;
	Connector *lc,*uc,*rc,*dc;
	lc = uc = rc = dc = 0;
	size_t next_x,next_z;
	//get next available position
	while (z < m.size() && !m[z][x])
	{
		if(x+1 < m[z].size()) x++;
		else{
			x=0;z++;
		}
	}
	if (z == m.size())
	{
		return m;
	}
	next_x = x;
	next_z = z;
	if( x != 0 )
	{
		lc = (m[z][x-1]);
	}
	if( z != 0 )
	{
		uc = (m[z-1][x]);
	}
	if( z < m.size() - 1 )
	{
		dc = m[z+1][x];
	}
	if ( x < m[z].size() - 1)
	{
		next_x ++;
		rc = m[z][x+1];
	}
	else
	{
		next_z++;
		next_x = 0;
	}
	if( lc == 0 ) pt = pt & ~PL;
	if( uc == 0 ) pt = pt & ~PU;
	if( rc == 0 ) pt = pt & ~PR;
	if( dc == 0 ) pt = pt & ~PD;

	std::vector<Connector*> cs = intersect_filter(lc,uc,rc,dc,filter_by_path_strict(pt,normal_connectors));//filter_by_path_strict(pt, intersect_filter(lc,uc,rc,dc,normal_connectors));
	//assert(!cs.empty());
	std::random_shuffle(cs.begin(),cs.end(),rand_func);
	ConnectorMatrix cur = m;

	//filter by neighbor 
	for(size_t i = 0;i < cs.size();++i)
	{
		if(cur[z][x] && cur[z][x]->get_key() == UnknownConnector)
			cur[z][x] = cs[i];
		if(is_connected(cur))
		{
			ConnectorMatrix ret;
			construct_matrix(next_z,next_x, cur, normal_connectors,rand_func).swap(ret);
			if(is_solution(ret))
				return ret;
		}
	}
	return ConnectorMatrix();
}
inline bool filter_check(std::vector<std::vector<Connector*> >& m,
						 const std::vector<Connector*>& normal_connectors
						 )
{
	for (size_t z = 0; z < m.size(); ++z)
		for(size_t x = 0; x < m[z].size(); ++x)
		{
			if(!m[z][x])continue;

			int pt = PL | PU | PR | PD;
			Connector *lc,*uc,*rc,*dc;
			lc = uc = rc = dc = 0;

			if( x != 0 )
			{
				lc = (m[z][x-1]);
			}
			if( z != 0 )
			{
				uc = (m[z-1][x]);
			}
			if( z < m.size() - 1 )
			{
				dc = m[z+1][x];
			}
			if ( x < m[z].size() - 1)
			{
				rc = m[z][x+1];
			}
			if( lc == 0 ) pt = pt & ~PL;
			if( uc == 0 ) pt = pt & ~PU;
			if( rc == 0 ) pt = pt & ~PR;
			if( dc == 0 ) pt = pt & ~PD;

			std::vector<Connector*> cs = intersect_filter(lc,uc,rc,dc,filter_by_path_strict(pt,normal_connectors));
			if(cs.empty()) return false;
		}
		return true;

}


inline void init_matrix_recur(size_t bz,size_t bx,size_t ez,size_t ex,float uz,float ux,std::queue<std::pair<size_t,size_t> >& visited_coords )
{
	size_t mid_z = bz + size_t(uz * (ez-bz));
	size_t mid_x = bx + size_t(ux * (ex-bx));
	if (mid_z == bz && mid_x == bx || (mid_z == ez || mid_x == ex) ) return;
	visited_coords.push(std::make_pair(mid_z,mid_x));
	init_matrix_recur(bz,bx,mid_z,mid_x,uz,ux,visited_coords);
	init_matrix_recur(mid_z,mid_x,ez,ex,uz,ux,visited_coords);
}
template<typename RandFunc>
inline void init_matrix(std::vector<std::vector<Connector*> >& initial,
						int prob_pz,int prob_px,int prob_nz,int prob_nx,int min_num_blocks,RandFunc rng)
{
	typedef std::pair<size_t,size_t> coord_t;
	coord_t coord;
	std::queue<coord_t> coord_queue;
	float u0 = 0.5f;
	float u1 = 0.5f;
	if(prob_pz + prob_nz > 0)
		u0 = float(prob_nz) / (prob_pz+prob_nz);
	if(prob_px + prob_nx > 0)
		u1 = float(prob_nx) / (prob_px+prob_nx);

	coord_queue.push(std::make_pair(0,0));
	init_matrix_recur(0,0,initial.size(),initial[0].size(),u0,u1,coord_queue);

	std::vector<bool> visited_row(initial[0].size(),false);
	std::vector<std::vector<bool> > visited(initial.size(),visited_row);

	/*
	size_t sz = std::min(size_t(coord_queue.size()*float(rand())/RAND_MAX),size_t(coord_queue.size()-1)) ;
	while(sz--)
	{	
	coord = coord_queue.front();
	visited[coord.first][coord.second] = true;
	coord_queue.pop();
	}
	if(!coord_queue.empty())
	coord = coord_queue.front();
	{
	coord_t coord_tmp;
	while(!coord_queue.empty())
	{
	coord_tmp = coord_queue.front();
	visited[coord_tmp.first][coord_tmp.second] = true;
	}
	}
	coord_queue.push(coord);
	*/
	coord.first = std::min(size_t(initial.size()* u0), initial.size() - 1);
	size_t sz = initial[coord.first].size();
	coord.second = std::min(size_t(sz * u1) ,sz - 1);
	coord_queue.push(coord);
	//breadth first 
	while(!coord_queue.empty() && min_num_blocks > 0)
	{
		coord = coord_queue.front();
		coord_queue.pop();
		if(visited[coord.first][coord.second])
			continue;
		visited[coord.first][coord.second] = true;
		min_num_blocks --;
		{
			bool go_nx = coord.second > 0 ;
			bool go_px = coord.second + 1 < initial[coord.first].size() ;
			bool go_nz = coord.first > 0 ;
			bool go_pz = coord.first + 1 < initial.size() ;

			if(go_nx) {coord_queue.push(std::make_pair(coord.first,coord.second-1));}
			if(go_px) {coord_queue.push(std::make_pair(coord.first,coord.second+1));}
			if(go_nz) {coord_queue.push(std::make_pair(coord.first-1,coord.second));}
			if(go_pz) {coord_queue.push(std::make_pair(coord.first+1,coord.second));}
		}
	}
	while(!coord_queue.empty())
	{
		coord = coord_queue.front();
		coord_queue.pop();
		if(visited[coord.first][coord.second])
			continue;
		visited[coord.first][coord.second] = true;
		{
			bool go_nx = coord.second > 0 && rng(100) < prob_nx;
			bool go_px = coord.second + 1 < initial[coord.first].size() && rng(100) < prob_px;
			bool go_nz = coord.first > 0 && rng(100) < prob_nz;
			bool go_pz = coord.first + 1 < initial.size() && rng(100) < prob_pz;

			if(go_nx) {coord_queue.push(std::make_pair(coord.first,coord.second-1));}
			if(go_px) {coord_queue.push(std::make_pair(coord.first,coord.second+1));}
			if(go_nz) {coord_queue.push(std::make_pair(coord.first-1,coord.second));}
			if(go_pz) {coord_queue.push(std::make_pair(coord.first+1,coord.second));}
		}
	}
	for(size_t i = 0;i < visited.size(); ++i)
		for(size_t j = 0;j < visited[i].size(); ++j)
		{
			if(!visited[i][j])
				initial[i][j] = 0;
		}
}

template<typename ShuffleRand>
inline ConnectorMatrix construct_by_connection(size_t width,size_t height, const std::vector<Connector*>& normal_connectors,ShuffleRand rand_func,
											   int prob_nx = 100,
											   int prob_px = 100,
											   int prob_nz = 100,
											   int prob_pz = 100
											   )
{
	std::vector<Connector*> input_connectors = normal_connectors;
	Connector unknown(UnknownConnector, UnknownPath);
	unknown.set_connector_1d<L>(normal_connectors.begin(),normal_connectors.end());
	unknown.set_connector_1d<U>(normal_connectors.begin(),normal_connectors.end());
	unknown.set_connector_1d<R>(normal_connectors.begin(),normal_connectors.end());
	unknown.set_connector_1d<D>(normal_connectors.begin(),normal_connectors.end());

	int min_blocks = 0;
	for(;;)
	{
		std::vector<Connector*> row(width, &unknown);
		ConnectorMatrix inited_matrix;
		do{
			inited_matrix.resize(0);
			row.resize(width, &unknown);
			inited_matrix.resize(height,row);
			init_matrix(inited_matrix,prob_pz,prob_px,prob_nz,prob_nx,min_blocks,rand_func);//todo: init policy can be custmized
			min_blocks ++;
		}while(!filter_check(inited_matrix,normal_connectors));

		ConnectorMatrix ret (construct_matrix(0,0,inited_matrix,input_connectors,rand_func));
		//if(ret.empty()) 
		//   debug_progress_output(inited_matrix);
		//assert(!ret.empty());
		if(ret.empty())
		{
			prob_pz += std::max(1,prob_pz/2);
			prob_px += std::max(1,prob_px/2);
			prob_nx += std::max(1,prob_nx/2);
			prob_nz += std::max(1,prob_nz/2);
		}
		else
			return ret;
	}
}

#endif
