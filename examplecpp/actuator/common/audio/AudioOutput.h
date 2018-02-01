#pragma once
//
// AudioOutput: generic class for playing audio
//		It can play files or text
//		WARNING: Client must call Init() and Start() to enable this class.
//
#include <stdint.h>
#include <vector>

#include "boost/variant.hpp"

#include "common/b2btypes.h"

#include "actuator/common/Actuator.h"
#include "apps/common/ExecuteListInThread.h"

class Neuron;

namespace AudioOutputType {
	// Data entry when playing an audio file
	// dataP: the raw data of a file to play.  Always NULL if
	//		m_deviceCanReadFile is true.
	// filenameFullPath: the name of a file to play.
	class AudioData {
	  public:
		AudioData(uint8_t *newDataP, std::string &newFilenameFullPath) :
		   dataP(newDataP), filenameFullPath(newFilenameFullPath) {}
		virtual ~AudioData()
		{
			if (dataP) {
				delete [] dataP;
				dataP = NULL;
			}
		}
			
		uint8_t *dataP;
		std::string filenameFullPath;
	};

	// List of audio or text to play
	// AudioData: entry contains info on file to play
	// std::string: entry contains the text to speak
	// 		The type stored, defines what action is taken (play a file or
	// 		call text to speech)
	typedef boost::variant<AudioData, std::string> PlayListEntry;
}

class AudioOutput : public ExecuteListInThread<std::string, AudioOutputType::PlayListEntry>, public Actuator
{
  public:
	// device: The device instance to output audio to.   This is platform
	//				dependent.  To use default audio output device, set to NULL.
	//				Set to ACTUATOR_DEVICE_INVALID to disable audio.
	// audioFilePath: defines directory path for audio files
	// filePath: defines directory path for audio files
	// deviceCanReadFile: true if we can just pass file name to audio device,
	//		false if we need to send an array of raw data to the audio device.
	// filesAreRemote: true if files are not on our machine, false if they
	//		are on our machine.
    AudioOutput(const void *device,
				const char *audioFilePath,
				bool deviceCanReadFile,
				bool filesAreRemote);
    virtual ~AudioOutput();

	// START: required virtuals from ExecuteListInThread; see that class for docc
	bool Init() { return true; } // nothing special to do here
	// END: required virtuals from ExecuteListInThread

	const b2b::Volume VOLUME_DEFAULT; // default volume

	static const b2b::Volume VOLUME_MAX;	 // max volume allowed
	static const b2b::Volume VOLUME_MIN;	 // min volume allowed
	static const b2b::Volume VOLUME_INVALID; // invalid volume value (used to denote "None")

	// PlayFile: Play one audio file asynchronously.
	// PlayFiles: Play audio files from a list.  Played in order in list.
	// (All files are located in m_audioFilePath directory.)
	// PAY ATTENTION: all audio file formats supported by aplay(1) will work
	//
	// First method uses SetVolume(newVolume).
	// Second method uses SetVolume(VOLUME_DEFAULT).
	//
	// filename: name of file (file must exist in audioFilePath set by ctor)
	// fileList: list of file names to play
	// newVolume: call SetVolume(newVolume) before playing the file. 
	// loop: If true, loop displaying until StopExecuting is called.
	//		 If false, display the file, fileList, dirName's file only once.
	// pDoneCallback: Contains method to call to report we are done.
	//		See DoneReason for all the reasons we can call this callback.
	//		Called once per PlayFile()/PlayFiles() call.
	//		Set to NULL if no reporting is desired.
	//
	// Returns: true on success, false otherwise
	//		Will fail and return false if already in process of playing
	//		a file or file list
	//
	// WARNING: mute is not changed when PlayFile/Playfiles is called
	//
	// Will fail if any filename does not exist.
	//
	// IMPROVE: allow separate volume for each file
	//
	bool PlayFile(const char *filename, b2b::Volume newVolume, bool loop=false,
					const ExecuteListType::DoneCallback *pDoneCallback=NULL)
	{ 
		std::vector<std::string> fileList;
		fileList.push_back(filename);
	  	return PlayFiles(fileList, newVolume, loop, pDoneCallback);
	}
	bool PlayFile(const char *filename, bool loop=false,
					const ExecuteListType::DoneCallback *pDoneCallback=NULL)
	  { return PlayFile(filename, VOLUME_DEFAULT, loop, pDoneCallback); }

	bool PlayFiles(const std::vector<std::string> &fileList, 
				b2b::Volume newVolume, bool loop=false,
				const ExecuteListType::DoneCallback *pDoneCallback=NULL);
	bool PlayFiles(const std::vector<std::string> &fileList, bool loop=false,
					const ExecuteListType::DoneCallback *pDoneCallback=NULL)
	  { return PlayFiles(fileList, VOLUME_DEFAULT, loop, pDoneCallback); }

