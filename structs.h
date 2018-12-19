#ifndef	__STRUCTS__H
#define	__STRUCTS__H
//----------------------------------------------------------------------------
#include "rect.h"
//----------------------------------------------------------------------------
#define	BAD_PIXELS_ARRAY_SIZE	256
//----------------------------------------------------------------------------

typedef __packed struct
{
	U16 x;
	U16 y;

} FIFO_ITEM;

typedef __packed struct
{
	U16       count;
	FIFO_ITEM array[BAD_PIXELS_ARRAY_SIZE];

} BAD_PIX_FILE_STRUCTURE;

//----------------------------------------------------------------------------
#define	SN_LENGTH				128
//----------------------------------------------------------------------------

typedef __packed struct
{
	U8    szSerialNumber[SN_LENGTH];// matrix serial number, zero-terminated string
	U16   nXc, nYc;					// centers of matrices, pix
	U16   nRs, nRb;					// lens field of view radius ('s'=small, 'b'=big), pix
	U8    cLc;						// neighbour distance for clasters, pix
	U16   nLimit;					// FIFO limit (10-bit)
	U16   nVinb, nVref;				// VINB & VREF (10-bit)
	float fA, fB, fC;				// Fi=A*x^2 + Bx + C

} CONFIG_FILE_STRUCTURE;

//----------------------------------------------------------------------------
#define	NO_MARKER				0	// 'no marker' cluster identifier
//----------------------------------------------------------------------------

typedef __packed struct
{
	FIFO_ITEM *pi;
	U8 *pm;

} THE_POINT;

typedef __packed struct
{
	U16      count;					// number of points (pixels) in cluster
	U8       excluded;				// flag, cluster should be ignored if TRUE
	U8       marker;				// cluster identicator
	ARM_RECT rect;					// rectangular area of cluster (in pixels)

} CLUSTER_COORD;

__inline void make_point(FIFO_ITEM *pi, U8 *pm, THE_POINT *point)
{
	point->pi = pi;
	point->pm = pm;
}

__inline BIT neighbour_points(const THE_POINT *p1, const THE_POINT *p2, U8 length)
{
	return (abs((S16)p2->pi->x - (S16)p1->pi->x) <= length) &&
		(abs((S16)p2->pi->y - (S16)p1->pi->y) <= length);
}

__inline BIT point_has_no_marker(const THE_POINT *p)
{
	return *(p->pm) == NO_MARKER;
}

__inline void copy_marker(THE_POINT *dst, const THE_POINT *src)
{
	*(dst->pm) = *(src->pm);
}

//----------------------------------------------------------------------------

typedef __packed struct
{
	U32 time;						// time of alarm, in 10us units
	S16 angle1;						//
	S16 angle2;						//

} COORDS_DATA;

#define	REASON_BY_REQUEST		0x0	// output status message initiated by host request
#define	REASON_BY_TIMER			0x1	// output status message initiated by timer event
#define	REASON_BY_STATE			0x2	// output status message initiated by state change

#define	MODE_OPERATIONAL		0x1	// normal mode
#define	MODE_PROGRAMMING		0x2	// device is being programmed

typedef __packed struct
{
	__packed struct
	{
		U8 reason    : 4;			// status transmittion reason (one of REASON_xxx)
		U8 status_ok : 1;			// common device status (1=device OK, 0=device failed)
		U8 mode      : 2;			// current device mode (1=operational, 2=programming)
		U8 reserved  : 1;			// not used

	} bits1;

	U8 soer_mode;					// SOER mode

	__packed struct
	{
		U8 plis_loaded_ok	: 1;
		U8 files_loaded_ok	: 1;
		U8 reserved			: 6;

	} bits2;

	S8 t1;							// temperature of ARM (external sensor)
	S8 t2;							// temperature of CMOS1 video matrix
	S8 t3;							// temperature of CMOS2 video matrix
	U8 reserved[2];					// not used

} STATUS_DATA;

typedef __packed struct
{
	float frequency;				// status auto-transmittion frequency (0=transmit once)

	U8    use_frequency : 1;		// 0=ignore frequency (transmit once), 1=auto-transmit
	U8    auto_status   : 1;		// 1=auto-transmit status if device status changed
	U8    reserved      : 6;		// not used

} STATUS_PARAMS;

typedef __packed struct
{
	U64 time_10us;

} TIME_SYNC_DATA;

//----------------------------------------------------------------------------

#endif	// __STRUCTS__H
