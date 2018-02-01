#pragma once
//
// Base classes for all sensor HW that measures a direction, magnitude,
// and level.   magnitude and level are defined by the derived class.
// All math is based on B2BMath::Vector3D.
// This class has a single data output (via SensorHWGetLatest) but can have
// one or more inputs.   
//
// SensorHWGetLatest returns a vector from center of vehicle to center of object
// (vector C to O in diagrams below).  This method must be written by
// user of SensorHW class (virtual function must be supplied).  In other words,
// this SensorHW class has no default SensorHWGetLatest behavior, and therefore
// SensorHW is an abstract class.
//
// Derived classes come in a couple of different types:
// * derived class has one piece of HW (see simple ctor below).  Examples:
//	 - STMicroDistanceHW class
// * derived class has multiple pieces of HW.  Examples:
//	 - EdgeDetect.   It fuses multiple sensors and returns the result.
//		Each of its sensors is a SensorHW class itself.
//	 - SparkfunSoundInputHW.   It also fuses multiple sensors and returns the
//			result.  It handles all logic of its multiple sensors in its class.
//
// Below are two views of an example sensor mounted on vehicle front.
// It is high, on right-side, angled outwards (to right) and angled 
// down (towards ground) and measured distance.
// Remember that this is just an example.  A sensor can be mounted anywhere
// on vehicle and with its distance sensing center at any angle outward
// from vehicle.  And SensorHW doesn't have to measure distance, this is only
// an example.
//
// First, a top view diagram of this example sensor, looking down.
// X = sensor is attached here
// O = location of object detected (specifically, the center of the object)
// U = offsetU: distance from C to Y.  offsetU is positive in this example
// V = offsetV: distance from center to sensor.  offsetV is positive number in
//	   this example.
// H = horizAngle: angle of center of the sensor's detection beam.
//	   H has same definition as Vector.angle in SensorSW (0 is 
//     forward, etc).  horizAngle is a positive number in this example
// Y = center of vehicle front edge
// C = center of vehicle
//
//	  ^
//    |                     				 |	 O
//    |            				     		 |  /
//	forward			                    	 |H/
//                      				     |/
// vehicle front edge	===============Y=====X=========   vehicle front edge
//                                    ^|<-V->|
//									  .|
//									  .|
//									  U|
//									  .|
//									  .|
//									  V|
//									   C
//
//
// Second, a side view diagram of the same example sensor, looking
// along front edge.
// X = sensor is attached here
// O = location of object detected
// U = offsetU: horizontal distance from C to X.
//	   offsetU is positive in this example.
// W = offsetW: distance from ground to sensor.
//	   offsetW is negative in this example.
// A = azimuth: same definition as Vector.azimuth in SensorSW.   However,
//	   A is azimuth from center of sensor to object.
//	   azimuth is a negative in this example.
// C = center of vehicle
//
//						Top of vehicle
//                              |
//                              |
//   <----backward            _ X........    ----> forward
//                            ^ |. A
//                            . | . 
//                            . |  . 
//                            W |   O
//                            . |
//                            . |
//              C             V |
//							The Ground
// 				 <------U------>
//
//
#include <pthread.h>
#include <vector>

#include "common/b2btypes.h"
#include "common/B2BMath.h"
#include "common/B2BLogic.h"
#include "common/B2BTime.h"

#include "apps/common/IOConfig.h"

//
// Derived class does not have to set all values in SensorHWData.  It can
// decide how many of these values it wants to define and use.
//
class SensorHWData {
  public:
	SensorHWData() : level(B2BLogic::LEVEL_TOTAL), timestamp(0){}

	// vector defines where the sensor detected something, relative to
	// the center of the vehicle.   
	// It doesn't matter where the sensor is mounted, this method always
	// returns a common orientation so that caller doesn't have to do the math.
	//
	// See B2BMath::Vector3D for definition of B2BMath::PlatformAxis.
	// REMEMBER!!! definition in B2BMath says that center of vehicle 
	// 		is on ground
	//
	// vector:
	//   magnitude: meaning and units of this value are defined by derived class
	//   angle: horizontal angle, in degrees, from center of vehicle.  
	//		  0 degrees is directly forward of center, 90 degrees is right of
	//		  center, 180 is backwards, 270 is left.
	//		  (this repeats documentation in B2BMath::PlatformAxis)
	//		  See picture in B2BMath::PlatformAxis
	//   azimuth: -90 is straight down, 0 is on horizontal plane, 90 straight up
	//		  (this repeats documentation in B2BMath::PlatformAxis)
	//

