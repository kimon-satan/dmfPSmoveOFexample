//
//  dmf_testDevice.cpp
//  soundLab_cmd
//
//  Created by Simon on 04/11/2014.
//  Copyright (c) 2014 __PUBLICDOMAINCORP___. All rights reserved.
//

#include <dmf_testDevice.hpp>

using namespace DMF;

struct TestDevDataStructure
{
	f32 mFloatPositionData1[3];
	f32 mFloatPositionData2[3];
	s32 mIntVector2[2];
	u32 mUIntVar;
};


ReturnCode TestDevice::GetDataMap(DeviceDataMap& dataMap)
{
	// MAKE ALL THIS LOCAL TO EACH DEVICE
	// i.e. each device has a DeviceDataMap structure setup at Init - which is then just copied to the client request
	// and stuff
	
	dataMap.mDeviceName = "CUSTOM_TEST_1";
	dataMap.mDeviceID = 1;
    
	DataBufferDescriptor customDescriptor;
    
	customDescriptor.mName = "CustomDataStructure";
	customDescriptor.mID = 0xFFFF;
	customDescriptor.mFormat = kDataFormatCustom;
	customDescriptor.mNumElements = 4;
	customDescriptor.mTotalSizeBytes = sizeof(TestDevDataStructure);
    
	dataMap.mCustomBuffer[0].mDescriptor = customDescriptor;
    
	const char* posNames[2] =
	{
		"FloatPositionData1",
		"FloatPositionData2"
	};
    
	u32 sizeOfPositions = 3 * sizeof(f32);
    
	DataBuffer dataBuffers[4];
    
	for (u32 i = 0; i < 2; ++i)
	{
		dataBuffers[i].mDescriptor.mName = posNames[i];
		dataBuffers[i].mDescriptor.mID = 0;
		dataBuffers[i].mDescriptor.mFormat = kDataFormatFloat;
		dataBuffers[i].mDescriptor.mNumElements = 3;
		dataBuffers[i].mDescriptor.mTotalSizeBytes = sizeOfPositions;
	}
	
	dataBuffers[2].mDescriptor.mName = "IntVector2";
	dataBuffers[2].mDescriptor.mID = 0;
	dataBuffers[2].mDescriptor.mFormat = kDataFormatInt;
	dataBuffers[2].mDescriptor.mNumElements = 2;
	dataBuffers[2].mDescriptor.mTotalSizeBytes = 2 * sizeof(s32);
    
	dataBuffers[3].mDescriptor.mName = "UIntVar";
	dataBuffers[3].mDescriptor.mID = 0;
	dataBuffers[3].mDescriptor.mFormat = kDataFormatUInt;
	dataBuffers[3].mDescriptor.mNumElements = 1;
	dataBuffers[3].mDescriptor.mTotalSizeBytes = sizeof(u32);
    
	for (u32 i = 0; i < 4; ++i)
	{
		dataMap.mCustomBuffer[0].mDataBuffer[i] = dataBuffers[i]; //everything packaged in a custom buffer
	}
    
	return kOK;
}

ReturnCode TestDevice::Update(DeviceDataMap* pDestination){

    DeviceDataMap &inputMap = *pDestination;
    
    f32* pData1 = NULL;
    ReturnCode ret = GetDataPtr(inputMap, &pData1, "FloatPositionData1", kDataFormatFloat);
    pData1[0] = (f32)rand()/ RAND_MAX;
    pData1[1] = (f32)rand()/ RAND_MAX;
    pData1[2] = (f32)rand()/ RAND_MAX;
    
    f32* pData2 = NULL;
    ret = GetDataPtr(inputMap, &pData2, "FloatPositionData2", kDataFormatFloat);
    pData2[0] = (f32)rand()/ RAND_MAX;
    pData2[1] = (f32)rand()/ RAND_MAX;
    pData2[2] = (f32)rand()/ RAND_MAX;
    
    
    s32* pData3 = NULL;
    ret = GetDataPtr(inputMap, &pData3, "IntVector2", kDataFormatInt);
    pData3[0] = (s32)rand() % 1000 - 500;
    pData3[1] = (s32)rand() % 1000 - 500;
    
    u32* pData4 = NULL;
    ret = GetDataPtr(inputMap, &pData4, "UIntVar", kDataFormatUInt);
    pData4[0] = (u32)rand() % 1000;


    
    return ret;
}