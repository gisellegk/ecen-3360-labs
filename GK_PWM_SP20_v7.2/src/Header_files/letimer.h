//***********************************************************************************
// Include files
//***********************************************************************************
#include "em_letimer.h"

//***********************************************************************************
// defined files
//***********************************************************************************
#define LETIMER_HZ		1000			// Utilizing ULFRCO oscillator for LETIMERs

//***********************************************************************************
// global variables
//***********************************************************************************
typedef struct {
	bool 			debugRun;			// True = keep LETIMER running while halted
	bool 			enable;				// enable the LETIMER upon completion of open
	uint8_t			out_pin_route0;		// out 0 route to gpio port/pin
	uint8_t			out_pin_route1;		// out 1 route to gpio port/pin
	bool			out_pin_0_en;		// enable out 0 route
	bool			out_pin_1_en;		// enable out 1 route
	float			period;				// seconds
	float			active_period;		// seconds
} APP_LETIMER_PWM_TypeDef ;


//***********************************************************************************
// function prototypes
//***********************************************************************************
void letimer_pwm_open(LETIMER_TypeDef *letimer, APP_LETIMER_PWM_TypeDef *app_letimer_struct);
void letimer_start(LETIMER_TypeDef *letimer, bool enable);
