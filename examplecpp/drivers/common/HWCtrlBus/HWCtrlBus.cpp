//
// File - HWCtrlBus.cpp
//
// Description -  Generic class interface to a HW bus.
//
#include "HWCtrlBus.h"

HWCtrlBus::HWCtrlBus(BusType busType, BusId busId, BusClockRateHZ clockHZ) :
		m_busType(busType),
		m_busId(busId),
		m_clockHZ(clockHZ),
		m_lastDevAddr(DEVADDR_INVALID)
		//m_lastBusMode() // set by its ctor
{
}

HWCtrlBus::~HWCtrlBus(void)
{
}

/*static*/ bool HWCtrlBus::ErrorCheckBusMode(const BusMode &busMode)
{
	switch (busMode.subAddrLenBytes) {
	  case 1:
	  case 2:
	  case 4:
		// good value, do nothing
		break;
	  default:
		return false; // FAIL
	}
	switch (busMode.wordLenBytes) {
	  case 1:
	  case 2:
	  case 4:
		// good value, do nothing
		break;
	  default:
		return false; // FAIL
	}

	return true; // PASS
}

/*static*/ void HWCtrlBus::SwapBytes(const uint8_t *fromBuf, uint8_t *toBuf,
										uint32_t len, uint8_t wordLenBytes)
{
	for (uint32_t k = 0; k < len; k += wordLenBytes) {
		if (wordLenBytes == 2) {
			if ((k+1) >= len) break;
			if (fromBuf == toBuf) {
				uint8_t swapByte = toBuf[k+1];
				toBuf[k+1] = fromBuf[k];
				toBuf[k] = swapByte;
			} else {
				toBuf[k+1] = fromBuf[k];
				toBuf[k] = fromBuf[k+1];
			}
		} else if (wordLenBytes == 4) {
			if ((k+3) >= len) break;
			if (fromBuf == toBuf) {
				uint8_t swapByte = toBuf[k+3];
				toBuf[k+3] = fromBuf[k];
				toBuf[k] = swapByte;
				swapByte = toBuf[k+2];
				toBuf[k+2] = fromBuf[k+1];
				toBuf[k+1] = swapByte;
			} else {
				toBuf[k+3] = fromBuf[k];
				toBuf[k+2] = fromBuf[k+1];
				toBuf[k+1] = fromBuf[k+2];
				toBuf[k] = fromBuf[k+3];
			}
		} // else: not supported, do nothing
	}
}
