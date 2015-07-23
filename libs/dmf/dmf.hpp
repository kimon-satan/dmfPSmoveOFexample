#pragma once

//////////////////////////////////////////////////////////////////////////
// common constants / macros here

// DLL base includes etc.

#include <string>

#define DMF_MAX_DATA_BUFFERS	16
#define DMF_MAX_CUSTOM_BUFFERS	2
#define DMF_MAX_CUSTOM_OFFSETS	8

namespace DMF
{
	typedef unsigned char		ubyte;
	typedef unsigned char		u8;
	typedef unsigned short int	u16;
	typedef unsigned int 		u32;
	typedef unsigned long long  u64;

	typedef signed char			byte;
	typedef signed char			s8;
	typedef signed short int	s16;
	typedef signed int 			s32;
	typedef signed long long  	s64;

	typedef float				f32;
	typedef double				f64;

	// common return codes
	enum ReturnCode
	{
		kOK			= 0,
		kFail		= -1,
		kNotFound	= -2,
		kInvalid	= -3
	};

	//////////////////////////////////////////////////////////////////////////
	enum DataFormat
	{
		kDataFormatUnknown	= 0x00000000,
		kDataFormatByte		= 0x00000001,
		kDataFormatUByte	= 0x00000002,
		kDataFormatShort	= 0x00000004,
		kDataFormatUShort	= 0x00000008,
		kDataFormatInt		= 0x00000010,
		kDataFormatUInt		= 0x00000020,
		kDataFormatLong		= 0x00000040,
		kDataFormatULong	= 0x00000080,
		kDataFormatFloat	= 0x00000100,
		kDataFormatDouble	= 0x00000200,
		kDataFormatCustom	= 0x20000000
	};

	//////////////////////////////////////////////////////////////////////////
	struct DataBufferDescriptor
	{
		std::string mName;
		f32			mRangeMin		= 0; // I changed these as doesn't make sense ints
		f32			mRangeMax		= 0;
		DataFormat	mFormat			= kDataFormatUnknown;
		u32			mID				= 0;
		u32			mNumElements	= 0;
		u32			mWidth			= 1;
		u32			mHeight			= 1;
		u32			mDepth			= 1;
		u32			mTotalSizeBytes = 0;// 32 bit should be enough for the moment..
	};

	//////////////////////////////////////////////////////////////////////////
	struct DataBuffer
	{
		~DataBuffer()
		{
            // the user will have to call free for the time being
			//Free();
		}
		void Free()
		{
			if (mpData)
			{
				delete[]mpData;
				mpData = NULL;
			}
		}
		DataBufferDescriptor	mDescriptor;
		ubyte*					mpData		= NULL; 
        u32						mFrameCount = 0;
	};
	
	//////////////////////////////////////////////////////////////////////////
	struct CustomDataBuffer
	{
		~CustomDataBuffer()
		{
            // the user will have to call free for the time being
			//Free();
		}
		void Free()
		{
			if (mpData)
			{
				delete[]mpData;
				mpData = NULL;
			}
		}
		DataBuffer				mDataBuffer[DMF_MAX_CUSTOM_OFFSETS];
		DataBufferDescriptor	mDescriptor;
		u32						mNumDataBuffers = 0;
        ubyte*					mpData			= NULL;
        u32						mFrameCount		= 0;
	};

	//////////////////////////////////////////////////////////////////////////
	struct DeviceDataMap
	{
		DataBuffer			mDataBuffer[DMF_MAX_DATA_BUFFERS];
		CustomDataBuffer	mCustomBuffer[DMF_MAX_CUSTOM_BUFFERS];
		std::string			mDeviceType;
		std::string			mDeviceName;
		u32					mDeviceID				= 0;
		u32					mNumCustomDataBuffers	= 0;
		u32					mNumDataBuffers			= 0;
	};	

	//////////////////////////////////////////////////////////////////////////
	struct DeviceBufferMap
	{
		~DeviceBufferMap()
		{
			Free();
		}
		void Free()
		{
			if (mpData)
			{
				delete[]mpData;
				mpData = NULL;
			}
		}
		DataBuffer* mpData		= NULL;
		u32			mDeviceID	= 0;
	};
}