#pragma once
//
// B2BTime: namespace that contains all the clock and timestamp logic
//

#include <time.h>
#include "b2btypes.h"
#include "B2BMath.h"

namespace B2BTime {

	// Decided to use timespec here because that is what clock_gettime uses.
	// Clients of this class should not know the difference as they
	// use methods to operate on TimeValue
  	class TimeValue {
	  public:
  		TimeValue()
		{
		  	m_time.tv_sec = 0;
		  	m_time.tv_nsec = 0;
		}
  		TimeValue(const TimeValue &t)
		{
		  	m_time = t.m_time;
		  	ResolveSecWithNanoSec();
		}
  		TimeValue(struct timespec &t)
		{
		  	m_time = t;
		  	ResolveSecWithNanoSec();
		}

  		void TimeValueSetMS(b2b::TimeMS ms)
		{
			m_time.tv_sec = ms/1000;
    		m_time.tv_nsec = (ms%1000)*1000;
		  	ResolveSecWithNanoSec();
		}

		TimeValue &operator=(const TimeValue &t) 
		{
			m_time = t.m_time;
			return *this;
		}

    	bool operator==(const TimeValue &t) const
		{
		  	return (m_time.tv_sec == t.m_time.tv_sec) &&
		  		   (m_time.tv_nsec == t.m_time.tv_nsec);
		}
    	bool operator!=(const TimeValue &t) const { return !(*this == t); }
    	bool operator<(const TimeValue &t) const
		{
   			if (m_time.tv_sec < t.m_time.tv_sec)
        		return true;
    		if ((m_time.tv_sec == t.m_time.tv_sec) &&
				(m_time.tv_nsec < t.m_time.tv_nsec))
			{
        		return true;
			}
    		return false;
		}
			
    	bool operator>(const TimeValue &t) const
		{
   			if (m_time.tv_sec > t.m_time.tv_sec)
        		return true;
    		if ((m_time.tv_sec == t.m_time.tv_sec) &&
				(m_time.tv_nsec > t.m_time.tv_nsec))
			{
        		return true;
			}
    		return false;
		}
    	bool operator<=(const TimeValue &t) const { return !(*this > t); }
    	bool operator>=(const TimeValue &t) const { return !(*this < t); }

		TimeValue operator+(const TimeValue &rhs) const
		{
			TimeValue temp = *this;
		  	temp.m_time.tv_sec += rhs.m_time.tv_sec;
		  	temp.m_time.tv_nsec += rhs.m_time.tv_nsec;
		  	temp.ResolveSecWithNanoSec();
			return temp;
		}
		TimeValue &operator+=(const TimeValue &rhs)
		{
			*this = *this + rhs; // use operator+
			return *this;
		}
		TimeValue operator-(const TimeValue &rhs) const
		{
			TimeValue temp = *this;
		  	temp.m_time.tv_sec -= rhs.m_time.tv_sec;
		  	temp.m_time.tv_nsec -= rhs.m_time.tv_nsec;
		  	temp.ResolveSecWithNanoSec();
			return temp;
		}
		TimeValue &operator-=(const TimeValue &rhs)
		{
			*this = *this - rhs; // use operator-
			return *this;
		}

		// Returns seconds represented by TimeValue.  Rounds to nearest second.
		// If you don't want rounded value, use Sec().
		#define ONE_BILLION		1000000000
  		int32_t ConvertToSec() const 
		{ 
			return m_time.tv_sec 
				   + B2BMath::DivRoundClosestInt(m_time.tv_nsec, ONE_BILLION);
		}

		// Returns milliseconds represented by TimeValue.  Rounds to nearest
		// milliseconds.
  		int32_t ConvertToMSec() const 
		{ 
			return (m_time.tv_sec*1000)
				   + B2BMath::DivRoundClosestInt(m_time.tv_nsec, 1000000);
		}

		// Returns timeval represented by TimeValue.  Rounds to nearest
		// microseconds.   
		// timeval is useful for use with asctime for example.
  		struct timeval ConvertToTimeval() const 
		{ 
			struct timeval time;
			time.tv_sec = m_time.tv_sec;
			time.tv_usec = B2BMath::DivRoundClosestInt(m_time.tv_nsec, 1000);
			return time;
		}

		// Returns timespec represented by TimeValue.  Rounds to nearest
		// nanoseconds.   
  		struct timespec ConvertToTimespec() const { return m_time; }

		// Sometimes it is useful to define an "invalid" TimeValue
		// Notice that we don't call ResolveSecWithNanoSec here.
  		void SetInvalid() { m_time.tv_nsec = ONE_BILLION; }
  		bool IsInvalid() const { return m_time.tv_nsec == ONE_BILLION; }

	  private:

		// Always want tv_nsec from 0 and ONE_BILLION-1, inclusive.  Adjust
		// tv_sec and tv_nsec until that is true.
		// We do this to keep values easy to read and debug and easier
		// to do operators like < and > etc.
  		void ResolveSecWithNanoSec();

		struct timespec m_time;
	};

	// Returns the current time using a monotonic clock.
	// This is the time elapsed since some unspecificed starting point which
	// is defined as 0.
	//
	// WARNING: This is not the "wall /clock".  Use GetTimeOfDay() and related
	//				functions for "wall clock".
	TimeValue GetCurrTimeMonotonic();

	// Returns the current time as a b2b::Timestamp.   Uses CurrTimeMonotonic()
	b2b::Timestamp GetCurrentB2BTimestamp();

	// Function that works like gettimeofday but uses clock_gettime and returns
	// a TimeValue.   This is time elapsed since "the Epoch".  This if often
	// called "ANSI time".  See gettimeofday for definition.
	TimeValue GetTimeOfDay();

}
