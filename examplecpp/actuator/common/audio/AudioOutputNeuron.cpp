/*
 * AudioOutputNeuron.cpp
 *
 *  Created on: May 11, 2017
 *      Author: root
 */

#include <actuator/common/audio/AudioOutputNeuron.h>

AudioOutputNeuron::AudioOutputNeuron(ModuleApplicator* applicator, PersonalityEngine* pe) :
	Neuron("AudioOutputNeuron", applicator, pe)
{
}

AudioOutputNeuron::~AudioOutputNeuron() {
}

bool AudioOutputNeuron::Excite()
{
	SetValue(2);
	SendData();
	return true;
}
bool AudioOutputNeuron::RisingEdgeSignal()
{
	return false;
}
bool AudioOutputNeuron::ContinuingSignal()
{
	return false;
}
bool AudioOutputNeuron::FallingEdgeSignal()
{
	return false;
}
