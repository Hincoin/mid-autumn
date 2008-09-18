#ifndef NULLTYPE_H
#define NULLTYPE_H


		class NullType;

		class EmptyType{
		private:
			EmptyType(){}
			~EmptyType(){}
			EmptyType(const EmptyType&);
			EmptyType& operator=(const EmptyType&){}
		};
#endif