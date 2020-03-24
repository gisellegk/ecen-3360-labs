/*
 * ble.h
 *
 *  Created on: May 22, 2019
 *      Author: xxx
 */
//***********************************************************************************
// Include files
//***********************************************************************************

//** Standard Libraries
#include <stdbool.h>
#include <stdint.h>

#include "leuart.h"

//***********************************************************************************
// defined files
//***********************************************************************************

#define HM10_LEUART0			LEUART0
#define HM10_BAUDRATE			9600
#define	HM10_DATABITS			leuartDatabits8
#define HM10_ENABLE				leuartEnable
#define HM10_PARITY				leuartNoParity
#define HM10_REFFREQ			0	// use reference clock
#define HM10_STOPBITS			leuartStopbits1

#define LEUART0_TX_ROUTE		_LEUART_ROUTELOC0_TXLOC_LOC18
#define LEUART0_RX_ROUTE		_LEUART_ROUTELOC0_RXLOC_LOC18
#define RX_DEFAULT_ENABLE 			true
#define TX_DEFAULT_ENABLE 			true

//***********************************************************************************
// global variables
//***********************************************************************************


//***********************************************************************************
// function prototypes
//***********************************************************************************
void ble_open(uint32_t tx_event, uint32_t rx_event);
void ble_write(char *string);

bool ble_test(char *mod_name);
