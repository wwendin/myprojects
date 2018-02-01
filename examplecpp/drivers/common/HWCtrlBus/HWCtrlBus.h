#pragma once
//
// File - HWCtrlBus.h
//
// Description -  Generic class interface to a HW bus.
//   This type of hardware bus can write (send) and read (receive) data.  We
//   only support bus master mode with this class.
//
//   There are two types of access to write or read a sensor:
//   	Block: This is a contiguous block.  Can be from 1 to X bytes.
//   			Use MasterWrite() and MasterRead().
//   	Multi: Scattered non-continguous access.
//   			Use MasterWriteMulti() and MasterReadMulti().
//
//  Details
//   This class has the following high level features:
//     BusId
//     		Instance of hardware bus (for example, I2C 0, SPI 0, etc)
//     DeviceAddress
//     		Abstract "address" for specifying a device on the bus:
//              	I2C: this is the slave address.
//              	SPI: this is the chip select number.
//     SubAddress
//     		This is the address (offset) inside the device where we want to
//     		read or write.  
//     BusMode
//     		Tells us how to use the sub-address on the hardware bus and how to
//     		read/write data on bus.  This run-time configuration is required
//     		because most devices differ slightly in how they are addressed
//     		and how they operate on I2C and SPI.
//
//   Any transfer length of 0 is an error and method will return false.
//
#include <stdint.h>
#include <string.h>
#include <endian.h>

class HWCtrlBus
{
public:
	// type: type of bus we are creating
	// busId: generic ID that identifies which instance of bus to use
	// clockHZ: clock rate in Hz
	typedef enum {
		HWCTRLBUS_TYPE_I2C = 0,
		HWCTRLBUS_TYPE_SPI,
		HWCTRLBUS_TYPE_TOTAL // size of enum (never used as a valid value)
	} BusType;
	typedef uint8_t BusId;
	typedef uint32_t BusClockRateHZ; // HW clock rate.
	HWCtrlBus(BusType busType, BusId busId, BusClockRateHZ clockHZ);
    virtual ~HWCtrlBus(void);

	// Abstract "address" for specifying a device on the bus:
	typedef uint8_t DeviceAddress;
	static const DeviceAddress DEVADDR_INVALID = 0xff;

	// Sub-address is the address (often called offset) inside device where
	// we start the send or receive.  Only sub-address sizes of 0-4 bytes are
	// supported.  How the sub-address is used is defined by BusMode
	typedef uint32_t SubAddress;

