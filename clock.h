#ifndef	__CLOCK__H
#define	__CLOCK__H
//----------------------------------------------------------------------------

typedef __packed struct
{
	unsigned short year;	// 2000 ... 2099
	unsigned char  month;	// 1 ... 12
	unsigned char  date;	// 1 ... 31
	unsigned char  day;		// 1 ... 7
	unsigned char  hour;	// 0 ... 23
	unsigned char  min;		// 0 ... 59
	unsigned char  sec;		// 0 ... 59

} CLOCK_DATE_TIME;

//----------------------------------------------------------------------------

BOOL clock_GetDateTime(CLOCK_DATE_TIME *dt);
BOOL clock_SetDateTime(const CLOCK_DATE_TIME *dt);
BOOL clock_IsValidDateTime(const CLOCK_DATE_TIME *dt);
BOOL clock_GetTemperature(float *temp);

//----------------------------------------------------------------------------

#endif	// __CLOCK__H
