#pragma once
// 
//		NAO face detection HW driver.  Sets the following data:
//			SensorHWData:
//				vector is set to all 0s
//				vector.magnitude depends on mode (see below)
//				level is always MEDIUM
//			FaceHWData:
// 				m_id: set to faceID 0
// 				m_confidence: set to scoreReco 0
// 				m_label: set to faceLabel 0
//		Two operating modes:
//		* Simple face detection: sets data if any face detected
//			This is the default operation
//			SensorHWData.vector.magnitude is set to 1.0
//		* Learned (known) face detection: sets data if a learned face 
//			is detected.   Must call LearnedFaceSet() to enable this.
//			SensorHWData.vector.magnitude is set to scoreReco (see C++ code)
//
#include <string>

#include "drivers/naoqi/common/NaoEventModuleSensorHW.h"
#include "drivers/common/vision/facedetection/FaceDetectionSensorHW.h"

class NaoFaceDetectionHW : public NaoEventModuleSensorHW, public FaceDetectionSensorHW
{
  public:
	// parameters: see NaoEventModuleSensorHW for documentation
	// IOConfig used: (all are optional)
	//	 "confidenceThreshold": Not used yet.
	//				Default is NAOFACEDETECTIONHW_CONFIDENCE_THRESH
	NaoFaceDetectionHW(boost::shared_ptr<AL::ALBroker> broker,
							const NaoEventModuleSensorHWConfig &config);

	// START: Overloading NaoEventModuleSensorHW virtual methods

    // This is called right after the module has been loaded
    virtual void init();
    // This method will be called every time the event is raised.
    virtual void onEvent(const std::string &key, 
						 const AL::ALValue &value, const AL::ALValue &message);
	// END: Overloading NaoEventModuleSensorHW virtual methods

  private:
  	float m_confidenceThreshold; // from IOConfig "confidenceThreshold"
};
