#pragma once
// 
// Description: Simple types used by all sensor classes.
//
#include <map>
#include <string.h>
#include <string>
#include "common/b2btypes.h"

class SensorEngine;

namespace Sensor {

	// Defines a list of SensorEngine
	typedef std::map<std::string, SensorEngine *> SensorList;

	// Return SensorEngine that matches name.  Returns NULL if not found.
	SensorEngine *GetFromSensorList(const char *name, const SensorList &list);

	// Used as p value from sensors
	typedef float PValue;

	class SensorData {
	  public:
		PValue p;	
		b2b::Timestamp timestamp; // timestamp of sensor data in milliseconds
	};

}
