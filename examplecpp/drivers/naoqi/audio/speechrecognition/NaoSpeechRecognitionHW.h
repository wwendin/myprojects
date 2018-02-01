#pragma once
// 
//		NAO speech recognition HW driver
//			SensorHWData.vector is set to all 0s
//			SensorHWData.vector.magnitude is set to confidence level
//			SensorHWData.level is always MEDIUM
//
//		WARNING!!! Must call InitSpeechRecognition() once before creating
//			*any* instance of this driver.  WARNING!!!
//
#include <string>

#include <boost/shared_ptr.hpp>
#include <alcommon/almodule.h>

#include "drivers/naoqi/common/NaoEventModuleSensorHW.h"

class NaoSpeechRecognitionHW : public NaoEventModuleSensorHW
{
  public:
	// 
	// IOConfig values used:
  	//   "confidenceThreshold": used to compare to confidence value that
	//			arrives in onEvent().  confidence value must be
	//			>= confidenceThreshold to consider the word "recognized"
  	//   "words": words that *this instance* will respond to.
	//			words *not* in this list will be ignored by onEvent()
	// 
  	NaoSpeechRecognitionHW(boost::shared_ptr<AL::ALBroker> broker,
		const NaoEventModuleSensorHW::NaoEventModuleSensorHWConfig &config);
  	virtual ~NaoSpeechRecognitionHW();

	// Only one instance of ALSpeechRecognition is running at a time on NAO.   
	// Therefore, only call this once!
	// wordList: passed to setVocabulary()   This must be a list of *all
	//		words* in *all instances* of NaoSpeechRecognitionHW.
	// enableWordSpotting: Passed to setVocabulary()
	//
	// NOTE: If you call this multiple times, the last wordList will be the
	//			word list.
	//
	// RETURNS: true on success, false otherwise
	typedef std::vector<std::string> WordList;
  	static bool InitSpeechRecognition(boost::shared_ptr<AL::ALBroker> broker,
						const WordList &wordList, bool enableWordSpotting);

	// Shutdown instance of ALSpeechRecognition (uses pause())
	// This is automatically called by our destructor
	static bool ShutdownSpeechRecognition(
									boost::shared_ptr<AL::ALBroker> broker);

	// START: Overloading NaoEventModuleSensorHW
  	virtual void init();
    virtual void onEvent(const std::string &key, 
						 const AL::ALValue &value, const AL::ALValue &message);
	// END: Overloading NaoEventModuleSensorHW

	// Additional data that we store
	// m_textString: the text we think we heard
	// m_confidence: confidence level of m_textString being correct.
	class SpeechHWData {
	  public:
	  	SpeechHWData() : m_confidence(0) {}
		std::string m_textString;
		float m_confidence;
	};

	// Same as SensorHWGetLatest but adds SpeechHWData
	bool SpeechRecognitionGetLatest(SensorHWData *pData,
							SpeechHWData *pSpeechHWData,
							SensorHWStatus *pStatus=NULL, bool raw=false);

	//Optional GetLatest that returns different data based on what level is wanted,
	//	only if that level was updated recently (according to overload).
	//If this is not overloaded, it will return SensorHWGetLatest by default.
	virtual bool SpeechRecognitionGetLatestOfLevel(B2BLogic::Level level, b2b::Timestamp since, SensorHWData *pData, SpeechHWData *pSpeechHWData, 
			SensorHWStatus *pStatus=NULL, bool raw=false)
	{return SpeechRecognitionGetLatest(pData, pSpeechHWData, pStatus, raw);}


private:
  	float m_confidenceThreshold; // from IOConfig "confidenceThreshold"
	WordList m_wordList;		 // from IOConfig "words"

	SpeechHWData m_speechHWData;
};
