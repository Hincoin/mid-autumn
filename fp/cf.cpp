#include <utility>
#include <functional>

#include <boost/variant.hpp>

namespace fp
{
    //pure function
    template<typename F > struct cf;
/*
    template<typename R>
        struct cf<R()>{
            typedef R result_type;
            typedef std::function<R()> function_type;

            explicit cf(std::function<R()> f):f_(f){}
            explicit cf(R (*f) ()):f_(f){}
            R operator()()const
            {
                if(boost::apply_visitor(is_function(),f_))
                {
                    f_ = boost::get<std::function<R()> >(f_)();
                }
                return boost::get<R>(f_);
            }
            private:

            typedef boost::variant<std::function<R()>,R> cached_f_t;
            mutable cached_f_t f_;
            private:
            struct is_function:boost::static_visitor<R>
            {
                R operator()(const R& )const{return false;}
                R operator()(const std::function<R()>& )const{return true;}
            };
 
        };
        */
    template<typename R, typename T>
        struct cf<R(T)>{
            typedef R result_type;
            typedef std::function<R(T)> function_type;

            explicit cf(std::function<R(T)> f):f_(f){}
            explicit cf(R (*f) (T)):f_(f){}
            R operator()(T x)const
            {
                return f_(x);
            }
            private:
            function_type f_;

        };


    template<typename R,typename T, typename... Args >
        struct cf<R(T,Args...)>
        {
            typedef cf<R(Args...)> result_type;
            typedef std::function<R(T,Args...)> function_type;

            explicit cf(std::function<R(T,Args...)> f):f_(f){}
            explicit cf(R (*f) (T,Args...)):f_(f){}
            result_type operator()(T x)const
            {
                return result_type(bind1st(f_,x));
            }
            private:
            typename result_type::function_type 
                bind1st(const std::function<R(T,Args...)>& f, T x)const
                {
                    return [=](Args... args){return f(x,std::forward<Args>(args)... );};
                }
            std::function<R(T,Args...)> f_;
        };
    namespace detail{
        template<typename R,typename A>
            R curry_apply(const cf<R(A)>& f,A x)
            {
                return f(x);
            }
        template<typename R,typename A,typename... Args>
            R curry_apply(const cf<R(A,Args...)>& f, A a1, Args... args)
            {
               return curry_apply(f(std::forward<A>(a1)),std::forward<Args>(args)...);
            }
    }

    //compose
    template<typename T,typename U,typename... Args>
        cf<T(Args...)> operator*(const cf<T(U)>& f0, const cf<U(Args...)>& f1)
        {
            typedef std::function<T(Args...)> f_t;
            f_t f = [=](Args... args){return f0(detail::curry_apply(f1,std::forward<Args>(args)...));};
            return cf<T(Args...)>(f);
        }
}

#include <iostream>
#include <cassert>

template<typename T0,typename T1, typename T2 >
T2 ret3(T0 a, T1 b, T2 c)
{
    std::cout<<"called:"<< c<<std::endl;
    return c;
}
int main()
{

    //very simple test
    fp::cf<char (int,float, char)> f0 ( ret3<int,float,char> );
    fp::cf<char(float,char)> f1 = f0(1);

    fp::cf<char(char)> f2 = f1(1.1f);

    char f3 = f2('c');

    assert( f3 == 'c');
    auto f2_f0 = (f2 *  f0) (1)(1.3);
    assert( f2_f0('a') == 'a');
    assert( f2_f0('b') == 'b');
}
