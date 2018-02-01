#include "ExecuteListType.h"

const char *ExecuteListType::DoneReasonToString(DoneReason reason) 
{
	switch (reason) {
	  case DONEREASON_COMPLETE:
		return "COMPLETE";
	  case DONEREASON_STOP:
		return "STOP";
	  case DONEREASON_NEWSTART:
		return "NEWSTART";
	  default:
		return "BADVALUE";
	}
}
