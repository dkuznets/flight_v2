#ifndef	__DAC__H
#define	__DAC__H

//----------------------------------------------------------------------------

BOOL dac_GetA(unsigned short *value);
BOOL dac_GetB(unsigned short *value);
BOOL dac_GetC(unsigned short *value);
BOOL dac_GetD(unsigned short *value);

BOOL dac_SetA(unsigned short value);
BOOL dac_SetB(unsigned short value);
BOOL dac_SetC(unsigned short value);
BOOL dac_SetD(unsigned short value);

//----------------------------------------------------------------------------

#endif	// __DAC__H
