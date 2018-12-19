#include <LPC22xx.h>
#include <RTL.H>
#include <string.h>
#include "i2c_mc.h"
//------------------------------------------------------------------------------
// I2C Status Message Codes (Master Codes)
//------------------------------------------------------------------------------
#define		START_TXD			0x08		// Start condition for bus transmited
#define		REPEAT_START_TXD	0x10		// Repeated Start condition for bus transmited
#define		ADDRESS_TXD_ACK		0x18		// Address plus write sent, ack recieved
#define		ADDRESS_TXD_NOACK	0x20		// Address plus write sent, NO ack recieved!
#define		DATA_TXD_ACK		0x28		// Data sent, ack recieved
#define		DATA_TXD_NOACK		0x30		// Data sent, NO ack recieved!
#define		ARB_LOST			0x38		// I2C Master Arbitration Lost
#define		ADDRESS_RXD_ACK		0x40		// Address plus read sent, ack recieved
#define		ADDRESS_RXD_NOACK	0x48		// Address plus read sent, NO ack recieved!
#define		DATA_RXD_ACK		0x50		// Data received, ack sent
#define		DATA_RXD_NOACK		0x58		// Data received, NO ack sent!
//------------------------------------------------------------------------------
// I2C Pins Definitions
//------------------------------------------------------------------------------
#define		I2C_SCL_PIN			2			// P0.2
#define		I2C_SDA_PIN			3			// P0.3
#define		I2C_SCL_FUNCBIT		4			// Used in PINSEL0
#define		I2C_SDA_FUNCBIT		6			// Used in PINSEL0

#define		VIC_NUM_I2C			6			// Use free vector (7-14 may be in use by RTX CAN1-CAN4)
//------------------------------------------------------------------------------
// I2CONSET register bit definitions
//------------------------------------------------------------------------------
#define		I2EN_BIT			6			//
#define		STA_BIT				5			//
#define		STO_BIT				4			//
#define		SI_BIT				3			//
#define		AA_BIT				2			//
//------------------------------------------------------------------------------
// I2CONCLR register bit definitions
//------------------------------------------------------------------------------
#define		I2ENC_BIT			6			//
#define		STAC_BIT			5			//
#define		SIC_BIT				3			//
#define		AAC_BIT				2			//
//------------------------------------------------------------------------------
// I2C status defenitions
//------------------------------------------------------------------------------
#define		I2C_ERROR			0x00		//
#define		I2C_OK				0x01		//
#define		I2C_BUSY			0x02		//
//------------------------------------------------------------------------------
// I2C device address R/W bit
//------------------------------------------------------------------------------
#define		I2C_RW_READ			0x01		//
#define		I2C_RW_WRITE		0x00		//
//------------------------------------------------------------------------------
// common variables
static OS_MUT mutex;						// inter-task sync. object
static unsigned char cmn_buffer[1 + 255];	// multi-byte transmition data: adr(1) + buf(255)
//------------------------------------------------------------------------------
static void i2c_ISR(void) __irq;
//------------------------------------------------------------------------------
static unsigned char *OUT_BUFFER, OUT_BYTE_NUMBER, *IN_BUFFER, IN_BYTE_NUMBER;
static unsigned char volatile TRANSFER_STATUS;
static unsigned char ADDRESS;
//------------------------------------------------------------------------------

static BOOL mc_i2cStart(void)
{
	U16 delay = 0;

	TRANSFER_STATUS = I2C_BUSY;
 	I2CONSET = (1<<I2EN_BIT)|(1<<STA_BIT);	// (I2EN=1, STA=1) Send start signal

	while(TRANSFER_STATUS == I2C_BUSY)		// Wait until finished
	{
		os_dly_wait(1);

		if(++delay == 10)					// 0.1 sec timeout occured
			TRANSFER_STATUS = I2C_ERROR;
	}

	return (TRANSFER_STATUS == I2C_OK ? __TRUE : __FALSE);
}

//------------------------------------------------------------------------------

