#include <dmf.hpp>
#include <dmf_devicehandler.hpp>
#include <dmf_device.hpp>
#include <dmf_testDevice.hpp>
#include <dmf_crc.hpp>

#ifdef _WIN32
#include <unordered_map>
#include <windows.h>
#define CPP_EXT std


#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/progress.hpp>

namespace fs = boost::filesystem;

#else
#include <tr1/unordered_map>
#define CPP_EXT std::tr1
#include <dirent.h>
#include <dlfcn.h>
#include <unistd.h>
#endif

#include <vector>

#include <sstream>
#include <iostream>


namespace DMF
{
	enum DeviceStatus
	{
		kPluginStatusUnknown,
		kPluginStatusLoaded,
		kPluginStatusUnloaded
	};
	//////////////////////////////////////////////////////////////////////////
	class DeviceRegistrationData
	{
	public:
		DeviceRegistrationData(){}
		DeviceRegistrationData
		(
			const std::string&			name,
			const std::string&			filename,
			DynamicLibraryAPIVersion	version,
			DeviceStatus				status,
			void*						pLoadedLibrary
		)
		{
			mName		= name;
			mFileName	= filename;
			mVersion	= version;
			mHashedID	= GetCRC(name);
			mStatus		= status;
			mpHandle	= pLoadedLibrary;
		}

		std::string					mName;
		std::string					mFileName;
		DynamicLibraryAPIVersion	mVersion;
		u32							mHashedID	= 0;
		DeviceStatus				mStatus		= kPluginStatusUnknown;
		void						*mpHandle	= NULL;
	};

	//////////////////////////////////////////////////////////////////////////
	struct DeviceRegistry
	{	
		typedef CPP_EXT::unordered_map<u32, DeviceRegistrationData> DeviceMap;
		typedef DeviceMap::iterator								DeviceIterator;
		DeviceMap mPlugins;

		std::vector<DeviceDataMap>	mCurrentDeviceInfo;
        std::vector<DeviceDataMap>	mCurrentDeviceOutputInfo;
		
        DeviceHandler*				mDeviceHandlers[kMaxNumDevices];
		u32							mNumDeviceHandlers	= 0;
	};

}

//////////////////////////////////////////////////////////////////////////
DMF::ReturnCode DMF::DeviceHandler::Init()
{	
	mpDeviceRegistry = new DeviceRegistry;
	for(u32 i = 0; i < kMaxNumDevices; ++i)
	{
        mpDeviceRegistry->mDeviceHandlers[i]	= NULL;
	}
	mpDeviceRegistry->mCurrentDeviceInfo.reserve(kMaxNumDevices);

	return kOK;
}

//////////////////////////////////////////////////////////////////////////
void* LoadDLL(const char* filename)
{
	void* handle = NULL;
	std::string  errorString;  
#ifdef _WIN32
	handle = LoadLibraryA(filename);
	if (handle == NULL)
	{
		DWORD errorCode = GetLastError();
		std::stringstream ss;
		{
			ss	<< std::string("LoadLibrary(") << filename
				<< std::string(") Failed. errorCode: ")
				<< errorCode << "\n";
		}
		errorString = ss.str();
		printf(errorString.c_str());
	}
#else
	handle = dlopen(filename, RTLD_NOW);
	if (!handle) 
	{
		std::string dlErrorString;
		const char *zErrorString = ::dlerror();
		if (zErrorString)
		{
			dlErrorString = zErrorString;
		}
        errorString += "Failed to load \"" + std::string(filename) + '"';
		if(dlErrorString.size())
		{
			errorString += ": " + dlErrorString;
		}
		printf(errorString.c_str());
	}
#endif
	return handle;
}

//////////////////////////////////////////////////////////////////////////
void* FindLibraryFunction(void* library, const char* functionName)
{	
	void* pSym = NULL;
#ifdef _WIN32
	pSym = GetProcAddress((HMODULE)library, functionName);
#else
	pSym =  dlsym(library, functionName);
#endif
	if(!pSym)
	{
		printf("Plugin has no GetLibraryVersion: %s\n", functionName);
	}
	return pSym;
}

//////////////////////////////////////////////////////////////////////////
void UnLoadDLL(void* library)
{
#ifndef _WIN32
	dlclose(library);
#else
	FreeLibrary((HMODULE)library);
#endif
}

