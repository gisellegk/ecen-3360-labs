//***********************************************************************************
// Include files
//***********************************************************************************
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
#define		LETIMER0_OUT0_EN	true
#define		LETIMER0_ROUTE_OUT1	0
#define		LETIMER0_OUT1_EN	false

#define 	LETIMER0_COMP0_EVT	0x00000001
#define 	LETIMER0_COMP1_EVT	0x00000002
#define 	LETIMER0_UF_EVT		0x00000004

//***********************************************************************************
// global variables
//***********************************************************************************


//***********************************************************************************
// function prototypes
//***********************************************************************************
void app_peripheral_setup(void);
void app_letimer_pwm_open(float period, float act_period);

