//
// AffectivaSensorHW: a SensorHW class listener for Affectiva SDK
// This is a very general purpose SensorHW class, that can listen to anything.
//
// You pass in the Affectiva camera detector you want to monitor (or any
// Affectiva detector of your choice).
// This class then looks for faces and reports Affectiva emotions, expressions,
// and appearances.   You enable them via our ioConfig (see below)
// PAY ATTENTION: Must call Init and Start to run this driver
// WARNING!!! This class only supports tracking one face (only looks at the data
//		of the first face in the list from Affectiva).
//
// Sets the following data if it detects the event:
//	   SensorHWData:
//		 vector is set to all 0s
//		 vector.magnitude set to the event's value (depends on event)
// 		 level: derived from Face.faceQuality.brightness.   
//					See AffectivaBrightnessToLevel below
//
#include <map>

#include "common/B2BLogic.h"
#include "apps/common/B2BModule.h"

#include "drivers/common/sensor/SensorHW.h"

class AffectivaSDKHW;

class AffectivaSensorHW : public B2BModule, public SensorHW
{
  public:
	// sdkHW: instance of Affective SDK HW driver
	// ioConfig: See SensorHW for documentation
	// ioConfigEntryName: name in ioConfig of sensor information
	//		"eventName" and *either* "threshold" or "compareValue" must
	//		exist in ioConfig.
	//		threshold is for events that have a float value and we only
	//		want to receive events when event value >= threshold.
	//		compareValue is for events that are enums and we only
	//		want to receive events when event value == compareValue.
  	AffectivaSensorHW(AffectivaSDKHW *sdkHW,
								  const IOConfig &ioConfig, 
								  const char *ioConfigEntryName);

	// START: required virtual from B2BModule.  See that class for doc
	bool Init() { return true; }
	bool Start(void *arg) { return true; }
	bool Stop(void **returnVal) { return true; }
	// END: required virtual from B2BModule.  See that class for doc

	// START: required virtual from SensorHW.  See that class for doc
	// See top of this file for SensorHWData documentation
	bool SensorHWGetLatest(SensorHWData *pData,
							SensorHWStatus *pStatus=NULL, bool raw=false);
	// END: required virtual from SensorHW

  private:

	// Convert Affectiva's brightness to a level
	// RETURNS:
	//	 HIGH: brightness 25-75.  Affectiva says "good".
	//	 MEDIUM: brightness of 10-25 or 75-90.  Affectiva says "suboptimal"
	//	 LOW: brightness of 0-10 or 90-100. Affectiva says "not reliable".
	//			All other values also return LOW.
	static B2BLogic::Level AffectivaBrightnessToLevel(int16_t brightness);

  	AffectivaSDKHW *m_sdkHW; // from ctor

	std::string m_eventName; // from ioConfig's eventName
	float m_eventValue;		 // from ioConfig's threshold or compareValue

	// Holding area for latest data
	SensorHWData m_data;

	// tests need access to privates
	friend class MomentaCreatureMenu;
};
