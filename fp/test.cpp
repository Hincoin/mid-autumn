/*
template<bool Condition,typename T0,typename T1>
struct if_c
{
	typedef T1 type;
};

template<typename T0,typename T1>
struct if_c<true,T0,T1>
{
	typedef T0 type;
};


template<typename T0,typename T1>
struct cons
{
	template<int idx>
		struct apply{
			typedef typename if_c<idx == 0, T0, T1>::type type;
		};
};

template<typename C>
struct car{
	typedef typename C::template apply<0>::type type;
};

template<typename C>
struct cdr{
	typedef typename C::template apply<1>::type type;
};
*/

#include <functional>

#include <iostream>

struct void_
{
	template<typename T>
		operator T()const{
			throw "dereference null type!";
		}
};

std::function<bool()> null(const void_& n)
{
	return [=](){return true;};
}
template<typename T>
const T& identity(const T& x)
{
	return x;
}
template<typename F>
typename F::result_type force(const F& f)
{
	return f();
}
template<typename F>
const F& delay(const F& f)
{
	return f;
}

template<typename T,typename T0,typename T1>
struct select_by_type;
template<typename T0,typename T1>
struct select_by_type<T1,T0,T1>
{
	typedef std::function<T1()> result_type;
	result_type operator()(T0 ,T1 y) const
	{
		return [=](){return identity(y);};
	}
};
template<typename T0,typename T1>
struct select_by_type<T0,T0,T1>
{
	typedef std::function<T0()> result_type;
	result_type operator()(T0 x,T1 ) const
	{
		return [=](){return identity(x);};
	}
};
template<typename T0>
struct select_by_type<T0,T0,void_>
{
	typedef std::function<T0()> result_type;
	result_type operator()(T0 x ) const
	{
		return [=](){return identity(x);};
	}
};

template<typename Impl,typename T0,typename T1>
struct base_cons
{
	private:
	const Impl& impl()const{return static_cast<const Impl&>(*this);}
	Impl& impl(){return static_cast<Impl&>(*this);}
	public:
	typedef T0 first_type;
	typedef T1 second_type;

	typedef std::function<std::size_t()> size_type;
	typedef typename select_by_type<first_type,first_type, second_type>::result_type car_result_type;
	typedef typename select_by_type<second_type,first_type, second_type>::result_type cdr_result_type;
	template<typename T>
	typename select_by_type<T,first_type, second_type>::result_type
	apply(T0 x,T1 y)const
	{
		return select_by_type<T, first_type, second_type>()(x,y);
	}

	car_result_type
	car()const{return impl().car_impl();}

	cdr_result_type
	cdr()const{return impl().cdr_impl();}

	size_type size()const
	{
		return impl().size_impl();
	}
};
template<typename Impl,typename T0>
struct base_cons<Impl,T0,void_>
{
	private:
	const Impl& impl()const{return static_cast<const Impl&>(*this);}
	Impl& impl(){return static_cast<Impl&>(*this);}
	
	public:
	typedef T0 first_type;
	typedef void_ second_type;
	typedef std::function<std::size_t()> size_type;
	template<typename T>
	typename select_by_type<T,first_type,second_type>::result_type
	apply(T0 x)const
	{
		return select_by_type<T,first_type,second_type>()(x);
	}
	typedef typename select_by_type<first_type,first_type, second_type>::result_type car_result_type;

	car_result_type
	car()const{return impl().car_impl();}
	
	size_type size()const{
		return impl().size_impl();
	}
};
template<typename Impl>
struct base_cons<Impl,void_,void_>
{
	private:
	const Impl& impl()const{return static_cast<const Impl&>(*this);}
	Impl& impl(){return static_cast<Impl&>(*this);}
	
	public:
	typedef void_ first_type;
	typedef void_ second_type;
	typedef std::function<std::size_t()> size_type;

	size_type size()const{
		return [=](){return 0;};
	}
};