	// BusMode
	// This structure defines how we define a sub-address and how we
	// read and write data on the bus.  
	// subAddrLenBytes: The size of a sub-address in bytes
	// 		Only values of 1, 2 or 4 are allowed here.
	// autoIncrMask: if != 0, must add to subAddr in order to read/write
	// 		multiple cycles that automatically increment subAddr.
	// 		If we don't set it and it is required, we will read/write same
	// 		location over and over.
	// 		If it is 0, auto-increment doesn't require a mask.
	// 		If it is BUSMODE_AUTOINCR_NOT_AVAILABLE, auto increment is not
	// 		supported by the device and sub-address must be output again for
	// 		each write/read of wordLenBytes.
	// wordLenBytes: word size on bus in bytes.
	// 		This is the minimum amount of bytes that must be written
	// 		or read by the HW in a transaction.
	// 		Affects how things are done if autoIncrMask is NOT_AVAILABLE.
	// 		Only values of 1, 2 or 4 are allowed here.
	// 		I2C: Always set to 1 for I2C future compatability.
	// 		SPI: Affects where CS occurs if spi.csMode is MULTI
	// bigEndianReg: true if device's registers are big endian, false if not.
	//		This only affects only MasterWriteUINT16/32 and MasterReadUINT16/32
	// All of the following i2c structure values are ignored by SPI:
	//   i2c.writeStopRequiredBeforeRead
	//		Controls if we do a restart(normal) or stop between write and read
	//			when doing a MasterRead operation
	//		true: Non-conforming device that wants a stop between write and read
	//		false: normal parts, does a restart between write and read,
	// All of the following spi structure values are ignored by I2C:
	//   spi.readWriteMask
	// 		Must use this mask with SPI sub-address to denote read or write.
	//   spi.readSetsMask:
	// 		If spiReadSetsMask is true, we add spiReadWriteMask when reading.
	// 		If spiReadSetsMask is false, we add spiReadWriteMask when writing.
	//   spi.fullDuplex
	// 		If true, SPI can read data while writing it.
	// 		If false, read/write transactions cannot overlap
	// 		Although all 4 wire SPI is usually full duplex, some parts don't
	// 		support full duplex.  For example, STMicro parts do things
	// 		serially for both 4-wire and 3-wire SPI.
	//   spi.dontCareSubAddr: for full duplex parts, the read of the last word
	// 		must clock out some read address.  There is no way around it if you
	// 		want to take advantage of full duplex.
	// 		This value defines which register is read on that last bus cycle.
	// 		If bus isn't using full duplex, dontCareSubAddr is not used.
	//   spi.csMode  
	//   	NONE: do not alter CS.  Client class is responsible for controlling
	//   		CS or no CS control is required.
	//   	SINGLE: the CS is asserted before the first transfer and remains
	//   		there until the last transfer in the MultiXXX request.
	//   	MULTI: the CS line is deasserted and then asserted
	//   		every wordLenBytes.  (ADIS parts require MULTI)
	//   spi.csStallDelayUS  
	//   	Minimum delay, in microseconds, between between CS deassert and CS
	//   	assert.  Some parts (like ADIS) require this.  
	// 		Set to 0 to disable this feature.
	//   spi.csDataRateDelayUS  
	//   	Minimum delay, in microseconds, between CS asserts.  Some parts
	//   	(like ADIS) require this to limit overall data rate.
	// 		Set to 0 to disable this feature.
	// 		PAY ATTENTION: Our SPI driver is smart here.  If the time to output
	// 				wordLenBytes on the SPI bus, plus csStallDelayUS is already
	// 				>= csDataRateDelayUS, the SPI driver sets csDataRateDelayUS
	// 				to 0 internally to disabled this feature.
	//   	Adeneo WEC7 driver doesn't have this feature; SpiHWBus implements it
	//   spi.threeWire: if true, SPI uses 3 wires (CS, SCLK, SDATA).  If false,
	// 		SPI uses 4 wires  (CS, SCLK, SDATAIN, SDATAOUT).
	//      IMPROVE: SPI 3-wire true is not supported yet.  Can add later if
	//      we have such a device.
	//
	// HOW IT WORKS
	// I2C: 
	//   All bus modes are currently performed using the same technique
	//   because all our devices work the same (no weird sub-addressing yet):
	//   * the sub-address byte(s) are written out using write-mode
	//     (sub-address gets autoIncrMask if length of access is > 1)
	//   * if writing, the data byte(s) are written to the bus.  Nothing special
	//     need to occur because we are already in write-mode
	//   * if reading, a repeated start is issued, then a read-mode, then
	//     bytes are read.
	// SPI: 
	//   A few facts: 
	//   * read data arrives after you write out sub-address.  This can occur
	//   	at same time as next sub-address if spi.fullDuplex is true.
	//   * if subAddrLenBytes < wordLenBytes, then writes can send some
	//     data out in same word as sub-address.  Reads cannot (because
	//     of how devices work, not because of SPI limitations).
	//   Bus access:
	//   * the sub-address byte(s) are written out
	//     (sub-address has spiReadWriteMask set according to spiReadSetsMask)
	//     (sub-address has autoIncrMask set if length of access is > 1)
	//   * if writing, write out all the bytes.  If wordLenBytes != 0,
	//     and length of output is > wordLenBytes, we must rewrite
	//     the sub-address for every wordLenBytes bytes we want to write.
	//   * if reading, read in all the bytes.   If wordLenBytes != 0,
	//     then we have to reissue the sub-address for every wordLenBytes
	//     bytes we want to read.    Also if wordLenBytes > subAddrLenBytes,
	//     the byte response will be after wordLenBytes.  See ADISxxxx parts
	//     for example of this.
	//     
	typedef enum {
		HWCTRLBUS_CSMODE_NONE = 0,
		HWCTRLBUS_CSMODE_SINGLE,
		HWCTRLBUS_CSMODE_MULTI,
		HWCTRLBUS_CSMODE_TOTAL // size of enum (never used as a valid value)
	} CSMode;
	class BusMode {
	  public:
		BusMode() :
			subAddrLenBytes(1),
			autoIncrMask(0), // HW doesn't need auto-increment mask
			wordLenBytes(1),
			bigEndianReg(true)  // assume hardware registers are big endian
		{
			i2c.writeStopRequiredBeforeRead = false; // normal I2C device
			spi.readWriteMask = 0x80; // bit to set in sub-address
      		spi.readSetsMask = false; // set readWriteMask when writing
      		spi.fullDuplex = true;
      		spi.dontCareSubAddr = 0x0;
      		spi.csMode = HWCtrlBus::HWCTRLBUS_CSMODE_MULTI; // need CS change at every wordLenBytes
      		spi.csStallDelayUS = 0;
      		spi.csDataRateDelayUS = 0;
      		spi.threeWire = false;
		}

