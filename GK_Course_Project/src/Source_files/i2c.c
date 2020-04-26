/**
 * @file i2c.c
 * @author Giselle Koo
 * @date Feb 20, 2020
 * @brief Contains all the i2c functions
 *
 */
//***********************************************************************************
// Include files
//***********************************************************************************

//** Silicon Lab include files
#include "em_i2c.h"
#include "em_cmu.h"
#include "em_assert.h"

//** User/developer include files
#include "i2c.h"
#include "sleep_routines.h"
#include "scheduler.h"

//***********************************************************************************
// defined files
//***********************************************************************************



//***********************************************************************************
// global variables
//***********************************************************************************


//***********************************************************************************
// private variables
//***********************************************************************************
static I2C_PAYLOAD_STRUCT i2c_payload;


//***********************************************************************************
// private function prototypes
//***********************************************************************************
static void i2c_ack();
static void i2c_nack();
static void i2c_rxdatav();
static void i2c_mstop();

//***********************************************************************************
// functions
//***********************************************************************************

/***************************************************************************//**
 * @brief
 *	Function to open an i2c bus.
 *
 * @details
 *	This routine initializes an i2c bus and sets its state to IDLE.
 *
 * @note
 *	This function enables the interrupt flags ACK, NACK, RXDATATV and MSTOP.
 *
 * @param[in] i2c
 * 	Pointer to the base peripheral address of the I2C peripheral being used. The
 * 	Pearl Gecko has 2 I2C peripherals.
 *
 * @param[in] i2c_open
 * 	The struct that this routine will use to configure the I2C bus.
 *
 * @param[in] i2c_io
 * 	The struct which specifies port and pin routing information for the I2C bus.
 *
 ******************************************************************************/
void i2c_open(I2C_TypeDef *i2c, I2C_OPEN_STRUCT *i2c_open, I2C_IO_STRUCT *i2c_io){
	I2C_Init_TypeDef init;

	/*  Enable the routed clock to the I2C peripheral */
	if(i2c == I2C0){
		CMU_ClockEnable(cmuClock_I2C0, true);
	} else if (i2c == I2C1){
		CMU_ClockEnable(cmuClock_I2C1, true);
	} else {
		EFM_ASSERT(false);
		// we only have i2c0 and i2c1
	}

	//confirm successful clock enable & clear IF bit 1
	if((i2c->IF & 0x01) == 0) {
		i2c->IFS = 0x01;
		EFM_ASSERT(i2c->IF & 0x01);
		i2c->IFC = 0x01;
	} else {
		i2c->IFC = 0x01;
		EFM_ASSERT( !(i2c->IF & 0x01) );
	}

	// Initialize I2C bus
	init.clhr = i2c_open->chlr;
	init.enable = i2c_open->enable;
	init.freq = i2c_open->freq;
	init.master = i2c_open->master;
	init.refFreq = i2c_open->refFreq;

	I2C_Init(i2c, &init);

	// Route SDA and SCL Pins
	i2c->ROUTELOC0 = ((i2c_open->scl_route0 << _I2C_ROUTELOC0_SCLLOC_SHIFT)
					| (i2c_open->sda_route0 << _I2C_ROUTELOC0_SDALOC_SHIFT));

	i2c->ROUTEPEN = ((i2c_open->scl_en << _I2C_ROUTEPEN_SCLPEN_SHIFT )
				    | (i2c_open->sda_en << _I2C_ROUTEPEN_SDAPEN_SHIFT ));

	// set interrupt flag bits: ACK, NACK, and RXDATAV.
	uint32_t interrupts = (1 << _I2C_IEN_ACK_SHIFT)
						| (1 << _I2C_IEN_NACK_SHIFT)
						| (1 << _I2C_IEN_RXDATAV_SHIFT)
						| (1 << _I2C_IEN_MSTOP_SHIFT);

	// clear interrupt flags
	I2C_IntClear(i2c, interrupts);

	// Enable interrupts
	I2C_IntEnable(i2c, interrupts);

	if(i2c == I2C0){
		NVIC_EnableIRQ(I2C0_IRQn);
	} else if (i2c == I2C1){
		NVIC_EnableIRQ(I2C1_IRQn);
	} else {
		EFM_ASSERT(false);
		// we only have i2c0 and i2c1
	}
	i2c_bus_reset(i2c, i2c_io);
	i2c_payload.state = I2C_IDLE; // start in idle mode
}