template<typename T>
const std::function<T()>& car(const std::function<T()>& f)
{
	return delay(f);	
}
template<typename T>
const std::function<T()>& cdr(const std::function<T()>& f)
{
	return delay(f);	
}
template<typename Imp,typename T0,typename T1>
typename select_by_type<T0,T0,T1>::result_type car(const base_cons<Imp,T0,T1>& c)
{
	return c.car();
}
template<typename Imp,typename T0,typename T1>
typename select_by_type<T1,T0,T1>::result_type cdr(const base_cons<Imp,T0,T1>& c)
{
	return c.cdr();
}

template<typename Imp,typename T0,typename T1>
typename base_cons<Imp,T0,T1>::size_type size(const base_cons<Imp,T0,T1>& c)
{
	return c.size();
}
const std::function<std::size_t()> size(const std::function<std::size_t()>& f)
{
	return delay(f);
}

template<typename T0,typename T1>
struct seq_cons;
template<typename T0,typename T1>
struct seq_cons:base_cons<seq_cons<T0,T1>, T0, T1>{
	typedef base_cons<seq_cons<T0, T1>, T0, T1> base_type;
	private:
	T0 x_;
	T1 y_;
	public:
	seq_cons(T0 x, T1 y):x_(x),y_(y){}
	typename base_type::car_result_type
		car_impl()const{return base_type::template apply<T0>(x_,y_);}
	typename base_type::cdr_result_type
		cdr_impl()const{return base_type::template apply<T1>(x_,y_);}
	typename base_type::size_type size_impl()const
	{
		return [=](){return 1 + force(size(force(cdr(*this))));};
	}

};
template<typename T0>
struct seq_cons<T0,void_>:base_cons<seq_cons<T0,void_>, T0, void_>{
	typedef base_cons<seq_cons<T0, void_>, T0, void_> base_type;
	private:
	T0 x_;
	public:
	seq_cons(T0 x):x_(x){}
	typename base_type::car_result_type
		car_impl()const{return base_type::template apply<T0>(x_);}
	typename base_type::size_type size_impl()const
	{
		return [=](){return 1 ;};
	}

};
template<typename S>
struct car_type;
template<typename S>
struct cdr_type;

template<typename T0,typename T1>
struct car_type<seq_cons<T0,T1> >
{
	typedef T0 type;
};
template<typename T0,typename T1>
struct cdr_type<seq_cons<T0,T1> >
{
	typedef T1 type;
};

template<typename T0, typename T1>
seq_cons<T0,T1> make_seq_cons(T0 x, T1 y)
{
	return seq_cons<T0,T1>(x,y);
}
template<typename T0>
seq_cons<T0,void_> make_seq_cons(T0 x)
{
	return seq_cons<T0,void_>(x);
}

template<typename T0, typename T1, typename P>
struct filter_cons:
	base_cons<filter_cons<T0,T1,P>,	T0,
	filter_cons<typename car_type<T1>::type,typename cdr_type<T1>::type,P> >{
		typedef base_cons<filter_cons<T0,T1,P>,	T0,
	filter_cons<typename car_type<T1>::type,typename cdr_type<T1>::type,P> >
		base_type;
	public:
		filter_cons(P f,seq_cons<T0,T1> s):f_(f),c_(s){}
		typename base_type::size_type size_impl()const{

			return [=]()->std::size_t{
				if( this->f_(force(car(this->c_))) )
					return 1 + force(size(force(cdr(*this))));
				else
					return force(size(force(cdr(*this))));
			};

		}
		typename base_type::car_result_type
			car_impl()const
			{

				typedef typename base_type::car_result_type::result_type result_type;
				return [=]()->result_type{
				if( this->f_(force(car(this->c_))) )
					return force(car(this->c_));
				else
					return force(car(force(cdr(*this))));
				};
			}
		typename base_type::cdr_result_type
			cdr_impl()const
			{
				typedef filter_cons<typename car_type<T1>::type,typename cdr_type<T1>::type,P> second_type;
				return [=](){return second_type(this->f_,force(cdr(this->c_)));};
			}
	private:
	P f_;
	seq_cons<T0,T1> c_;	
};
template<typename T0,  typename P>
struct filter_cons<T0,void_,P>:
	base_cons<filter_cons<T0,void_,P>,	T0,
	filter_cons<void_,void_,P> >{
typedef base_cons<filter_cons<T0,void_,P>,	T0,
	filter_cons<void_,void_,P> > base_type;
	public:
		filter_cons(P f,seq_cons<T0,void_> s):f_(f),c_(s){}
		typename base_type::size_type size_impl()const{

			return [=](){
				if( this->f_(force(car(this->c_))) )
					return 1 ;
				else
					return 0;
			};

		}

		typename base_type::car_result_type
			car_impl()const
			{
				typedef typename base_type::car_result_type::result_type result_type;
				return [=]()->result_type{
				if( this->f_(force(car(this->c_))) )
					return force(car(this->c_));
				else
					return void_();
				};
			}
	private:
	P f_;
	seq_cons<T0,void_> c_;	
};

