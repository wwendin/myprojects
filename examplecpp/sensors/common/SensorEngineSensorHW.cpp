//
// Description: Sensor base class
//
#include <stdio.h>

#include "common/b2bassert.h"
#include "common/b2btypes.h"
#include "log/B2BLog.h"
#include "common/B2BTime.h"

#include "drivers/common/sensor/SensorHW.h"

#include "SensorEngineSensorHW.h"

SensorEngineSensorHW::SensorEngineSensorHW(const char *name, ModuleApplicator* applicator,
						PersonalityEngine* pe, char singleCharID,
						SensorHW *sensorHW, B2BLogic::Level triggerLevel) :
	SensorEngine(name, applicator, pe, singleCharID, sensorHW, triggerLevel),
	m_sensorData(Name())
{
	m_sensorData.ChangeOwnerInfo()->SetContainers(GetContainersWithAccess());
	m_sensorData.ChangeOwnerInfo()->SetOwnerContainer(GetNameOfOwnerContainer());
	m_sensorData.ChangeOwnerInfo()->SetTraits(*GetTraits()); //FIXME This may ignore traits added in the Sensor's constructor.

//	GetData()->AddSource(&m_sensorData); //This shouldn't be necessary
}

SensorEngineSensorHW::~SensorEngineSensorHW()
{
}

// TEST and DEBUG ONLY!!!
void SensorEngineSensorHW::DebugSetSensorPValue(Cue newValue, bool setSensorData)
{
    PrePoll();

	if (setSensorData && (newValue>0)) {
	  pthread_mutex_lock(&m_lck);
	    // Set m_sensorData to something valid.
	    // Reset vector to all 0 angles, which is legal
	    // FIXME: magnitude of 0.9 is arbitrary, need something smarter
	    m_sensorData.m_sensorHWData.vector.Reset();
	    m_sensorData.m_sensorHWData.vector.magnitude = 0.9;
	    m_sensorData.SetValue(1); //FIXME: This is for debug and demo only
	    m_sensorData.m_sensorHWData.level = B2BLogic::LEVEL_MEDIUM;
	    // Load w/ valid "now" timestmap
	    m_sensorData.m_sensorHWData.timestamp = B2BTime::GetCurrentB2BTimestamp();
	  pthread_mutex_unlock(&m_lck);
	  IncrementValue(m_sensorData.GetValue());
	  GetData()->AddSource(&m_sensorData);
	}

	// false means don't set sensorData twice
	SensorEngine::DebugSetSensorPValue(newValue, false);
}

bool SensorEngineSensorHW::PrePoll()
{

    //WARNING: NO CHANGE TO m_sensorData.m_value SHOULD OCCUR
    //  BETWEEN Poll()s. IF IT DOES, THIS CODE MUST BE RUN BEFORE
    //  THE VALUE IS CHANGED.

    //Before we poll, remove the current sensor data.
    if(!IsFakeDataAvailable()){
        //B2BLog::Info(LogFilt::LM_SENSOR, "%s decrementing value of %f by %f", Name(), GetValue(), m_sensorData.GetValue());
        DecrementValue(m_sensorData.GetValue());
        m_sensorData.SetValue(0);
        GetData()->RemoveSourceByPointer(&m_sensorData);
    }
    return Neuron::PrePoll();
}

bool SensorEngineSensorHW::PollSensorHW(B2BLogic::Level triggerLevel)
{
	SensorHWData sensorHWData;
	if (GetLatestSensorHWData(&sensorHWData)) {
		// We have new sensor HW data
		if ((triggerLevel == B2BLogic::LEVEL_NONE) ||
			(sensorHWData.level >= triggerLevel))
		{
			// We have HW data that triggers the neuron, set P and store HW data
		    //m_sensorData.SetValue(sensorHWData.vector.magnitude);
		    m_sensorData.SetValue(1); //FIXME: This is for debug and demo only
			m_sensorData.m_sensorHWData = sensorHWData;
			IncrementValue(m_sensorData.GetValue());
            GetData()->AddSource(&m_sensorData);
		} // else: no triggering of neuron
	} // else: no new data

	return Neuron::OnPollAction();
}