/***************************************************************************//**
 * @brief
 *	Function to reset the I2C state machines.
 *
 * @details
 *	This function resets the state machines of both the Pearl Gecko and external
 *	I2C devices connected to the bus.
 *
 * @note
 * 	This function resets the peripheral I2C devices by NACKing 9 times by manually
 * 	clocking the SCK pin while leaving SDA in its default asserted state.
 *
 * @param[in] i2c
 *	Pointer to the base peripheral address of the I2C peripheral being used. The
 * 	Pearl Gecko has 2 I2C peripherals.
 *
 * 	@param[in] i2c_io
 * 	The struct which specifies port and pin routing information for the I2C bus.
 *
 ******************************************************************************/
void i2c_bus_reset(I2C_TypeDef *i2c, I2C_IO_STRUCT *i2c_io){
	EFM_ASSERT(GPIO_PinInGet(i2c_io->scl_port, i2c_io->scl_pin));
	EFM_ASSERT(GPIO_PinInGet(i2c_io->sda_port, i2c_io->sda_pin));
	int i;
	for(i = 0; i < RESET_TOGGLE_NUMBER; i++){
		GPIO_PinOutToggle(i2c_io->scl_port, i2c_io->scl_pin);
	}

	i2c->CMD = I2C_CMD_ABORT;
}
/***************************************************************************//**
 * @brief
 *	IRQ handler for I2C0
 *
 * @details
 *	This is an IRQ handler for I2C0.
 *
 * @note
 *	This is currently configured to handle the interrupts for ACK, NACK, RXDATAV,
 *	and MSTOP. Interrupts are enabled in the i2c_open function.
 *
 *
 *
 ******************************************************************************/
void I2C0_IRQHandler(void){
	uint32_t interrupt_flags = I2C_IntGet(I2C0) & I2C_IntGetEnabled(I2C0);
	I2C_IntClear(I2C0, interrupt_flags);
	if(interrupt_flags & I2C_IEN_ACK){
		i2c_ack();
	}
	if(interrupt_flags & I2C_IEN_NACK){
		i2c_nack();
	}
	if(interrupt_flags & I2C_IEN_RXDATAV){
		i2c_rxdatav();
	}
	if(interrupt_flags & I2C_IEN_MSTOP){
		i2c_mstop();
	}
}

/***************************************************************************//**
 * @brief
 *	Function to start an I2C read or write operation
 *
 * @details
 *	Initializes the I2C payload which stores the state of the I2C operation.
 *	All information required by the I2C state machine interacts with this
 *	I2C payload struct.
 *
 *	Once the payload is initialized, this function initiates the I2C operation
 *	by entering the first state of the state machine
 *
 *	@note
 *	This function must only be called if the state of the I2C peripheral and
 *	the state of the I2C state machine are both in the idle state.
 *
 *
 * @param[in] i2c
 *  Pointer to the base peripheral address of the I2C peripheral being used. The
 * 	Pearl Gecko has 2 I2C peripherals.
 *
 * 	@param[in] device_address
 * 	 The 7 bit i2c slave device address of the peripheral device.
 *
 * 	@param[in] read
 * 	 Boolean parameter indicating whether to start a read or write command
 *
 * 	@param[in] command_code
 * 	 The 8 bit command code to write to the peripheral device.
 *
 * 	@param[in] data_arr
 * 	 A pointer to an array to store data from read commands.
 *I2C_START_STRUCT
 * 	@param[in] data_arr_length
 * 	 the length of the array to store data (number of bytes).
 *
 * 	@param[in] event
 * 	 The scheduler event associated with a completed i2c operation.
 *
 ******************************************************************************/

