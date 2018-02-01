#pragma once
//
//  LED functions for host hardware
//
#include <string>

#include <boost/shared_ptr.hpp>
#include <alcommon/albroker.h>

#include "apps/common/B2BModule.h"

class LedOutputHW: public B2BModule {

public:
	LedOutputHW(boost::shared_ptr<AL::ALBroker> broker);
	virtual ~LedOutputHW() {}
	//
	// colorName: white, red, green, blue, yellow, magenta, cyan
	// RETURNS: true on success, false otherwise
	bool FadeRGB(const std::string& groupName,const std::string& colorName,
					  const float duration);

	// START: required virtual from B2BModule.  See that class for doc
	bool Init() { return true; }
	bool Start(void *arg) { return true; }
	bool Stop(void **returnVal) { return true; }
	// END: required virtual from B2BModule.  See that class for doc

private:
	boost::shared_ptr<AL::ALBroker> m_broker; // from ctor
};
