#pragma once
//
//  PWM hardware driver.  Each instance drives one PWM.
//
#include <stdint.h>
#include <string>

#include "common/b2btypes.h"

#include "apps/common/IOConfig.h"
#include "apps/common/B2BModule.h"

class PWMDriverHW : public B2BModule {
  public:
	// pwmPortNum: the HW port to use for this instance of PWM
	PWMDriverHW(const char *name, IOConfig::IOPortNum pwmPortNum);

	~PWMDriverHW();

	// START: required virtual from B2BModule.  See that class for doc
	bool Init() { return true; }
	bool Start(void *arg) { return true; }
	bool Stop(void **returnVal) { return true; }
	// END: required virtual from B2BModule.  See that class for doc

	typedef b2b::TimeMS Period;
	typedef float DutyCycle;

	// Set the PWM to the given period and duty cycle
	//
	// periodMS: period in milliseconds
	// dutyCycle: percent.  For example, 50 means 50%
	// WARNING: After setting period and duty cycle, this method ALWAYS
	//			calls On()
	//
	// RETURNS: true on success, false otherwise
	bool Set(Period periodMS, DutyCycle dutyCycle);

	// Turn PWM on
	// RETURNS: true on success, false otherwise
	// Will fail and return false if Set never called before first On() call
	bool On() const;

	// Turn PWM off
	// RETURNS: true on success, false otherwise
	bool Off() const;

  private:
	// Returns "pwmchip<x>" that corresponds to hwDeviceName
	static std::string GetPWMClassName(const char *hwDeviceName);

	typedef enum {
		PWMCHANNEL_0 = 0,
		PWMCHANNEL_1,
		PWMCHANNEL_TOTAL, // size of enum (never used as a valid value)
		PWMCHANNEL_A = PWMCHANNEL_0,
		PWMCHANNEL_B = PWMCHANNEL_1
	} Channel;

	// Maps pwmPortNum to HW module values.
	// INPUT
	// 	 pwmPortNum: GPIO port number
	// OUTPUTS
	// 	 pModuleName: see m_moduleName below for documentation
	// 	 pChannel: see m_channel below for documentation
	// RETURNS: value returned by GetPWMClassName(), NULL if this method fails
	//
	static const char *MapPortNumToPWM(IOConfig::IOPortNum pwmPortNum,
												std::string *pModuleName,
												Channel *pChannel);

    IOConfig::IOPortNum m_pwmPortNum;		// from ctor

	// These are derived from ctor's pwmPortNum
	std::string m_moduleName; // HW name in file system for the PWM (pwmchip<x>)
	Channel m_channel;	// HW sometimes has more than one PWM channel
						// inside each module.  If PWM only has one channel,
						// this value is set to PWMCHANNEL_0

	// true if Set() executed at least once without error
	bool m_setCalledSuccessfully;

	// tests need access to privates
	friend class GizmoCreatureMenu;
	friend class SeonTestCreatureMenu;
	friend class MomentaCreatureMenu;
	friend class GunnarTestCreatureMenu;
};
