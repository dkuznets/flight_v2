#include <LPC22xx.H>
#include <RTL.h>
#include "termostat.h"
#include "adc.h"
//----------------------------------------------------------------------------
#define	CHANNEL1_BIT			(1<< 0)
#define	CHANNEL2_BIT			(1<<21)
//----------------------------------------------------------------------------
#define	MIN_T					50.0	// degree  [K]
#define	MAX_T					300.0	// degree  [K]
#define	MIN_V					0.73f	// voltage [V]
#define	MAX_V					1.11f	// voltage [V]
//----------------------------------------------------------------------------
#define	TEMPERATURE_KELVIN(v)	((float)((MAX_V-(v))/((MAX_V-MIN_V)/(MAX_T-MIN_T))+MIN_T))
#define	KELVIN_TO_CELCIUS(k)	((k)-273.16f)
//----------------------------------------------------------------------------

void termostat_Init(void)
{
	// P0.0 as GPIO output
	PINSEL0 &= ~((1<< 0)|(1<< 1));
	IODIR0  |=  CHANNEL1_BIT;
	IOSET0   =  CHANNEL1_BIT;
	// P0.21 as GPIO output
	PINSEL1 &= ~((1<<10)|(1<<11));
	IODIR0  |=  CHANNEL2_BIT;
	IOSET0   =  CHANNEL2_BIT;
}

//----------------------------------------------------------------------------

void termostat_Enable(TERMOSTAT_CHANNEL channel, BIT enable)
{
	switch(channel)
	{
		case tchCMOS1:
			IOPIN0 = (IOPIN0 & ~CHANNEL1_BIT) | (enable ? 0 : CHANNEL1_BIT);
			break;

		case tchCMOS2:
			IOPIN0 = (IOPIN0 & ~CHANNEL2_BIT) | (enable ? 0 : CHANNEL2_BIT);
			break;

		default:
			break;
	}
}

//----------------------------------------------------------------------------

float termostat_GetTemperature(TERMOSTAT_CHANNEL channel)
{
	static float voltage;
	static float t_K, t_C;
	
	switch(channel)
	{
		case tchCMOS1:
			voltage = ADC_VOLTAGE(adc_GetValue(adcchCMOS1));
			break;

		case tchCMOS2:
			voltage = ADC_VOLTAGE(adc_GetValue(adcchCMOS2));
			break;

		default:
			return 0;
	}

	t_K = TEMPERATURE_KELVIN(voltage);
	t_C = KELVIN_TO_CELCIUS(t_K);

	return t_C;
}

//----------------------------------------------------------------------------
