#pragma once
//
// ExecuteListInThread: generic base class for executing a list
//		in a background thread.
//		Each entry in execute list is described by an ENTRY and
//			its data structure is ENTRY_INTERNAL.
//		WARNING: Client must call Init() and Start() to enable this class.
//
#include <vector>

#include "common/b2bassert.h"
#include "common/B2BTime.h"
#include "log/B2BLog.h"

#include "ExecuteListType.h"

#include "ThreadModule.h"

template <typename ENTRY, typename ENTRY_INTERNAL>
class ExecuteListInThread : public ThreadModule {
  public:
	// name: passed to ThreadModule
    ExecuteListInThread(const char *name);
    virtual ~ExecuteListInThread();

	// ExecuteList(execList, ...): Execute items from a list.  
	// 		execList: list of entries to execute
	//		Executed in order in execList.
	//		If not looping, will fail and return false if this instance is
	//			already in process of executing a list.
	//		If looping, will cancel the current looping list and execute
	//			this ExecuteList() call.  Current loop list's pDoneCallback
	//			*will be* called with reason NEWSTART.
	// Execute(entry, ...): Execute one item.   Uses ExecuteList()
	// 		entry: the one entry to execute
	//		This is equivalent to calling ExecuteList() with a list of length 1
	//
	// label: label for using when logging (for example, caller's method name)
	// buildExecListInternal: Create an internal list given the execList to
	//		execute.  Also handle any other needed logic.
	// 		This RETURNS: true on success, false otherwise
	//		PAY ATTENTION: This routine is called with mutex locked.
	//			Therefore this routine must be fast and simple.
	// loopExecList: If true, loop executing execList until StopExecuting()
	//		 called.
	//		 If false, execute execList once only.
	// pDoneCallback: Contains method to call to report we are done.
	//		See DoneReason for all the reasons we can call this callback.
	//		Called once per ExecuteList()/ExecuteLists() call.
	//		Caller should set to NULL if no reporting is desired.
	//		If loopExecList is true, only called once when StopExecuting() is
	//			called and StopExecuting gives option of calling it or not.
	//
	// RETURNS: true on success, false otherwise
	typedef std::vector<ENTRY> ExecList;
	typedef std::vector<ENTRY_INTERNAL> ExecListInternal;
	typedef boost::function<bool (const ExecList &execList, ExecListInternal *pNewInternalList)> BuildExecListInternal;

	bool ExecuteList(const char *label, const ExecList &execList,  
					BuildExecListInternal &buildExecListInternal,
					bool loopExecList=false,
					const ExecuteListType::DoneCallback *pDoneCallback=NULL);
				
	bool Execute(const char *label, ENTRY entry, 
					BuildExecListInternal &buildExecListInternal,
					bool loopExecList=false,
					const ExecuteListType::DoneCallback *pDoneCallback=NULL)
	{ 
		ExecList execList;
		execList.push_back(entry);
	  	return ExecuteList(label, execList, buildExecListInternal, 
							loopExecList, pDoneCallback);
	}

	// Stops execution of current list
	// This DOES NOT stop executing the entry in progress.  
	// If executing a list, the remainder of the list will be cancelled.
	//		IMPROVE: do stop current entry by killing and restarting thread?
	//					But if we do this, we have to check everybody
	//					that calls StopExecuting and make sure that would
	//					work for them.   Better to add this kill as an option
	//					so that we can retain previous behavior.
	//
	// reason: passed to ExecState::pDoneCallback(reason)
	// executeCallback: If true, StopExecuting method will directly call
	//		ExecState::pDoneCallback(reason).
	//		If false, StopExecuting method will not directly call it.
	//		executeCallback is only used if ExecState::pDoneCallback != NULL
	//		WARNING: Be very careful when using true: you do not want
	//			to call callback twice (once in StopExecuting and once
	//			in WorkerExecute())
	// logInfo: If true, B2BLog::Info that this method is called with
	//			reason information.  If false, use B2BLog:Debug.
	//
	// RETURNS: true on success, false otherwise
	// CORNER CASE: If list is NOT executing (exectListInProg is already NULL),
	//		this method is a NOP and it does not call the callback.
	//		In this case, this method will return true.
	bool StopExecuting(ExecuteListType::DoneReason reason,
						bool executeCallback=false,
						bool logInfo=false);

	// Returns true if currently executing a list, false otherwise
	bool IsExecuting() { return m_execState.execListInProg; }

