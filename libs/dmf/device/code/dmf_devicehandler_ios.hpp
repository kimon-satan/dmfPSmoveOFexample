#pragma once

#include <dmf_devicehandler.hpp>

namespace DMF
{

	//////////////////////////////////////////////////////////////////////////
	class DeviceHandlerIOS : public DeviceHandler
	{
        
    public:
        
        ReturnCode Acquire(const std::string& id, Device** ppDevice);
    };

}