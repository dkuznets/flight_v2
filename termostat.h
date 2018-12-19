#ifndef	__TERMOSTAT__H
#define	__TERMOSTAT__H
//----------------------------------------------------------------------------

typedef enum
{
	tchCMOS1 = 0,
	tchCMOS2 = 1

} TERMOSTAT_CHANNEL;

//----------------------------------------------------------------------------

void termostat_Init(void);
void termostat_Enable(TERMOSTAT_CHANNEL channel, BIT enable);
float termostat_GetTemperature(TERMOSTAT_CHANNEL channel);

//----------------------------------------------------------------------------

#endif	// __TERMOSTAT__H
