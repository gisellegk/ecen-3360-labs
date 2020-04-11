/*
 * ble.h
 *
 *  Created on: May 22, 2019
 *      Author: xxx
 */
//***********************************************************************************
// Include files
//***********************************************************************************
#ifndef BLE_H
#define	BLE_H
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

#define CIRC_TEST				true
#define	CIRC_OPER				false

#define	CSIZE					64 // should be a power of two for best performance
typedef struct {
	char		cbuf[CSIZE];
	uint8_t		size_mask;
	uint32_t	size;
	uint32_t	read_ptr;
	uint32_t	write_ptr;
} BLE_CIRCULAR_BUF;

#define CIRC_TEST_SIZE		3
typedef struct {
	char test_str[CIRC_TEST_SIZE][64];
	char result_str[64];
} CIRC_TEST_STRUCT;


//***********************************************************************************
// global variables
//***********************************************************************************


//***********************************************************************************
// function prototypes
//***********************************************************************************
void ble_open(uint32_t tx_event, uint32_t rx_event);
void ble_write(char *string);

void ble_circ_init(void);
void ble_circ_push(char *string);
bool ble_circ_pop(bool test);

bool ble_test(char *mod_name);
void circular_buff_test(void);
#endif
