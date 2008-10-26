#ifndef MA_PRINTER_HPP
#define MA_PRINTER_HPP

#include "Logger.hpp"
namespace ma{
	template<typename Configure>
	class Printer
	{
		typedef Configure::LoggerPtr;
	public:
		// prints out a string to the console out stdout or debug log or whatever
		static void print(const char* message){}
		static void log(const char* message, ELOG_LEVEL ll ){}
		static void log(const char* message, const char* hint, ELOG_LEVEL ll ){}
		static void log(const wchar_t* message, ELOG_LEVEL ll ){}
		static LoggerPtr Logger;
	};
}
#endif