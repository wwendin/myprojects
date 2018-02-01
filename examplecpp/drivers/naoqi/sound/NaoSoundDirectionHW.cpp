#include "common/B2BTime.h"
#include "log/B2BLog.h"

#include <alvalue/alvalue.h>
#include <alproxies/alsoundlocalizationproxy.h>

#include "NaoSoundDirectionHW.h"

// default values if there is nothing in ioconfig 
#define NAOSOUNDHW_CONFIDENCE_THRESH 		0.9
#define NAOSOUNDHW_SENSITIVITY_THRESH 		0.9

NaoSoundDirectionHW::NaoSoundDirectionHW(boost::shared_ptr<AL::ALBroker> broker,
							const NaoEventModuleSensorHWConfig &config) :
		SensorHW(config.m_ioConfig, config.m_ioConfigEntryNames),
		NaoEventModuleSensorHW(broker, config),
		m_confidenceThreshold(NAOSOUNDHW_CONFIDENCE_THRESH),
		m_sensitivityThreshold(NAOSOUNDHW_SENSITIVITY_THRESH)
{
	if (!config.m_ioConfig.GetExtraSettingsValue<float>(
				config.m_ioConfigEntryNames[0], "sensitivityThreshold",
				&m_sensitivityThreshold))
	{
	  B2BLog::Err(LogFilt::LM_DRIVERS,
		"%s: IOConfig %s sensitivityThreshold is not defined.  Using default %.1g",
			  getName().c_str(), config.m_ioConfigEntryNames[0],
			  m_sensitivityThreshold);
	}

	if (!config.m_ioConfig.GetExtraSettingsValue<float>(
				config.m_ioConfigEntryNames[0], "confidenceThreshold",
				&m_confidenceThreshold))
	{
	  B2BLog::Err(LogFilt::LM_DRIVERS,
		"%s: IOConfig %s confidenceThreshold is not defined.  Using default %.1g",
			  getName().c_str(), config.m_ioConfigEntryNames[0],
			  m_confidenceThreshold);
	}

	B2BLog::Debug(LogFilt::LM_DRIVERS, "%s: confidenceThreshold: %.1g sensitivityThreshold: %.1g",
			  		getName().c_str(), 
					m_confidenceThreshold, m_sensitivityThreshold);

	// Create vector with all timestamp 0.  Indexed from LOW to HIGH.
	for (int level = B2BLogic::LEVEL_LOW; level < B2BLogic::LEVEL_TOTAL;++level)
	{
		m_levelSoundData.push_back(SensorHWData());
	}
}

void NaoSoundDirectionHW::init()
{
  	if (!IsEnabled())
  		return; // We're not enabled, don't init anything

  	try {
	  // This turns sound on!   It isn't on by default
  	  AL::ALSoundLocalizationProxy sound(getParentBroker()); 
  	  sound.setParameter("Sensitivity", m_sensitivityThreshold);
  	  sound.setParameter("EnergyComputation", true);
  	  sound.subscribe(getName().c_str());
  	} catch (const AL::ALError& e) {
	  B2BLog::Err(LogFilt::LM_DRIVERS, 
				"ALSoundLocalizationProxy instantiation failed %s.", e.what());
	  return; // FAIL
  	}

	NaoEventModuleSensorHW::init(); // finish up	
}

void NaoSoundDirectionHW::onEvent(const std::string &key, 
					const AL::ALValue &value, const AL::ALValue &message)
{
  B2BLog::Debug(LogFilt::LM_DRIVERS, 
  			   "%s: onEvent %skey: %s value: %s (type: %s) message: %s",
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
  if (!value.isArray())
  	return; // NO DATA: value passed in is not an array (should never happen)
  if (value.getSize() == 0)
  	return; // NO DATA: nothing in array, sensor is not "present"

  // value is [[timestamp], [localizationInfo], [6D torso], [6D robot]]
  AL::ALValue localizationInfo = value[1]; // [azimuth, elevation, confidence, energy]
  B2BLog::Debug(LogFilt::LM_DRIVERS, "%s: %s",
  				getName().c_str(), localizationInfo.toString().c_str());
  if ((float)(localizationInfo[2]) < m_confidenceThreshold) 
  	return; // not enough confidence, don't store poor data

  // FIXME: move thresholds to config file
  float energy = localizationInfo[3];
  B2BLogic::Level level = B2BLogic::FindLevelFromThresholds(energy, m_thresholdsEntries[0].threshValues);
  if ((level == B2BLogic::LEVEL_NONE) || (level == B2BLogic::LEVEL_TOTAL)) {
	return; // no sound signal, get out of here
  } 

  pthread_mutex_lock(&m_lockSensorHW);
  m_data.vector.magnitude = energy;
  // FIXME: need to repair these angles based on torso and head frames???
  m_data.vector.angle = -B2BMath::RadiansToDegrees(localizationInfo[0]);
  m_data.vector.azimuth = B2BMath::RadiansToDegrees(localizationInfo[1]);
  m_data.timestamp = B2BTime::GetCurrentB2BTimestamp();
  m_data.level = level;
  m_levelSoundData[level] = m_data;
  //B2BLog::Info(LogFilt::LM_DRIVERS, "%s: %s %s",
  //				getName().c_str(), localizationInfo.toString().c_str(),
  //				B2BLogic::LevelToString(level));
  pthread_mutex_unlock(&m_lockSensorHW);
}

bool NaoSoundDirectionHW::SensorHWGetLatestOfLevel(B2BLogic::Level level,
							b2b::Timestamp since, SensorHWData *pData,
							SensorHWStatus *pStatus, bool raw)
{
	if (!IsValid()) {
		if (pStatus) *pStatus = HWSTATUS_ERROR;
		return false; // FAIL: error in init()
	}

	if ((level <= B2BLogic::LEVEL_NONE) ||
		(level >= B2BLogic::LEVEL_TOTAL))
	{
		if (pStatus) *pStatus = HWSTATUS_NODATA;
		return false; // invalid level
	}

	bool result = true; // assume SUCCESS
	SensorHWData *dataP = &m_levelSoundData[level];
	pthread_mutex_lock(&m_lockSensorHW);
	if (dataP->timestamp && (dataP->timestamp > since)) {
		*pData = *dataP; // SUCCESS
		if (pStatus) *pStatus = HWSTATUS_OK;
	} else {
		result = false; // FAIL: no data
		if (pStatus) *pStatus = HWSTATUS_NODATA;
	}
	pthread_mutex_unlock(&m_lockSensorHW);

	return result;
}
