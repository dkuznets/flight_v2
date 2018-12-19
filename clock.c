#include <LPC22xx.H>
#include <RTL.H>
#include <STRING.H>
#include "clock.h"
#include "i2c_mc.h"
//----------------------------------------------------------------------------
#define	ADDRESS		0xD0	// clock slave address [11010000]
//----------------------------------------------------------------------------
// internal registers definition (time)
#define	SEC_REG		0x00
#define	MIN_REG		0x01
#define	HOUR_REG	0x02
#define	DAY_REG		0x03
#define	DATE_REG	0x04
#define	MONTH_REG	0x05
#define	YEAR_REG	0x06
//----------------------------------------------------------------------------
// internal registers definition (temperature)
#define	TEMP_HI_REG	0x11
#define	TEMP_LO_REG	0x12
//----------------------------------------------------------------------------

typedef __packed union
{
	__packed struct
	{
		unsigned char sec;		// REG=0x00
		unsigned char min;		// REG=0x01
		unsigned char hour;		// REG=0x02
		unsigned char day;		// REG=0x03
		unsigned char date;		// REG=0x04
		unsigned char month;	// REG=0x05
		unsigned char year;		// REG=0x06

	} bytes;

	unsigned char raw_data[7];

} CLOCK_DATA_UNION;

typedef __packed union
{
	__packed struct
	{
		unsigned char hi;		// REG=0x11
		unsigned char lo;		// REG=0x12

	} bytes;

	unsigned char raw_data[2];

} CLOCK_TEMP_DATA_UNION;

//----------------------------------------------------------------------------

BOOL clock_GetDateTime(CLOCK_DATE_TIME *dt)
{
	CLOCK_DATA_UNION cdu;
	memset(cdu.raw_data, 0, sizeof(cdu));

	if(!mc_i2cReadRegs_8bit(ADDRESS, SEC_REG, sizeof(cdu), cdu.raw_data))
		return __FALSE;

	dt->year = 2000 + 10 * ((cdu.bytes.year & 0xf0) >> 4) + (cdu.bytes.year & 0x0f);
	dt->month = 10 * ((cdu.bytes.month & 0x10) >> 4) + (cdu.bytes.month & 0x0f);
	dt->date = 10 * ((cdu.bytes.date & 0x30) >> 4) + (cdu.bytes.date & 0x0f);
	dt->day = (cdu.bytes.day & 0x07);
	dt->hour = 10 * ((cdu.bytes.hour & 0x30) >> 4) + (cdu.bytes.hour & 0x0f);
	dt->min = 10 * ((cdu.bytes.min & 0x70) >> 4) + (cdu.bytes.min & 0x0f);
	dt->sec = 10 * ((cdu.bytes.sec & 0x70) >> 4) + (cdu.bytes.sec & 0x0f);

	return __TRUE;
}

//----------------------------------------------------------------------------

BOOL clock_SetDateTime(const CLOCK_DATE_TIME *dt)
{
	CLOCK_DATA_UNION cdu;
	memset(cdu.raw_data, 0, sizeof(cdu));

	cdu.bytes.year = ((((dt->year - 2000) / 10) << 4) & 0xf0) | (((dt->year - 2000) % 10) & 0x0f);
	cdu.bytes.month = (((dt->month / 10) << 4) & 0x10) | ((dt->month % 10) & 0x0f);
	cdu.bytes.date = (((dt->date / 10) << 4) & 0x30) | ((dt->date % 10) & 0x0f);
	cdu.bytes.day = (dt->day & 0x07);
	cdu.bytes.hour = (((dt->hour / 10) << 4) & 0x30) | ((dt->hour % 10) & 0x0f);
	cdu.bytes.min = (((dt->min / 10) << 4) & 0x70) | ((dt->min % 10) & 0x0f);
	cdu.bytes.sec = (((dt->sec / 10) << 4) & 0x70) | ((dt->sec % 10) & 0x0f);

	if(!mc_i2cWriteRegs_8bit(ADDRESS, SEC_REG, sizeof(cdu), cdu.raw_data))
		return __FALSE;

	return __TRUE;
}

//----------------------------------------------------------------------------

BOOL clock_GetTemperature(float *temp)
{
	unsigned short result;
	CLOCK_TEMP_DATA_UNION ctdu;
	memset(ctdu.raw_data, 0, sizeof(ctdu));

 	if(!mc_i2cReadRegs_8bit(ADDRESS, TEMP_HI_REG, sizeof(ctdu), ctdu.raw_data))
		return __FALSE;

	result = (((unsigned short)ctdu.bytes.hi << 8) | ctdu.bytes.lo);
	*temp = (float)((signed short)result) / 256.0;

	return __TRUE;
}

//----------------------------------------------------------------------------

BOOL clock_IsValidDateTime(const CLOCK_DATE_TIME *dt)
{
	if
	(
		dt->year  < 2000	|| dt->year  > 2099	||
		dt->month < 1		|| dt->month > 12	||
		dt->date  < 1		|| dt->date  > 31	||
		dt->day   < 1		|| dt->day   > 7	||
		dt->hour  > 23		||
		dt->min   > 59		||
		dt->sec   > 59
	)
	{
		return __FALSE;
	}

	return __TRUE;
}

//----------------------------------------------------------------------------