		uint8_t subAddrLenBytes;
		uint32_t autoIncrMask;
		uint8_t wordLenBytes;
		bool bigEndianReg;
		struct {
			bool writeStopRequiredBeforeRead;
		} i2c;
		struct {
			uint32_t readWriteMask;
			bool readSetsMask;
			bool fullDuplex;
			uint32_t dontCareSubAddr;
			CSMode csMode;
			uint32_t csStallDelayUS;
			uint32_t csDataRateDelayUS;
			bool threeWire;
		} spi;
	};
	static const uint8_t BUSMODE_SUBADDRLEN_INVALID = 0xff;
	static const uint32_t BUSMODE_AUTOINCR_NOT_AVAILABLE = 0xffffffff;

	// limit XXXRegMulti's totalSubAddr to be safe
	static const uint32_t MULTIXFER_MAX_SIZE = 60;

	// Returns true if class instance is valid.  Returns false if something
	// bad happened initializing the class and it is unusable.
	virtual bool IsValid() = 0;

	// Set target device's slave address to devAddr and sets the bus mode.
	// Returns true on success, false otherwise.
	// This class is flexible and SlaveSetup() can be called at any time.
	// However, it is faster and more efficient to create one instance
	// of this class for each device on the bus and only call SlaveSetup once
	// at startup; this technique means less register read/writes to setup
	// the slave in hardware.
	virtual bool SlaveSetup(DeviceAddress devAddr, const BusMode &busMode) = 0;
	// Same as above, but only sets devAddr.   SlaveSetup() w/ bus mode MUST
	// have been called some time in the past, or caller must be satified
	// with default BusMode settings.
	bool SlaveSetupUsingExistingBusMode(DeviceAddress devAddr) 
	  { return SlaveSetup(devAddr, m_lastBusMode); }

	// Write len bytes from buf to device location starting at subAddr
	// devAddr: target device's slave address
	// busMode: target device's bus mode
	// Returns true on success, false otherwise.
	// WARNING: Calling SlaveSetup once and MasterWrite(subAddr, buf, len) is
	// 			faster, more efficient technique.
	bool MasterWrite(DeviceAddress devAddr, const BusMode &busMode,
						SubAddress subAddr, const uint8_t *buf, uint32_t len)
	{ 
		if (devAddr != m_lastDevAddr) {
			if (!SlaveSetup(devAddr, busMode)) return false;
		}
		return MasterWrite(subAddr, buf, len);
	}

	// Same as above MasterWrite() but BusMode MUST
	// have been set by SlaveSetup() or by previous MasterXXX()
	// Useful for devices that have more than one slave address, and
	// busMode does not change
	bool MasterWrite(DeviceAddress devAddr,
						SubAddress subAddr, const uint8_t *buf, uint32_t len)
	{ 
		return MasterWrite(devAddr, m_lastBusMode, subAddr, buf, len);
	}

	// Same as above MasterWrite() but DeviceAddress and BusMode MUST
	// have been set by SlaveSetup() or by previous MasterXXX()
	virtual bool MasterWrite(SubAddress subAddr, const uint8_t *buf,
							 uint32_t len)
	{ return false; }

