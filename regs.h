#ifndef	__REGS__H
#define	__REGS__H
//----------------------------------------------------------------------------

// CMOS1 (PLIS) registers definition
#define	CMOS1_RKS_CMD_REG			(*((volatile unsigned char *)0x82000000))	// command register
#define	CMOS1_RKS_MEM_REG			(*((volatile unsigned char *)0x82000001))	// video memory register
#define	CMOS1_RKS_RAM_REG			(*((volatile unsigned char *)0x82000012))	// FIFO RAM	register
#define	CMOS1_DAT_REG				(*((volatile unsigned char *)0x82000002))	// data register
#define	CMOS1_TIMER_REG				(*((volatile unsigned long *)0x82000004))	// timer register
#define	CMOS1_POROG_REG				(*((volatile unsigned short*)0x82000008))	// limit register
#define	CMOS1_FIFO_TIMER_REG		(*((volatile unsigned short*)0x8200000A))	// FIFO timer register
#define	CMOS1_FIFO_Y_REG			(*((volatile unsigned short*)0x8200000E))	// Y-coordinate register
#define	CMOS1_FIFO_X_REG			(*((volatile unsigned short*)0x82000010))	// X-coordinate register

// CMOS2 (PLIS) registers definition
#define	CMOS2_RKS_CMD_REG			(*((volatile unsigned char *)0x83000000))	// command register
#define	CMOS2_RKS_MEM_REG			(*((volatile unsigned char *)0x83000001))	// video memory register
#define	CMOS2_RKS_RAM_REG			(*((volatile unsigned char *)0x83000012))	// FIFO RAM	register
#define	CMOS2_DAT_REG				(*((volatile unsigned char *)0x83000002))	// data register
#define	CMOS2_POROG_REG				(*((volatile unsigned short*)0x83000008))	// limit register
#define	CMOS2_FIFO_Y_REG			(*((volatile unsigned short*)0x8300000E))	// Y-coordinate register
#define	CMOS2_FIFO_X_REG			(*((volatile unsigned short*)0x83000010))	// X-coordinate register

//----------------------------------------------------------------------------

// CMOSx_RKS_MEM_REG bits definition
#define	MEM_RAM_TO_CMOS_BIT			(1<<0)	// 1-switch video memory to CMOS, 0-switch video memory to CPU (ARM)
#define	MEM_READ_AUTO_INC_BIT		(1<<1)	// 1-enable auto-incrementation mode for read operations
#define	MEM_WRITE_AUTO_INC_BIT		(1<<2)	// 1-enable auto-incrementation mode for write operations
#define	MEM_RESET_ADDR_BIT			(1<<7)	// 1-reset read/write address, 0-continue working

//----------------------------------------------------------------------------

// CMOSx_RKS_CMD_REG bits definition
#define	CMD_START_CAPTURE_BIT		(1<<0)	// start capture video image (1->0 sequence)
#define	CMD_FIRE_A_SHOT_BIT			(1<<1)	// one shot simulation (1->0 sequence)
#define	CMD_TIMER_FIFO_FULL_BIT		(1<<2)	// 1-timer fifo is full, 0-not full
#define	CMD_TIMER_FIFO_EMPTY_BIT	(1<<3)	// 1-timer fifo is empty, 0-not empty
#define	CMD_TIMER_READ_FIFO_BIT		(1<<4)	// 1->0 sequence
#define	CMD_TIMER_FIFO_BUSY_BIT		(1<<5)	// 1-fifo busy, 0-fifo ready
#define	CMD_TIMER_RESET_BIT			(1<<6)	// write only: 1->0 to reset timer
#define	CMD_TIMER_CAPTURED_BIT		CMD_TIMER_RESET_BIT	// read only: 1-shot captured, 0-shot not captured
#define	CMD_TIMER_FIFO_READY_BIT	(1<<7)	// 1-ready, 0-not ready

// CMOSx_RKS_RAM_REG bits definition
#define	RAM_FIFO_FULL_BIT			(1<<2)	// 1-full, 0-not full
#define	RAM_FIFO_EMPTY_BIT			(1<<3)	// 1-empty, 0-not empty
#define	RAM_READ_FIFO_BIT			(1<<4)	// 1->0 sequence
#define	RAM_FIFO_BUSY_BIT			(1<<5)	// 1-fifo busy, 0-fifo ready

//----------------------------------------------------------------------------
#endif	// __REGS__H
