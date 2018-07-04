/*
 * I2CSlaveCommunication.c
 * @ for I2C Slave Communication API
 * Author: WuXiuHua
 */
 
 #include "I2CSlaveCommunication.h"
 #include "stm32f0xx_hal_i2c.h"
 #include "i2c.h"
 
 
 #define I2C_SLAVE_ADDR	0x07
 #define I2C_SLAVE_BUFFER_SIZE	64
 
 
 uint8_t gI2CWriteBuffer[I2C_SLAVE_BUFFER_SIZE];
 uint8_t gI2CReadBuffer[I2C_SLAVE_BUFFER_SIZE];
 uint8_t gI2CWrittenDoneFlag;
 uint8_t gI2CReadDoneFlag;
 uint8_t gI2CErrorFlag;
 
 
void I2C_ITInit(void)
 {
//	 HAL_I2C_Mem_Write_DMA(&hi2c1, I2C_SLAVE_ADDR<<1, 0x00, I2C_MEMADD_SIZE_8BIT, gI2CWriteBuffer, 1);
//	 HAL_I2C_Mem_Read_DMA(&hi2c1, I2C_SLAVE_ADDR<<1, 0x00, I2C_MEMADD_SIZE_8BIT, gI2CReadBuffer, 1);
	 HAL_I2C_Mem_Write_IT(&hi2c1, I2C_SLAVE_ADDR<<1, 0x00, I2C_MEMADD_SIZE_8BIT, gI2CWriteBuffer, 1);	
	 HAL_I2C_Mem_Read_IT(&hi2c1, I2C_SLAVE_ADDR<<1, 0x00, I2C_MEMADD_SIZE_8BIT, gI2CReadBuffer, 1);	
 }
 
 
 
void HAL_I2C_MemTxCpltCallback(I2C_HandleTypeDef *hi2c)
{
	if(hi2c->Instance == hi2c1.Instance)
	{
		gI2CWrittenDoneFlag = 1;
	}
}

void HAL_I2C_MemRxCpltCallback(I2C_HandleTypeDef *hi2c)
{
	if(hi2c->Instance == hi2c1.Instance)
	{
		gI2CReadDoneFlag = 1;
	}
}

void HAL_I2C_ErrorCallback(I2C_HandleTypeDef *hi2c)
{
	if(hi2c->Instance == hi2c1.Instance)
	{
		gI2CErrorFlag = 1;
	}
}

