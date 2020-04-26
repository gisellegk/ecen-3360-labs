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
#include "HW_delay.h"

//***********************************************************************************
// defined files
//***********************************************************************************

//***********************************************************************************
// global variables
//***********************************************************************************


//***********************************************************************************
// private variables
//***********************************************************************************

static uint8_t	write_data[SI7021_ARR_MAX_LEN];
static uint8_t	read_data[SI7021_ARR_MAX_LEN];
static uint8_t	command_data[SI7021_ARR_MAX_LEN];

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

static void clear_i2c_arrays(void)
{
	memset(&write_data[0], 0, SI7021_ARR_MAX_LEN);
	memset(&read_data[0], 0, SI7021_ARR_MAX_LEN);
	memset(&command_data[0], 0, SI7021_ARR_MAX_LEN);
}

/***************************************************************************//**
 * @brief
 *	A function that requests a read from the SI7021
 *	Temperature and Humidity sensor.
 *
 * @details
 *  This starts the I2C state machine. It will save the data in the static local
 *  variable in Si7021.c which can be accessed with the accessor function
 *
 *
 * @param[in] command_code
 * 	 The command code of the read operation. This must be a pointer, and should
 * 	 point to an array if there is more than 1 byte in the command code.
 *
 * @param[in] command_code_length
 *   The number of bytes in the command code array
 *
 * @param[in] read_length
 *   The number of bytes expected to be read
 *
 * @param[in] event
 * 	 The scheduler event associated with a completed Read operation.
 *
 ******************************************************************************/
void si7021_read(uint8_t command_code_length, uint8_t read_length, uint32_t event)
{
	I2C_START_STRUCT start_struct;
	memset(&start_struct, 0, sizeof(I2C_START_STRUCT));

	// put stuff in start_struct here
	start_struct.command_code = &command_data[0];
	start_struct.command_code_length = command_code_length;
	start_struct.device_address = SI7021_DEV_ADDR;
	start_struct.event = event;
	start_struct.i2c = SI7021_I2C;
	start_struct.read = I2C_READ;
	start_struct.write_arr = (void *)0; // no data in write command
	start_struct.write_length = 0;
	start_struct.read_arr = &read_data[0];
	start_struct.read_length = read_length;

	i2c_start(&start_struct);
}

/***************************************************************************//**
 * @brief
 *	A function that requests a write from the SI7021 Temperature and Humidity sensor.
 *
 * @details
 *  This starts the I2C state machine. It will write bytes to the device
 *
 * @param[in] command_code
 * 	 The command code of the write operation. This must be a pointer, and should
 * 	 point to an array if there is more than 1 byte in the command code.
 *
 * @param[in] command_code_length
 *   The number of bytes in the command code array
 *
 * @param[in] write_arr
 *   The pointer to where the data to write to the sensor is stored

 * @param[in] read_length
 *   The number of bytes of data to be written
 *
 * @param[in] event
 * 	 The scheduler event associated with a completed Write operation.
 *
 ******************************************************************************/
void si7021_write(uint8_t command_code_length, uint8_t write_length, uint32_t event)
{
	I2C_START_STRUCT start_struct;
	memset(&start_struct, 0, sizeof(I2C_START_STRUCT));

	// put stuff in start_struct here
	start_struct.command_code = &command_data[0];
	start_struct.command_code_length = command_code_length;
	start_struct.device_address = SI7021_DEV_ADDR;
	start_struct.event = event;
	start_struct.i2c = SI7021_I2C;
	start_struct.read = I2C_WRITE;
	start_struct.write_arr = &write_data[0]; // no data in write command
	start_struct.write_length = write_length;
	start_struct.read_arr = (void *)0;
	start_struct.read_length = 0;

	i2c_start(&start_struct);
}

