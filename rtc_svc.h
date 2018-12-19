#ifndef	__RTC_SERVICE__H
#define	__RTC_SERVICE__H
//---------------------------------------------------------------------------

void   rtc_Init(void);
U32    rtc_GetTickCount(void);			// in [ms]
U64    rtc_GetTickCount_us(void);		// in [um]
U64    rtc_GetTickCount_quartz(void);	// in quartz ticks (1/32768 sec)
double rtc_GetSecCount(void);			// in [sec]

//---------------------------------------------------------------------------
#endif	// __RTC_SERVICE__H
