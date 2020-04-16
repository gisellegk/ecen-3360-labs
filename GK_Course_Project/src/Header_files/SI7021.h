#ifndef SRC_HEADER_FILES_SI7021_H_
#define SRC_HEADER_FILES_SI7021_H_


//***********************************************************************************
// Include files
//***********************************************************************************


//***********************************************************************************
// defined files
//***********************************************************************************
#define 	SI7021_DEV_ADDR			0x40
#define		SI7021_TEMP_NO_HOLD		0xF3
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

#define 	SI7021_NUM_BYTES_TEMP_CHECKSUM		6
#define		SI7021_NUM_BYTES_TEMP_NOCHECKSUM	2

//***********************************************************************************
// global variables
//***********************************************************************************

void si7021_i2c_open();
void si7021_read(uint32_t event);
float si7021_last_temp_f();

#endif /* SRC_HEADER_FILES_SI7021_H_ */
