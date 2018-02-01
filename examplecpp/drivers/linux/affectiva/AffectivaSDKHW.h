//
// AffectivaSDKHW: a listener for the Affectiva SDK
// You pass in the Affectiva camera detector you want to monitor (or any
// Affectiva detector of your choice).
// This class then looks for faces and reports Affectiva emotions, expressions,
// and appearances.
// PAY ATTENTION: Must call Init and Start to run this driver
// WARNING!!! This class only supports tracking one face (only looks at the data
//		of the first face in the list from Affectiva).
// 		IMPROVE: support multiple faces some day in far future
//
#include <map>

#include "common/b2btypes.h"

#include "drivers/common/sensor/SensorHW.h"

#include "apps/common/B2BModule.h"
#include "Detector.h"
#include "FaceListener.h"
#include "ImageListener.h"
#include "ProcessStatusListener.h"

class AffectivaSDKHW : public B2BModule,
								public affdex::FaceListener, 
							    public affdex::ImageListener,
							    public affdex::ProcessStatusListener
{
  public:
  	AffectivaSDKHW(affdex::Detector *pDetector);
  	virtual ~AffectivaSDKHW();

	// START: required virtuals from B2BModule.  See that class for doc
	// Initializes this drivers, does not start detecting anything
	bool Init();

	// Starts running the Affectiva detector.   
	// Callbacks will occur after this function has completed successfully.
	bool Start(void *arg);

	// Stop the Affectiva detector
	// Callbacks will stop after this function has completed successfully.
	bool Stop(void **returnVal);
	// END: required virtual from B2BModule.  See that class for doc

	// AffEvent is value supported by Affectiva in its Face class.
	// All AffEvent's in Affectiva come with a value.
	// These events come in groups:
	// * other (OTH).  These depend on the event, see documenation below.
	// * expressions (EXP).  These can be 0 to 100
	// * emotions (EMO).  These can be 0 to 100
	// * appearances (APP).  Each one is an enum (see comments below)
	// We don't currently support events based on other values in Face class
	// but they could be added here if desired (for example, emojis, etc)
	typedef enum {
		AFFEVENT_OTH_FACE = 0, // any face detected.  value is boolean (0 or 1)

		AFFEVENT_EXP_JOY,
		AFFEVENT_EXP_FEAR,
		AFFEVENT_EXP_DISGUST,
		AFFEVENT_EXP_SADNESS,
		AFFEVENT_EXP_ANGER,
		AFFEVENT_EXP_SURPRISE,
		AFFEVENT_EXP_CONTEMPT,
		AFFEVENT_EXP_VALENCE,
		AFFEVENT_EXP_ENGAGEMENT,

		AFFEVENT_EMO_SMILE,
		AFFEVENT_EMO_INNER_BROW_RAISE,
		AFFEVENT_EMO_BROW_RAISE,
		AFFEVENT_EMO_BROW_FURROW,
		AFFEVENT_EMO_NOSE_WRINKLE,
		AFFEVENT_EMO_UPPER_LIP_RAISE,
		AFFEVENT_EMO_LIP_CORNER_DEPRESSOR,
		AFFEVENT_EMO_CHIN_RAISE,
		AFFEVENT_EMO_LIP_PUCKER,
		AFFEVENT_EMO_LIP_PRESS,
		AFFEVENT_EMO_LIP_SUCK,
		AFFEVENT_EMO_MOUTH_OPEN,
		AFFEVENT_EMO_SMIRK,
		AFFEVENT_EMO_EYE_CLOSURE,
		AFFEVENT_EMO_ATTENTION,
		AFFEVENT_EMO_EYE_WIDEN,
		AFFEVENT_EMO_CHEEK_RAISE,
		AFFEVENT_EMO_LID_TIGHTEN,
		AFFEVENT_EMO_DIMPLER,
		AFFEVENT_EMO_LIP_STRETCH,
		AFFEVENT_EMO_JAW_DROP,

		AFFEVENT_APP_GENDER,	// value is enum, see Face::Gender
		AFFEVENT_APP_GLASSES,	// value is enum, see Face::Glasses
		AFFEVENT_APP_AGE, 	  	// value is enum, see Face::Age
		AFFEVENT_APP_ETHNICITY, // value is enum, see Face::Ethnicity
		AFFEVENT_TOTAL,	// size of enum (never used as a valid value)

		// Mark START/END of EXP group (9 total)
		AFFEVENT_EXP_START = AFFEVENT_EXP_JOY,
		AFFEVENT_EXP_END = AFFEVENT_EXP_ENGAGEMENT,

		// Mark START/END of EMO group (21 total)
		AFFEVENT_EMO_START = AFFEVENT_EMO_SMILE,
		AFFEVENT_EMO_END = AFFEVENT_EMO_JAW_DROP,

		// Mark START/END of APP group (4 total)
		AFFEVENT_APP_START = AFFEVENT_APP_GENDER,
		AFFEVENT_APP_END = AFFEVENT_APP_ETHNICITY,

		// Group that is defined with float values, all others are integer
		AFFEVENT_FLOAT_START = AFFEVENT_EXP_START,
		AFFEVENT_FLOAT_END = AFFEVENT_EMO_END
	} AffEvent;

	// Used as the value of an event
	// Notice that this an integer.  
	// We don't need float for event values because we don't need more
	// precision than 0 to 100 for emotions and expressions.  And appearances
	// are all enums (integers).
	typedef int16_t AffEventValue;  

	// struct-like class for event data
	class AffEventData {
	  public:
	    AffEventData() :
			m_event(AFFEVENT_TOTAL), m_value(0), m_brightness(0),
			m_timestamp(0) {}
	    AffEventData(AffEvent event, AffEventValue value, float brightness,
					 b2b::Timestamp timestamp) :
			m_event(event), m_value(value), m_brightness(brightness),
			m_timestamp(timestamp) {}

		AffEvent m_event;		  // The event
		AffEventValue m_value;	  // The value of the event
		float m_brightness;	  	  // From Face::faceQuality.brightness
								  // This is brightness reported when this event
								  // occurred.
		b2b::Timestamp m_timestamp; // When data was read, 0 if data is invalid
	};

	// EventValueList describes events and event values.
	// If tracking and recording an event:
	//   AffEvent: the event we want to track
	//   AffEventValue: two meanings.
	//      * For most events, this is the threshold for declaring that the
	//		  event occurred.  Value of event must be >= this threshold
	//		  to be recorded.
	//		* For events in appearances group (APP) an event has occurred if
	//		  value is exactly equal to threshold.
	// 
	//   Examples:
	//   * an entry of <AFFEVENT_EXP_JOY, 60> will declare that the
	//     event JOY occurred if its value is >= 60.
	//   * an entry of <AFFEVENT_EXP_, 60> will declare that the
	//     event JOY occurred if its value is >= 60.
	//
	// If describing an event that has occurred:
	//   AffEvent: the event
	//   AffEventValue: the event's value from Affectiva Face class
	//
	typedef std::map<AffEvent, AffEventValue> EventValueList;

	// Enable/disable tracking a event or events
	// enable: 
	//	  If true, enables tracking all events in eventValueList.
	// 	  	This method adds to list of tracked events.  
	//		Thus, you can call EventEnable multiple times or just once.
	//		If an event has already been enabled, its entry is overwritten
	//		by latest EventEnable() call.
	//	  If false, disable tracking all events in eventValueList.
	// RETURNS: true on success, false otherwise.
	//		Disabling an event that was not enabled is not an error.
	//			It silently does nothing in that scenario.
	bool EventEnable(bool enable, const EventValueList &eventValueList);
	// Helper routine: Same as above but used to track one event
	bool EventEnable(bool enable, AffEvent event, AffEventValue value)
	{ 
		EventValueList eventValueList;
		eventValueList[event] = value;
		return EventEnable(enable, eventValueList);
	}
	// Helper routine: Same as above but uses string eventName
	bool EventEnable(bool enable, const char *eventName, AffEventValue value)
	{
		AffEvent event = StringToAffEvent(eventName);
		if (event == AFFEVENT_TOTAL)
			return false;
		return EventEnable(enable, event, value);
	}

	// List of events and corresponding data *with timestamps*
	typedef std::map<AffEvent, AffEventData> EventDataList;

	// Event data that we store from Affectiva
	// m_eventDataList: list of event data.
	class AffectivaEventHWData {
	  public:
	  	AffectivaEventHWData() {}

		// RETURNS: true if AffectivaEventHWData is valid, false otherwise
		bool Valid() const { return m_eventDataList.size() > 0; }

		EventDataList m_eventDataList;
	};

	// Same documention as SensorHWGetLatest but returns AffectivaEventHWData
	// eventList: list of events we are interested in
	// pAffectivaHWData: Set to event data that has been recorded.  Only events
	//		in eventList are stored here.   These are events that have occurred
	//		since EventEnabled was called for this event.  Caller must check
	//		timestamps itself.
	bool AffectivaEventsGetLatest(const std::vector<AffEvent> &eventList,
							AffectivaEventHWData *pAffectivaHWData,
							SensorHW::SensorHWStatus *pStatus=NULL) const;
	// Helper routine: Same as above but used to track one event
	bool AffectivaEventsGetLatest(AffEvent event,
							AffectivaEventHWData *pAffectivaHWData,
							SensorHW::SensorHWStatus *pStatus=NULL) const
	{
		std::vector<AffEvent> eventList;
		eventList.push_back(event);
		return AffectivaEventsGetLatest(eventList, pAffectivaHWData, pStatus);
	}
	// Helper routine: Same as above but uses string eventName
	bool AffectivaEventsGetLatest(const char *eventName,
							AffectivaEventHWData *pAffectivaHWData,
							SensorHW::SensorHWStatus *pStatus=NULL) const
	{
		AffEvent event = StringToAffEvent(eventName);
		if (event == AFFEVENT_TOTAL)
			return false;
		return AffectivaEventsGetLatest(event, pAffectivaHWData, pStatus);
	}

	// RETURNS: true if onProcessingFinished() has been called, false otherwise
	bool OnProcessingFinishedCalled() { return m_onProcessingFinishedCalled; }

	// RETURNS: AffEvent that matches eventName.  Lower or upper case agnostic.
	//			Returns AFFEVENT_TOTAL if eventName is invalid.
	static AffEvent StringToAffEvent(const char *eventName);
	// RETURNS: string for event.   Returns "Unknown" if event is invalid.
	static const char * AffEventToString(AffEvent event);

  private:
  	// START: Required FaceListener methods.  See that class for doc
	// Indicates that the face detector has started tracking a new face.
	// When the face tracker detects a face for the first time method is called.
	// The receiver should expect that tracking continues until detection
	// has stopped. 
	//
	// timestamp: Frame timestamp when new face was first observed.
	// faceId: Face identified.
	void onFaceFound(float timestamp, affdex::FaceId faceId);

	// Indicates that the face detector has stopped tracking a face.  
	// When the face tracker no longer finds a face this method is called.
	// The receiver should expect that there is no face tracking until the
	// detector is started. 
	//
	// timestamp: Frame timestamp when previously observed face is no longer
	//				present.
	// faceId: Face identified.
	void onFaceLost(float timestamp, affdex::FaceId faceId);
  	// END: Required FaceListener methods

  	// START: Required ImageListener methods.  See that class for doc
	// Callback providing results for a processed frame.
	// The current interface allows for multiple faces to be processed. 
	//
	// faces: A dictionary of Face objects containing metrics about each face
	//			found in the image.
	// image: The Frame that was processed. 
	void onImageResults(std::map<affdex::FaceId, 
						affdex::Face> faces, affdex::Frame image);

	// Callback for every input Frame.
	// In cases where the processing framerate is lower than the input
	// framerate, this method will all captured frames, including those that
	// aren't processed. This can be useful for the CameraDetector
	// or VideoDetector when used with a low processing framerate. 
	//
	// image: The Frame captured. 
	void onImageCapture(affdex::Frame image);
  	// END: Required ImageListener methods

  	// START: Required ProcessStatusListener methods.  See that class for doc
	// Indicates that the face detector has failed with an exception. 
	// ex: the exception encountered during processing
  	void onProcessingException(affdex::AffdexException ex);

	// Indicates that the face detector has processed the video.
	// When the face tracker completed processing of the last frame,
	// this method is called.   
	// FYI: This will only get called when processing images from a file.
	void onProcessingFinished();
  	// END: Required ProcessStatusListener methods


  	static const char * const m_affEventStrings[AFFEVENT_TOTAL];

  	affdex::Detector *m_pDetector; // from ctor

	// true if onProcessingFinished has been called, false otherwise
	bool m_onProcessingFinishedCalled;

	// For protecting m_eventValueEnabledList and m_eventDataList
	mutable pthread_mutex_t m_lockAffHW;

	// List of events being tracked (enabled by EventEnable())
	EventValueList m_eventValueEnabledList;
	
	// Holding area for events that have occurred that are being tracked
	// in m_eventValueEnabledList
	EventDataList m_eventDataList;

	// DEBUG ONLY: Turns on/off printf!  Useful for test menus and debugging.
	// If true, printf when callbacks are called and printf any callback data.
	// If false, no printfs.
	bool m_debugCallbacks; 
	void DebugCallbacksEnable(bool enable) { m_debugCallbacks = enable; }

	// tests need access to privates
	friend class MomentaCreatureMenu;
};
