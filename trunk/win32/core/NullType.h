#ifndef NULLTYPE_H
#define NULLTYPE_H

namespace ma
{
	namespace core{
		class NullType;

		class EmptyType{
		private:
			EmptyType(){}
			~EmptyType(){}
			EmptyType(const EmptyType&);
			EmptyType& operator=(const EmptyType&){}
		};
	}
}
#endif