#include "FrontRearObjectSensorEngine.h"
#include "common/B2BTime.h"

FrontRearObjectSensorEngine::FrontRearObjectSensorEngine(const char *name, ModuleApplicator* applicator, PersonalityEngine* pe, char singleCharID,
							b2b::TimeMS risingEdgeMinPeriodMS,
							SensorHW *frontSensorHW, SensorHW *rearSensorHW):
	
	SensorEngine(name, applicator, pe, singleCharID, frontSensorHW),
	m_objectSensorData(Name()),
	m_rearSensorHW(rearSensorHW),
	m_risingEdgeMinPeriodMS(risingEdgeMinPeriodMS),
	m_lastRearSensorHWDataTimestamp(0),
	m_timeOfLastRisingEdge(0)
{
    //GetData()->AddSource(&m_objectSensorData);

	AddTrait("Object");
}

FrontRearObjectSensorEngine::~FrontRearObjectSensorEngine()
{
}

bool FrontRearObjectSensorEngine::PrePoll()
{
    //WARNING: NO CHANGE TO m_sensorData.m_value SHOULD OCCUR
    //  BETWEEN Poll()s. IF IT DOES, THIS CODE MUST BE RUN BEFORE
    //  THE VALUE IS CHANGED.

    //Before we poll, remove the current sensor data.
    if(!IsFakeDataAvailable()){
        DecrementValue(m_objectSensorData.GetValue());
        m_objectSensorData.SetValue(0);
        GetData()->RemoveSourceByPointer(&m_objectSensorData);
    }
    return Neuron::PrePoll();
}

bool FrontRearObjectSensorEngine::OnPollAction()
{
	if(!GetData())
		return false;

	SensorHWData frontSensorHWData;
	bool frontSensorResult = GetLatestSensorHWData(&frontSensorHWData);
	SensorHWData rearSensorHWData;
	bool rearSensorResult = GetLatestSensorHWData(m_rearSensorHW,
											&m_lastRearSensorHWDataTimestamp,
											&rearSensorHWData);
	if (frontSensorResult || rearSensorResult) {
		// We have new sensor data, and we have a object (or edge), set P
		b2b::Timestamp now = B2BTime::GetCurrentB2BTimestamp();
		if((m_risingEdgeMinPeriodMS == 0) 
				||
			((now - m_timeOfLastRisingEdge) > m_risingEdgeMinPeriodMS))
		{
		    m_objectSensorData.SetValue(1); //FIXME: This is for debug and demo only
			//printf("now: %u last: %u min: %u\n", now, m_timeOfLastRisingEdge, m_risingEdgeMinPeriodMS);
			StartAtRisingEdge(); //we always want to respond to objects, there should be no continuing signals right now.
		} // else: it's been <= 2sec since last rising edge, don't do it again

		// Give front sensors priority
		// FIXME: do something more sophisticated if front and back detect
		//			an object at same time
		if (frontSensorResult)
			m_objectSensorData.m_frontData = frontSensorHWData;
		else
			m_objectSensorData.m_frontData.timestamp = 0; // mark as invalid
		if (rearSensorResult)
			m_objectSensorData.m_rearData = rearSensorHWData;
		else
			m_objectSensorData.m_rearData.timestamp = 0; // mark as invalid

	    IncrementValue(m_objectSensorData.GetValue());
	    GetData()->AddSource(&m_objectSensorData);

	} // else: no new data

	//Get gain value and set as operativeThreshold.

    return Neuron::OnPollAction();
}

// TEST and DEBUG ONLY!!!
void FrontRearObjectSensorEngine::DebugSetSensorPValue(Cue newValue, bool setSensorData)
{
	if(!GetData())
		return;

    PrePoll();

	if (setSensorData && (newValue > 0)) {
	  // m_objectSensorData has never been set.   Set frontData to valid values.
	  // Reset vector to all 0 angles, which is legal
	  m_objectSensorData.m_frontData.vector.Reset();
	  m_objectSensorData.m_frontData.vector.magnitude = 0.3; // half of max
	  m_objectSensorData.m_frontData.level = B2BLogic::LEVEL_MEDIUM;
	  // Load w/ valid "now" timestmap
	  m_objectSensorData.m_frontData.timestamp = B2BTime::GetCurrentB2BTimestamp();
	  m_objectSensorData.m_rearData.timestamp = 0; // mark rearData as invalid

	  // NeuronData stuff
	  m_objectSensorData.SetValue(1);
	  IncrementValue(m_objectSensorData.GetValue());

	  // Add this data to our Neuron
	  GetData()->AddSource(&m_objectSensorData);
	}

	SensorEngine::DebugSetSensorPValue(newValue, false);
}
