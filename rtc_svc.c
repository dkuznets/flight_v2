#include <LPC214X.H>
#include <RTL.H>
#include "rtc_svc.h"
#include "common.h"
//---------------------------------------------------------------------------
#define	CPU_CLOCK	60000000						// in Hz
//---------------------------------------------------------------------------
//#define	USE_EXTERNAL_32768_QUARTZ				// flag, if defined - external quartz used
//---------------------------------------------------------------------------
typedef struct
{
	U16 ctc;										// 15-bit tick counter (freq.=32768 Hz)
	U32 ct0;										// consolidated time #0 placeholder
	U32 ct1;										// consolidated time #1 placeholder
	U32 ct2;										// consolidated time #2 placeholder

} RTC_TIMES_STRUCT, *PRTC_TIMES_STRUCT;
//---------------------------------------------------------------------------

void rtc_Init(void)
{
	CCR		= (1<<1);								// disable & reset RTC (for configuration)
	ILR		= 0x00;									// generate neighter increment nor alarm interrupts

	// reset all
	SEC		= 0;									// sec 0...59
	MIN		= 0;									// min 0...59
	HOUR	= 0;									// hour 0...23
	DOM		= 1;									// day of month 1...28,(29),(30),(31)
	DOW		= 0;									// day of week 0...6
	DOY		= 1;									// day of year 1...365 (366)
	MONTH	= 1;									// month 1...12
	YEAR	= 0;									// year 0...4095

#ifdef	USE_EXTERNAL_32768_QUARTZ
	CCR    |=  (1<<4);								// use external 32.768kHz quartz instead of prescaler
													// (be careful with CTC read, see manual for details)
#else
	PREINT	= (CPU_CLOCK / 32768) - 1;				// integer prescaler
	PREFRAC	= CPU_CLOCK - ((PREINT + 1) * 32768);	// fractional prescaler
#endif

	CCR    &= ~(1<<1);								// clear RESET bit
	CCR    |=  (1<<0);								// enable RTC
}

//---------------------------------------------------------------------------

U32 rtc_GetTickCount(void)
{
	U32 interrupts;
	static RTC_TIMES_STRUCT ts1, ts2;

	interrupts = disable_interrupts();

	// capture registers until they are all captured at the same moment of time
	while(1)
	{
		ts1.ctc = CTC;
		ts1.ct0 = CTIME0;
		ts1.ct1 = CTIME1;
		ts1.ct2 = CTIME2;

		ts2.ctc = CTC;
		ts2.ct0 = CTIME0;
		ts2.ct1 = CTIME1;
		ts2.ct2 = CTIME2;

		if
		(
			(ts1.ctc & 0xfffe) == (ts2.ctc & 0xfffe) &&
			ts1.ct0 == ts2.ct0 && ts1.ct1 == ts2.ct1 &&
			ts1.ct2 == ts2.ct2
		)
		{
			break;
		}
	}

	enable_interrupts(interrupts);

	return (
		((U32)(ts1.ctc >> 1   )       ) * 1000 / 32768        +	// ms
		((ts1.ct0 & 0x0000003f)       ) *                1000 +	// sec
		((ts1.ct0 & 0x00003f00) >>  8 ) *           60 * 1000 +	// min
		((ts1.ct0 & 0x001f0000) >> 16 ) *      60 * 60 * 1000 +	// hour
		((ts1.ct2 & 0x00000fff)    -1 ) * 24 * 60 * 60 * 1000	// day of year
	);
}

//---------------------------------------------------------------------------

U64 rtc_GetTickCount_us(void)
{
	U32 interrupts;
	static RTC_TIMES_STRUCT ts1, ts2;

	interrupts = disable_interrupts();

	// capture registers until they are all captured at the same moment of time
	while(1)
	{
		ts1.ctc = CTC;
		ts1.ct0 = CTIME0;
		ts1.ct1 = CTIME1;
		ts1.ct2 = CTIME2;

		ts2.ctc = CTC;
		ts2.ct0 = CTIME0;
		ts2.ct1 = CTIME1;
		ts2.ct2 = CTIME2;

		if
		(
			(ts1.ctc & 0xfffe) == (ts2.ctc & 0xfffe) &&
			ts1.ct0 == ts2.ct0 && ts1.ct1 == ts2.ct1 &&
			ts1.ct2 == ts2.ct2
		)
		{
			break;
		}
	}

	enable_interrupts(interrupts);

	return (
		((U64)(ts1.ctc >> 1)                ) * 1000000 / 32768        +	// us
		((U64)((ts1.ct0 & 0x0000003f)      )) *                1000000 +	// sec
		((U64)((ts1.ct0 & 0x00003f00) >>  8)) *           60 * 1000000 +	// min
		((U64)((ts1.ct0 & 0x001f0000) >> 16)) *      60 * 60 * 1000000 +	// hour
		((U64)((ts1.ct2 & 0x00000fff)    -1)) * 24 * 60 * 60 * 1000000		// day of year
	);
}

//---------------------------------------------------------------------------

U64 rtc_GetTickCount_quartz(void)
{
	U32 interrupts;
	static RTC_TIMES_STRUCT ts1, ts2;

	interrupts = disable_interrupts();

	// capture registers until they are all captured at the same moment of time
	while(1)
	{
		ts1.ctc = CTC;
		ts1.ct0 = CTIME0;
		ts1.ct1 = CTIME1;
		ts1.ct2 = CTIME2;

		ts2.ctc = CTC;
		ts2.ct0 = CTIME0;
		ts2.ct1 = CTIME1;
		ts2.ct2 = CTIME2;

		if
		(
			(ts1.ctc & 0xfffe) == (ts2.ctc & 0xfffe) &&
			ts1.ct0 == ts2.ct0 && ts1.ct1 == ts2.ct1 &&
			ts1.ct2 == ts2.ct2
		)
		{
			break;
		}
	}

	enable_interrupts(interrupts);

	return (
		((U64)(ts1.ctc >> 1)                )                         +	// quartz ticks (32768 Hz)
		((U64)((ts1.ct0 & 0x0000003f)      ))                 * 32768 +	// sec
		((U64)((ts1.ct0 & 0x00003f00) >>  8)) *           60  * 32768 +	// min
		((U64)((ts1.ct0 & 0x001f0000) >> 16)) *      60 * 60  * 32768 +	// hour
		((U64)((ts1.ct2 & 0x00000fff)    -1)) * 24 * 60 * 60  * 32768	// day of year
	);
}

//---------------------------------------------------------------------------