  protected:
	// used to protect m_execState from async callers to Execute*()
	pthread_mutex_t m_execListLock; 

	// START: required protected virtual from ThreadModule
	// Executes list if execListInProg is true and execList is not empty
	void *Worker(void *arg);
	// END: required protected virtual from ThreadModule

	// Called to execute logic when list is executing
	// RETURNS: true if Worker should keep executing the list, false if Worker
	//			should stop the list
	virtual bool WorkerExecute(const ENTRY_INTERNAL &entry) = 0;

	// Called at end of each loop in Worker.  Client must do something
	// to yield control to other threads (for example, sleep will do it).
	virtual void WorkerYield() = 0;

	// execListInProg: if true, we have an execList loaded to execute
	//				or the execList is currently executing.
	//			 if false, no execList is active
	// execList: the execution list
	// loopExecList: if true, we want to loop execution of the execList
	// 				 if false, we execute the execList once
    class ExecState {
	  public:
        ExecState() : 
			execListInProg(false), loopExecList(false), pDoneCallback(NULL) {}

		// Reset to empty and nothing running.
        void Reset()
		{
			loopExecList = false;
			execListInProg = false;
			execList.clear();
			pDoneCallback = NULL;
		}

		bool execListInProg;
		ExecListInternal execList;

		bool loopExecList;
		const ExecuteListType::DoneCallback	*pDoneCallback;
	};
	ExecState m_execState;	// Our execution state

	// tests need access to privates
	friend class CreatureMenu;
};

template <typename ENTRY, typename ENTRY_INTERNAL>
ExecuteListInThread<ENTRY, ENTRY_INTERNAL>::ExecuteListInThread(
														const char *name) :
  ThreadModule(name)
  //m_execState() // init'ed by its own ctor
{
	pthread_mutexattr_t mutexattr;
	pthread_mutexattr_init(&mutexattr);
	pthread_mutexattr_settype(&mutexattr, PTHREAD_MUTEX_RECURSIVE);
	pthread_mutex_init(&m_execListLock, &mutexattr);
}

template <typename ENTRY, typename ENTRY_INTERNAL>
ExecuteListInThread<ENTRY, ENTRY_INTERNAL>::~ExecuteListInThread()
{
	Stop(NULL); // Stop our thread
	m_execState.Reset(); // Reset to empty and nothing running
	pthread_mutex_destroy(&m_execListLock);
}

template <typename ENTRY, typename ENTRY_INTERNAL>
bool ExecuteListInThread<ENTRY, ENTRY_INTERNAL>::ExecuteList(const char *label,
					const ExecList &execList,
					BuildExecListInternal &buildExecListInternal,
					bool loopExecList,
					const ExecuteListType::DoneCallback *pDoneCallback)
{
	pthread_mutex_lock(&m_execListLock);
	const ExecuteListType::DoneCallback *pExecStateDoneCallback = NULL;
	if (m_execState.execListInProg) {
		if (m_execState.loopExecList) {
			// Stop previous loop and continue
			pExecStateDoneCallback = m_execState.pDoneCallback; // save it for later
			// Do not call callback in here (2nd param is false).  Must be
			// called with no mutex lock held to avoid deadlock.
			StopExecuting(ExecuteListType::DONEREASON_NEWSTART, false, false);
		} else {
			// We are already executing a list and we are not looping.
			// This is not allowed.
			// Do not start a new execution.
			pthread_mutex_unlock(&m_execListLock);
			B2BLog::Info(LogFilt::LM_APP, "%s(%d) ignored because it is already busy executing a previous list.", 
				label, (int)execList.size());
			return false;  // FAIL
		}
	} // else: not currently executing a list

	b2bassert(m_execState.execList.empty());
	m_execState.Reset(); // Reset to empty and nothing running
	m_execState.loopExecList = loopExecList;
	m_execState.pDoneCallback = pDoneCallback;

	if (!(buildExecListInternal)(execList, &m_execState.execList)) {
		m_execState.Reset(); // Reset to empty and nothing running
		pthread_mutex_unlock(&m_execListLock);
		return false; // FAIL
	}

	// enable after building ExecState::execList
	m_execState.execListInProg = true; 

	pthread_mutex_unlock(&m_execListLock);

	if (pExecStateDoneCallback) {
		// Call cancelled looping list's callback.
		// STOPPED: We don't call inside mutex lock to avoid deadlock.
		// And don't access ExecState::pDoneCallback directly to avoid
		// race conditions where someone may have called Start between
		// this if-check and this line here.
		// And BTW, Reset() sets ExecState::pDoneCallback to NULL
		(*pExecStateDoneCallback)(ExecuteListType::DONEREASON_NEWSTART);
	} // else: there was no cancelled looping list, or it had no callback

	return true; // SUCCESS
}

