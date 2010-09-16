#include <functional>
#include <boost/variant.hpp>

namespace fp
{
    template<typename F > struct cf;

    template<typename R>
        struct cf<R()>{
            typedef R result_type;
            typedef std::function<R()> function_type;

            explicit cf(std::function<R()> f):f_(f){}
            explicit cf(R (*f) ()):f_(f){}
            R operator()()const
            {
                return f_();
            }
            private:
            std::function<R()> f_;
            //typedef boost::variant<std::function<R()>,R> cached_f_t;
            //struct result_visitor
        };

    template<typename R,typename T, typename... Args >
        struct cf<R(T,Args...)>
        {
            typedef cf<R(Args...)> result_type;
            typedef std::function<R(T,Args...)> function_type;

            explicit cf(std::function<R(T,Args...)> f):f_(f){}
            explicit cf(R (*f) (T,Args...)):f_(f){}
            cf<R(Args...)> operator()(T x)const
            {
                typedef cf<R(Args...)> result_type;
                typedef typename result_type::function_type result_function;
                return result_type(result_function([=](Args... args){return this->f_(x,args... );}));
            }
            private:
            std::function<R(T,Args...)> f_;
        };
    namespace detail{
        template<typename R>
            R curry_apply(const cf<R()>& f)
            {
                return f();
            }
        template<typename R,typename A,typename... Args>
            R curry_apply(const cf<R(A,Args...)>& f, A a1, Args... args)
            {
               return curry_apply(f(a1),args...);
            }
    }

    //compose
    template<typename T,typename U,typename... Args>
        cf<T(Args...)> operator*(const cf<T(U)>& f0, const cf<U(Args...)>& f1)
        {
            typedef std::function<T(Args...)> f_t;
            f_t f = [=](Args... args){return f0(detail::curry_apply(f1,args...))();};
            return cf<T(Args...)>(f);
        }
}

#include <iostream>
#include <cassert>

template<typename T0,typename T1, typename T2 >
T2 ret3(T0 a, T1 b, T2 c)
{
    return c;
}
int main()
{

    //very simple test
    fp::cf<char (int,float, char)> f0 ( ret3<int,float,char> );
    fp::cf<char(float,char)> f1 = f0(1);

    fp::cf<char(char)> f2 = f1(1.1f);

    fp::cf<char()> f3 = f2('c');

    assert( f3() == 'c');
    assert( (f2 * f3) () == 'c');
    assert( (f2 * f0) (1)(1.3)('a')() == 'a');
}
