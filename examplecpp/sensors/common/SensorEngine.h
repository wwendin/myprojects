#pragma once
/*
SensorEngine: base class upon which all sensors are built

API
    Excite: overload of Neuron::Excite that calls Neuron::SendPs() if 
        the p value of the class (from Neuron) is past the threshold.
    Note that the threshold here can be set as the gain value in each sensor
        individually.
    OnPollAction: required method that will be called by main clock (fast clock)

Design
	SensorEngine allows for a standard base for all sensors to operate on.
	As an child class of Neuron, all sensors will inherit a standard method
		of passing on data to other nodes.
*/
#include <vector>

#include "neural/Neuron.h"
#include "neural/Typedefs.h"
#include "common/B2BLogic.h"

class SensorHW;
class SensorHWData;

class SensorEngine : public Neuron {
  public:
	// name: the name of the sensor
	// singleCharID: a single char used to ID this sensor (used in menus
	// 		and must be unique among all sensors)
	// triggerLevel: triggerLevel.  Stored in m_triggerLevel.   How it is used
	//		is up to the derived class.
	//		If GetLatestSensorHWData() is not overridded and sensorHW is
	//		not NULL, triggerLevel is passed to 
	//		sensorHW->SensorHWGetLatestOfLevel().  In this case, only recent
	//		readings of exactly that triggerLevel will be read

	SensorEngine(const char *name, ModuleApplicator* applicator, PersonalityEngine* pe,
			char singleCharID, SensorHW *sensorHW=NULL, B2BLogic::Level triggerLevel=B2BLogic::LEVEL_TOTAL);
	virtual ~SensorEngine();

	// Default routine to get latest sensor data
	// pSensorHWData: sensor HW data is written here.
	//			BE CAREFUL: in future,
	//			some sensors may have a data class derived from SensorHWData and
	//			therefore pSensorData must point to the derived type.
	// timeWindow: window in past that sensor must be valid.   If sensor
	//			data's timestamp is older that now minus this timeWindow,
	//			this method returns false.   Set to 0 to disable this feature.
	// RETURNS: true if sensor data exists and has changed since
	//			last time this method was called.   Returns false otherwise.
	//			Also see optional timeWindow feature.
	//
	// The default method here has the following functionality:
	//	* Calls sensorHW->SensorHWGetLatestOfLevel() to get sensor data
	//    WARNING: most sensorHW treat SensorHWGetLatestOfLevel the same
	//			as SensorHWGetLatest().   Therefore triggerLevel is ignored
	//			in these cases.  Only sound HW classes currently support
	//			SensorHWGetLatestOfLevel using triggerLevel to 
	//			return true of false.
	//	* Only returns true if sensor data timestamp has changed since last call
	//  * timeWindow is used if not 0
	//	* If sensorHW (from ctor) is NULL, this method always returns false
	//
	bool GetLatestSensorHWData(SensorHWData *pSensorHWData, b2b::TimeMS timeWindow=0)
	{
		return GetLatestSensorHWData(m_sensorHW, &m_lastSensorHWDataTimestamp,
									pSensorHWData, timeWindow);
	}

	virtual void ChangePersonalities();

  protected:
	// Same as above version but we pass in SensorHW and lastTimestamp
	bool GetLatestSensorHWData(SensorHW *sensorHW,
							 b2b::Timestamp *pLastSensorHWDataTimestamp,
							 SensorHWData *pSensorHWData, 
							 b2b::TimeMS timeWindow=0);

	// START: Override of Neuron
	// Set the sensor's P value to newValue
	// PAY ATTENTION!!!! This method is for TEST AND DEBUG ONLY!!!!
	virtual void DebugSetPValue(Cue newValue) {DebugSetSensorPValue(newValue,true);}
	// END: Override of Neuron

	// Set the sensor's P value to newValue, and optionally sets sensor data.
    // setSensorData: if true and P has been changed to 1, this method will
	//					set the sensor's data to something that makes sense
	//					to pair with P value of 1.
	//				  if false, sensor's data is not altered.
	// PAY ATTENTION: Derived sensor classes will need to override this method
	// if the derived class wants to do something with setSensorData=true.
	// PAY ATTENTION!!!! This method is for TEST AND DEBUG ONLY!!!!
	virtual void DebugSetSensorPValue(Cue newValue, bool setSensorData=true);

	SensorHW *m_sensorHW;   		// from ctor
	B2BLogic::Level m_triggerLevel; // from ctor

	// used to protect data from async calls
	pthread_mutex_t m_lck;

	// Timestamp of last data returned by GetLatestSensorHWData
	// CLARIFICATION: This is set every time we call GetLatestSensorHWData *and*
	//	 	we have valid sensor data.   timeWindow doesn't matter in
	//		setting of this value.
	b2b::Timestamp m_lastSensorHWDataTimestamp;

  private:

	char m_singleCharID; //from ctor

	// tests and GUI need access to privates
    friend class GraphicNode;
	friend class CreatureMenu;
};
