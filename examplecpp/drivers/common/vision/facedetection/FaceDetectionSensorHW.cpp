#include "log/B2BLog.h"

#include "FaceDetectionSensorHW.h"

FaceDetectionSensorHW::FaceDetectionSensorHW(const IOConfig &ioConfig, 
							const char *ioConfigEntryName) :
		SensorHW(ioConfig, ioConfigEntryName)
{
	if (!IsEnabled()) return;
}

FaceDetectionSensorHW::FaceDetectionSensorHW(const IOConfig &ioConfig, 
					const IOConfigEntryNames &ioConfigEntryNames) :
		SensorHW(ioConfig, ioConfigEntryNames)
{
	if (!IsEnabled()) return;
}

FaceDetectionSensorHW::~FaceDetectionSensorHW()
{
}

bool FaceDetectionSensorHW::FaceDetectionGetLatest(SensorHWData *pData,
		FaceHWData *pFaceHWData, SensorHWStatus *pStatus, bool raw)
{
	bool result = true; // assume SUCCESS

	// Make sure SensorSWGetLatest and m_faceHWData are accessed atomically
	pthread_mutex_lock(&m_lockSensorHW);
		if (SensorHWGetLatest(pData, pStatus, raw)) {
			// SUCCESS
			if (pFaceHWData) *pFaceHWData = m_faceHWData;
		} else {
			result = false; // FAIL: SensorSWGetLatest already set pStatus
		}
	pthread_mutex_unlock(&m_lockSensorHW);

	return result;
}
