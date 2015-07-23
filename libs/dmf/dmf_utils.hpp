#pragma once

#include <assert.h>
#include <dmf.hpp>
//////////////////////////////////////////////////////////////////////////
/*////////////////////////////////////////////////////////////////////////

	// General work-flow concepts

	PRE: Get available devices													(TODO)

		1.	Define working data to mapping layer / outputs etc. - i.e. what does this program want from a device?
			
		2.	Get device map														(PER-DEVICE)

		3.	Find any matching input data streams to our requirements			(TODO)

		4.	Allocate a DeviceDataMap from the deviceMap (client side memory)	(DONE)

		5.	Cache pointers to data offsets into client side memory				(DONE)
		
		6.	Call->pDevice->Update(pDataBuffer);									(PER-DEVICE)
				OR:
			pDevice->Update(pCustomDataBuffer);						
				OR:
			pDevice->Update(DeviceMap* pDevMap); // the whole thing					

		7.	Work with new updated data:
			
			e.g.	Use data directly 
						OR:
					Pass DataBuffer pointer to Mapping Layer 
						etc.

		8. Do outputs.




	Utility code to query data buffers / maps returned from devices etc.

	Some Ideas (TODO):
		Get all compatible data buffers by type:
			e.g. if the application say, wants 3 float vectors, it could:

			Get For each device (map)


			ReturnCode GetDataBuffersByType(DeviceMap& deviceMap, DataFormat format, DataBuffer **ppDataBuffers, u32& numBuffersFound )
			{
				for i -> dataBuffers 
					
				for i -> customBuffers

				Allocate an array of pointers, fill in with data pointers that match the type.

				this could be extended to the device handler as a global query function

				returning an array of:

				struct DeviceBufferMap
				{
					DataBuffer* pDataBuffer; 
					u32 deviceID;
				}
			}

			So the client then knows what device, and the data buffer it needs
				
			// we might then allocate a local DeviceBufferMap according to this, and pass to the handler as:

			DeviceHandler::Update(DeviceBufferMap->deviceID, DeviceBufferMap->pDataBuffer);
			

*/////////////////////////////////////////////////////////////////////////
namespace DMF
{

	
#define DMF_STRINGIFY(ITEM) #ITEM

	//////////////////////////////////////////////////////////////////////////
	static inline s32 GetSizeInBytes(DataFormat dataFormat)
	{
		switch (dataFormat)
		{
		case kDataFormatByte	:	return sizeof(byte);
		case kDataFormatUByte	:	return sizeof(ubyte);
		case kDataFormatShort	:	return sizeof(s16);
		case kDataFormatUShort	:	return sizeof(u16);
		case kDataFormatInt		:	return sizeof(s32);
		case kDataFormatUInt	:	return sizeof(u32);
		case kDataFormatLong	:	return sizeof(s64);
		case kDataFormatULong	:	return sizeof(u64);
		case kDataFormatFloat	:	return sizeof(f32);
		case kDataFormatDouble	:	return sizeof(f64);
		case kDataFormatUnknown: 
			default:
				return -1;
		}
	}

	//////////////////////////////////////////////////////////////////////////
	static inline ReturnCode Allocate(DeviceDataMap& inputMap, DeviceDataMap** ppOutDeviceMap)
	{
		DeviceDataMap* pDeviceMap = new(DeviceDataMap);

		*pDeviceMap	= inputMap;

		//pDeviceMap->mDeviceName = inputMap.mDeviceName;
		//pDeviceMap->mDeviceID	= inputMap.mDeviceID;
		//
		// Allocate memory for data buffers
		for (u32 i = 0; inputMap.mDataBuffer[i].mDescriptor.mFormat != kDataFormatUnknown && i < DMF_MAX_DATA_BUFFERS; ++i)
		{
			DataBuffer& input = inputMap.mDataBuffer[i];
			pDeviceMap->mDataBuffer[i]			= input;
			pDeviceMap->mDataBuffer[i].mpData = new ubyte[input.mDescriptor.mNumElements * GetSizeInBytes(input.mDescriptor.mFormat)];
		}

		// Allocate for any custom data buffers
		for (u32 i = 0; inputMap.mCustomBuffer[i].mDescriptor.mFormat == kDataFormatCustom && i < DMF_MAX_CUSTOM_BUFFERS; ++i)
		{
			CustomDataBuffer& customDB = inputMap.mCustomBuffer[i];
			pDeviceMap->mCustomBuffer[i] = customDB;
			pDeviceMap->mCustomBuffer[i].mpData = new ubyte[customDB.mDescriptor.mTotalSizeBytes];
						
			u32 offSet = 0;
			for (u32 j = 0; customDB.mDataBuffer[j].mDescriptor.mFormat != kDataFormatUnknown && j < DMF_MAX_CUSTOM_OFFSETS; ++j)
			{
				DataBuffer& input = customDB.mDataBuffer[j];
				pDeviceMap->mCustomBuffer[i].mDataBuffer[j] = input;
				pDeviceMap->mCustomBuffer[i].mDataBuffer[j].mpData = pDeviceMap->mCustomBuffer[i].mpData + offSet; 
				offSet += input.mDescriptor.mTotalSizeBytes;
			}
		}
	
		*ppOutDeviceMap = pDeviceMap;
		return kOK;
	}
    
