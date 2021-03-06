#pragma once

#include <dmf_deviceinterface.hpp>

namespace DMF
{
	enum DMFDeviceConstant
	{
		kMaxNumDevices = 64
	};

	class Device;
	struct DeviceRegistry;
    
	//////////////////////////////////////////////////////////////////////////
	class DeviceHandler : public DynamicLibraryBase
	{
	public:
		//////////////////////////////////////////////////////////////////////////
		virtual ~DeviceHandler();
		//////////////////////////////////////////////////////////////////////////
		virtual ReturnCode Init(/* XML / JSON init file goes here*/);
		//////////////////////////////////////////////////////////////////////////
		virtual ReturnCode LoadPlugins(const char* directory);
		//////////////////////////////////////////////////////////////////////////
		virtual ReturnCode GetDeviceInfo(DeviceDataMap** ppDeviceMaps, u32& numDevices);
		//////////////////////////////////////////////////////////////////////////
        virtual ReturnCode GetDeviceOutputInfo(DeviceDataMap** ppDeviceMaps, u32& numDevices);
        //////////////////////////////////////////////////////////////////////////
		virtual ReturnCode Shutdown();
		//////////////////////////////////////////////////////////////////////////
		// Note: potentially return a u32 device ID instead of a pointer
		//virtual ReturnCode Acquire(const std::string& id, Device** ppDevice);
		//////////////////////////////////////////////////////////////////////////
		//ReturnCode Release(Device* ppDevice);
        //////////////////////////////////////////////////////////////////////////
        virtual ReturnCode Update();
		//////////////////////////////////////////////////////////////////////////
		virtual ReturnCode Start(u32 deviceID);
		//////////////////////////////////////////////////////////////////////////
		virtual ReturnCode Stop(u32 deviceID);
		//////////////////////////////////////////////////////////////////////////
		virtual ReturnCode Update(u32 deviceID, DataBuffer* pDestination);
		//////////////////////////////////////////////////////////////////////////
		virtual ReturnCode Update(u32 deviceID, CustomDataBuffer* pDestination);
		//////////////////////////////////////////////////////////////////////////
		virtual ReturnCode Update(u32 deviceID, DeviceDataMap* pDestination);
        //////////////////////////////////////////////////////////////////////////
        virtual ReturnCode UpdateOutput(u32 deviceID, DataBuffer* pSrc);
        //////////////////////////////////////////////////////////////////////////
        virtual ReturnCode UpdateOutput(u32 deviceID, CustomDataBuffer* pSrc);
        //////////////////////////////////////////////////////////////////////////
        virtual ReturnCode UpdateOutput(u32 deviceID, DeviceDataMap * pSrc);
		
	private:
        
        virtual bool HandleDLL(const char* filename);
        
		DeviceRegistry*		mpDeviceRegistry	= NULL;
	};
}