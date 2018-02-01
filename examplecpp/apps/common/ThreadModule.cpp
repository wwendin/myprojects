#include <apps/common/ThreadModule.h>
#include <unistd.h>
#include <sys/syscall.h>

#include "common/b2bthread.h"
#include "common/b2bassert.h"
#include "log/B2BLog.h"


ThreadModule::ThreadModule(const char *name) :
	B2BModule(name),
	m_pthreadCancelRequest(false)
{
	m_threadState.InitState(); // make sure
}

ThreadModule::~ThreadModule()
{
	if (m_threadState.m_running) {
      B2BLog::Err(LogFilt::LM_APP, 
	  		"ThreadModule derived class %s must call Stop before ThreadModule destructor",
			Name());
	}
}

bool ThreadModule::Start(void *arg)
{
	b2bassert(!m_threadState.m_running);

	m_threadState.InitState(); // make sure

	m_threadState.m_arg = arg;

    // Create our thread
    int result = pthread_create(&m_threadState.m_thread, NULL, WorkerInternal,
								this);
    if (result) {
      B2BLog::Err(LogFilt::LM_APP, 
	  		"ThreadModule::Start(%s) FAIL. pthread_create return code: %d",
			Name(), result);
	  m_threadState.m_created = false; // make sure
	  return false;
    }

	m_threadState.m_created = true;

	return true;
}

bool ThreadModule::Stop(void **returnVal)
{
	if (m_threadState.m_created) {
		SendThreadCancelRequest();
		// Wait for thread to quit.  We wait because I want all threads
		// to end cleanly.  Specifically, I want derived classes to exit
		// their Worker functions when their Stop is called.
		// FIXME: Add a timeout in case thread never stops, so we are 
		//  		guaranteed to exit in real product.
		int result = pthread_join(m_threadState.m_thread, returnVal);
		m_pthreadCancelRequest = false;
		m_threadState.InitState(); // re-init state
		if (result) {
      		B2BLog::Err(LogFilt::LM_APP, 
				"ThreadModule::Stop(%s) FAIL.  pthread_join return code: %d",
				Name(), result);
			return false;
		} else {
      		B2BLog::Debug(LogFilt::LM_APP, "ThreadModule::Stop(%s) SUCCESS",
										  Name());
		}
	} else {
		return false;
	}
	return true;
}

bool ThreadModule::WaitForThreadToStart(pid_t *pTID, 
										useconds_t sleepPeriodUS) const
{
	while (m_threadState.m_tid == 0)
		usleep(sleepPeriodUS);

	*pTID = m_threadState.m_tid;
	return true;
}

/*static*/ void *ThreadModule::WorkerInternal(void *arg)
{
	b2bassert(arg);

	ThreadModule *tm = (ThreadModule *)arg;
	tm->m_threadState.m_running = true;
	tm->m_threadState.m_tid = gettid();
    B2BLog::Debug(LogFilt::LM_OS, "ThreadModule %s tid: %d",
									tm->Name(), (int)tm->m_threadState.m_tid);

	void *returnVal = NULL;
	while (!tm->IsThreadCancelRequested()) {
		returnVal = tm->Worker(tm->m_threadState.m_arg);
	}

	tm->m_threadState.m_running = false;

	return returnVal;
}
