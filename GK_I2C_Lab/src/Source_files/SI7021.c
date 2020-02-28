/**
 * @file SI7021.c
 * @author Giselle Koo
 * @date Feb 25, 2020
 * @brief Contains all the functions to interface with the SI7021 Temp/Humidity sensor
 *
 */
//***********************************************************************************
// Include files
//***********************************************************************************

//** Silicon Lab include files
#include "em_i2c.h"
#include "em_cmu.h"
#include "em_assert.h"

//** User/developer include files
#include "SI7021.h"
#include "gpio.h"
#include "i2c.h"

//***********************************************************************************
// defined files
//***********************************************************************************

//***********************************************************************************
// global variables
//***********************************************************************************


//***********************************************************************************
// private variables
//***********************************************************************************
static uint8_t data[SI7021_NUM_BYTES_TEMP_CHECKSUM];

//***********************************************************************************
// functions
//***********************************************************************************

/***************************************************************************//**
 * @brief
 *
 *
 * @details
 *
 *
 * @note
 *
 *
 * @param[in] i2c
 *
 *
 ******************************************************************************/
void si7021_i2c_open()
{
	I2C_IO_STRUCT i2c_io_struct;

	i2c_io_struct.scl_pin = SI7021_SCL_PIN;
	i2c_io_struct.scl_port = SI7021_SCL_PORT;
	i2c_io_struct.sda_pin = SI7021_SDA_PIN;
	i2c_io_struct.sda_port = SI7021_SDA_PORT;

	I2C_OPEN_STRUCT i2c_open_struct;

	i2c_open_struct.chlr = SI7021_I2C_CLK_RATIO;
	i2c_open_struct.enable = SI7021_ENABLE;
	i2c_open_struct.freq = SI7021_I2C_FREQ;
	i2c_open_struct.master = true;
	i2c_open_struct.refFreq = SI7021_REF_FREQ;

	i2c_open_struct.scl_en = SI7021_SCL_EN;
	i2c_open_struct.scl_route0 = SI7021_SDA_LOC;
	i2c_open_struct.sda_en = SI7021_SDA_EN;
	i2c_open_struct.sda_route0 = SI7021_SDA_LOC;

	i2c_open(SI7021_I2C, &i2c_open_struct, &i2c_io_struct);


}

/***************************************************************************//**
 * @brief
 *
 *
 * @details
 *
 *
 * @note
 *
 *
 * @param[in] i2c
 *
 *
 ******************************************************************************/
void si7021_read(uint32_t event){
	i2c_start(SI7021_I2C, SI7021_DEV_ADDR, I2C_READ, SI7021_TEMP_NO_HOLD, data, SI7021_NUM_BYTES_TEMP_CHECKSUM, event);
}

/***************************************************************************//**
 * @brief
 *
 *
 * @details
 *
 *
 * @note
 *
 *
 * @param[in] i2c
 *
 *
 ******************************************************************************/
float si7021_last_temp_f(){
	uint16_t temp_code = (data[0] << 8) | data[1];
	float temp_c = (175.72*temp_code / 65536) - 46.85;
	return (temp_c * 1.8 + 32);
}