	// TextToSpeechString: Speak text asynchronously.
	// TextToSpeechStrings: Speak text from a list.  Played in order in list.
	//
	// First method uses SetVolume(newVolume).
	// Second method uses SetVolume(VOLUME_DEFAULT).
	//
	// text: the text to speak
	// fileList: list of text to speak
	// newVolume: call SetVolume(newVolume) before speaking. 
	// loop: If true, loop displaying until StopExecuting is called.
	//		 If false, display the file, fileList, dirName's file only once.
	// pDoneCallback: Contains method to call to report we are done.
	//		See DoneReason for all the reasons we can call this callback.
	//		Called once per TextToSpeechString()/TextToSpeechStrings() call.
	//		Set to NULL if no reporting is desired.
	//
	// Returns: true on success, false otherwise
	//		Will fail and return false if already in process of speaking
	//		a text string or string list
	//
	// WARNING: mute is not changed when 
	//			TextToSpeechString/TextToSpeechStrings is called
	//
	// example: ("\\vct=50\\ Say something"); pitch (value between 50 and 200%)
	// example: ("\\rspd=50\\Say something"); rate (value between 50 and 400%)
	// example: ("Say \\pau=1000\\ something"); pause time (duration in ms)
	// example: ("\\vol=50\\ Say something"); volume (value between 0 and 100%, default = 80)
	// other examples in ALTextToSpeechTutorial
	bool TextStringToSpeech(const char *text, b2b::Volume newVolume,
					bool loop = false,
					const ExecuteListType::DoneCallback *pDoneCallback=NULL)
	{ 
		std::vector<std::string> textList;
		textList.push_back(text);
	  	return TextStringsToSpeech(textList, newVolume, loop, pDoneCallback);
	}
	bool TextStringToSpeech(const char *text, bool loop = false,
					const ExecuteListType::DoneCallback *pDoneCallback=NULL)
	{ return TextStringToSpeech(text, VOLUME_DEFAULT, loop, pDoneCallback); }

	bool TextStringsToSpeech(const std::vector<std::string> &textList, 
				b2b::Volume newVolume, bool loop = false,
				const ExecuteListType::DoneCallback *pDoneCallback=NULL);
	bool TextStringsToSpeech(const std::vector<std::string> &textList,
					bool loop = false,
					const ExecuteListType::DoneCallback *pDoneCallback=NULL)
	{ 
		return TextStringsToSpeech(textList, VOLUME_DEFAULT, 
									loop, pDoneCallback);
	}

	// volume: MIN_VOLUME to MAX_VOLUME (0 to 10)
	//			0 may still output some power to audio output.  Use mute to
	//			completely shut off all output.
	// Returns: true on success, false otherwise
	// If volume is same as m_volume, this method is a NOP and returns true.
	// WARNING: this is independent of mute.
	bool SetVolume(b2b::Volume volume);

	b2b::Volume GetVolume() { return m_volume; }

	// mute: if true, mute the audio output
	// Returns: true on success, false otherwise
	// If mute is same as m_mute, this method is a NOP and returns true.
	// WARNING: this is independent of volume
	bool SetMute(bool mute);

	// Returns true if we are muted, false otherwise.
	bool Muted() { return m_mute; }

	// Returns true if currently playing an audio file list, false otherwise
	bool IsPlaying() { return IsExecuting(); }

	//Using a neuron will allow the audio output to utilize the connectivity of the neural framework.
	//The primary example of how this might be useful is to inhibit the sound sensor(s) whenever
	//		sound is being produced by the creature.
	void UseNeuron(Neuron* toUse);

  protected:
	// START: required virtuals from ExecuteListInThread
	bool WorkerExecute(const AudioOutputType::PlayListEntry &entry);
	void WorkerYield();
	// END: required virtuals from ExecuteListInThread

	Neuron * m_neuron;

  private:
	// These 2 are used with ExecuteListInThread::ExecuteList
	BuildExecListInternal m_BuildExecListInternalPlayFile;
	bool BuildExecListInternalPlayFile(const ExecList &execList, 
							   ExecListInternal *pNewInternalList);
	BuildExecListInternal m_BuildExecListInternalTextStringToSpeech;
	bool BuildExecListInternalTextStringToSpeech(const ExecList &execList, 
							   ExecListInternal *pNewInternalList);

    const void *m_device;			// from ctor
	std::string m_audioFilePath;	// from ctor
	bool m_deviceCanReadFile;		// from ctor
	bool m_filesAreRemote;			// from ctor

    bool m_mute;   			// set by SetMute()
    b2b::Volume m_volume;	// set by SetVolume()

	// Set as temporary variable by PlayFiles/TextStringsToSpeech()  
	// BuildExecListInternal calls SetVolume() with this volume
    b2b::Volume m_newVolume;

	// tests need access to privates
	friend class CreatureMenu;
};
