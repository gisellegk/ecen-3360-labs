/**
 * @file leuart.c
 * @author Keith Graham
 * @date November 27th, 2019
 * @brief Contains all the functions of the LEUART peripheral
 *
 */

//***********************************************************************************
// Include files
//***********************************************************************************

//** Standard Library includes
#include <string.h>

//** Silicon Labs include files
#include "em_gpio.h"
#include "em_cmu.h"
#include "em_assert.h"

//** Developer/user include files
#include "leuart.h"
#include "scheduler.h"

//***********************************************************************************
// defined files
//***********************************************************************************
typedef enum {
	LEUART_IDLE,
	LEUART_START,
	LEUART_TRANSMIT,
	LEUART_END_OF_DATA
} LEUART_State;

typedef struct {
	LEUART_State		state;
	LEUART_TypeDef* 	leuart;
	char*				string;
	uint8_t				string_length;
	uint8_t				char_index;
} LEUART_PAYLOAD_STRUCT;
//***********************************************************************************
// private variables
//***********************************************************************************
uint32_t							rx_done_evt;
uint32_t							tx_done_evt;
bool								leuart0_tx_busy;
static LEUART_PAYLOAD_STRUCT 		leuart_payload;

//***********************************************************************************
// private function prototypes
//***********************************************************************************
static void leuart_txc(void);
static void leuart_txbl(void);

/***************************************************************************//**
 * @brief LEUART driver
 * @details
 *  This module contains all the functions to support the driver's state
 *  machine to transmit a string of data across the LEUART bus.  There are
 *  additional functions to support the Test Driven Development test that
 *  is used to validate the basic set up of the LEUART peripheral.  The
 *  TDD test for this class assumes that the LEUART is connected to the HM-18
 *  BLE module.  These TDD support functions could be used for any TDD test
 *  to validate the correct setup of the LEUART.
 *
 ******************************************************************************/

//***********************************************************************************
// Private functions
//***********************************************************************************



//***********************************************************************************
// Global functions
//***********************************************************************************



/***************************************************************************//**
 * @brief
 *   Function to open Low Energy UART peripheral communication port.
 *
 * @details
 *   This routine initializes a LEUART port, routes the RX and TX pins, and
 *   enables / clears RX and TX and their respective buffers as needed.
 *
 * @param[in] leuart
 *   Pointer to the base peripheral address of the LEUART peripheral being used.
 *
 * @param[in] leuart_settings
 *   Pointer to the LEUART_OPEN_STRUCT containing the desired settings for this
 *   instance of the LEUART port.
 *
 * ******************************************************************************/

void leuart_open(LEUART_TypeDef *leuart, LEUART_OPEN_STRUCT *leuart_settings){
	LEUART_Init_TypeDef init;

	// Enable Peripheral Clock for LEUART
	if(leuart == LEUART0) {
		CMU_ClockEnable(cmuClock_LEUART0, true);
	} else {
		EFM_ASSERT(false);
	}

	// confirm successful clock enable
	leuart->STARTFRAME = 0x01;
	while(leuart->SYNCBUSY);
	EFM_ASSERT(leuart->STARTFRAME == 0x01);

	leuart->STARTFRAME = 0x0;
	while(leuart->SYNCBUSY);
	EFM_ASSERT(leuart->STARTFRAME == 0x0);

	init.baudrate = leuart_settings->baudrate;
	init.databits = leuart_settings->databits;
	init.enable = leuart_settings->enable;
	init.parity = leuart_settings->parity;
	init.stopbits = leuart_settings->stopbits;
	init.refFreq = leuart_settings->refFreq;

	LEUART_Init(leuart, &init);
	while(leuart->SYNCBUSY);

	// Route RX and TX Pins
	leuart->ROUTELOC0 = (leuart_settings->rx_loc << _LEUART_ROUTELOC0_RXLOC_SHIFT)
					| (leuart_settings->tx_loc << _LEUART_ROUTELOC0_TXLOC_SHIFT);
	leuart->ROUTEPEN = (leuart_settings->rx_pin_en << _LEUART_ROUTEPEN_TXPEN_SHIFT)
					| (leuart_settings->tx_pin_en << _LEUART_ROUTEPEN_RXPEN_SHIFT );

	// Clear RX and TX buffers, enables RX and TX as necessary.
	leuart->CMD = (LEUART_CMD_CLEARRX & leuart_settings->rx_en << _LEUART_CMD_CLEARRX_SHIFT)
					| (LEUART_CMD_CLEARTX & leuart_settings->tx_en << _LEUART_CMD_CLEARTX_SHIFT)
					| (leuart_settings->rx_en << _LEUART_CMD_RXEN_SHIFT )
					| (leuart_settings->tx_en << _LEUART_CMD_TXEN_SHIFT );

	if(leuart_settings->tx_en){
		while(!(leuart->STATUS & LEUART_STATUS_TXENS)); // wait for TX to be enabled.
	}
	if(leuart_settings->rx_en){
		while(!(leuart->STATUS & LEUART_STATUS_RXENS)); // wait for RX to be enabled.
	}
	while(leuart->SYNCBUSY );

	// configure interrupts
	uint32_t interrupts = LEUART_IEN_TXC
						| LEUART_IEN_TXBL;

	LEUART_IntClear(leuart, interrupts);
	//LEUART_IntEnable(leuart, interrupts); // do not enable these right now.

	if(leuart == LEUART0){
		NVIC_EnableIRQ(LEUART0_IRQn);
	} else {
		EFM_ASSERT(false);
	}

	// Check that TX and RX are properly enabled.
	EFM_ASSERT((leuart->STATUS & LEUART_STATUS_TXENS) & leuart_settings->tx_en << _LEUART_STATUS_TXENS_SHIFT);
	EFM_ASSERT((leuart->STATUS & LEUART_STATUS_RXENS) & leuart_settings->rx_en << _LEUART_STATUS_RXENS_SHIFT);

	rx_done_evt = leuart_settings->rx_done_evt;
	tx_done_evt = leuart_settings->tx_done_evt;
	leuart_payload.state = LEUART_IDLE;

}