/***************************************************************************//**
 * @brief
 *	A function to initiate a Read RH Sequence
 *
 * @details
 *	This function initiates the read command to get a Relative Humidity reading
 *	from the SI021 sensor
 *
 *
 * @note
 *	After this function is called, you can use si7021_read_rh_temp() to read
 *	the associated temperature reading without starting a whole new conversion.
 *
 *
 ******************************************************************************/
void si7021_read_rh(uint32_t event){
	clear_i2c_arrays();
	command_data[0] = SI7021_RH_NO_HOLD;
	si7021_read(I2C_ONE_BYTE_CC, SI7021_NUM_BYTES_RH_NOCHECKSUM, event);
}

/***************************************************************************//**
 * @brief
 *	A function to initiate a Read Temperature Sequence
 *
 * @details
 *	This function initiates the read command to get a Temperature reading
 *	from the SI021 sensor
 *
 *
 * @note
 *	This version of temperature read will start a conversion, and then report
 *	the results back. Use si7021_read_rh_temp() to read a temperature that was
 *	measured during a relative humidity conversion.
 *
 ******************************************************************************/
void si7021_read_temp(uint32_t event){
	clear_i2c_arrays();
	command_data[0] = SI7021_TEMP_NO_HOLD;
	si7021_read(I2C_ONE_BYTE_CC, SI7021_NUM_BYTES_TEMP_NOCHECKSUM, event);
}

/***************************************************************************//**
 * @brief
 *	A function to initiate a Read Temperature from Previous RH Sequence
 *
 * @details
 *	This function initiates the read command to get a Temperature reading
 *	from the SI021 sensor that was taken during a Relative Humidity conversion
 *
 * @note
 *	This version of temperature read will NOT start a new conversion, and can only
 *	be called after si7021_read_rh().
 *
 ******************************************************************************/
