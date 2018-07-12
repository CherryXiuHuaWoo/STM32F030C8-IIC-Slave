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
 uint8_t gI2CDummyWriterBuffer[I2C_DUMMYWRITE_Bytes];
 uint8_t gI2CWrittenDoneFlag;
 uint8_t gI2CReadDoneFlag;
 uint8_t gI2CErrorFlag;
 uint8_t gI2CDummyWrittenDoneFlag = 0;
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

static HAL_StatusTypeDef I2C_WaitOnFlagUntilTimeout(I2C_HandleTypeDef *hi2c, uint32_t Flag, FlagStatus Status, uint32_t Timeout, uint32_t Tickstart)
{
  while (__HAL_I2C_GET_FLAG(hi2c, Flag) == Status)
  {
    /* Check for the Timeout */
    if (Timeout != HAL_MAX_DELAY)
    {
      if ((Timeout == 0U) || ((HAL_GetTick() - Tickstart) > Timeout))
      {
        hi2c->State = HAL_I2C_STATE_READY;
        hi2c->Mode = HAL_I2C_MODE_NONE;

        /* Process Unlocked */
        __HAL_UNLOCK(hi2c);
        return HAL_TIMEOUT;
      }
    }
  }
  return HAL_OK;
}


static void I2C_Flush_TXDR(I2C_HandleTypeDef *hi2c)
{
  /* If a pending TXIS flag is set */
  /* Write a dummy data in TXDR to clear it */
  if (__HAL_I2C_GET_FLAG(hi2c, I2C_FLAG_TXIS) != RESET)
  {
    hi2c->Instance->TXDR = 0x00U;
  }

  /* Flush TX register if not empty */
  if (__HAL_I2C_GET_FLAG(hi2c, I2C_FLAG_TXE) == RESET)
  {
    __HAL_I2C_CLEAR_FLAG(hi2c, I2C_FLAG_TXE);
  }
}


static HAL_StatusTypeDef I2C_IsAcknowledgeFailed(I2C_HandleTypeDef *hi2c, uint32_t Timeout, uint32_t Tickstart)
{
  if (__HAL_I2C_GET_FLAG(hi2c, I2C_FLAG_AF) == SET)
  {
    /* Wait until STOP Flag is reset */
    /* AutoEnd should be initiate after AF */
    while (__HAL_I2C_GET_FLAG(hi2c, I2C_FLAG_STOPF) == RESET)
    {
      /* Check for the Timeout */
      if (Timeout != HAL_MAX_DELAY)
      {
        if ((Timeout == 0U) || ((HAL_GetTick() - Tickstart) > Timeout))
        {
          hi2c->State = HAL_I2C_STATE_READY;
          hi2c->Mode = HAL_I2C_MODE_NONE;

          /* Process Unlocked */
          __HAL_UNLOCK(hi2c);
          return HAL_TIMEOUT;
        }
      }
    }

    /* Clear NACKF Flag */
    __HAL_I2C_CLEAR_FLAG(hi2c, I2C_FLAG_AF);

    /* Clear STOP Flag */
    __HAL_I2C_CLEAR_FLAG(hi2c, I2C_FLAG_STOPF);

    /* Flush TX register */
    I2C_Flush_TXDR(hi2c);

    /* Clear Configuration Register 2 */
    I2C_RESET_CR2(hi2c);

    hi2c->ErrorCode = HAL_I2C_ERROR_AF;
    hi2c->State = HAL_I2C_STATE_READY;
    hi2c->Mode = HAL_I2C_MODE_NONE;

    /* Process Unlocked */
    __HAL_UNLOCK(hi2c);

    return HAL_ERROR;
  }
  return HAL_OK;
}


static HAL_StatusTypeDef I2C_WaitOnRXNEFlagUntilTimeout(I2C_HandleTypeDef *hi2c, uint32_t Timeout, uint32_t Tickstart)
{
  while (__HAL_I2C_GET_FLAG(hi2c, I2C_FLAG_RXNE) == RESET)
  {
    /* Check if a NACK is detected */
    if (I2C_IsAcknowledgeFailed(hi2c, Timeout, Tickstart) != HAL_OK)
    {
      return HAL_ERROR;
    }

    /* Check if a STOPF is detected */
    if (__HAL_I2C_GET_FLAG(hi2c, I2C_FLAG_STOPF) == SET)
    {
      /* Clear STOP Flag */
      __HAL_I2C_CLEAR_FLAG(hi2c, I2C_FLAG_STOPF);

      /* Clear Configuration Register 2 */
      I2C_RESET_CR2(hi2c);

      hi2c->ErrorCode = HAL_I2C_ERROR_NONE;
      hi2c->State = HAL_I2C_STATE_READY;
      hi2c->Mode = HAL_I2C_MODE_NONE;

      /* Process Unlocked */
      __HAL_UNLOCK(hi2c);

      return HAL_ERROR;
    }

    /* Check for the Timeout */
    if ((Timeout == 0U) || ((HAL_GetTick() - Tickstart) > Timeout))
    {
      hi2c->ErrorCode |= HAL_I2C_ERROR_TIMEOUT;
      hi2c->State = HAL_I2C_STATE_READY;

      /* Process Unlocked */
      __HAL_UNLOCK(hi2c);

      return HAL_TIMEOUT;
    }
  }
  return HAL_OK;
}



