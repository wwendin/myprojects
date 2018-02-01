#include <stdio.h>

#include "boost/bind.hpp"

#include "common/b2bassert.h"
#include "common/B2BTime.h"
#include "common/FileUtil.h"
#include "log/B2BLog.h"

#include "drivers/common/audio/AudioOutputHW.h"

#include "AudioOutput.h"

#include "neural/Neuron.h"

/*static*/ const b2b::Volume AudioOutput::VOLUME_MAX = 10;
/*static*/ const b2b::Volume AudioOutput::VOLUME_MIN = 0;
/*static*/ const b2b::Volume AudioOutput::VOLUME_INVALID = -1;

AudioOutput::AudioOutput(const void *device,
						 const char *audioFilePath,
						 bool deviceCanReadFile,
						 bool filesAreRemote) :
  ExecuteListInThread("AudioOutput"),
  VOLUME_DEFAULT((VOLUME_MAX-VOLUME_MIN)/2), // halfway
  m_neuron(NULL),
  m_device(device),
  m_audioFilePath(audioFilePath),
  m_deviceCanReadFile(deviceCanReadFile),
  m_filesAreRemote(filesAreRemote),
  m_mute(true), // change to false below
  m_volume(VOLUME_INVALID)
{
	if (device == reinterpret_cast<const void *>(ACTUATOR_DEVICE_INVALID)) {
		B2BLog::Debug(LogFilt::LM_ACTUATOR, "%s: ioconfig enable false", Name());
		return;
	}

	// The following must use different values than ctor init above.  If you
	// don't, these methods are NOPs (because of no change in state).
	SetVolume(VOLUME_DEFAULT);
	SetMute(false); 

	m_BuildExecListInternalPlayFile = 
	  	boost::bind(&AudioOutput::BuildExecListInternalPlayFile, this, _1, _2);
	m_BuildExecListInternalTextStringToSpeech = 
	  	boost::bind(&AudioOutput::BuildExecListInternalTextStringToSpeech, this, _1, _2);
}

AudioOutput::~AudioOutput()
{
	Stop(NULL); // Stop our thread
}

bool AudioOutput::BuildExecListInternalPlayFile(const ExecList &execList, 
							   ExecListInternal *pNewInternalList)
{
	if (!SetVolume(m_newVolume)) {
		B2BLog::Info(LogFilt::LM_ACTUATOR, "PlayFiles SetVolume(%d) failed", 
											m_newVolume);
		// don't exit, try to play anyway
	}

	// Play to audio output, and do it asynchronously in thread (Worker)
	std::vector<std::string>::const_iterator iter;
	for (iter = execList.begin(); iter != execList.end(); ++iter) {
    	uint8_t *dataP = NULL;
		std::string filenameFullPath = m_audioFilePath + "/" + *iter;
  		if (!m_deviceCanReadFile) {
    		FILE *fp = fopen(filenameFullPath.c_str(), "rb");
    		if (fp) {
        		fseek(fp, 0, SEEK_END);
        		long size = ftell(fp);
        		fseek(fp, 0, SEEK_SET);
        		dataP = new uint8_t[size];
        		if (dataP) {
            		size_t bytesRead = fread(dataP, size, 1, fp);
					if (bytesRead != (size_t)size) {
						B2BLog::Err(LogFilt::LM_ACTUATOR, 
								"PlayFile(%s): fread read %lu bytes, expected %d",
								filenameFullPath.c_str(), bytesRead, size);
        				delete [] dataP;
        				fclose(fp);
						return false; // FAIL: failed to read data from file
					}
				} else {
					B2BLog::Err(LogFilt::LM_ACTUATOR, 
								"PlayFile(%s) cannot allocate data",
								filenameFullPath.c_str());
        			fclose(fp);
					return false; // FAIL: failed to allocate data
				}
        		fclose(fp);
    		} else {
				B2BLog::Err(LogFilt::LM_ACTUATOR, 
								"PlayFile cannot open file \"%s\"", 
								filenameFullPath.c_str());
				return false; // FAIL
			}
		} else {
			// No need to create dataP
    		if (!m_filesAreRemote
			    && !FileUtil::Exists(filenameFullPath.c_str()))
			{
				B2BLog::Err(LogFilt::LM_ACTUATOR, 
								"PlayFile file \"%s\" does not exist", 
								filenameFullPath.c_str());
				return false; // FAIL
			} // else: don't check is file exists if files are on remote host
		}
		// It's OK if dataP is NULL
		pNewInternalList->push_back(AudioOutputType::PlayListEntry(
						AudioOutputType::AudioData(dataP, filenameFullPath)));
	}

	return true; // SUCCESS
}

