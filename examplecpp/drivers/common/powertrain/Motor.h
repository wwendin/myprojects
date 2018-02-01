#pragma once
//
// Base class that drives one HW motor 
//
#include "common/b2btypes.h"
#include "apps/common/B2BModule.h"

class Motor : public B2BModule {
  public:
  	Motor(const char *name) : B2BModule(name) {}
  	virtual ~Motor() {}

	// Motor direction:
	// 		FWD: forward
	// 		REV: reverse
	// 		NEU: neutral (engines may not support this)
	typedef enum {
		MOTOR_DIR_FWD,
		MOTOR_DIR_REV,
		MOTOR_DIR_NEU,
		MOTOR_DIR_TOTAL		// size of enum (never used as a valid value)
	} MotorDir;
	typedef float MotorSpeed; // 0 to 1.0
	
	// dir: direction
	// speed: speed to drive the motor.  Value is 0 to 1.0
	//			where 1.0 is maximum speed.
	// RETURNS: true on success, false otherwise
	virtual bool SetSpeed(MotorDir dir, MotorSpeed speed) = 0;

	// Stop the motor ASAP
	virtual bool StopMotor() = 0;

	// START: required virtual from B2BModule.  See that class for doc
	bool Init() { return true; }
	bool Start(void *arg) { return true; }
	bool Stop(void **returnVal) { return StopMotor(); }
	// END: required virtual from B2BModule.  See that class for doc
};
