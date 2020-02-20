#ifndef SRC_HEADER_FILES_I2C_H_
#define SRC_HEADER_FILES_I2C_H_

//***********************************************************************************
// Include files
//***********************************************************************************


//***********************************************************************************
// defined files
//***********************************************************************************


//***********************************************************************************
// global variables
//***********************************************************************************
typedef struct {
	//	I2C_TypeDef* i2c;
	//	const I2C_Init_TypeDef* i2c_init;

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

} I2C_PAYLOAD ;

//***********************************************************************************
// function prototypes
//***********************************************************************************

void i2c_open(I2C_TypeDef *i2c, I2C_OPEN_STRUCT *i2c_open);
void i2c_bus_reset();


#endif /* SRC_HEADER_FILES_I2C_H_ */
