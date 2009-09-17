#ifndef SHAREDPOINTER_HPP_INCLUDED
#define SHAREDPOINTER_HPP_INCLUDED

#include <boost/shared_ptr.hpp>
namespace ma{
    template<typename T>
    struct shared_pointer{
        typedef boost::shared_ptr<T> type;
        };
    }

#endif // SHAREDPOINTER_HPP_INCLUDED
