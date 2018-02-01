// 
// Description: Sensor base class
//
#include "common/b2bassert.h"
#include "common/b2btypes.h"
#include "log/B2BLog.h"
#include "common/B2BTime.h"

#include "drivers/common/sensor/SensorHW.h"

#include "SensorEngine.h"

#include <stdio.h>

SensorEngine::SensorEngine(const char *name, ModuleApplicator* applicator, PersonalityEngine* pe,
		char singleCharID, SensorHW *sensorHW, B2BLogic::Level triggerLevel) :
	Neuron(name,applicator,pe),
	m_sensorHW(sensorHW),
	m_triggerLevel(triggerLevel),
	m_lastSensorHWDataTimestamp(0),
	m_singleCharID(singleCharID)
{
	AddTrait("Sensor");

 	pthread_mutex_init(&m_lck, NULL);
}

SensorEngine::~SensorEngine()
{
	pthread_mutex_destroy(&m_lck);
}
void SensorEngine::ChangePersonalities()
{
	//Nothing yet
}

bool SensorEngine::GetLatestSensorHWData(SensorHW *sensorHW,
							 b2b::Timestamp *pLastSensorHWDataTimestamp,
							 SensorHWData *pSensorHWData, 
							 b2b::TimeMS timeWindow)
{
	bool ret=true; //assume success.

	b2bassert(pSensorHWData);
	pthread_mutex_lock(&m_lck);
	if (sensorHW && sensorHW->SensorHWGetLatestOfLevel(m_triggerLevel, *pLastSensorHWDataTimestamp, pSensorHWData))
	{
		// We have data
		if (pSensorHWData->timestamp != *pLastSensorHWDataTimestamp)
		{
			// We have new data
			*pLastSensorHWDataTimestamp = pSensorHWData->timestamp;
			if (timeWindow)
			{
			  // Using timeWindow feature.   Make sure data is within window.
			  if ((B2BTime::GetCurrentB2BTimestamp() - pSensorHWData->timestamp)
					> timeWindow)
			  {
			  	ret=false; // FAIL: sensor data is too old
			  }
			}
		} else {
			ret=false; // FAIL: same sensor data as last call
		}
	} else {
		ret=false; // FAIL: no sensor or no sensor data available
	}
	pthread_mutex_unlock(&m_lck);

	return ret;
}

// TEST and DEBUG ONLY!!!
void SensorEngine::DebugSetSensorPValue(Cue newValue, bool setSensorData)
{
	// PAY ATTENTION: Make sure this is last if you add changes to SensorData
	Neuron::DebugSetPValue(newValue);  

	B2BLog::Debug(LogFilt::LM_SENSOR, "Sensor p value is now %f", GetValue());

	// We are running the default version of this method.  Derived sensor 
	// classes will need to override this method if the derived class
	// wants to do something with setSensorData=true.
	if (setSensorData) {
		B2BLog::Debug(LogFilt::LM_SENSOR, 
					  "%s ToggleSensorPValue ignores setSensorData", Name());
	}
}
