//
//  dmf_ios.h
//  dmfIOs
//
//  Created by Simon on 06/11/2014.
//
//

#pragma once



#include <dmf_device.hpp>
#import <UIKit/UIKit.h>
#import <CoreMotion/CoreMotion.h>



namespace DMF
{
    
    //this will be just for providing some random data for testing out fann
    
	class DeviceIOS : public Device
    {

        
	public:
        
        ReturnCode Start();
        ReturnCode GetDataMap(DeviceDataMap& dataMap);
		ReturnCode Update(DeviceDataMap* pDestination);

        
	private:
        
        f32 mPitch, mRoll, mYaw;
        
	};
    
}


