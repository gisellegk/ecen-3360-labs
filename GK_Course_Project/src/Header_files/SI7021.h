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
#define		SI7021_SCL_LOC			_I2C_ROUTELOC0_SCLLOC_LOC15
#define		SI7021_SCL_LOC_i2c0			_I2C_ROUTELOC0_SCLLOC_LOC15
#define 	SI7021_SCL_LOC_i2c1			_I2C_ROUTELOC0_SCLLOC_LOC19

#define		SI7021_SCL_EN			true

#define		SI7021_SDA_LOC			_I2C_ROUTELOC0_SDALOC_LOC15
#define		SI7021_SDA_LOC_i2c0			_I2C_ROUTELOC0_SDALOC_LOC15
#define		SI7021_SDA_LOC_i2c1			_I2C_ROUTELOC0_SDALOC_LOC19

#define		SI7021_SDA_EN			true
#define 	SI7021_I2C				I2C0 // default i2c for app use
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
#define		SI7021_MAX_READ_BYTES				8
#define 	SI7021_NUM_BYTES_USER_REG			1
#define		SI7021_NUM_BYTES_SNA				8
#define		SI7021_NUM_BYTES_SNB				6

#define 	SI7021_ARR_MAX_LEN					10

#define		SI7021_USRREG1_DEFAULT_VAL			0x3A
#define		SI7021_USRREG1_13_BIT_RES_SET		0xBA

#define NO_EVENT 					(uint32_t)0
#define SI7021_POWERUP_DELAY_MS 	80
#define SI7021_USRREG1_DELAY_MS 	80

//***********************************************************************************
// global variables
//***********************************************************************************

void si7021_i2c_open();
void si7021_read(uint8_t command_code_length, uint8_t read_length, uint32_t event);
void si7021_write(uint8_t command_code_length, uint8_t write_length, uint32_t event);

// r/w presets
void si7021_read_rh(uint32_t event);
void si7021_read_temp(uint32_t event);
void si7021_read_rh_temp(uint32_t event);

// get last data
float si7021_last_temp_f(void);
float si7021_last_humidity(void);

// TDD test
void si7021_test(void);

#endif /* SRC_HEADER_FILES_SI7021_H_ */
