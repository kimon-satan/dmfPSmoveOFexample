#include <dmf.hpp>
#include <dmf_devicehandler_ios.hpp>
#include <dmf_device_ios.hpp>

using namespace DMF;

//////////////////////////////////////////////////////////////////////////
ReturnCode DeviceHandlerIOS::Acquire(const std::string& id, Device** ppDevice)
{
	// Attempt to load plug-ins

	// If successful - create and return device pointer
    
    //will need some kind of device recognition conditional here to choose the correct class

	DeviceIOS* pDevice = new DeviceIOS();
	pDevice->Init();

	*ppDevice = pDevice;

	return kOK;
}


