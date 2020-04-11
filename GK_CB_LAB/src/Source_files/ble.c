/**
 * @file ble.c
 * @author Keith Graham
 * @date November 28th, 2019
 * @brief Contains all the functions to interface the application with the HM-18
 *   BLE module and the LEUART driver
 *
 */


//***********************************************************************************
// Include files
//***********************************************************************************
#include "ble.h"
#include <string.h>

//***********************************************************************************
// defined files
//***********************************************************************************


//***********************************************************************************
// private function prototypes
//***********************************************************************************
static uint8_t ble_circ_space(void);
static void update_circ_wrtindex(BLE_CIRCULAR_BUF *index_struct, uint32_t update_by);
static void update_circ_readindex(BLE_CIRCULAR_BUF *index_struct, uint32_t update_by);

//***********************************************************************************
// private variables
//***********************************************************************************
CIRC_TEST_STRUCT test_struct;
static BLE_CIRCULAR_BUF ble_cbuf;
static char pop_str[CSIZE];
/***************************************************************************//**
 * @brief BLE module
 * @details
 *  This module contains all the functions to interface the application layer
 *  with the HM-18 Bluetooth module.  The application does not have the
 *  responsibility of knowing the physical resources required, how to
 *  configure, or interface to the Bluetooth resource including the LEUART
 *  driver that communicates with the HM-18 BLE module.
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
 *   Function to open a LEUART port for the BLE HM10/HM18 module.
 *
 * @details
 * 	This function creates the LEUART_OPEN_STRUCT which specifies the configuration
 * 	required of the LEUART peripheral that will be used to communicate with the BLE
 * 	module. It then opens a LEUART port using this information.
 *
 * @param[in] tx_event
 *   The scheduler event associated with a TX Done Event.
 *
 * @param[in] rx_event
 *   The scheduler event associated with a RX Done Event.
 *
 ******************************************************************************/

void ble_open(uint32_t tx_event, uint32_t rx_event){
	LEUART_OPEN_STRUCT leuart_settings;

	ble_circ_init();

	leuart_settings.baudrate = HM10_BAUDRATE;
	leuart_settings.databits = HM10_DATABITS;
	leuart_settings.enable = HM10_ENABLE;
	leuart_settings.parity = HM10_PARITY;
	leuart_settings.stopbits = HM10_STOPBITS;
	leuart_settings.refFreq = HM10_REFFREQ;
	leuart_settings.rx_done_evt = rx_event;
	leuart_settings.tx_done_evt = tx_event;
	leuart_settings.tx_en = TX_DEFAULT_ENABLE;
	leuart_settings.tx_loc = LEUART0_TX_ROUTE;
	leuart_settings.tx_pin_en = TX_DEFAULT_ENABLE;
	leuart_settings.rx_en = RX_DEFAULT_ENABLE;
	leuart_settings.rx_loc = LEUART0_RX_ROUTE;
	leuart_settings.rx_pin_en = RX_DEFAULT_ENABLE;

	leuart_open(HM10_LEUART0, &leuart_settings);
}

/***************************************************************************//**
 * @brief
 *   This is a function to write a string to a BLE module.
 * @details
 *   This function packages a write request to send a string over LEUART for
 *   the bluetooth module.
 *
 * @param[in] string
 *   the string that shall be transmitted over BLE
 *
 ******************************************************************************/

void ble_write(char* string){
	ble_circ_push(string);
	ble_circ_pop(false);
	//leuart_start(HM10_LEUART0, string, strlen(string));
}

/***************************************************************************//**
 * @brief
 *   BLE Test performs two functions.  First, it is a Test Driven Development
 *   routine to verify that the LEUART is correctly configured to communicate
 *   with the BLE HM-18 module.  Second, the input argument passed to this
 *   function will be written into the BLE module and become the new name
 *   advertised by the module while it is looking to pair.
 *
 * @details
 * 	 This global function will use polling functions provided by the LEUART
 * 	 driver for both transmit and receive to validate communications with
 * 	 the HM-18 BLE module.  For the assignment, the communication with the
 * 	 BLE module must use low energy design principles of being an interrupt
 * 	 driven state machine.
 *
 * @note
 *   For this test to run to completion, the phone most not be paired with
 *   the BLE module.  In addition for the name to be stored into the module
 *   a breakpoint must be placed at the end of the test routine and stopped
 *   at this breakpoint while in the debugger for a minimum of 5 seconds.
 *
 * @param[in] *mod_name
 *   The name that will be written to the HM-18 BLE module to identify it
 *   while it is advertising over Bluetooth Low Energy.
 *
 * @return
 *   Returns bool true if successfully passed through the tests in this
 *   function.
 ******************************************************************************/

