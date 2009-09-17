#ifndef INCLUDE_SCENEMANAGER_HPP
#define INCLUDE_SCENEMANAGER_HPP

#include "NullType.hpp"
namespace ma{
	template<typename Derived,typename ClassConfig = NullType>
	class SceneManager{
		typedef typename ClassConfig::EventType EventType;
		Derived& derived(){return static_cast<Derived&>(*this);}
		const Derived& derived()const{return static_cast<const Derived&>(*this);}
	public:
		bool postEventFromUser(const EventType& evt){
			return derived().postEventFromUser(evt);
		}
	protected:
		~SceneManager(){}
	};
}
#endif