	B2BMath::Vector3D vector;

	// level: strength of sensor signal.  Exact meaning depends on derived class
	B2BLogic::Level level; 

	b2b::Timestamp timestamp; // When the data was read, 0 if data is invalid
};

class SensorHW {
  public:
	// Ranging optimization choices:
	//   DEFAULT: use a default optimization that is defined by derived class.
	//   ACCURACY: optimize for accuracy
	//   LONG: optimize for long range
	//   SHORT: optimize for short range
	//   SPEED: optimize for measurement speed
	// Derived classes are not required to support all of these.   
	// Derived classes may only support DEFAULT.
	typedef enum {
		RANGEOPT_DEFAULT = 0,
		RANGEOPT_ACCURACY,
		RANGEOPT_LONG,
		RANGEOPT_SHORT,
		RANGEOPT_SPEED,
		RANGEOPT_TOTAL		// size of enum (never used as a valid value)
	} RangeOptimization;

	typedef std::vector<const char *> IOConfigEntryNames;

	//
	// ioConfig: io config definition class
	//
	// ioConfigEntryNames: ioConfig entry names for each sensor we want to
	//	 monitor.  This base class only loads sensorLoc and sensorOrientation
	//   which are optional:
	//   	sensorLoc: set using json values "offsetU", "offsetV", "offsetW"
	//   	sensorOrientation: set using json values "horizAngle", "azimuth"
	//      If any value is not defined in json entry, it is set to 0.
	//	 If you do not wish to use ioConfig feature, make this list empty
	//		and ioConfig and ioConfigEntryNames will be ignored.
	//		sensorLoc and sensorOrientation are optional and therefore will
	//		not be set in this case (will remvain all 0s).
	//
	// rangeOptimization: see enum for documentation.  This feature is optional.
	//		Caller should set to DEFAULT if this feature is not used.
	// 
	SensorHW(const IOConfig &ioConfig,
				const IOConfigEntryNames &ioConfigEntryNames,
				RangeOptimization rangeOptimization=RANGEOPT_DEFAULT);
	// Same as above, but simplified for sensors that only have one piece of HW
	// Calls above ctor version with ioConfigEntryName stored as the single 
	//		entry in ioConfigEntryNames.
	// If you do not wish to use ioConfig feature, set ioConfigEntryName to
	//		NULL and ioConfigEntryNames will be passed as an empty list
	//		to above ctor version.
	SensorHW(const IOConfig &ioConfig,
				const char *ioConfigEntryName,
				RangeOptimization rangeOptimization=RANGEOPT_DEFAULT);
	virtual ~SensorHW();

	// OK: sensor is operating correctly
	// NODATA: sensor has no data.  This is not necessarily and error.  For
	//		example, the cliff sensor usually has no data.
	// ERROR: sensor is in an error state.
	typedef enum {
		HWSTATUS_OK = 0,
		HWSTATUS_NODATA,
		HWSTATUS_ERROR,
		HWSTATUS_TOTAL		// size of enum (never used as a valid value)
	} SensorHWStatus;

	// Returns latest data in pData.
	// See SensorHWData for data details.
	// pStatus: Status of sensor.   If this method returns true, then
	//			status is always OK.   If this method returns false,
	//			see enum for explanation of values.
	//			Call can set pStatus to NULL if it does not need status.
	// raw: if true, return raw sensor data not altered by geometry in any way.
	//			if false, return sensor data fixed for geometry, etc.
	//			false is the "normal case".
	// RETURNS: true if data exists, false otherwise
	//
	// This method must be fast:
	// * do not read slow hardware here
	// * do not block for a long time
	// * do not sleep
	// If derived classes must do slow work to measure and create the
	// SensorHWData, that slow logic MUST BE placed in a separate thread.
	// This method then gets the data stored by that separate thread and returns
	// the data *quickly*.  And once again, MAKE SURE that the thread never 
	// causes a long mutex wait as a side-effect in this SensorHWGetLatest.
	// method.  We must not block for more than a few *micro*-seconds here.
	virtual bool SensorHWGetLatest(SensorHWData *pData, SensorHWStatus *pStatus=NULL, bool raw=false) = 0;

