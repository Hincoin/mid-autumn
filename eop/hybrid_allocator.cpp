
#ifndef _AUTO_BUFFER_HPP_
#define _AUTO_BUFFER_HPP_

#include <memory>
#include <limits>
#include <boost/aligned_storage.hpp>


namespace detail{
    struct double_size{
        std::size_t operator()(std::size_t s) const 
        {
            return 2*s;
        }
    };
    struct fibonacci_size{
        std::size_t operator()(std::size_t s) const
        {}
    };
    //single-extent: single_ended, double_ended, circular
    //segmented: single-extent index, segmented index, slanted index
    //

template<int N>
struct stack_n_objects_policy{
    //
    enum {value = N};
};
template<int N>
struct stack_n_bytes_policy{
    //
    enum {value = N};
};
template<typename Policy, typename T>
struct compute_stack_buffer_size;

template<typename Policy, typename T>
struct compute_stack_object_size
{
    enum {value = compute_stack_buffer_size<Policy,T>::value / sizeof(T)};
};
template<typename T,int N>
struct compute_stack_buffer_size<stack_n_objects_policy<N>, T>
{
    enum {value = stack_n_objects_policy<N>::value * sizeof(T)};
};
template<typename T,int N>
struct compute_stack_buffer_size<stack_n_bytes_policy<N>, T>
{
    enum {value = stack_n_objects_policy<N>::value };
};

    template<typename T, typename StackPolicy = stack_n_objects_policy<256>,
   typename HeapAllocator = std::allocator<T>  >
        class hybrid_allocator;


    template<typename StackPolicy ,typename HeapAllocator>
        class hybrid_allocator<void,StackPolicy,HeapAllocator>:public HeapAllocator{
            public:
            typedef std::size_t size_type;
            typedef std::ptrdiff_t difference_type;
            typedef void* pointer;
            typedef const void* const_pointer;
        };


    template<typename T, typename StackPolicy ,typename HeapAllocator >
        class hybrid_allocator{
            struct impl:public HeapAllocator
            {
                enum{BufferSize = compute_stack_buffer_size<StackPolicy, T>::value};
                boost::aligned_storage<BufferSize , boost::alignment_of<T>::value> buffer;
                void* address()const
                {
                    return const_cast<void*>(buffer.address());
                }
           };
            enum{ObjectCount = compute_stack_object_size<StackPolicy, T>::value};
            impl impl_;
            public:
            typedef std::size_t size_type;
            typedef std::ptrdiff_t difference_type;
            typedef T* pointer;
            typedef const T* const_pointer;
            typedef T& reference;
            typedef const T& const_reference;
            typedef T value_type;

            template<typename U>
                struct rebind{
                    typedef hybrid_allocator<U> other;
                };
            pointer address(reference value)const{return &value;}
            const_pointer address(const_reference value) const
            {
                return &value;
            }
            hybrid_allocator()throw(){}
            hybrid_allocator(const hybrid_allocator&)throw(){}
            template<class U>
                hybrid_allocator(const hybrid_allocator<U>&) throw()
                {}
            ~hybrid_allocator()throw()
            {}
            size_type max_size()const throw()
            {
                return std::numeric_limits<size_type>::max()/sizeof(T);
            }
            pointer allocate(size_type num,
                    hybrid_allocator<void>::const_pointer hint = 0)
            {
                if (num < ObjectCount) 
                    return (pointer)(impl_.address());
                else
                    return impl_.allocate(num, hint);
            }
            void construct (pointer p, const T& value)
            {
                new ((void*)p) T(value);
            }
            void destroy(pointer p)
            {
                p->~T();
            }
            void deallocate(pointer p, size_type num)
            {
                if ( !is_in_stack(p) )
                {
                    return impl_.deallocate(p,num);
                }
            }
            private:
            bool is_in_stack(pointer p)const
            {
                const_pointer s = (const_pointer)(impl_.address());
                return (s <= p && p < s + ObjectCount);
            }
        };

     
template<typename T0,typename T1>
bool operator == (const hybrid_allocator<T0>&,
        const hybrid_allocator<T1>&)throw()
{
    return true;
}
template<class T0, class T1>
bool operator != (const hybrid_allocator<T0>&,
        const hybrid_allocator<T1>&)throw()
{
    return false;
}

}
/*
template<typename T,typename StackPolicy=stack_n_objects_policy<T,256>,typename SizePolicy = detail::fibonacci_size,typename Allocator = std::allocator<T> >
class ruto_buffer{
        typedef Allocator allocator_type;
        typedef T              value_type;
        typedef T*             iterator;
        typedef const T*       const_iterator;
        typedef T&             reference;
        typedef const T&       const_reference;
        typedef std::size_t    size_type;
        typedef std::ptrdiff_t difference_type;

 
        struct impl:Allocator{
         T* head;     
        };
      impl buffer_;
      auto_buffer(size_type n, const value_type& v = value_type()){}

      auto_buffer(){}
      template<typename InputIterator>
          auto_buffer(InputIterator f, InputIterator l)
          {}
      iterator begin(){}
      const_iterator begin()const{}
      iterator end(){}
      const_iterator end()const{}
      size_type size()const{}
      bool empty()const{}
      void insert_before(iterator i, const value_type& v){}
      void insert_before(iterator i, size_t n, const value_type& v){}
      template<typename InputIterator>
      void insert_before(iterator i, InputIterator f, InputIterator l){}

      void insert_after(iterator i, const value_type& v){}
      void insert_after(iterator i, size_t n, const value_type& v){}
      template<typename InputIterator>
      void insert_after(iterator i, InputIterator f, InputIterator l){}

      void erase(iterator i){}
      void erase(iterator f, iterator l){}

      void resize(size_type n, const value_type& v = value_type()){}
      void reserve(size_type n){}
      //lack of splicing
};
*/
#endif


#include <iostream>
#include <vector>

struct test{
    int a ;
    test():a(0){}
};
int main()
{
    std::vector<test,detail::hybrid_allocator<test> > v;
    for(int i = 0;i < 100000;i++)
    {
        v.push_back(test());
    } 
    for(int i = 0;i < 100000;i++)
    {
        v[i].a = i;
    }
    for(int i = 0;i < 100000; ++i)
    {
//        v.pop_front();
    }
    
    return 0;
}
