#include <stdio.h>

#include "log/B2BLog.h"

#include "AffectivaSDKHW.h"

AffectivaSDKHW::AffectivaSDKHW(affdex::Detector *pDetector) :
	B2BModule("AffectivaSDKHW"),
	m_pDetector(pDetector),
	m_onProcessingFinishedCalled(false),
	m_debugCallbacks(false)
{
	pthread_mutexattr_t mutexattr;
	pthread_mutexattr_init(&mutexattr);
	pthread_mutexattr_settype(&mutexattr, PTHREAD_MUTEX_RECURSIVE);
	pthread_mutex_init(&m_lockAffHW, &mutexattr);
}

AffectivaSDKHW::~AffectivaSDKHW()
{
	pthread_mutex_destroy(&m_lockAffHW);
}

bool AffectivaSDKHW::Init()
{ 
	if (!m_pDetector) 
		return false; // FAIL

	m_pDetector->setClassifierPath("/opt/affdexsdk/data");

	// IMPROVE: we could only detect those events enabled by EnableEvent()
	//		This makes the Affectiva SDK more efficient (and use only the CPU
	//		required to detect events we want to detect).
	//		However, for now, we'll keep things simple and just detect
	//		everything.   We'll improve this later.

	// Detect smile and joy classifiers
	//m_pDetector->setDetectSmile(true);
	//m_pDetector->setDetectJoy(true);
	
	// To turn on all expressions, emotions, emojies, appearances
	m_pDetector->setDetectAllExpressions(true);
	m_pDetector->setDetectAllEmotions(true);
	//m_pDetector->setDetectAllEmojis(true);  don't want to detect emojis
	m_pDetector->setDetectAllAppearances(true);

	// The FaceListener is a client callback interface which sends
	// notification when the detector has started or stopped tracking a
	// face.  Call setFaceListener to set the FaceListener.
    m_pDetector->setFaceListener(this);

	// The ImageListener is a client callback interface which delivers 
	// information about an image which has been handled by the Detector.
	// Call setImageListener to set the ImageListene
    m_pDetector->setImageListener(this);

	// The ProcessStatusListener is a callback interface which provides
	// information regarding the processing state of the detector.
	// Call setProcessStatusListener to set the ProcessStatusListener
    m_pDetector->setProcessStatusListener(this);

	return true;
}

bool AffectivaSDKHW::Start(void *arg)
{ 
	if (!m_pDetector) 
		return false; // FAIL

	// Init the detector
	m_pDetector->start();

	return true;
}

bool AffectivaSDKHW::Stop(void **returnVal)
{
	if (!m_pDetector) 
		return false; // FAIL

	// stop the detector
	m_pDetector->stop();

	// FYI: You can reset the processing state by calling detector.reset()
	// Face IDs and timestamps are set to 0 when reset is called.

	return true;
}

void AffectivaSDKHW::onFaceFound(float timestamp, affdex::FaceId faceId)
{
	if (m_debugCallbacks)
		printf("onFaceFound(%f,%d)\n", timestamp, (int)faceId);
}

void AffectivaSDKHW::onFaceLost(float timestamp, affdex::FaceId faceId)
{
	if (m_debugCallbacks) printf("onFaceLost(%f,%d)\n", timestamp, (int)faceId);
}

