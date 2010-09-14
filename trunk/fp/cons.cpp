
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

template<typename R>
std::function<R()> delay(std::function<R()> f)
{return f;}

template<typename F>
F delay(F f)
{return f;}


template<typename R>
R force(std::function<R()> f)
{return f();}
template<typename T>
T force(T x)
{return x;}


/*
template<typename F,typename R,typename C>
R foldr(F f, R v, C xs)
{
    if ( is_nil( xs ) ) return v;
    return f(car(xs), foldr(f, v, cdr(xs) ));
}
*/
template<typename F,typename C>
typename cons<typename F::result_type>::type map(F f,C xs)
{
   return make_cons(f(car(xs)),[=](){return map(f,cdr(xs));});
}
//test code
cons<int>::type int_stream(int x)
{
    typedef cons<int>::type result_type;
    return make_cons(x,[=]()->result_type{return int_stream(x+1);}) ;
}


#include <iostream>

#include <cassert>

cons<int>::type cin_stream()
{
    typedef cons<int>::type result_type;
    int x ;
    std::cin>>x;
    return make_cons(x,[=]()->result_type{return cin_stream();}) ;
}

int main()
{
    //infinite stream
    auto n = int_stream(1);
    std::cout<<car(n)<<std::endl;
    std::cout<<car(cdr(n))<<std::endl;
    std::cout<<car(cdr(cdr(n)))<<std::endl;
    //finite
    auto finite = make_cons(1);
    finite = make_cons(2,[=](){return finite;});
    finite = make_cons(3,[=](){return finite;});
    finite = make_cons(4,[=](){return finite;});
    std::cout<<car(finite)<<std::endl;
    //assert(is_nil(car(cdr(finite))));
    //std::cout<<car(cdr(finite))<<std::endl;//bad_get exception
    
    std::function<int(int)> f = ([=](int x){return x*x;});
    cons<int>::type sqr_finite = map(
            f,
            n//finite
            );
    std::cout<<car(cdr(sqr_finite))<<std::endl;

    sqr_finite = map(
            f,
            cin_stream()
            );
    
    std::cout<<car(cdr(sqr_finite))<<std::endl;
    //
}
