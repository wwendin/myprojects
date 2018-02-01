#pragma once
//
// Abstract class that drives the engine and wheels of a vehicle
//

#include "common/b2btypes.h"
#include "apps/common/B2BModule.h"

#include "drivers/common/powertrain/Motor.h"

class PowerTrain : public B2BModule {
  public:
	PowerTrain(const char *name) : B2BModule(name) {}
	virtual ~PowerTrain() {}

	// headingChange: degrees relative to our nose which will drive.
	// speed: speed to drive the vehicle at new heading.  Value is 0 to 1.0
	//			where 1.0 is maximum speed.
	// RETURNS: true on success, false otherwise
	virtual bool DriveVector(b2b::Heading headingChange, 
							 Motor::MotorSpeed speed) = 0;

	// Stop the vehicle ASAP
	virtual bool StopMotor() = 0;

	// START: required virtual from B2BModule.  See that class for doc
	bool Init() { return true; }
	bool Start(void *arg) { return true; }
	bool Stop(void **returnVal) { return StopMotor(); }
	// END: required virtual from B2BModule.  See that class for doc
};
