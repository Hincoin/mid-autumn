#ifndef MADEVICEWIN32_H
#define MADEVICEWIN32_H
#include "MADevice.h"
namespace ma
{
	template<typename Configure>
	class MADeviceWin32:public MADevice<Configure>
	{
	public:
		MADeviceWin32(){}
		~MADeviceWin32(){}
	};
}

#endif

#include "MADeviceWin32Impl.h"