#include "common/B2BTime.h"
#include "log/B2BLog.h"

#include <alvalue/alvalue.h>
#include <alcommon/alproxy.h>
#include <alcommon/albroker.h>

#include "NaoEventModuleSensorHW.h"

NaoEventModuleSensorHW::NaoEventModuleSensorHW(
						boost::shared_ptr<AL::ALBroker> broker,
						const NaoEventModuleSensorHWConfig &config) :
	SensorHW(config.m_ioConfig, config.m_ioConfigEntryNames),
  	AL::ALModule(broker, config.m_moduleName)
{
  m_eventNames = config.m_eventNames;

  //B2BLog::Info(LogFilt::LM_DRIVERS, "Creating module \"%s\"",
  //									getName().c_str());

  std::string desc = "This module presents how to subscribe to a event";
  setModuleDescription(desc.c_str());

  if (!IsEnabled())
  	return; // driver is disabled; don't enable anything

  EventNames::const_iterator iter;
  std::string eventString;
  for (iter = m_eventNames.begin(); iter != m_eventNames.end(); ++iter) {
	if (eventString.length())
  		eventString += ", " + *iter;
	else
  		eventString = *iter;
  }
  B2BLog::Debug(LogFilt::LM_DRIVERS, 
  					"Enabling module \"%s\" to listen to events \"%s\"", 
  		 			getName().c_str(), eventString.c_str());

  functionName("onEvent", getName().c_str(), "Method called when the event occurs.");
  BIND_METHOD(NaoEventModuleSensorHW::onEvent);
}

NaoEventModuleSensorHW::~NaoEventModuleSensorHW()
{
  	EventNames::const_iterator iter;
  	for (iter = m_eventNames.begin(); iter != m_eventNames.end(); ++iter) {
  		fMemoryProxy.unsubscribeToEvent(iter->c_str(), getName().c_str());
	}

  	//printf("delete NaoEventModuleSensorHW");
}

void NaoEventModuleSensorHW::init()
{

  if (!IsEnabled())
  	return; // We're not enabled, don't init anything

  try {
    /** Create a proxy to ALMemory.  */
    fMemoryProxy = AL::ALMemoryProxy(getParentBroker());

    /** Subscribe to event
    * Arguments:
    * - name of the event
    * - name of the module to be called for the callback
    * - name of the bound method to be called on event
    */
  	EventNames::const_iterator iter;
  	for (iter = m_eventNames.begin(); iter != m_eventNames.end(); ++iter) {
  		//B2BLog::Info(LogFilt::LM_DRIVERS, 
  		//	"Subscribe \"%s\"::onEvent to listen to event \"%s\"", 
  		//	 getName().c_str(), iter->c_str());
    	fMemoryProxy.subscribeToEvent(iter->c_str(), getName().c_str(),
									"onEvent");
	}

	SetValid();

  } catch (const AL::ALError& e) {
    B2BLog::Err(LogFilt::LM_DRIVERS, "%s: init() error %s", 
				getName().c_str(), e.what());
  }
}

void NaoEventModuleSensorHW::onEvent(const std::string &key, 
					const AL::ALValue &value, const AL::ALValue &message)
{
  B2BLog::Debug(LogFilt::LM_DRIVERS, 
  			   "%s::onEvent %skey: %s value: %s (type: %s) message: %s",
  				getName().c_str(), 
				IsValid() ? "" : "NOT VALID ",
				key.c_str(), 
				value.toString().c_str(),
				AL::ALValue::TypeToString(value.getType()).c_str(),
				message.toString().c_str());

  if (!IsValid())
    return; // NO DATA: this class' ctor or init() failed
  if (!value.isValid())
  	return; // NO DATA: value passed in is not valid (should never happen)

  if (value.isFloat()) {
  	float floatValue = value;
  	if (floatValue) {
	  pthread_mutex_lock(&m_lockSensorHW);
	  m_data.level = B2BLogic::LEVEL_MEDIUM;
	  m_data.vector.magnitude = floatValue;
	  m_data.timestamp = B2BTime::GetCurrentB2BTimestamp();
	  pthread_mutex_unlock(&m_lockSensorHW);
  	} // else: value is 0, sensor is not "present" (falling edge)
  } else if (value.isInt()) {
  	int intValue = value;
  	if (intValue) {
	  pthread_mutex_lock(&m_lockSensorHW);
	  m_data.level = B2BLogic::LEVEL_MEDIUM;
	  m_data.vector.magnitude = intValue;
	  m_data.timestamp = B2BTime::GetCurrentB2BTimestamp();
	  pthread_mutex_unlock(&m_lockSensorHW);
  	} // else: value is 0, sensor is not "present" (falling edge)
  } else if (value.isBool()) {
  	bool boolValue = value;
  	if (boolValue) {
	  pthread_mutex_lock(&m_lockSensorHW);
	  m_data.level = B2BLogic::LEVEL_MEDIUM;
	  m_data.vector.magnitude = 1.0;
	  m_data.timestamp = B2BTime::GetCurrentB2BTimestamp();
	  pthread_mutex_unlock(&m_lockSensorHW);
  	} // else: value is 0, sensor is not "present" (falling edge)
  } else if (value.isArray()) {
  	if (value.getSize() > 0) {
  	  // Always set magnitude to 1.
	  pthread_mutex_lock(&m_lockSensorHW);
	  m_data.level = B2BLogic::LEVEL_MEDIUM;
	  m_data.vector.magnitude = 1.0;
	  m_data.timestamp = B2BTime::GetCurrentB2BTimestamp();
	  pthread_mutex_unlock(&m_lockSensorHW);
  	} // else: nothing in array, sensor is not "present" (falling edge)
  } else {
  	// not a expected type.
  	if (value.getSize() > 0) {
  	  // Always set magnitude to 1.
	  pthread_mutex_lock(&m_lockSensorHW);
	  m_data.level = B2BLogic::LEVEL_MEDIUM;
	  m_data.vector.magnitude = 1.0;
	  m_data.timestamp = B2BTime::GetCurrentB2BTimestamp();
	  pthread_mutex_unlock(&m_lockSensorHW);
  	} // else: nothing in value, sensor is not "present" (falling edge)
  }
}

bool NaoEventModuleSensorHW::SensorHWGetLatest(SensorHWData *pData,
									   SensorHWStatus *pStatus, bool raw)
{
	if (!IsValid()) {
		if (pStatus) *pStatus = HWSTATUS_ERROR;
		return false; // FAIL: error in init()
	}

	bool result = true; // assume SUCCESS
	pthread_mutex_lock(&m_lockSensorHW);
	if (m_data.timestamp) {
		*pData = m_data; // SUCCESS
		if (pStatus) *pStatus = HWSTATUS_OK;
	} else {
		result = false; // FAIL: no data
		if (pStatus) *pStatus = HWSTATUS_NODATA;
	}
	pthread_mutex_unlock(&m_lockSensorHW);

	return result;
}
