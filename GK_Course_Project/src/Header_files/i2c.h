#ifndef SRC_HEADER_FILES_I2C_H_
#define SRC_HEADER_FILES_I2C_H_

//***********************************************************************************
// Include files
//***********************************************************************************


//***********************************************************************************
// defined files
//***********************************************************************************
#define RESET_TOGGLE_NUMBER 		18
#define I2C_EM_BLOCK 				EM2 // cannot enter EM2
#define I2C_WRITE					0
#define I2C_READ					1
#define I2C_ONE_BYTE_CC				1
#define I2C_TWO_BYTE_CC				2
#define I2C_WRITE_LIMIT				20
//***********************************************************************************
// global variables
//***********************************************************************************
typedef enum {
	I2C_IDLE,
	I2C_REQUEST_DEVICE,
	I2C_WRITE_DATA,
	I2C_REQUEST_DATA,
	I2C_READ_DATA,
	I2C_CLOSE_FUNCTION
} State;

typedef struct {

	// I2C_Init_TypeDef Struct Values
	bool 					enable;				// enable I2C upon completion of open
	bool					master;
	uint32_t				refFreq;
	uint32_t				freq;
	I2C_ClockHLR_TypeDef 	chlr;

	// I2C Route Register Values
	uint8_t			sda_route0;		// sda route to gpio pin
	uint8_t			scl_route0;		// scl route to gpio pin
	bool			sda_en;		// enable out 0 route
	bool			scl_en;		// enable out 1 route

} I2C_OPEN_STRUCT ;

typedef struct {
	uint8_t			sda_port;
	uint8_t			sda_pin;
	uint8_t			scl_port;
	uint8_t			scl_pin;
} I2C_IO_STRUCT ;

typedef struct {
	State			state; // what is current state
	I2C_TypeDef* 	i2c; // which i2c bus are we using
	uint8_t 		device_address;
	bool			read; // read = 1 write = 0
	uint8_t* 		write_arr; // read: command code(s). Write: command code(s) + data
	uint8_t			write_length; // read: command code length. Write: cc length + data length
	uint8_t			num_bytes_written; // how many bytes have been written (iterator). Initialize to 0.
	uint8_t* 		read_arr; // read: where to save data. Write: null pointer
	uint8_t			read_length; // read: number of bytes expected. Write: 0.
	uint8_t			num_bytes_read; // how many bytes have been read (iterator). Initialize to 0
	uint32_t		event; // for scheduler. 0 = no event.
} I2C_PAYLOAD_STRUCT ;

typedef struct {
	uint8_t 		device_address;
	bool			read;
	uint8_t*		command_code; // command code
	uint8_t			command_code_length;
	uint8_t*		write_arr; // pure data
	uint8_t			write_length;
	uint8_t*		read_arr; // where to put the data
	uint8_t			read_length;
	uint32_t		event;
} I2C_START_STRUCT;

//***********************************************************************************
// function prototypes
//***********************************************************************************

void i2c_open(I2C_TypeDef *i2c, I2C_OPEN_STRUCT *i2c_open, I2C_IO_STRUCT *i2c_io);
void i2c_bus_reset(I2C_TypeDef *i2c, I2C_IO_STRUCT *i2c_io);
void I2C0_IRQHandler(void);
void i2c_start(I2C_TypeDef *i2c, I2C_START_STRUCT* start_struct);

bool i2c_idle(void);

#endif /* SRC_HEADER_FILES_I2C_H_ */
