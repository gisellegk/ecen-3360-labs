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

#include <string.h>

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
#define NO_EVENT 	0
#define SI7021_TEST_DELAY 80 // ms

//***********************************************************************************
// private variables
//***********************************************************************************
static uint8_t command_code[SI7021_MAX_CC_LENGTH];
static uint8_t write_arr[SI7021_MAX_WRITE_BYTES];
static uint8_t read_arr[SI7021_MAX_READ_BYTES];




//***********************************************************************************
// functions
//***********************************************************************************
/***************************************************************************//**
 * @brief
 * 	A private function that clears the arrays used as parameters for the i2c
 * 	peripheral.
 *
 *
 * @details
 * 	This function resets the write_arr, read_arr, and command_code arrays
 * 	to be filled with zeros.
 *
 *
 ******************************************************************************/
static void clear_i2c_arrays(void)
{
	memset(&write_arr[0], 0, SI7021_MAX_WRITE_BYTES);
	memset(&read_arr[0], 0, SI7021_MAX_READ_BYTES);
	memset(&command_code[0], 0, SI7021_MAX_CC_LENGTH);
}

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
void si7021_i2c_open(void)
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
 *	A function that requests a read from the SI7021 Temperature and Humidity sensor.
 *
 * @details
 *  This starts the I2C state machine to perform a read.
 * 	It is required to put the command code in the private command_code array
 * 	before calling this function.
 * 	Results will be stored in the private read_arr array which can be accessed
 * 	with either si7021_convert_temp_f() or si7021_convert_rh().
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
void si7021_read(uint8_t command_code_length, uint8_t read_length, uint32_t event){
	I2C_START_STRUCT start_struct;
	// put stuff in start_struct
	start_struct.device_address = SI7021_DEV_ADDR;
	start_struct.read = I2C_READ;
	start_struct.command_code = command_code;
	start_struct.command_code_length = command_code_length;
	start_struct.write_arr = 0; // no data in write command
	start_struct.write_length = 0;
	start_struct.read_arr = read_arr;
	start_struct.read_length = read_length;
	start_struct.event = event;
	i2c_start(SI7021_I2C, &start_struct);

}

/***************************************************************************//**
 * @brief
 *	A function that requests a write from the SI7021 Temperature and Humidity sensor.
 *
 * @details
 *  This starts the I2C state machine. It will write bytes to the device.
 * 	It is required to put the command code in the private command_code array,
 * 	and the desired write bytes in the private write_arr array before calling
 * 	this function.
 *
 * @param[in] command_code_length
 *   The number of bytes in the command code array
 *
 * @param[in] write_length
 *   The number of bytes of data to be written
 *
 * @param[in] event
 * 	 The scheduler event associated with a completed Write operation.
 *
 ******************************************************************************/
void si7021_write(uint8_t command_code_length, uint8_t write_length, uint32_t event){
	I2C_START_STRUCT start_struct;
	// put stuff in start_struct here
	start_struct.device_address = SI7021_DEV_ADDR;
	start_struct.read = I2C_WRITE;
	start_struct.command_code = command_code;
	start_struct.command_code_length = command_code_length;
	start_struct.write_arr = write_arr;
	start_struct.write_length = write_length;
	start_struct.read_arr = 0;
	start_struct.read_length = 0;
	start_struct.event = event;
	i2c_start(SI7021_I2C, &start_struct);
}


/***************************************************************************//**
 * @brief
 *	A function to initiate a Read RH Sequence
 *
 * @details
 *	This function initiates the read command to get a Relative Humidity reading
 *	from the SI021 sensor
 *
 * @note
 *  To get the data read from this function, use si7021_convert_rh(void).
 *	After the RH data has been converted and stored, you can use
 *	si7021_read_rh_temp() to read the associated temperature reading without
 *	starting a whole new conversion.
 *	You must access the data before calling another reading function.
 *
 * @param[in] event
 * 	 The scheduler event associated with a completed Read RH operation.
 *
 ******************************************************************************/
void si7021_read_rh(uint32_t event){
	clear_i2c_arrays();
	command_code[0] = SI7021_RH_NO_HOLD;
	si7021_read(I2C_ONE_BYTE_CC, SI7021_NUM_BYTES_RH_NOCHECKSUM, event);
}

