#pragma once

#include <dmf.hpp>
#include <dmf_utils.hpp>
#include <dmf_deviceinterface.hpp>

namespace DMF
{
	//////////////////////////////////////////////////////////////////////////
	class Device : public DynamicImplementation<DynamicInterface>
	{
		friend class DeviceHandler;
        friend class DeviceHandlerIOS;
	public:
		//////////////////////////////////////////////////////////////////////////
		virtual ReturnCode GetDataMap(DeviceDataMap& dataMap) = 0;
		//////////////////////////////////////////////////////////////////////////
		virtual ReturnCode Start() = 0;
		//////////////////////////////////////////////////////////////////////////
		virtual ReturnCode Stop() = 0;
		//////////////////////////////////////////////////////////////////////////
		virtual ReturnCode Update(DataBuffer* pDestination) = 0;
		//////////////////////////////////////////////////////////////////////////
		virtual ReturnCode Update(CustomDataBuffer* pDestination) = 0;
		//////////////////////////////////////////////////////////////////////////
		virtual ReturnCode Update(DeviceDataMap* pDestination) = 0;
        //////////////////////////////////////////////////////////////////////////
        virtual ReturnCode UpdateOutput(DataBuffer* pDestination) = 0;
        //////////////////////////////////////////////////////////////////////////
        virtual ReturnCode UpdateOutput(CustomDataBuffer* pDestination) = 0;
        //////////////////////////////////////////////////////////////////////////
        virtual ReturnCode UpdateOutput(DeviceDataMap* pDestination) = 0;
        //////////////////////////////////////////////////////////////////////////
		u32 GetDeviceID()
		{
			return mDeviceMap.mDeviceID;
		}
	protected:
		//////////////////////////////////////////////////////////////////////////
		virtual ReturnCode Init() = 0;

		//////////////////////////////////////////////////////////////////////////
		virtual ReturnCode Service() = 0;

		//////////////////////////////////////////////////////////////////////////
		virtual ReturnCode Shutdown() = 0;

		//////////////////////////////////////////////////////////////////////////
		DeviceDataMap mDeviceMap;
        DeviceDataMap mDeviceOutputMap;
	};
    
    //////////////////////////////////////////////////////////////////////////
    enum ButtonID
    {
        kButtonStart			= 0x00000001,
        kButtonSelect			= 0x00000002,
        kDPadLeft				= 0x00000004,
        kDPadRight				= 0x00000008,
        kDPadUp					= 0x00000010,
        kDPadDown				= 0x00000020,
        kButton1				= 0x00000040,
        kButton2				= 0x00000080,
        kButton3				= 0x00000100,
        kButton4				= 0x00000200,
        kButtonSquare			= kButton1,
        kButtonTriangle			= kButton2,
        kButtonCross			= kButton3,
        kButtonCircle			= kButton4,
        kButtonX				= kButton1,
        kButtonY				= kButton2,
        kButtonA				= kButton3,
        kButtonB				= kButton4,
        kButtonLeftShoulder		= 0x00000400,
        kButtonRightShoulder	= 0x00000800,
        kButtonLeftShoulder2	= 0x00001000,
        kButtonRightShoulder2	= 0x00002000,
        kButtonLeft3			= 0x00004000,
        kButtonRight3			= 0x00008000,
        kButtonTouch			= 0x00010000,
        kButtonOption			= 0x00020000,
        kButtonTrigger			= 0x00040000,
        kButtonSystem			= 0x00080000,
        kButtonSpecial			= 0x00100000,
        
        kNumButtonIDs			= 29		  
    };
    
    //////////////////////////////////////////////////////////////////////////
    static inline bool ButtonDown(u32 buttonState, ButtonID buttonID)
    {
        return (buttonState & (buttonID)) ? true : false;
    }
    
    //////////////////////////////////////////////////////////////////////////
    static inline bool ButtonUp(u32 buttonState, ButtonID buttonID)
    {
        return ( ( ~buttonState ) & ( buttonID ) ) ? true : false;
    }
    
    //////////////////////////////////////////////////////////////////////////
    static inline bool ButtonPressed(u32 buttonState, u32 prevButtonState, ButtonID buttonID)
    {
        u32 pressedButtonState = (buttonState & ~prevButtonState);
        return ButtonDown(pressedButtonState, buttonID);
    }
    
    //////////////////////////////////////////////////////////////////////////
    static inline bool ButtonReleased(u32 buttonState, u32 prevButtonState, ButtonID buttonID)
    {
        u32 releasedButtonState = (~buttonState & prevButtonState);
        return ButtonDown(releasedButtonState, buttonID);
    }

}