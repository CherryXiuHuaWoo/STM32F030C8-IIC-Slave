#ifndef I2CSlaveCommunication_H
#define I2CSlaveCommunication_H

#include "stm32f0xx_hal.h"


 #define I2C_SLAVE_ADDR			0x07
 #define I2C_BUFFER_SIZE		64
 #define I2C_READ_BYTES			8
 
 extern uint8_t gI2CWrittenDoneFlag;
 extern uint8_t gI2CReadDoneFlag;
 extern uint8_t gI2CErrorFlag;

void I2C_WriteData(void);
void I2C_ReadData(void);
void I2C_UsartUpdateReadData(uint32_t BufLen);

#endif /*I2CSlaveCommunication_H*/

