#pragma once

/**
* DLL Interface classes for C++ Binary Compatibility article
* article at http://aegisknight.org/cppinterface.html
*
* code author:     Ben Scott   (bscott@iastate.edu)
* article author:  Chad Austin (aegis@aegisknight.org)
*/

#ifdef _WIN32
	  #ifdef DMF_EXPORT
		#define DMF_DLL_API __declspec(dllexport)
	  #else
		#define DMF_DLL_API __declspec(dllimport)
	  #endif
	  #define DMF_DLL_CALL __stdcall 
#else
    #define DMF_DLL_API
	#define DMF_DLL_CALL
#endif


// TODO - pass in resourcemanager and any other required class instances here
// define the interfaces for such
// possibly register the class with the resman? then can use that interface to delete it
// could do with a different register func - maybe register plugin?
// or is this just the job of a plugin manager?
// not really - plugin manager acts as the interface - still creates and deletes via the resman
// so maybe a global "C" register function that registers it with the resman
// and returns the hashed typeid for creation to the application
// application can store this in the plugin manager with 

// TODO - this should be the global DLL entry point
// so probably not a macro unless the plugin doesn't register any other classes
// otherwise we need to register create functions for each new class 
// defined in the plugin - if they derive from DynImplementation etc. then
// delete will be overridden properly - just the create func needs adding
// resource manager all ready has a register create func anyway so 
// this is a no brainer

namespace DMF
{
	struct DynamicLibraryAPIVersion
	{
		s32 mMajor = 0;
		s32 mMinor = 0;
	};

	class DynamicInterface;
	class PluginManager;
	class DeviceHandler;

	
	// General function pointer for plugin creation
	typedef DeviceHandler* (*CreateDeviceHandlerFunc)();

	// version info
	typedef void (*GetLibraryVersionFunc)(s32& major, s32& minor, const char** ppName);
	
	/**
	 * Interfaces exposed across a DLL boundary should derive from this
	 * class to ensure that their memory is released cleanly and
	 * safely. This class should be used in conjunction with the DLLImpl
	 * template. <b>Your interface should NOT define a destructor!</b>
	 *
	 * <h3>Example Usage</h3>
	 * \code
	 *    class MyInterface : public DLLInterface {
	 *       // MyInterface method declarations here...
	 *       // should NOT include a destructor!
	 *    };
	 * \endcode
	 *
	 * @see DLLImpl */
	class DynamicInterface 
	{
	public:	
		virtual ~DynamicInterface(){}
		
	public:
		/**
		* Specialized delete that calls destroy instead of the destructor.
		*/
		void operator delete(void* p) 
		{
			if (p) 
			{
			  DynamicInterface* pInterface = static_cast<DynamicInterface*>(p);
			  pInterface->Destroy();
			}
		}

	protected:


		/**
		* Handles the destruction of this class. This is essentially a destructor
		* that works across DLL boundaries.
		*/
		 virtual void DMF_DLL_CALL Destroy() = 0;
	};

		

	/**
	* Implementations of interfaces across DLL boundaries that derive
	* from DLLInterface should derive from this template to ensure that
	* their memory is released safely and cleanly.
	*
	* <h3>Example Usage</h3>
	* \code
	*    MyImpl : public DLLImpl<MyInterface> {
	*       // MyImpl method declarations, including constructors and
	*       // destructors here ...
	*    };
	* \endcode
	*
	* @see DLLInterface */
	template<class DMF_DYN_LIB_INTERFACE>
	class DynamicImplementation : public DMF_DYN_LIB_INTERFACE 
	{
	public:
		/**
		* This allows the implementation to simply define a destructor.  It
		* will automatically be called at the right time.
		*/
		virtual ~DynamicImplementation() 
		{
		}

		//virtual RetCode Shutdown() = 0;
		/**
		* Implementation of DLLInterface's destroy method. This function will
		* make sure this object gets deleted in a polymorphic manner. You
		* should NOT reimplement this method.
		*/
		virtual void DMF_DLL_CALL Destroy() 
		{
			delete this;
		}

		/**
		* Overloaded operator delete calls global operator delete since
		* DLLInterface modified this functionality to protect memory.
		*
		* @param p    pointer to the memory to delete
		*/
		void operator delete(void* p) 
		{
			::operator delete(p);
		}
	};
    
    //
    typedef DynamicImplementation<DynamicInterface> DynamicLibraryBase;
}



//
#define DMF_REG_DEVICE_HANDLER(NAME, MAJOR, MINOR)\
extern "C" DMF_DLL_API void DMF_DLL_CALL GetLibraryVersion(int& major, int& minor, const char** ppName)\
{\
	major	= MAJOR;\
	minor	= MINOR;\
	*ppName = #NAME;\
}

// So this is now CreateDynamicLibrary
#define DMF_DECL_CREATE_DEVICE_HANDLER(DMF_DYN_IMPL)\
extern "C" DMF_DLL_API DMF::DeviceHandler* DMF_DLL_CALL CreateDeviceHandler();

// So this is now CreateDynamicLibrary
#define DMF_DEF_CREATE_DEVICE_HANDLER(DMF_DYN_IMPL)\
extern "C" DMF_DLL_API DMF::DeviceHandler* DMF_DLL_CALL CreateDeviceHandler()\
{\
	return new DMF_DYN_IMPL;\
}