	//Optional GetLatest that returns different data based on what level is wanted,
	//	only if that level was updated more recently than "since" parameter.
	//  If this is not overloaded, it will return SensorHWGetLatest by default.
	virtual bool SensorHWGetLatestOfLevel(B2BLogic::Level level, b2b::Timestamp since, SensorHWData *pData,
			SensorHWStatus *pStatus=NULL, bool raw=false)
	{return SensorHWGetLatest(pData, pStatus, raw);}

	// RETURNS: true if SensorHW has been enabled, false otherwise.
	//			Only false if either of the following is true:
	//			* IOConfig entry has "enable":false
	//			* derived class called SetEnabled(false) explicitely
	bool IsEnabled() const { return m_enabled; }

	// RETURNS: true if SensorHW has been initialized correctly and is usable;
	//			false otherwise.
	bool IsValid() const { return m_isValid; }

	// sensorLoc: location of sensor on vehicle: U, V, W
	// sensorOrientation: orientation of sensor from its location (NOT from
	//			the center of vehicle)
	// See diagrams at top of this file.
	class SensorHWGeometry {
	  public:
		SensorHWGeometry() : locAtOrigin(true) {}

		bool locAtOrigin;					// true if sensorLoc is all 0s
		B2BMath::Vector3DRect sensorLoc;	  // from ctor
		B2BMath::Vector3D sensorOrientation;  // from ctor
	};

	// idx into original ioConfigEntryNames passed to ctor
	// RETURNS: SensorHWEntry for idx, or NULL if idx is out of range
	const SensorHWGeometry *SensorGeometryGet(uint8_t idx) const
	{ 
		if (idx >= m_sensorHWGeometries.size())
			return NULL; // FAIL: idx out of range

		return &m_sensorHWGeometries[idx]; // SUCCESS
	}

	// Takes pData and adds to it the given m_sensorHWGeometries[idx]
	// RETURNS: true on success, false otherwise (idx is out of range)
	bool FixVectorForSensorGeometry(uint8_t idx, SensorHWData *pData) const
	{
		if (idx >= m_sensorHWGeometries.size())
			return NULL; // FAIL: idx out of range

		b2b::Magnitude saveMagnitude = pData->vector.magnitude;
		const SensorHWGeometry &geom = m_sensorHWGeometries[idx];
    	pData->vector = geom.sensorOrientation;
      	pData->vector.magnitude = saveMagnitude;
		if (!geom.locAtOrigin)
      		pData->vector += geom.sensorLoc;

		return true; // SUCCESS
	}

	static const char * const RangeOptimizationString[RANGEOPT_TOTAL];

  protected:
	void SetEnabled(bool value) { m_enabled = value; }
	void SetValid() { m_isValid = true; }
	void SetInvalid() { m_isValid = false; }

	// One entry for each entry in ctor's ioConfigEntryNames
	std::vector<IOConfig::IOThresholds> m_thresholdsEntries;
	std::vector<SensorHWGeometry> m_sensorHWGeometries;

    RangeOptimization m_rangeOptimization;	// from ctor

	// For protecting any derived class' data
	mutable pthread_mutex_t m_lockSensorHW;

	// DEBUG ONLY: 
	// Inject test data so that next call to SensorHWGetLatest
	//	 will return this value instead of real data.  Sets m_fakeSensorHWData.
	void InjectTestData(const SensorHWData &data)
	{ m_fakeSensorHWData = data; }
	// Returns injected test data if it is valid.
	// Only alters pData and pStatus if injected test data is valid.
	//	 Sets m_fakeSensorHWData.timestamp to 0 after copying injected test
	//   data to pData.
	virtual bool CheckForTestData(SensorHWData *pData,
								  SensorHWStatus *pStatus=NULL);
	SensorHWData m_fakeSensorHWData; // valid only if timestamp != 0

  private:
  	bool m_enabled;   // true if driver enabled; see IsEnabled()
  	bool m_isValid;   // true if driver init'ed successfully; see IsValid()

    // Helper function for the ctors
	// RETURNS: true on success, false otherwise
	bool CTORCommon(const IOConfig &ioConfig, 
					const IOConfigEntryNames &ioConfigEntryNames);
};
