//
//  dmf_ios.cpp
//  dmfIOs
//
//  Created by Simon on 06/11/2014.
//
//

#include <dmf_device_ios.hpp>

using namespace DMF;

static CMMotionManager * spMotionManager = NULL;


ReturnCode DeviceIOS::Start()
{

    if(!spMotionManager)
    {
        spMotionManager = [[CMMotionManager alloc] init];
    }
    
    spMotionManager.deviceMotionUpdateInterval = 0.1;
    
    [spMotionManager startDeviceMotionUpdatesToQueue:[NSOperationQueue mainQueue]
                                       withHandler:^(CMDeviceMotion * motion, NSError *error) {
     
     this->mRoll = motion.attitude.roll;
     this->mPitch = motion.attitude.pitch;
     this->mYaw = motion.attitude.yaw;
     
     }];
    
    return kOK;

}


ReturnCode DeviceIOS::GetDataMap(DeviceDataMap& dataMap)
{
	// MAKE ALL THIS LOCAL TO EACH DEVICE
	// i.e. each device has a DeviceDataMap structure setup at Init - which is then just copied to the client request
	// and stuff
	
	dataMap.mDeviceName = "IOS_DEV";
	dataMap.mDeviceID = 1;
    
    u32 sizeOfPositions = 3 * sizeof(f32);
    
	DataBufferDescriptor descriptor;
    
	descriptor.mName = "FloatPositionData";
	descriptor.mID = 0xFFFF;
    descriptor.mFormat = kDataFormatFloat;
	descriptor.mNumElements = 3;
    descriptor.mRangeMin = -M_PI;
    descriptor.mRangeMax = M_PI;
	descriptor.mTotalSizeBytes = sizeof(sizeOfPositions);
    
	dataMap.mDataBuffer[0].mDescriptor = descriptor;
    

	return kOK;
}

ReturnCode DeviceIOS::Update(DeviceDataMap* pDestination)
{
    
    DeviceDataMap &inputMap = *pDestination;
    
    f32* pData1 = NULL;
    ReturnCode ret = GetDataPtr(inputMap, &pData1, "FloatPositionData", kDataFormatFloat);
    pData1[0] = mPitch;
    pData1[1] = mRoll;
    pData1[2] = mYaw;
    
    return ret;
}