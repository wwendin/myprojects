#pragma once
//
// ExecuteListType: types used by ExecuteListInThread
//
#include "boost/function.hpp"

namespace ExecuteListType {
	// Reason why any actuator request has been declared "DONE".
	// COMPLETE: We finished because we completed the request.
	// STOP: Done because of a Stop call
	// NEWSTART: Done because a new Start() was called.  Therefore this actuator
	//		request is terminated in order to start the new actuator request.
	typedef enum {
		DONEREASON_COMPLETE = 0,
		DONEREASON_STOP,
		DONEREASON_NEWSTART,
		DONEREASON_TOTAL	// size of enum (never used as a valid value)
	} DoneReason;

	// Type for our "Done" callbacks.
	// RETURNS: true on success, false otherwise.
	typedef boost::function<bool (DoneReason)> DoneCallback;

	const char *DoneReasonToString(DoneReason reason);
}
