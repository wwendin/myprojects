#pragma once
//
// Description: base class for all of our modules that run one timer and
//		service that timer.
//
#include <apps/common/ThreadModule.h>
#include <stdint.h>

#include "common/b2btypes.h"


class TimerModule : public ThreadModule {
  public:
	// name: the name of the module
	// intervalMS: Time expires at this interval and calls Worker.
	//		For example, if 50, this is 50 milliseconds which is 20Hz.
	//		Setting value to 0 will disarm timer (probably not what you want).
	// firstExpirationIsImmediate: if true, timer expires immediately when
	//		first created then at intervalMS period after that.
	//		if false, first expiration is at intervalMS after timer is created
	//		in first call to Worker().
	typedef b2b::TimeMS TimeIntervalMS;
	TimerModule(const char *name, TimeIntervalMS intervalMS,
				bool firstExpirationIsImmediate=true);
	virtual ~TimerModule();

	// START: ThreadModule overrides
	// Start the timer.
	// arg: sent to ThreadModule::Worker
	// RETURNS: true on success, false otherwise
	bool Start(void *arg);

	// Stop the timer and cleans up.
	// Must be called before destroy.
	// The timer can be restarted and thus it is legal to call Start() after
	// calling Stop().
	// returnVal: value returned by TimerHandler()
	// RETURNS: true on success
	//			false if any of the following happen:
	//			* ThreadModule::Stop() returns false
	//			* call to delete timer fails
	bool Stop(void **returnVal);
	// END: ThreadModule overrides

	// Check if we have a cancel request (via Stop)
	// RETURNS: true on success, false otherwise.
	bool IsTimerCancelRequested() { return IsThreadCancelRequested(); }

	// Change our time interval.
	// PAY ATTENTION: Does not take effect until next timeout (specifically
	// 		until next call of TimerModule::Worker()
	//		If you call this feature in TimerHandler, it will take
	//		effect IMMEDIATELY (which is probably what you want).
	// Setting value to 0 will disarm timer (probably NOT what you want).
	// RETURNS: true on success, false otherwise.
	bool ChangeTimeInterval(TimeIntervalMS intervalMS);

  protected:
	// Called every intervalMS milliseconds.  Method must be created by
	//		all derived classes.
	// arg: value from Start()
	// RETURNS: value will be passed to Stop()
	virtual void *TimerHandler(void *arg) = 0;

  private:
    uint32_t m_intervalMS;			// set by ctor and ChangeTimerInterval
    uint32_t m_lastIntervalMS;		// last value passed to timer_settime
	bool m_firstExpirationIsImmediate;	// from ctor

    // START: ThreadModule required methods.  See that class for documentation
	// The routine that services the timer
	// arg: value from Start()
	// RETURNS: value to give to various pthread_xxx functions like
	//		pthread_create, etc
	//
	// Implementation detail: it calls TimerHandler()
	void *Worker(void *arg);
    // END: ThreadModule required methods.

	// Call to stop timer (don't call directly, use Stop)
	void SendTimerCancelRequest() { SendThreadCancelRequest(); }

	class TimerState {
	  public:
		TimerState()
		{
			InitState();
		}

		// Initialize member variables
		void InitState()
		  {
			m_timerId = 0;
			m_running = false;
		  }

		timer_t m_timerId;	// The timer
    	bool m_running;		// true if timer is valid and running
	};

	TimerState m_timerState;

    friend class B2BResources;
};
