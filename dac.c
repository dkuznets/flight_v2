#include <LPC22xx.H>
#include <RTL.H>
#include <STRING.H>
#include "DAC.h"
#include "i2c_mc.h"
//----------------------------------------------------------------------------
#define	ADDRESS		0x18	// DAC slave address [00011000]
//----------------------------------------------------------------------------
// internal registers definitions
#define	DAC_A		(1<<0)	// DACs' addresses, may be combined using
#define	DAC_B		(1<<1)	// logical OR operation
#define	DAC_C		(1<<2)	//
#define	DAC_D		(1<<3)	//
//----------------------------------------------------------------------------
// control bits definitions
#define	LDAC_BIT	(1<<12)	// [0]	All four DAC registers and, therefore, all DAC outputs,
							// 		are simultaneously updated on completion of the write sequence.
							// [1]	Only addressed input register is updated. There is no change
							//		in the contents of the DAC registers.
#define	CLR_BIT		(1<<13)	// [0]	All DAC registers and input registers are filled
							// 		with 0s on completion of the write sequence.
							// [1]	Normal operation
#define	PD0_BIT		(1<<14)	// PD1=0,PD0=0	Normal Operation
#define	PD1_BIT		(1<<15)	// PD1=0,PD0=1	Power-Down (1 kOhm load to GND)
							// PD1=1,PD0=0	Power-Down (100 kOhm load to GND)
							// PD1=1,PD0=1	Power-Down (three-state output)
//----------------------------------------------------------------------------

BOOL dac_GetA(unsigned short *value)
{
	unsigned short tmp;

	if(!mc_i2cReadReg_16bit(ADDRESS, DAC_A, &tmp))
		return __FALSE;

	*value = ((tmp >> 2) & 0x3ff);

	return __TRUE;
}

//----------------------------------------------------------------------------

BOOL dac_GetB(unsigned short *value)
{
	unsigned short tmp;

	if(!mc_i2cReadReg_16bit(ADDRESS, DAC_B, &tmp))
		return __FALSE;

	*value = ((tmp >> 2) & 0x3ff);

	return __TRUE;
}

//----------------------------------------------------------------------------

BOOL dac_GetC(unsigned short *value)
{
	unsigned short tmp;

	if(!mc_i2cReadReg_16bit(ADDRESS, DAC_C, &tmp))
		return __FALSE;

	*value = ((tmp >> 2) & 0x3ff);

	return __TRUE;
}

//----------------------------------------------------------------------------

BOOL dac_GetD(unsigned short *value)
{
	unsigned short tmp;

	if(!mc_i2cReadReg_16bit(ADDRESS, DAC_D, &tmp))
		return __FALSE;

	*value = ((tmp >> 2) & 0x3ff);

	return __TRUE;
}

//----------------------------------------------------------------------------

BOOL dac_SetA(unsigned short value)
{
	unsigned short tmp = ((value & 0x3ff) << 2) | CLR_BIT;

	if(!mc_i2cWriteReg_16bit(ADDRESS, DAC_A, tmp))
		return __FALSE;

	return __TRUE;
}

//----------------------------------------------------------------------------

BOOL dac_SetB(unsigned short value)
{
	unsigned short tmp = ((value & 0x3ff) << 2) | CLR_BIT;

	if(!mc_i2cWriteReg_16bit(ADDRESS, DAC_B, tmp))
		return __FALSE;

	return __TRUE;
}

//----------------------------------------------------------------------------

BOOL dac_SetC(unsigned short value)
{
	unsigned short tmp = ((value & 0x3ff) << 2) | CLR_BIT;

	if(!mc_i2cWriteReg_16bit(ADDRESS, DAC_C, tmp))
		return __FALSE;

	return __TRUE;
}

//----------------------------------------------------------------------------

BOOL dac_SetD(unsigned short value)
{
	unsigned short tmp = ((value & 0x3ff) << 2) | CLR_BIT;

	if(!mc_i2cWriteReg_16bit(ADDRESS, DAC_D, tmp))
		return __FALSE;

	return __TRUE;
}

//----------------------------------------------------------------------------
