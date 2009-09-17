#ifndef MA_LOGGER_HPP
#define MA_LOGGER_HPP


#include "Logger.hpp"

namespace ma{
	class MALogger:public Logger<MALogger>{
	public:
		ELOG_LEVEL getLogLevel() const{ return LOG_INFORMATION;}
		void setLogLevel(ELOG_LEVEL ll){return ;}
		void log(const char* text, ELOG_LEVEL ll){return  ;}
		void log(const char* text, const char* hint, ELOG_LEVEL ll){return ;}
		void log(const wchar_t* text, const wchar_t* hint, ELOG_LEVEL ll){return ;}
		void log(const wchar_t* text, ELOG_LEVEL ll) {
			return ;}

	};
}
#endif