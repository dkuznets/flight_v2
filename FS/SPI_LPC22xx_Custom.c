/*----------------------------------------------------------------------------
 *      RL-ARM - FlashFS
 *----------------------------------------------------------------------------
 *      Name:    SPI_LPC22xx.c
 *      Purpose: Serial Peripheral Interface Driver for NXP LPC2294/01 with SSP
 *      Rev.:    V4.12
 *----------------------------------------------------------------------------
 *      This code is part of the RealView Run-Time Library.
 *      Copyright (c) 2004-2010 KEIL - An ARM Company. All rights reserved.
 *---------------------------------------------------------------------------*/

#include <File_Config.h>
#include <LPC22xx.H>                 /* LPC22xx definitions                  */

/* SSPSR - bit definitions. */
#define TFE     0x01
#define TNF     0x02
#define RNE     0x04
#define RFF     0x08
#define BSY     0x10


/*----------------------------------------------------------------------------
 *      SPI Driver Functions
 *----------------------------------------------------------------------------
 *  Required functions for SPI driver module:
 *   - void spi_init     ()
 *   - void spi_ss       (U32 ss)
 *   - U8   spi_send     (U8 outb)
 *   - void spi_hi_speed (BOOL on)
 *---------------------------------------------------------------------------*/

/*--------------------------- spi_init --------------------------------------*/

void spi_init (void) {
  /* Initialize and enable the SSP Interface module. */

  /* Disable SPI1 power (enabled by default), enable SSP power */
  PCONP = (PCONP & ~(1<<10)) | (1<<21);

  /* SCK1, MISO1, MOSI1 are SSP pins. */
  PINSEL1  = (PINSEL1 & ~0x000003FC) | 0x000000A8;
  PINSEL2 &= ~(1<<3);

  /* Custom pins usage: P0.13, P0.15, P0.16 - as GPIO output
      P0.13 - 0=select ROM chip as active device, 1=unselect ROM chip
      P0.15 - 0=turn on SD-Card power, 1=turn off SD-Card power
      P0.16 - 0=select SD-Card, 1=unselect SD-Card as active device */
//  IODIR0  |= (1<<13)|(1<<15)|(1<<16);
//  IOSET0   = (1<<13)|(1<<16);	/* P0.16 used instead of SSEL1 */
//  IOCLR0   = (1<<15);

  IODIR0  |= (1<<13)|(1<<15)|(1<<16);
  IOSET0   = (1<<13)|(1<<15)|(1<<16);

  IODIR1  |= (1<<20)|(1<<21);
  IOSET1   = (1<<20)|(1<<21);

  /* Enable SSP in Master Mode, SPI, 8-bit Transfer, CPOL=0, CPHA=0, SCR=1
     Note: SSP_Clock = CCLK/(VPB*(SCR + 1)), must be <= 25 MHz */
  SSPCR0   = 0x0107;
  SSPCR1   = 0x0002;
  SSPCPSR  = 0xFE;
}


/*--------------------------- spi_hi_speed ----------------------------------*/

void spi_hi_speed (BOOL on) {
  /* Set a SPI clock to low/high speed for SD/MMC. */

  if (on == __TRUE) {
    /* Max. rate used for Data Transfer. */
    SSPCPSR = 0x02;
  }
  else {
    /* Min. rate used in Card Initialization. */
    SSPCPSR = 0xFE;
  }
}


/*--------------------------- spi_ss ----------------------------------------*/

void spi_ss (U32 ss) {
  /* Enable/Disable SPI Chip Select (drive it high or low). */

  if(ss) {
//	IOSET0 = (1<<16);
	IOSET0 = (1<<13);
  }
  else {
//	IOCLR0 = (1<<16);
	IOCLR0 = (1<<13);
  }
}


/*--------------------------- spi_send --------------------------------------*/

U8 spi_send (U8 outb) {
  /* Write and Read a byte on SPI interface. */

  /* Wait if TNF cleared, Tx FIFO is full. */
  while (!(SSPSR & TNF));
  SSPDR = outb;

  /* Wait if RNE cleared, Rx FIFO is empty. */
  while (!(SSPSR & RNE));
  return (U8)(SSPDR);
}


/*----------------------------------------------------------------------------
 * end of file
 *---------------------------------------------------------------------------*/