void i2c_start(I2C_START_STRUCT* start_struct)
{

	__disable_irq();

	EFM_ASSERT(i2c_idle(start_struct->i2c));
	EFM_ASSERT(start_struct->command_code_length > 0);
	if(start_struct->read == I2C_READ)	EFM_ASSERT(start_struct->read_length > 0);
	else	EFM_ASSERT(start_struct->write_length > 0);

	sleep_block_mode(I2C_EM_BLOCK);
	i2c_payload.state = I2C_REQUEST_DEVICE;
	i2c_payload.index = 0;

	memcpy(&i2c_payload.start, start_struct, sizeof(I2C_START_STRUCT));

	// Start bit, Device address, write bit.
	i2c_payload.start.i2c->CMD = I2C_CMD_START;
	i2c_payload.start.i2c->TXDATA = I2C_ADDR_W(i2c_payload.start.device_address);

	__enable_irq();

}

/***************************************************************************//**
 * @brief
 *	Function that the I2C interrupt handler will call upon receiving
 *	the I2C ACK interrupt
 *
 * @details
 *	This function defines the behavior of the state machine in each state
 *	when an ACK is received.
 *
 *
 ******************************************************************************/
static void i2c_ack(){
	switch(i2c_payload.state){
		case I2C_IDLE:
			EFM_ASSERT(false);
			break;
		case I2C_REQUEST_DEVICE:
			i2c_payload.state = I2C_WRITE_DATA;
			// send measurement command
			i2c_payload.start.i2c->TXDATA = *(i2c_payload.start.command_code + i2c_payload.index);
			i2c_payload.start.command_code_length -= 1;
			i2c_payload.index += 1;
			if(i2c_payload.start.command_code_length == 0)		i2c_payload.index = 0;
			break;
		case I2C_WRITE_DATA:
			if(i2c_payload.start.read == true)
			{
				i2c_payload.state = I2C_REQUEST_DATA;
				i2c_payload.index = 0;
				i2c_payload.start.i2c->CMD = I2C_CMD_START;
				i2c_payload.start.i2c->TXDATA = I2C_ADDR_R(i2c_payload.start.device_address);
			}
			else
			{
				if(i2c_payload.start.command_code_length > 0)
				{
					i2c_payload.start.i2c->TXDATA = *(i2c_payload.start.command_code + i2c_payload.index);
					i2c_payload.start.command_code_length -= 1;
					i2c_payload.index += 1;
					if(i2c_payload.start.command_code_length == 0)		i2c_payload.index = 0;
				}
				else if(i2c_payload.start.write_length > 0)
				{
					i2c_payload.start.i2c->TXDATA = *(i2c_payload.start.write_arr + i2c_payload.index);
					i2c_payload.start.write_length -= 1;
					i2c_payload.index += 1;
				}
				else if(i2c_payload.start.write_length == 0)
				{
					i2c_payload.start.i2c->CMD = I2C_CMD_STOP;
					i2c_payload.state = I2C_CLOSE_FUNCTION;
				}
				else	EFM_ASSERT(false);
			}
			break;
		case I2C_REQUEST_DATA:
			i2c_payload.state = I2C_READ_DATA;
			break;
		case I2C_READ_DATA:
			EFM_ASSERT(false);
			break;
		case I2C_CLOSE_FUNCTION:
			EFM_ASSERT(false);
			break;
		default:
			EFM_ASSERT(false);
			break;
	}
}

/***************************************************************************//**
 * @brief
 *	Function that the I2C interrupt handler will call upon receiving
 *	the I2C NACK interrupt
 *
 * @details
 *	This function defines the behavior of the state machine in each state
 *	when a NACK is received.
 *
 *
 ******************************************************************************/
