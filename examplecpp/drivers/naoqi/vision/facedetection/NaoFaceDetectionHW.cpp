#include "common/B2BTime.h"
#include "log/B2BLog.h"

#include <alvalue/alvalue.h>
#include <alproxies/alfacedetectionproxy.h>
//#include <alproxies/altexttospeechproxy.h>

#include "NaoFaceDetectionHW.h"

// default values if there is nothing in ioconfig 
#define NAOFACEDETECTIONHW_CONFIDENCE_THRESH 		0.4

NaoFaceDetectionHW::NaoFaceDetectionHW(boost::shared_ptr<AL::ALBroker> broker,
							const NaoEventModuleSensorHWConfig &config) :
		SensorHW(config.m_ioConfig, config.m_ioConfigEntryNames),
		NaoEventModuleSensorHW(broker, config),
		FaceDetectionSensorHW(config.m_ioConfig, config.m_ioConfigEntryNames),
		m_confidenceThreshold(NAOFACEDETECTIONHW_CONFIDENCE_THRESH)
{
	if (!config.m_ioConfig.GetExtraSettingsValue<float>(
				config.m_ioConfigEntryNames[0], "confidenceThreshold",
				&m_confidenceThreshold))
	{
	  B2BLog::Err(LogFilt::LM_DRIVERS,
		"%s: IOConfig %s confidenceThreshold is not defined.  Using default %.1g",
			  getName().c_str(), config.m_ioConfigEntryNames[0],
			  m_confidenceThreshold);
	}

	B2BLog::Debug(LogFilt::LM_DRIVERS, "%s: confidenceThreshold: %.1g",
			  				getName().c_str(), m_confidenceThreshold);
}

void NaoFaceDetectionHW::init()
{
  	if (!IsEnabled())
  		return; // We're not enabled, don't init anything

  	try {
  	  AL::ALFaceDetectionProxy face(getParentBroker()); 

	  AL::ALValue list = face.getLearnedFacesList();
	  B2BLog::Info(LogFilt::LM_DRIVERS, "list: %s", list.toString().c_str());

  	} catch (const AL::ALError& e) {
	  B2BLog::Err(LogFilt::LM_DRIVERS, 
				"ALFaceDetectionProxy instantiation failed %s.", e.what());
	  return; // FAIL
  	}

	NaoEventModuleSensorHW::init(); // finish up	
}