	// Helper routines for 1, 2, and 4-byte MasterWrite
	bool MasterWriteUINT8(SubAddress subAddr, uint8_t value)
	{ 
		return MasterWrite(subAddr, &value, 1);
	}
	bool MasterWriteUINT16(SubAddress subAddr, uint16_t value)
	{
		if (m_lastBusMode.bigEndianReg)
			value = htobe16(value);
		else
			value = htole16(value);
		return MasterWrite(subAddr, (uint8_t *)&value, 2);
	}
	bool MasterWriteUINT32(SubAddress subAddr, uint32_t value)
	{
		if (m_lastBusMode.bigEndianReg)
			value = htobe32(value);
		else
			value = htole32(value);
		return MasterWrite(subAddr, (uint8_t *)&value, 4);
	}

	// Write len bytes from buf to device.
	// devAddr: target device's slave address
	// busMode: target device's bus mode
	// Returns true on success, false otherwise.
	// WARNING: only use this raw feature if you really know what you are doing
	// WARNING: Calling SlaveSetup once and MasterWrite(subAddr, buf, len) is
	// 			faster, more efficient technique.
	bool MasterWriteRaw(DeviceAddress devAddr, const BusMode &busMode,
						const uint8_t *buf, uint32_t len)
	{ 
		if (devAddr != m_lastDevAddr) {
			if (!SlaveSetup(devAddr, busMode)) return false;
		}
		return MasterWriteRaw(buf, len);
	}

	// Same as above MasterWriteRaw() but BusMode MUST
	// have been set by SlaveSetup() or by previous MasterXXX()
	// Useful for devices that have more than one slave address, and
	// busMode does not change
	bool MasterWriteRaw(DeviceAddress devAddr,
						const uint8_t *buf, uint32_t len)
	{ 
		return MasterWriteRaw(devAddr, m_lastBusMode, buf, len);
	}

	// Same as above MasterWriteRaw() but DeviceAddress and BusMode MUST
	// have been set by SlaveSetup() or by previous MasterXXX()
	// WARNING: only use this raw feature if you really know what you are doing
	virtual bool MasterWriteRaw(const uint8_t *buf, uint32_t len) = 0;

	// Write multiple sub-address locations in subAddrList with values from buf.
	// totalSubAddr: defines how many values are in subAddrList.
	// 		Maximum totalSubAddr value allowed is MULTIXFER_MAX_SIZE.
	// xferLen: defines how many bytes to write to each sub-address location.
	// Specifically: subAddrList[0] location = buf[0]
	// 				 subAddrList[0] location = buf[1]
	// 				           .
	// 				 subAddrList[totalSubAddr-1] location = buf[totalSubAddr-1]
	// Returns true on success, false on failure.
	// Returns true if totalSubAddr is 0.
	virtual bool MasterWriteMulti(const SubAddress *subAddrList, 
								  const uint8_t *buf, 
								  uint32_t totalSubAddr, uint32_t xferLen)
	  { return false; }

	// Read len bytes from device into buf
	// devAddr: target device's slave address
	// busMode: target device's bus mode
	// Returns true on success, false otherwise.
	// WARNING: Calling SlaveSetup once and MasterRead(subAddr, buf, len) is
	// 			faster, more efficient technique.
	bool MasterRead(DeviceAddress devAddr, const BusMode &busMode,
							SubAddress subAddr, uint8_t *buf, uint32_t len)
	{
		if (devAddr != m_lastDevAddr) {
			if (!SlaveSetup(devAddr, busMode)) return false;
		}
		return MasterRead(subAddr, buf, len);
	}

	// Same as above MasterRead() but BusMode MUST
	// have been set by SlaveSetup() or by previous MasterXXX()
	// Useful for devices that have more than one slave address, and
	// busMode does not change
	bool MasterRead(DeviceAddress devAddr,
							SubAddress subAddr, uint8_t *buf, uint32_t len)
	{
		return MasterRead(devAddr, m_lastBusMode, subAddr, buf, len);
	}

	// Same as above MasterRead() but DeviceAddress and BusMode MUST
	// have been set by SlaveSetup() or by previous MasterXXX()
	virtual bool MasterRead(SubAddress subAddr, uint8_t *buf, uint32_t len)
	  { return false; }

