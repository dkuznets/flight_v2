#ifndef	__I2C_MC__H
#define	__I2C_MC__H
//------------------------------------------------------------------------------

void mc_i2cInit(void);
BOOL mc_i2cWriteReg_0bit (unsigned char adr, unsigned char reg);
BOOL mc_i2cReadReg_16bit (unsigned char adr, unsigned char reg, unsigned short *val);
BOOL mc_i2cWriteReg_16bit(unsigned char adr, unsigned char reg, unsigned short  val);
BOOL mc_i2cReadReg_8bit  (unsigned char adr, unsigned char reg, unsigned char  *val);
BOOL mc_i2cWriteReg_8bit (unsigned char adr, unsigned char reg, unsigned char   val);
// multi-byte transfer functions
BOOL mc_i2cReadRegs_8bit (unsigned char adr, unsigned char start_reg, unsigned char reg_count /*1...255*/,       unsigned char *buffer);
BOOL mc_i2cWriteRegs_8bit(unsigned char adr, unsigned char start_reg, unsigned char reg_count /*1...254*/, const unsigned char *buffer);

//
// Note: all functions are thread-safe, i.e. they can be called from different tasks
// without danger of transfers get interrupted
//

//------------------------------------------------------------------------------

#endif	//__I2C_MC__H
