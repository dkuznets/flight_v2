#include <LPC22XX.H>
#include <RTL.H>
#include "adc.h"
#include "common.h"
//------------------------------------------------------------------------------
#define	ADC_DIVIDER		14	// ADC devider, (VPBCLK / ADC_DIV < 4.5 MHz), 60/14=4.3MHz
#define	ADC_AVARAGING	20
//------------------------------------------------------------------------------

void adc_Init(void)
{
	// P0.27 as AIN0
	PINSEL1 &= ~(1<<23);
	PINSEL1 |=  (1<<22);
	// P0.28 as AIN1
	PINSEL1 &= ~(1<<25);
	PINSEL1 |=  (1<<24);
}

//------------------------------------------------------------------------------

U16 adc_GetValue(ADC_CHANNEL channel)
{
	U32 i;
	U32 acc = 0;

	U32 ctrl_reg;
	U16 adc_value;

	switch(channel)
	{
		case adcchCMOS1:	ctrl_reg = (1<<0)|(ADC_DIVIDER<<8)|(1<<21)|(1<<24);	break;
		case adcchCMOS2:	ctrl_reg = (1<<1)|(ADC_DIVIDER<<8)|(1<<21)|(1<<24);	break;
		default:
			return 0;
	}

	// ignore 1-st reading
	ADCR = ctrl_reg;
	while(!(ADDR & (1UL<<31)));
	adc_value = (ADDR >> 6) & 0x03ff;

	// avarage readings
	for(i = 0; i < ADC_AVARAGING; i++)
	{
		ADCR = ctrl_reg;
		while(!(ADDR & (1UL<<31)));
		adc_value = (ADDR >> 6) & 0x03ff;

		acc += adc_value;
	}
	acc /= ADC_AVARAGING;

	return (U16)acc;
}

//------------------------------------------------------------------------------
