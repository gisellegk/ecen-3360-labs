/*
 * @file scheduler.c
 * @author Giselle Koo
 * @date Feb 4, 2020
 * @brief Contains all the scheduler functions
 */

//***********************************************************************************
// Include files
//***********************************************************************************

//** Silicon Lab include files
#include "em_emu.h"
#include "em_assert.h"

//** User/developer include files
#include "scheduler.h"

//***********************************************************************************
// private variables
//***********************************************************************************
static unsigned int event_scheduled;

//***********************************************************************************
// functions
//***********************************************************************************

/***************************************************************************//**
 * @brief
 *   Initialize the scheduler
 *
 * @details
 * 	 This routine resets the #event_scheduled variable
 *
 * @note
 *
 *
 ******************************************************************************/

void scheduler_open(void){
	event_scheduled = 0;
}

/***************************************************************************//**
 * @brief
 *   Adds an event to the schedule
 *
 * @details
 * 	 This routine adds an event to the #event_scheduled variable
 *
 * @note
 * 	This only adds events, cannot clear or remove them.
 *
 * @param[in] event
 * 	A 32 bit integer that contains all events that you want to add.
 *
 ******************************************************************************/

void add_scheduled_event(uint32_t event){
	event_scheduled |= event;
}

/***************************************************************************//**
 * @brief
 *   Removes an event to the schedule
 *
 * @details
 * 	 This routine removes an event to the #event_scheduled variable
 *
 * @note
 * 	This only removes events.
 *
 * @param[in] event
 * 	A 32 bit integer that contains all events that you want to remove.
 *
 ******************************************************************************/

void remove_scheduled_event(uint32_t event){
	event_scheduled &= ~event;
}


/***************************************************************************//**
 * @brief
 *   Returns the value of event_scheduled.
 *
 * @details
 * 	 This routine returns the value of the #event_scheduled variable
 *
 * @return
 * 	A 32 bit integer with the event that is scheduled.
 *
 ******************************************************************************/

uint32_t get_scheduled_event(void){
	return event_scheduled;
}
