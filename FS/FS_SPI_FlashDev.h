/***********************************************************************/
/*  This file is part of the ARM Toolchain package                     */
/*  Copyright KEIL ELEKTRONIK GmbH 2003 - 2007                         */
/***********************************************************************/
/*                                                                     */
/*  FlashDev.H:  Device Description for Intel  8MBit Serial Flash      */
/*                                                   Memory            */
/*                                                                     */
/***********************************************************************/

#define SPI_FLASH_DEVICE                             \
  DSB(0x10000, 0x000000),     /* Sector Size 64kB */ \
  DSB(0x10000, 0x010000),     /* Sector Size 64kB */ \
  DSB(0x10000, 0x020000),     /* Sector Size 64kB */ \
  DSB(0x10000, 0x030000),     /* Sector Size 64kB */ \
  DSB(0x10000, 0x040000),     /* Sector Size 64kB */ \
  DSB(0x10000, 0x050000),     /* Sector Size 64kB */ \
  DSB(0x10000, 0x060000),     /* Sector Size 64kB */ \
  DSB(0x10000, 0x070000),     /* Sector Size 64kB */ \
  DSB(0x10000, 0x080000),     /* Sector Size 64kB */ \
  DSB(0x10000, 0x090000),     /* Sector Size 64kB */ \
  DSB(0x10000, 0x0A0000),     /* Sector Size 64kB */ \
  DSB(0x10000, 0x0B0000),     /* Sector Size 64kB */ \
  DSB(0x10000, 0x0C0000),     /* Sector Size 64kB */ \
  DSB(0x10000, 0x0D0000),     /* Sector Size 64kB */ \
  DSB(0x10000, 0x0E0000),     /* Sector Size 64kB */ \
  DSB(0x10000, 0x0F0000),     /* Sector Size 64kB */ \

#define SF_NSECT    16