bool ble_test(char *mod_name){
	uint32_t	str_len;

	__disable_irq();
	// This test will limit the test to the proper setup of the LEUART
	// peripheral, routing of the signals to the proper pins, pin
	// configuration, and transmit/reception verification.  The test
	// will communicate with the BLE module using polling routines
	// instead of interrupts.

	// How is polling different than using interrupts?
	// ANSWER:
	// Polling uses CPU cycles constantly to check for an event, whereas interrupts allow the CPU to go to sleep until needed.
	// In polling you might react to an event later, also, because you'd be checking at time intervals instead of reacting to when it actually happens.

	// How does interrupts benefit the system for low energy operation?
	// ANSWER:
	// The CPU (which draws the most energy usually, compared to peripherals) can go to sleep and is only enabled when needed

	// How does interrupts benefit the system that has multiple tasks?
	// ANSWER:
	// It allows the system to initiate multiple tasks at the same time and then wait for/handle the responses,
	//instead of waiting for one task to complete before initiating the next.

	// First, you will need to review the DSD HM10 datasheet to determine
	// what the default strings to write data to the BLE module and the
	// expected return statement from the BLE module to test / verify the
	// correct response

	// The break_str is used to tell the BLE module to end a Bluetooth connection
	// such as with your phone.  The ok_str is the result sent from the BLE module
	// to the micro-controller if there was not active BLE connection at the time
	// the break command was sent to the BLE module.
	// Replace the break_str "" with the command to break or end a BLE connection
	// Replace the ok_str "" with the result that will be returned from the BLE
	//   module if there was no BLE connection
	char		break_str[80] = "AT";
	char		ok_str[80] = "OK";

	// output_str will be the string that will program a name to the BLE module.
	// From the DSD HM10 datasheet, what is the command to program a name into
	// the BLE module?
	// The  output_str will be a string concatenation of the DSD HM10 command
	// and the input argument sent to the ble_test() function
	// Replace the otput_str "" with the command to change the program name
	// Replace the result_str "" with the first part of the expected result
	//  the backend of the expected response will be concatenated with the
	//  input argument
	char		output_str[80] = "AT+NAME";
	char		result_str[80] = "OK+Set:";

	// To program the name into your module, you must reset the module after you
	// have sent the command to update the modules name.  What is the DSD HM10
	// name to reset the module?
	// Replace the reset_str "" with the command to reset the module
	// Replace the reset_result_str "" with the expected BLE module response to
	//  to the reset command
	char		reset_str[80] = "AT+RESET";
	char		reset_result_str[80] = "OK+RESET";
	char		return_str[80] = "";

	bool		success;
	bool		rx_disabled, rx_en, tx_en;
	uint32_t	status;

	// These are the routines that will build up the entire command and response
	// of programming the name into the BLE module.  Concatenating the command or
	// response with the input argument name
	strcat(output_str, mod_name);
	strcat(result_str, mod_name);

	// The test routine must not alter the function of the configuration of the
	// LEUART driver, but requires certain functionality to insure the logical test
	// of writing and reading to the DSD HM10 module.  The following c-lines of code
	// save the current state of the LEUART driver that will be used later to
	// re-instate the LEUART configuration

	status = leuart_status(HM10_LEUART0);
	if (status & LEUART_STATUS_RXBLOCK) {
		rx_disabled = true;
		// Enabling, unblocking, the receiving of data from the LEUART RX port
		leuart_cmd_write(HM10_LEUART0, LEUART_CMD_RXBLOCKDIS);
	}
	else rx_disabled = false;
	if (status & LEUART_STATUS_RXENS) {
		rx_en = true;
	} else {
		rx_en = false;
		// Enabling the receiving of data from the RX port
		leuart_cmd_write(HM10_LEUART0, LEUART_CMD_RXEN);
		while (!(leuart_status(HM10_LEUART0) & LEUART_STATUS_RXENS));
	}

	if (status & LEUART_STATUS_TXENS){
		tx_en = true;
	} else {
		// Enabling the transmission of data to the TX port
		leuart_cmd_write(HM10_LEUART0, LEUART_CMD_TXEN);
		while (!(leuart_status(HM10_LEUART0) & LEUART_STATUS_TXENS));
		tx_en = false;
	}
//	leuart_cmd_write(HM10_LEUART0, (LEUART_CMD_CLEARRX | LEUART_CMD_CLEARTX));

	// This sequence of instructions is sending the break ble connection
	// to the DSD HM10 module.
	// Why is this command required if you want to change the name of the
	// DSD HM10 module?
	// ANSWER: This is going to check whether the device is connected.
	// It will simply respond "OK" if the device is not connected, and
	// will respond "OK+LOST" if there is a connection, and then
	// disconnect the remote device.
	str_len = strlen(break_str);
	for (int i = 0; i < str_len; i++){
		leuart_app_transmit_byte(HM10_LEUART0, break_str[i]);
	}

	// What will the ble module response back to this command if there is
	// a current ble connection?
	// ANSWER: OK+LOST
	str_len = strlen(ok_str);
	for (int i = 0; i < str_len; i++){
		return_str[i] = leuart_app_receive_byte(HM10_LEUART0);
		if (ok_str[i] != return_str[i]) {
				EFM_ASSERT(false);;
		}
	}

	// This sequence of code will be writing or programming the name of
	// the module to the DSD HM10
	str_len = strlen(output_str);
	for (int i = 0; i < str_len; i++){
		leuart_app_transmit_byte(HM10_LEUART0, output_str[i]);
	}

	// Here will be the check on the response back from the DSD HM10 on the
	// programming of its name
	//return_str[0] = "x";
	str_len = strlen(result_str);
	for (int i = 0; i < str_len; i++){
		return_str[i] = leuart_app_receive_byte(HM10_LEUART0);
		if (result_str[i] != return_str[i]) {
				EFM_ASSERT(false);;
		}
	}

	// It is now time to send the command to RESET the DSD HM10 module
	str_len = strlen(reset_str);
	for (int i = 0; i < str_len; i++){
		leuart_app_transmit_byte(HM10_LEUART0, reset_str[i]);
	}

	// After sending the command to RESET, the DSD HM10 will send a response
	// back to the micro-controller
	str_len = strlen(reset_result_str);
	for (int i = 0; i < str_len; i++){
		return_str[i] = leuart_app_receive_byte(HM10_LEUART0);
		if (reset_result_str[i] != return_str[i]) {
				EFM_ASSERT(false);;
		}
	}

	// After the test and programming have been completed, the original
	// state of the LEUART must be restored
	if (!rx_en) leuart_cmd_write(HM10_LEUART0, LEUART_CMD_RXDIS);
	if (rx_disabled) leuart_cmd_write(HM10_LEUART0, LEUART_CMD_RXBLOCKEN);
	if (!tx_en) leuart_cmd_write(HM10_LEUART0, LEUART_CMD_TXDIS);
	leuart_if_reset(HM10_LEUART0);

	success = true;

	__enable_irq();
	return success;
}

