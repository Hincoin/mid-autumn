#ifndef INCLUDE_OSOPERATOR_HPP
#define INCLUDE_OSOPERATOR_HPP

#include "NullType.hpp"
namespace ma{
	template<typename Derived,typename ClassConfig = NullType>
	class OSOperator{
		OSOperator& derived(){return static_cast<Derived&>(*this);}
		const OSOperator& derived()const {return static_cast<const Derived&>(*this);}
	public:
		//Copies text to the clipboard. 
		void  copyToClipboard (const char *text) const {return derived().copyToClipboard(text);} 

		//Get the current operation system version as string. 
		const wchar_t *  getOSVersion () const {return derived().getOSVersion();}

		//Get the processor speed in megahertz. 
		bool  getProcessorSpeedMHz (unsigned int *MHz) const {return derived().getProcessorSpeedMHz(MHz);}
		//Get the total and available system RAM. 	
		bool  getSystemMemory (unsigned int *Total, unsigned int *Avail) const 
		{
			return derived().getSystemMemory(Total,Avail);
		}

		//Get text from the clipboard. 
		char *  getTextFromClipboard () const
		{
			return derived().getTextFromClipboard();
		}

	protected:
		~OSOperator () {}

	};
}
#endif