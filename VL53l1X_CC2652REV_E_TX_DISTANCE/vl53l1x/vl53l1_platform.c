
/* 
* This file is part of VL53L1 Platform 
* 
* Copyright (c) 2016, STMicroelectronics - All Rights Reserved 
* 
* License terms: BSD 3-clause "New" or "Revised" License. 
* 
* Redistribution and use in source and binary forms, with or without 
* modification, are permitted provided that the following conditions are met: 
* 
* 1. Redistributions of source code must retain the above copyright notice, this 
* list of conditions and the following disclaimer. 
* 
* 2. Redistributions in binary form must reproduce the above copyright notice, 
* this list of conditions and the following disclaimer in the documentation 
* and/or other materials provided with the distribution. 
* 
* 3. Neither the name of the copyright holder nor the names of its contributors 
* may be used to endorse or promote products derived from this software 
* without specific prior written permission. 
* 
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" 
* AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE 
* IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE 
* DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE 
* FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL 
* DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR 
* SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER 
* CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, 
* OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE 
* OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE. 
* 
*/

#include "vl53l1_platform.h"
#include "vl53l1_platform_user_config.h"
#include <string.h>
#include <time.h>
#include <math.h>

uint8_t _I2CBuffer[256];

int _I2CWrite(I2C_Handle i2cHandle, uint16_t Dev, uint8_t *pdata, uint32_t count)
{
	int status;
	// Initialize slave address of transaction
    I2C_Transaction transaction = { 0 };
	transaction.slaveAddress = Dev;
	transaction.writeBuf = pdata;
    transaction.writeCount = count;
	transaction.readBuf = NULL;
	transaction.readCount = 0;
	status = I2C_transfer(i2cHandle, &transaction);
	return !status;
}

int _I2CRead(I2C_Handle i2cHandle, uint16_t Dev, uint8_t *pdata, uint32_t count)
{
	int status;
    // Initialize slave address of transaction
    I2C_Transaction transaction = { 0 };
    transaction.slaveAddress = Dev;
    transaction.writeBuf = NULL;
    transaction.writeCount = 0;
    // Read from I2C slave device
    transaction.readBuf = pdata;
    transaction.readCount = count;

    status = I2C_transfer(i2cHandle, &transaction);
    return !status;
}

int8_t VL53L1_WriteMulti(I2C_Handle i2cHandle, uint16_t dev, uint16_t index, uint8_t *pdata, uint32_t count)
{
	int status_int;
	VL53L1_Error Status = VL53L1_ERROR_NONE;
	if (count > sizeof(_I2CBuffer) - 1)
	{
		return VL53L1_ERROR_INVALID_PARAMS;
	}
	_I2CBuffer[0] = index >> 8;
	_I2CBuffer[1] = index & 0xFF;
	memcpy(&_I2CBuffer[2], pdata, count);
	status_int = _I2CWrite(i2cHandle, dev, _I2CBuffer, count + 2);
	if (status_int != 0)
	{
		Status = VL53L1_ERROR_CONTROL_INTERFACE;
	}
	return Status;
}

int8_t VL53L1_ReadMulti(I2C_Handle i2cHandle, uint16_t dev, uint16_t index, uint8_t *pdata, uint32_t count)
{
	VL53L1_Error Status = VL53L1_ERROR_NONE;
	int32_t status_int;

	_I2CBuffer[0] = index >> 8;
	_I2CBuffer[1] = index & 0xFF;
	status_int = _I2CWrite(i2cHandle, dev, _I2CBuffer, 2);
	if (status_int != 0)
	{
		Status = VL53L1_ERROR_CONTROL_INTERFACE;
		goto done;
	}
	status_int = _I2CRead(i2cHandle, dev, pdata, count);
	if (status_int != 0)
	{
		Status = VL53L1_ERROR_CONTROL_INTERFACE;
	}
done:
	return Status;
}

int8_t VL53L1_WrByte(I2C_Handle i2cHandle, uint16_t dev, uint16_t index, uint8_t data)
{
	VL53L1_Error Status = VL53L1_ERROR_NONE;
	int32_t status_int;

	_I2CBuffer[0] = index >> 8;
	_I2CBuffer[1] = index & 0xFF;
	_I2CBuffer[2] = data;
	status_int = _I2CWrite(i2cHandle, dev, _I2CBuffer, 3);
	if (status_int != 0)
	{
		Status = VL53L1_ERROR_CONTROL_INTERFACE;
	}
	return Status;
}

int8_t VL53L1_WrWord(I2C_Handle i2cHandle, uint16_t dev, uint16_t index, uint16_t data)
{
	VL53L1_Error Status = VL53L1_ERROR_NONE;
	int32_t status_int;

	_I2CBuffer[0] = index >> 8;
	_I2CBuffer[1] = index & 0xFF;
	_I2CBuffer[2] = data >> 8;
	_I2CBuffer[3] = data & 0x00FF;
	status_int = _I2CWrite(i2cHandle, dev, _I2CBuffer, 4);
	if (status_int != 0)
	{
		Status = VL53L1_ERROR_CONTROL_INTERFACE;
	}
	return Status;
}

