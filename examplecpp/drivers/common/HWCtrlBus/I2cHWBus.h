#pragma once
//
// File - I2cHWBus.h
//
// Description -  Interface to a I2C HW bus
// 		All the run-time configuration was implemented to make this class
// 		flexible.  Thus this class supports different device types based
//		on BusMode.
//
#include "HWCtrlBus.h"

// Maximum read or write transfer size.  You may call MasterRead/MasterWrite
// family of functions with a length longer than this.   But caller beware
// that these methods will automatically break up the xfer into
// I2C_SMBUS_BLOCK_MAX pieces and bump SubAddress accordingly on each xfer.
// IMPROVE: Grab value from I2C_SMBUS_BLOCK_MAX
#define I2C_XFER_MAX_SIZE		32	

class I2cHWBus : public HWCtrlBus
{
public:
	// FYI: CE defaults to 400KHz if we don't set its baud rate
	static const BusClockRateHZ BUS_CLOCK_RATE_HZ_DEFAULT = 400 * 1000; //400KHz

	// busId. identifies hardware I2c bus #.  Starts counting at 0.
	// clockHZ: The I2C SCL clock in Hz.  
	I2cHWBus(BusId busId, BusClockRateHZ clockHZ=BUS_CLOCK_RATE_HZ_DEFAULT);
    virtual ~I2cHWBus(void);

	// START: HWCtrlBus class; see its documentation of the following functions

	bool IsValid() { return m_file >= 0; }

	// DeviceAddress is the 7-bit I2C device address.
	bool SlaveSetup(DeviceAddress devAddr, const BusMode &busMode);
	bool MasterWrite(SubAddress subAddr, const uint8_t *buf, uint32_t len);
	bool MasterWriteRaw(const uint8_t *buf, uint32_t len);
	bool MasterWriteMulti(const SubAddress *subAddrList, const uint8_t *buf, 
								uint32_t totalSubAddr, uint32_t xferLen);
	bool MasterRead(SubAddress subAddr, uint8_t *buf, uint32_t len);
	bool MasterReadMulti(const SubAddress *subAddrList, uint8_t *buf, 
								uint32_t totalSubAddr, uint32_t xferLen);
	// See HWCtrlBus for details on this method.
	// All OS drivers: Generic buffers are same as I2C bus order.
	bool OSDriverSwapsBytes() const { return false; }
	// END: HWCtrlBus class


private:
	void Init(BusId busId, BusClockRateHZ clockHZ);

	int m_file; // used by linux version for open I2c device
};
