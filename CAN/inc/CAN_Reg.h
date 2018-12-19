/*****************************************************************************/
/*  This file is part of the uVision/ARM development tools                   */
/*  Copyright KEIL ELEKTRONIK GmbH 2002-2008                                 */
/*****************************************************************************/
/*                                                                           */
/*  CAN_reg.h:  Header file for CAN registers                                */
/*              Philips LPC2xxx                                              */
/*                                                                           */
/*****************************************************************************/


#ifndef __CAN_REG_H
#define __CAN_REG_H

#define CAN1_BASE        0xE0044000   /* CAN Controller 1 Base Address       */
#define CAN2_BASE        0xE0048000   /* CAN Controller 2 Base Address       */
#define CAN3_BASE        0xE004C000   /* CAN Controller 3 Base Address       */
#define CAN4_BASE        0xE0050000   /* CAN Controller 4 Base Address       */

/* CAN Controller Register Addresses                                         */
U32 CAN_BASE[] = { CAN1_BASE, CAN2_BASE, CAN3_BASE, CAN4_BASE };

#define CAN_AFRAM_BASE   0xE0038000   /* CAN Acceptance Filter RAM Base Address */
#define CAN_AF_BASE      0xE003C000   /* CAN Acceptance Filter Base Address  */
#define CAN_CR_BASE      0xE0040000   /* CAN Central Registers Base Address  */

/* CAN Acceptance Filter RAM                                                 */
typedef struct  {
  volatile U32 mask[512];             /* ID Masks                            */
}  regCAN_AFRAM;

/* Undefining names already defined in LPC21xx.h so they can be reused       */
#undef AFMR
#undef SFF_sa
#undef SFF_GRP_sa
#undef EFF_sa
#undef EFF_GRP_sa
#undef ENDofTable
#undef LUTerrAd
#undef LUTerr

/* CAN Acceptance Filter Registers (regCAN_AF)                               */
typedef struct  {
  volatile U32 AFMR;        /* Acceptance filter register                    */
  volatile U32 SFF_sa;      /* Standard frame individual start address register */
  volatile U32 SFF_GRP_sa;  /* Standard frame group start address register   */
  volatile U32 EFF_sa;      /* Extended frame individual start address register */
  volatile U32 EFF_GRP_sa;  /* Extended frame group start address register   */
  volatile U32 ENDofTable;  /* End of acceptance filter tables register      */
  volatile U32 LUTerrAd;    /* LUT error address register                    */
  volatile U32 LUTerr;      /* LUT error register                            */
}  regCAN_AF;

/* Undefining names already defined in LPC21xx.h so they can be reused       */
#undef CANTxSR
#undef CANRxSR
#undef CANMSR

/* CAN Central Registers (regCAN_CR)                                         */
typedef struct  {
  volatile U32 CANTxSR;     /* CAN central transmit status register          */
  volatile U32 CANRxSR;     /* CAN central receive status register           */
  volatile U32 CANMSR;      /* CAN central miscellaneous register            */
}  regCAN_CR;       

/* CAN Controller Registers  (regCAN)                                        */
typedef struct  {
  volatile U32 CANMOD;      /* Controls operating mode of CAN controller     */
  volatile U32 CANCMR;      /* Command bits that affect the state of CAN ctrlrs */
  volatile U32 CANGSR;      /* Global controller status and error counter    */
  volatile U32 CANICR;      /* Interrupt status, arb lost capture, error code */
  volatile U32 CANIER;      /* Interrupt enable                              */
  volatile U32 CANBTR;      /* Bus timing                                    */
  volatile U32 CANEWL;      /* Error warning limit                           */
  volatile U32 CANSR;       /* Status register                               */
  volatile U32 CANRFS;      /* Receive frame status                          */
  volatile U32 CANRID;      /* Received identifier                           */
  volatile U32 CANRDA;      /* Received data bytes 1 - 4                     */
  volatile U32 CANRDB;      /* Received data bytes 5 - 8                     */
  volatile U32 CANTF1;      /* Transmit frame info       (1)                 */
  volatile U32 CANTID1;     /* Transmit identifier       (1)                 */
  volatile U32 CANTDA1;     /* Transmit data bytes 1 - 4 (1)                 */
  volatile U32 CANTDB1;     /* Transmit data bytes 5 - 8 (1)                 */
  volatile U32 CANTF2;      /* Transmit frame info       (2)                 */
  volatile U32 CANTID2;     /* Transmit identifier       (2)                 */
  volatile U32 CANTDA2;     /* Transmit data bytes 1 - 4 (2)                 */
  volatile U32 CANTDB2;     /* Transmit data bytes 5 - 8 (2)                 */
  volatile U32 CANTF3;      /* Transmit frame info       (3)                 */
  volatile U32 CANTID3;     /* Transmit identifier       (3)                 */
  volatile U32 CANTDA3;     /* Transmit data bytes 1 - 4 (3)                 */
  volatile U32 CANTDB3;     /* Transmit data bytes 5 - 8 (3)                 */
}  regCAN;

/* Pointers to CAN Controllers Registers                                     */
#define ptrCAN1        ((regCAN *)CAN1_BASE)
#define ptrCAN2        ((regCAN *)CAN2_BASE)
#define ptrCAN3        ((regCAN *)CAN3_BASE)
#define ptrCAN4        ((regCAN *)CAN4_BASE)


#endif /* __CAN_REG_H */
