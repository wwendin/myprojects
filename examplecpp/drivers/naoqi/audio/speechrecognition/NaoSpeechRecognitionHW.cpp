#include <algorithm>

#include "common/B2BLogic.h"
#include "common/B2BTime.h"
#include "common/StringUtil.h"

#include <alvalue/alvalue.h>
#include <alproxies/alspeechrecognitionproxy.h>

#include "NaoSpeechRecognitionHW.h"

// default values if there is nothing in ioconfig 
#define NAOSPEECHHW_CONFIDENCE_THRESH 		0.40

NaoSpeechRecognitionHW::NaoSpeechRecognitionHW(
		boost::shared_ptr<AL::ALBroker> broker,
		const NaoEventModuleSensorHW::NaoEventModuleSensorHWConfig &config) :
		SensorHW(config.m_ioConfig, config.m_ioConfigEntryNames),
		NaoEventModuleSensorHW(broker, config),
		m_confidenceThreshold(NAOSPEECHHW_CONFIDENCE_THRESH)
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

	std::string words;
	if (config.m_ioConfig.GetExtraSettingsValue<std::string>(
				config.m_ioConfigEntryNames[0], "words", &words))
	{
	  m_wordList = StringUtil::StringParse(words, ',');
	} else {
	  B2BLog::Err(LogFilt::LM_DRIVERS,
			  "%s: no IOConfig \"words\" defined.  Must have at least one word defined to use this driver.   Driver init FAILED!!!",
			  getName().c_str());
	  SetEnabled(false);
	  return; // FAIL
	}

	B2BLog::Debug(LogFilt::LM_DRIVERS,
				"%s: confidenceThreshold: %.1g words: %s", getName().c_str(), 
				m_confidenceThreshold,
				StringUtil::VectorOfStringsToString(m_wordList, ',').c_str());
}

NaoSpeechRecognitionHW::~NaoSpeechRecognitionHW()
{

  	EventNames::const_iterator iter;
  	for (iter = m_eventNames.begin(); iter != m_eventNames.end(); ++iter) {
  		//B2BLog::Info(LogFilt::LM_DRIVERS, 
  		//	"Subscribe \"%s\"::onEvent to listen to event \"%s\"", 
  		//	 getName().c_str(), iter->c_str());
    	fMemoryProxy.unsubscribeToEvent(iter->c_str(), "onEvent");
	}

	ShutdownSpeechRecognition(getParentBroker());

  	/// Setting up a proxy to ALSpeechRecognition
  	AL::ALSpeechRecognitionProxy speechRecognition(getParentBroker());

  	speechRecognition.unsubscribe(getName());

	// FIXME: shutdown animation behaviors.  How do we do this?

  //printf("delete NaoSpeechRecognitionHW");
}

/*static*/ bool NaoSpeechRecognitionHW::InitSpeechRecognition(
									boost::shared_ptr<AL::ALBroker> broker,
									const WordList &wordList, 
									bool enableWordSpotting)
{
  try {
  	/// Setting up a proxy to ALSpeechRecognition
  	AL::ALSpeechRecognitionProxy speechRecognition(broker);

  	/// Setting the working language of speech recognition engine
  	speechRecognition.setLanguage("English");
  	speechRecognition.setVisualExpressionMode(0); // turn all animation off

  	speechRecognition.pause(true); // stop ALSpeechRecognition

  	/// Setting the word list that should be recognized
  	speechRecognition.setVocabulary(wordList, enableWordSpotting);

  	speechRecognition.pause(false); // restart ALSpeechRecognition

  } catch (const AL::ALError& e) {
    B2BLog::Err(LogFilt::LM_DRIVERS, "InitSpeechRecognition() error %s", 
				e.what());
  }

  return true; // SUCCESS
}

/*static*/ bool NaoSpeechRecognitionHW::ShutdownSpeechRecognition(
									boost::shared_ptr<AL::ALBroker> broker)
{
  try {
  	/// Setting up a proxy to ALSpeechRecognition
  	AL::ALSpeechRecognitionProxy speechRecognition(broker);

  	speechRecognition.pause(true); // stop ALSpeechRecognition

  } catch (const AL::ALError& e) {
    B2BLog::Err(LogFilt::LM_DRIVERS, "PauseSpeechRecognition() error %s", 
				e.what());
  }

  return true; // SUCCESS
}