/***************************************************************************//**
 * @brief
 *	A function to initiate a Read Temperature Sequence
 *
 * @details
 *	This function initiates the read command to get a Temperature reading
 *	from the SI021 sensor.
 *	To get the data read from this function, use si7021_convert_temp_f(void).
 *
 * @note
 *	This version of temperature read will start a conversion, and then report
 *	the results back. Use si7021_read_rh_temp() to read a temperature that was
 *	measured during a relative humidity conversion.
 *	You must access the data before calling another reading function.
 *
 * @param[in] event
 * 	 The scheduler event associated with a completed Read Temperature operation.
 *
 ******************************************************************************/
void si7021_read_temp(uint32_t event){
	clear_i2c_arrays();
	command_code[0] = SI7021_TEMP_NO_HOLD;
	si7021_read(I2C_ONE_BYTE_CC, SI7021_NUM_BYTES_TEMP_NOCHECKSUM, event);
}

/***************************************************************************//**
 * @brief
 *	A function to initiate a Read Temperature from Previous RH Sequence
 *
 * @details
 *	This function initiates the read command to get a Temperature reading
 *	from the SI021 sensor that was taken during a Relative Humidity conversion.
 *	To get the data read from this function, use si7021_convert_temp_f(void).
 *
 * @note
 *	This version of temperature read will NOT start a new conversion, and can only
 *	be called after si7021_read_rh().
 *	You must access the data before calling another reading function.
 *
 * @param[in] event
 * 	 The scheduler event associated with a completed Read Temperature operation.
 *
 ******************************************************************************/
void si7021_read_rh_temp(uint32_t event){
	clear_i2c_arrays();
	command_code[0] = SI7021_TEMP_FROM_RH;
	si7021_read(I2C_ONE_BYTE_CC, SI7021_NUM_BYTES_TEMP_FROM_RH, event);
}

/***************************************************************************//**
 * @brief
 *	A function to initiate a Read User Register 1
 *
 * @details
 *	This function initiates the read command to get the current value of the
 *	User Register from the Si7021 device
 *	To get the data read from this function, read the private read_arr array.
 *	You must access the data before calling another reading function.
 *
 * @param[in] event
 * 	 The scheduler event associated with a completed Read UR1 operation.
 *
 ******************************************************************************/
void si7021_read_ur1(uint32_t event){
	clear_i2c_arrays();
	command_code[0]= SI7021_READ_UR1;
	si7021_read(I2C_ONE_BYTE_CC, SI7021_NUM_BYTES_USER_REG, event);
}

/***************************************************************************//**
 * @brief
 *	A function to initiate a Write User Register 1
 *
 * @details
 *	This function initiates the write command to overwrite the current value of the
 *	User Register from the Si7021 device with the provided value.
 *
 * @param[in] byte
 *   The byte value that will be written to the register.
 *
 * @param[in] event
 * 	 The scheduler event associated with a completed Write UR1 operation.
 *
 ******************************************************************************/
void si7021_write_ur1(uint8_t byte, uint32_t event){
	clear_i2c_arrays();
	command_code[0] = SI7021_WRITE_UR1;
	write_arr[0] = byte;
	si7021_write(I2C_ONE_BYTE_CC, SI7021_NUM_BYTES_USER_REG, event);
}

/***************************************************************************//**
 * @brief
 *	A function to initiate a Read Serial Number sequence part B
 *
 * @details
 *	This function initiates the read command to get the Electronic Serial
 *	Number part B from the Si7021 device. The first byte return indicates the
 *	sensor part number of the device you are communicating with.
 *
 *	You must access the data before calling another reading function.
 *
 * @param[in] event
 * 	 The scheduler event associated with a completed Write UR1 operation.
 *
 ******************************************************************************/