void mc_i2cInit(void)
{
	// Reset P0.3:2 pins functionallity, then set pins as I2C (SCL & SDA)
	PINSEL0 &= ~((3<<I2C_SCL_FUNCBIT)|(3<<I2C_SDA_FUNCBIT));
	PINSEL0 |=  ((1<<I2C_SCL_FUNCBIT)|(1<<I2C_SDA_FUNCBIT));

	// Clear all I2C flags, then enable I2C interface
	I2CONCLR = (1<<I2ENC_BIT)|(1<<STAC_BIT)|(1<<SIC_BIT)|(1<<AAC_BIT);
	I2CONSET = (1<<I2EN_BIT);

	// Set I2C bitrate to 100 kHz			// Bitrate = PCLK / (I2SCLH + I2SCLL).
	I2SCLH = 300; I2SCLL = 300;				// PCLK = CPU_CLK / VPBDIV, I2SCLH and I2SCLL
											// must not have same values at one time.
	// Set interrupt vector for I2C interrupt #9
	*(&VICVectAddr0 + VIC_NUM_I2C) = (unsigned long)i2c_ISR;
	*(&VICVectCntl0 + VIC_NUM_I2C) = 0x20 | 9;

	// Enable I2C interrupt #9
	VICIntEnable = (1<<9);

	// Init inter-task sync. object
	os_mut_init(mutex);
}

//------------------------------------------------------------------------------

BOOL mc_i2cWriteReg_0bit(unsigned char adr, unsigned char reg)
{
	BOOL i2c_result;
	unsigned char out_buffer[1];

	os_mut_wait(mutex, 0xffff);

	// Store out buffer with data
	out_buffer[0]	= reg;
	// Store I/O parameters processed by interrupt
	ADDRESS			= adr | I2C_RW_WRITE;
	OUT_BYTE_NUMBER	= 1;
	OUT_BUFFER		= out_buffer;
	IN_BYTE_NUMBER	= 0;
	// Start operating
	i2c_result = mc_i2cStart();

	os_mut_release(mutex);

	return i2c_result;
}

//------------------------------------------------------------------------------

BOOL mc_i2cReadReg_16bit(unsigned char adr, unsigned char reg, unsigned short *val)
{
	BOOL i2c_result;
	unsigned char out_buffer[1], in_buffer[2];

	os_mut_wait(mutex, 0xffff);

	// Store out buffer with data
	out_buffer[0]	= reg;
	// Store I/O parameters processed by interrupt
	ADDRESS			= adr | I2C_RW_READ;
	OUT_BYTE_NUMBER	= 1;
	OUT_BUFFER		= out_buffer;
	IN_BYTE_NUMBER	= 2;
	IN_BUFFER		= in_buffer;
	// Start operating
	i2c_result = mc_i2cStart();

	os_mut_release(mutex);

	if(!i2c_result)
		return __FALSE;

	*val = ((unsigned short)in_buffer[0] << 8) | (in_buffer[1]);

	return __TRUE;
}

//------------------------------------------------------------------------------

BOOL mc_i2cWriteReg_16bit(unsigned char adr, unsigned char reg, unsigned short val)
{
	BOOL i2c_result;
	unsigned char out_buffer[3];

	os_mut_wait(mutex, 0xffff);

	// Store out buffer with data
	out_buffer[0]	= reg;
	out_buffer[1]	= ((val & 0xFF00) >> 8);
	out_buffer[2]	= (val & 0x00FF);
	// Store I/O parameters processed by interrupt
	ADDRESS			= adr | I2C_RW_WRITE;
	OUT_BYTE_NUMBER	= 3;
	OUT_BUFFER		= out_buffer;
	IN_BYTE_NUMBER	= 0;
	// Start operating
	i2c_result = mc_i2cStart();

	os_mut_release(mutex);

	return i2c_result;
}

//------------------------------------------------------------------------------

BOOL mc_i2cReadReg_8bit(unsigned char adr, unsigned char reg, unsigned char *val)
{
	BOOL i2c_result;
	unsigned char out_buffer[1], in_buffer[1];

	os_mut_wait(mutex, 0xffff);

	// Store out buffer with data
	out_buffer[0]	= reg;
	// Store I/O parameters processed by interrupt
	ADDRESS			= adr | I2C_RW_READ;
	OUT_BYTE_NUMBER	= 1;
	OUT_BUFFER		= out_buffer;
	IN_BYTE_NUMBER	= 1;
	IN_BUFFER		= in_buffer;
	// Start operating
	i2c_result = mc_i2cStart();

	os_mut_release(mutex);

	if(!i2c_result)
		return __FALSE;

	*val = in_buffer[0];

	return __TRUE;
}

