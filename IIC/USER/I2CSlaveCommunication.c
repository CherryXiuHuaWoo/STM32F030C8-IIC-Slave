/*
 * I2CSlaveCommunication.c
 * @ for I2C Slave Communication API
 * Author: WuXiuHua
 * Date£º2018-7-6
 */
 
 #include "I2CSlaveCommunication.h"
 #include "stm32f0xx_hal_i2c.h"
 #include "i2c.h"
 
 
 uint8_t gI2CWriteBuffer[I2C_BUFFER_SIZE] = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08};
 uint8_t gI2CReadBuffer[I2C_BUFFER_SIZE];
 uint8_t gI2CWrittenDoneFlag;
 uint8_t gI2CReadDoneFlag;
 uint8_t gI2CErrorFlag;
 
 uint8_t gUsartUpdateBuffer[I2C_BUFFER_SIZE];

 
void I2CSlaveInit(void)
{
	if(HAL_I2C_EnableListen_IT(&hi2c1) != HAL_OK)
	{
		printf("HAL_I2C_EnableListen_IT Error\r\n");
	}
}


void I2C_StatusProc(HAL_StatusTypeDef status)
{
	switch(status)
	{
		case HAL_OK :
			printf("I2C Done\r\n");
			break;
		case HAL_ERROR :
			printf("I2C Error\r\n");
			break;
		case HAL_BUSY :
			printf("I2C Busy\r\n");
			break;
		case HAL_TIMEOUT :
			printf("I2C Timeout\r\n");
			break;
		default:
			printf("I2C Unknow\r\n");
			break;
	}
}

void I2C_WriteData(void)
 {
	 HAL_StatusTypeDef writeStatus;
	 
	 writeStatus = HAL_I2C_Mem_Write_IT(&hi2c1, I2C_SLAVE_ADDR<<1, 0x00, I2C_MEMADD_SIZE_8BIT, gI2CWriteBuffer, 8);	
	 I2C_StatusProc(writeStatus);
 }
 
  
void I2C_ReadData(void)
 {
	 HAL_StatusTypeDef readStatus;
	 
	 readStatus = HAL_I2C_Mem_Read_IT(&hi2c1, I2C_SLAVE_ADDR<<1, 0x00, I2C_MEMADD_SIZE_8BIT, gI2CReadBuffer, I2C_READ_BYTES);
	 I2C_StatusProc(readStatus);	 
 }
 
 
void I2C_UsartUpdateReadData(uint32_t BufLen)
{
	uint32_t bufIdx;
	
	for(bufIdx = 0; bufIdx < BufLen; bufIdx++)
	{
		printf("0x%2x ", gUsartUpdateBuffer[bufIdx]);
	}
	
	printf("\r\n");
}


void I2C_SaveReadDataToUsartUpdataBuffer(uint32_t BufLen)
{
	uint32_t bufIdx;
	
	for(bufIdx = 0; bufIdx < BufLen; bufIdx++)
	{
		gUsartUpdateBuffer[bufIdx] = gI2CReadBuffer[bufIdx];
	}	 
}


void HAL_I2C_SlaveTxCpltCallback(I2C_HandleTypeDef *hi2c)
{
	if(hi2c->Instance == hi2c1.Instance)
	{
		gI2CWrittenDoneFlag = 1;
	}
}


void HAL_I2C_SlaveRxCpltCallback(I2C_HandleTypeDef *hi2c)
{
	if(hi2c->Instance == hi2c1.Instance)
	{
		I2C_SaveReadDataToUsartUpdataBuffer(I2C_READ_BYTES);
		gI2CReadDoneFlag = 1;
	}
}


void HAL_I2C_AddrCallback(I2C_HandleTypeDef *hi2c, uint8_t TransferDirection, uint16_t AddrMatchCode)
{
	if(hi2c->Instance == hi2c1.Instance)
	{
		if(0x07 == (AddrMatchCode >> 1))	// Addr
		{
			hi2c->State = HAL_I2C_STATE_READY;
			if(TransferDirection == 0x01)	// Read
			{
				if(HAL_I2C_Slave_Transmit(&hi2c1, gI2CWriteBuffer, I2C_WRITE_BYTES, 1) == HAL_OK)
				{
					HAL_I2C_SlaveTxCpltCallback(&hi2c1);
				}
			}
			else	//Write
			{
				if(HAL_I2C_Slave_Receive(&hi2c1, gI2CReadBuffer, I2C_READ_BYTES, 1) == HAL_OK)
				{
					HAL_I2C_SlaveRxCpltCallback(&hi2c1);
				}
			}
		}
		__HAL_I2C_GENERATE_NACK(&hi2c1);
	}
	I2CSlaveInit();
}


void HAL_I2C_ErrorCallback(I2C_HandleTypeDef *hi2c)
{
	if(hi2c->Instance == hi2c1.Instance)
	{
		gI2CErrorFlag = 1;
		__HAL_I2C_GENERATE_NACK(&hi2c1);
		I2CSlaveInit();
	}
}