void si7021_read_SNB(uint32_t event){
	clear_i2c_arrays();
	command_code[0] = SI7021_SNB_MSB;
	command_code[1] = SI7021_SNB_LSB;
	si7021_read(I2C_TWO_BYTE_CC, SI7021_NUM_BYTES_SNB, event);
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
 *	This should only be called upon the completion of either si7021_read_temp
 *	or si7021_read_rh_temp functions. Calling it at any other time will give
 *	an invalid result
 *
 * @return
 * 	the last received temperature, as a float, in degrees Fahrenheit.
 *
 ******************************************************************************/
float si7021_convert_temp_f(void){
	uint16_t temp_code = (read_arr[0] << 8) | read_arr[1];
	float temp_c = ((float)175.72*(float)temp_code / (float)65536) - (float)46.85;
	return (temp_c * (float)1.8 + (float)32);
}

/***************************************************************************//**
 * @brief
 *	A function which returns the most recently read relative humidity measurement.
 *
 * @details
 *	This function interprets the data received from the SI7021 as raw humidity
 *	data and converts it to Relative Humidity as a percent.
 *
 * @note
 *	This should only be called upon the completion of the si7021_read_rh function.
 *  Calling it at any other time will give an invalid result
 *
 * @return
 * 	the last received relative humidity value as a percentage.
 *
 ******************************************************************************/
float si7021_convert_rh(void){
	uint16_t rh_code = (read_arr[0] << 8) | read_arr[1];
	float rh = ((float)125.0*(float)rh_code / (float)65536) - (float)6.0;
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
	timer_delay(SI7021_TEST_DELAY); // wait for SI7021 to initialize
	// Test 1: Read from User Register 1.
	// This is a single byte read to test simplest read functionality
	clear_i2c_arrays();
	read_arr[0] = 0xff; // this is just for debugging purposes
	command_code[0]= SI7021_READ_UR1;
	si7021_read(I2C_ONE_BYTE_CC, SI7021_NUM_BYTES_USER_REG, NO_EVENT);
	//si7021_read_ur1(NO_EVENT); // this also works :)

	while(!i2c_idle());// stall until i2c is done
	uint8_t expected_value = 0b00111010;	// compare with expected value (default)
	EFM_ASSERT(read_arr[0] == expected_value); // will fail if not default value
	//EFM_ASSERT(read_arr[0] == expected_value || read_arr[0] == 0b10111010); // for testing

	// Test 2: Write to User Register 1 to change from 12b to 13b temp measurement
	// this is a single byte write to test simplest write functionality
	clear_i2c_arrays();
	command_code[0]= SI7021_WRITE_UR1;
	write_arr[0] = 0b10111010;
	si7021_write(I2C_ONE_BYTE_CC, SI7021_NUM_BYTES_USER_REG, NO_EVENT);
	//si7021_write_ur1(write_data, NO_EVENT);

	while(!i2c_idle());// stall until i2c is done
	timer_delay(SI7021_TEST_DELAY); // 80 ms delay to assure write completes before attempting to read
	// Read User Register 1 to confirm successful write.
	read_arr[0] = 0xff;
	si7021_read_ur1(NO_EVENT);
	while(!i2c_idle());// stall until i2c is done
	EFM_ASSERT(read_arr[0] == 0b10111010); // will fail if not modified value

	// Test 3: Read temp and validate within room temperature range
	// This will test a multi-byte read
	clear_i2c_arrays();
	read_arr[0] = 0xff;
	read_arr[1] = 0xff;
	command_code[0]= SI7021_TEMP_NO_HOLD;
	si7021_read(I2C_ONE_BYTE_CC, SI7021_NUM_BYTES_TEMP_NOCHECKSUM, NO_EVENT);
	//si7021_read_temp(NO_EVENT);
	while(!i2c_idle()); // stall until i2c is done
	float temp = si7021_convert_temp_f();	// get data and compare
	EFM_ASSERT(temp > 60 && temp < 90); // will fail if the temperature isn't in a reasonable range

	// Test 4: Read Electronic Serial Number
	// This will test a multi-byte command code (2 byte command code) as well as
	// a larger multi-byte read (ESN2 returns 6 bytes).
	// the first byte returned (SNB_3) identifies the part number
	// and should equal 0x15 for the SI7021.
	// honestly i don't think this really tests a multibyte read since we can only
	// really validate the first returned byte and not the subsequent ones.
	// but it's cool to see all 6 bytes end up in the read array from the debugger!
	clear_i2c_arrays();
	command_code[0] = SI7021_SNB_MSB;
	command_code[1] = SI7021_SNB_LSB;
	int i;
	for(i=0; i< SI7021_NUM_BYTES_SNB; i++){
		read_arr[i] = 0xaa;
	}
	si7021_read(I2C_TWO_BYTE_CC, SI7021_NUM_BYTES_SNB, NO_EVENT);
	//si7021_read_SNB(NO_EVENT);

	while(!i2c_idle());
		//compare to expected value
	EFM_ASSERT(read_arr[0] == 0x15);

}
