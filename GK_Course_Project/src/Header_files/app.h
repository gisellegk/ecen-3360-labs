//***********************************************************************************
// Include files
//***********************************************************************************
#ifndef APP_H
#define	APP_H
#include "em_cmu.h"
#include "em_prs.h"
#include "cmu.h"
#include "gpio.h"

//***********************************************************************************
// defined files
//***********************************************************************************
#define		PWM_PER				3.1		// PWM period in seconds
#define		PWM_ACT_PER			0.10	// PWM active period in seconds
#define		LETIMER0_ROUTE_OUT0	LETIMER_ROUTELOC0_OUT0LOC_LOC28
#define		LETIMER0_OUT0_EN	false
#define		LETIMER0_ROUTE_OUT1	0
#define		LETIMER0_OUT1_EN	false

#define 	LETIMER0_COMP0_EVT					0x00000001
#define 	LETIMER0_COMP1_EVT					0x00000002
#define 	LETIMER0_UF_EVT						0x00000004
#define		BOOT_UP_EVT							0x00000008
#define		BLE_TX_DONE_EVT						0x00000010
#define		BLE_RX_DONE_EVT						0x00000020
#define		SI7021_READ_RH_DONE_EVT				0x00000040
#define 	SI7021_READ_RH_TEMP_DONE_EVT		0x00000080
#define		SI7021_READ_TEMP_DONE_EVT			0x00000100

// TDD Test Enables
// #define BLE_TEST_ENABLED
// #define CIRC_BUFF_TEST_ENABLED
#define SI7021_TEST_ENABLED
//***********************************************************************************
// global variables
//***********************************************************************************


//***********************************************************************************
// function prototypes
//***********************************************************************************
void app_peripheral_setup(void);
void app_letimer_pwm_open(float period, float act_period);
void scheduled_letimer0_uf_evt(void);
void scheduled_letimer0_comp0_evt(void);
void scheduled_letimer0_comp1_evt(void);
void scheduled_boot_up_evt(void);
void scheduled_tx_done_evt(void);
void scheduled_rx_done_evt(void);
void scheduled_si7021_read_temp_done_evt(void);
void scheduled_si7021_read_rh_done_evt(void);
void scheduled_si7021_read_rh_temp_done_evt(void);

#endif
