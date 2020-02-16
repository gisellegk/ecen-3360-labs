/**
 * @file sleep_routines.c
 * @author Giselle Koo
 * @date February 6th, 2020
 * @brief Contains all sleep routine functions
 *
 *
 ***************************************************************************
 * @section License
 * <b>(C) Copyright 2015 Silicon Labs, http://www.silabs.com</b>
 ****************************************************************************
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must not
 *    claim that you wrote the original software.
 * 2. Altered source versions must be plainly marked as such, and must not be
 *    misrepresented as being the original software.
 * 3. This notice may not be removed or altered from any source distribution.
 *
 * DISCLAIMER OF WARRANTY/LIMITATION OF REMEDIES: Silicon Labs has no
 * obligation to support this Software. Silicon Labs is providing the
 * Software "AS IS", with no express or implied warranties of any kind,
 * including, but not limited to, any implied warranties of merchantability
 * or fitness for any particular purpose or warranties against infringement
 * of any proprietary rights of a third party.
 *
 * Silicon Labs will not be liable for any consequential, incidental, or
 * special damages, or any other relief, or for any claim by any third party,
 * arising from your use of this Software.
 ***************************************************************************/

//***********************************************************************************
// Include files
//***********************************************************************************

//** Standard Libraries

//** Silicon Lab include files
#include "em_cmu.h"
#include "em_assert.h"

//** User/developer include files
#include "sleep_routines.h"

//***********************************************************************************
// defined files
//***********************************************************************************


//***********************************************************************************
// global variables
//***********************************************************************************
#define UNBLOCKED 0

//***********************************************************************************
// private variables
//***********************************************************************************
static int lowest_energy_mode[MAX_ENERGY_MODES];

//***********************************************************************************
// functions
//***********************************************************************************

/***************************************************************************//**
 * @brief
 *   Used enable sleep modes
 *
 * @details
 * 	 Initialize private variable for sleep_routines so all energy modes are unblocked
 *
 ******************************************************************************/
void sleep_open(void){
	lowest_energy_mode[EM0] = UNBLOCKED;
	lowest_energy_mode[EM1] = UNBLOCKED;
	lowest_energy_mode[EM2] = UNBLOCKED;
	lowest_energy_mode[EM3] = UNBLOCKED;
	lowest_energy_mode[EM4] = UNBLOCKED;
}

/***************************************************************************//**
 * @brief
 *   Sleep Block Mode
 *
 * @details
 *	Utilized by a peripheral to prevent the Pearl Gecko from going
 *	into that sleep mode while the peripheral is active.
 *
 * @param[in] EM
 *   Unsigned 32 bit integer representing the Energy Mode (EM0 - EM4)
 *
 ******************************************************************************/
void sleep_block_mode(uint32_t EM){
	lowest_energy_mode[EM]++;
	EFM_ASSERT(lowest_energy_mode[EM] < 10);
}

/***************************************************************************//**
 * @brief
 *   Sleep Unblock Mode
 *
 * @details
 *	Utilized to release the processor from going into a sleep mode
 *	when a peripheral is no longer active.
 *
 * @param[in] EM
 *   Unsigned 32 bit integer representing the Energy Mode (EM0 - EM4)
 *
 ******************************************************************************/
void sleep_unblock_mode(uint32_t EM){
	lowest_energy_mode[EM]--;
	EFM_ASSERT(lowest_energy_mode[EM] >= 0);
}

/***************************************************************************//**
 * @brief
 *   Enter Sleep
 *
 * @details
 *  Function to enter sleep mode
 *
 ******************************************************************************/
void enter_sleep(void){
	if(lowest_energy_mode[EM0] > 0) return;
	else if(lowest_energy_mode[EM1] > 0) return;
	else if (lowest_energy_mode[EM2] > 0) {
		EMU_EnterEM1();
		return;
	} else if (lowest_energy_mode[EM3] > 0){
		EMU_EnterEM2(true);
		return;
	} else {
		EMU_EnterEM3(true);
		return;
	}
}
/***************************************************************************//**
 * @brief
 *   Returns current block energy mode
 *
 * @details
 *	Function that returns which energy mode that the current system cannot enter.
 *
 * @return
 * 	A 32 bit integer with the current block energy mode.
 *
 ******************************************************************************/
uint32_t current_block_energy_mode(void){
	int i;
	for(i = 0; i < MAX_ENERGY_MODES; i++ ){
		if(lowest_energy_mode[i] != 0) return i;
	}
	return MAX_ENERGY_MODES-1;
}