void si7021_read_rh_temp(uint32_t event){
	clear_i2c_arrays();
	command_data[0] = SI7021_TEMP_FROM_RH;
	si7021_read(I2C_ONE_BYTE_CC, SI7021_NUM_BYTES_TEMP_FROM_RH, event);
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
 *	This should only be called upon the completion of either the
 *	SI7021_READ_RH_TEMP_DONE_EVT or SI_7021_READ_TEMP_DONE_EVT.
 *
 *
 * @return
 * 	the last received temperature, as a float, in degrees Fahrenheit.
 *
 ******************************************************************************/
float si7021_last_temp_f(){
	uint16_t temp_code = (read_data[0] << 8) | read_data[1];
	float temp_c = (((float)175.72*(float)temp_code) / (float)65536) - (float)46.85;
	return ((temp_c * (float)1.8) + (float)32);
}

/***************************************************************************//**
 * @brief
 *	A function which returns the most recently read relative humidity measurement.
 *
 * @details
 *	This function reads the raw humidity data received from the SI7021
 *	Temperature and Humidity sensor,
 *
 * @note
 *	This should only be called upon the completion of the SI7021_READ_RH_DONE_EVT
 *
 * @return
 * 	the last received relative humidity value as a percentage.
 *
 ******************************************************************************/
float si7021_last_rh(){
	uint16_t rh_code = (read_data[0] << 8) | read_data[1];
	float rh = (((float)125.0*(float)rh_code) / (float)65536) - (float)6.0;
	return rh;
}

/***************************************************************************//**
 * @brief
 *   SI7021 Read/Write test. This is a Test Driven Development routine to verify
 *   that the I2C peripheral is correctly configured to communicate with the
 *   SI7021 device. It also configures the SI7021 sensor to report 10 bit
 *   Relative Humidity and 13 bit Temperature data.
 *
 * @details
 *   This tests our ability to read and write bytes to and from the SI7021.
 *   The SI7021 requires at least 80 ms of delay to make sure that the device
 *   has successfully initialized. After this, it reads the the SI7021 User 1
 *   Register to make sure that we can perform a simple 1 byte read. It checks
 *   to make sure that the SI7021 is in its default state (0011 1010) before
 *   proceeding
 *
 *   Then it writes a new byte such that the temp resolution is now 13b instead
 *   of 14b (1011 1010). This allows us to check that we can write 1 byte.
 *
 *   The SI7021 requires at least 80 ms of delay to make sure that the
 *   write has completed. We delay for that amount of time, and then read
 *   the register to make sure that the write competed.
 *
 *   Finally, it performs one temperature reading and validates that the
 *   data is in a reasonable (near room temperature) range.
 *
 *
 ******************************************************************************/

void si7021_test(void){

	timer_delay(SI7021_POWERUP_DELAY_MS); // wait for SI7021 to initialize

	// Test 1: Read from User Register 1.
	// This is a single byte read to test simplest read functionality
	// prepare variables
	clear_i2c_arrays();
	command_data[0] = SI7021_READ_UR1;
	// run command
	si7021_read(I2C_ONE_BYTE_CC, SI7021_NUM_BYTES_USER_REG, NO_EVENT);
	while(!i2c_idle(SI7021_I2C));// stall until i2c is done
	// compare with expected value (default)
	EFM_ASSERT(read_data[0] == SI7021_USRREG1_DEFAULT_VAL); // will fail if not default value

	// Test 2: Write to User Register 1 to change from 12b to 13b temp measurement
	// this is a single byte write to test simplest write functionality
		// prepare variables
	clear_i2c_arrays();
	command_data[0] = SI7021_WRITE_UR1;
	write_data[0] = SI7021_USRREG1_13_BIT_RES_SET;
	// run command
	si7021_write(I2C_ONE_BYTE_CC, SI7021_NUM_BYTES_USER_REG, NO_EVENT);
	while(!i2c_idle(SI7021_I2C));// stall until i2c is done
	// 80 ms delay to assure write completes before attempting to read
	timer_delay(SI7021_USRREG1_DELAY_MS);

	// Read User Register 1 to confirm successful write.
	// prepare variables for read
	clear_i2c_arrays();
	command_data[0] = SI7021_READ_UR1;
	// run command
	si7021_read(I2C_ONE_BYTE_CC, SI7021_NUM_BYTES_USER_REG, NO_EVENT);
	while(!i2c_idle(SI7021_I2C));// stall until i2c is done
	// compare
	EFM_ASSERT(read_data[0] == SI7021_USRREG1_13_BIT_RES_SET); // will fail if not modified value

	// Test 3: Read temp and validate within room temperature range
	// This will test a multi-byte read
	// prepare variables
	clear_i2c_arrays();
	command_data[0] = SI7021_TEMP_NO_HOLD;
	// run command
	si7021_read(I2C_ONE_BYTE_CC, SI7021_NUM_BYTES_TEMP_NOCHECKSUM, NO_EVENT);
	while(!i2c_idle(SI7021_I2C));// stall until i2c is done
	// get data and compare
	float temp = si7021_last_temp_f();
	EFM_ASSERT(temp > 65 && temp < 90); // will fail if the temperature isn't in a reasonable range

	// Test 4: Read Electronic Serial Number
	// This will test a multi-byte command code (2 byte command code) as well as
	// a larger multi-byte read (ESN2 returns 6 bytes).
	// the first byte returned (SNB_3) identifies the part number
	// and should equal 0x15 for the SI7021.
	// honestly i don't think this really tests a multibyte read since we can only
	// really validate the first returned byte and not the subsequent ones.
		// prepare variables
//	clear_i2c_arrays();
//	command_data[0] = SI7021_SNB_MSB;
//	command_data[1] = SI7021_SNB_LSB;
//	// run command
//	si7021_read(I2C_TWO_BYTE_CC, SI7021_NUM_BYTES_SNB, NO_EVENT);
//	while(!i2c_idle(SI7021_I2C));
//		//compare to expected value
//	EFM_ASSERT(read_data[0] == 0x15);
}