    static inline ReturnCode Allocate(DeviceDataMap& inputMap)
    {
        // Allocate memory for data buffers
        for (u32 i = 0; inputMap.mDataBuffer[i].mDescriptor.mFormat != kDataFormatUnknown && i < DMF_MAX_DATA_BUFFERS; ++i)
        {
            inputMap.mDataBuffer[i].mpData = new ubyte[inputMap.mDataBuffer[i].mDescriptor.mTotalSizeBytes];
        }
        
        // Allocate for any custom data buffers
        for (u32 i = 0; inputMap.mCustomBuffer[i].mDescriptor.mFormat == kDataFormatCustom && i < DMF_MAX_CUSTOM_BUFFERS; ++i)
        {
            CustomDataBuffer& customDB = inputMap.mCustomBuffer[i];
            customDB.mpData = new ubyte[customDB.mDescriptor.mTotalSizeBytes];
            
            u32 offSet = 0;
            for (u32 j = 0; customDB.mDataBuffer[j].mDescriptor.mFormat != kDataFormatUnknown && j < DMF_MAX_CUSTOM_OFFSETS; ++j)
            {
                DataBuffer& input               = customDB.mDataBuffer[j];
                customDB.mDataBuffer[j].mpData  = customDB.mpData + offSet;
                offSet                          += input.mDescriptor.mTotalSizeBytes;
            }
        }
        
        return kOK;
    }
    
    static inline ReturnCode Allocate(DataBuffer & inDb, DataBuffer ** ppOutDb = NULL)
    {
        
        inDb.mpData = new ubyte[inDb.mDescriptor.mNumElements * GetSizeInBytes(inDb.mDescriptor.mFormat)];
		

        if(ppOutDb != NULL)
        {
            DataBuffer * pOutDb = new (DataBuffer);
            *pOutDb = inDb;
            *ppOutDb = pOutDb;
        }
        
        return kOK;
    }
    
    //////////////////////////////////////////////////////////////////////////
    
    
	template <typename DATA_TYPE>
	static inline ReturnCode GetPtrAs(ubyte * pData, DATA_TYPE **ppData)
	{
        
        *ppData = reinterpret_cast<DATA_TYPE*>(pData);
        return kOK;
        
	}
    
    /////////////////////////////////////////////////////////////////////////////    
    static inline ReturnCode Clone(const DataBuffer & inDb, DataBuffer ** ppOutDb){
        
        DataBuffer * pOutDb = new (DataBuffer);
        *pOutDb = inDb;
        *ppOutDb = pOutDb;
        
        pOutDb->mpData = new ubyte[inDb.mDescriptor.mNumElements * GetSizeInBytes(inDb.mDescriptor.mFormat)];
        
        
        for(u8 i = 0; i < inDb.mDescriptor.mNumElements; i++)
        {
            
            f32 * s_ptr, * d_ptr;
            GetPtrAs(pOutDb->mpData , &d_ptr);
            GetPtrAs(inDb.mpData , &s_ptr);
            
            d_ptr[i] = s_ptr[i];
           
        }
        
        return kOK;
    }
	
	//////////////////////////////////////////////////////////////////////////
	static inline ReturnCode CopyData(DataBuffer& src, DataBuffer& dst )
	{
		assert(dst.mDescriptor.mTotalSizeBytes >= src.mDescriptor.mTotalSizeBytes);
		if(dst.mDescriptor.mTotalSizeBytes < src.mDescriptor.mTotalSizeBytes)
		{
			return kFail;
		}
		memcpy(dst.mpData, src.mpData, src.mDescriptor.mTotalSizeBytes);
        return kOK;
	}
    
