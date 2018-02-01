#pragma once
//
//  Audio functions for host hardware
//
#include <stdint.h>

#include "common/b2btypes.h"

namespace AudioOutputHW {
	// Play an audio file.  
	// This function plays the whole file without interruption.
	// Returns after the play has completed.
	// We pass in the audio using two possible sources (because our target
	// platform may only be able to use one or other input format): 
	// 	 * the raw data from the audio file, presented as a buffer of bytes
	//	 * the audio file name
	//
	// device: The device instance to output audio to.   This is platform
	//				dependent.  To use default audio output device, set to NULL.
	// audioDataP: the audio data
	// fileName: the full path name of the file containing audio data
	//
	// PAY ATTENTION: all audio file formats supported by aplay(1) will work
	//
	// RETURNS: true on success, false otherwise
	bool PlayAudio(const void *device,
					  uint8_t *audioDataP, const char *fileName);

	// 
	// This function plays the whole text string without interruption.
	// Returns after it has completed.
	//
	// device: See documentation in PlayAudio() above
	// textString: the text to speak
	//
	// RETURNS: true on success, false otherwise
	bool TextStringToSpeech(const void *device, const char *textString);

	// Set volume to volume
	// device: See documentation in PlayAudio() above
	// volume: 0 to 100%
	bool VolumeSet(const void *device, b2b::Volume volume);

	// Enable/disable mute
	// device: See documentation in PlayAudio() above
	// mute: true to mute, false to unmute
	// volume: if mute is false, set volume to this value.   0 to 100%
	bool MuteSet(const void *device, bool mute, b2b::Volume volume);
}
