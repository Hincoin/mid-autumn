
 #include <functional>
#include <boost/mpl/at.hpp>
#include <boost/variant.hpp>

template<typename T>
struct lazy
{
typedef std::function<T()> type;
};


struct nil{};
template<typename T>
struct cons_impl{
  typedef T head_type;
  typedef typename lazy< boost::variant<cons_impl<T>,nil> >::type tail_type;
  
head_type head;
tail_type tail;
cons_impl(head_type h, tail_type t):head(h),tail(t){}
};

template<typename T>
struct cons{
  typedef boost::variant<cons_impl<T>,nil> type;
};

namespace detail
{
template<typename C>
struct cons_inner_type{typedef typename boost::mpl::at_c<typename C::types, 0>::type type;};
struct cons_is_nil:boost::static_visitor<bool>{
  bool operator()(nil)const
  {
    return true;
  }
  template<typename T>
  bool operator()(T)const
  {
    return false;
  }
};
}

namespace result_of
{
template<typename S>
struct cons_car{
  typedef typename detail::cons_inner_type<S>::type inner_type;
  typedef typename inner_type::head_type type;
};

template<typename S>
struct cons_cdr{
  typedef S type;
};
}

template<typename C>
bool is_nil(C c)
{
  return boost::apply_visitor(detail::cons_is_nil(),c);
}

template<typename C>
const typename result_of::cons_car<C>::type 
car(const C& c)
{
  typedef typename detail::cons_inner_type<C>::type inner_type;
  return boost::get<inner_type>(c).head;
}


template<typename C>
const typename result_of::cons_cdr<C>::type
cdr(const C& c)
{
  assert(!is_nil(c));
  typedef typename detail::cons_inner_type<C>::type inner_type;
  return (boost::get<inner_type>(c).tail());
}

template<typename T>
typename cons<T>::type make_cons(T x)
{
  typedef typename cons<T>::type result_type;
  return cons_impl<T>(x, []()->result_type{return nil();});
}


template<typename T,typename F>
typename cons<T>::type make_cons(T x, F f)
{
  return cons_impl<T>(x, f);
}

cons<int>::type int_stream(int x)
{
typedef cons<int>::type result_type;
return make_cons(x,[=]()->result_type{return int_stream(x+1);}) ;
}
#include <iostream>


int main()
{
    //infinite stream
auto n = int_stream(1);
std::cout<<car(n)<<std::endl;
std::cout<<car(cdr(n))<<std::endl;
std::cout<<car(cdr(cdr(n)))<<std::endl;
//finite
    auto finite = make_cons(1);
    std::cout<<car(finite)<<std::endl;
    std::cout<<car(cdr(finite))<<std::endl;//bad_get exception
    //
}