int8_t VL53L1_WrDWord(I2C_Handle i2cHandle, uint16_t dev, uint16_t index, uint32_t data)
{
	VL53L1_Error Status = VL53L1_ERROR_NONE;
	int32_t status_int;
	_I2CBuffer[0] = index >> 8;
	_I2CBuffer[1] = index & 0xFF;
	_I2CBuffer[2] = (data >> 24) & 0xFF;
	_I2CBuffer[3] = (data >> 16) & 0xFF;
	_I2CBuffer[4] = (data >> 8) & 0xFF;
	_I2CBuffer[5] = (data >> 0) & 0xFF;
	status_int = _I2CWrite(i2cHandle, dev, _I2CBuffer, 6);
	if (status_int != 0)
	{
		Status = VL53L1_ERROR_CONTROL_INTERFACE;
	}
	return Status;
}

// VL53L1_Error VL53L1_UpdateByte(uint16_t Dev, uint16_t index, uint8_t AndData, uint8_t OrData)
// {
// 	VL53L1_Error Status = VL53L1_ERROR_NONE;
// 	uint8_t data;

// 	Status = VL53L1_RdByte(Dev, index, &data);
// 	if (Status)
// 	{
// 		goto done;
// 	}
// 	data = (data & AndData) | OrData;
// 	Status = VL53L1_WrByte(Dev, index, data);
// done:
// 	return Status;
// }

int8_t VL53L1_RdByte(I2C_Handle i2cHandle, uint16_t dev, uint16_t index, uint8_t *data)
{
	VL53L1_Error Status = VL53L1_ERROR_NONE;
	int32_t status_int;

	_I2CBuffer[0] = index >> 8;
	_I2CBuffer[1] = index & 0xFF;
	status_int = _I2CWrite(i2cHandle, dev, _I2CBuffer, 2);
	if (status_int)
	{
		Status = VL53L1_ERROR_CONTROL_INTERFACE;
		goto done;
	}
	status_int = _I2CRead(i2cHandle, dev, data, 1);
	if (status_int != 0)
	{
		Status = VL53L1_ERROR_CONTROL_INTERFACE;
	}
done:
	return Status;
}

int8_t VL53L1_RdWord(I2C_Handle i2cHandle, uint16_t dev, uint16_t index, uint16_t *data)
{
	VL53L1_Error Status = VL53L1_ERROR_NONE;
	int32_t status_int;

	_I2CBuffer[0] = index >> 8;
	_I2CBuffer[1] = index & 0xFF;
	status_int = _I2CWrite(i2cHandle, dev, _I2CBuffer, 2);

	if (status_int)
	{
		Status = VL53L1_ERROR_CONTROL_INTERFACE;
		goto done;
	}
	status_int = _I2CRead(i2cHandle, dev, _I2CBuffer, 2);
	if (status_int != 0)
	{
		Status = VL53L1_ERROR_CONTROL_INTERFACE;
		goto done;
	}

	*data = ((uint16_t)_I2CBuffer[0] << 8) + (uint16_t)_I2CBuffer[1];
done:
	return Status;
}

int8_t VL53L1_RdDWord(I2C_Handle i2cHandle, uint16_t dev, uint16_t index, uint32_t *data)
{
	VL53L1_Error Status = VL53L1_ERROR_NONE;
	int32_t status_int;

	_I2CBuffer[0] = index >> 8;
	_I2CBuffer[1] = index & 0xFF;
	status_int = _I2CWrite(i2cHandle, dev, _I2CBuffer, 2);
	if (status_int != 0)
	{
		Status = VL53L1_ERROR_CONTROL_INTERFACE;
		goto done;
	}
	status_int = _I2CRead(i2cHandle, dev, _I2CBuffer, 4);
	if (status_int != 0)
	{
		Status = VL53L1_ERROR_CONTROL_INTERFACE;
		goto done;
	}

	*data = ((uint32_t)_I2CBuffer[0] << 24) + ((uint32_t)_I2CBuffer[1] << 16) + ((uint32_t)_I2CBuffer[2] << 8) + (uint32_t)_I2CBuffer[3];

done:
	return Status;
}

int8_t VL53L1_WaitMs(uint16_t dev, int32_t wait_ms)
{
	return 0; // to be implemented
}

VL53L1_Error VL53L1_GetTickCount(
	uint32_t *ptick_count_ms)
{

	/* Returns current tick count in [ms] */

	VL53L1_Error status = VL53L1_ERROR_NONE;

	//*ptick_count_ms = timeGetTime();
	*ptick_count_ms = 0;

	return status;
}