/***************************************************************************//**
 * @brief
 *   Circular buffer test
 *
 * @details
 *   This tests the circular buffer, ensuring that it wraps around properly
 *   as well as allowing multiple pushes and pops.
 *
 * @note
 *  At the end it will transmit over bluetooth that it passed the circular
 *  buffer test.
 *
 ******************************************************************************/

void circular_buff_test(void){
	 bool buff_empty;
	 int test1_len = 50;
	 int test2_len = 25;
	 int test3_len = 5;

	 // Why this 0 initialize of read and write pointer?
	 // Student Response:
	 // This initializes both to point at the 0 index of the buffer because the buffer is empty.
	 ble_cbuf.read_ptr = 0;
	 ble_cbuf.write_ptr = 0;

	 // Why do none of these test strings contain a 0?
	 // Student Response:
	 // If it puts a 0, it will be a null character. This just fills them with random integers, which
	 // will not print out as anything useful (since it would be translated to ASCII / Unicode) but
	 // if it puts a zero that will be interpreted as the end of the string.
	 for (int i = 0;i < test1_len; i++){
		 test_struct.test_str[0][i] = i+1;
	 }
	 for (int i = 0;i < test2_len; i++){
		 test_struct.test_str[1][i] = i + 20;
	 }
	 for (int i = 0;i < test3_len; i++){
		 test_struct.test_str[2][i] = i +  35;
	 }

	 // Why is there only one push to the circular buffer at this stage of the test
	 // Student Response:
	 // We are testing push only, not multiple pushes / overflow situations
	 ble_circ_push(&test_struct.test_str[0][0]);

	 // Why is the expected buff_empty test = false?
	 // Student Response:
	 // ble_circ_pop will return false as long as you called the pop function while the buffer is not empty
	 // ble_circ_pop will return true if there is no data available to pop.
	 // EFM_ASSERT will halt the program if buff_empty is TRUE. Since we pushed something to the buffer
	 // this should return FALSE and successfully pop.


	 buff_empty = ble_circ_pop(CIRC_TEST);
	 EFM_ASSERT(buff_empty == false);
	 for (int i = 0; i < test1_len; i++){
		 EFM_ASSERT(test_struct.test_str[0][i] == test_struct.result_str[i]);
	 }

	 // What does this next push on the circular buffer test?
	 // Student Response: it tests that push another string after popping. This would also
	 // test that the data gets wrapped around since the write pointer would have started at index 50
	 // and only has 14 remaining spaces to the end of the buffer (not 24)

	 ble_circ_push(&test_struct.test_str[1][0]);

	 // What does this next push on the circular buffer test?
	 // Student Response: test whether you can have 2 strings on the buffer (no overflow condition)
	 // When we pop the 2 strings off we can also make sure that they are being popped off in order.
	 ble_circ_push(&test_struct.test_str[2][0]);


	 // Why is the expected buff_empty test = false?
	 // Student Response:
	 // ble_circ_pop will return false as long as you called the pop function while the buffer is not empty
	 // we put 2 strings on the buffer, so it should not be empty

	 buff_empty = ble_circ_pop(CIRC_TEST);
	 EFM_ASSERT(buff_empty == false);
	 for (int i = 0; i < test2_len; i++){
		 EFM_ASSERT(test_struct.test_str[1][i] == test_struct.result_str[i]);
	 }

	 // Why is the expected buff_empty test = false?
	 // Student Response:
	 // There should be 1 string remaining after the previous pop (above)

	 buff_empty = ble_circ_pop(CIRC_TEST);
	 EFM_ASSERT(buff_empty == false);
	 for (int i = 0; i < test3_len; i++){
		 EFM_ASSERT(test_struct.test_str[2][i] == test_struct.result_str[i]);
	 }

	 // Using these three writes and pops to the circular buffer, what other test
	 // could we develop to better test out the circular buffer?
	 // Student Response:
	 // we should try to write strings that do not have enough space (although in my current implementation
	 // it would assert / fail the test).


	 // Why is the expected buff_empty test = true?
	 // Student Response:
	 // The buffer should be completely empty at this point. so it should indicate that no data was popped

	 buff_empty = ble_circ_pop(CIRC_TEST);
	 EFM_ASSERT(buff_empty == true);
	 ble_write("\nPassed Circular Buffer Test\n");

}
/***************************************************************************//**
 * @brief
 *   Circular buffer init
 *
 * @details
 * 	Initializes the circular buffer for the BLE module.
 *
 * 	Sets size, read_ptr and write_ptr to 0.
 * 	Sets size_mask to be 1 less than CSIZE.
 * 	Initializes all chars in the buffer to 0 / null.
 *
 *
 ******************************************************************************/