void AffectivaSDKHW::onImageResults(std::map<affdex::FaceId,
						affdex::Face> faces, affdex::Frame image)
{
  if (m_debugCallbacks) {
	printf("%f: onImageResults\n", image.getTimestamp());

	// Output contents of faces
	std::map<affdex::FaceId, affdex::Face>::const_iterator iter;
	int count = 0;
	for (iter = faces.begin(); iter != faces.end(); ++iter) {
		const affdex::FaceId faceId = iter->first;
		const affdex::Face &face = iter->second;
		printf("%d: id: %d(%d)\n", count, (int)faceId, (int)(iter->second.id));
		printf("  Emotions: joy: %f fear: %f disgust: %f sadness: %f anger: %f surprise: %f contempt: %f valence: %f engagement: %f\n",
										face.emotions.joy,
										face.emotions.fear,
        								face.emotions.disgust,
        								face.emotions.sadness,
        								face.emotions.anger,
        								face.emotions.surprise,
        								face.emotions.contempt,
        								face.emotions.valence,
        								face.emotions.engagement);

		printf("  Expressions: smile: %f innerBrowRaise: %f browRaise: %f browFurrow: %f noseWrinkle: %f upperLipRaise: %f lipCornerDepressor: %f chinRaise: %f lipPucker: %f lipPress: %f lipSuck: %f mouthOpen: %f smirk: %f eyeClosure: %f attention: %f eyeWiden: %f cheekRaise: %f lidTighten: %f dimpler: %f lipStretch: %f jawDrop: %f\n", 
										face.expressions.smile, 
										face.expressions.innerBrowRaise,
        								face.expressions.browRaise,
        								face.expressions.browFurrow,
        								face.expressions.noseWrinkle,
        								face.expressions.upperLipRaise,
        								face.expressions.lipCornerDepressor,
        								face.expressions.chinRaise,
        								face.expressions.lipPucker,
        								face.expressions.lipPress,
        								face.expressions.lipSuck,
        								face.expressions.mouthOpen,
        								face.expressions.smirk,
        								face.expressions.eyeClosure,
        								face.expressions.attention,
        								face.expressions.eyeWiden,
        								face.expressions.cheekRaise,
        								face.expressions.lidTighten,
        								face.expressions.dimpler,
        								face.expressions.lipStretch,
        								face.expressions.jawDrop);
		printf("  Measurements: interocularDistance: %f pitch: %f yaw: %f roll: %f\n", 
        								face.measurements.interocularDistance,
        								face.measurements.orientation.pitch,
        								face.measurements.orientation.yaw,
        								face.measurements.orientation.roll);
		printf("  Appearance: gender: %d glasses: \"%s\" age: %d ethnicity: %d\n", 
					(int)face.appearance.gender,
					(face.appearance.glasses == affdex::Glasses::Yes) ?
							"yes" : "no",
					(int)face.appearance.age,
					(int)face.appearance.ethnicity);
		//printf("  Emojis\n", face.emojis); 
		//printf("  ...\n", face.featurePoints); 
		printf("  FaceQuality: brightness: %f\n",
										face.faceQuality.brightness); 
	}
  } // else: don't printf

  if (faces.size() > 0) {
	// Only look at first face in faces map
	// IMPROVE: support multiple faces some day in far future
	std::map<affdex::FaceId, affdex::Face>::const_iterator facesIter = faces.begin();
	const affdex::Face &face = facesIter->second;
    bool changedEventDataList = false; // true if changed m_eventDataList

	pthread_mutex_lock(&m_lockAffHW); // protect m_eventValueEnabledList/m_eventDataList
    EventValueList::const_iterator iter;
    for (iter = m_eventValueEnabledList.begin(); 
    	 iter != m_eventValueEnabledList.end(); ++iter)
    {
	  AffEvent event = iter->first;
	  AffEventValue threshold = iter->second;
  
	  const int * pIntValue = NULL; // For Face enums
	  const float * pFloatValue = NULL; // For Face floats
	  int faceExists = 1; // if we got here, a face exists.  We set to 1.
  	  switch (event) {
	  	case AFFEVENT_OTH_FACE:
	  	  pIntValue = &faceExists; // Special case
		  break;
	  	case AFFEVENT_EXP_JOY:
	  	  pFloatValue = &face.emotions.joy;
		  break;
	  	case AFFEVENT_EXP_FEAR:
		  pFloatValue = &face.emotions.fear;
		  break;
	  	case AFFEVENT_EXP_DISGUST:
          pFloatValue = &face.emotions.disgust;
		  break;
	  	case AFFEVENT_EXP_SADNESS:
          pFloatValue = &face.emotions.sadness;
		  break;
	  	case AFFEVENT_EXP_ANGER:
          pFloatValue = &face.emotions.anger;
		  break;
	  	case AFFEVENT_EXP_SURPRISE:
          pFloatValue = &face.emotions.surprise;
		  break;
	  	case AFFEVENT_EXP_CONTEMPT:
          pFloatValue = &face.emotions.contempt;
		  break;
	  	case AFFEVENT_EXP_VALENCE:
          pFloatValue = &face.emotions.valence;
		  break;
	  	case AFFEVENT_EXP_ENGAGEMENT:
          pFloatValue = &face.emotions.engagement;
		  break;
	  	case AFFEVENT_EMO_SMILE:
	  	  pFloatValue = &face.expressions.smile;
		  break;
	  	case AFFEVENT_EMO_INNER_BROW_RAISE:
		  pFloatValue = &face.expressions.innerBrowRaise;
		  break;
	  	case AFFEVENT_EMO_BROW_RAISE:
          pFloatValue = &face.expressions.browRaise;
		  break;
	  	case AFFEVENT_EMO_BROW_FURROW:
          pFloatValue = &face.expressions.browFurrow;
		  break;
	  	case AFFEVENT_EMO_NOSE_WRINKLE:
          pFloatValue = &face.expressions.noseWrinkle;
		  break;
	  	case AFFEVENT_EMO_UPPER_LIP_RAISE:
          pFloatValue = &face.expressions.upperLipRaise;
		  break;
	  	case AFFEVENT_EMO_LIP_CORNER_DEPRESSOR:
          pFloatValue = &face.expressions.lipCornerDepressor;
		  break;
	  	case AFFEVENT_EMO_CHIN_RAISE:
          pFloatValue = &face.expressions.chinRaise;
		  break;
	  	case AFFEVENT_EMO_LIP_PUCKER:
          pFloatValue = &face.expressions.lipPucker;
		  break;
	  	case AFFEVENT_EMO_LIP_PRESS:
          pFloatValue = &face.expressions.lipPress;
		  break;
	  	case AFFEVENT_EMO_LIP_SUCK:
          pFloatValue = &face.expressions.lipSuck;
		  break;
	  	case AFFEVENT_EMO_MOUTH_OPEN:
          pFloatValue = &face.expressions.mouthOpen;
		  break;
	  	case AFFEVENT_EMO_SMIRK:
          pFloatValue = &face.expressions.smirk;
		  break;
	  	case AFFEVENT_EMO_EYE_CLOSURE:
          pFloatValue = &face.expressions.eyeClosure;
		  break;
	  	case AFFEVENT_EMO_ATTENTION:
          pFloatValue = &face.expressions.attention;
		  break;
	  	case AFFEVENT_EMO_EYE_WIDEN:
          pFloatValue = &face.expressions.eyeWiden;
		  break;
	  	case AFFEVENT_EMO_CHEEK_RAISE:
          pFloatValue = &face.expressions.cheekRaise;
		  break;
	  	case AFFEVENT_EMO_LID_TIGHTEN:
          pFloatValue = &face.expressions.lidTighten;
		  break;
	  	case AFFEVENT_EMO_DIMPLER:
          pFloatValue = &face.expressions.dimpler;
		  break;
	  	case AFFEVENT_EMO_LIP_STRETCH:
          pFloatValue = &face.expressions.lipStretch;
		  break;
	  	case AFFEVENT_EMO_JAW_DROP:
          pFloatValue = &face.expressions.jawDrop;
		  break;
	  	case AFFEVENT_APP_GENDER:
		  pIntValue = (const int *)(&face.appearance.gender);
		  break;
	  	case AFFEVENT_APP_GLASSES:
	  	  pIntValue = (const int *)(&face.appearance.glasses);
		  break;
	  	case AFFEVENT_APP_AGE:
		  pIntValue = (const int *)(&face.appearance.age);
		  break;
	  	case AFFEVENT_APP_ETHNICITY:
		  pIntValue = (const int *)(&face.appearance.ethnicity);
	  	  break;
	  	default:
		  B2BLog::Err(LogFilt::LM_DRIVERS, "Event %d does not exist!  Ignored");
		  break;
	  }

	  float brightness = face.faceQuality.brightness; 
	  b2b::Timestamp timestamp = B2BTime::GetCurrentB2BTimestamp();
	  if ((event >= AFFEVENT_FLOAT_START) && (event <= AFFEVENT_FLOAT_END)) {
		// Events with float values
		if (*pFloatValue >= threshold) {
			AffEventData data(event, *pFloatValue, brightness, timestamp);
			m_eventDataList[event] = data;
  			changedEventDataList = true;
		} // else: do nothing, event occurred but didn't *break* threshold
	  } else {
		// Events with integer values
		if (*pIntValue == threshold) {
			AffEventData data(event, *pIntValue, brightness, timestamp);
			m_eventDataList[event] = data;
  			changedEventDataList = true;
		} // else: do nothing, event occurred but didn't *equal* threshold
	  }
    }

    if (changedEventDataList) {
		// Altered m_eventDataList
	}
	pthread_mutex_unlock(&m_lockAffHW);

  } // else: no faces detected
}

