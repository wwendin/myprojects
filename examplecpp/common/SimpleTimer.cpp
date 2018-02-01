// 
// This the main clock in our entire system.  Every interval we poll all the
//		sensors and send all the values to waiting routines.
//

#include "SimpleTimer.h"

SimpleTimer::SimpleTimer(const char *name, 
					TimeIntervalMS intervalMS, bool firstExpirationIsImmediate,
					const SimpleTimerCallback &callback, void *arg) :
	TimerModule(name, intervalMS, firstExpirationIsImmediate),
	m_callback(callback),
	m_callbackArg(arg)
{
}

SimpleTimer::~SimpleTimer()
{
	Stop(); // Stop our timer
}

void *SimpleTimer::TimerHandler(void *arg)
{
	m_callback(m_callbackArg);
	return 0;
}