void ble_circ_init(void){
	memset(&ble_cbuf.cbuf[0], 0, CSIZE);
	ble_cbuf.size_mask = CSIZE-1;
	ble_cbuf.size = 0;
	ble_cbuf.read_ptr = 0;
	ble_cbuf.write_ptr = 0;
}


/***************************************************************************//**
 * @brief
 *   Circular buffer push
 *
 * @details
 *	Pushes a string on to the circular buffer.
 *
 * @note
 * 	This function is atomic.
 *
 * @param[in] *string
 *   pointer to the string that will be added to the buffer.
 *
 ******************************************************************************/
void ble_circ_push(char *string){
	__disable_irq();

	uint8_t str_len= strlen(string);
	if(str_len == 0) {
		__enable_irq();
		return;
	}

	uint8_t space = 0;
	space = ble_circ_space();
	EFM_ASSERT(space != 0); // will assert if no space left

	uint8_t packet_len = str_len + 1;
	EFM_ASSERT(packet_len <= space); // packet length must be less than or equal to available space

	ble_cbuf.cbuf[ble_cbuf.write_ptr] = packet_len; // put packet_length in buffer at index Head
	update_circ_wrtindex(&ble_cbuf, 1); // update write pointer
	ble_cbuf.size++; // increment counter

	int i;
	for(i=0; i < str_len; i++){
		ble_cbuf.cbuf[ble_cbuf.write_ptr] = string[i];
		update_circ_wrtindex(&ble_cbuf, 1); // increment head
		ble_cbuf.size++; // increment byte counter
	}

	__enable_irq();
}