void NaoSpeechRecognitionHW::init()
{
  if (!IsEnabled())
  	return; // We're not enabled, don't init anything

  try {
  	/// Setting up a proxy to ALSpeechRecognition
  	AL::ALSpeechRecognitionProxy speechRecognition(getParentBroker());

  	speechRecognition.subscribe(getName());

    /** Create a proxy to ALMemory.  */
    fMemoryProxy = AL::ALMemoryProxy(getParentBroker());

    /** Subscribe to event
    * Arguments:
    * - name of the event
    * - name of the module to be called for the callback
    * - name of the bound method to be called on event
    */
  	EventNames::const_iterator iter;
  	for (iter = m_eventNames.begin(); iter != m_eventNames.end(); ++iter) {
  		//B2BLog::Info(LogFilt::LM_DRIVERS, 
  		//	"Subscribe \"%s\"::onEvent to listen to event \"%s\"", 
  		//	 getName().c_str(), iter->c_str());
    	fMemoryProxy.subscribeToEvent(iter->c_str(), getName().c_str(), "onEvent");
	}

	SetValid();

  } catch (const AL::ALError& e) {
    B2BLog::Err(LogFilt::LM_DRIVERS, "%s: init() error %s", 
				getName().c_str(), e.what());
  }
}

/// This function is called by ALMemory every time the value of
/// the key "WordRecognized" is modified by the speech recognition engine.
void NaoSpeechRecognitionHW::onEvent(const std::string &key, 
					const AL::ALValue &value, const AL::ALValue &message)
{
  B2BLog::Debug(LogFilt::LM_DRIVERS, 
  			   "%s::onEvent %skey: %s value: %s (type: %s) message: %s",
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
  if (value.getSize() == 0)
  	return; // NO DATA: nothing in array, sensor is not "present"
			//			This happens sometimes, not sure why

  float confidence = value[1];  // value format: [word, confidence]
  if (confidence < m_confidenceThreshold) 
  	return; // NO DATA: not enough confidence in this word being recognitioned

  std::string textString = value[0]; // recognized speech text
  if (std::find(m_wordList.begin(), m_wordList.end(), textString) 
  	  == m_wordList.end())
  {
  	return; // NO DATA: not in our word list, this should never happen
  }

  B2BLog::Info(LogFilt::LM_DRIVERS, "%s::onEvent value: \"%s\", %.4f",
  				getName().c_str(), textString.c_str(), confidence);

  pthread_mutex_lock(&m_lockSensorHW);
  // FIXME: use thresholds on confidence value to set level?
  m_data.level = B2BLogic::LEVEL_MEDIUM;
  m_data.vector.magnitude = confidence;
  m_data.timestamp = B2BTime::GetCurrentB2BTimestamp();
  m_speechHWData.m_textString = textString;
  m_speechHWData.m_confidence = confidence;
  pthread_mutex_unlock(&m_lockSensorHW);

  /// Parse the list of words that has been recognized by ALSpeechRecognition
  for(unsigned int i = 0; i < value.getSize()/2 ; ++i)
  {
    /// If our "command" has been recognized, start a speech synthesis reaction
    if(((std::string)value[i*2] == m_wordList[0]) && (float)value[i*2+1] > m_confidenceThreshold)
    {
    }
  }
}

bool NaoSpeechRecognitionHW::SpeechRecognitionGetLatest(SensorHWData *pData,
		SpeechHWData *pSpeechHWData, SensorHWStatus *pStatus, bool raw)
{
	bool result = true; // assume SUCCESS

	// Make sure SensorSWGetLatest and m_speechHWData are accessed atomically
	pthread_mutex_lock(&m_lockSensorHW);
		if (SensorHWGetLatest(pData, pStatus, raw)) {
			// SUCCESS
			if (pSpeechHWData) *pSpeechHWData = m_speechHWData;
		} else {
			result = false; // FAIL: SensorSWGetLatest already set pStatus
		}
	pthread_mutex_unlock(&m_lockSensorHW);

	return result;
}