	//////////////////////////////////////////////////////////////////////////
	static inline ReturnCode CopyData(CustomDataBuffer& src, CustomDataBuffer& dst )
	{		
		assert(dst.mDescriptor.mTotalSizeBytes >= src.mDescriptor.mTotalSizeBytes);
		if(dst.mDescriptor.mTotalSizeBytes < src.mDescriptor.mTotalSizeBytes)
		{
			return kFail;
		}
		memcpy(dst.mpData, src.mpData, src.mDescriptor.mTotalSizeBytes);
		return kOK;
	}    

	//////////////////////////////////////////////////////////////////////////
	// Gets the data buffer by id and type within a custom structure
	// For use when querying the device for a data subset
	// expects valid data in the first elements of the array
	static inline ReturnCode GetDataBuffer(CustomDataBuffer& customDB, DataBuffer **ppBuffer, u32 id, DataFormat format)
	{
		for (u32 i = 0; customDB.mDataBuffer[i].mDescriptor.mFormat == kDataFormatCustom && i < DMF_MAX_CUSTOM_OFFSETS; ++i)
		{
			DataBuffer& dataBuffer = customDB.mDataBuffer[i];
			if (dataBuffer.mDescriptor.mFormat == format && id == dataBuffer.mDescriptor.mID)
			{
				*ppBuffer = &customDB.mDataBuffer[i];
				return kOK;
			}
		}
		return kNotFound;
	}

	//////////////////////////////////////////////////////////////////////////
	// Gets the data buffer by id and type within a custom structure
	// For use when querying the device for a data subset
	static inline ReturnCode GetDataBuffer(CustomDataBuffer& customDB, DataBuffer **ppBuffer, const std::string& name, DataFormat format)
	{
		for (u32 i = 0; customDB.mDataBuffer[i].mDescriptor.mFormat == kDataFormatCustom && i < DMF_MAX_CUSTOM_OFFSETS; ++i)
		{
			DataBuffer& dataBuffer = customDB.mDataBuffer[i];
			if (dataBuffer.mDescriptor.mFormat == format && name == dataBuffer.mDescriptor.mName)
			{
				*ppBuffer = &customDB.mDataBuffer[i];
				return kOK;
			}
		}
		return kNotFound;
	}

	//////////////////////////////////////////////////////////////////////////
	// Gets the data buffer by id and type within a DeviceDataMap
	// For use when querying the device for a data subset
	static inline ReturnCode GetDataBuffer(DeviceDataMap& deviceMap, DataBuffer **ppBuffer, u32 id, DataFormat format)
	{
		for (u32 i = 0; deviceMap.mCustomBuffer[i].mDescriptor.mFormat == kDataFormatCustom && i < DMF_MAX_CUSTOM_BUFFERS; ++i)
		{
			CustomDataBuffer& customDB = deviceMap.mCustomBuffer[i];
			if (kOK == GetDataBuffer(customDB, ppBuffer, id, format))
			{
				return kOK;
			}
		}
		for (u32 j = 0; deviceMap.mDataBuffer[j].mDescriptor.mFormat != kDataFormatUnknown && j < DMF_MAX_DATA_BUFFERS; ++j)
		{
			DataBuffer&	dataBuffer = deviceMap.mDataBuffer[j];
			if (dataBuffer.mDescriptor.mID == id && dataBuffer.mDescriptor.mFormat == format)
			{
				*ppBuffer = &deviceMap.mDataBuffer[j];
				return kOK;
			}
		}
		return kNotFound;
	}

	//////////////////////////////////////////////////////////////////////////
	// Gets the data buffer by id and type within a DeviceDataMap
	// For use when querying the device for a data subset
	static inline ReturnCode GetDataBuffer(DeviceDataMap& deviceMap, DataBuffer **ppBuffer, const std::string& name, DataFormat format)
	{
		for (u32 i = 0; deviceMap.mCustomBuffer[i].mDescriptor.mFormat == kDataFormatCustom && i < DMF_MAX_CUSTOM_BUFFERS; ++i)
		{
			CustomDataBuffer& customDB = deviceMap.mCustomBuffer[i];
			if (kOK == GetDataBuffer(customDB, ppBuffer, name, format))
			{
				return kOK;
			}
		}
		for (u32 j = 0; deviceMap.mDataBuffer[j].mDescriptor.mFormat != kDataFormatUnknown && j < DMF_MAX_DATA_BUFFERS; ++j)
		{
			DataBuffer&	dataBuffer = deviceMap.mDataBuffer[j];
			if (dataBuffer.mDescriptor.mName == name && dataBuffer.mDescriptor.mFormat == format)
			{
				*ppBuffer = &deviceMap.mDataBuffer[j];
				return kOK;
			}
		}
		return kNotFound;
	}
	
