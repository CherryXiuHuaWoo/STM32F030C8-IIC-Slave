#ifndef I2CSlaveCommunication_H
#define I2CSlaveCommunication_H

#include "stm32f0xx_hal.h"


 #define I2C_SLAVE_ADDR			0x07
 #define I2C_BUFFER_SIZE		64
 #define I2C_READ_BYTES			8
 #define I2C_WRITE_BYTES		8
 #define I2C_DUMMYWRITE_Bytes	3
 extern uint8_t gI2CWrittenDoneFlag;
 extern uint8_t gI2CReadDoneFlag;
 extern uint8_t gI2CErrorFlag;
 
uint8_t RegisterControl(uint16_t RegisterAddress, uint8_t OperaBytesLength, uint8_t Operation);


#endif /*I2CSlaveCommunication_H*/