void AffectivaSDKHW::onImageCapture(affdex::Frame image)
{
  	if (m_debugCallbacks) printf("%f: onImageCapture\n", image.getTimestamp());
}

void AffectivaSDKHW::onProcessingException(affdex::AffdexException ex)
{
  	if (m_debugCallbacks) 
		printf("Error: %s\n", ex.getExceptionMessage().c_str());
}

void AffectivaSDKHW::onProcessingFinished()
{
  	if (m_debugCallbacks) printf("onProcessingFinished\n");

	m_onProcessingFinishedCalled = true;
}

bool AffectivaSDKHW::EventEnable(bool enable,
								 const EventValueList &eventValueList)
{
	const char *enableString = enable ? "true" : "false";

	pthread_mutex_lock(&m_lockAffHW); // protect m_eventValueEnabledList
	EventValueList::const_iterator iter;
	for (iter = eventValueList.begin(); iter != eventValueList.end(); ++iter) {
	  	AffEvent event = static_cast<AffEvent>(iter->first);
		AffEventValue threshold = iter->second;
		B2BLog::Debug(LogFilt::LM_DRIVERS, "%s: %d %d", enableString,
													event, threshold);
		if (enable) {
			m_eventValueEnabledList[event] = threshold; // add (or replace)
		} else {
			EventValueList::const_iterator existingIter;
			existingIter = m_eventValueEnabledList.find(event);
			if (existingIter != m_eventValueEnabledList.end()) {
				m_eventValueEnabledList.erase(existingIter);
			} else {
				B2BLog::Warn(LogFilt::LM_DRIVERS, 
					"Disabling an event, %d(thresh: %d), that was not enabled",
					event, threshold);
			}
		}
	}
	pthread_mutex_unlock(&m_lockAffHW);

	return true;
}

