#pragma once
// 
//		Generic NAO event module base class that supports SensorHW interface
//
#include <vector>
#include <string>

#include <boost/shared_ptr.hpp>
#include <alcommon/almodule.h>
#include <alproxies/almemoryproxy.h>

#include "log/B2BLog.h"

#include "drivers/common/sensor/SensorHW.h"

namespace AL
{
  class ALBroker;
}

class NaoEventModuleSensorHW : public AL::ALModule, public virtual SensorHW
{
  public:
	// Helper to create an module instance of NaoEventModuleSensorHW
	typedef std::vector<std::string> EventNames;
	class NaoEventModuleSensorHWConfig {
	  public:
	  	NaoEventModuleSensorHWConfig(const std::string &moduleName,
	  					const EventNames &eventNames,
						const IOConfig &ioConfig,
						const SensorHW::IOConfigEntryNames &ioConfigEntryNames):
			m_moduleName(moduleName),
			m_eventNames(eventNames),
			m_ioConfig(ioConfig),
			m_ioConfigEntryNames(ioConfigEntryNames)
		{}
	  	NaoEventModuleSensorHWConfig(const std::string &moduleName,
	  					const std::string &eventName,
						const IOConfig &ioConfig,
						const char *ioConfigEntryName) :
			m_moduleName(moduleName),
			m_eventNames(EventNames(1, eventName)),
			m_ioConfig(ioConfig),
			m_ioConfigEntryNames(ioConfigEntryName ? SensorHW::IOConfigEntryNames(1, ioConfigEntryName) : SensorHW::IOConfigEntryNames())
		{}

	  	NaoEventModuleSensorHWConfig(const std::string &moduleName,
	  					const EventNames &eventNames,
						const IOConfig &ioConfig,
						const char *ioConfigEntryName) :
			m_moduleName(moduleName),
			m_eventNames(eventNames),
			m_ioConfig(ioConfig),
			m_ioConfigEntryNames(ioConfigEntryName ? SensorHW::IOConfigEntryNames(1, ioConfigEntryName) : SensorHW::IOConfigEntryNames())
		{}

	  	const std::string &m_moduleName; // from ctor
	  	const EventNames &m_eventNames; // from ctor
		const IOConfig &m_ioConfig; // from ctor
		const SensorHW::IOConfigEntryNames &m_ioConfigEntryNames; // from ctor
	};

	// Helper to create an module instance of NaoEventModuleSensorHW
  	template <typename T>
  	static boost::shared_ptr<T> CreateModule(
						boost::shared_ptr<AL::ALBroker> pBroker,
	  					const std::string &moduleName,
	  					const EventNames &eventNames,
						const IOConfig &ioConfig,
						const SensorHW::IOConfigEntryNames &ioConfigEntryNames)
	{
  	  boost::shared_ptr<T> module;
	  try {
		NaoEventModuleSensorHWConfig config(moduleName, eventNames,
											ioConfig, ioConfigEntryNames);
  	  	module = AL::ALModule::createModule<T>(pBroker, config);
	  } catch (const AL::ALError& e) {
	  	B2BLog::Err(LogFilt::LM_DRIVERS, "createModule(%s) failed %s.",
								moduleName.c_str(), e.what());
	  }
  	  return module;
	}

  	template <typename T>
  	static boost::shared_ptr<T> CreateModule(
						boost::shared_ptr<AL::ALBroker> pBroker,
	  					const std::string &moduleName,
	  					const std::string &eventName,
						const IOConfig &ioConfig,
						const char *ioConfigEntryName)
	{
		EventNames eventNames;
		eventNames.push_back(eventName);

		SensorHW::IOConfigEntryNames entryNames;
		if (ioConfigEntryName)
			entryNames.push_back(ioConfigEntryName);

  		return CreateModule<T>(pBroker, moduleName, eventNames,
								ioConfig, entryNames);
	}

  	template <typename T>
  	static boost::shared_ptr<T> CreateModule(
						boost::shared_ptr<AL::ALBroker> pBroker,
	  					const std::string &moduleName,
	  					const EventNames &eventNames,
						const IOConfig &ioConfig,
						const char *ioConfigEntryName)
	{
		SensorHW::IOConfigEntryNames entryNames;
		if (ioConfigEntryName)
			entryNames.push_back(ioConfigEntryName);

  		return CreateModule<T>(pBroker, moduleName, eventNames,
								ioConfig, entryNames);
	}

	// moduleName: string name for module.   Must be of format
	//		"instancename_eventname" where "eventname" is NAO event you want
	//		to listen to (see doc.aldebaran.com/2-1/naoqi-eventindex.html).
	//		We add "instancename" so that more
	//		than one client can listen to same "eventname" and you can
	//		easily see which module is which when debugging.
    NaoEventModuleSensorHW(boost::shared_ptr<AL::ALBroker> broker, 
							const NaoEventModuleSensorHWConfig &config);
    virtual ~NaoEventModuleSensorHW();

    /** Overloading ALModule::init().
    * This is called right after the module has been loaded
    */
    virtual void init();

    // This method will be called every time the event is raised.
	// Can be overridden by derived class
    virtual void onEvent(const std::string &key, 
						 const AL::ALValue &value, const AL::ALValue &message);

	// START: required virtual from SensorHW.  See that class for doc
	// angle, azimuth: direction of face (currently set to all 0s)
	// level: confidence there is a face (always set to MEDIUM)
	// magnitude: depends on mode (see documentation at top of this file)
	bool SensorHWGetLatest(SensorHWData *pData,
							SensorHWStatus *pStatus=NULL, bool raw=false);
	// END: required virtual from SensorHW

  protected:
	EventNames m_eventNames;  // list of events names (from ctor's config.m_eventNames)

    AL::ALMemoryProxy fMemoryProxy;

	// Holding area for latest data
	SensorHWData m_data;
};
