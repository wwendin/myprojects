#pragma once
//
// Description: base class for all of our modules
//
#include <string.h>

class B2BModule {
  public:
	// name: the name of the module
	B2BModule(const char *name) : m_name(name) {}
	virtual ~B2BModule() {}

	// Initialize the module
	// RETURNS: true on success, false otherwise
	virtual bool Init() = 0;

	// Start the module.
	// arg: Definition depends on module type.
	// RETURNS: true on success, false otherwise
	virtual bool Start(void *arg) = 0;

	// Stop the module and cleans up.  
	// Must be called before destroy.
	// The module can be restarted and thus it is legal to call Start() after
	// calling Stop().
	// returnVal: Definition depends on module type.
	//		Nothing is stored in returnVal if this method returns false.
	//		Set returnVal to NULL if you don't want any value returned.
	// RETURNS: true on success, false otherwise
	virtual bool Stop(void **returnVal) = 0;

	const char *Name() const { return m_name; }
	bool NameEqual(const char *name) const
		{ return (strcmp(m_name, name) == 0); }

	// == means B2BModule has same name.  Override if you don't want that
    bool operator==(const B2BModule &m) const
	{
		  return (NameEqual(m.m_name));
	}
    bool operator!=(const B2BModule &m) const { return !(*this == m); }


  private:
	const char *m_name; // from ctor
};