/***************************************************************************//**
 * @brief
 *
 * ******************************************************************************/

void leuart_start(LEUART_TypeDef *leuart, char *string, uint32_t string_len){
	EFM_ASSERT(leuart->STATUS & LEUART_STATUS_TXIDLE); // must be 1 / idle
	EFM_ASSERT(leuart_payload.state == LEUART_IDLE); // state must be idle

	leuart_payload.state = LEUART_START;
	sleep_block_mode(LEUART_TX_EM_BLOCK);

	EFM_ASSERT(string_len > 0);

	leuart_payload.string_length = string_len;
	leuart_payload.char_index = 0;

	LEUART_IntEnable(leuart, LEUART_IEN_TXBL);

	leuart_payload.state = LEUART_TRANSMIT;
	// write char n
	leuart_payload.leuart->TXDATA = leuart_payload.string[leuart_payload.char_index];
	// n++
	leuart_payload.char_index++;
}

/***************************************************************************//**
 * @brief
 *   Function that the LEUART interrupt handler will call upon receiving the
 *   LEUART TXBL interrupt
 *
 * @details
 * 	 This function defines the behavior of the state machine in each state when
 * 	 a LEUART TXBL interrupt is received.
 *
 *
 * ******************************************************************************/

static void leuart_txbl(void){
	switch(leuart_payload.state){
		case LEUART_IDLE:
			EFM_ASSERT(false);
			break;
		case LEUART_START:
			EFM_ASSERT(false);
			break;
		case LEUART_TRANSMIT:
			// if n >= string_length, string is done. state = END_OF_DATA
			if(leuart_payload.char_index >= leuart_payload.string_length) {
				leuart_payload.state = LEUART_END_OF_DATA;
				LEUART_IntDisable(leuart_payload.leuart, LEUART_IEN_TXBL); // disable TXBL
				LEUART_IntEnable(leuart_payload.leuart, LEUART_IEN_TXC); // enable TXC
			} else {
				// transmit the next char
				// write char n
				leuart_payload.leuart->TXDATA = leuart_payload.string[leuart_payload.char_index];
				// n++
				leuart_payload.char_index++;
			}
			break;
		case LEUART_END_OF_DATA:
			EFM_ASSERT(false);
			break;

		}
}

/***************************************************************************//**
 * @brief
 *   Function that the LEUART interrupt handler will call upon receiving the
 *   LEUART TXC interrupt
 *
 * @details
 * 	 This function defines the behavior of the state machine in each state when
 * 	 a LEUART TXC interrupt is received.
 *
 *
 * ******************************************************************************/

static void leuart_txc(void){
	switch(leuart_payload.state){
	case LEUART_IDLE:
		EFM_ASSERT(false);
		break;
	case LEUART_START:
		EFM_ASSERT(false);
		break;
	case LEUART_TRANSMIT:
		EFM_ASSERT(false);
		break;
	case LEUART_END_OF_DATA:
		LEUART_IntDisable(leuart_payload.leuart, LEUART_IEN_TXC);
		sleep_unblock_mode(LEUART_TX_EM_BLOCK);
		leuart_payload.state = LEUART_IDLE;
		add_scheduled_event(tx_done_evt);
		// Disable TXC
		// Unblock EM3
		// set state to IDLE
		break;
	}

}

/***************************************************************************//**
 * @brief
 *   IRQ handler for LEUART0.
 *
 * @details
 * 	 This is an IRQ handler for LEUART0. It is used to determine when the LEUART0
 * 	 is available to transmit and when the LEUART0 has completed transmission.
 *
 * 	 It uses the TXBL and TXC interrupts.
 *
 * @note
 *   The BLE test uses polling, not interrupts to function. Normal BLE functionality
 *   will use interrupts in order to maintain low energy operation.
 *
 *
 * ******************************************************************************/

