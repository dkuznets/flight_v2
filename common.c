#include <LPC22xx.H>
#include <RTL.H>
#include "common.h"

//------------------------------------------------------------------------------

unsigned int disable_interrupts(void)
{
	unsigned int interrupts = VICIntEnable;

    VICIntEnClr = interrupts;

	return interrupts;
}

//------------------------------------------------------------------------------

void enable_interrupts(unsigned int interrupts)
{
    VICIntEnable = interrupts;
}

//------------------------------------------------------------------------------
