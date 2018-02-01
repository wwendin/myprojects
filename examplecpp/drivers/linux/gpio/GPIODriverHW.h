#pragma once
//
// Class for setting ARM GPIOs directly
//
// portNum = (gpioBank * 32) + gpioBitNum
// For all values (portNum, gpioBank, gpioBitNum), 0 is the first value (in
//		other words, we are "zero based")
//
//	Create(portNum)			// Enable GPIO to be used
//
//	ConfigAsInput(portNum)	// Configure GPIO as input
//	ConfigAsOutput(portNum)	// Configure GPIO as output
//	DirectionGet(portNum)	// RETURNS: 1 if Output, 0 if input; -1 on error
//
//	SetHigh(portNum)        // Set GPIO high
//	SetLow(portNum)         // Set GPIO low
//  Set(portNum, level) // Generic set of GPIO to high(true) or low(false)
//
//	Read(portNum);          // RETURNS: 1 if high, 0 if low; -1 on error
//
//
// RETURNS: All above methods, except DirectionGet and Read, return true on
//		success, false otherwise.
//
// FIXME: change to use direct memory mapping to improve speed
// 	const uint32_t BeagleGPIO::gpioAddrs[] =
//		{ 0x44E07000, 0x4804C000, 0x481AC000, 0x481AE000 };
//  gpioFd = open("/dev/mem", O_RDWR | O_SYNC);
//  m_gpio = (uint32_t *) mmap(NULL, GpioMemBlockLength,
//				PROT_READ | PROT_WRITE, MAP_SHARED, gpioFd, gpioAddrs[i]);
//  m_gpio[GPIO_Pin_Bank[_pin]][kOE/4] |= v;
//  munmap(gpios[i], GpioMemBlockLength);
//	close(gpioFd);
//
#include <stdio.h>
#include <stdlib.h>

#include "apps/common/IOConfig.h"
#include "apps/common/B2BModule.h"

#include "drivers/linux/BoardIO.h"

#define LINUX_GPIO_ROOT_DIR "/sys/class/gpio"
#define LINUX_GPIO_PIN_DIR_TEMPLATE LINUX_GPIO_ROOT_DIR "/gpio%d"
#define LINUX_GPIO_VALUE_FILE_TEMPLATE LINUX_GPIO_PIN_DIR_TEMPLATE "/value"

#define GPIO_BANK_TOTAL			4
#define GPIO_BITS_PER_BANK		32
#define GPIO_PORTNUM_TOTAL		(GPIO_BANK_TOTAL*GPIO_BITS_PER_BANK)
#define GPIO_PORTNUM_MAX		(GPIO_PORTNUM_TOTAL-1)

// Some macro-functons.  In all macros, p is IOPortNum
//   Get Bank that contains GPIO IOPortNum.
#define GPIO_PORTNUM_BANK(p)		((p) >> 5)
//   Create BitMask inside bank for GPIO IOPortNum.
#define GPIO_PORTNUM_TO_BITMASK(p)   (1 << ((p) & (GPIO_BITS_PER_BANK-1)))
//   Return true if GPIO IOPortNum is a valid port, false otherwise.
#define GPIO_PORTNUM_VALID(p)          ((p) < GPIO_PORTNUM_TOTAL)

class GPIODriverHW : public B2BModule {

  public:
	GPIODriverHW() :
		B2BModule("GPIODriverHW")
	{
		// FIXME: if we switch to memory mapped IO, set up here
	}

	virtual ~GPIODriverHW()
	{
		for (Bank bank = 0; bank < GPIO_BANK_TOTAL; ++bank) {
			// FIXME: if we switch to memory mapped IO, tear down up here
		}
	}

	// START: required virtual from B2BModule.  See that class for doc
	bool Init() { return true; }
	bool Start(void *arg) { return true; }
	bool Stop(void **returnVal) { return true; }
	// END: required virtual from B2BModule.  See that class for doc

	typedef uint32_t BitMask;
	typedef uint8_t Bank;

	// Enable pin to be used as GPIO
	bool Create(IOConfig::IOPortNum portNum);

	// Set GPIO to be an input
	bool ConfigAsInput(IOConfig::IOPortNum portNum)
	{
		if (!GPIO_PORTNUM_VALID(portNum))
			return false; // FAIL

		// FIXME: direct register access would do this: reg[bank]->OE |= bit;
		//BitMask bit = GPIO_PORTNUM_TO_BITMASK(portNum);
        //Bank bank = GPIO_PORTNUM_BANK(portNum);

		char cmd[256];
		sprintf(cmd, "echo in > " LINUX_GPIO_PIN_DIR_TEMPLATE "/direction",
						(int)portNum);
		EXEC_COMMAND_LINE(cmd);

		return true; // SUCCESS
	}

