#pragma once
//
// IOConfig: class that reads in IO config json file and stores it contents
//
#include <string.h>
#include <stdint.h>
#include <string>
#include <map>

#include "boost/variant.hpp"
#include "boost/property_tree/ptree.hpp"

#include "common/B2BLogic.h"
#include "log/B2BLog.h"

#include "apps/common/B2BModule.h"

class GPIODriverHW;

namespace pt = boost::property_tree; // make it easier to use ptree.hpp

class IOConfig : public B2BModule {
  public:
	IOConfig(const char *ioConfigFileNameFullPath);
	IOConfig() : B2BModule("IOConfigNULL") {} // If you want an empty instance
	~IOConfig() {}

	// START: required virtual from B2BModule.  See that class for doc
	bool Init() { return true; }
	bool Start(void *arg) { return true; }
	bool Stop(void **returnVal) { return true; }
	// END: required virtual from B2BModule.  See that class for doc

	typedef std::string IOName;


	// Format in json:
	//
	// Examples:
	// "IOName" : { "GPIO" : IOPortNum, "dir" : dir, "polarity" : polarity },
	// "IOName" : { "PWM" : IOPortNum, "dir" : dir, "polarity" : polarity },
  	// "IOName" : { "I2C" : busId, "deviceAddr" : address },
  	// "IOName" : { "SPI" : busId, "chipSelect" : address },
	// "IOName" : { "COUNTER" : IOPortNum, "dir" : dir, "polarity" : polarity },
  	// "IOName" : { "ANALOG" : channel, "dir" : dir, "lowThreshold" : float, "mediumThreshold" : float, "highThreshold" : float }, 
  	// "IOName" : { "anything" : anyint, "dir" : dir, "polarity" : polarity},
	// 		"anything": wildcard that accepts any string gets labelled with
	// 		IOType GENERAL.   dir and polarity are optional.  You can
	//		define as many "valueName" : value pairs as you wish
	//		(defining zero pairs is legal too!).
	//		"anything" CANNOT be one of the reserved words you see above
	//		like "dir", "polarity", _comment, etc.
	// Any label in quotes that is not recognized is stored in extraSettings
	//		and you can define as many as you wish.  This allows you to have
	//		flexible definitions for any IO type.  The number of pairs
	//		(from json's "valueName" : value) in extraSettings is unlimited.
	//
	// IOType: description     IOPortNum	dir 	polarity  Note
	// GPIO: generic GPIO pin    R     	  	 R         O		1
	// PWM: PWM port			 R     	  	 R         O
	// I2C: I2C bus				 R(busId)	 N         N		2
	// SPI: SPI bus				 R(busId)	 N         N		3
	// COUNTER: counter 		 R			 R		   O
	// ANALOG: analog channel 	 R(channel)	 R         N        4
	// GENERAL: all others 		 R(any int)  O         O		5
	//
	// R = required, O = optional, N = not used(do not define in json!)
	// "polarity" defaults to "active_high"
	// All types support json "enable" : true|false,   The default is true.
	//
	// Note 1: For GPIO output pins, "init" should be defined (stored in
	//		extraSettings).  If "init" is not defined, GPIO state is the
	//		default state as defined by the OS.
	//		PAY ATTENTION: "init" ignores polarity.   If high,
	//		then GPIO is set HIGH (1), if LOW it is set LOW (0).
	// Note 2: For I2C, "deviceAddr" must be defined (stored in extraSettings).
	// Note 3: For SPI, "chipSelect" must be defined (stored in extraSettings).
	// Note 4: For ANALOG, json can define "lowThreshold", "mediumThreshold",
	//		and "highThreshold" (all are optional).  The associated float values
	//		are stored in IOConfigEntry.thresholds.   If a threshold is not
	//		defined, its value in IOConfigEntry.thresholds is 0.
	// Note 5: IOType GENERAL is used for IO type strings not listed above.
	//		For example, "Instance" is not in above IOType column.
	//   	  "AudioOut": { "Instance": 0, "dir": "output", "deviceName": "X" },
	//	 	  IOConfigEntry will be: IOType GENERAL, IOName "AudioOut",
	//			portNum is 0, and extraSettings have have one entry in
	//			std::map of std::pair<"deviceName", "X">
	//	 
	typedef enum {
	   IOTYPE_GPIO = 0,
	   IOTYPE_PWM,
	   IOTYPE_I2C,
	   IOTYPE_SPI,
	   IOTYPE_COUNTER,
	   IOTYPE_ANALOG,
	   IOTYPE_GENERAL,
	   IOTYPE_TOTAL		// size of enum (never used as a valid value)
	} IOType;

	typedef int16_t IOPortNum; // IO type's port number (defines which pin)
	typedef enum {
	   IODIR_INPUT = 0,
	   IODIR_OUTPUT,
	   IODIR_TOTAL		// size of enum (never used as a valid value)
	} IODirection;

