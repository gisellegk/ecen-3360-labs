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