	// Set GPIO to be edge triggered
	// FIXME: we support rising or falling.   How about adding "both"?
	bool ConfigAsEdgeTriggered(IOConfig::IOPortNum portNum, bool rising)
	{
		if (!GPIO_PORTNUM_VALID(portNum))
			return false; // FAIL

		// FIXME: direct register access would do this: ???
		//BitMask bit = GPIO_PORTNUM_TO_BITMASK(portNum);
        //Bank bank = GPIO_PORTNUM_BANK(portNum);

		char cmd[256];
		sprintf(cmd, "echo %s > " LINUX_GPIO_PIN_DIR_TEMPLATE "/edge",
					(rising ? "rising" : "falling"), (int)portNum);
		EXEC_COMMAND_LINE(cmd);

		return true; // SUCCESS
	}

	// Set GPIO to be an output
	bool ConfigAsOutput(IOConfig::IOPortNum portNum)
	{
		if (!GPIO_PORTNUM_VALID(portNum))
			return false; // FAIL


		// FIXME: direct register access would do this: reg[bank]->OE &= ~bit;
		//BitMask bit = GPIO_PORTNUM_TO_BITMASK(portNum);
        //Bank bank = GPIO_PORTNUM_BANK(portNum);

		char cmd[256];
		sprintf(cmd, "echo out > " LINUX_GPIO_PIN_DIR_TEMPLATE "/direction",
						(int)portNum);
		EXEC_COMMAND_LINE(cmd);

		return true; // SUCCESS
	}

	// RETURNS: pin direction: 1 if output, 0 if input.
	//				-1 on error
	int DirectionGet(IOConfig::IOPortNum portNum);

	// Set GPIO pin high
	bool SetHigh(IOConfig::IOPortNum portNum)
	{
		if (!GPIO_PORTNUM_VALID(portNum))
			return false; // FAIL


		// FIXME: direct register access would do this:
		//		reg[bank]->SETDATAOUT = bit;
		// We use SETDATAOUT register and not DATAOUT so that we can set the 
		// bit atomically.
		//BitMask bit = GPIO_PORTNUM_TO_BITMASK(portNum);
        //Bank bank = GPIO_PORTNUM_BANK(portNum);

		char cmd[256];
		sprintf(cmd, "echo 1 > " LINUX_GPIO_VALUE_FILE_TEMPLATE, (int)portNum);
		EXEC_COMMAND_LINE(cmd);

		return true; // SUCCESS
	}

	// Set GPIO pin low
	bool SetLow(IOConfig::IOPortNum portNum)
	{
		if (!GPIO_PORTNUM_VALID(portNum))
			return false; // FAIL

		// FIXME: direct register access would do this:
		//		reg[bank]->CLEARDATAOUT = bit;
		// We use CLEARDATAOUT register and not DATAOUT so that we can set the 
		// bit atomically.
		//BitMask bit = GPIO_PORTNUM_TO_BITMASK(portNum);
        //Bank bank = GPIO_PORTNUM_BANK(portNum);

		char cmd[256];
		sprintf(cmd, "echo 0 > " LINUX_GPIO_VALUE_FILE_TEMPLATE, (int)portNum);
		EXEC_COMMAND_LINE(cmd);

		return true; // SUCCESS
	}

	// Set GPIO pin to level: true means high, false means low
	bool Set(IOConfig::IOPortNum portNum, bool level)
	{
		if (level)
			return SetHigh(portNum);
		else
			return SetLow(portNum);
	}

	// Set GPIO pin to "active" state, based on polarity
	// activeHigh: if true, pin is active high; if false, pin is active low
	// active: true means set pin "active"; false means set pin "inactive"
	bool SetWithPolarity(IOConfig::IOPortNum portNum, bool activeHigh,
						 bool active)
	{
		if ((activeHigh && active) || (!activeHigh && !active))
			return SetHigh(portNum);
		else
			return SetLow(portNum);
	}
	bool SetActive(IOConfig::IOPortNum portNum, bool activeHigh)
		{ return SetWithPolarity(portNum, activeHigh, true); }
	bool SetInactive(IOConfig::IOPortNum portNum, bool activeHigh)
		{ return SetWithPolarity(portNum, activeHigh, false); }

	// RETURNS: Return 0 if GPIO is low, 1 if high, and -1 on error.
	int8_t Read(IOConfig::IOPortNum portNum);

  private:
	// Helper function that reads an integer from fname.  label used for logging
	// RETURNS: the integer 0 or 1, -1 on error
	int8_t Read0Or1FromFile(const char *fname, const char *label);
};
