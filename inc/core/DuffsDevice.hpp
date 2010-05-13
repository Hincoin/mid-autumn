#ifndef __INCLUDE_DUFFS_DEVICE_HPP__
#define __INCLUDE_DUFFS_DEVICE_HPP__

//http://www.boost.org/doc/libs/1_37_0/libs/preprocessor/doc/examples/duffs_device.c
//improved by lzy

# include <cassert>

# include <boost/preprocessor/repetition/repeat.hpp>
# include <boost/preprocessor/tuple/elem.hpp>
# include <boost/mpl/if.hpp>
# include <boost/mpl/bool.hpp>

namespace ma
{
    namespace details
    {

        template<unsigned>
        struct is_power_2;
        template<>
        struct is_power_2<1>:boost::mpl::true_
            {};
        template<unsigned n>
        struct is_power_2:boost::mpl::if_c<n&1, boost::mpl::false_ ,is_power_2<(n>>1)> >::type
            {};

        template<unsigned int N>
        struct _mod
        {
private:

            enum{N_1 = (N-1)};
            static unsigned mod_impl(unsigned i,boost::mpl::true_*)
            {
                return i&(N_1);
            }
            static unsigned mod_impl(unsigned i,boost::mpl::false_*)
            {
                return i%N;
            }
public:
            static unsigned mod(unsigned i)
            {
                return mod_impl(i,(is_power_2<N>*)(0));
            }
        };
    }
}

# /* Expands to a Duff's Device. */
#
#
# define DUFFS_DEVICE_C(Z, I, UNROLLING_FACTOR_STATEMENT) \
   case (I ? BOOST_PP_TUPLE_ELEM(2, 0, UNROLLING_FACTOR_STATEMENT) - I : 0): \
   BOOST_PP_TUPLE_ELEM(2, 1, UNROLLING_FACTOR_STATEMENT); \
   /**/
#
#
# define DUFFS_DEVICE(UNROLLING_FACTOR, COUNTER_TYPE, N, STATEMENT) \
	{ \
	COUNTER_TYPE duffs_device_initial_cnt = (N); \
	if (duffs_device_initial_cnt > 0) { \
	COUNTER_TYPE duffs_device_running_cnt = (duffs_device_initial_cnt + (UNROLLING_FACTOR - 1)) / UNROLLING_FACTOR; \
	switch (/*duffs_device_initial_cnt % UNROLLING_FACTOR*/  \
	::ma::details::_mod<UNROLLING_FACTOR>::mod(duffs_device_initial_cnt)) { \
	do { \
	BOOST_PP_REPEAT(UNROLLING_FACTOR, DUFFS_DEVICE_C, (UNROLLING_FACTOR, { STATEMENT })) \
	} while (--duffs_device_running_cnt); \
		 } \
	} \
	}\
	/**/
#



#endif
