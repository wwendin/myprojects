#pragma once
//
// PowerTrain that has two motors, one motor for left wheels (or tank treads),
// and one motor for and right wheels (or tank treads).
//

#include "drivers/common/powertrain/PowerTrain.h"

class LeftRightPowerTrain : public PowerTrain {
  public:
	// left: the left side's motor
	// right: the right side's motor
	LeftRightPowerTrain(Motor *left, Motor *right);

	~LeftRightPowerTrain();

	//// START: required PowerTrain virtuals.  See that class for documentation.
	bool DriveVector(b2b::Heading headingChange, Motor::MotorSpeed speed);
	bool StopMotor();
	//// END: required PowerTrain virtuals.

  private:
	Motor *m_left, *m_right;	// from ctor
};
