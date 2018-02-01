
#include <neural/Neuron.h>

class AudioOutputNeuron: public Neuron {
public:
	AudioOutputNeuron(ModuleApplicator* applicator, PersonalityEngine* pe);
	virtual ~AudioOutputNeuron();

	//START: Neuron overrides
	virtual bool ExciteTrigger() {return true;} //nothing fancy, just safer and easier to control.
	virtual bool Excite(); //Main method called by AudioOutput.cpp
	virtual bool OnPollAction() {return false;} //This should not be called.
	virtual bool RisingEdgeSignal();
	virtual bool ContinuingSignal();
	virtual bool FallingEdgeSignal();
	//END: Neuron overrides
};
