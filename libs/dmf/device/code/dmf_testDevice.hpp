#pragma once

#include <dmf_device.hpp>

namespace DMF
{
    
    //this will be just for providing some random data for testing out fann

	class TestDevice : public Device{

	public:

        ReturnCode GetDataMap(DeviceDataMap& dataMap);
		ReturnCode Update(DeviceDataMap* pDestination);

	private:
        

	};

}

