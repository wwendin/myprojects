//
// Audio hardware driver for the following OS: NAOqi
//
#include <string>

#include "log/B2BLog.h"

#include <alerror/alerror.h>
#include <alcommon/albroker.h>
#include <alproxies/alaudioplayerproxy.h>
#include <alproxies/altexttospeechproxy.h>

#include "drivers/common/audio/AudioOutputHW.h"

static float naoVolume = 0; // assume off

bool AudioOutputHW::PlayAudio(const void *device,
								 uint8_t *audioDataP, const char *fileName)
{
  boost::shared_ptr<AL::ALBroker> broker = *(const_cast<boost::shared_ptr<AL::ALBroker> *>(static_cast<const boost::shared_ptr<AL::ALBroker> *>(device)));

	// Play the file
  	try {
  	  AL::ALAudioPlayerProxy player(broker);
  	  player.playFile(fileName, naoVolume, 0);
  	} catch (const AL::ALError& e) {
	  B2BLog::Err(LogFilt::LM_DRIVERS, 
					"ALAudioPlayer.playFile() failed %s.", e.what());
	  return false; // FAIL
  	}

	return true; // SUCCESS
}

bool AudioOutputHW::TextStringToSpeech(const void *device,
										const char *textString)
{
  boost::shared_ptr<AL::ALBroker> broker = *(const_cast<boost::shared_ptr<AL::ALBroker> *>(static_cast<const boost::shared_ptr<AL::ALBroker> *>(device)));

	// Speak the textString
  	try {
  	  AL::ALTextToSpeechProxy tts(broker);
  	  tts.setVolume(naoVolume);
  	  tts.say(textString);
  	} catch (const AL::ALError& e) {
	  B2BLog::Err(LogFilt::LM_DRIVERS, 
					"ALTextToSpeech.say() failed %s.", e.what());
	  return false; // FAIL
  	}

	return true; // SUCCESS
}

bool AudioOutputHW::VolumeSet(const void *device, b2b::Volume volume)
{
  	naoVolume = volume/100.0;
	return true; // SUCCESS
}

bool AudioOutputHW::MuteSet(const void *device, bool mute, 
													b2b::Volume volume)
{
  if (mute)
  	naoVolume = 0;
  else
  	naoVolume = volume/100.0;

	return true; // SUCCESS
}
