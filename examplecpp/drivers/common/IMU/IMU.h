#pragma once
//
// Base class for all IMUs
//
#include <string.h>
#include <pthread.h>
#include <vector>

#include "common/b2btypes.h"
#include "common/B2BMath.h"
#include "common/B2BLogic.h"

#include "drivers/common/sensor/SensorHW.h"

class IMU : public SensorHW {
  public:
	IMU(const IOConfig &ioConfig, const char *ioConfigEntryName);
	virtual ~IMU();

	typedef float GyroAxisData; // one axis of gyro data
	class GyroData {
	  public:
		GyroData() : timestamp(0) { memset(data, 0, sizeof(data)); }
		GyroAxisData data[B2BMath::PLATAXIS_TOTAL]; // deg/s (dps) or deg 
		b2b::Timestamp timestamp; // When the data was read, 0 if invalid
	};
	// Returns latest gyro data in pData.  Data units are deg/s.
	// RETURNS: true if there is data, false otherwise
	virtual bool GyroGetLatestDPS(GyroData *pData, 
								  SensorHW::SensorHWStatus *pStatus=NULL) = 0;

	///// Gyro Delta Degree methods (see B2BMath::PlatformAxis for details)
	//	This group of methods provide a service to clients that want to know how
	//  much vehicle has moved (and not have to query deg/s at a high rate!).
	//  The GyroDeltaDegreeXXXX group of methods provide the functionality
	//	of degrees changed (delta).   
	//  Each client must call GyroDeltaDegreeCreate to create an "instance"
	//  to track gyro change (delta).
	typedef uint8_t GyroDeltaDegreeId; // id of delta degree instance
	// Must be called to get a deltaId for tracking a change in degrees.
	// This call starts the tracking and calls GyroDeltaDegreeReset(deltaId)
	// where deltaId is the id returned by this method.
	// This method never fails.   A valid deltaId is always returned.
	GyroDeltaDegreeId GyroDeltaDegreeCreate();
	// Called when caller no longer needs to track degrees
	// RETURNS: true if deltaId existed, false otherwise
	bool GyroDeltaDegreeFree(GyroDeltaDegreeId deltaId);
	// Returns degrees moved since last time Create or Reset was called with
	// this deltaId.
	// deltaId: different deltaId allow multiple callers to track delta value
	//			independently.
	// pData: data is returned here (data is in degrees)
	// The first time this method is called, it returns degrees moved
	//		since GyroDeltaDegreeCreate was called.
	// RETURNS: true if deltaId has been created and there is valid gyro data,
	//			false otherwise
	bool GyroDeltaDegreeGet(GyroDeltaDegreeId deltaId,
										GyroData *pData) const;
	// Restarts the delta at 0.
	// RETURNS: true if deltaId has been allocated, false otherwise
	bool GyroDeltaDegreeReset(GyroDeltaDegreeId deltaId);

	typedef float AccelAxisData; // one axis of accel data
	class AccelData {
	  public:
		AccelData() : timestamp(0) { memset(data, 0, sizeof(data)); }
		AccelAxisData data[B2BMath::PLATAXIS_TOTAL];	// Gs
		b2b::Timestamp timestamp; // When the data was read, 0 if invalid
	};
	// Returns latest accel data in pData.   Data units are G (9.8m/s units)
	// RETURNS: true if there is data, false otherwise
	virtual bool AccelGetLatestG(AccelData *pData,
								  SensorHW::SensorHWStatus *pStatus=NULL) = 0;

	typedef float CompassAxisData; // one axis of compass data
	class CompassData {
	  public:
		CompassData() : timestamp(0) { memset(data, 0, sizeof(data)); }
		CompassAxisData data[B2BMath::PLATAXIS_TOTAL]; // gauss
		b2b::Timestamp timestamp; // When the data was read, 0 if invalid
	};
	// Returns latest compass data in pData.   Data units are gauss.
	// RETURNS: true if there is data, false otherwise
	virtual bool CompassGetLatestGauss(CompassData *pData,
								  SensorHW::SensorHWStatus *pStatus=NULL) = 0;

  protected:
	// RETURNS: true if deltaId was allocated via GyroDeltaDegreeCreate and
	//			has not been free'd, false otherwise
	bool GyroDeltaDegreeIdIsValid(GyroDeltaDegreeId deltaId) const;

	// For protecting m_gyroDegreeDeltaDB and any other IMU derived class' data
	mutable pthread_mutex_t m_lockIMU;

	// index is GyroDeltaDegreeCreate deltaId
	typedef std::vector<GyroData> GyroDegreeDeltaDB;  // DB for tracking deltas
	// List of allocated trackers.  Indexed by deltaId.   When entry's timestamp
	// is 0, the deltaId entry has been free'd and can be reused.
	GyroDegreeDeltaDB m_gyroDegreeDeltaDB;
};
