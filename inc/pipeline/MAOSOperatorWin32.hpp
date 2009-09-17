#ifndef MA_OSOPERATOR_WIN32_HPP
#define MA_OSOPERATOR_WIN32_HPP

#include "OSOperator.hpp"
namespace ma{
	class MAOSOperatorWin32:public OSOperator<MAOSOperatorWin32>{
	public:
		//Copies text to the clipboard.
		void  copyToClipboard (const char *text) const {return ;}

		//Get the current operation system version as string.
		const wchar_t *  getOSVersion () const
		{return 0;}

		//Get the processor speed in megahertz.
		bool  getProcessorSpeedMHz (unsigned int *MHz) const
		{return false;}
		//Get the total and available system RAM.
		bool  getSystemMemory (unsigned int *Total, unsigned int *Avail) const
		{
			return false;
		}

		//Get text from the clipboard.
		char *  getTextFromClipboard () const
		{
			return 0;
		}
	};
}
#endif
