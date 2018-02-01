#include "log/B2BLog.h"

#include "SensorHW.h"

SensorHW::SensorHW(const IOConfig &ioConfig, 
					const IOConfigEntryNames &ioConfigEntryNames,
					RangeOptimization rangeOptimization) :
	//m_sensorHWGeometries, // init'ed by own ctor to 0s
	m_rangeOptimization(rangeOptimization),
	m_enabled(true),  // Assume driver is enabled
	m_isValid(false)  // Driver not initialized yet
{
	CTORCommon(ioConfig, ioConfigEntryNames);
}

SensorHW::SensorHW(const IOConfig &ioConfig, 
					const char * ioConfigEntryName,
					RangeOptimization rangeOptimization) :
	//m_sensorHWGeometries, // init'ed by own ctor to 0s
	m_rangeOptimization(rangeOptimization),
	m_isValid(false)
{
	IOConfigEntryNames ioConfigEntryNames;
	if (ioConfigEntryName)
		ioConfigEntryNames.push_back(ioConfigEntryName);
	CTORCommon(ioConfig, ioConfigEntryNames);
}

SensorHW::~SensorHW()
{
	pthread_mutex_destroy(&m_lockSensorHW);
}

bool SensorHW::CTORCommon(const IOConfig &ioConfig, 
					const IOConfigEntryNames &ioConfigEntryNames)
{
	bool returnVal = true; // assume SUCCESS

	for (uint8_t entryCount = 0; entryCount < ioConfigEntryNames.size();
		 ++entryCount)
	{
		m_thresholdsEntries.push_back(IOConfig::IOThresholds());
		m_sensorHWGeometries.push_back(SensorHWGeometry());

		const IOConfig::IOConfigEntry *entry = 
							ioConfig.Lookup(ioConfigEntryNames[entryCount]);
		if (entry) {

			if (!entry->enable) {
				B2BLog::Err(LogFilt::LM_DRIVERS,
					"SensorHW %s: IOConfig \"enable\":false", 
					ioConfigEntryNames[entryCount]);
				m_enabled = false;
			}

			m_thresholdsEntries[entryCount] = entry->thresholds;

			SensorHWGeometry &geom = m_sensorHWGeometries[entryCount];
			// sensorLoc
			if (ioConfig.GetExtraSettingsValue<float>(
						ioConfigEntryNames[entryCount], "offsetU",
						&geom.sensorLoc.U, false))
			{
				if (geom.sensorLoc.U != 0)
					geom.locAtOrigin = false;
			} // else: leave value set to 0

			if (ioConfig.GetExtraSettingsValue<float>(
						ioConfigEntryNames[entryCount], "offsetV",
						&geom.sensorLoc.V, false))
			{
				if (geom.sensorLoc.V != 0)
					geom.locAtOrigin = false;
			} // else: leave value set to 0

			if (ioConfig.GetExtraSettingsValue<float>(
						ioConfigEntryNames[entryCount], "offsetW",
						&geom.sensorLoc.W, false))
			{
				if (geom.sensorLoc.W != 0)
					geom.locAtOrigin = false;
			} // else: leave value set to 0

			// sensorOrientation
			ioConfig.GetExtraSettingsValue<float>(
						ioConfigEntryNames[entryCount], "horizAngle",
						&geom.sensorOrientation.angle, false);
			ioConfig.GetExtraSettingsValue<float>(
						ioConfigEntryNames[entryCount], "azimuth",
						&geom.sensorOrientation.azimuth, false);
		} else {
			B2BLog::Err(LogFilt::LM_DRIVERS,
				"SensorHW FAIL: %s not defined in IOConfig", 
				ioConfigEntryNames[entryCount]);
			returnVal = false; // FAIL
		}
	}

	pthread_mutexattr_t mutexattr;
	pthread_mutexattr_init(&mutexattr);
	pthread_mutexattr_settype(&mutexattr, PTHREAD_MUTEX_RECURSIVE);
	pthread_mutex_init(&m_lockSensorHW, &mutexattr);

	return returnVal;
}

// DEBUG ONLY!!!
bool SensorHW::CheckForTestData(SensorHWData *pData, SensorHWStatus *pStatus)
{
	if (m_fakeSensorHWData.timestamp) {
		*pData = m_fakeSensorHWData;
		// So that injected test data only gets used once, mark as invalid
		m_fakeSensorHWData.timestamp = 0;
		if (pStatus) *pStatus = HWSTATUS_OK;
		return true; // We have injected test data
	}
	return false; // no injected test data
} 

/*static*/ const char * const SensorHW::RangeOptimizationString[SensorHW::RANGEOPT_TOTAL] =
{
	"DEFAULT", "ACCURACY", "LONG", "SHORT", "SPEED"
};
