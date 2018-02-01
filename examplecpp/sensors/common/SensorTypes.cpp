//
#include "SensorEngine.h"

#include "SensorTypes.h"

SensorEngine *Sensor::GetFromSensorList(const char *name, 
										const Sensor::SensorList &list)
{
	SensorList::const_iterator iter;
	iter = list.find(name);
	if (iter != list.end()) {
  		// SUCCESS: found name
		return iter->second;
	} else {
		return NULL; // FAIL: DID NOT find name
	}
}
