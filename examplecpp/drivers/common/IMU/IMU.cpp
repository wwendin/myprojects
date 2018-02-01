#include <string.h>

#include "common/b2bassert.h"
#include "common/B2BTime.h"

#include "IMU.h"

IMU::IMU(const IOConfig &ioConfig, const char *ioConfigEntryName) :
	SensorHW(ioConfig, ioConfigEntryName)
{
	pthread_mutexattr_t mutexattr;
	pthread_mutexattr_init(&mutexattr);
	pthread_mutexattr_settype(&mutexattr, PTHREAD_MUTEX_RECURSIVE);
	pthread_mutex_init(&m_lockIMU, &mutexattr);
}

IMU::~IMU()
{
	pthread_mutex_destroy(&m_lockIMU);
}

IMU::GyroDeltaDegreeId IMU::GyroDeltaDegreeCreate()
{
	
	GyroDeltaDegreeId deltaId = 0;
	bool found = false; // reusable location not found

	pthread_mutex_lock(&m_lockIMU);
	for (deltaId = 0; deltaId < m_gyroDegreeDeltaDB.size(); ++deltaId) {
		if (m_gyroDegreeDeltaDB[deltaId].timestamp == 0) {
			found = true;
			break; // Available location, reuse it
		}
	}

	if (!found) {
		// No reusable location found.   Add one to end
		m_gyroDegreeDeltaDB.push_back(GyroData());
		b2bassert(deltaId == (m_gyroDegreeDeltaDB.size()-1));
	}

	m_gyroDegreeDeltaDB[deltaId].timestamp = B2BTime::GetCurrentB2BTimestamp();
	GyroDeltaDegreeReset(deltaId);
	pthread_mutex_unlock(&m_lockIMU);

	return deltaId;
}

bool IMU::GyroDeltaDegreeFree(GyroDeltaDegreeId deltaId)
{
	bool returnVal = true; // Assume we free successfully

	pthread_mutex_lock(&m_lockIMU);
	if (GyroDeltaDegreeIdIsValid(deltaId)) {
		// Good deltaId: free it
		m_gyroDegreeDeltaDB[deltaId].timestamp = 0;
		returnVal = true; // SUCCESS
	} else {
		returnVal = false; // FAIL: deltaId out of range or already free'd
	}
	pthread_mutex_unlock(&m_lockIMU);


	return returnVal;
}

bool IMU::GyroDeltaDegreeGet(GyroDeltaDegreeId deltaId, GyroData *pData) const
{
	bool returnVal = true; // Assume we Get successfully

	pthread_mutex_lock(&m_lockIMU);
	if (GyroDeltaDegreeIdIsValid(deltaId)) {
		// Good deltaId: return its data
		*pData = m_gyroDegreeDeltaDB[deltaId];
		returnVal = true; // SUCCESS
	} else {
		returnVal = false; // FAIL: deltaId out of range or already free'd
	}
	pthread_mutex_unlock(&m_lockIMU);

	return returnVal;
}

bool IMU::GyroDeltaDegreeReset(GyroDeltaDegreeId deltaId)
{
	bool returnVal = true; // Assume we Get successfully

	pthread_mutex_lock(&m_lockIMU);
	if (GyroDeltaDegreeIdIsValid(deltaId)) {
		// Good deltaId: Zero the data
		memset(m_gyroDegreeDeltaDB[deltaId].data, 0,
				sizeof(m_gyroDegreeDeltaDB[deltaId].data));
		returnVal = true; // SUCCESS
	} else {
		returnVal = false; // FAIL: deltaId out of range or already free'd
	}
	pthread_mutex_unlock(&m_lockIMU);

	return returnVal;
}

bool IMU::GyroDeltaDegreeIdIsValid(GyroDeltaDegreeId deltaId) const
{
	// Return false if deltaId is "in range" and it was not already free'd
	pthread_mutex_lock(&m_lockIMU);
	bool result = (deltaId < m_gyroDegreeDeltaDB.size())
				  && (m_gyroDegreeDeltaDB[deltaId].timestamp != 0);
	pthread_mutex_unlock(&m_lockIMU);
	return result;
}
