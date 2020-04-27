/**
 * @file app.c
 * @author Giselle Koo
 * @date January 28th, 2020
 * @brief Contains app driver functions.
 *
 */


//***********************************************************************************
// Include files
//***********************************************************************************
#include "app.h"
#include "letimer.h"
#include "scheduler.h"
#include "SI7021.h"
#include "ble.h"
#include <stdio.h>

//***********************************************************************************
// defined files
//***********************************************************************************

//***********************************************************************************
// global variables
//***********************************************************************************
char buffer[50];

//***********************************************************************************
// function
//***********************************************************************************

/***************************************************************************//**
 * @brief
 *	Used to initialize all peripherals for this application.
 *
 * @details
 *	This application uses the CMU to enable the LE clock tree,
 *	and initializes the GPIO and LETIMER needed to produce a
 *	PWM signal.
 *
 * @note
 *	This should be called only once at the beginning of main.
 *
 ******************************************************************************/
void app_peripheral_setup(void){
	cmu_open();
	gpio_open();
	app_letimer_pwm_open(PWM_PER, PWM_ACT_PER);
	scheduler_open();
	sleep_open();
	si7021_i2c_open();
	ble_open(BLE_TX_DONE_EVT, BLE_RX_DONE_EVT);
	add_scheduled_event(BOOT_UP_EVT);
}


/***************************************************************************//**
 * @brief
 *  Function to open a PWM signal out of LETIMER0 at a particular frequency
 *  duty cycle. This includes values/settings for our specific application.
 *
 * @details
 *	This routine defines the values of the APP_LETIMER_PWM_TypeDef struct
 *	specific to our application then passes it to the lower level driver
 *	letimer_pwm_open function along with which LETIMER we want to use
 *	(there is only one LETIMER on the Pearl Gecko, so it will always be 0).
 *
 * @note
 *  This function is used to setup PWM for this app.
 *
 * @param[in] period
 * 	PWM period in seconds
 *
 *
 * @param[in] act_period
 *	PWM active period in seconds
 *
 ******************************************************************************/
void app_letimer_pwm_open(float period, float act_period){
	// Initializing LETIMER0 for PWM operation by creating the
	// letimer_pwm_struct and initializing all of its elements
	APP_LETIMER_PWM_TypeDef letimer_pwm_struct;
	letimer_pwm_struct.active_period = act_period;
	letimer_pwm_struct.period = period;
	letimer_pwm_struct.debugRun = false;
	letimer_pwm_struct.enable = false;
	letimer_pwm_struct.out_pin_0_en = LETIMER0_OUT0_EN;
	letimer_pwm_struct.out_pin_1_en = LETIMER0_OUT1_EN;
	letimer_pwm_struct.out_pin_route0 = LETIMER0_ROUTE_OUT0;
	letimer_pwm_struct.out_pin_route1 = LETIMER0_ROUTE_OUT1;
	letimer_pwm_struct.comp0_irq_enable = false;
	letimer_pwm_struct.comp0_evt = LETIMER0_COMP0_EVT;
	letimer_pwm_struct.comp1_irq_enable = false;
	letimer_pwm_struct.comp1_evt = LETIMER0_COMP1_EVT;
	letimer_pwm_struct.uf_irq_enable = true;
	letimer_pwm_struct.uf_evt = LETIMER0_UF_EVT;

	letimer_pwm_open(LETIMER0, &letimer_pwm_struct);
}


/***************************************************************************//**
 * @brief
 *	Handles the letimer0 underflow event
 *
 * @details
 *	This function clears the scheduled event and then handles the underflow event.
 *
 *
 ******************************************************************************/
void scheduled_letimer0_uf_evt(void){
	EFM_ASSERT(get_scheduled_events() & LETIMER0_UF_EVT);
	remove_scheduled_event(LETIMER0_UF_EVT);
	si7021_read_rh(SI7021_READ_RH_DONE_EVT);
}

/***************************************************************************//**
 * @brief
 *	Handles the letimer0 comp0 event
 *
 * @details
 *	This function clears the scheduled event and then handles the comp0 event.
 *
 *
 ******************************************************************************/
void scheduled_letimer0_comp0_evt(void){
	EFM_ASSERT(get_scheduled_events() & LETIMER0_COMP0_EVT);
	remove_scheduled_event(LETIMER0_COMP0_EVT);
	EFM_ASSERT(false);
}

/***************************************************************************//**
 * @brief
 *	Handles the letimer0 comp1 event
 *
 * @details
 *	This function clears the scheduled event and then handles the comp1 event.
 *
 *
 ******************************************************************************/
void scheduled_letimer0_comp1_evt(void){
	EFM_ASSERT(get_scheduled_events() & LETIMER0_COMP1_EVT);
	remove_scheduled_event(LETIMER0_COMP1_EVT);
	EFM_ASSERT(false);

}

