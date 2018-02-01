#pragma once
//
// NAO behavior hardware driver
//
#include <stdint.h>

#include <boost/shared_ptr.hpp>

namespace AL
{
  class ALBroker;
}

namespace NaoBehaviorHW {
	// Run behavior named behaviorName
	// force: if true, we always try to play the behavior, even if 
	//			others are running
	// If behavior is already running, this function attempts to stop it,
	// then waits 1sec and tries to run behavior.
	// RETURNS: true on success, false otherwise
	bool RunBehavior(const char *behaviorName, bool force,
  								 boost::shared_ptr<AL::ALBroker> broker);

	// Stop all behaviors.
	// RETURNS: true on success, false otherwise
	bool StopAllBehaviors(boost::shared_ptr<AL::ALBroker> broker);
}
