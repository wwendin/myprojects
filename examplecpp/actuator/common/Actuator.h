#pragma once
//
// Actuator is the base class for all actuators
//		NOTE: Currently this base class adds nothing!
//

// General purpose value for defining an actuator device as invalid
#define ACTUATOR_DEVICE_INVALID				-1

class Actuator {
  public:
	Actuator() {}
	virtual ~Actuator() {}

};
