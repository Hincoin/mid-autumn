#ifndef INCLUDE_LOGGER_HPP
#define INCLUDE_LOGGER_HPP


#include "NullType.hpp"
namespace ma{
	enum ELOG_LEVEL
	{
		LOG_INFORMATION = 0,
		LOG_WARNING,
		LOG_ERROR,
		LOG_NONE
	};

	template<typename Derived,typename ClassConfig = NullType>
	class Logger{
		Logger& derived(){return static_cast<Derived&>(*this);}
		const Logger& derived()const {return static_cast<const Derived&>(*this);}
	protected:
		~Logger() {}
	public:
		ELOG_LEVEL getLogLevel() const{return derived().getLogLevel();}
		void setLogLevel(ELOG_LEVEL ll){return derived().setLogLevel();}
		void log(const char* text, ELOG_LEVEL ll){return derived().log(text,ll);}
		void log(const char* text, const char* hint, ELOG_LEVEL ll){return derived().log(text,hint,ll);}
		void log(const wchar_t* text, const wchar_t* hint, ELOG_LEVEL ll){return derived().log(text,hint,ll);}
		void log(const wchar_t* text, ELOG_LEVEL ll) {
			return derived().log(text,ll);}

	};
}

#endif