	// Affects GPIO and COUNTER types
	typedef enum {
	   IOPOLARITY_LOW = 0,	// active LOW
	   IOPOLARITY_HIGH,		// active HIGH
	   IOPOLARITY_TOTAL		// size of enum (never used as a valid value)
	} IOPolarity;

	typedef float IOThreshold;
	// This class groups the thresholds found in ioconfig entries
	class IOThresholds {
	  public:
		IOThresholds() 
		{
			memset(threshValues, 0, sizeof(threshValues));
		}
		// 0 value means it is not used.
		IOThreshold threshValues[B2BLogic::LEVEL_TOTAL];
	};
	static const IOPortNum IOPORTNUM_INVALID = -1;

	class IOConfigEntry {
	  public:
		IOConfigEntry() : 
			//ioName(), // init'ed by its own ctor
			ioType(IOTYPE_TOTAL),
			enable(true),				// default to enabled
			portNum(IOPORTNUM_INVALID),
			direction(IODIR_TOTAL),
			polarity(IOPOLARITY_HIGH)	// default to "active_high"
		{}

		IOName ioName;
		IOType ioType;
		bool enable;
		IOPortNum portNum;
		IODirection direction;
		IOPolarity polarity;
		IOThresholds thresholds;

		// FIXME: Allow integer too
		typedef boost::variant<bool, float, std::string> ExtraData;
		// key is the label from json.  ExtraData is the value.
		typedef std::map<std::string, ExtraData> ExtraSettings;
		ExtraSettings extraSettings;

		// RETURNS: true if our entry is valid, false otherwise
		bool IsValid() const
			{ 
				return (ioName.length() != 0) && (ioType != IOTYPE_TOTAL) 
					   && (portNum != IOPORTNUM_INVALID)
					   && (direction != IODIR_TOTAL);
			}

	};

	// name: the IO name
	// RETURNS: the IO config entry for name, NULL if name not found
	const IOConfigEntry *Lookup(const IOName &name) const;

	#ifdef OS_IS_LINUX
	// Go through all the IOs and do some initialization
	// PWM: none
	// GPIO: Create and set input/output mode
    void InitIOs(GPIODriverHW *gpio);
	#endif // OS_IS_LINUX

	// For entry ioName, return if enabled or not
	// TRUE: enabled
	// FALSE: disabled
	// UNKNOWN: IOConfig ioName entry does not exist
	B2BLogic::Bool3 IsEntryEnabled(const IOName &ioName)
	{
	  const IOConfig::IOConfigEntry *ioConfigEntry = Lookup(ioName);
	  if (ioConfigEntry) {
		// found entry, return enable value
	  	return B2BLogic::BoolToBool3(ioConfigEntry->enable);
	  } else {
		return B2BLogic::BOOL3_UNKNOWN; // entry doesn't exist
	  }
	}

	// For entry ioName, return extraSettings value associated with label.
	// pValue: only set if return value is true.
	// logError: if true, log errors; if false, do not log
	// RETURNS: true on success, false otherwise
	template <typename T>
	bool GetExtraSettingsValue(const IOName &ioName, std::string label,
							T *pValue, bool logError=true) const
	{
	  const IOConfig::IOConfigEntry *ioConfigEntry = Lookup(ioName);
	  if (ioConfigEntry) {
		IOConfig::IOConfigEntry::ExtraSettings::const_iterator iter =
								ioConfigEntry->extraSettings.find(label);
		if (iter != ioConfigEntry->extraSettings.end()) {
			*pValue = boost::get<T>(iter->second);
			return true; // SUCCESS
		} else {
			if (logError) {
				B2BLog::Err(LogFilt::LM_APP, "%s: no IOConfig \"%s\" defined.",
			  								ioName.c_str(), label.c_str());
			}
			return false; // FAIL
		}
	  } else {
		if (logError) {
			B2BLog::Err(LogFilt::LM_APP, "%s not defined in IOConfig",
											ioName.c_str());
		}
		return false; // FAIL
	  }
	}

  private:
	// Helper function that does actual parsing of the pt::ptree node
	// parentNodeName: the name of the parent node that contains this ptree
	void ParseJSON(const char *parentNodeName, const pt::ptree &node);

	// Read in IO config file, and load IO config tables from it.
	// ioConfigFileNameFullPath: full path name of IO config file
	// pErr: errors, only valid if method returns false
	// RETURNS: true on success, false otherwise.
	bool IOConfigFileParse(const char *ioConfigFileNameFullPath, 
										std::string *pErr);

	// table of IO definitions
	typedef std::map<IOName, IOConfigEntry> IOConfigEntryList;
	IOConfigEntryList m_ioEntries;
};