static void i2c_nack(){
	switch(i2c_payload.state){
		case I2C_IDLE:
			EFM_ASSERT(false);
			break;
		case I2C_REQUEST_DEVICE:
			EFM_ASSERT(false);
			break;
		case I2C_WRITE_DATA:
			EFM_ASSERT(false);
			break;
		case I2C_REQUEST_DATA:
			// request data again
			if(i2c_payload.start.read == true)
			{
				i2c_payload.start.i2c->CMD = I2C_CMD_START;
				i2c_payload.start.i2c->TXDATA = I2C_ADDR_R(i2c_payload.start.device_address);
			}
			else	EFM_ASSERT(false);
			break;
		case I2C_READ_DATA:
			EFM_ASSERT(false);
			break;
		case I2C_CLOSE_FUNCTION:
			EFM_ASSERT(false);
			break;
		default:
			EFM_ASSERT(false);
			break;
	}
}
/***************************************************************************//**
 * @brief
 *	Function that the I2C interrupt handler will call upon receiving
 *	the I2C RXDATAV interrupt
 *
 * @details
 *	This function defines the behavior of the state machine in each state
 *	when data becomes available in the receive buffer.
 *
 *
 ******************************************************************************/
static void i2c_rxdatav(){
	switch(i2c_payload.state){
		case I2C_IDLE:
			EFM_ASSERT(false);
			break;
		case I2C_REQUEST_DEVICE:
			EFM_ASSERT(false);
			break;
		case I2C_WRITE_DATA:
			EFM_ASSERT(false);
			break;
		case I2C_REQUEST_DATA:
			EFM_ASSERT(false);
			break;
		case I2C_READ_DATA:
			// read byte
			if(i2c_payload.start.read_length > 0)
			{
				*(i2c_payload.start.read_arr + i2c_payload.index) = i2c_payload.start.i2c->RXDATA;
				i2c_payload.start.i2c->CMD = I2C_CMD_ACK;
				i2c_payload.start.read_length -= 1;
				i2c_payload.index += 1;
			}
			else
			{
				*(i2c_payload.start.read_arr + i2c_payload.index) = i2c_payload.start.i2c->RXDATA;
				i2c_payload.start.i2c->CMD = I2C_CMD_NACK;
				i2c_payload.start.i2c->CMD = I2C_CMD_STOP;
				i2c_payload.state = I2C_CLOSE_FUNCTION;
			}
			break;
		case I2C_CLOSE_FUNCTION:
			EFM_ASSERT(false);
			break;
		default:
			EFM_ASSERT(false);
			break;
	}
}

/***************************************************************************//**
 * @brief
 *	Function that the I2C interrupt handler will call upon receiving
 *	the I2C MSTOP interrupt
 *
 * @details
 *	This function defines the behavior of the state machine in each state
 *	when a STOP condition has been successfully transmitted.
 *
 *
 ******************************************************************************/
static void i2c_mstop(){
	switch(i2c_payload.state){
		case I2C_IDLE:
			EFM_ASSERT(false);
			break;
		case I2C_REQUEST_DEVICE:
			EFM_ASSERT(false);
			break;
		case I2C_WRITE_DATA:
			EFM_ASSERT(false);
			break;
		case I2C_REQUEST_DATA:
			EFM_ASSERT(false);
			break;
		case I2C_READ_DATA:
			EFM_ASSERT(false);
			break;
		case I2C_CLOSE_FUNCTION:
			i2c_payload.state = I2C_IDLE;
			sleep_unblock_mode(I2C_EM_BLOCK); // allow sleep
			add_scheduled_event(i2c_payload.start.event); // schedule event
			i2c_payload.index = 0;
			break;
		default:
			EFM_ASSERT(false);
			break;
	}
}

/***************************************************************************//**
 * @brief
 *   I2C Idle indicates whether the I2C state machine is in the IDLE state
 *
 * @return
 * 	 Returns TRUE if the state machine is IDLE and FALSE if the state machine is
 * 	 busy (ie. any state other than IDLE)
 *
 ******************************************************************************/

bool i2c_idle(I2C_TypeDef *i2c){
	if((i2c->STATE & _I2C_STATE_STATE_MASK) == I2C_STATE_STATE_IDLE)	return true;
	return false;
}