bool AudioOutput::PlayFiles(const std::vector<std::string> &fileList,
							b2b::Volume newVolume, bool loop,
							const ExecuteListType::DoneCallback *pDoneCallback)
{
	m_newVolume = newVolume; // save for BuildExecListInternal()
	bool returnVal = ExecuteList("PlayFiles", fileList, 
						m_BuildExecListInternalPlayFile, loop, pDoneCallback);
	if (!returnVal) {
		B2BLog::Err(LogFilt::LM_ACTUATOR, "PlayFiles(%s,%d) failed.", 
				fileList[0].c_str(), (int)fileList.size());
	}
	return returnVal;
}

bool AudioOutput::BuildExecListInternalTextStringToSpeech(
								const ExecList &execList,
								ExecListInternal *pNewInternalList)
{
	if (!SetVolume(m_newVolume)) {
		B2BLog::Info(LogFilt::LM_ACTUATOR, "TextStringToSpeech SetVolume(%d) failed", 
											m_newVolume);
		// don't exit, try to play anyway
	}

	// Play to audio output, and do it asynchronously in thread (Worker)
	std::vector<std::string>::const_iterator iter;
	for (iter = execList.begin(); iter != execList.end(); ++iter) {
		const std::string &oneTextString = *iter;
		pNewInternalList->push_back(AudioOutputType::PlayListEntry(oneTextString));
	}

	return true; // SUCCESS
}

bool AudioOutput::TextStringsToSpeech(const std::vector<std::string> &textList,
							b2b::Volume newVolume, bool loop,
							const ExecuteListType::DoneCallback *pDoneCallback)
{
	m_newVolume = newVolume; // save for BuildExecListInternal()
	bool returnVal = ExecuteList("TextStringsToSpeech", textList, 
				m_BuildExecListInternalTextStringToSpeech, loop, pDoneCallback);
	if (!returnVal) {
		B2BLog::Err(LogFilt::LM_ACTUATOR, "TextStringsToSpeech(%s,%d) failed.", 
				textList[0].c_str(), (int)textList.size());
	}
	return returnVal;
}

bool AudioOutput::SetVolume(b2b::Volume volume)
{
	if (m_volume == volume) 
		return true; // SUCCESS: value already set, do nothing

	m_volume = volume;
	
	if (m_device == reinterpret_cast<const void *>(ACTUATOR_DEVICE_INVALID))
		return true;

	if (m_mute) {
		// Be carefule to stay muted
		return AudioOutputHW::MuteSet(m_device, m_mute, m_volume*10);
	} else {
		return AudioOutputHW::VolumeSet(m_device, m_volume*10);
	}
}

bool AudioOutput::SetMute(bool mute)
{
	if (m_mute == mute) 
		return true; // SUCCESS: value already set, do nothing

	m_mute = mute;
	if (m_device == reinterpret_cast<const void *>(ACTUATOR_DEVICE_INVALID))
		return true;

	return AudioOutputHW::MuteSet(m_device, m_mute, m_volume*10);
}

void AudioOutput::UseNeuron(Neuron* toUse)
{
	m_neuron=toUse;
}

bool AudioOutput::WorkerExecute(const AudioOutputType::PlayListEntry &entry)
{
	if (m_device == reinterpret_cast<const void *>(ACTUATOR_DEVICE_INVALID))
		return false; // audio disabled

	//Use the neuron, if available, to handle any neural integrations (e.g. inhibiting the sound sensor)
	if(m_neuron){
		m_neuron->Excite(); //FIXME this causes a segfault????
	}

	const AudioOutputType::AudioData *audioDataP = boost::get<AudioOutputType::AudioData>(&entry);
	if (audioDataP) {
		// entry is AudioData 
		B2BLog::Info(LogFilt::LM_ACTUATOR, "Play file \"%s\" at volume %d", 
						audioDataP->filenameFullPath.c_str(), (int)m_volume);
		// Play the file to audio HW
		// This call waits until the audio has completed before returning.
		// Playing audio takes time and that is why we are in a
		// separate thread
		AudioOutputHW::PlayAudio(m_device,
		  					audioDataP->dataP,
		  					audioDataP->filenameFullPath.c_str());
	} else {
		// entry is text string
		const std::string *textStringP = boost::get<std::string>(&entry);
		b2bassert(textStringP);
		if (textStringP) {
			B2BLog::Info(LogFilt::LM_ACTUATOR, "Play text \"%s\" at volume %d", 
							textStringP->c_str() , (int)m_volume);
		  	AudioOutputHW::TextStringToSpeech(m_device, textStringP->c_str());
		} // else: impossible!!!
	}

	return true; // SUCCESS: keep running
}

void AudioOutput::WorkerYield()
{
	usleep(500*1000); // sleep 1/2 second after every play (reduces crashes)
}
