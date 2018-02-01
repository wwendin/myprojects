#include "B2BLogic.h"

inline const char *B2BLogic::Bool3ToString(Bool3 val)
{
	switch (val) {
	  case BOOL3_FALSE:
	  	return "FALSE";
	  case BOOL3_TRUE:
	  	return "TRUE";
	  case BOOL3_UNKNOWN:
	  	return "UNK";
	  default:
	  	return "INVALID";
	}
}

const char *B2BLogic::LevelToString(Level val)
{
	switch (val) {
	  case LEVEL_NONE:
	  	return "NONE";
	  case LEVEL_LOW:
	  	return "LOW";
	  case LEVEL_MEDIUM:
	  	return "MEDIUM";
	  case LEVEL_HIGH:
	  	return "HIGH";
	  default:
	  	return "INVALID";
	}
}

B2BLogic::Level B2BLogic::FindLevelFromThresholds(float value,
												  float thresholds[])
{
	if (value <= thresholds[B2BLogic::LEVEL_LOW]) {
		// no threshold reached by value.   I have this if here for the
		// special corner case where all thresholds are 0 and value <= 0
		return B2BLogic::LEVEL_TOTAL;
	}

	Level levelFound = B2BLogic::LEVEL_TOTAL; // this is the result
  	int levelCount;
  	for (levelCount = (B2BLogic::LEVEL_TOTAL-1);
		 levelCount > B2BLogic::LEVEL_NONE;
		 --levelCount)
	{
		if (value > thresholds[levelCount]) {
			// SUCCESS: found level matching value
			levelFound = static_cast<B2BLogic::Level>(levelCount);
			break;
		} // else: value > thresholds[levelCount]
	}

  	if (levelCount == B2BLogic::LEVEL_NONE) {
		// Did not find anything above.  No threshold reached by value
		levelFound = B2BLogic::LEVEL_TOTAL;
	} // else: value did reach threshold and levelFound is good

	return levelFound;
}
