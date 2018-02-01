#include "common/B2BMath.h"

#include "neural/NeuralData.h"
#include "sensors/common/SensorEngineSensorHW.h"

#include "SensorUtil.h"

b2b::Angle SensorUtil::GetHeadingFrom(const char *sensorName,
										NeuralData *neuralData,
											bool towardsStimulus)
{
	b2b::Angle headingChange = 0;
	if(neuralData) {
		const SensorEngineSensorHWData *dataP =
				dynamic_cast<const SensorEngineSensorHWData*>(neuralData);
		if (dataP) {
			headingChange = 
				HorizontalDirectionTowardStimulus(dataP->m_sensorHWData.vector);
			if (!towardsStimulus)
				headingChange = B2BMath::InvertHeading(headingChange);
		} else {
			// We are not a SensorEngineSensorHWData type of sensor
			//FIXME this is triggered by lightSensor and soundSensor(on ubuntu test.json)
			B2BLog::Err(LogFilt::LM_BEHAVIOR, "%s is not type SensorEngineSensorHWData", sensorName);
		}
	}
	return headingChange;
}

b2b::Heading SensorUtil::HorizontalDirectionTowardStimulus(
												const B2BMath::Vector3D &v)
{
	b2b::Heading headingChange = v.angle;  // horizonal direction of stimulus
	if (fabs(fabs(v.azimuth) - 90.0f) < 10) {
		// SPECIAL CASE: some sensors point directly up or down
		// I arbitrarily chose if stimulus comes from bottom, move backward
		// to go towards it.  To me, it made more sense than the opposite logic.
		if (v.azimuth > 0)
			headingChange = 180; // stimulus from bottom: move backward
		else
			headingChange = 0; // stimulus from top: move forward
	} // else: azimuth is not directly up or down, use horizontal angle

	return headingChange;
}

b2b::Heading SensorUtil::HorizontalDirectionAwayFromStimulus(
												const B2BMath::Vector3D &v)
{
	return B2BMath::InvertHeading(HorizontalDirectionTowardStimulus(v));
}
