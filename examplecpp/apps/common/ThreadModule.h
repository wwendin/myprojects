#pragma once
//
// Description: base class for all of our modules that run one thread
//    Any derived class must call Start() to start thread.
//    Any derived class must call Stop() before calling ThreadModule destructor.
//    Any derived class must provide a Worker() that runs in the thread.
//
#include <unistd.h>
#include <pthread.h>

#include "apps/common/B2BModule.h"

class ThreadModule : public B2BModule {
  public:
	// name: the name of the module
	ThreadModule(const char *name);
	virtual ~ThreadModule();

	// START: B2BModule required virtuals.
	// Start the module.  This starts the thread.
	// arg: passed to pthread_create's arg
	// RETURNS: true on success, false otherwise
	virtual bool Start(void *arg);

	// Stop the thread and cleans up.  
	// Must be called before destroy.
	// The thread can be restarted and thus it is legal to call Start() after
	// calling Stop().
	// returnVal: value returned by pthread_join's retval (which is the value
	//		returned by Worker.
	//		See B2BModule for general features of this returnVal.
	// RETURNS: true on success
	//			false if any of the following are true:
	//			* thread doesn't exist
	//			* pthread_join fails
	virtual bool Stop(void **returnVal);
	// END: B2BModule required virtuals.

	// Check if we have a cancel request (via Stop)
	bool IsThreadCancelRequested() { return m_pthreadCancelRequest; }

  protected:
	// The routine runs in the thread
	// arg: value from Start()
	// RETURNS: value to give to various pthread_xxx functions like 
	//		pthread_join, etc
	// If Worker returns, then the thread exits.
	virtual void *Worker(void *arg) = 0;

	bool WaitForThreadToStart(pid_t *pTID, useconds_t sleepPeriodUS) const;

  protected:
	// Call to get Worker to quit (don't call directly, use Stop)
	void SendThreadCancelRequest() { m_pthreadCancelRequest = true; }

  private:

	// actual routine first called in thread, this calls Worker() in a loop
	static void *WorkerInternal(void *arg);

	class ThreadState {
	  public:
		ThreadState() :
			m_arg(NULL)
		{
			InitState();
		}

		// Initialize member variables
		void InitState()
		  {
			m_tid = 0;
			m_created = false;
			m_running = false;
		  }

    	void *m_arg; 			// the thread's arg parameter
    	pthread_t m_thread;		// the thread
    	pid_t m_tid;			// the thread's tid
    	bool m_created;			// true if thread was created
    	bool m_running;		// true if thread is a valid and running
	};

	ThreadState m_threadState;

    bool m_pthreadCancelRequest;   // set to true to request Worker to quit
};