HAL_StatusTypeDef I2C_DummyWrite(I2C_HandleTypeDef *hi2c, uint8_t *pData, uint16_t Size, uint32_t Timeout)
{
  uint32_t tickstart = 0U;

  if (hi2c->State == HAL_I2C_STATE_READY)
  {
    if ((pData == NULL) || (Size == 0U))
    {
      return  HAL_ERROR;
    }
    /* Process Locked */
    __HAL_LOCK(hi2c);

    /* Init tickstart for timeout management*/
    tickstart = HAL_GetTick();

    hi2c->State     = HAL_I2C_STATE_BUSY_RX;
    hi2c->Mode      = HAL_I2C_MODE_SLAVE;
    hi2c->ErrorCode = HAL_I2C_ERROR_NONE;

    /* Prepare transfer parameters */
    hi2c->pBuffPtr  = pData;
    hi2c->XferCount = Size;
    hi2c->XferISR   = NULL;

    /* Enable Address Acknowledge */
    hi2c->Instance->CR2 &= ~I2C_CR2_NACK;

    /* Wait until ADDR flag is set */
    if (I2C_WaitOnFlagUntilTimeout(hi2c, I2C_FLAG_ADDR, RESET, Timeout, tickstart) != HAL_OK)
    {
      /* Disable Address Acknowledge */
      hi2c->Instance->CR2 |= I2C_CR2_NACK;
      return HAL_TIMEOUT;
    }

    /* Clear ADDR flag */
    __HAL_I2C_CLEAR_FLAG(hi2c, I2C_FLAG_ADDR);

    /* Wait until DIR flag is reset Receiver mode */
    if (I2C_WaitOnFlagUntilTimeout(hi2c, I2C_FLAG_DIR, SET, Timeout, tickstart) != HAL_OK)
    {
      /* Disable Address Acknowledge */
      hi2c->Instance->CR2 |= I2C_CR2_NACK;
      return HAL_TIMEOUT;
    }

    while (hi2c->XferCount > 0U)
    {
      /* Wait until RXNE flag is set */
      if (I2C_WaitOnRXNEFlagUntilTimeout(hi2c, Timeout, tickstart) != HAL_OK)
      {
        /* Disable Address Acknowledge */
        hi2c->Instance->CR2 |= I2C_CR2_NACK;

        /* Store Last receive data if any */
        if (__HAL_I2C_GET_FLAG(hi2c, I2C_FLAG_RXNE) == SET)
        {
          /* Read data from RXDR */
          (*hi2c->pBuffPtr++) = hi2c->Instance->RXDR;
          hi2c->XferCount--;
        }

        if (hi2c->ErrorCode == HAL_I2C_ERROR_TIMEOUT)
        {
          return HAL_TIMEOUT;
        }
        else
        {
          return HAL_ERROR;
        }
      }

      /* Read data from RXDR */
      (*hi2c->pBuffPtr++) = hi2c->Instance->RXDR;
      hi2c->XferCount--;
    }

    /* Disable Address Acknowledge */
    hi2c->Instance->CR2 |= I2C_CR2_NACK;

    hi2c->State = HAL_I2C_STATE_READY;
    hi2c->Mode  = HAL_I2C_MODE_NONE;

    /* Process Unlocked */
    __HAL_UNLOCK(hi2c);

    return HAL_OK;
	}
    else
	{
		return HAL_BUSY;
	}
}
  

void HAL_I2C_AddrCallback(I2C_HandleTypeDef *hi2c, uint8_t TransferDirection, uint16_t AddrMatchCode)
{
	int status = 0;
	if(hi2c->Instance == hi2c1.Instance)
	{
		hi2c->State = HAL_I2C_STATE_READY;
		if(TransferDirection == 0x00)	// write
		{
			if(0 == gI2CDummyWrittenDoneFlag)
			{
				if(I2C_DummyWrite(&hi2c1, gI2CDummyWriterBuffer, I2C_DUMMYWRITE_Bytes, 1) == HAL_OK)
				{
					gI2CDummyWrittenDoneFlag = 1;
					I2CSlaveInit();
					return;
				}
			}
			else
			{
				status = HAL_I2C_Slave_Receive(&hi2c1, gI2CReadBuffer, gI2CDummyWriterBuffer[2], 1);
				if(status == HAL_OK)
				{
					HAL_I2C_SlaveRxCpltCallback(&hi2c1);
					
				}
				else
				{
					printf("Err: I2C Write.Status =%d\r\n", status);
				}
			}
		}
		else	//Read
		{
			if(1 == gI2CDummyWrittenDoneFlag)
			{
				status = HAL_I2C_Slave_Transmit(&hi2c1, gI2CWriteBuffer, gI2CDummyWriterBuffer[2], 1);
				
				if(status == HAL_OK)
				{
					HAL_I2C_SlaveTxCpltCallback(&hi2c1);
				}
				else
				{
					printf("Err: I2C Read. Status =%d\r\n", status);
				}
			}
		}
		gI2CDummyWrittenDoneFlag = 0;
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



