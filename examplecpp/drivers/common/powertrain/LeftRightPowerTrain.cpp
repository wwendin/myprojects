#include "common/b2bassert.h"

#include "LeftRightPowerTrain.h"

LeftRightPowerTrain::LeftRightPowerTrain(Motor *left, Motor *right) :
	PowerTrain("LeftRightPowerTrain"),
	m_left(left),
	m_right(right)
{
	b2bassert(m_left);
	b2bassert(m_right);
}

LeftRightPowerTrain::~LeftRightPowerTrain()
{
	m_left->StopMotor();
	m_right->StopMotor();
}

bool LeftRightPowerTrain::DriveVector(b2b::Heading headingChange,
									  Motor::MotorSpeed speed)
{
	Motor::MotorDir leftDir, rightDir;
	Motor::MotorSpeed leftSpeed, rightSpeed;

	// FIXME: calculate all 4 values from headingChange and speed
	leftDir = Motor::MOTOR_DIR_FWD;
	rightDir = Motor::MOTOR_DIR_FWD;
	leftSpeed = 0;
	rightSpeed = 0;

	if (!m_left->SetSpeed(leftDir, leftSpeed)) 
		return false; // FAIL
	if (!m_right->SetSpeed(rightDir, rightSpeed))
		return false; // FAIL

	return true; // SUCCESS
}

bool LeftRightPowerTrain::StopMotor()
{
	// Always try to stop both, even if one fails

	bool result = m_left->StopMotor();
	if (!m_right->StopMotor())
		result = false;	// FAIL

	return result;
}
