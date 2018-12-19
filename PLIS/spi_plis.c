#include <LPC22XX.H>
#include <RTL.h>
#include "spi_plis.h"
//----------------------------------------------------------------------------
// SP0SPCR  Bit-Definitions
#define CPHA		      3
#define CPOL		      4
#define MSTR		      5
// SP0SPSR  Bit-Definitions
#define SPIF			  7

#define SPI_IODIR         IODIR0
#define SPI_SCK_PIN       4    /* Clock       P0.4  out */
#define SPI_MISO_PIN      5    /* from Card   P0.5  in  */
#define SPI_MOSI_PIN      6    /* to Card     P0.6  out */
#define SPI_SS_PIN	      7    /* Card-Select P0.7 - GPIO out */

#define SPI_PINSEL        PINSEL0
#define SPI_SCK_FUNCBIT   8
#define SPI_MISO_FUNCBIT  10
#define SPI_MOSI_FUNCBIT  12
#define SPI_SS_FUNCBIT    14

#define SPI_PRESCALE_REG  S0SPCCR
#define SPI_PRESCALE_MIN  8
//----------------------------------------------------------------------------

static void plis_spiSetSpeed(U8 speed)
{
	speed &= 0xFE;

	if(speed < SPI_PRESCALE_MIN)
		speed = SPI_PRESCALE_MIN;

	SPI_PRESCALE_REG = speed;
}

//----------------------------------------------------------------------------

void plis_spiInit(void)
{
	// setup pins direction
	SPI_IODIR |=  (1<<SPI_SCK_PIN)|(1<<SPI_MOSI_PIN);	//out
	SPI_IODIR &= ~((1<<SPI_MISO_PIN)|(1<<SPI_SS_PIN));	//in

	// reset pins functionality, set SPI0 pins functionality
	SPI_PINSEL &= ~( (3<<SPI_SCK_FUNCBIT) | (3<<SPI_MISO_FUNCBIT) |
		(3<<SPI_MOSI_FUNCBIT) | (3<<SPI_SS_FUNCBIT) );
	SPI_PINSEL |=  ( (1<<SPI_SCK_FUNCBIT) | (1<<SPI_MISO_FUNCBIT) |
		(1<<SPI_MOSI_FUNCBIT) | (1<<SPI_SS_FUNCBIT) );

	// enable SPI-Master
	S0SPCR = (1<<MSTR)|(0<<CPOL); // TODO: check CPOL

	// set max speed
	plis_spiSetSpeed(SPI_PRESCALE_MIN);
}

//----------------------------------------------------------------------------

U8 plis_spiSend(U8 outgoing)
{
	U8 incoming;

	S0SPDR = outgoing;
	while(!(S0SPSR & (1<<SPIF)));
	incoming = S0SPDR;

	return incoming;
}

//----------------------------------------------------------------------------