/***************************************************************************//**
 * @brief
 *   Circular buffer pop
 *
 * @details
 *	Pops a string off the circular buffer and transmits over LEUART to BLE device.
 *
 * @param[in] test
 *   test boolean flag
 *
 * @return
 *   Returns bool false if the buffer is not empty. Will assert/halt the program
 *    if there is a critical error and will return true if there is nothing to pop.
 *
 ******************************************************************************/
bool ble_circ_pop(bool test){
	__disable_irq();

	if (!leuart_idle()) {
		__enable_irq();
		return false;
	}

	if(ble_cbuf.size == 0) {
		__enable_irq();
		return true;
	}
	EFM_ASSERT(ble_cbuf.size != 1); // if only 1 byte in buffer, malformed packet, halt

	uint8_t str_len = ble_cbuf.cbuf[ble_cbuf.read_ptr] - 1; // sub 1 because it returns packet length
	update_circ_readindex(&ble_cbuf, 1);
	ble_cbuf.size--; // remove packet len from byte count

	EFM_ASSERT(ble_cbuf.size >= str_len); // buffer must contain at least the # of bytes indicated by str_len if not more

	int i;
//	for(i=0; i < CSIZE; i++){
//		pop_str[i] = 0;
//	}
//	// filling with zeros means it will definitely be null terminated.
//	// Even if there's the maximum # of chars, there will be 1 spot left
//	//at the end bc the buffer includes the extra byte for the length

	for(i=0; i < str_len; i++){
		pop_str[i] = ble_cbuf.cbuf[ble_cbuf.read_ptr];
		update_circ_readindex(&ble_cbuf, 1); // increment tail
		ble_cbuf.size--; // increment byte counter
	}
	pop_str[str_len] = 0;

	if(test){
		memcpy(test_struct.result_str, pop_str, str_len+1); // +1 to make sure a null char is copied too
	} else {
		leuart_start(HM10_LEUART0, pop_str, strlen(pop_str));//str_len+1);
		//ble_write(pop_str); // ble write calls leuart start
	}

	__enable_irq();
	return false;
}

/***************************************************************************//**
 * @brief
 *   BLE Circ Space
 *
 * @details
 *   checks if there is space in the buffer and return how many spaces are available
 *
 * @return
 *  8 bit integer indicating the number of spaces available in the buffer.
 *
 ******************************************************************************/
static uint8_t ble_circ_space(void){
	return CSIZE - ble_cbuf.size;
}

/***************************************************************************//**
 * @brief
 *   Update circular buffer write index
 *
 * @details
 *   This increments the write pointer by the update_by value and does a bitwise
 *   AND with the size mask to effectively perform a fast modulo.
 *
 * @note
 * 	the length of the buffer must be a power of 2.
 *
 *
 * @param[in] *index_struct
 *   Pointer to the circular buffer struct to update
 *
 *
 * @param[in] update_by
 *   32 bit integer indicating the number to update the write index by
 *
 *
 ******************************************************************************/
static void update_circ_wrtindex(BLE_CIRCULAR_BUF *index_struct, uint32_t update_by){
	index_struct->write_ptr = (index_struct->write_ptr + update_by) & index_struct->size_mask;
}

/***************************************************************************//**
 * @brief
 *   Update circular buffer read index
 *
 * @details
 *  This increments the read pointer by the update_by value and does a bitwise
 *   AND with the size mask to effectively perform a fast modulo.
 *
 * @note
 * 	the length of the buffer must be a power of 2.
 *
 * @param[in] *index_struct
 *   Pointer to the circular buffer struct to update
 *
 * @param[in] update_by
 *   32 bit integer indicating the number to update the read index by
 *
 *
 ******************************************************************************/
static void update_circ_readindex(BLE_CIRCULAR_BUF *index_struct, uint32_t update_by){
	index_struct->read_ptr = (index_struct->read_ptr + update_by) & index_struct->size_mask;
}
