//
// GPIO: The AM335x CPU has 128 GPIOs total: 32 GPIOs in each bank, and 4 banks.
//		These GPIO pins can be configured to many different usages depending
//		on features needed.
//		These are numbered in all the AM335x documentation
//		as GPIO<bank>_<gpio_pin_number_in_bank>.   For example: 
//			GPIO0_12 is bank #0, pin #12
//			GPIO3_23 is bank #3, pin #23
//		We will number GPIOs using a single number called "port" which
//		is defined as (bank# * 32) + pin#.  For example:
//			GPIO1_5 is port 37
//			GPIO3_31 is port 127
//
#include "boost/property_tree/json_parser.hpp"

#include "common/b2bassert.h"
#include "common/B2BTime.h"
#include "common/StringUtil.h"
#include "common/JSONUtil.h"

#include "log/B2BLog.h"

#include "IOConfig.h"


IOConfig::IOConfig(const char *ioConfigFileNameFullPath) :
	B2BModule("IOConfig")
{
	b2bassert(ioConfigFileNameFullPath);

	std::string err;
	if (IOConfigFileParse(ioConfigFileNameFullPath, &err)) {
		// successful read
		B2BLog::Debug(LogFilt::LM_APP, 
				"IOConfig file %s: parse SUCCESS %d entries",
				ioConfigFileNameFullPath, m_ioEntries.size());

	} else {
		B2BLog::Err(LogFilt::LM_APP, 
				"IOConfig file %s: parse FAIL \"%s\".  Using default settings.",
				ioConfigFileNameFullPath, err.c_str());
		// Continue on, we will use empty table
	} 
}

void IOConfig::ParseJSON(const char *parentNodeName, const pt::ptree &node)
{
	IOConfigEntry entry;
    for (pt::ptree::const_iterator pos = node.begin(); pos != node.end();) {

	  // Get the node name as a string
      std::string node_name = pos->first;

	  // recursively call ourselves to dig deeper into the tree
      if (!pos->second.empty()) {

  		// recurse into node
        ParseJSON(node_name.c_str(), pos->second);

      } else {
	  	// node has no children, must be data

		if (parentNodeName == NULL) {
			// Parent is json root, do nothing
		} else {
			entry.ioName = parentNodeName;
			const char *dataString = pos->second.data().c_str();
        	if (node_name == "GPIO") {
			  	entry.ioType = IOTYPE_GPIO;
				entry.portNum = strtol(dataString, NULL, 10); // GPIO port
        	} 
			else if (node_name == "PWM") {
			  	entry.ioType = IOTYPE_PWM;
				entry.portNum = strtol(dataString, NULL, 10);  // GPIO port
        	} else if (node_name == "I2C") {
			  	entry.ioType = IOTYPE_I2C;
				entry.portNum = strtol(dataString, NULL, 10);  // GPIO port
				entry.direction = IODIR_OUTPUT;
        	} else if (node_name == "SPI") {
			  	entry.ioType = IOTYPE_SPI;
				entry.portNum = strtol(dataString, NULL, 10);  // GPIO port
				entry.direction = IODIR_OUTPUT;
        	} else if (node_name == "COUNTER") {
			  	entry.ioType = IOTYPE_COUNTER;
				entry.portNum = strtol(dataString, NULL, 10);  // GPIO port
        	} else if (node_name == "ANALOG") {
			  	entry.ioType = IOTYPE_ANALOG;
				entry.portNum = strtol(dataString, NULL, 10);  // GPIO port
        	} else if (node_name == "enable") {
			  	entry.enable = StringUtil::StringToBool(pos->second.data());
			} else if (node_name == "dir") {
				std::string tempStr = pos->second.data();
				if (tempStr == "input") {
					entry.direction = IODIR_INPUT;
				} else if (tempStr == "output") {
					entry.direction = IODIR_OUTPUT;
				}
        	} else if (node_name == "polarity") {
				std::string tempStr = pos->second.data();
				if ((tempStr == "active_high") || (tempStr == "high")) {
					entry.polarity = IOPOLARITY_HIGH; // active HIGH
				} else if ((tempStr == "active_low") || (tempStr == "low")) {
					entry.polarity = IOPOLARITY_LOW; // active LOW
				}
			} else if (node_name == "lowThreshold") {
				// ANALOG
				entry.thresholds.threshValues[B2BLogic::LEVEL_LOW] = 
													strtof(dataString, NULL);
			} else if (node_name == "mediumThreshold") {
				// ANALOG
				entry.thresholds.threshValues[B2BLogic::LEVEL_MEDIUM] =
													strtof(dataString, NULL);
			} else if (node_name == "highThreshold") {
				// ANALOG
				entry.thresholds.threshValues[B2BLogic::LEVEL_HIGH] =
													strtof(dataString, NULL);
        	} else if (node_name == "_comment") {
				// skip comments
        	} else {
				if (entry.ioType == IOTYPE_TOTAL) {
					// Unknown node name and haven't chosen IOType yet
					// This is the wildcard type, which we label as GENERAL
			  		entry.ioType = IOTYPE_GENERAL;
					// any integer goes here
					entry.portNum = strtol(dataString, NULL, 10);  
				} else {
					// Unknown node name and IOType already chosen
					// Add to IOType's extraSettings std::map
					int32_t intValue;
					float floatValue;
					if (StringUtil::StringToInt(pos->second.data(),
												&intValue))
					{
					  	entry.extraSettings.insert(
							std::pair<std::string, IOConfigEntry::ExtraData>(
												node_name, (float)intValue));
					} else if (StringUtil::StringToFloat(pos->second.data(),
														 &floatValue))
					{
					  	entry.extraSettings.insert(
							std::pair<std::string, IOConfigEntry::ExtraData>(
												node_name, floatValue));
					} else if ((pos->second.data() == "true") ||
							   (pos->second.data() == "false"))
					{
					  	entry.extraSettings.insert(
							std::pair<std::string, IOConfigEntry::ExtraData>(
												node_name, 
			  					StringUtil::StringToBool(pos->second.data())));
					} else {
					  	entry.extraSettings.insert(
							std::pair<std::string, IOConfigEntry::ExtraData>(
												node_name, pos->second.data()));
					}
				}
			} 
		}
      }

	  ++pos;
	}

	if (entry.IsValid()) {
	  //B2BLog::Debug(LogFilt::LM_APP, "IOConfig insert %s", 
	  //								entry.ioName.c_str());
	  m_ioEntries.insert(std::pair<IOName, IOConfigEntry>(entry.ioName, entry));
	}
 
}

bool IOConfig::IOConfigFileParse(const char *ioConfigFileNameFullPath, 
										std::string *pErr)
{
	// Read in file and load those settings into tree
	pt::ptree jsonTree;
	try {
		pt::read_json(ioConfigFileNameFullPath, jsonTree);

		//JSONUtil::PrintTreeJSON(jsonTree);
        //printf("\n");

	} catch(const pt::ptree_error &e) {
		*pErr = std::string("Read error ") + e.what();
		return false; // FAIL
    }

	ParseJSON(NULL, jsonTree);

	return true; // SUCCESS
}

const IOConfig::IOConfigEntry *IOConfig::Lookup(const IOName &name) const
{
	IOConfigEntryList::const_iterator iter;
	iter = m_ioEntries.find(name);
	if (iter != m_ioEntries.end()) {
		return &iter->second; // SUCCESS: found name
	} else {
		return NULL; // FAIL: name not found
	}
}