template<typename P, typename T0, typename T1>
filter_cons<T0,T1,P> filter(const seq_cons<T0,T1>& c,P f)
{
	return filter_cons<T0,T1,P>(f,c);
}

template<typename F,typename T>
typename F::result_type
apply_f(F f,T x){
	return f(x);
}

int square(int x){return x * x;}

#include <boost/mpl/at.hpp>
#include <boost/fusion/container.hpp>
#include <boost/fusion/sequence.hpp>
#include <boost/fusion/mpl.hpp>
#include <boost/variant.hpp>

/*
struct boost::recursive_variant_{
    template<typename T>
        recursive_variant_(T){}
};
*/
namespace fp{
   template<typename R>
      struct lazy{
          typedef std::function<R()> type;
      }; 

   template<typename T>
    struct stream{
        typedef typename boost::make_recursive_variant< boost::fusion::vector2
            < T
            , 
            typename lazy
            <
                boost::recursive_variant_
            >::type
            >
            >::type type;
    };
   namespace detail
   {
       template< typename Stream>
           struct stream_inner_type
           {
               typedef typename boost::mpl::at_c< typename Stream::types
                   , 0
                   >::type type;
           };
   }
   namespace result_of
   {
       template< typename Stream>
           struct stream_head
           {
               typedef typename detail::stream_inner_type<Stream>::type inner_type;
               typedef typename 
                   boost::fusion::result_of::at_c
                   < const inner_type, 0>::type type;
           };
   }
   template< typename Stream > 
       typename result_of::stream_head<Stream>::type
       stream_head( const Stream &s)
       {
           typedef typename detail::stream_inner_type<Stream>::type inner_type;
           return boost::fusion::at_c<0>
               (boost::get<inner_type>( s ) );
       }
namespace result_of
   {
       template< typename Stream>
           struct stream_tail
           {
               typedef typename detail::stream_inner_type<Stream>::type inner_type;
               typedef typename 
                   boost::fusion::result_of::at_c
                   < const inner_type, 1>::type type;
           };
   }
   template< typename Stream > 
       typename result_of::stream_tail<Stream>::type
       stream_tail( const Stream &s)
       {
           typedef typename detail::stream_inner_type<Stream>::type inner_type;
           return boost::fusion::at_c<1>
               (boost::get<inner_type>( s ) );
       }

   template< typename T, typename F>
       typename stream<T>::type
       stream_create( const T& t, F f)
       {
           typedef typename stream<T>::type Stream;
           typedef typename detail::stream_inner_type<Stream>::type inner_type;
           return inner_type( t, f);
       }
}

#include <cassert>
int main()
{
    typedef fp::stream<int>::type int_stream;
    typedef fp::detail::stream_inner_type<int_stream>::type int_inner_type;
    int_stream x;
    std::function<int_stream()> f = [&](){return fp::stream_create(1,f);};
    x = fp::stream_create(1,f);
   
    assert(1 == fp::stream_head(x));
    x = fp::stream_tail(x)();
    assert(1 == fp::stream_head(x));

	auto z = make_seq_cons(1.f,make_seq_cons(1.2f));
	auto gt1 = filter(z, [=](float x) {return x> 1.1f;});

	std::cout<<force(car(force(cdr(z))))<<std::endl;
	std::cout<<force(size(gt1))<<std::endl;
	std::cout<<force(car(gt1)) <<std::endl;
	std::cout<<force(car(force(cdr(gt1)))) <<std::endl;
}
