#pragma once
// 
//		NAO sound HW driver: returns direction and magnitude of sound
//
#include <string>

#include "drivers/naoqi/common/NaoEventModuleSensorHW.h"

class NaoSoundDirectionHW : public NaoEventModuleSensorHW {
  public:
	// parameters: see NaoEventModuleSensorHW for documentation
	// IOConfig used: (all are optional)
	//	 "sensitivityThreshold": passed to setParameter("SensitivityThreshold")
	//				Default is NAOSOUNDHW_SENSITIVITY_THRESH
	//	 "confidenceThreshold": used to compare with "energy" value that
	//				comes to onEvent().  If "energy" value >=
	//				this confidenceThreshold, we consider sound valid.
	//				Default is NAOSOUNDHW_CONFIDENCE_THRESH
	NaoSoundDirectionHW(boost::shared_ptr<AL::ALBroker> broker,
							const NaoEventModuleSensorHWConfig &config);

	// START: Overloading NaoEventModuleSensorHW virtual methods

    // This is called right after the module has been loaded
    virtual void init();
	// We need to calculate magnitude and vector from value array.
    // This method will be called every time the event is raised.
    virtual void onEvent(const std::string &key, 
						 const AL::ALValue &value, const AL::ALValue &message);

	// END: Overloading NaoEventModuleSensorHW virtual methods

	////// START: override virtual from SensorHW.  See that class for doc
	virtual bool SensorHWGetLatestOfLevel(B2BLogic::Level level,
			b2b::Timestamp since, SensorHWData *pData,
			SensorHWStatus *pStatus=NULL, bool raw=false);
	////// END: override virtual from SensorHW.  See that class for doc

  private:
  	float m_confidenceThreshold; // from IOConfig "confidenceThreshold"
  	float m_sensitivityThreshold; // from IOConfig "sensitivityThreshold"

	// one entry for each level.  Index from LOW to HIGH
	typedef std::vector<SensorHWData> LevelSoundData;
	LevelSoundData m_levelSoundData;
};
