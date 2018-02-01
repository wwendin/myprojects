#include <algorithm>

#include "boost/bind.hpp"

#include "log/B2BLog.h"
#include "common/b2bassert.h"

#include "B2BModuleManager.h"

B2BModuleManager::~B2BModuleManager()
{
	B2BModules::reverse_iterator iter;
	for (iter = m_modules.rbegin(); iter != m_modules.rend(); ++iter) {
		B2BModule *module = *iter;
		delete module;
	}
}

B2BModule *B2BModuleManager::AddModuleImpl(B2BModule *module)
{
	b2bassert(module);
	if (!module)
		return NULL; // FAIL

	// FIXME: DEBUG only
	if (GetModuleImpl(module->Name(), false)) {
		// add failed because module already exists.  It was not added
		B2BLog::Err(LogFilt::LM_APP,
			"AddModule %s already loaded in B2BModuleManager.  New %s ignored.",
			module->Name(), module->Name());
		return NULL; // FAIL
	}

	// Add it
	m_modules.push_back(module);

	return module; // SUCCESS
}

/*static*/ bool B2BModuleManager::ModuleCompare(const B2BModule *module, 
											 	const char *moduleName)
{
	return module->NameEqual(moduleName);
}

B2BModule *B2BModuleManager::GetModuleImpl(const char *moduleName,
											bool expected) const
{
	B2BModules::const_iterator iter = 
			std::find_if(m_modules.begin(), m_modules.end(), 
				  boost::bind(&B2BModuleManager::ModuleCompare,_1,moduleName));
	if (iter != m_modules.end()) {
		return *iter; // SUCCESS: Found the module, return it
	} else {
		if (expected) {
			// We expected to find the module
			B2BLog::Err(LogFilt::LM_APP, "GetModule %s not found.", moduleName);
		}
		return NULL; // FAIL: moduleName does not exist in B2BModuleManager
	}
}

bool B2BModuleManager::DeleteModule(const char *moduleName)
{
	B2BModule *module = GetModuleImpl(moduleName, true);
	if (module) {
	  delete module; // call destructor
	  // Remove module from B2BModuleManager
	  m_modules.remove(module);
	  return true;  // SUCCESS: Found the module and erase succeeded
	} else {
		B2BLog::Err(LogFilt::LM_APP, "GetModule %s not found.", moduleName);
		return false; // FAIL: moduleName does not exist in B2BModuleManager
	}
}

bool B2BModuleManager::InitAll()
{
	bool returnVal = true; // assume success
	B2BModules::iterator iter;
	for (iter = m_modules.begin(); iter != m_modules.end(); ++iter) {
		B2BModule *module = *iter;
		if (module->Init())
			returnVal = false; // FAIL: one failure means our method fails
	}

	return returnVal;
}

bool B2BModuleManager::StartAll()
{
	bool returnVal = true; // assume success
	B2BModules::iterator iter;
	for (iter = m_modules.begin(); iter != m_modules.end(); ++iter) {
		B2BModule *module = *iter;
		if (module->Start(NULL))
			returnVal = false; // FAIL: one failure means our method fails
	}

	return returnVal;
}

bool B2BModuleManager::StopAll()
{
	bool returnVal = true; // assume success
	B2BModules::iterator iter;
	for (iter = m_modules.begin(); iter != m_modules.end(); ++iter) {
		B2BModule *module = *iter;
		if (module->Stop(NULL))
			returnVal = false; // FAIL: one failure means our method fails
	}

	return returnVal;
}
