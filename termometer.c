#include <LPC22xx.H>
#include <RTL.H>
#include "termometer.h"
#include "i2c_mc.h"
//----------------------------------------------------------------------------
#define	ADDRESS				0x90	// termometer I2C slave address [10010000]
//----------------------------------------------------------------------------
#define	TEMP_REG			0xAA	// read temperature register, 16-bit
#define	START_REG			0x51	// start convert register, 8-bit
#define	CFG_REG				0xAC	// configuration register, 8-bit
//----------------------------------------------------------------------------
#define	CFG_REG_1SHOT_BIT	(1<<0)	// 0=continuous conversion, 1=single conversion mode
//----------------------------------------------------------------------------

void termo_Init(void)
{
	unsigned char result = 0;

	if(!mc_i2cReadReg_8bit(ADDRESS, CFG_REG, &result))
		return;

	if(result & CFG_REG_1SHOT_BIT)
	{
		// set continuous conversion mode and start conversion
		mc_i2cWriteReg_8bit(ADDRESS, CFG_REG, result & (~CFG_REG_1SHOT_BIT));
		mc_i2cWriteReg_0bit(ADDRESS, START_REG);
	}
}

//----------------------------------------------------------------------------

BOOL termo_GetTemperature(float *temp)
{
	unsigned short result = 0;

	if(!mc_i2cReadReg_16bit(ADDRESS, TEMP_REG, &result))
		return __FALSE;

	*temp = ((float)((signed short)result) / 256.0);

	return __TRUE;
}

//----------------------------------------------------------------------------
