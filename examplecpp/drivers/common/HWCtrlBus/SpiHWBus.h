#pragma once
//
// File - SpiHWBus.h
//
// Description -  Interface to a SPI HW bus
// 		All the run-time configuration was implemented to make this class
// 		flexible.  Thus this class supports different device types based
//		on BusMode.
//

#include "HWCtrlBus.h"

// Maximum read or write transfer size
// IMPROVE: read max spi buffer size from /sys/module/spidev/parameters/bufsiz
#define SPI_XFER_MAX_SIZE		4096

class SpiHWBus : public HWCtrlBus
{
public:
	// busId. identifies hardware SPI bus #.  Starts counting at 0.
	// 		Hardware SPI bus # is always ARM bus #, which starts at 0.
	// 		WARNING: linux /dev/spidev starts counting at 1.  Thus linux
	// 			/dev/spidev-1.0 is ARM bus 0, /dev/spidev-2.0 is 1, etc.
	// 			Specifically, the linux version of this class will open
	// 			device /dev/spidev-x.0 where x is ARM busId+1.
	// clockHZ: The SPI SCL clock in Hz.  
	SpiHWBus(BusId busId, BusClockRateHZ clockHZ=BUS_CLOCK_RATE_HZ_DEFAULT);
    virtual ~SpiHWBus(void);

	// START: HWCtrlBus class; see its documentation of the following functions
	
	bool IsValid() { return m_file >= 0; }

	// SlaveSetup() documentation specific to SPI:
	//   DeviceAddress is chip select # on the SPI bus.  CS0 is 0, CS1 is 1, etc
	//     AM335x only: McSPI DeviceAddress is limited to 0-3 and only 0-1 are
	// 		 pinned out to a CS pin on AM335x.  If you want to use DeviceAddress
	// 		 2 or 3, you will have to control any chip select yourself and not
	// 		 via this class.  If devAddr is out of range, this function fails.  
	//     NOTE: Caller controls how chip select is used via busMode.spi.csMode
	bool SlaveSetup(DeviceAddress devAddr, const BusMode &busMode);

	// PAY ATTENTION: len cannot be > SPI_XFER_MAX_SIZE
	bool MasterWrite(SubAddress subAddr, const uint8_t *buf, uint32_t len);
	bool MasterWriteRaw(const uint8_t *buf, uint32_t len)
		{ return MasterWriteExecute(buf, len, "MasterWriteRaw"); }
	// PAY ATTENTION: xferLen cannot be > SPI_XFER_MAX_SIZE
	bool MasterWriteMulti(const SubAddress *subAddrList, const uint8_t *buf,
								uint32_t totalSubAddr, uint32_t xferLen);
	bool MasterRead(SubAddress subAddr, uint8_t *buf, uint32_t len);
	bool MasterReadMulti(const SubAddress *subAddrList, uint8_t *buf, 
								uint32_t totalSubAddr, uint32_t xferLen);

	// See HWCtrlBus for details on this method.
	bool OSDriverSwapsBytes() const
	  {
		// Linux OS driver: generic buffers are same as SPI bus byte order
		return false;
	  }
	// END: HWCtrlBus class

	//*** START: old deprecated ADIS full-duplex methods
	// Start read transaction at address
	bool FullDuplexRegister16(int address);

	// Read() reads from previous register request
	// data: written with 16-bit data from last Register/ReadAndRequestNextRead
	bool FullDuplexRead16(int& data);

	// ReadAndRequestNextRead() does full duplex read (read and request next
	// within the same data frame).  More efficient than Register/Read
	// for multiple register reads.
	// regAddr: 8-bit register offset
	// data: written with 16-bit data from last Register/ReadAndRequestNextRead
	bool FullDuplexReadAndRequestNextRead16(int &data, int address);
	//*** END: old deprecated ADIS full-duplex methods

	// Of all our SPI parts, ADIS16364 has smallest max at 300KHz (in low power
	// mode).  Let's choose default to be a little slower than that, 250KHz.
	static const BusClockRateHZ BUS_CLOCK_RATE_HZ_DEFAULT = 250 * 1000; //250KHz

private:
	void Init(BusId busId, BusClockRateHZ clockHZ);

	// MasterWrite() helper method that returns a transmit buffer "x" for
	// use with SPIWriteRead().  Caller must do delete []x when done with "x".
	// subAddr: sub-address to write to
	// buf: original data to write
	// *pLen: length, in bytes, of original data to write.
	// 		  Changed to length of data in buffer "x".
	uint8_t *MasterWriteCreateBuffer(SubAddress subAddr, const uint8_t *buf,
									uint32_t *pLen) const;
	// Calls OS to do Write.
	// txbuf: contains data to transmit to SPI device
	// len: number of bytes to write
	// label(DEBUG ONLY): used in logging
	bool MasterWriteExecute(const uint8_t *txbuf, uint32_t len, 
							const char *label);

	// MasterRead() helper method that returns a transmit buffer "x" for
	// use with SPIWriteRead().  Caller must do delete []x when done with "x".
	// subAddr: sub-address to read from
	// *pLen: Changed to length of data in buffer "x".
	uint8_t *MasterReadCreateBuffer(SubAddress subAddr, uint32_t *pLen) const;

	// MasterRead() helper method that takes data returned by SPIWriteRead()
	// and fixes it to be buffer expected by MasterRead()
	// buf: buffer to write MasterRead() data to
	// len: length, in bytes, of buf
	// spibuf: buffer received from SPIWriteRead().  When done with spibuf, this
	//		method does a delete []spibuf
	// spibufLen: length, in bytes, of spibuf
	void MasterReadCreateContinguousBuffer(uint8_t *buf, uint32_t len, 
								const uint8_t *spibuf, uint32_t spibufLen) const;

	// Calls OS to do full duplex Write/Read.   
	// txbuf: contains data to transmit to SPI device (required to do read)
	// rxbuf: Data read from SPI device is written into rxbuf
	// len: number of bytes to read
	// label(DEBUG ONLY): used in logging
	bool MasterReadExecute(const uint8_t *txbuf, uint8_t *rxbuf, uint32_t len,
								 const char *label);

	int m_file;		// SPI device
};
