#ifndef _MA_INCLUDED_ERROR_REPORTER_HPP_
#define _MA_INCLUDED_ERROR_REPORTER_HPP_

#include <cstdio>
#include <cstdlib>
#include <cstdarg>
#include <stdexcept>
#include <exception>
namespace ma{
	inline void report_error(const char* s, ...)
	{
		char buff[1024]={};
		::memset(buff,0,sizeof(buff));
		va_list args;
		va_start(args,s);
#ifdef _MSC_VER
		::_snprintf(buff,sizeof(buff),s, args);
#else
		::snprintf(buff,sizeof(buff),s,args);
#endif
		va_end(args);

		throw std::runtime_error(buff);
	}
	inline void report_warning(const char* s, ...)
	{
		va_list args;
		va_start(args,s);
		printf(s,args);
		va_end(args);
	}
}
#endif