//------------------------------------------------------------------------------

BOOL mc_i2cWriteReg_8bit(unsigned char adr, unsigned char reg, unsigned char val)
{
	BOOL i2c_result;
	unsigned char out_buffer[2];

	os_mut_wait(mutex, 0xffff);

	// Store out buffer with data
	out_buffer[0]	= reg;
	out_buffer[1]	= val;
	// Store I/O parameters processed by interrupt
	ADDRESS			= adr | I2C_RW_WRITE;
	OUT_BYTE_NUMBER	= 2;
	OUT_BUFFER		= out_buffer;
	IN_BYTE_NUMBER	= 0;
	// Start operating
	i2c_result = mc_i2cStart();

	os_mut_release(mutex);

	return i2c_result;
}

//------------------------------------------------------------------------------

BOOL mc_i2cReadRegs_8bit(unsigned char adr, unsigned char start_reg,
	unsigned char reg_count, unsigned char *buffer)
{
	BOOL i2c_result;
	unsigned char out_buffer[1], *in_buffer = cmn_buffer;

	os_mut_wait(mutex, 0xffff);

	// Store out buffer with data
	out_buffer[0]	= start_reg;
	// Store I/O parameters processed by interrupt
	ADDRESS			= adr | I2C_RW_READ;
	OUT_BYTE_NUMBER	= 1;
	OUT_BUFFER		= out_buffer;
	IN_BYTE_NUMBER	= reg_count;
	IN_BUFFER		= in_buffer;
	// Start operating
	i2c_result = mc_i2cStart();

	os_mut_release(mutex);

	if(!i2c_result)
		return __FALSE;

	memcpy(buffer, in_buffer, reg_count);

	return __TRUE;
}

//------------------------------------------------------------------------------

BOOL mc_i2cWriteRegs_8bit(unsigned char adr, unsigned char start_reg,
	unsigned char reg_count, const unsigned char *buffer)
{
	BOOL i2c_result;
	unsigned char *out_buffer = cmn_buffer;

	os_mut_wait(mutex, 0xffff);

	// Store out buffer with data
	out_buffer[0] = start_reg;
	memcpy(&out_buffer[1], buffer, reg_count);
	// Store I/O parameters processed by interrupt
	ADDRESS			= adr | I2C_RW_WRITE;
	OUT_BYTE_NUMBER	= 1 + reg_count;
	OUT_BUFFER		= out_buffer;
	IN_BYTE_NUMBER	= 0;
	// Start operating
	i2c_result = mc_i2cStart();

	os_mut_release(mutex);

	return i2c_result;
}

//------------------------------------------------------------------------------

