//***********************************************************************************
// Include files
//***********************************************************************************
#include "em_gpio.h"

//***********************************************************************************
// defined files
//***********************************************************************************

// LED0 pin is
#define	LED0_port				gpioPortF
#define LED0_pin				04u
#define LED0_default			false 	// off
// LED1 pin is
#define LED1_port				gpioPortF
#define LED1_pin				05u
#define LED1_default			false	// off

// Si7021 Pins
#define SI7021_SCL_PORT			gpioPortC
#define SI7021_SCL_PIN			11u
#define SI7021_SDA_PORT			gpioPortC
#define SI7021_SDA_PIN			10u
#define SI7021_SENSOR_EN_PORT	gpioPortB
#define SI7021_SENSOR_EN_PIN	10u
#define SI7021_ENABLE 			1
#define SI7021_I2C_DEFAULT		1

//***********************************************************************************
// global variables
//***********************************************************************************


//***********************************************************************************
// function prototypes
//***********************************************************************************
void gpio_open(void);

