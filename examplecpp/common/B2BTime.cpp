//
// See B2BTime.h for documentation
//
#include <time.h>

#include "B2BTime.h"

B2BTime::TimeValue B2BTime::GetCurrTimeMonotonic() 
{
	// Get high resolution timer.  MONOTONIC means it always moves up
	//   even if our internal clock is getting adjusted by NTP or similar.
	struct timespec timestamp;
	clock_gettime(CLOCK_MONOTONIC, &timestamp);

	// Convert to TimeValue
	TimeValue currentTime(timestamp);
	return currentTime;
}

b2b::Timestamp B2BTime::GetCurrentB2BTimestamp()
{
	return GetCurrTimeMonotonic().ConvertToMSec();
}

B2BTime::TimeValue B2BTime::GetTimeOfDay() 
{
	// Get high resolution timer.  REALTIME means it ANSI time.
	//   It is affected by adjustments from NTP or similar.
	struct timespec time;
	clock_gettime(CLOCK_REALTIME, &time);

	// Convert to TimeValue
	TimeValue currentTime(time);
	return currentTime;
}

/////// B2BTime::TimeValue

void B2BTime::TimeValue::ResolveSecWithNanoSec()
{
	if (m_time.tv_nsec >= ONE_BILLION) {
		do { 
			++m_time.tv_sec;
			m_time.tv_nsec -= ONE_BILLION;
		} while (m_time.tv_nsec >= ONE_BILLION);
	}
	if (m_time.tv_nsec <= -ONE_BILLION) {
		do { 
			--m_time.tv_sec;
			m_time.tv_nsec += ONE_BILLION;
		} while (m_time.tv_nsec <= -ONE_BILLION);
	}
}
