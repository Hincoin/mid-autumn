#ifndef VIRTUALINTERFACE_HPP_INCLUDED
#define VIRTUALINTERFACE_HPP_INCLUDED

#include <boost/type_traits.hpp>
#include "NullType.hpp"
#include "PtrTraits.hpp"
namespace ma
{

    template<typename D>
    class virtual_destructible;
    namespace details
    {
        //template<typename D>
        //inline void virtual_delete_impl(D* d,boost::false_type)
        //{
        //   (*d).destruct();
        //   operator delete(d);
        //}
        //issue a compiler error
        template<typename D>
        inline void virtual_delete_impl(D* d, boost::false_type)
        {
			BOOST_STATIC_ASSERT((boost::is_base_and_derived<virtual_destructible<D>,D>::value));
            (*d).destruct();
			::operator delete(d);
        }
        template<typename D>
        inline void virtual_delete_impl(D* d,boost::true_type)
        {
            delete d;
        }

    }
    template<typename D>
    inline void virtual_delete(D* d)
    {
        details::virtual_delete_impl(d,boost::has_virtual_destructor<D>());
    }

    template<typename I>
    inline void virtual_delete(I first,I last)
    {
        for (;first != last;++ first )
        {
            virtual_delete(*first);
        }
    }
	template<typename D>
	inline void delete_ptr(virtual_destructible<D>* p)
	{
		virtual_delete(p);
	}

    struct virtual_destructor
    {
        template <typename D>
        void operator()(D* d)const
        {
            virtual_delete(d);
        }
        template <typename I>
        void operator()(I f,I l)const
        {
            virtual_delete(f,l);
        }
    };

    template<typename Derived>
    class virtual_destructible
    {
	public:
        typedef virtual_destructible<Derived> class_type;

        template<typename D> friend void details::virtual_delete_impl(D*,boost::false_type);
protected:
        void destruct()
        {
            return (static_cast< Derived&>(*this)).destructImpl();
        }
        ~virtual_destructible(){}
    };



#define MA_INTERFACE_BEGIN(INTERFACE,CRTP_PARENT_CLASS)\
template<typename Conf>\
class INTERFACE:public CRTP_PARENT_CLASS<INTERFACE<Conf>,Conf >,public virtual_destructible<INTERFACE<Conf> >\
{\
friend class CRTP_PARENT_CLASS<INTERFACE<Conf>,Conf >;\
friend class virtual_destructible<INTERFACE<Conf> >;\
protected:\
typedef INTERFACE<Conf> class_type;\
typedef CRTP_PARENT_CLASS<INTERFACE<Conf>,Conf > parent_type;\
typedef INTERFACE<Conf> interface_type;\
\
template<typename Conf_>\
struct __##INTERFACE##__virtual_table_;\
typedef __##INTERFACE##__virtual_table_<Conf> virtual_table_type;\
\
private:\
const virtual_table_type* v_table;\
protected:\
INTERFACE(const virtual_table_type& v):v_table(&v){}\
~INTERFACE(){}\
template<typename Conf_>\
struct __##INTERFACE##__virtual_table_\
{\
typedef CRTP_PARENT_CLASS<INTERFACE<Conf>,Conf > parent_type;\

#define MA_VTABLE_BEGIN\

#define MA_VTABLE_FUNC(RET_,FUNC_,PARAMS)\
RET_ (*FUNC_)PARAMS;

#define MA_VTABLE_END };\
private:\

#define MA_VTABLE_IMPL(RET_,INTERFACE_FUNC_,PARAMS,CALL_PARAMS)\
RET_ INTERFACE_FUNC_##Impl PARAMS{\
return v_table->INTERFACE_FUNC_ CALL_PARAMS;\
}\


#define MA_FORWARD_TYPEDEF(X)\
protected:\
typedef typename parent_type::X X;\

#define MA_INTERFACE_END };

template<typename S,int D,typename Tag = NullType>
struct MAConfigure{
    typedef S ScalarType;
    static const int Dimension = D;
};

template<typename S,int D,typename Tag>
struct scalar_type<MAConfigure<S,D,Tag> >
{
    typedef S type;
};

template<typename S,int D,typename Tag>
struct dimensions<MAConfigure<S,D,Tag> >
{
    static const int value = D;
};
/// example

    /*
    template<typename Derived,typename S,int D>
    class crtp_Shape
    {
        Derived& derived()
        {
            return static_cast<Derived&>(*this);
        }
        const Derived& derived()const
        {
            return static_cast<const Derived&>(*this);
        }
    protected:
        ~crtp_Shape(){}
        crtp_Shape(){}

    public:
        void draw()const
        {
            return derived().drawImpl();
        }
        bool intersect()const
        {
            return derived().intersectImpl();
        }
        void swap(Derived& other)
        {
            return derived().swapImpl(other);
        }
    };


    MA_INTERFACE_BEGIN(shape_interface,crtp_Shape)
    MA_VTABLE_BEGIN
    MA_VTABLE_FUNC(void ,destruct, (const interface_type&))
    MA_VTABLE_FUNC(void ,swap, (interface_type&,interface_type& ))
    MA_VTABLE_FUNC(void ,draw, (const interface_type&))
    MA_VTABLE_FUNC(bool,intersect, (const interface_type&))
    MA_VTABLE_END

    MA_VTABLE_IMPL(void , destruct, (), (*this) )
    MA_VTABLE_IMPL(void ,swap, (interface_type& other),(*this,other) )
    MA_VTABLE_IMPL(void, draw, ()const, (*this) )
    MA_VTABLE_IMPL(bool, intersect, ()const,(*this) )

    MA_INTERFACE_END



    template<typename S,int D>
    class Shape_CRTP_Vtable:public shape_interface<S,D>
    {
        typedef Shape_CRTP_Vtable<S,D> class_type;
        typedef shape_interface<S,D> parent_type;
        typedef parent_type interface_type;
        typedef typename parent_type::virtual_table_type virtual_table_type;

        static const virtual_table_type v_table;

        static class_type& self(interface_type& me)
        {
            return static_cast<class_type&>(me);
        }
        static const class_type& self(const interface_type& me)
        {
            return static_cast<const class_type&>(me);
        }
        static void destruct(const interface_type& me)
        {
            self(me).~Shape_CRTP_Vtable();
        }
        static void swap(interface_type& me,interface_type& other)
        {
            self(me).stored.swap(self(other).stored);
        }

        static void draw(const interface_type& me)
        {
            std::vector<parent_type*>& stored = self(me).stored;
            if (stored.size() < 100000)stored.push_back(new class_type);
            else
            {
                stored.back()->draw();
            }
        }
        static bool intersect(const interface_type& me)
        {
            S t = S();
            return D != 2 && t == 0;
        }
    public:
        Shape_CRTP_Vtable():parent_type(v_table){}
        ~Shape_CRTP_Vtable()
        {
            for (size_t i = 0;i<stored.size();++i)
                virtual_delete( stored[i]);
        }
        mutable std::vector<parent_type*> stored;
    };
    template<typename S,int D>
    const typename Shape_CRTP_Vtable<S,D>::virtual_table_type
    Shape_CRTP_Vtable<S,D>::v_table=
    {
        &Shape_CRTP_Vtable<S,D>::destruct,
        &Shape_CRTP_Vtable<S,D>::swap,
        &Shape_CRTP_Vtable<S,D>::draw,
        &Shape_CRTP_Vtable<S,D>::intersect,
    };
    */

}

#endif // VIRTUALINTERFACE_HPP_INCLUDED