	//////////////////////////////////////////////////////////////////////////
	// Gets the data pointer by id and type within a custom structure 
	// ( type safe pointer cache of client side memory )
	template <typename DATA_TYPE>
	static inline ReturnCode GetDataPtr(CustomDataBuffer& customDB, DATA_TYPE **ppData, u32 id, DataFormat format)
	{
		for (u32 i = 0; customDB.mDataBuffer[i].mDescriptor.mFormat != kDataFormatUnknown && i < DMF_MAX_CUSTOM_OFFSETS; ++i)
		{
			DataBuffer& dataBuffer = customDB.mDataBuffer[i];
			if (dataBuffer.mDescriptor.mFormat == format && id == dataBuffer.mDescriptor.mID)
			{
				*ppData = reinterpret_cast<DATA_TYPE*>(dataBuffer.mpData);
				return kOK;
			}
		}
		return kNotFound;
	}



	//////////////////////////////////////////////////////////////////////////
	// Gets the data buffer by id and type within a DeviceDataMap  
	// ( type safe pointer cache of client side memory )
	template <typename DATA_TYPE>
	static inline ReturnCode GetDataPtr(DeviceDataMap& deviceMap, DATA_TYPE **ppData, u32 id, DataFormat format)
	{
		for (u32 i = 0; deviceMap.mCustomBuffer[i].mDescriptor.mFormat == kDataFormatCustom && i < DMF_MAX_CUSTOM_BUFFERS; ++i)
		{
			CustomDataBuffer& customDB = deviceMap.mCustomBuffer[i];
			if (kOK == GetDataPtr(customDB, ppData, id, format))
			{
				return kOK;
			}
		}
		for (u32 i = 0; deviceMap.mDataBuffer[i].mDescriptor.mFormat != kDataFormatUnknown && i < DMF_MAX_DATA_BUFFERS; ++i)
		{
			DataBuffer& dataBuffer = deviceMap.mDataBuffer[i];
			if (kOK == GetDataPtr(dataBuffer, ppData, id, format))
			{
				return kOK;
			}
		}
		return kNotFound;
	}
    
    //////////////////////////////////////////////////////////////////////////
	// Gets the data buffer by id and type within a custom structure
	// ( type safe pointer cache of client side memory )
	template <typename DATA_TYPE>
	static inline ReturnCode GetDataPtr(CustomDataBuffer& customDB, DATA_TYPE **ppData, const std::string& name, DataFormat format)
	{
		for (u32 i = 0; customDB.mDataBuffer[i].mDescriptor.mFormat != kDataFormatUnknown && i < DMF_MAX_CUSTOM_OFFSETS; ++i)
		{
			DataBuffer& dataBuffer = customDB.mDataBuffer[i];
			if (dataBuffer.mDescriptor.mFormat == format && name == dataBuffer.mDescriptor.mName)
			{
				*ppData = reinterpret_cast<DATA_TYPE*>(dataBuffer.mpData);
				return kOK;
			}
		}
		return kNotFound;
	}

	//////////////////////////////////////////////////////////////////////////
	// Gets the data buffer by id and type within a DeviceDataMap  
	// ( type safe pointer cache of client side memory )
	template <typename DATA_TYPE>
	static inline ReturnCode GetDataPtr(DeviceDataMap& deviceMap, DATA_TYPE **ppData, const std::string& name, DataFormat format)
	{
		for (u32 i = 0; deviceMap.mCustomBuffer[i].mDescriptor.mFormat == kDataFormatCustom && i < DMF_MAX_CUSTOM_BUFFERS; ++i)
		{
			CustomDataBuffer& customDB = deviceMap.mCustomBuffer[i];
			if (kOK == GetDataPtr(customDB, ppData, name, format))
			{
				return kOK;
			}
		}
		for (u32 i = 0; deviceMap.mDataBuffer[i].mDescriptor.mFormat != kDataFormatUnknown && i < DMF_MAX_DATA_BUFFERS; ++i)
		{
			DataBuffer& dataBuffer = deviceMap.mDataBuffer[i];
			if (kOK == GetDataPtr(dataBuffer, ppData, name, format))
			{
				return kOK;
			}
		}
		return kNotFound;
	}

	//////////////////////////////////////////////////////////////////////////
	// Gets the data pointer by id and type within a DataBuffer structure 
	// ( type safe pointer cache of client side memory )
	template <typename DATA_TYPE>
	static inline ReturnCode GetDataPtr(DataBuffer& dataBuffer, DATA_TYPE **ppData, u32 id, DataFormat format)
	{
		if (dataBuffer.mDescriptor.mFormat == format && id == dataBuffer.mDescriptor.mID)
		{
			*ppData = reinterpret_cast<DATA_TYPE*>(dataBuffer.mpData);
			return kOK;
		}
		return kInvalid;
	}

