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
#define		SI7021_SCL_LOC			I2C_ROUTELOC0_SCLLOC_LOC15
#define		SI7021_SCL_EN			true
#define		SI7021_SDA_LOC			I2C_ROUTELOC0_SDALOC_LOC15
#define		SI7021_SDA_EN			true
#define 	SI7021_I2C				I2C0  // PG i2c peripheral
#define 	SI7021_REF_FREQ			0 // HF peripheral clock



//***********************************************************************************
// global variables
//***********************************************************************************

void si7021_i2c_open();

#endif /* SRC_HEADER_FILES_SI7021_H_ */