/***************************************************************************//**
 * @brief
 *	Handles the SI7021 Relative Humidity Read Complete event
 *
 * @details
 *	This function clears the scheduled event and then handles the
 *	Relative Humidity Read Complete event.
 *
 *
 ******************************************************************************/
void scheduled_si7021_read_rh_done_evt(void){
	EFM_ASSERT(get_scheduled_events() & SI7021_READ_RH_DONE_EVT);
	remove_scheduled_event(SI7021_READ_RH_DONE_EVT);

	float rh = si7021_convert_rh();
	sprintf(buffer, "Humidity = %d.%d %% \n", (int)rh, (int)(rh*10)%10);
	ble_write(buffer);
	si7021_read_rh_temp(SI7021_READ_RH_TEMP_DONE_EVT);
}

/***************************************************************************//**
 * @brief
 *	Handles the SI7021 Read Temperature from RH Complete event
 *
 * @details
 *	This function clears the scheduled event and then handles the
 *	Read Temperature from RH Complete event.
 *
 *
 ******************************************************************************/
void scheduled_si7021_read_rh_temp_done_evt(void){
	EFM_ASSERT(get_scheduled_events() & SI7021_READ_RH_TEMP_DONE_EVT);
	remove_scheduled_event(SI7021_READ_RH_TEMP_DONE_EVT);

	float temp = si7021_convert_temp_f();
	if(temp >= 80.0) {
		// turn on GPIO pin LED 1
		GPIO_PinOutSet(LED1_port, LED1_pin);
	} else {
		// turn off LED 1
		GPIO_PinOutClear(LED1_port, LED1_pin);
	}
	sprintf(buffer, "Temp = %d.%d F\n", (int)temp, (int)(temp*10)%10);
	ble_write(buffer);
}

/***************************************************************************//**
 * @brief
 *	Handles the SI7021 Temperature Read Complete event
 *
 * @details
 *	This function clears the scheduled event and then handles the
 *	Temperature Read Complete event.
 *
 *
 ******************************************************************************/
void scheduled_si7021_read_temp_done_evt(void){
	EFM_ASSERT(get_scheduled_events() & SI7021_READ_TEMP_DONE_EVT);
	remove_scheduled_event(SI7021_READ_TEMP_DONE_EVT);
	float temp = si7021_convert_temp_f();
	if(temp >= 80.0) {
		// turn on GPIO pin LED 1
		GPIO_PinOutSet(LED1_port, LED1_pin);
	} else {
		// turn off LED 1
		GPIO_PinOutClear(LED1_port, LED1_pin);
	}
	sprintf(buffer, "Temp = %d.%d F\n", (int)temp, (int)(temp*10)%10);
	ble_write(buffer);
}

/***************************************************************************//**
 * @brief
 *	Handles the BOOT UP event
 *
 * @details
 *	This function clears the scheduled event and then handles the
 *	Boot Up event. This includes all tests and the initial BLE writes.
 *
 *
 ******************************************************************************/
void scheduled_boot_up_evt(void){
	EFM_ASSERT(get_scheduled_events() & BOOT_UP_EVT);
	remove_scheduled_event(BOOT_UP_EVT);

	// enable tests in app.h
#ifdef BLE_TEST_ENABLED
	bool test = ble_test("GiselleKoo");
	EFM_ASSERT(test);
	for(int i = 0; i < 20000000; i++); // only works with no optimization
#endif
#ifdef CIRC_BUFF_TEST_ENABLED
	circular_buff_test();
#endif
#ifdef SI7021_TEST_ENABLED
	si7021_test();
#endif
	ble_write("\nHello World\n");
	ble_write("Circular Buffer Lab\n");
	ble_write("Giselle Koo\n");


}

/***************************************************************************//**
 * @brief
 *	Handles the TX DONE event
 *
 * @details
 *	This function clears the scheduled event and then handles the
 *	TX Done event. It checks to see if there's more things to send in the
 *	circular buffer, and initiates that transmit if there is.
 *
 *	Then it restarts the timer.
 *
 *
 ******************************************************************************/
void scheduled_tx_done_evt(void){
	EFM_ASSERT(get_scheduled_events() & BLE_TX_DONE_EVT);
	remove_scheduled_event(BLE_TX_DONE_EVT);
	ble_circ_pop(false); // if there's other stuff to send, pop it off. otherwise this will return true.

	letimer_start(LETIMER0, true);

}

/***************************************************************************//**
 * @brief
 *	Handles the RX DONE event
 *
 * @details
 *	This function clears the scheduled event and then handles the
 *	RX Done event.
 *
 *
 ******************************************************************************/
void scheduled_rx_done_evt(void){
	EFM_ASSERT(get_scheduled_events() & BLE_RX_DONE_EVT);
	remove_scheduled_event(BLE_RX_DONE_EVT);

	//do stuff
	// TODO: complete this function if needed

}
