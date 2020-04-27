#ifndef SRC_HEADER_FILES_SI7021_H_
#define SRC_HEADER_FILES_SI7021_H_


//***********************************************************************************
// Include files
//***********************************************************************************


//***********************************************************************************
// defined files
//***********************************************************************************
#define 	SI7021_DEV_ADDR			0x40
#define		SI7021_I2C_FREQ			I2C_FREQ_FAST_MAX // 400kHz max.
#define		SI7021_I2C_CLK_RATIO	i2cClockHLRAsymetric // 6:3 for fast max. try standard if having issues.
// for the Si7021 ROUTELOC value is #15 for both SDA and SCL (i2c0 on pc10 and 11)
#define		SI7021_SCL_LOC			SI7021_SCL_LOC_i2c1
#define		SI7021_SCL_LOC_i2c0			_I2C_ROUTELOC0_SCLLOC_LOC15
#define 	SI7021_SCL_LOC_i2c1			_I2C_ROUTELOC0_SCLLOC_LOC19

#define		SI7021_SCL_EN			true

#define		SI7021_SDA_LOC			SI7021_SDA_LOC_i2c1
#define		SI7021_SDA_LOC_i2c0			_I2C_ROUTELOC0_SDALOC_LOC15
#define		SI7021_SDA_LOC_i2c1			_I2C_ROUTELOC0_SDALOC_LOC19

#define		SI7021_SDA_EN			true
#define 	SI7021_I2C				I2C1 // default i2c for app use
#define 	SI7021_REF_FREQ			0 // HF peripheral clock

// Command Codes

#define		SI7021_TEMP_NO_HOLD		0xF3
#define		SI7021_RH_NO_HOLD		0xF5
#define		SI7021_TEMP_FROM_RH		0xE0
#define		SI7021_WRITE_UR1		0xE6
#define		SI7021_READ_UR1			0xE7

#define		SI7021_SNA_MSB			0xFA
#define		SI7021_SNA_LSB			0x0F
#define		SI7021_SNB_MSB			0xFC
#define		SI7021_SNB_LSB			0xC9

//Data Lengths
#define 	SI7021_NUM_BYTES_TEMP_CHECKSUM		3
#define		SI7021_NUM_BYTES_TEMP_NOCHECKSUM	2
#define		SI7021_NUM_BYTES_RH_CHECKSUM		3
#define		SI7021_NUM_BYTES_RH_NOCHECKSUM 		2
#define 	SI7021_NUM_BYTES_TEMP_FROM_RH		2
#define 	SI7021_NUM_BYTES_USER_REG			1
#define		SI7021_NUM_BYTES_SNA				8
#define		SI7021_NUM_BYTES_SNB				6

#define		SI7021_MAX_READ_BYTES				8
#define		SI7021_MAX_WRITE_BYTES				8

#define 	SI7021_MAX_CC_LENGTH				2
//***********************************************************************************
// global variables
//***********************************************************************************

void si7021_i2c_open(void);
void si7021_read(uint8_t command_code_length, uint8_t read_length, uint32_t event);
void si7021_write(uint8_t command_code_length, uint8_t write_length, uint32_t event);

// r/w presets
void si7021_read_rh(uint32_t event);
void si7021_read_temp(uint32_t event);
void si7021_read_rh_temp(uint32_t event);
void si7021_write_ur1(uint8_t byte, uint32_t event);
void si7021_read_ur1(uint32_t event);
void si7021_read_SNB(uint32_t event);

// get last data
float si7021_convert_temp_f(void);
float si7021_convert_rh(void);

// TDD test
void si7021_test(void);

#endif /* SRC_HEADER_FILES_SI7021_H_ */