	// Helper routines for 1, 2, and 4-byte MasterRead
	bool MasterReadUINT8(SubAddress subAddr, uint8_t *pValue)
	{
		return MasterRead(subAddr, pValue, 1);
	}
	bool MasterReadUINT16(SubAddress subAddr, uint16_t *pValue)
	{
		uint16_t value;
		bool result = MasterRead(subAddr, (uint8_t *)&value, 2);
		if (m_lastBusMode.bigEndianReg)
			*pValue = be16toh(value);
		else
			*pValue = le16toh(value);
		return result;
	}
	bool MasterReadUINT32(SubAddress subAddr, uint32_t *pValue)
	{
		uint32_t value;
		bool result = MasterRead(subAddr, (uint8_t *)&value, 4);
		if (m_lastBusMode.bigEndianReg)
			*pValue = be32toh(value);
		else
			*pValue = le32toh(value);
		return result;
	}

	// Read multiple sub-address locations in subAddrList and store in buf.
	// totalSubAddr: defines how many values are in subAddrList.
	// 		Maximum totalSubAddr value allowed is MULTIXFER_MAX_SIZE.
	// xferLen: defines how many bytes to read from each sub-address location.
	// Specifically: buf[0] = subAddrList[0] value
	// 				 buf[1] = subAddrList[1] value
	// 				           .
	// 				 buf[totalSubAddr-1] = subAddrList[totalSubAddr-1] value
	// Returns true on success, false on failure.
	// Returns true if totalSubAddr is 0.
	virtual bool MasterReadMulti(const SubAddress *subAddrList, uint8_t *buf, 
								uint32_t totalSubAddr, uint32_t xferLen)
	  { return false; }

	// Returns true if OS driver swaps bytes on reads and writes.
	// Returns false if OS driver preserves byte order on bus.
	// Discussion: SPI and I2C are bit streams.  So the generic buffers we pass
	// 	  to and from read/write routines should be output in same order on bus.
	//
	//    But the WEC7 SPI driver swaps bytes before putting things on the bus.
	//    The WEC7 SPI driver treats the raw buffers passed to it like
	//    streams of words of wordLenBytes size.  
	//    The WEC7 I2C driver does not do swapping, bus byte order is preserved.
	//    The Linux I2C and SPI drivers don't do byte swapping either.
	virtual bool OSDriverSwapsBytes() const = 0;

	BusType GetBusType() const { return m_busType; }
	DeviceAddress GetDeviceAddress() const { return m_lastDevAddr; }
	BusMode GetBusMode() const { return m_lastBusMode; }

	// return true if busMode looks good, false if it has some error.
	static bool ErrorCheckBusMode(const BusMode &busMode);

	// Swap bytes from fromBuf into toBuf.
	// wordLenBytes: word size for swapping.  Only 2 and 4 are supported.
	// 		Any other value will be a NOP (no bytes copied to toBuf).
	// If fromBuf and toBuf are same, this routine handles it.
	// If len is not an even multiple of wordLenBytes, the last short word
	// is not copied to toBuf.
	static void SwapBytes(const uint8_t *fromBuf, uint8_t *toBuf,
							uint32_t len, uint8_t wordLenBytes);

	// I2C is big endian: copy value into buf
	// valueSize: 1, 2, 3, or 4.   Denotes how many bytes are significant
	//			in value.   For example:
	//			* 1 means value is a uint8_t
	//			* 2 is uint16_t,
	//			* 3 means 3 bytes
	//			* 4 is uint32_t
	static void CopyValueToI2CBuffer(uint32_t value, uint8_t valueSize,
									 uint8_t *buf)
	{
		// Left justify subAddr
		value <<= ((4 - valueSize) * 8);
		value = htobe32(value);
		memcpy(buf, &value, valueSize);
	}

protected:
	BusType m_busType;			// from ctor
	BusId m_busId;				// from ctor
	BusClockRateHZ m_clockHZ;	// from ctor

	// last devAddr, bus mode passed to SlaveSetup() or MasterXXX()
	DeviceAddress m_lastDevAddr;
	BusMode m_lastBusMode;
};