//////////////////////////////////////////////////////////////////////////
bool DMF::DeviceHandler::HandleDLL(const char* filename)
{
    void* dllHandle = LoadDLL(filename);
    std::string  errorString;
    
    if (dllHandle == NULL)
    {
        return false;
    }
    
    const char* libraryName = NULL;
    DynamicLibraryAPIVersion version;
    
    void *pSym	= FindLibraryFunction(dllHandle, "GetLibraryVersion");
    if(pSym)
    {
        GetLibraryVersionFunc getLibFunc = (GetLibraryVersionFunc)pSym;
        getLibFunc(version.mMajor, version.mMinor, &libraryName);
    }
    
    u32 hashedID = GetCRC(libraryName);
    if(mpDeviceRegistry->mPlugins.find(hashedID) != mpDeviceRegistry->mPlugins.end())
    {
        UnLoadDLL(dllHandle);
        return false;
    }
    
    DeviceHandler*  pPluginDeviceHandler	= NULL;
    pSym									= FindLibraryFunction(dllHandle, "CreateDeviceHandler");
    
    if(!pSym)
    {
		printf("Cannot find Symbol for CreateDeviceHandler in: %s\n", filename);
    }
    else
    {
        CreateDeviceHandlerFunc func = (CreateDeviceHandlerFunc)pSym;
        pPluginDeviceHandler = func();
        assert(pPluginDeviceHandler);
        if(pPluginDeviceHandler)
        {
            assert(mpDeviceRegistry->mNumDeviceHandlers < kMaxNumDevices);
            if(mpDeviceRegistry->mNumDeviceHandlers+1 >= kMaxNumDevices)
            {
                printf("Too many device handlers attached (%d) - need to up the limit", mpDeviceRegistry->mNumDeviceHandlers);
                delete pPluginDeviceHandler;
                return false;
            }
            
            // Init
            pPluginDeviceHandler->Init();
            
            // add to device list
            mpDeviceRegistry->mDeviceHandlers[mpDeviceRegistry->mNumDeviceHandlers++] = pPluginDeviceHandler;
            
            // add plugin registration data
            DeviceRegistrationData pluginData(libraryName, filename, version, kPluginStatusLoaded, dllHandle);
            mpDeviceRegistry->mPlugins[pluginData.mHashedID] = pluginData;
        }
        else
        {
            printf("Cannot create DeviceHandler from: %s\n", filename);						
            UnLoadDLL(dllHandle);
            return false;
        }
    }
    return true;
}

//////////////////////////////////////////////////////////////////////////
// Note - no handling of Unloading yet
DMF::ReturnCode DMF::DeviceHandler::LoadPlugins(const char* directory)
{
#ifdef _WIN32
    assert(directory);
    if(!directory)
    {
        printf("No plugin directory specified\n");
        return kFail;
    }
    //const char* directory = "plugins";
    fs::path full_path( fs::initial_path<fs::path>() );
    full_path = fs::system_complete( fs::path( directory) );

	if ( !fs::exists( full_path ) )
	{		
        printf("Plugin Manager failed to find directory: %s\n", full_path.filename().string().c_str());//ectory);
		return kFail;
	}

	if ( fs::is_directory( full_path ) )
	{
		fs::directory_iterator end_iter;
		for ( fs::directory_iterator dir_itr( full_path );  dir_itr != end_iter; ++dir_itr )
		{
			try
			{
				if ( fs::is_regular_file( dir_itr->status() ) )
				{
#ifdef _WIN32
					const char* dll = ".dll";
					std::string name (dir_itr->path().filename().string());
					if(std::strstr(name.c_str(), dll)) 
#else
					// osx is dylib?
					if(	strstr(dir_itr->path().filename().c_str(), ".so"))
#endif
					{
						std::string fullPathFileName( full_path.string().c_str());
						fullPathFileName += "\\";
						fullPathFileName += dir_itr->path().filename().string();

                        const char* filename = fullPathFileName.c_str();
                        printf ("Attempting to load plugin: %s\n", filename);
						//////////////////////////////////////////////////////////////////////////
                        bool result = HandleDLL(filename);
                        printf ("%s\n", result ? "Success\n" : "Fail\n" );
                    }
				}
			}
			catch ( const std::exception & ex )
			{
				std::cout << dir_itr->path().filename() << " " << ex.what() << std::endl;
			}
			
		}
	}
	else
	{
		std::cout << "\nPlugin Found: " << full_path.native().c_str() << "\n";    
	}
#else
    assert(directory);
    if(!directory)
    {
        printf("No plugin directory specified\n");
        return kFail;
    }
    DIR *dir;
    struct dirent *ent;
    if ((dir = opendir (directory)) != NULL)
    {
        /* print all the files and directories within directory */
        while ((ent = readdir (dir)) != NULL)
        {
            if(	strstr(ent->d_name, ".dylib"))
            {
                std::string fullPathName = std::string(directory) + "/" + std::string(ent->d_name);
                printf ("Attempting to load plugin: %s\n", fullPathName.c_str());
                bool result = HandleDLL(fullPathName.c_str());
                printf ("%s\n", result ? "Success\n" : "Fail\n" );
            }
        }
        closedir (dir);
    }
    else
    {
        /* could not open directory */
        perror ("");
        return kFail;
    }
    
#endif
	return kOK;
}