void NaoFaceDetectionHW::onEvent(const std::string &key, 
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
  if (value.getSize() < 2)
  	return; // NO DATA: nothing in array, face is not "present"
			//			This happens sometimes, not sure why


  // value is 
  // [
  //   [1493862951, 379700],  // Timestamp: sec, microsec
  //   [
  //     [
  //       [0, 0.0232511, -0.114232, 0.219224, 0.228464], // ShapeInfo [0, alpha, beta, sizeX, sizeY ]
  //       [2,  // faceID
  //        0,  // scoreReco
  //        "", // faceLabel
  //        [0.0332158, -0.159232, 0.0132863, -0.148848, 0.0564669, -0.162694, 0, 0, 0, 0, 0, 0, 0, 0], // leftEyePoints
  //        [-0.0763963, -0.13154, -0.0498237, -0.13154, -0.0996474, -0.121155, 0, 0, 0, 0, 0, 0, 0, 0],  // rightEyePoints
  //        [0, 0, 0, 0, 0, 0],  // unused
  //        [0, 0, 0, 0, 0, 0],  // unused
  //        [0.0016608, -0.0830777, 0.0265726, -0.0900008, -0.0232511, -0.0761545], // nosePoints
  //        [0.0597884, -0.0415388, -0.039859, -0.0103847, 0.0132863, -0.0519235, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0] // moutPoints
  //       ] // ExtraInfo
  //     ], // FaceInfo[0]
  //     [] // Time_Filtered_Reco_Info
  //   ],
  //   [0.0651024, -0.000102598, 0.183584, 0, 0.126747, -0.00157595], //CameraPose_InTorsoFrame
  //   [0.0639957, -0.00909317, 0.413557, 0.0207612, 0.0835554, -0.000440706], //CameraPose_InRobotFrame
  //   0 // Camera_Id
  // ]
  //

  AL::ALValue faceInfoAndMoreArray = value[1]; // [faceInfo[n], Time_Filtered_Reco_Info]
  // Get first face's info (index 0)
  AL::ALValue face0 = faceInfoAndMoreArray[0]; // get first face
  AL::ALValue shapeInfo0 = face0[0]; // [0, alpha, beta, sizeX, sizeY]
  AL::ALValue extraInfo0 = face0[1]; // [faceID, scoreReco, faceLabel, ...]
  int faceID0 = extraInfo0[0];
  float scoreReco0 = extraInfo0[1];
  std::string faceLabel0 = extraInfo0[2];
  AL::ALValue timeFilteredRecoInfo = faceInfoAndMoreArray[faceInfoAndMoreArray.getSize()-1];
  B2BLog::Debug(LogFilt::LM_DRIVERS, 
  				"%s: shapeInfo0: %s faceID0: %d scoreReco0: %f faceLabel0: \"%s\" timeFilteredRecoInfo: %s",
  				getName().c_str(), 
				shapeInfo0.toString().c_str(),
				faceID0, scoreReco0, faceLabel0.c_str(),
				timeFilteredRecoInfo.toString().c_str());

  if (timeFilteredRecoInfo.getSize() > 0) {
  	// has format [number] or [number, arrayOfFaceLabels]
	int number = timeFilteredRecoInfo[0];
	switch (number) {
	  case 2:
	  	// 1 face
		{
			AL::ALValue faceArray = timeFilteredRecoInfo[1];
  			B2BLog::Debug(LogFilt::LM_DRIVERS, "%s: 1 face: %s (face id: %d \"%s\")",
  				getName().c_str(), faceArray[0].toString().c_str(),
				faceID0, faceLabel0.c_str());
		}
		break;
	  case 3:
	  	// multiple faces (2 or more)
		{
			AL::ALValue faceArray = timeFilteredRecoInfo[1];
  			B2BLog::Debug(LogFilt::LM_DRIVERS, "%s: faces: %s", 
  						getName().c_str(), faceArray.toString().c_str());
		}
		break;
	  case 4:
	  	// Just the number 4.   Face has been there for 8 seconds.
		{
  			B2BLog::Debug(LogFilt::LM_DRIVERS, "%s: face id: %d \"%s\" learning suggested",
							getName().c_str(), faceID0, faceLabel0.c_str());
		}
		break;
	  default:
  		B2BLog::Debug(LogFilt::LM_DRIVERS, "%s: impossible value %d", number);
		break;
	}
  }


	bool setData = false; // assume we do not set m_data/m_faceHWData
	std::string faceLabel = LearnedFaceGet();
	if (faceLabel.length()) {
		// Learned (known) face detection
		if (faceLabel == faceLabel0) {
			setData = true; // We have matching face; set the data
			#if 0
			// Speak the textString
  	  		AL::ALTextToSpeechProxy tts(getParentBroker());
  	  		tts.say(faceLabel0);
			#endif
		}
	} else {
		// Simple face detection.   
		setData = true; // If we got here, a face was detected; set the data
	}

	if (setData) {
  		pthread_mutex_lock(&m_lockSensorHW);
  		m_data.level = B2BLogic::LEVEL_MEDIUM;
    	m_data.vector.magnitude = 1.0;
    	// FIXME: need to repair these angles based on torso and head frames
    	m_data.vector.angle = 0;
    	m_data.vector.azimuth = 0;
  		if (faceLabel0.length() > 0) {
  			// We found a face we recognize, store it
			m_faceHWData.m_id = faceID0;
			m_faceHWData.m_confidence = scoreReco0;
			m_faceHWData.m_label = faceLabel0;
    		m_data.vector.magnitude = m_faceHWData.m_confidence;
  		}
    	m_data.timestamp = B2BTime::GetCurrentB2BTimestamp();
  		pthread_mutex_unlock(&m_lockSensorHW);
	}
}