template <typename ENTRY, typename ENTRY_INTERNAL>
bool ExecuteListInThread<ENTRY, ENTRY_INTERNAL>::StopExecuting(
								ExecuteListType::DoneReason reason, 
								bool executeCallback, bool logInfo)
{
	pthread_mutex_lock(&m_execListLock);
	if (!m_execState.execListInProg) {
		// We're not executing, exit immediately
		pthread_mutex_unlock(&m_execListLock);
		return true;
	}
	const ExecuteListType::DoneCallback *pDoneCallback = m_execState.pDoneCallback;
	size_t size = m_execState.execList.size();
	m_execState.Reset(); // Reset to empty and nothing running
	pthread_mutex_unlock(&m_execListLock); 
	// Create informative reason information
	std::string reasonString = ExecuteListType::DoneReasonToString(reason);
	if (reason == ExecuteListType::DONEREASON_NEWSTART) {
		// Stopped because of a NEWSTART
		reasonString = std::string("STOP(") + ExecuteListType::DoneReasonToString(reason) + ")";
	}
	char buf[200+1];
	snprintf(buf, 200, "%s::StopExecuting(%s): %s at %lu (execList.size=%u)", 
				Name(), 
				ExecuteListType::DoneReasonToString(reason),
				reasonString.c_str(),
				(unsigned long)B2BTime::GetCurrentB2BTimestamp(),
				(unsigned)size);
	if (logInfo)
		B2BLog::Info(LogFilt::LM_APP, "%s", buf);
	else
		B2BLog::Debug(LogFilt::LM_APP, "%s", buf);

	if (executeCallback && pDoneCallback) {
		// STOPPED: We don't call inside mutex lock to avoid deadlock.
		// And don't access ExecState::pDoneCallback directly to avoid
		// race conditions where someone may have called Start between
		// this if-check and this line here.
		// And BTW, Reset() sets ExecState::pDoneCallback to NULL
		(*pDoneCallback)(reason);
	}

	return true;
}

template <typename ENTRY, typename ENTRY_INTERNAL>
void *ExecuteListInThread<ENTRY, ENTRY_INTERNAL>::Worker(void *arg)
{
	  // Make a copy to handle race conditions.  
	  // We only use m_execState to look at execListInProg
	  pthread_mutex_lock(&m_execListLock);
      ExecState localExecState = m_execState;

	  bool execCallback = false; // Assume we do not call callback
	  // Assume we complete running the list normally
	  ExecuteListType::DoneReason reason = ExecuteListType::DONEREASON_COMPLETE;

	  if (m_execState.execListInProg && !localExecState.execList.empty()) {

		for (std::vector<uint8_t *>::size_type i = 0;
			 i < localExecState.execList.size(); ++i)
		{
	  	  pthread_mutex_unlock(&m_execListLock);
		  bool result = WorkerExecute(localExecState.execList[i]);
	  	  pthread_mutex_lock(&m_execListLock);

		  if (!result) {
			// WorkerExecute returns false if we should stop executing
			// Make sure this is false so we don't keep looping!
	  	  	m_execState.execListInProg = false;
	  	  	localExecState.loopExecList = false;
			reason = ExecuteListType::DONEREASON_STOP;
			break;
		  }

	  	  if (!m_execState.execListInProg) {
			// We've been told to stop
			// Make sure this is false so we don't keep looping!
	  	  	localExecState.loopExecList = false; 
			reason = ExecuteListType::DONEREASON_STOP;
		  	break;
		  }
		}

		if (!localExecState.loopExecList) {
			// FINISHED: Done playing list, delete and re-init state
			m_execState.Reset();
			execCallback = true;

		} // Skip the "FINISHED" logic, looping and executing the list again
	  }

	  pthread_mutex_unlock(&m_execListLock);

	  if (execCallback && localExecState.pDoneCallback) {
				(*localExecState.pDoneCallback)(reason);
	  }


	WorkerYield(); // give up control and let other threads run

	return NULL;
}
