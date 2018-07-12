/*
 * RegisterMaps.c
 * @ for RegisterMaps API
 * Author: WuXiuHua
 * Date£º2018-7-6
 */
 
 #include "RegisterMaps.h"
 #include "i2c.h"
 
 #define ConfigurationRegisterBytes			0x58
 #define TriggerRegisterBytes				0x28
 #define ListRegisterBytes					(1490+3054+3054)
 #define ConfigurationRegisterStartAddrs	0x00
 #define TriggerRegisterStartAddr			0x100
 #define ListRegisterStartAddr				0x0400
 
 uint8_t gConfigurationRegister[ConfigurationRegisterBytes];
 uint8_t gTriggerRegister[TriggerRegisterBytes];
 uint8_t gI2CWriteBuffer[I2C_BUFFER_SIZE] = {0};
 uint8_t gI2CReadBuffer[I2C_BUFFER_SIZE] = {0};

 
uint8_t RegisterControl(uint16_t RegisterAddress, uint8_t OperaBytesLength, uint8_t Operation)
 {
 
	HAL_StatusTypeDef status = HAL_ERROR;
	uint8_t *registerMaps;
	uint8_t idx;
	
	if(RegisterAddress < TriggerRegisterStartAddr)
 	{
 		registerMaps = &gConfigurationRegister[RegisterAddress];
	}
	else if((RegisterAddress >= TriggerRegisterStartAddr) && (RegisterAddress < (TriggerRegisterStartAddr + TriggerRegisterBytes)))
	{
		registerMaps = &gTriggerRegister[RegisterAddress];
	}
	else
	{
		return status;
	}
	
	if(Operation == I2C_WRITE_OPERATION)
	{
		status = HAL_I2C_Slave_Receive(&hi2c1, registerMaps, OperaBytesLength, 1);
	}
	else
	{
		status = HAL_I2C_Slave_Transmit(&hi2c1, registerMaps, OperaBytesLength, 1);
	}


	if(status == HAL_OK)
	{
		printf("Address=0x%4x\r\n", RegisterAddress);
		for(idx = 0; idx < OperaBytesLength; idx++)
		{
			printf("0x%x\r\n", *(registerMaps + idx));
		}
	}
	return status;
 }