//////////////////////////////////////////////////////////////////////////
DMF::ReturnCode DMF::DeviceHandler::GetDeviceInfo(DeviceDataMap** ppDeviceMaps, u32& numDevices)
{
	assert(mpDeviceRegistry);
	if(!mpDeviceRegistry)
	{
		return kNotFound;
	}
	mpDeviceRegistry->mCurrentDeviceInfo.clear();

	for(u32 i = 0; i < mpDeviceRegistry->mNumDeviceHandlers; ++i)
	{
		if (mpDeviceRegistry->mDeviceHandlers[i])
		{
			DeviceDataMap* pDeviceMaps = NULL;
			u32 devicesAttached = 0;
			ReturnCode result = mpDeviceRegistry->mDeviceHandlers[i]->GetDeviceInfo(&pDeviceMaps, devicesAttached);

			if(devicesAttached > 0)
			{
				for(u32 j = 0; j < devicesAttached; ++j)
				{
					mpDeviceRegistry->mCurrentDeviceInfo.push_back(pDeviceMaps[j]);
				}
				delete []pDeviceMaps;
			}
		}	
	}
	if(mpDeviceRegistry->mCurrentDeviceInfo.size() > 0)
	{
		numDevices = (u32)mpDeviceRegistry->mCurrentDeviceInfo.size();
		*ppDeviceMaps = &mpDeviceRegistry->mCurrentDeviceInfo[0];
		return kOK;
	}
	return kNotFound;
}


DMF::ReturnCode DMF::DeviceHandler::GetDeviceOutputInfo(DeviceDataMap** ppDeviceMaps, u32& numDevices){
    
    assert(mpDeviceRegistry);
    if(!mpDeviceRegistry)
    {
        return kNotFound;
    }
    mpDeviceRegistry->mCurrentDeviceOutputInfo.clear();
    
    for(u32 i = 0; i < mpDeviceRegistry->mNumDeviceHandlers; ++i)
    {
        if (mpDeviceRegistry->mDeviceHandlers[i])
        {
            DeviceDataMap* pDeviceMaps = NULL;
            u32 devicesAttached = 0;
            ReturnCode result = mpDeviceRegistry->mDeviceHandlers[i]->GetDeviceOutputInfo(&pDeviceMaps, devicesAttached);
            
            if(devicesAttached > 0)
            {
                for(u32 j = 0; j < devicesAttached; ++j)
                {
                    mpDeviceRegistry->mCurrentDeviceOutputInfo.push_back(pDeviceMaps[j]);
                }
                delete []pDeviceMaps;
            }
        }
    }
    if(mpDeviceRegistry->mCurrentDeviceInfo.size() > 0)
    {
        numDevices = (u32)mpDeviceRegistry->mCurrentDeviceOutputInfo.size();
        *ppDeviceMaps = &mpDeviceRegistry->mCurrentDeviceOutputInfo[0];
        return kOK;
    }
    return kNotFound;
}


//////////////////////////////////////////////////////////////////////////
DMF::DeviceHandler::~DeviceHandler()
{
	Shutdown();
}

//////////////////////////////////////////////////////////////////////////
DMF::ReturnCode DMF::DeviceHandler::Shutdown()
{
	if(!mpDeviceRegistry)
	{
		return kNotFound;
	}
	for(u32 i = 0; i < mpDeviceRegistry->mNumDeviceHandlers; ++i)
	{
		if(mpDeviceRegistry->mDeviceHandlers[i])
		{
			delete mpDeviceRegistry->mDeviceHandlers[i];
			mpDeviceRegistry->mDeviceHandlers[i] = NULL;
		}
	}
	mpDeviceRegistry->mNumDeviceHandlers = 0;
	DeviceRegistry::DeviceIterator iter = mpDeviceRegistry->mPlugins.begin();

	while(iter!= mpDeviceRegistry->mPlugins.end())
	{
		u32 id = iter->first;
		UnLoadDLL(iter->second.mpHandle);
		mpDeviceRegistry->mPlugins.erase(id);
		iter = mpDeviceRegistry->mPlugins.begin();
	}
	return kOK;
}

