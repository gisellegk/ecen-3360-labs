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
// functions
//***********************************************************************************
static void i2c_ack();
static void i2c_nack();
static void i2c_rxdatav();
static void i2c_mstop();

/***************************************************************************//**
 * @brief
 *
 *
 * @details
 *
 *
 * @note
 *
 *
 * @param[in] i2c
 *
 *
 ******************************************************************************/
void i2c_open(I2C_TypeDef *i2c, I2C_OPEN_STRUCT *i2c_open, I2C_IO_STRUCT *i2c_io){
	I2C_Init_TypeDef init;
	i2c_payload.state = IDLE; // start in idle mode

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
						| (1 << _I2C_IEN_RXDATAV_SHIFT);

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

}

/***************************************************************************//**
 * @brief
 *
 *
 * @details
 *
 *
 * @note
 *
 *
 * @param[in] i2c
 *
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
 *
 *
 * @details
 *
 *
 * @note
 *
 *
 * @param[in] i2c
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
 *
 *
 * @details
 *
 *
 * @note
 *
 *
 * @param[in] i2c
 *
 *
 ******************************************************************************/

void i2c_start(I2C_TypeDef *i2c, uint8_t device_address, bool read, uint8_t command_code, uint8_t* data_arr, uint8_t data_arr_length, uint32_t event){
	EFM_ASSERT((i2c->STATE & _I2C_STATE_STATE_MASK) == I2C_STATE_STATE_IDLE); // this assert will trigger if your i2c peripheral hasn't completed its previous operation
	EFM_ASSERT(i2c_payload.state == IDLE); // state machine should only be started in idle mode.

	sleep_block_mode(I2C_EM_BLOCK);
	i2c_payload.i2c = i2c;
	i2c_payload.device_address = device_address;
	i2c_payload.read = read;
	i2c_payload.command_code = command_code;
	i2c_payload.data_arr = data_arr;
	i2c_payload.data_arr_length = data_arr_length;
	i2c_payload.num_data_saved = 0;

	i2c_payload.state = REQUEST_TEMP_SENSOR;

	// Start bit, Device address, read bit.
	i2c_payload.i2c->CMD = I2C_CMD_START;
	i2c_payload.i2c->TXDATA = (i2c_payload.device_address << 1) | I2C_WRITE;

}

/***************************************************************************//**
 * @brief
 *
 *
 * @details
 *
 *
 * @note
 *
 *
 * @param[in] i2c
 *
 *
 ******************************************************************************/
static void i2c_ack(){
	switch(i2c_payload.state){
		case IDLE:
			EFM_ASSERT(false);
			break;
		case REQUEST_TEMP_SENSOR:
			i2c_payload.state = REQUEST_MEASUREMENT;
			// send measurement command
			i2c_payload.i2c->TXDATA = i2c_payload.command_code;
			break;
		case REQUEST_MEASUREMENT:
			if(i2c_payload.read){
				i2c_payload.state = WAIT_FOR_CONVERSION;
				i2c_payload.i2c->CMD = I2C_CMD_START;
				i2c_payload.i2c->TXDATA = (i2c_payload.device_address << 1) | I2C_READ;
			}
			break;
		case WAIT_FOR_CONVERSION:
			i2c_payload.state = READ_DATA;
			break;
		case READ_DATA:
			EFM_ASSERT(false);
			break;
		case CLOSE_FUNCTION:
			EFM_ASSERT(false);
			break;
		default:
			EFM_ASSERT(false);
			break;
	}
}

/***************************************************************************//**
 * @brief
 *
 *
 * @details
 *
 *
 * @note
 *
 *
 * @param[in] i2c
 *
 *
 ******************************************************************************/
static void i2c_nack(){
	switch(i2c_payload.state){
		case IDLE:
			EFM_ASSERT(false);
			break;
		case REQUEST_TEMP_SENSOR:
			EFM_ASSERT(false);
			break;
		case REQUEST_MEASUREMENT:
			EFM_ASSERT(false);
			break;
		case WAIT_FOR_CONVERSION:
			// request data again
			if(i2c_payload.read){
				i2c_payload.i2c->CMD = I2C_CMD_START;
				i2c_payload.i2c->TXDATA = (i2c_payload.device_address << 1) | I2C_READ;
			}
			break;
		case READ_DATA:
			EFM_ASSERT(false);
			break;
		case CLOSE_FUNCTION:
			EFM_ASSERT(false);
			break;
		default:
			EFM_ASSERT(false);
			break;
	}
}
/***************************************************************************//**
 * @brief
 *
 *
 * @details
 *
 *
 * @note
 *
 *
 * @param[in] i2c
 *
 *
 ******************************************************************************/
static void i2c_rxdatav(){
	switch(i2c_payload.state){
		case IDLE:
			EFM_ASSERT(false);
			break;
		case REQUEST_TEMP_SENSOR:
			EFM_ASSERT(false);
			break;
		case REQUEST_MEASUREMENT:
			EFM_ASSERT(false);
			break;
		case WAIT_FOR_CONVERSION:
			EFM_ASSERT(false);
			break;
		case READ_DATA:
			// read byte
			i2c_payload.data_arr[i2c_payload.num_data_saved] = i2c_payload.i2c->RXDATA; //data;
			i2c_payload.num_data_saved++;
			if(i2c_payload.num_data_saved >= i2c_payload.data_arr_length){
				i2c_payload.state = CLOSE_FUNCTION;
				i2c_payload.i2c->CMD = I2C_CMD_NACK;
				i2c_payload.i2c->CMD = I2C_CMD_STOP;
			} else {
				// send ACK
				i2c_payload.i2c->CMD = I2C_CMD_ACK;
			}
			break;
		case CLOSE_FUNCTION:
			EFM_ASSERT(false);
			break;
		default:
			EFM_ASSERT(false);
			break;
	}
}

/***************************************************************************//**
 * @brief
 *
 *
 * @details
 *
 *
 * @note
 *
 *
 * @param[in] i2c
 *
 *
 ******************************************************************************/
static void i2c_mstop(){
	switch(i2c_payload.state){
		case IDLE:
			EFM_ASSERT(false);
			break;
		case REQUEST_TEMP_SENSOR:
			EFM_ASSERT(false);
			break;
		case REQUEST_MEASUREMENT:
			EFM_ASSERT(false);
			break;
		case WAIT_FOR_CONVERSION:
			EFM_ASSERT(false);
			break;
		case READ_DATA:
			EFM_ASSERT(false);
			break;
		case CLOSE_FUNCTION:
			i2c_payload.state = IDLE;
			sleep_block_mode(I2C_EM_BLOCK); // allow sleep
			add_scheduled_event(i2c_payload.event); // schedule event
			break;
		default:
			EFM_ASSERT(false);
			break;
	}
}

