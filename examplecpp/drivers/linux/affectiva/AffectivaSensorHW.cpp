#include <stdio.h>

#include "common/b2bassert.h"

#include "AffectivaSDKHW.h"

#include "AffectivaSensorHW.h"

AffectivaSensorHW::AffectivaSensorHW(AffectivaSDKHW *sdkHW,
					const IOConfig &ioConfig, const char *ioConfigEntryName) :
	B2BModule(ioConfigEntryName),
	SensorHW(ioConfig, ioConfigEntryName),
	m_sdkHW(sdkHW),
	m_eventValue(0)
{
	std::string eventName;
	if (ioConfig.GetExtraSettingsValue<std::string>(
				ioConfigEntryName, "eventName", &eventName))
	{
	  // SUCCESS: eventName exists in ioConfig
	  m_eventName = eventName;
	} else {
	  B2BLog::Err(LogFilt::LM_DRIVERS,
			  "%s: no IOConfig \"eventName\" defined.  Driver init FAILED!!!",
			  Name());
	  SetEnabled(false); // Make sure SensorHW knows ctor failed
	  return; // FAIL
	}

	float tempFloat;
	if (ioConfig.GetExtraSettingsValue<float>(
				ioConfigEntryName, "threshold", &tempFloat, false))
	{
		// SUCCESS: threshold exists in ioConfig
		m_eventValue = tempFloat;
	} else {
		if (ioConfig.GetExtraSettingsValue<float>(
				ioConfigEntryName, "compareValue", &tempFloat, false))
		{
		  // SUCCESS: compareValue exists in ioConfig
		  m_eventValue = tempFloat;
		} else {
	  	  B2BLog::Err(LogFilt::LM_DRIVERS,
		"%s: IOConfig \"threshold\" and \"compareValue\" are is not defined.  Driver init FAILED!!!",
			  Name());
	  	  SetEnabled(false); // Make sure SensorHW knows ctor failed
	  	  return; // FAIL
		}
	}

	// Enable event and value in SDK HW driver
	if (!m_sdkHW->EventEnable(true, m_eventName.c_str(), m_eventValue)) {
	  	B2BLog::Err(LogFilt::LM_DRIVERS,
			"%s: AffectivaSDKHW::EventEnable failed.  Bad eventName \"%s\"", 
			Name(), m_eventName.c_str());
	  	SetEnabled(false); // Make sure SensorHW knows ctor failed
	  	return; // FAIL
	}

	SetValid(); // we're ready to go
}

bool AffectivaSensorHW::SensorHWGetLatest(SensorHWData *pData,
									   SensorHWStatus *pStatus, bool raw)
{
	if (!IsValid()) {
		if (pStatus) *pStatus = HWSTATUS_ERROR;
		return false; // FAIL: error in init()
	}

	bool result = true; // assume SUCCESS
	pthread_mutex_lock(&m_lockSensorHW);

	AffectivaSDKHW::AffectivaEventHWData affectivaHWData;
	if (m_sdkHW->AffectivaEventsGetLatest(m_eventName.c_str(),
							&affectivaHWData, pStatus)
			&&
		affectivaHWData.Valid())
	{
		b2bassert(affectivaHWData.m_eventDataList.size() == 1);
		AffectivaSDKHW::EventDataList::const_iterator iter = 
									affectivaHWData.m_eventDataList.begin();

		#if 0
		AffectivaSDKHW::AffEvent event = AffectivaSDKHW::StringToAffEvent(m_eventName.c_str());
		b2bassert(iter->first == event);
		#endif

		const AffectivaSDKHW::AffEventData &eventData = iter->second;
		m_data.level = AffectivaBrightnessToLevel(eventData.m_brightness); 
		m_data.vector.magnitude = eventData.m_value;
		m_data.timestamp = eventData.m_timestamp;
		*pData = m_data; // SUCCESS
		if (pStatus) *pStatus = HWSTATUS_OK;
	} else {
		result = false; // FAIL: no data
		if (pStatus) *pStatus = HWSTATUS_NODATA;
	}
	pthread_mutex_unlock(&m_lockSensorHW);

	return result;
}

/*static*/ B2BLogic::Level AffectivaSensorHW::AffectivaBrightnessToLevel(
															int16_t brightness)
{
	if ((brightness >= 25) && (brightness <= 65))
		return B2BLogic::LEVEL_HIGH;
	else if ((brightness >= 10) && (brightness <= 90))
		return B2BLogic::LEVEL_MEDIUM;
	else 
		return B2BLogic::LEVEL_LOW;
}