void LEUART0_IRQHandler(void){
	uint32_t interrupt_flags = LEUART_IntGet(LEUART0) & LEUART_IntGetEnabled(LEUART0);
	LEUART_IntClear(LEUART0, interrupt_flags);
	if(interrupt_flags & LEUART_IEN_TXBL){
		leuart_txbl();
	}
	if(interrupt_flags & LEUART_IEN_TXC){
		leuart_txc();
	}
}


/***************************************************************************//**
 * @brief
 ******************************************************************************/

bool leuart_tx_busy(LEUART_TypeDef *leuart){

}

/***************************************************************************//**
 * @brief
 *   LEUART STATUS function returns the STATUS of the peripheral for the
 *   TDD test
 *
 * @details
 * 	 This function enables the LEUART STATUS register to be provided to
 * 	 a function outside this .c module.
 *
 * @param[in] *leuart
 *   Defines the LEUART peripheral to access.
 *
 * @return
 * 	 Returns the STATUS register value as an uint32_t value
 *
 ******************************************************************************/

uint32_t leuart_status(LEUART_TypeDef *leuart){
	uint32_t	status_reg;
	status_reg = leuart->STATUS;
	return status_reg;
}

/***************************************************************************//**
 * @brief
 *   LEUART CMD Write sends a command to the CMD register
 *
 * @details
 * 	 This function is used by the TDD test function to program the LEUART
 * 	 for the TDD tests.
 *
 * @note
 *   Before exiting this function to update  the CMD register, it must
 *   perform a SYNCBUSY while loop to ensure that the CMD has by synchronized
 *   to the lower frequency LEUART domain.
 *
 * @param[in] *leuart
 *   Defines the LEUART peripheral to access.
 *
 * @param[in] cmd_update
 * 	 The value to write into the CMD register
 *
 ******************************************************************************/

void leuart_cmd_write(LEUART_TypeDef *leuart, uint32_t cmd_update){

	leuart->CMD = cmd_update;
	while(leuart->SYNCBUSY);
}

/***************************************************************************//**
 * @brief
 *   LEUART IF Reset resets all interrupt flag bits that can be cleared
 *   through the Interrupt Flag Clear register
 *
 * @details
 * 	 This function is used by the TDD test program to clear interrupts before
 * 	 the TDD tests and to reset the LEUART interrupts before the TDD
 * 	 exits
 *
 * @param[in] *leuart
 *   Defines the LEUART peripheral to access.
 *
 ******************************************************************************/

void leuart_if_reset(LEUART_TypeDef *leuart){
	leuart->IFC = 0xffffffff;
}

/***************************************************************************//**
 * @brief
 *   LEUART App Transmit Byte transmits a byte for the LEUART TDD test
 *
 * @details
 * 	 The BLE module will respond to AT commands if the BLE module is not
 * 	 connected to the phone app.  To validate the minimal functionality
 * 	 of the LEUART peripheral, write and reads to the LEUART will be
 * 	 performed by polling and not interrupts.
 *
 * @note
 *   In polling a transmit byte, a while statement checking for the TXBL
 *   bit in the Interrupt Flag register is required before writing the
 *   TXDATA register.
 *
 * @param[in] *leuart
 *   Defines the LEUART peripheral to access.
 *
 * @param[in] data_out
 *   Byte to be transmitted by the LEUART peripheral
 *
 ******************************************************************************/

void leuart_app_transmit_byte(LEUART_TypeDef *leuart, uint8_t data_out){
	while (!(leuart->IF & LEUART_IF_TXBL));
	leuart->TXDATA = data_out;
}


/***************************************************************************//**
 * @brief
 *   LEUART App Receive Byte polls a receive byte for the LEUART TDD test
 *
 * @details
 * 	 The BLE module will respond to AT commands if the BLE module is not
 * 	 connected to the phone app.  To validate the minimal functionality
 * 	 of the LEUART peripheral, write and reads to the LEUART will be
 * 	 performed by polling and not interrupts.
 *
 * @note
 *   In polling a receive byte, a while statement checking for the RXDATAV
 *   bit in the Interrupt Flag register is required before reading the
 *   RXDATA register.
 *
 * @param[in] leuart
 *   Defines the LEUART peripheral to access.
 *
 * @return
 * 	 Returns the byte read from the LEUART peripheral
 *
 ******************************************************************************/

uint8_t leuart_app_receive_byte(LEUART_TypeDef *leuart){
	uint8_t leuart_data;
	while (!(leuart->IF & LEUART_IF_RXDATAV));
	leuart_data = leuart->RXDATA;
	return leuart_data;
}
