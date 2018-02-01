#pragma once
//
// This file contains definitions for BBB board: GPIOs, I2C, SPI, etc.
//

// Define how to execute something on the linux shell
#define DO_EXEC_COMMAND_LINE(c)	do { \
								  if (system(c)) \
									B2BLog::Debug(LogFilt::LM_DRIVERS, \
														"FAIL: %s", (c)); \
								} while(false)

// Use EXEC_COMMAND_LINE if you want non-BBB targets to be NOP
#ifdef TARGET_IS_BBB
  #define EXEC_COMMAND_LINE(c)	DO_EXEC_COMMAND_LINE(c)
#else // not TARGET_IS_BBB
  #define EXEC_COMMAND_LINE(c)	B2BLog::Debug(LogFilt::LM_DRIVERS, "%s", (c));
#endif // not TARGET_IS_BBB

//////// I2C
// BUSID is the AM335x I2C bus number: 0 to BOARD_I2C_BUSID_MAX
//BOARD_I2C_STMICRO_SLAVE_ADDR_START	
  #define BOARD_I2C_BUSID_TOTAL					3
  #define BOARD_I2C_BUSID_MAX					(BOARD_I2C_BUSID_TOTAL-1)

  ///// START: Define I2C addresses of parts not found in ioconfig json files

  // 7-bit I2C addresses assigned to STMicro ToF parts.
  #define BOARD_I2C_STMICRO_TOF_SLAVE_ADDR_START			0x29
  #define BOARD_I2C_STMICRO_TOF_SLAVE_ADDR_END				0x2f
  #define BOARD_I2C_STMICRO_TOF_SLAVE_ADDR(devAddr)	\
  	(((devAddr) >= BOARD_I2C_STMICRO_TOF_SLAVE_ADDR_START) && \
  	 ((devAddr) <= BOARD_I2C_STMICRO_TOF_SLAVE_ADDR_END))

  // 7-bit I2C address of BBB power cape devices on I2C bus 2 (I2C2)
  #define BOARD_I2C_POWER_CAPE_AVR_SLAVE_ADDR				0x21
  #define BOARD_I2C_POWER_CAPE_INA219_SLAVE_ADDR			0x40

  // 7-bit I2C addresses of MLX90621 EEPROM "registers"
  // The EEPROM internal to MLX90621 is part 24AA02 (responds to all addresses
  // in range 0x50-0x53).   But because our TMD4903 test board uses 0x50-51, we
  // will only use 0x52-0x53 here.
  #define BOARD_I2C_MLX90621_SLAVE_ADDR_EEPROM_START		0x52
  #define BOARD_I2C_MLX90621_SLAVE_ADDR_EEPROM_END			0x53
  #define BOARD_I2C_MLX90621_SLAVE_ADDR(devAddr)	\
  	(((devAddr) >= BOARD_I2C_MLX90621_SLAVE_ADDR_EEPROM_START) && \
  	 ((devAddr) <= BOARD_I2C_MLX90621_SLAVE_ADDR_EEPROM_END))

  // 7-bit I2C addresses assigned to BBB capes on I2C bus 2
  #define BOARD_I2C_BBBCAPE_SLAVE_ADDR_START				0x54
  #define BOARD_I2C_BBBCAPE_SLAVE_ADDR_END					0x57
  #define BOARD_I2C_BBBCAPE_SLAVE_ADDR(devAddr)	\
  	(((devAddr) >= BOARD_I2C_BBBCAPE_SLAVE_ADDR_START) && \
  	 ((devAddr) <= BOARD_I2C_BBBCAPE_SLAVE_ADDR_END))

  // 7-bit I2C address of MLX90621 IR ARRAY registers
  #define BOARD_I2C_MLX90621_SLAVE_ADDR_IR_ARRAY			0x60

  ///// END: Define I2C addresses of parts not found in ioconfig json files

  // ADS1015: We use Vdd of 3.3V on our board
  #define BOARD_ADS1x15_ADC_VOLTS_MAX						3.3f
  #define BOARD_ADS1x15_ADC_VOLTS_OFFSET					0.0f

//////// SPI
// BUSID is the AM335x SPI bus number: 0 to BOARD_SPI_BUSID_MAX
// Each AM335x SPI has chip selects: 0 to BOARD_SPI_CHIPSELECT_MAX
//
  #define BOARD_SPI_BUSID_TOTAL					2
  #define BOARD_SPI_BUSID_MAX					(BOARD_SPI_BUSID_TOTAL-1)
  #define BOARD_SPI_CHIPSELECT_TOTAL			2
  #define BOARD_SPI_CHIPSELECT_MAX				(BOARD_SPI_CHIPSELECT_TOTAL-1)
  // BBB universal-io file defines spi-max-frequency of 16MHz
  #define BOARD_SPI_CLOCK_HZ_MAX				(16*1000000)

//////// GPIO
// GPIO port number can be from 0 to GPIO_PORTNUM_MAX (defined in GPIDriverHW.h)
//

//////// ADC
// CHANID is the AM335x ADC channel number: 0 to BOARD_ADC_CHANID_MAX
// Notice that TOTAL is 7 because BBB does not allow us to use use
// all 8 channels (we are blocked from using AIN7 by the Linux OS)
// Readings are from 0 to 4095 (12-bit ADC)
// This corresponds (on BBB) to 0 to 1.8V
  #define BOARD_ADC_CHANID_TOTAL				7
  #define BOARD_ADC_CHANID_MAX					(BOARD_ADC_CHANID_TOTAL-1)
  #define BOARD_ADC_CHANID_INVALID				(BOARD_ADC_CHANID_MAX+1)
  #define BOARD_ADC_RAW_VALUE_MAX				4095
  #define BOARD_ADC_VOLTS_MAX					1.8f
  #define BOARD_ADC_VOLTS_OFFSET				0.0f
