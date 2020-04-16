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
 * 	A function to open an I2C port for the SI7021 Temperature and Humidity Sensor
 * 	onboard the Pearl Gecko Starter Kit.
 *
 *
 * @details
 * 	This function creates an I2C IO Struct specifying the configuration of the SDA
 * 	and SCL pins that connect the SI7021 to the EFM32PG12 on the Pearl Gecko
 * 	Starter Kit board, and an I2C Open Struct specifying the I2C configuration
 * 	necessary to interact with the SI7021 sensor. Then it opens the I2C bus using
 * 	this information.
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
	i2c_open_struct.scl_route0 = SI7021_SCL_LOC;
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
 ******************************************************************************/
void si7021_i2c_test_open(bool i2c)
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

	I2C_TypeDef* current_i2c;
	if(i2c == 1) {
		current_i2c = I2C1;
		i2c_open_struct.scl_route0 = SI7021_SCL_LOC_i2c1;
		i2c_open_struct.sda_route0 = SI7021_SDA_LOC_i2c1;
	} else {
		current_i2c = I2C0;
		i2c_open_struct.scl_route0 = SI7021_SCL_LOC_i2c0;
		i2c_open_struct.sda_route0 = SI7021_SDA_LOC_i2c0;
	}

	i2c_open_struct.scl_en = SI7021_SCL_EN;
	i2c_open_struct.sda_en = SI7021_SDA_EN;


	i2c_open(current_i2c, &i2c_open_struct, &i2c_io_struct);


}

/***************************************************************************//**
 * @brief
 *	A function that requests the temperature to be read from the SI7021
 *	Temperature and Humidity sensor.
 *
 * @details
 *	This function initiates a Measure Temperature command in No Hold Master Mode.
 *
 * @note
 * This starts the I2C state machine.
 *
 * @param[in] event
 * 	 The scheduler event associated with a completed Measure Temperature operation.
 *
 ******************************************************************************/
void si7021_read(uint32_t event){
	i2c_start(SI7021_I2C, SI7021_DEV_ADDR, I2C_READ, SI7021_TEMP_NO_HOLD, data, SI7021_NUM_BYTES_TEMP_CHECKSUM, event);
}

/***************************************************************************//**
 * @brief
 *	A function which returns the most recently read temperature measurement.
 *
 * @details
 *	This function reads the raw temperature data received from the SI7021
 *	Temperature and Humidity sensor, converts it into Celsius per the
 *	data sheet specifications, and then converts it into Fahrenheit to return.
 *
 * @note
 *	This should only be called upon the completion of the SI7021_TEMP_READ_COMPLETE_EVT
 *
 *
 * @return
 * 	the last received temperature, as a float, in degrees Fahrenheit.
 *
 ******************************************************************************/
float si7021_last_temp_f(){
	uint16_t temp_code = (data[0] << 8) | data[1];
	float temp_c = ((float)175.72*(float)temp_code / (float)65536) - (float)46.85;
	return (temp_c * (float)1.8 + (float)32);
}
