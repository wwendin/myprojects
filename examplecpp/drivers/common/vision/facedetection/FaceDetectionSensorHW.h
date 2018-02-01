#pragma once
// 
//		Face detection HW driver base class
//			SensorHWData values are defined by derived class
//		Two operating modes:
//		* Simple face detection: sets data if any face detected
//			This is the default operating mode.
//		* Learned (known) face detection: sets data if a learned face 
//			is detected.   Must call SetLearnedFace() to enable this.
//
#include <string>

#include "apps/common/IOConfig.h"

#include "drivers/common/sensor/SensorHW.h"

class FaceDetectionSensorHW : public virtual SensorHW {
  public:
	// IOConfig used: all are SensorHW only.  This class doesn't do anything
	//		except pass it on
	FaceDetectionSensorHW(const IOConfig &ioConfig, 
							const char *ioConfigEntryName);
	FaceDetectionSensorHW(const IOConfig &ioConfig, 
					const IOConfigEntryNames &ioConfigEntryNames);
	virtual ~FaceDetectionSensorHW();

	// Sets face that onEvent will detect when setting SensorHW data
	// Only one learned face is allowed per instance of this driver
	void LearnedFaceSet(const std::string &faceLabel)
		{ m_faceLabel = faceLabel; }
	// Return the value set by LearnedFaceSet()
	std::string LearnedFaceGet() { return m_faceLabel; }
	// Deletes face set by SetLearnedFace()
	void LearnedFaceDelete() { m_faceLabel.clear(); }

	// Additional data that we store
	// m_id: set to faceID 0
	// m_confidence: depends on HW (see derived class for info)
	// m_label: set to faceLabel 0
	class FaceHWData {
	  public:
	  	FaceHWData() : 
			m_id(FACEHWDATA_ID_INVALID),
			m_confidence(0) {}

		// RETURNS: true if FaceHWData is valid, false otherwise
		bool Valid() const { return m_id != FACEHWDATA_ID_INVALID; }

		static const int32_t FACEHWDATA_ID_INVALID = -1;
		int32_t m_id;
		float m_confidence;
		std::string m_label;
	};

	// Same as SensorHWGetLatest but adds FaceHWData
	bool FaceDetectionGetLatest(SensorHWData *pData,
							FaceHWData *pFaceHWData,
							SensorHWStatus *pStatus=NULL, bool raw=false);

	//Optional GetLatest that returns different data based on what level is wanted,
	//	only if that level was updated recently (according to overload).
	//If this is not overloaded, it will return SensorHWGetLatest by default.
	virtual bool FaceDetectionGetLatestOfLevel(B2BLogic::Level level, b2b::Timestamp since, SensorHWData *pData, FaceHWData *pFaceHWData, 
			SensorHWStatus *pStatus=NULL, bool raw=false)
	{return FaceDetectionGetLatest(pData, pFaceHWData, pStatus, raw);}

  protected:
	FaceHWData m_faceHWData; // Holding area for latest face data

  private:
	std::string m_faceLabel; // set by SetLearnedFace()
};
