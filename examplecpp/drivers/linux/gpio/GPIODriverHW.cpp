#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

#include "common/FileUtil.h"
#include "log/B2BLog.h"

#include "drivers/linux/gpio/GPIODriverHW.h"

bool GPIODriverHW::Create(IOConfig::IOPortNum portNum)
{
	if (!GPIO_PORTNUM_VALID(portNum))
		return false; // FAIL

	char tempStr[256];
	sprintf(tempStr, LINUX_GPIO_PIN_DIR_TEMPLATE, (int)portNum);
	if (FileUtil::Exists(tempStr)) {
		return true; // SUCCESS: GPIO pin portNum is enabled (export not needed)
	}

	// PAY ATTENTION: Do we need to call config-pin too?
	//   From my testing with debian 4.4.x, outputting to export file always 
	//   sets the pin to GPIO mode.   Note that I didn't test every pin.

	// Enable GPIO pin portNum
	// This creates folder gpio<portNum> inside /sys/class/gpio
	sprintf(tempStr, "echo %d > " LINUX_GPIO_ROOT_DIR "/export", (int)portNum);
	EXEC_COMMAND_LINE(tempStr);

	return true; // SUCCESS
}

int8_t GPIODriverHW::Read0Or1FromFile(const char *fname, const char *label)
{
 
	int fd = open(fname, O_RDONLY);
	if (fd < 0) {
		B2BLog::Err(LogFilt::LM_DRIVERS,
					"FAIL: GPIO::%s cannot open %s error: %s(%d)", label, fname,
		                        						strerror(errno), errno);
		return -1; // FAIL
	}
 
	char result[10] = {0};
	int len = read(fd, result, sizeof(result - 1));

	if (len < 0) {
		B2BLog::Err(LogFilt::LM_DRIVERS,
					"FAIL: GPIO::%s %s failed error: %s(%d)", label, fname,
		                        						strerror(errno), errno);
		close(fd);
		return -1; // FAIL
	}

	if (len == 0) {
		B2BLog::Err(LogFilt::LM_DRIVERS,
				"FAIL: GPIO::%s %s failed: buffer empty", label, fname);
		close(fd);
		return -1; // FAIL
	}

	close(fd);

	// if we get here, all is good
	result[len] ='\0';

	int8_t value = strtoul(result, NULL, 10); 
		
	if (value > 1) 
		return -1; // FAIL
	else
		return value; // SUCCESS
}

int GPIODriverHW::DirectionGet(IOConfig::IOPortNum portNum)
{
	if (!GPIO_PORTNUM_VALID(portNum))
		return -1; // FAIL

	//BitMask bit = GPIO_PORTNUM_TO_BITMASK(portNum);
    //Bank bank = GPIO_PORTNUM_BANK(portNum);
	// FIXME: direct register access return this (note ARM is 1
	// 		if input, 0 if output which is opposite of this method):
	//	reg[bank]->OE & bit ? 0 : 1;

	char fname[80];
	sprintf(fname, LINUX_GPIO_PIN_DIR_TEMPLATE "/direction", (int)portNum);
 
	const char *label = "DirectionGet";
	int fd = open(fname, O_RDONLY);
	if (fd < 0) {
		B2BLog::Err(LogFilt::LM_DRIVERS,
					"FAIL: GPIO::%s cannot open %s error: %s(%d)", label, fname,
		                        						strerror(errno), errno);
		return -1; // FAIL
	}
 
	char result[10] = {0};
	int len = read(fd, result, sizeof(result - 1));

	if (len < 0) {
		B2BLog::Err(LogFilt::LM_DRIVERS,
					"FAIL: GPIO::%s %s failed error: %s(%d)", label, fname,
		                        						strerror(errno), errno);
		close(fd);
		return -1; // FAIL
	}

	if (len == 0) {
		B2BLog::Err(LogFilt::LM_DRIVERS,
				"FAIL: GPIO::%s %s failed: buffer empty", label, fname);
		close(fd);
		return -1; // FAIL
	}

	close(fd);

	// if we get here, all is good
	result[len] ='\0';

	if (strcmp(result, "out") == 0)
		return 1; // SUCCESS
	else if (strcmp(result, "in") == 0)
		return 0; // SUCCESS
	else
		return -1; // FAIL
}

int8_t GPIODriverHW::Read(IOConfig::IOPortNum portNum)
{
	if (!GPIO_PORTNUM_VALID(portNum))
		return -1; // FAIL

	//BitMask bit = GPIO_PORTNUM_TO_BITMASK(portNum);
    //Bank bank = GPIO_PORTNUM_BANK(portNum);
	// FIXME: direct register access return this:
	// 		(reg[bank]->DATAIN & bit) ? 1 : 0

	char fname[80];
	sprintf(fname, LINUX_GPIO_VALUE_FILE_TEMPLATE, (int)portNum);
	return Read0Or1FromFile(fname, "Read");
}
