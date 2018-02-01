#include <apps/common/TimerModule.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <string.h>
#include <signal.h>
#include <errno.h>

#include "common/b2bassert.h"
#include "common/b2bthread.h"
#include "log/B2BLog.h"


// Ugly, but ....
#define sigev_notify_thread_id _sigev_un._tid

TimerModule::TimerModule(const char *name, TimeIntervalMS intervalMS,
							bool firstExpirationIsImmediate) :
	ThreadModule(name),
	m_intervalMS(intervalMS),
	m_lastIntervalMS(0),
	m_firstExpirationIsImmediate(firstExpirationIsImmediate)
{
	if (intervalMS == 0) {
    	B2BLog::Err(LogFilt::LM_APP,
						"Timer intervalMS is 0.  Timer will be disarmed");
	}
	m_timerState.InitState(); // make sure
}

TimerModule::~TimerModule()
{
	Stop(NULL); // Stop our timer and thread
}

bool TimerModule::Start(void *arg)
{
	b2bassert(!m_timerState.m_running);

    // Create our thread
	if (!ThreadModule::Start(arg))
		return false; // FAIL


	return true;
}

bool TimerModule::Stop(void **returnVal)
{
	bool success = true; // Assume SUCCESS

	SendTimerCancelRequest();
	if (m_timerState.m_timerId) {
		// Change timer to expire immediately
		struct itimerspec tspec;
		tspec.it_interval.tv_sec = 0;
		tspec.it_interval.tv_nsec = 1;
		tspec.it_value.tv_sec = 0;
		tspec.it_value.tv_nsec = 1;
		int result = timer_settime(m_timerState.m_timerId, 0, &tspec, NULL);
		if (result) {
      		B2BLog::Err(LogFilt::LM_APP, 
	  					"TimerModule::Stop(%s) FAIL. timer_settime errno: %d",
						Name(), errno);
			success = false; // FAIL
			// keep going, don't exit
		}
	} else {
		success = false; // FAIL
	}

	if (!ThreadModule::Stop(returnVal))
		success = false; // FAIL
	// After returning from above, TimerModule::Worker is no longer called

	if (m_timerState.m_timerId) {
		int result = timer_delete(m_timerState.m_timerId);
		m_timerState.InitState(); // re-init state
		if (result) {
      		B2BLog::Err(LogFilt::LM_APP, 
	  					"TimerModule::Stop(%s) FAIL. timer_delete errno: %d",
						Name(), errno);
			// keep going, don't exit
			success = false; // FAIL
		}
	} else {
		success = false; // FAIL
	}

	if (success) {
        B2BLog::Debug(LogFilt::LM_APP, "TimerModule::Stop(%s) SUCCESS", Name());
	}

	return success;
}

void *TimerModule::Worker(void *arg)
{
	struct sigevent sigev;
	memset(&sigev, 0, sizeof(sigev));
	sigev.sigev_notify = SIGEV_THREAD_ID | SIGEV_SIGNAL;
	sigev.sigev_signo = SIGRTMIN+1;
	sigev.sigev_notify_thread_id = gettid();

	sigset_t sigset;
	sigemptyset(&sigset);
	sigaddset(&sigset, sigev.sigev_signo);
	sigprocmask(SIG_BLOCK, &sigset, NULL);

	// Create the time
	int result = timer_create(CLOCK_MONOTONIC, &sigev, &m_timerState.m_timerId);
    if (result) {
      B2BLog::Err(LogFilt::LM_APP, 
	  				"TimerModule::Start(%s) FAIL. timer_create errno: %d",
					Name(), errno);
	  m_timerState.m_running = false; // make sure
	  return NULL;
    }

	void *returnVal = NULL;
	while (!IsTimerCancelRequested()) {
		if (m_intervalMS != m_lastIntervalMS) {
			// Need to update timer with timeout interval m_intervalMS
			struct itimerspec tspec;
			// set interval
			time_t sec = m_intervalMS/1000;
			tspec.it_interval.tv_sec = sec;
			tspec.it_interval.tv_nsec = (m_intervalMS - (sec*1000)) * 1000000;
			if (m_firstExpirationIsImmediate) {
				// set initial expiration to "immediate"
				tspec.it_value.tv_sec = 0;
				tspec.it_value.tv_nsec = 1;
			} else {
				tspec.it_value = tspec.it_interval; // 1st expire at intervalMS
			}
			m_firstExpirationIsImmediate = false; // never immediate again

			// start the timer
			result = timer_settime(m_timerState.m_timerId, 0, &tspec, NULL);
			m_lastIntervalMS = m_intervalMS;
    		if (result) {
      			B2BLog::Err(LogFilt::LM_APP, 
	  					"TimerModule::Start(%s) FAIL. timer_settime errno: %d",
						Name(), errno);
	  			m_timerState.m_running = false; // make sure
	  			return NULL;
    		}
			m_timerState.m_running = true;
		}

		int sigs;
		if (sigwait(&sigset, &sigs) < 0)
			break;
		returnVal = TimerHandler(arg);
	}

	m_timerState.m_running = false;

	return returnVal;
}

bool TimerModule::ChangeTimeInterval(TimeIntervalMS intervalMS)
{
	if (intervalMS == 0) {
    	B2BLog::Err(LogFilt::LM_APP,
						"Timer intervalMS is 0.  Timer will be disarmed");
	}
	m_intervalMS = intervalMS;

	return true; // SUCCESS
}

