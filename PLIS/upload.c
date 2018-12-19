#include <LPC22xx.H>
#include <RTL.h>
#include "upload.h"
#include "spi_plis.h"
#include "compress.h"
#include "plis_fw.h"
//----------------------------------------------------------------------------
// Pins managed by PINSEL0
#define	bit_CONF_DONE_PIN		8		// P0.8 (in)
#define	bit_CONF_DONE_FUNCBIT	16
#define	bit_nSTATUS_PIN			9		// P0.9 (in)
#define	bit_nSTATUS_FUNCBIT		18
#define	bit_2nCE_PIN			10		// P0.10 (out)
#define	bit_2nCE_FUNCBIT		20
#define	bit_1nCE_PIN			11		// P0.11 (out)
#define	bit_1nCE_FUNCBIT		22
#define	bit_nCONFIG_PIN			12		// P0.12 (out)
#define	bit_nCONFIG_FUNCBIT		24
// Pins managed by PINSEL2
#define	bit_2oe_PIN				22		// P1.22 (out)
#define	bit_1oe_PIN				23		// P1.23 (out)
//----------------------------------------------------------------------------
#define	BUF_SIZE				512
static unsigned char plis_decompress_buffer[BUF_SIZE];
static unsigned char *plis_data_base_addr = 0;
//----------------------------------------------------------------------------
static unsigned char plis_ReadAddrFnc(unsigned long addr);
//----------------------------------------------------------------------------
#define	PLIS_PAUSE	\
	{	\
		volatile int i;	\
		for(i = 0; i < 100; i++);	\
	}
#define	PLIS_AFTER_UPLOAD_PAUSE	\
	{	\
		volatile int i;	\
		for(i = 0; i < 100000; i++);	\
	}
//----------------------------------------------------------------------------
#define	INIT_TIMEOUT	100		// 1000 ms
//----------------------------------------------------------------------------

unsigned char plis_Init(void)
{
	U16 count_10ms = 0;

	// Init SPI0 interface
	plis_spiInit();

	// Init I/O PLIS pins direction
	IODIR0 &= ~( (1<<bit_CONF_DONE_PIN)|(1<<bit_nSTATUS_PIN) );			// in
	IODIR0 |= (1<<bit_2nCE_PIN)|(1<<bit_1nCE_PIN)|(1<<bit_nCONFIG_PIN);	// out
	// Init PLIS pins as GPIO
	PINSEL0 &= ~( (3<<bit_CONF_DONE_FUNCBIT)|(3<<bit_nSTATUS_FUNCBIT)|
		(3<<bit_2nCE_FUNCBIT)|(3<<bit_1nCE_FUNCBIT)|(3<<bit_nCONFIG_FUNCBIT) );

	// Pins P1.23:22 must be configured as GPIO
	// (see Startup.s for PINSEL2 configuration)
	IODIR1 |= (1<<bit_1oe_PIN)|(1<<bit_2oe_PIN);	// out
	IOCLR1 |= (1<<bit_1oe_PIN)|(1<<bit_2oe_PIN);	// Write 0 in pin

	// Before programming, unselect both PLIS
	IOSET0 |= (1<<bit_1nCE_PIN);					// 1nCE = 1
	IOSET0 |= (1<<bit_2nCE_PIN);					// 2nCE = 1

	// Start programming firmware (1->0->1 sequence)
	IOSET0 |= (1<<bit_nCONFIG_PIN);
	PLIS_PAUSE;
	IOCLR0 |= (1<<bit_nCONFIG_PIN);
	PLIS_PAUSE;
	IOSET0 |= (1<<bit_nCONFIG_PIN);
	PLIS_PAUSE;

	// Wait until nSTATUS becomes 1
	while(!(IOPIN0 & (1<<bit_nSTATUS_PIN)))
	{
		os_dly_wait(1);	// wait 10 ms

		if((++count_10ms) >= INIT_TIMEOUT)
			break;
	}
	if(count_10ms == INIT_TIMEOUT)
		return 0;

	return 1;
}

//----------------------------------------------------------------------------

unsigned char plis_Upload(void)
{
	unsigned int i, decompressed;

	//-----------------------------------------------------------
	// Uploading PLIS #1
	//-----------------------------------------------------------

	// Select PLIS 1 to upload
	IOCLR0 |= (1<<bit_1nCE_PIN);					// 1nCE = 0
	PLIS_PAUSE;

	// Prepare decompressing of 'plis1_data'
	plis_data_base_addr = plis1_data;
	arithmetic_decompress_init(plis_ReadAddrFnc, plis1_data_size);

	do
	{
		arithmetic_decompress_chunk(plis_decompress_buffer,
			BUF_SIZE, &decompressed);

		for(i = 0; i < decompressed; i++)
		{
			plis_spiSend(plis_decompress_buffer[i]);

			if(!(IOPIN0 & (1<<bit_nSTATUS_PIN)))
				return 0;
		}

	} while(decompressed > 0);

	arithmetic_decompress_done();

	//-----------------------------------------------------------
	// Uploading PLIS #2
	//-----------------------------------------------------------

	// Select PLIS 2 to upload
	IOCLR0 |= (1<<bit_2nCE_PIN);					// 2nCE = 0
	PLIS_PAUSE;

	// Prepare decompressing of 'plis2_data'
	plis_data_base_addr = plis2_data;
	arithmetic_decompress_init(plis_ReadAddrFnc, plis2_data_size);

	do
	{
		arithmetic_decompress_chunk(plis_decompress_buffer,
			BUF_SIZE, &decompressed);

		for(i = 0; i < decompressed; i++)
		{
			plis_spiSend(plis_decompress_buffer[i]);

			if(!(IOPIN0 & (1<<bit_nSTATUS_PIN)))
				return 0;
		}

	} while(decompressed > 0);

	arithmetic_decompress_done();

	// Wait a little for both PLIS to become ready
	// (INIT_DONE pin monitoring not implemented)
	PLIS_AFTER_UPLOAD_PAUSE;

	return 1;
}

//----------------------------------------------------------------------------

static unsigned char plis_ReadAddrFnc(unsigned long addr)
{
	return plis_data_base_addr[addr];
}

//----------------------------------------------------------------------------
