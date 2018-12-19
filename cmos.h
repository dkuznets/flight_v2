#ifndef	__CMOS__H
#define	__CMOS__H
//----------------------------------------------------------------------------
#include "regs.h"
//----------------------------------------------------------------------------

// Prepare video RAM for write operation (from ARM software)
#define	CMOS_PREPARE_FOR_WRITE()							\
{															\
	CMOS1_RKS_MEM_REG &= ~MEM_RAM_TO_CMOS_BIT;				\
	CMOS1_RKS_MEM_REG |=  MEM_RESET_ADDR_BIT;				\
	CMOS1_RKS_MEM_REG &= ~MEM_RESET_ADDR_BIT;				\
	CMOS1_RKS_MEM_REG |=  MEM_WRITE_AUTO_INC_BIT;			\
															\
	CMOS2_RKS_MEM_REG &= ~MEM_RAM_TO_CMOS_BIT;				\
	CMOS2_RKS_MEM_REG |=  MEM_RESET_ADDR_BIT;				\
	CMOS2_RKS_MEM_REG &= ~MEM_RESET_ADDR_BIT;				\
	CMOS2_RKS_MEM_REG |=  MEM_WRITE_AUTO_INC_BIT;			\
}

// Prepare video RAM for read operation (from ARM software)
#define	CMOS_PREPARE_FOR_READ()								\
{															\
	CMOS1_RKS_MEM_REG &= ~MEM_RAM_TO_CMOS_BIT;				\
	CMOS1_RKS_MEM_REG |=  MEM_RESET_ADDR_BIT;				\
	CMOS1_RKS_MEM_REG &= ~MEM_RESET_ADDR_BIT;				\
	CMOS1_RKS_MEM_REG |=  MEM_READ_AUTO_INC_BIT;			\
															\
	CMOS2_RKS_MEM_REG &= ~MEM_RAM_TO_CMOS_BIT;				\
	CMOS2_RKS_MEM_REG |=  MEM_RESET_ADDR_BIT;				\
	CMOS2_RKS_MEM_REG &= ~MEM_RESET_ADDR_BIT;				\
	CMOS2_RKS_MEM_REG |=  MEM_READ_AUTO_INC_BIT;			\
}

// Clear FIFO for both matrices
#define	CMOS_CLEAR_FIFO()									\
{															\
	while(!(CMOS1_RKS_RAM_REG & RAM_FIFO_EMPTY_BIT))		\
		CMOS1_RKS_RAM_REG |= RAM_READ_FIFO_BIT;				\
   	while(!(CMOS2_RKS_RAM_REG & RAM_FIFO_EMPTY_BIT))		\
		CMOS2_RKS_RAM_REG |= RAM_READ_FIFO_BIT;				\
}

// Captures video frame data into RAM
#define	CMOS_CAPTURE_FRAME()								\
{															\
	CMOS1_RKS_MEM_REG |= MEM_RAM_TO_CMOS_BIT;				\
	CMOS2_RKS_MEM_REG |= MEM_RAM_TO_CMOS_BIT;				\
															\
	CMOS1_RKS_CMD_REG = CMD_START_CAPTURE_BIT;				\
	CMOS1_RKS_CMD_REG = 0;									\
}

// Captures video frame data into RAM for both matrices
#define	CMOS_SIMULATE_SHOT()								\
{															\
	os_dly_wait(1);											\
															\
	CMOS1_RKS_CMD_REG =  CMD_FIRE_A_SHOT_BIT;				\
	CMOS1_RKS_CMD_REG = 0;									\
	CMOS2_RKS_CMD_REG =  CMD_FIRE_A_SHOT_BIT;				\
	CMOS2_RKS_CMD_REG = 0;									\
}

// 
#define	CMOS_PREPARE_TIMER_FIFO_READ()						\
{															\
	CMOS1_RKS_CMD_REG = CMD_TIMER_READ_FIFO_BIT;			\
	CMOS1_RKS_CMD_REG = 0;									\
	while(CMOS1_RKS_CMD_REG & CMD_TIMER_FIFO_BUSY_BIT);		\
}

// Reads video frame data byte for specified CMOS matrix
#define	CMOS_READ_RAM_BYTE(n)		(CMOS##n##_DAT_REG)

//----------------------------------------------------------------------------

#define	CMOS_MEMORY_SIZE			(512*1024L)

//----------------------------------------------------------------------------

#endif	// __CMOS__H
