#pragma once
// 
// Description: sensor utilities
//
#include "common/B2BMath.h"

class NeuralData;

namespace SensorUtil {
	//Uses the sensor's data type to cast the data from a sensor to
	//	the appropriate type and then:
	//towardsStimulus: If true, returns vector.angle from sensor data.
	//			If false, returns InvertHeading(vector.angle) from sensor data
	//Returns 0 if neuralData is NULL
	b2b::Angle GetHeadingFrom(const char *sensorName,
								NeuralData *neuralData,
								bool towardsStimulus);

	// Helper routine to determine direction to move horizontally towards
	// stimulus, given a Vector3D of where stimulus came from.
	// SPECIAL CASE: If angle is 0 and azimuth is not 0, then:
	//		* if azimuth > 0 (bottom), return 180 (backward)
	//		* if azimuth < 0 (top), return 0 (forward)
	// REMINDER: negative azimuth is down, positive is up
	b2b::Heading HorizontalDirectionTowardStimulus(
												const B2BMath::Vector3D &v);
	// Same as HorizontalDirectionTowardStimulus but in opposite direction.
	b2b::Heading HorizontalDirectionAwayFromStimulus(
												const B2BMath::Vector3D &v);
}