static void i2c_ISR(void) __irq
{
	switch(I2STAT)								// Status code for the I2C
	{
		// Master Transmitter/Receiver: START condition transmitted.
		// The R/W bit of the COMMAND word sent after this state will
		// always be a zero (W) because for both read and write,
		// the memory address must be written first.
	case START_TXD:
		I2DAT = (ADDRESS & 0xFE);				// Load address of the slave to be accessed.
   		I2CONCLR = (1<<STAC_BIT);				// Manually clear start bit (STA = 0)
		break;

		// Master Transmitter/Receiver: Repeated START condition transmitted.
		// This state should only occur during a read, after the memory address has been
		// sent and acknowledged.
	case REPEAT_START_TXD:
		I2DAT = ADDRESS;						// ADDRESS should hold slave address + R.
		I2CONCLR = (1<<STAC_BIT);				// Manually clear start bit (STA = 0)
		break;

		// Master Transmitter: Slave address + WRITE transmitted.  ACK received.
	case ADDRESS_TXD_ACK:
		I2DAT = *OUT_BUFFER++;					// Load first byte from data buffer
		OUT_BYTE_NUMBER--;						// Decrement number of bytes
		break;

		// Master Transmitter: Slave address + WRITE transmitted.  NACK received.
		// The slave is not responding.  Send a STOP followed by a START to try again.
	case ADDRESS_TXD_NOACK:
		I2CONSET = (1<<STO_BIT);				// (STO = 1) Stop the transfer - ERROR
		TRANSFER_STATUS = I2C_ERROR;
		break;

		// Master Transmitter: Data byte transmitted.  ACK received.
		// This state is used in both READ and WRITE operations.  
	case DATA_TXD_ACK:
		if(ADDRESS & I2C_RW_READ)				// If R/W=READ, send repeated START.
		{
			I2CONSET = (1<<STA_BIT);			// STA = 1
		}
		else									// If R/W=WRITE, sent register value
		{
			if(OUT_BYTE_NUMBER == 0)			// If OUT_BYTE_NUMBER=0, transfer is finished.
			{
				I2CONSET = (1<<STO_BIT);		// STO = 1
				TRANSFER_STATUS = I2C_OK;
			}
			else
			{
				I2DAT = *OUT_BUFFER++;			// Load next byte from data buffer
				OUT_BYTE_NUMBER--;				// Decrement for next time around.
			}
		}
		break;

		// Master Transmitter: Data byte transmitted.  NACK received.
		// Slave not responding.  Send STOP followed by START to try again.
	case DATA_TXD_NOACK:
		I2CONSET = (1<<STO_BIT);				// (STO = 1) Stop the transfer - ERROR
		TRANSFER_STATUS = I2C_ERROR;
		break;

		// Master Transmitter: Arbitration lost.
		// Should not occur.  If so, restart transfer.
	case ARB_LOST:
		I2CONSET = (1<<STO_BIT);				// (STO = 1) Stop the transfer - ERROR
		TRANSFER_STATUS = I2C_ERROR;
		break;

		// Master Receiver: Slave address + READ transmitted.  ACK received.
		// Set to transmit NACK after next transfer (AA=0) since it will be
		// the last (only) byte. Else set AA=1.
	case ADDRESS_RXD_ACK:
		if(IN_BYTE_NUMBER == 1)					// If only 1 byte is to be received
			I2CONCLR = (1<<AAC_BIT);			// (AA = 0) This will generate NACK right after 
												// a bytes is received to stop receiving
		else
			I2CONSET = (1<<AA_BIT);				// (AA = 1) This will generate ACK
		break;

		// Master Receiver: Slave address + READ transmitted.  NACK received.
		// Slave not responding.  Send repeated start to try again.
	case ADDRESS_RXD_NOACK:
		I2CONSET = (1<<STO_BIT);				// (STO = 1) Stop the transfer - ERROR
		TRANSFER_STATUS = I2C_ERROR;
		break;

		// Data byte received.  ACK transmitted.
		// Read operation has completed.  Read data register.
	case DATA_RXD_ACK:
		// This state shouldn`t occure because we use AA=0 instead
		// to stop receiving after the last byte is received.
		// If occures, stop operation and free the bus.
		if(IN_BYTE_NUMBER == 0)					// If IN_BYTE_NUMBER=0, receive is finished. 
		{
			I2CONSET = (1<<STO_BIT);			// (STO = 1) Reset communication.
			TRANSFER_STATUS = I2C_OK;			// Free I2C
		}
		else
		{
			if(IN_BYTE_NUMBER == 1)				// The last byte is to be received.
				I2CONCLR = (1<<AAC_BIT);		// (AA = 0) This will generate NACK right after 
												// the last bytes is received to stop receiving
			*IN_BUFFER++ = I2DAT;				// Read a byte.
			IN_BYTE_NUMBER--;					// Decrement for next time around.
		}
		break;

		// Data byte received.  NACK transmitted.
		// Receive is finished. Send STOP.
	case DATA_RXD_NOACK:
		*IN_BUFFER = I2DAT;						// Receive the last byte.
		I2CONSET = (1<<STO_BIT)|(1<<AA_BIT);	// (STO = 1, AA = 1) Reset communication, set AA for next transfer.
		TRANSFER_STATUS = I2C_OK;
		break;

		// All other status codes meaningless in this application. Reset communication.
	default:
		I2CONSET = (1<<STO_BIT);				// (STO = 1) Reset communication.
		TRANSFER_STATUS = I2C_ERROR;
		break;
	}

	I2CONCLR = (1<<SIC_BIT);					// (SI = 0) Clear interrupt flag.
	VICVectAddr = 0;							// Acknowledge the interrupt, written value doesn`t matter
}

//------------------------------------------------------------------------------
