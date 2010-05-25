#ifndef _MA_INCLUDED_SERIALIZER_HPP_
#define _MA_INCLUDED_SERIALIZER_HPP_
#include <iostream>
#include <streambuf>
#include <boost/cstdint.hpp>
#include <vector>
#include <map>
#include <limits>
#include <cassert>
#include <exception>
#include <boost/type_traits/is_arithmetic.hpp>
#include <boost/static_assert.hpp>
#include <boost/mpl/push_back.hpp>
#include <boost/mpl/pop_front.hpp>
#include <boost/fusion/adapted.hpp>
#include <boost/fusion/container.hpp>
#include <boost/fusion/sequence.hpp>
#include <boost/fusion/include/pop_front.hpp>
#include <boost/fusion/include/push_back.hpp>
#include <boost/fusion/include/for_each.hpp>
#include <boost/fusion/algorithm/iteration/for_each.hpp>

namespace ma{
namespace serialization
{
	class serialization_error//:public std::exception
	{
	public:
		serialization_error(const std::string& e){}
	};
	struct rpc_streambuf:public std::streambuf
	{
		rpc_streambuf(char* buf,size_t sz)
		{
			setg(buf,buf,buf+sz);
		}
	};
	template <typename Derived>
		class serializable{
				const Derived& derived()const{return static_cast<const Derived&>(*this);}
				Derived& derived(){return static_cast<Derived&>(*this);}
			public:
				void serialize(std::ostream& out)const
				{
					derived().serializeImpl(out);
				}
				void deserialize(std::istream& in)
				{
					derived().deserializeImpl(in);
				}
		};
	template<typename T>
	void	serialize(const serializable<T>& s,std::ostream& out)
	{
		return s.serialize(out);
	}
	template<typename T>
		void deserialize(serializable<T>& s,std::istream& in)
		{
			return s.deserialize(in);
		}
	template <typename domain, typename range, typename compare, typename alloc>
	void serialize(const std::map<domain, range, compare, alloc>& item, std::ostream& out);

	template <typename domain, typename range, typename compare, typename alloc>
	void deserialize(std::map<domain, range, compare, alloc>& item, std::istream& in);

	template <typename T, typename alloc>
	void serialize(const std::vector<T, alloc>& item, std::ostream& out);


	template <typename T, typename alloc>
	void deserialize(std::vector<T>& item, std::istream& in);

	inline void serialize(const std::string& item, std::ostream& out);

	inline void deserialize(std::string& item, std::istream& in);

	template <typename T, size_t length>
	inline void serialize(const T (&array)[length], std::ostream& out);

	template <typename T, size_t length>
	inline void deserialize(T (&array)[length], std::istream& in);


	namespace detail
	{
		template<typename T>
		inline void read_number(T& item,std::istream& in)
		{
			BOOST_STATIC_ASSERT((boost::is_arithmetic<T>::value));
			in.read(reinterpret_cast<char*>(&item),sizeof(T));
			if(!in)
			{
				throw serialization_error("Error deserializing number");
			}
		}
		template<typename T>
		inline void write_number(T item,std::ostream& out)
		{
			BOOST_STATIC_ASSERT((boost::is_arithmetic<T>::value));
			out.write(reinterpret_cast<char*>(&item),sizeof(T));
			if (!out)
			{
				throw serialization_error("Error serializing number");
			}
		}
	}
#define NUMBER_SERIALIZATION_FOR(T)\
	inline void serialize(T item,std::ostream& out)\
	{\
		return detail::write_number(item,out);\
	}\
	inline void deserialize(T &item,std::istream& in)\
	{\
		return detail::read_number(item,in);\
	}\

	NUMBER_SERIALIZATION_FOR(bool)
	NUMBER_SERIALIZATION_FOR(boost::int16_t)
	NUMBER_SERIALIZATION_FOR(int)
	NUMBER_SERIALIZATION_FOR(boost::int64_t)

	NUMBER_SERIALIZATION_FOR(boost::uint16_t)
	NUMBER_SERIALIZATION_FOR(unsigned int)
    NUMBER_SERIALIZATION_FOR(boost::uint64_t)

	NUMBER_SERIALIZATION_FOR(float)
	NUMBER_SERIALIZATION_FOR(double)
    NUMBER_SERIALIZATION_FOR(long double)

	NUMBER_SERIALIZATION_FOR(boost::int8_t)
	NUMBER_SERIALIZATION_FOR(boost::uint8_t)
	NUMBER_SERIALIZATION_FOR(char)

#undef NUMBER_SERIALIZATION_FOR

namespace detail
	{
		struct fusion_seq_serialize_impl{
			std::ostream& out;	
			fusion_seq_serialize_impl(std::ostream& o):out(o){}
			template<typename T>
			void operator()(const T& v)const
			{
				serialize(v,out);
			}

		};
		struct fusion_seq_deserialize_impl{
			std::istream& in;
			fusion_seq_deserialize_impl(std::istream& i):in(i){}
			template<typename T>
				void operator()(T& v)const
				{
					deserialize(v,in);
				}
		};
	}
#define BOOST_FUSION_MAX_SEQ_SIZE FUSION_MAX_VECTOR_SIZE

#define SERIALIZATION_FOR_FUSION_SEQ(SEQ)\
template <BOOST_PP_ENUM_PARAMS(BOOST_FUSION_MAX_SEQ_SIZE, typename T)>\
	inline void	serialize(const boost::fusion::SEQ<BOOST_PP_ENUM_PARAMS(BOOST_FUSION_MAX_SEQ_SIZE,T)>& v,std::ostream& out)\
		{\
			boost::fusion::for_each(v,detail::fusion_seq_serialize_impl(out));\
		}\
template <BOOST_PP_ENUM_PARAMS(BOOST_FUSION_MAX_SEQ_SIZE, typename T)>\
	inline void deserialize(boost::fusion::SEQ<BOOST_PP_ENUM_PARAMS(BOOST_FUSION_MAX_SEQ_SIZE,T)>& v, std::istream& in)\
	{\
		boost::fusion::for_each(v,detail::fusion_seq_deserialize_impl(in));\
	}\

	SERIALIZATION_FOR_FUSION_SEQ(vector)
	SERIALIZATION_FOR_FUSION_SEQ(list)
	SERIALIZATION_FOR_FUSION_SEQ(set)

#undef BOOST_FUSION_MAX_SEQ_SIZE
#undef SERIALIZATION_FOR_FUSION_SEQ 


template <typename T, typename alloc>
	void serialize(const std::vector<T, alloc>& item, std::ostream& out)
	{
		serialize(item.size(),out);
		for(size_t i = 0;i < item.size(); ++i)
		{
			serialize(item[i],out);
		}
	}


	template <typename T, typename alloc>
	void deserialize(std::vector<T>& item, std::istream& in)
	{
		size_t sz = 0;
		deserialize(sz,in);
		item.resize(sz);
		for(size_t i = 0;i < item.size();++i)
		{
			deserialize(item[i],in);
		}	
	}
	inline void serialize(const std::string& item, std::ostream& out)
	{
		serialize(item.size(),out);
		for(size_t i = 0;i < item.size(); ++i)
		{
			serialize(item[i],out);
		}
	}

	inline void deserialize(std::string& item, std::istream& in)
	{
		size_t sz = 0;
		deserialize(sz,in);
		item.resize(sz);
		for(size_t i = 0;i < item.size(); ++i)
		{
			deserialize(item[i],in);
		}

	}

	

}


}

#endif
