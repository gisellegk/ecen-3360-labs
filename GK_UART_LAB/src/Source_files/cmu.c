//***********************************************************************************
// Include files
//***********************************************************************************
#include "cmu.h"

//***********************************************************************************
// defined files
//***********************************************************************************


//***********************************************************************************
// global variables
//***********************************************************************************


//***********************************************************************************
// prototypes
//***********************************************************************************
void cmu_open(void){

		CMU_ClockEnable(cmuClock_HFPER, true);

		/*
		 *  Configure Low Frequency Clock
		 *  We are using ULFRCO in this case.
		 *
		 **/


		// By default, LFRCO is enabled, disable the LFRCO oscillator
		CMU_OscillatorEnable(cmuOsc_LFRCO, false, false);	// using LFXO or ULFRCO

		// Route LF clock to the LF clock tree
		// No requirement to enable the ULFRCO oscillator.  It is always enabled in EM0-4H

		CMU_OscillatorEnable(cmuOsc_LFXO, false, false);		// Disable LFXO
		CMU_ClockSelectSet(cmuClock_LFA, cmuSelect_ULFRCO);	// route ULFRCO to proper Low Freq clock tree

		CMU_ClockEnable(cmuClock_CORELE, true);					// Enable the Low Freq clock tree

		// HF clock is enabled in main.c
}

