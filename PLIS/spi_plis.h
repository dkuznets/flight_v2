#ifndef __SPI_PLIS_H_ 
#define __SPI_PLIS_H_ 

#include <LPC22xx.H>
#include <RTL.h>

//----------------------------------------------------------------------------

void plis_spiInit(void);
U8 plis_spiSend(U8 outgoing);

//----------------------------------------------------------------------------

#endif
