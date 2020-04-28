/**
 * @file gpio.c
 * @author Giselle Koo
 * @date April 26, 2020
 * @brief Contains all the functions to set up the GPIO.
 *
 */
//***********************************************************************************
// Include files
//***********************************************************************************
#include "gpio.h"
#include "em_cmu.h"

//***********************************************************************************
// defined files
//***********************************************************************************


//***********************************************************************************
// global variables
//***********************************************************************************


//***********************************************************************************
// function prototypes
//***********************************************************************************


//***********************************************************************************
// functions
//***********************************************************************************
void gpio_open(void){

	CMU_ClockEnable(cmuClock_GPIO, true);

	// Set LED ports to be standard output drive with default off (cleared)
	GPIO_DriveStrengthSet(LED0_port, gpioDriveStrengthStrongAlternateStrong);
//	GPIO_DriveStrengthSet(LED0_port, gpioDriveStrengthWeakAlternateWeak);
	GPIO_PinModeSet(LED0_port, LED0_pin, gpioModePushPull, LED0_default);

	GPIO_DriveStrengthSet(LED1_port, gpioDriveStrengthStrongAlternateStrong);
//	GPIO_DriveStrengthSet(LED1_port, gpioDriveStrengthWeakAlternateWeak);
	GPIO_PinModeSet(LED1_port, LED1_pin, gpioModePushPull, LED1_default);

	// SI7021 Pin Configurations
	GPIO_DriveStrengthSet(SI7021_SENSOR_EN_PORT, gpioDriveStrengthWeakAlternateWeak);
	GPIO_PinModeSet(SI7021_SENSOR_EN_PORT, SI7021_SENSOR_EN_PIN, gpioModePushPull, SI7021_ENABLE);
	GPIO_PinModeSet(SI7021_SCL_PORT, SI7021_SCL_PIN, gpioModeWiredAnd, SI7021_I2C_DEFAULT);
	GPIO_PinModeSet(SI7021_SDA_PORT, SI7021_SDA_PIN, gpioModeWiredAnd, SI7021_I2C_DEFAULT);


	// BLE Pin Configuration
	GPIO_DriveStrengthSet(BLE_UART_TX_PORT, gpioDriveStrengthStrongAlternateWeak);
	GPIO_PinModeSet(BLE_UART_TX_PORT, BLE_UART_TX_PIN, gpioModePushPull, BLE_DEFAULT);
	GPIO_PinModeSet(BLE_UART_RX_PORT, BLE_UART_RX_PIN, gpioModeInput, BLE_DEFAULT);

}
