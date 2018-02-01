#pragma once
//
// Description: A module manager for loading, running, looking up,
//				and automatically destructing all B2BModule in system.
//
#include <list>
#include <string>

#include "common/b2bassert.h"

#include "B2BModule.h"

class B2BModuleManager {
  public:
	B2BModuleManager() {}

	// This destructor will call the destructor of all modules added
	// by AddModule().
	// Modules are destroyed in *reverse* order that they were added.
	virtual ~B2BModuleManager();

	// Add a B2BModule to this manager.
	// RETURNS: module on success, NULL otherwise.
	//		Fails if module is NULL or module is already loaded
	//		in B2BModuleManager.
	template <typename T>
	T *AddModule(T *module)
		{ 
			b2bassert(module);
			if (!module)
				return NULL; // FAIL
			B2BModule *b2bModule = dynamic_cast<B2BModule *>(module);
			if (!b2bModule)
				return NULL; // FAIL: couldn't typecast
			module = dynamic_cast<T *>(AddModuleImpl(b2bModule));
			if (!module)
				return NULL; // FAIL: couldn't typecast
			return module;
		}

	// Returns B2BModule loaded by AddModule.
	// RETURNS: module instance on success, NULL otherwise.
	//
	// PAY ATTENTION: GetModule is not necessarily efficient.  If you need
	//		efficiency, then cache its return value in your own variable.
	template <typename T>
	T *GetModule(const char *moduleName) const
		{ 
			T *module = dynamic_cast<T *>(GetModuleImpl(moduleName, true));
			if (!module)
				return NULL; // FAIL: couldn't typecast
			return module;
		}

	// Delete a B2BModule from this manager.  
	// If module is found, its destructor is called.
	// RETURNS: true on success, false otherwise
	bool DeleteModule(const char *moduleName);

	// Initialize all the B2BModule
	// Calls the B2BModule::Init() of all modules added by AddModule()
	// Modules are called in order that they were added.
	// RETURNS: true if every module's Init returned true, false otherwise
	bool InitAll();

	// Start all the B2BModule
	// Calls the B2BModule::Start() of all modules added by AddModule()
	// Modules are called in order that they were added.
	// RETURNS: true if every module's Start returned true, false otherwise
	bool StartAll();

	// Stop all the B2BModule
	// Calls the B2BModule::Start() of all modules added by AddModule()
	// Modules are called in order that they were added.
	// RETURNS: true if every module's Stop returned true, false otherwise
	bool StopAll();

  private:
	// See template versions above for documentation
	B2BModule *AddModuleImpl(B2BModule *module);
	// expected: if true, it means we expect moduleName to be there
	//			 if false, we don't expect it to be there
	B2BModule *GetModuleImpl(const char *moduleName, bool expected) const;

	// Return true if module is labeled by moduleName
	static bool ModuleCompare(const B2BModule *module, const char *moduleName);

  	typedef std::list<B2BModule *> B2BModules;
  	B2BModules m_modules; // all the modules added by AddModule()
};
