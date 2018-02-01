#pragma once
// 
// Description: A general purpose object (or edge) SensorEngine
//

#include "drivers/common/sensor/SensorHW.h"
#include "SensorData.h"

#include "SensorEngine.h"


class FrontRearObjectSensorEngine : public SensorEngine {
  public:
	// name,singleCharID: see SensorEngine for documentation
	// risingEdgeMinPeriodMS: used by OnPollAction().   We send a rising edge
	//		no faster than this time period.  Set to 0 to disable this feature.
	// frontSensorHW: the front HW sensor(s)
	// rearSensorHW: the rear HW sensor(s)
	FrontRearObjectSensorEngine(const char *name, ModuleApplicator* applicator, PersonalityEngine* pe, char singleCharID,
							b2b::TimeMS risingEdgeMinPeriodMS,
							SensorHW *frontSensorHW, SensorHW *rearSensorHW);
	virtual ~FrontRearObjectSensorEngine();

	class FrontRearObjectSensorEngineData: public SensorData {
	public:
		FrontRearObjectSensorEngineData(const char *source) : SensorData(source) {}
		virtual ~FrontRearObjectSensorEngineData() {}

		virtual FrontRearObjectSensorEngineData *clone() const
			{ return new FrontRearObjectSensorEngineData(*this); }

		SensorHWData m_frontData;
		SensorHWData m_rearData;
	};

	//Store and update the sensor data.
	FrontRearObjectSensorEngineData m_objectSensorData;

    // START: SensorEngine required methods.  See that class for documentation
    virtual bool PrePoll();
    virtual bool OnPollAction(); // Read and process front and rear edge detectors.
    // END: SensorEngine required methods.

    // START: SensorEngine override.  See that class for documentation.
	// setSensorData: if true and P has been changed to 1, sets
	//					m_objectSensorData.m_frontData.timestamp to "now" and 
	//					  sets to magnitude 0.3 and vector to all 0s and
	//					  level to MEDIUM
	//					  m_objectSensorData.m_rearData.timestamp is set to 0
	// 				  if false, m_objectSensorData is not altered.
	// PAY ATTENTION!!!! This method is for TEST AND DEBUG ONLY!!!!
	virtual void DebugSetSensorPValue(Cue newValue, bool setSensorData=true);


  private:
    SensorHW *m_rearSensorHW; // from ctor
	b2b::TimeMS m_risingEdgeMinPeriodMS; // from ctor

	// Timestamp of last data returned by m_rearSensor->SensorHWGetLatest
	// CLARIFICATION: This is set every time we call SensorHWGetLatest *and*
	//	 	we have valid sensor data.
	b2b::Timestamp m_lastRearSensorHWDataTimestamp;

	//Allow the sensor to firing on a rising edge every so often.
	//TODO improve this system.
	b2b::Timestamp m_timeOfLastRisingEdge;


	// tests and test menus needs access to privates
	friend class CreatureMenu;
};