//////////////////////////////////////////////////////////////////////////
DMF::ReturnCode DMF::DeviceHandler::Update()
{
	assert(mpDeviceRegistry);
	if(!mpDeviceRegistry)
	{
		return kFail;
	}
	for(u32 i = 0; i < mpDeviceRegistry->mNumDeviceHandlers; ++i)
	{
		if(mpDeviceRegistry->mDeviceHandlers[i])
		{
			mpDeviceRegistry->mDeviceHandlers[i]->Update();
		}
	}


	return kOK;
}

//////////////////////////////////////////////////////////////////////////
DMF::ReturnCode DMF::DeviceHandler::Update(u32 deviceID, DataBuffer* pDestination)
{
	assert(mpDeviceRegistry);
	if(!mpDeviceRegistry)
	{
		return kFail;
	}
	for(u32 i = 0; i < mpDeviceRegistry->mNumDeviceHandlers; ++i)
	{
		if(mpDeviceRegistry->mDeviceHandlers[i])
		{
			if(kOK == mpDeviceRegistry->mDeviceHandlers[i]->Update(deviceID, pDestination))
			{
				return kOK;
			}
		}
	}
	return kNotFound;
}

//////////////////////////////////////////////////////////////////////////
DMF::ReturnCode DMF::DeviceHandler::UpdateOutput(u32 deviceID, DataBuffer* pSrc)
{
    assert(mpDeviceRegistry);
    if(!mpDeviceRegistry)
    {
        return kFail;
    }
    for(u32 i = 0; i < mpDeviceRegistry->mNumDeviceHandlers; ++i)
    {
        if(mpDeviceRegistry->mDeviceHandlers[i])
        {
            if(kOK == mpDeviceRegistry->mDeviceHandlers[i]->UpdateOutput(deviceID, pSrc))
            {
                return kOK;
            }
        }
    }
    return kNotFound;
}

//////////////////////////////////////////////////////////////////////////
DMF::ReturnCode DMF::DeviceHandler::Update(u32 deviceID, CustomDataBuffer* pDestination)
{	
	assert(mpDeviceRegistry);
	if(!mpDeviceRegistry)
	{
		return kFail;
	}
	for(u32 i = 0; i < mpDeviceRegistry->mNumDeviceHandlers; ++i)
	{
		if(mpDeviceRegistry->mDeviceHandlers[i])
		{
			if(kOK == mpDeviceRegistry->mDeviceHandlers[i]->Update(deviceID, pDestination))
			{
				return kOK;
			}
		}
	}
	return kNotFound;
}

//////////////////////////////////////////////////////////////////////////
DMF::ReturnCode DMF::DeviceHandler::UpdateOutput(u32 deviceID, CustomDataBuffer* pSrc)
{
    assert(mpDeviceRegistry);
    if(!mpDeviceRegistry)
    {
        return kFail;
    }
    for(u32 i = 0; i < mpDeviceRegistry->mNumDeviceHandlers; ++i)
    {
        if(mpDeviceRegistry->mDeviceHandlers[i])
        {
            if(kOK == mpDeviceRegistry->mDeviceHandlers[i]->UpdateOutput(deviceID, pSrc))
            {
                return kOK;
            }
        }
    }
    return kNotFound;
}

//////////////////////////////////////////////////////////////////////////
DMF::ReturnCode DMF::DeviceHandler::Update(u32 deviceID, DeviceDataMap* pDestination)
{
	assert(mpDeviceRegistry);
	if(!mpDeviceRegistry)
	{
		return kFail;
	}
	for(u32 i = 0; i < mpDeviceRegistry->mNumDeviceHandlers; ++i)
	{
		if(mpDeviceRegistry->mDeviceHandlers[i])
		{
			if(kOK == mpDeviceRegistry->mDeviceHandlers[i]->Update(deviceID, pDestination))
			{
				return kOK;
			}
		}
	}
	return kNotFound;
}

DMF::ReturnCode DMF::DeviceHandler::UpdateOutput(u32 deviceID, DeviceDataMap * pSrc){
    
    assert(mpDeviceRegistry);
    if(!mpDeviceRegistry)
    {
        return kFail;
    }
    for(u32 i = 0; i < mpDeviceRegistry->mNumDeviceHandlers; ++i)
    {
        if(mpDeviceRegistry->mDeviceHandlers[i])
        {
            if(kOK == mpDeviceRegistry->mDeviceHandlers[i]->UpdateOutput(deviceID, pSrc))
            {
                return kOK;
            }
        }
    }
    return kNotFound;

}

//////////////////////////////////////////////////////////////////////////
DMF::ReturnCode DMF::DeviceHandler::Start(u32 deviceID)
{
	return kOK;
}

//////////////////////////////////////////////////////////////////////////
DMF::ReturnCode DMF::DeviceHandler::Stop(u32 deviceID)
{
	return kOK;
}