bool AffectivaSDKHW::AffectivaEventsGetLatest(
							const std::vector<AffEvent> &eventList,
							AffectivaEventHWData *pAffectivaHWData,
							SensorHW::SensorHWStatus *pStatus) const
{
	bool result = true; // assume SUCCESS

	// true if any event requested in eventList exists in m_eventDataList
	bool atLeastOneEventDataFound = false; 

	pthread_mutex_lock(&m_lockAffHW); // protect m_eventDataList
	if ((m_eventDataList.size() > 0) && (eventList.size() > 0)) {
	  // Look through eventList, one event at a time
      std::vector<AffEvent>::const_iterator iter;
      for (iter = eventList.begin(); iter != eventList.end(); ++iter) {
	  	AffEvent event = *iter;
      	EventDataList::const_iterator foundIter = m_eventDataList.find(event);
		if (foundIter != m_eventDataList.end()) {
			// SUCCESS: Event data found, return it
	  		atLeastOneEventDataFound = true;
	  		pAffectivaHWData->m_eventDataList.clear();
	  		pAffectivaHWData->m_eventDataList[event] = foundIter->second;
		} // else: event not found in m_eventDataList
	  }
	} // else: no event data recorded or eventList is empty
	pthread_mutex_unlock(&m_lockAffHW);

	if (atLeastOneEventDataFound) {
	  	if (pStatus) *pStatus = SensorHW::HWSTATUS_OK;
	} else {
		// FAIL: no data for any event in eventList or eventList is empty
	  	result = false; 
	  	if (pStatus) *pStatus = SensorHW::HWSTATUS_NODATA;
	}

	return result;
}

/*static*/ const char *AffectivaSDKHW::AffEventToString(AffEvent event)
{
	if (event < AFFEVENT_TOTAL)
		return m_affEventStrings[event]; // SUCCESS: return string
	else
		return "Unknown"; // FAIL: level is invalid
}

/*static*/ AffectivaSDKHW::AffEvent AffectivaSDKHW::StringToAffEvent(
														const char *eventName)
{
	  for (int event = 0; event < AFFEVENT_TOTAL; ++event) {
		if (strcasecmp(m_affEventStrings[event], eventName) == 0)
			return static_cast<AffEvent>(event); // SUCCESS: found a match
	  }
	  return AFFEVENT_TOTAL; // FAIL: eventName is invalid
}

/*static*/ const char * const AffectivaSDKHW::m_affEventStrings[AffectivaSDKHW::AFFEVENT_TOTAL] =
{
	"FACE",
	"JOY", "FEAR", "DISGUST", "SADNESS", "ANGER", "SURPRISE", "CONTEMPT", "VALENCE", "ENGAGEMENT",
	"SMILE", "INNER_BROW_RAISE", "BROW_RAISE", "BROW_FURROW", "NOSE_WRINKLE", "UPPER_LIP_RAISE", "LIP_CORNER_DEPRESSOR", "CHIN_RAISE", "LIP_PUCKER", "LIP_PRESS", "LIP_SUCK", "MOUTH_OPEN", "SMIRK", "EYE_CLOSURE", "ATTENTION", "EYE_WIDEN", "CHEEK_RAISE", "LID_TIGHTEN", "DIMPLER", "LIP_STRETCH", "JAW_DROP",
	"GENDER", "GLASSES", "AGE", "ETHNICITY"
};
