#include "Move.h"
#include <iostream>
#include <algorithm>
#include <boost/operators.hpp>

using std::swap;

struct implementation:boost::equality_comparable<implementation>
{
	explicit implementation(int x = 0):member(x){
		std::cout<<"constructor \n";
	}
	implementation(const implementation& x):member(x.member)
	{
		std::cout<<"copy remote part:"<<member<<std::endl;
	}
	~implementation(){
		std::cout<<"destructor\n";
	}

	implementation& operator=(const implementation& x)
	{
		member = x.member;
		std::cout<<"assign remote part: "<<member<<std::endl;
		return *this;
	}

	friend bool operator == (const implementation& x,const implementation& y)
	{return x.member == y.member;}

	int member;
};
implementation operator+(const implementation& lhs,const implementation& rhs)
{
	return implementation(lhs.member + rhs.member);
}

class movable:public boost::equality_comparable<movable>
{
public:
	//model concept regular
	explicit movable(int x = 0):member(new implementation(x)){}
	~movable(){delete member;}

	movable(const movable& x):member(new implementation(ma::move(*x.member))){}

	friend bool operator==(const movable& x,const movable& y)
	{return *x.member == *y.member;}

	friend void swap(movable& x,movable& y)
	{
		swap(x.member,y.member);
	}
	//model concept movable

	movable(ma::move_from<movable> x):member(x.source.member){
		x.source.member = 0;
	}

	movable& operator=(movable x)
	{swap(*this,x);return *this;}
	movable& operator=(ma::move_from<movable> x){
		swap(*this,movable(x));return *this;
	}

	friend movable  operator+(const movable& lhs,const movable& rhs);
	template<typename T>
	friend ma::move_from<T> operator+(ma::move_from<T> lhs,ma::move_from<T> rhs);
	template<typename T>
	friend ma::move_from<T> operator+(ma::move_from<T> lhs,const T& rhs);
	template<typename T>
	friend ma::move_from<T> operator+(const T& lhs,ma::move_from<T> rhs);
private:
	implementation* member;
};

movable operator+(const movable& lhs,const movable& rhs)
{
	return movable(lhs.member->member+rhs.member->member);
}
template<typename T>
ma::move_from<T> operator+(ma::move_from<T> lhs,ma::move_from<T> rhs)
{
	((T& )lhs).member->member+=rhs.source.member->member;
	return lhs;
}
template<typename T>
ma::move_from<T> operator+(ma::move_from<T> lhs,const T& rhs)
{
	((T&)lhs).member->member+=rhs.member->member;
	return lhs;
}
template<typename T>
ma::move_from<T> operator+(const T& lhs,ma::move_from<T> rhs)
{
	return rhs + lhs;
}
template<typename T>
T return_test(int x,int y)
{
T r(x*y);
return r;//T(x*y);
}
template<typename T>
void temporary_objects()
{
	T x = return_test<T>(10,2);
	T y;
	T z;
	T w;
	std::cout<<"cont equal:\n";
	y=return_test<T>(1,2);
	z = y;
	std::cout<<"cont +:\n";
	w = ma::move_from<T>(x + y) + z + x + y + z + x + y + z;
	//w = x + y + z + x + y + z + x + y + z;
	std::cout<<"end\n";
}
#include "SpaceSegment.h"

void space_seg_test()
{
	using namespace ma;
   boxi bi,bb;
   bi = bb;
}
inline void test_move()
{
	std::cout<<"------------------------movable:\n";
	temporary_objects<movable>();
	std::cout<<"------------------------nonmovable:\n";
	//temporary_objects<implementation>();

	space_seg_test();
}
