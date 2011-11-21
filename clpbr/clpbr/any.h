#ifndef _ANY_H_
#define _ANY_H_

#include <utility>
class Any
{
public:
	typedef uintptr_t type_id_t;
private:
	class place_holder{
	public:
		virtual type_id_t type_id()const = 0;
		virtual place_holder* clone() const= 0;
		virtual ~place_holder(){}
	};
	template<typename T>
	class value_place_holder:public place_holder{
	public:
		T value_;
		value_place_holder(){}
		value_place_holder(const T& value):value_(value){}
		virtual type_id_t type_id()const
		{
			static char tag = 0;
			return (type_id_t)&tag;
		}
		virtual place_holder* clone()const 
		{
			return new value_place_holder<T>(value_);
		}
	};
public:

	Any():holder_(0){}
	template<typename T>
	Any(const T& value):holder_(new value_place_holder<T>(value)){}

	Any(const Any& other):holder_(other.holder_ ? other.holder_->clone() : 0){}
	
	~Any(){delete holder_;}

public:
	Any& swap(Any& rhs)
	{
		std::swap(holder_, rhs.holder_);
		return *this;
	}
	template<typename OtherT>
	Any& operator =(const OtherT& rhs)
	{
		Any(rhs).swap(*this);
		return *this;
	}

	Any& operator=(Any rhs)
	{
		rhs.swap(*this);
		return *this;
	}
	bool empty()const{return !holder_;}

	type_id_t type_id()const{
		if(!empty())
			return holder_->type_id();
		return void_type_id;//void type
	}
	static type_id_t void_type_id ;
protected:
private:
	template<typename ValueType> 
	friend ValueType * any_cast(Any *);
	place_holder *holder_;
};

template<typename ValueType> const ValueType * any_cast(const Any *);
template<typename ValueType> ValueType * any_cast(Any *);

template<typename ValueType> ValueType * any_cast(Any *operand)
{
	return operand && operand->type_id() == Any::value_place_holder<ValueType>().type_id() ?
		&static_cast<Any::value_place_holder<ValueType>*>(operand->holder_)->value_:0;
}

template<typename ValueType> const ValueType * any_cast(const Any *operand)
{
	return any_cast<ValueType>(const_cast<Any*>(operand));
}

#endif