	//////////////////////////////////////////////////////////////////////////
	// Gets the data pointer by name and type within a DataBuffer structure 
	// ( type safe pointer cache of client side memory )
	template <typename DATA_TYPE>
	static inline ReturnCode GetDataPtr(DataBuffer& dataBuffer, DATA_TYPE **ppData, const std::string& name, DataFormat format)
	{
		if (dataBuffer.mDescriptor.mFormat == format && name == dataBuffer.mDescriptor.mName)
		{
			*ppData = reinterpret_cast<DATA_TYPE*>(dataBuffer.mpData);
			return kOK;
		}
		return kInvalid;
	}

	//////////////////////////////////////////////////////////////////////////
	static inline  std::string GetDataFormatString( DataFormat fmt)
	{
		switch(fmt)
		{

			case kDataFormatUnknown		:	 return DMF_STRINGIFY( kDataFormatUnknown	 );
			case kDataFormatByte		:	 return DMF_STRINGIFY( kDataFormatByte		 );
			case kDataFormatUByte		:	 return DMF_STRINGIFY( kDataFormatUByte		 );
			case kDataFormatShort		:	 return DMF_STRINGIFY( kDataFormatShort		 );
			case kDataFormatUShort		:	 return DMF_STRINGIFY( kDataFormatUShort	 );
			case kDataFormatInt			:	 return DMF_STRINGIFY( kDataFormatInt		 );
			case kDataFormatUInt		:	 return DMF_STRINGIFY( kDataFormatUInt		 );
			case kDataFormatLong		:	 return DMF_STRINGIFY( kDataFormatLong		 );
			case kDataFormatULong		:	 return DMF_STRINGIFY( kDataFormatULong		 );
			case kDataFormatFloat		:	 return DMF_STRINGIFY( kDataFormatFloat		 );
			case kDataFormatDouble		:	 return DMF_STRINGIFY( kDataFormatDouble	 );
			case kDataFormatCustom		:	 return DMF_STRINGIFY( kDataFormatCustom	 );

		default:   return DMF_STRINGIFY(kDataFormatUnknown );
		}
	}
	
	static inline void PrintDescriptor(DataBufferDescriptor& desc)
	{
		printf("Device Attribute: %s, Device Attribute ID: 0x%x\n", desc.mName.c_str(), desc.mID);
		printf("       RangeMin: %.2f, RangeMax: %.2f\n",desc.mRangeMin, desc.mRangeMax);
		printf("       Format: %s\n", GetDataFormatString(desc.mFormat).c_str());		
		printf("       NumElements: %d\n", desc.mNumElements);
		printf("       Width: %d, Height: %d, Depth: %d\n", desc.mWidth, desc.mHeight, desc.mDepth);	
	}
    
    static inline void printDeviceParams(DeviceDataMap & clientData)
    {

        for(u8 i = 0; i < DMF_MAX_DATA_BUFFERS; ++i)
        {
            
            
            DataFormat format = clientData.mDataBuffer[i].mDescriptor.mFormat;
            std::string name = clientData.mDataBuffer[i].mDescriptor.mName;
            u16 numElements = clientData.mDataBuffer[i].mDescriptor.mNumElements;
            
            if(numElements < 1)
            {
               continue;
            }
            
            printf("\n%s, %i, %i \n",
                   name.c_str(),
                   format,
                   numElements);
        
            
            switch(format)
            {
                    
                case kDataFormatFloat:
                {
                    f32* pData = NULL;
                    ReturnCode ret = GetDataPtr(clientData, &pData, name, format);
                    for(u16 k = 0; k < numElements; ++k){
                        printf( "%f, ", pData[k]);
                    }
                }
                    break;
                    
                case kDataFormatInt:
                {
                    s32* pData = NULL;
                    ReturnCode ret = GetDataPtr(clientData, &pData, name, format);
                    for(u16 k = 0; k < numElements; ++k){
                        printf( " %i, ", pData[k]);
                    }
                }
                    break;
                    
                case kDataFormatUInt:
                {
                    u32* pData = NULL;
                    ReturnCode ret = GetDataPtr(clientData, &pData, name, format);
                    for(u16 k = 0; k < numElements; ++k){
                        printf( " %i, ", pData[k]);
                    }
                }
                    break;
                    
                    default:
                    break;
                    
            }
        }
        
    }

}