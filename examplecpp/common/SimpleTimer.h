#pragma once
// 
// Description: timer that fires at defined interval and calls all the sensors
//                 to execute their system. 
//		WARNING: Client must call Init() and Start() to enable SimpleTimer.
// FIXME: Change the name of this to SystemClock because it drives more than
//			just the sensors
//
#include <stdint.h>

#include "boost/function.hpp"

#include "apps/common/TimerModule.h"

class SimpleTimer : public TimerModule {
  public:
	// Type for our SimpleTimer callback.
	// RETURNS: true on success, false otherwise.
	typedef boost::function<bool (void *arg)> SimpleTimerCallback;

	// name: used for debugging
	// intervalMS: See TimerModule for documentation 
	// firstExpirationIsImmediate: See TimerModule for documentation 
	// callback: function to call at timer interval defined by intervalMS
	// arg: argument to  to call at timer interval defined by intervalMS
	SimpleTimer(const char *name, 
				TimeIntervalMS intervalMS, bool firstExpirationIsImmediate,
				const SimpleTimerCallback &callback, void *arg=NULL);
	virtual ~SimpleTimer();

	// START: TimerModule required methods.  See that class for documentation
	bool Init() { return true; } // nothing to do yet

	// Called every intervalMS milliseconds
	void *TimerHandler(void *arg);
	// END: TimerModule required methods.

	// Easy-to-use version of TimerModule::Start() and TimerModule::Stop()
	bool Start() { return TimerModule::Start(NULL); }
	bool Stop() { return TimerModule::Stop(NULL); }

  private:
	const SimpleTimerCallback m_callback;	// from ctor
	void *m_callbackArg;					// from ctor arg
};
