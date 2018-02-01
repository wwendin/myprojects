#pragma once
//
// SensorEngineSensorHW: base class for all SensorEngine sensors that are based
//		on SensorHW low level sensor HW driver.   There are many of these
//		and their common code warranted a base class.
// SensorEngineSensorHWData: base class for all SensorEngine sensor's data
//
#include "drivers/common/sensor/SensorHW.h"

#include "SensorData.h"
#include "SensorEngine.h"

class SensorEngineSensorHWData : public SensorData {
public:
	SensorEngineSensorHWData(const char *source) :
		SensorData(source)
		//m_sensorHWData()  // init'ed by its own ctor
		{
		}
	virtual ~SensorEngineSensorHWData() {}

	virtual SensorEngineSensorHWData *clone() const
	  { return new SensorEngineSensorHWData(*this); }

	SensorHWData m_sensorHWData; // The HW data
};

class SensorEngineSensorHW : public SensorEngine {
  public:
	// name: the name of the sensor
	// singleCharID: a single char used to ID this sensor (used in menu
	// 		and must be unique among all sensors)
	// sensorHW: the SensorHW instance we will use to get our sensor data
	// triggerLevel: see PollSensorHW() below for documentation
	SensorEngineSensorHW(const char *name, ModuleApplicator* applicator,
						PersonalityEngine* pe, char singleCharID,
						SensorHW *sensorHW, B2BLogic::Level triggerLevel);
	virtual ~SensorEngineSensorHW();


	// The sensor's data.  
	// PAY ATTENTION: must be write/read with a m_lck
	// FIXME: enforce this by requiring a method to access data?!!?
	SensorEngineSensorHWData m_sensorData;

  protected:
    // START: SensorEngine override.  See that class for documentation.
	// setSensorData: if true and P has been changed to 1, sets
	//					sets m_sensorHWData to:
	//					* m_sensorHWData.timestamp: GetCurrentB2BTimestamp()
	//					* magnitude: 0.9
	//					* vector: all 0s
	//					* level: MEDIUM
	// 				  if false, m_sensorHWData is not altered.
	// PAY ATTENTION!!!! This method is for TEST AND DEBUG ONLY!!!!
	virtual void DebugSetSensorPValue(Cue newValue, bool setSensorData=true);

	// Default is MEDIUM trigger (most derived classes use this level)
    virtual bool PrePoll();
	virtual bool OnPollAction() { return PollSensorHW(m_triggerLevel); } //override of Neuron::OnPollAction
    // END: SensorEngine override

	// Same as SensorEngine::OnPollAction() but also looks at SensorHW data using
	// SensorEngine's GetLatestSensorHWData().
	// Sets P to 1 is all the following are true:
	// * GetLatestSensorHWData() returns true (we have new data)
	// * SensorHWData.level >= triggerLevel
	//
	// Set triggerLevel to LEVEL_NONE to ignore SensorHWData.level.
	//
	// Derived classes can override if they need something special here.
	virtual bool PollSensorHW(B2BLogic::Level triggerLevel);
};
