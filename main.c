#include <LPC22xx.h>
#include <RTL.h>
#include <STDLIB.h>
#include <STRING.h>
#include <STDIO.h>
#include <MATH.h>
#include "upload.h"
#include "RTX_CAN.h"
#include "CAN_EX.h"
#include "termometer.h"
#include "termostat.h"
#include "i2c_mc.h"
#include "dac.h"
#include "cmos.h"
#include "rtc_svc.h"
#include "structs.h"
#include "common.h"
#include "ver.h"

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
// pause needed for JTAG capturing at CPU startup
#define	DEBUG_STARTUP_PAUSE()				\
{											\
	U32 i;									\
	for(i = 0; i < 1000000; i++);			\
}
//----------------------------------------------------------------------------
#define	M_PI								3.14159265358979323846
//----------------------------------------------------------------------------
// CAN device ID
#ifdef	THIS_DEVICE_IS_LEFT
#define	CAN_DEV_ID							0x11	// SOLO device ID for left wing
#else
#define	CAN_DEV_ID							0x12	// SOLO device ID for right wing
#endif
// CAN individual message IDs (input for ARM)
#define	CAN_MSG_ID_IN_RESET					((0x01 << 5) | CAN_DEV_ID)
#define	CAN_MSG_ID_IN_CLOCK_SYNC			((0x02 << 5) | CAN_DEV_ID)
#define	CAN_MSG_ID_IN_GET_STATUS			((0x04 << 5) | CAN_DEV_ID)
#define	CAN_MSG_ID_IN_SOER_MODE				((0x07 << 5) | CAN_DEV_ID)
// CAN broadcast message IDs (input for ARM)
#define	CAN_MSG_ID_IN_BROADCAST_RESET		((0x01 << 5) | 0x00)
#define	CAN_MSG_ID_IN_BROADCAST_CLOCK_SYNC	((0x02 << 5) | 0x00)
#define	CAN_MSG_ID_IN_BROADCAST_GET_STATUS	((0x04 << 5) | 0x00)
#define	CAN_MSG_ID_IN_BROADCAST_SOER_MODE	((0x07 << 5) | 0x00)
// CAN message IDs (output for ARM)
#define	CAN_MSG_ID_OUT_STATUS				((0x05 << 5) | CAN_DEV_ID)
#define	CAN_MSG_ID_OUT_COORDS				((0x2D << 5) | CAN_DEV_ID)
// CAN controller definitions
#define	CAN_CTRL							1		// CAN1 controller
#define	CAN_RD_TIMEOUT						100		// 1 sec CAN R/W timeout
#define	CAN_WR_TIMEOUT						10		// 0.1 sec CAN R/W timeout
#define	CAN_BAUDRATE						500000	// 500 kbps CAN bitrate
// inter-task event codes
#define	EVT_SEND_STATUS_BY_REQUEST			0x0001
#define	EVT_SEND_STATUS_BY_TIMER			0x0002
#define	EVT_SEND_STATUS_BY_STATE			0x0004
#define	EVT_SEND_ALARM						0x0008
//----------------------------------------------------------------------------
// FIFO coordinate XY-correction
#define	FIFO_X_OFFSET						-11
#define	FIFO_Y_OFFSET						-4
//---------------------------------------------------------------------------
// CMOS matrix definitions
#define	IMAGE_CX							(320-1)
#define	IMAGE_CY							(256-1)
#define	CELL_SIZE							8		// cluster cell size, in pix
#define	IMAGE_CELL_CX						((IMAGE_CX + CELL_SIZE - 1) / CELL_SIZE)
#define	IMAGE_CELL_CY						((IMAGE_CY + CELL_SIZE - 1) / CELL_SIZE)
//----------------------------------------------------------------------------
// thermostat definitions
#define	THERMOSTAT_AVARAGE_COUNT			100
#define	THERMOSTAT_T						-20.0	// in Celsius degree
#define	THERMOSTAT_DT						5.0		// (+-) in Celsius degree
//----------------------------------------------------------------------------
#define	COORDS_FIFO_SIZE					16		// maximum number of items in coordinates FIFO array
//----------------------------------------------------------------------------
#define	ENTER_CRITICAL_SECTION()			os_mut_wait(g_mutex, 0xffff)
#define	LEAVE_CRITICAL_SECTION()			os_mut_release(g_mutex)
//----------------------------------------------------------------------------
// constant variables
static const char							g_badpix_filename1[] = "S:\\bad_pix1.raw";
static const char							g_badpix_filename2[] = "S:\\bad_pix2.raw";
static const char							g_config_filename1[] = "S:\\config1.raw";
static const char							g_config_filename2[] = "S:\\config2.raw";
//----------------------------------------------------------------------------
// common inter-task variables (access-protected by mutex)
static OS_MUT								g_mutex;
static STATUS_DATA							g_status;
static STATUS_PARAMS						g_status_params;
static U64									g_init_time_10us;
static COORDS_DATA							g_coords[COORDS_FIFO_SIZE];	// coordinates FIFO array
static U16									g_coords_index = 0;			// index of 1-st free item in array
//----------------------------------------------------------------------------
// transmittion task CAN id
static OS_TID								g_task_id;
//----------------------------------------------------------------------------
// bad pixels & config variables
static BAD_PIX_FILE_STRUCTURE				g_bad_pixels1;				// bad pixel array for CMOS1
static BAD_PIX_FILE_STRUCTURE				g_bad_pixels2;				// bad pixel array for CMOS2
static CONFIG_FILE_STRUCTURE				g_config1;					// config settings for CMOS1
static CONFIG_FILE_STRUCTURE				g_config2;					// config settings for CMOS2
// shot fifo & cluster variables
static FIFO_ITEM							g_shot_array1[4096];		// shot XY-coordinates from FIFO1
static FIFO_ITEM							g_shot_array2[4096];		// shot XY-coordinates from FIFO2
static U16									g_shot_count1;				// number of shot XY-coordinate pairs in FIFO1
static U16									g_shot_count2;				// number of shot XY-coordinate pairs in FIFO2
static CLUSTER_COORD						g_cluster_coords1[IMAGE_CELL_CX*IMAGE_CELL_CY];	// clusters mass centre & coordinates in FIFO1
static CLUSTER_COORD						g_cluster_coords2[IMAGE_CELL_CX*IMAGE_CELL_CY];	// clusters mass centre & coordinates in FIFO2
static CLUSTER_COORD						*g_cluster_ptrs1[IMAGE_CELL_CX*IMAGE_CELL_CY];	//
static CLUSTER_COORD						*g_cluster_ptrs2[IMAGE_CELL_CX*IMAGE_CELL_CY];	//
static U16									g_cluster_count1;			// number of clusters (8x8) in FIFO1
static U16									g_cluster_count2;			// number of clusters (8x8) in FIFO2
static CLUSTER_COORD						g_super_cluster_coords1[IMAGE_CELL_CX*IMAGE_CELL_CY];
static CLUSTER_COORD						g_super_cluster_coords2[IMAGE_CELL_CX*IMAGE_CELL_CY];
static U16									g_super_cluster_count1;		// number of super clusters in FIFO1
static U16									g_super_cluster_count2;		// number of super clusters in FIFO2
//----------------------------------------------------------------------------
// task definitions
__task void task_Init(void);
__task void task_Alarm(void);
__task void task_CAN_Receive(void);
__task void task_CAN_Send(void);
__task void task_AutoStatusTimer(void);
__task void task_StatusMonitor(void);
//----------------------------------------------------------------------------
// bad pixel files helpers
static BIT load_bad_pixel_files(void);
static __weak BIT update_bad_pixel_file1(void);		// function not used
static __weak BIT update_bad_pixel_file2(void);		// function not used
// configuration files helpers
static BIT load_config_files(void);
static __weak BIT update_config_file1(void);		// function not used
static __weak BIT update_config_file2(void);		// function not used
//----------------------------------------------------------------------------
// common helpers
static BIT  load_application_settings(void);
static void store_shot_array1(BIT use_extended_field_of_view);
static void store_shot_array2(BIT use_extended_field_of_view);
static void build_clusters_array1(void);
static void build_clusters_array2(void);
static void exclude_intersected_clusters_from_array1(void);
static void exclude_intersected_clusters_from_array2(void);
static void enlarge_clusters_in_array1(void);
static void enlarge_clusters_in_array2(void);
static BIT  calculate_mass_centre_by_array1(float *px, float *py);
static BIT  calculate_mass_centre_by_array2(float *px, float *py);
static void calculate_angles(float x, float y, S16 *pangle1, S16 *pangle2);
static void bad_pixels_dynamic_calibration(void);
static U64  get_time_10us(void);
static BIT  calculate_mass_centre_1(void);
static BIT  calculate_mass_centre_2(void);
//----------------------------------------------------------------------------
// shot coordinates fifo helpers
static BIT  push_coords_item(const COORDS_DATA *coords);
static BIT  pop_coords_item(COORDS_DATA *coords);
static U16  get_coords_count(void);
//----------------------------------------------------------------------------
static BOOL CAN_safe_send(CAN_msg *msg, U16 timeout);
static BOOL CAN_safe_receive(CAN_msg *msg, U16 timeout);
static CAN_ERROR CAN_init_controller(void);
//----------------------------------------------------------------------------
//
// Entry-point of the RTX-application
//

int main(void)
{
	DEBUG_STARTUP_PAUSE();

	os_sys_init(task_Init);
}

//----------------------------------------------------------------------------
//
// Task initializes variables & creates other application tasks
//

__task void task_Init(void)
{
	BIT ok1 = __FALSE;
	BIT ok2;

	// initialize & upload PLIS
	if(plis_Init())
		ok1 = plis_Upload();

	rtc_Init();
	mc_i2cInit();
	termo_Init();	// must be called after mc_i2cInit()
	termostat_Init();

	// load bad pixels arrays & config files
	ok2 = load_application_settings();

	// initialize CAN controller
	CAN_init_controller();

	memset(&g_status       , 0, sizeof(g_status       ));
	memset(&g_status_params, 0, sizeof(g_status_params));
	// set initial device status bits
	g_status.bits2.plis_loaded_ok = (ok1 ? 1 : 0);
	g_status.bits2.files_loaded_ok = (ok2 ? 1 : 0);
	g_status.bits1.status_ok = (ok1 && ok2 ? 1 : 0);

	// reset initial time variable
	g_init_time_10us = 0;

	os_mut_init(g_mutex);

	g_task_id = os_tsk_create(task_CAN_Send, 1);
	os_tsk_create(task_CAN_Receive, 1);
	os_tsk_create(task_AutoStatusTimer, 1);
	os_tsk_create(task_StatusMonitor, 1);
	os_tsk_create(task_Alarm, 1);

	os_tsk_delete_self();
}

//----------------------------------------------------------------------------
//
// Task monitors laser shots and initiates alarm & coordinate events
//

__task void task_Alarm(void)
{
	U32         i;
	COORDS_DATA coords;
	U32         start_time = rtc_GetTickCount();
	U32         cur_time;
//	S16         angle1, angle2;
//	float       x = 0, y = 0;
	BIT         ok;
	BIT         second_pass_flag;

	while(1)
	{
		CMOS_CLEAR_FIFO();
		CMOS_CAPTURE_FRAME();

		do
		{
			if(!(CMOS1_RKS_CMD_REG & CMD_TIMER_FIFO_EMPTY_BIT))
			{
				CMOS_PREPARE_TIMER_FIFO_READ();

				coords.time = (U32)get_time_10us();
				coords.angle1 = 0x7fff;
				coords.angle2 = 0x7fff;

				// enough empty space in stack?
				if((COORDS_FIFO_SIZE - get_coords_count()) >= 2)
				{
					push_coords_item(&coords);
					os_evt_set(EVT_SEND_ALARM, g_task_id);
				}
			}

			cur_time = rtc_GetTickCount();

			if((cur_time - start_time) >= 60000 &&	// 1 min timeout
				CMOS1_RKS_CMD_REG & CMD_TIMER_FIFO_EMPTY_BIT)
			{
				bad_pixels_dynamic_calibration();

				start_time = cur_time;
			}

			os_tsk_pass();

		} while(!(CMOS1_RKS_CMD_REG & CMD_TIMER_FIFO_READY_BIT));

		// reset second-pass flag
		second_pass_flag = __FALSE;

		for(i = 0; i < 2; i++)
		{
			BIT extended_field_of_view = second_pass_flag;

			store_shot_array1(extended_field_of_view);
			store_shot_array2(extended_field_of_view);

			build_clusters_array1();
			build_clusters_array2();

			if(CMOS1_RKS_CMD_REG & CMD_TIMER_CAPTURED_BIT)	// CMOS1 has captured a shot
			{
				enlarge_clusters_in_array2();
				exclude_intersected_clusters_from_array1();
//				ok = calculate_mass_centre_by_array1(&x, &y);
				ok = calculate_mass_centre_1();
			}
			else											// CMOS2 has captured a shot
			{
				enlarge_clusters_in_array1();
				exclude_intersected_clusters_from_array2();
//				ok = calculate_mass_centre_by_array2(&x, &y);
				ok = calculate_mass_centre_2();
			}

			if(ok)
			{
				// XY-coordinates have been successfully calculated,
				// now calculate angles
/*
				calculate_angles(x, y, &angle1, &angle2);

				coords.time = (U32)get_time_10us();
				coords.angle1 = angle1;
				coords.angle2 = angle2;

				push_coords_item(&coords);
*/
				break;
			}

			// if couldn't calculate mass center, try to do it again using
			// extended field of view in 2-nd processing pass
			second_pass_flag = __TRUE;
		}

		os_evt_set(EVT_SEND_ALARM, g_task_id);
	}
}

//----------------------------------------------------------------------------
//
// Task receives CAN messages & processes them according to the protocol
//

__task void task_CAN_Receive(void)
{
	CAN_msg msg;
	STATUS_PARAMS *psp = (STATUS_PARAMS *)msg.data;
	TIME_SYNC_DATA *ptsd = (TIME_SYNC_DATA *)msg.data;

	while(1)
	{
		if(!CAN_safe_receive(&msg, CAN_RD_TIMEOUT))
			continue;

		switch(msg.id)
		{
			case CAN_MSG_ID_IN_BROADCAST_RESET:
			case CAN_MSG_ID_IN_RESET:
				{
					disable_interrupts();

					// initiate RESET using watchdog
					WDTC	= 0xFF;					// smallest available counter value
					WDMOD	= 0x03;					// ENABLE + RESET
					WDFEED	= 0xAA;					// send 1-st feed sequence...
					WDFEED	= 0x55;					// ...
					while(1);						// and wait for soon RESET
				}

			case CAN_MSG_ID_IN_BROADCAST_CLOCK_SYNC:
			case CAN_MSG_ID_IN_CLOCK_SYNC:
				{
					ENTER_CRITICAL_SECTION();
					g_init_time_10us = ptsd->time_10us;
					LEAVE_CRITICAL_SECTION();
				}
				break;

			case CAN_MSG_ID_IN_BROADCAST_GET_STATUS:
			case CAN_MSG_ID_IN_GET_STATUS:
				{
					// update global variable
					ENTER_CRITICAL_SECTION();
					g_status_params = *psp;
					LEAVE_CRITICAL_SECTION();

					if(psp->use_frequency == 0)
						os_evt_set(EVT_SEND_STATUS_BY_REQUEST, g_task_id);
				}
				break;

			case CAN_MSG_ID_IN_BROADCAST_SOER_MODE:
			case CAN_MSG_ID_IN_SOER_MODE:
				{
					// update global variable
					ENTER_CRITICAL_SECTION();
					g_status.soer_mode = msg.data[0];
					LEAVE_CRITICAL_SECTION();
				}
				break;

			default:
				// ignore unknown input IDs
				break;
		}
	}
}

//----------------------------------------------------------------------------
//
// Task receives in-application events & transfers CAN messages to the host
//

__task void task_CAN_Send(void)
{
	U16 evt;
	CAN_msg msg;
	STATUS_DATA *psd = (STATUS_DATA *)msg.data;
	COORDS_DATA *pcd = (COORDS_DATA *)msg.data;

	while(1)
	{
		os_evt_wait_or(0xffff, 0xffff);
		evt = os_evt_get();

		switch(evt)
		{
			case EVT_SEND_STATUS_BY_REQUEST:
				{
					memset(&msg, 0, sizeof(msg));
					msg.id					= CAN_MSG_ID_OUT_STATUS;
					msg.len					= sizeof(STATUS_DATA);
					msg.ch					= 0;	// channel not used
					msg.format				= STANDARD_FORMAT;
					msg.type				= DATA_FRAME;
					// copy status from global variable
					ENTER_CRITICAL_SECTION();
					*psd					= g_status;
					LEAVE_CRITICAL_SECTION();
					// update status transmittion reason
					psd->bits1.reason		= REASON_BY_REQUEST;
					// Fix error
					psd->bits1.mode = MODE_OPERATIONAL;
					CAN_safe_send(&msg, CAN_WR_TIMEOUT);
				}
				break;

			case EVT_SEND_STATUS_BY_TIMER:
				{
					memset(&msg, 0, sizeof(msg));
					msg.id					= CAN_MSG_ID_OUT_STATUS;
					msg.len					= sizeof(STATUS_DATA);
					msg.ch					= 0;	// channel not used
					msg.format				= STANDARD_FORMAT;
					msg.type				= DATA_FRAME;
					// copy status from global variable
					ENTER_CRITICAL_SECTION();
					*psd					= g_status;
					LEAVE_CRITICAL_SECTION();
					// update status transmittion reason
					psd->bits1.reason		= REASON_BY_TIMER;
					// Fix error
					psd->bits1.mode = MODE_OPERATIONAL;
					CAN_safe_send(&msg, CAN_WR_TIMEOUT);
				}
				break;

			case EVT_SEND_STATUS_BY_STATE:
				{
					memset(&msg, 0, sizeof(msg));
					msg.id					= CAN_MSG_ID_OUT_STATUS;
					msg.len					= sizeof(STATUS_DATA);
					msg.ch					= 0;	// channel not used
					msg.format				= STANDARD_FORMAT;
					msg.type				= DATA_FRAME;
					// copy status from global variable
					ENTER_CRITICAL_SECTION();
					*psd					= g_status;
					LEAVE_CRITICAL_SECTION();
					// update status transmittion reason
					psd->bits1.reason		= REASON_BY_STATE;

					CAN_safe_send(&msg, CAN_WR_TIMEOUT);
				}
				break;

			case EVT_SEND_ALARM:
				{
					memset(&msg, 0, sizeof(msg));
					msg.id					= CAN_MSG_ID_OUT_COORDS;
					msg.len					= sizeof(COORDS_DATA);
					msg.ch					= 0;	// channel not used
					msg.format				= STANDARD_FORMAT;
					msg.type				= DATA_FRAME;

/*
					if(pop_coords_item(pcd))
						CAN_safe_send(&msg, CAN_WR_TIMEOUT);
*/
					while(pop_coords_item(pcd))
						CAN_safe_send(&msg, CAN_WR_TIMEOUT);
				}
				break;

			default:
				// ignore unknown events
				break;
		}
	}
}

//----------------------------------------------------------------------------
//
// Task monitors device status & updates global status variable
//

__task void task_StatusMonitor(void)
{
	float tmp, mc_t, t1, t2;
	float summ1 = THERMOSTAT_AVARAGE_COUNT * termostat_GetTemperature(tchCMOS1);
	float summ2 = THERMOSTAT_AVARAGE_COUNT * termostat_GetTemperature(tchCMOS2);
	BIT   status_changed, auto_status;

	while(1)
	{
		termo_GetTemperature(&mc_t);

		tmp = termostat_GetTemperature(tchCMOS1);
		summ1 -= (summ1 / THERMOSTAT_AVARAGE_COUNT) - tmp;
		t1 = summ1 / THERMOSTAT_AVARAGE_COUNT;

		tmp = termostat_GetTemperature(tchCMOS2);
		summ2 -= (summ2 / THERMOSTAT_AVARAGE_COUNT) - tmp;
		t2 = summ2 / THERMOSTAT_AVARAGE_COUNT;

		if(t1 > THERMOSTAT_T + THERMOSTAT_DT) termostat_Enable(tchCMOS1, __TRUE );
		if(t1 < THERMOSTAT_T - THERMOSTAT_DT) termostat_Enable(tchCMOS1, __FALSE);
		if(t2 > THERMOSTAT_T + THERMOSTAT_DT) termostat_Enable(tchCMOS2, __TRUE );
		if(t2 < THERMOSTAT_T - THERMOSTAT_DT) termostat_Enable(tchCMOS2, __FALSE);

		ENTER_CRITICAL_SECTION();
		// update temperatures in global variable
		g_status.t1 = (S8)mc_t;
		g_status.t2 = (S8)t1;
		g_status.t3 = (S8)t2;
		// read out current auto-status flag
		auto_status = g_status_params.auto_status;
		LEAVE_CRITICAL_SECTION();

		//
		// TO DO (check if status changed)
		status_changed = 0;
		// TO DO (check if status changed)
		//

		if(status_changed == __TRUE && auto_status == __TRUE)
			os_evt_set(EVT_SEND_STATUS_BY_STATE, g_task_id);

		os_dly_wait(10);	// delay 100 ms
	}
}

//----------------------------------------------------------------------------
//
// Task is used to periodicaly auto-transfer status message to the host
// without request but using timer interval
//

__task void task_AutoStatusTimer(void)
{
	STATUS_PARAMS params;
	U16 count = 0;
	U16 delay;	// in 10ms units

	while(1)
	{
		os_dly_wait(1);	// wait 10 ms

		ENTER_CRITICAL_SECTION();
		params = g_status_params;
		LEAVE_CRITICAL_SECTION();

		if(params.use_frequency == __FALSE || params.frequency == 0.0)
		{
			count = 0;
			continue;
		}

		delay = (U16)(1000.0 / params.frequency / 10.0 + 0.5);

		if(delay <     1)	delay =     1;	// 0.01 sec
		if(delay > 10000)	delay = 10000;	// 100 sec

		if((++count) >= delay)
		{
			count = 0;

			os_evt_set(EVT_SEND_STATUS_BY_TIMER, g_task_id);
		}
	}
}

//----------------------------------------------------------------------------

static BIT load_bad_pixel_files(void)
{
	FILE *f;

	f = fopen(g_badpix_filename1, "rb");	// 'read binary' mode 
	if(f == NULL)
		return __FALSE;
	if(fread(&g_bad_pixels1, 1, sizeof(g_bad_pixels1), f) != sizeof(g_bad_pixels1))
	{
		fclose(f);
		return __FALSE;
	}
	fclose(f);

	f = fopen(g_badpix_filename2, "rb");	// 'read binary' mode
	if(f == NULL)
		return __FALSE;
	if(fread(&g_bad_pixels2, 1, sizeof(g_bad_pixels2), f) != sizeof(g_bad_pixels2))
	{
		fclose(f);
		return __FALSE;
	}
	fclose(f);

	return __TRUE;
}

//----------------------------------------------------------------------------

static __weak BIT update_bad_pixel_file1(void)
{
	FILE *f;

	if(fdelete(g_badpix_filename1) != 0)
	{
		// nothing to delete
	}

	f = fopen(g_badpix_filename1, "wb");
	if(f == NULL)
		return __FALSE;

	if(fwrite(&g_bad_pixels1, 1, sizeof(g_bad_pixels1), f) != sizeof(g_bad_pixels1))
	{
		fclose(f);
		return __FALSE;
	}

	fflush(f);
	fclose(f);

	return __TRUE;
}

//----------------------------------------------------------------------------

static __weak BIT update_bad_pixel_file2(void)
{
	FILE *f;

	if(fdelete(g_badpix_filename2) != 0)
	{
		// nothing to delete
	}

	f = fopen(g_badpix_filename2, "wb");
	if(f == NULL)
		return __FALSE;

	if(fwrite(&g_bad_pixels2, 1, sizeof(g_bad_pixels2), f) != sizeof(g_bad_pixels2))
	{
		fclose(f);
		return __FALSE;
	}

	fflush(f);
	fclose(f);

	return __TRUE;
}

//----------------------------------------------------------------------------

static BIT load_config_files(void)
{
	FILE *f;

	f = fopen(g_config_filename1, "rb");	// 'read binary' mode
	if(f == NULL)
		return __FALSE;
	if(fread(&g_config1, 1, sizeof(g_config1), f) != sizeof(g_config1))
	{
		fclose(f);
		return __FALSE;
	}
	fclose(f);

	f = fopen(g_config_filename2, "rb");	// 'read binary' mode
	if(f == NULL)
		return __FALSE;
	if(fread(&g_config2, 1, sizeof(g_config2), f) != sizeof(g_config2))
	{
		fclose(f);
		return __FALSE;
	}
	fclose(f);

	return __TRUE;
}

//----------------------------------------------------------------------------

static __weak BIT update_config_file1(void)
{
	FILE *f;

	if(fdelete(g_config_filename1) != 0)
	{
		// nothing to delete
	}

	f = fopen(g_config_filename1, "wb");
	if(f == NULL)
		return __FALSE;

	if(fwrite(&g_config1, 1, sizeof(g_config1), f) != sizeof(g_config1))
	{
		fclose(f);
		return __FALSE;
	}

	fflush(f);
	fclose(f);

	return __TRUE;
}

//----------------------------------------------------------------------------

static __weak BIT update_config_file2(void)
{
	FILE *f;

	if(fdelete(g_config_filename2) != 0)
	{
		// nothing to delete
	}

	f = fopen(g_config_filename2, "wb");
	if(f == NULL)
		return __FALSE;

	if(fwrite(&g_config2, 1, sizeof(g_config2), f) != sizeof(g_config2))
	{
		fclose(f);
		return __FALSE;
	}

	fflush(f);
	fclose(f);

	return __TRUE;
}

//----------------------------------------------------------------------------

static void store_shot_array1(BIT use_extended_field_of_view)
{
	U16 i;
	U16 x, y;
	S32 dx, dy, r;

	g_shot_count1 = 0;
	while(!(CMOS1_RKS_RAM_REG & CMD_TIMER_FIFO_EMPTY_BIT))
	{
		BIT pixel_is_bad = __FALSE;
		BIT pixel_out_of_view = __FALSE;

		CMOS1_RKS_RAM_REG |= RAM_READ_FIFO_BIT;
		x = CMOS1_FIFO_X_REG + FIFO_X_OFFSET;
		y = CMOS1_FIFO_Y_REG + FIFO_Y_OFFSET;

		// try to find pixel in bad pixels array
		for(i = 0; i < g_bad_pixels1.count; i++)
		{
			if(x == g_bad_pixels1.array[i].x && y == g_bad_pixels1.array[i].y)
			{
				pixel_is_bad = __TRUE;
				break;
			}
		}

		// check if pixel is out of view
		dx = (S32)x - (S32)g_config1.nXc;
		dy = (S32)y - (S32)g_config1.nYc;
		// select field-of-view radius for processing
		if(use_extended_field_of_view == __FALSE)	r = (CMOS1_RKS_CMD_REG & CMD_TIMER_CAPTURED_BIT ? g_config1.nRs : g_config1.nRb);	// standard field-of-view
		else										r = (CMOS1_RKS_CMD_REG & CMD_TIMER_CAPTURED_BIT ? g_config1.nRb : g_config1.nRb+1);	// extended field-of-view
		// check if shot is out of field-of-view
		if((dx*dx + dy*dy) > r*r)					pixel_out_of_view = __TRUE;

		// store pixel if not in bad & not out of view
		if(!pixel_is_bad && !pixel_out_of_view)
		{
			g_shot_array1[g_shot_count1].x = x;
			g_shot_array1[g_shot_count1].y = y;
			g_shot_count1++;
		}
	}
}

//----------------------------------------------------------------------------

static void store_shot_array2(BIT use_extended_field_of_view)
{
	U16 i;
	U16 x, y;
	S16 __x, __y;
	S32 dx, dy, r;

	g_shot_count2 = 0;
	while(!(CMOS2_RKS_RAM_REG & CMD_TIMER_FIFO_EMPTY_BIT))
	{
		BIT pixel_is_bad = __FALSE;
		BIT pixel_out_of_view = __FALSE;

		CMOS2_RKS_RAM_REG |= RAM_READ_FIFO_BIT;
		x = CMOS2_FIFO_X_REG + FIFO_X_OFFSET;
		y = CMOS2_FIFO_Y_REG + FIFO_Y_OFFSET;

		// try to find pixel in bad pixels array
		for(i = 0; i < g_bad_pixels2.count; i++)
		{
			if(x == g_bad_pixels2.array[i].x && y == g_bad_pixels2.array[i].y)
			{
				pixel_is_bad = __TRUE;
				break;
			}
		}

		// check if pixel is out of view
		dx = (S32)x - (S32)g_config2.nXc;
		dy = (S32)y - (S32)g_config2.nYc;
		// select field-of-view radius for processing
		if(use_extended_field_of_view == __FALSE)	r = (CMOS2_RKS_CMD_REG & CMD_TIMER_CAPTURED_BIT ? g_config2.nRs : g_config2.nRb);	// standard field-of-view
		else										r = (CMOS2_RKS_CMD_REG & CMD_TIMER_CAPTURED_BIT ? g_config2.nRb : g_config2.nRb+1);	// extended field-of-view
		// check if shot is out of field-of-view
		if((dx*dx + dy*dy) > r*r)					pixel_out_of_view = __TRUE;

		// convert XY-coords from CMOS2 to CMOS1 sensor coords
		__x = (S16)x - (S16)g_config2.nXc + (S16)g_config1.nXc;
		__y = (S16)y - (S16)g_config2.nYc + (S16)g_config1.nYc;

		// store pixel if not in bad & not out of view
		if(!pixel_is_bad && !pixel_out_of_view && __x > 0 && __y > 0)
		{
			g_shot_array2[g_shot_count2].x = (U16)__x;
			g_shot_array2[g_shot_count2].y = (U16)__y;
			g_shot_count2++;
		}
	}
}

//----------------------------------------------------------------------------

static void build_clusters_array1(void)
{
	U8  marker;
	U16 i, j, k, count;
	U16 l, t, r, b;

	memset(g_cluster_coords1, 0, sizeof(g_cluster_coords1));

	for(i = 0; i < g_shot_count1; i++)
	{
		FIFO_ITEM			*fi = &g_shot_array1[i];
		U16					x = fi->x / CELL_SIZE;
		U16					y = fi->y / CELL_SIZE;
		CLUSTER_COORD		*pcc = &g_cluster_coords1[y*IMAGE_CELL_CX + x];

		pcc->count++;
		pcc->excluded		= __FALSE;
		pcc->marker			= NO_MARKER;
		pcc->rect.left		= (x    ) * CELL_SIZE;
		pcc->rect.top		= (y    ) * CELL_SIZE;
		pcc->rect.right		= (x + 1) * CELL_SIZE;
		pcc->rect.bottom	= (y + 1) * CELL_SIZE;
	}

	memset(g_cluster_ptrs1, 0, sizeof(g_cluster_ptrs1));

	g_cluster_count1 = 0;
	for(i = 0; i < IMAGE_CELL_CX*IMAGE_CELL_CY; i++)
	{
		if(g_cluster_coords1[i].count > 0)
			g_cluster_ptrs1[g_cluster_count1++] = &g_cluster_coords1[i];
	}

	marker = NO_MARKER;
	for(i = 0; i < g_cluster_count1; i++)
	{
		CLUSTER_COORD *pcc1 = g_cluster_ptrs1[i];
		S16 x1 = (pcc1->rect.left + pcc1->rect.right) / 2;
 		S16 y1 = (pcc1->rect.top + pcc1->rect.bottom) / 2;

		if(pcc1->marker == NO_MARKER)
			pcc1->marker = (++marker);

		for(j = i + 1; j < g_cluster_count1; j++)
		{
			CLUSTER_COORD *pcc2 = g_cluster_ptrs1[j];
			S16 x2 = (pcc2->rect.left + pcc2->rect.right) / 2;
 			S16 y2 = (pcc2->rect.top + pcc2->rect.bottom) / 2;

			if(abs(x1 - x2) <= CELL_SIZE && abs(y1 - y2) <= CELL_SIZE)
			{
				if(pcc2->marker == NO_MARKER)
				{
					pcc2->marker = pcc1->marker;
				}
				else
				{
					for(k = 0; k < g_cluster_count1; k++)
					{
						CLUSTER_COORD *pcc3 = g_cluster_ptrs1[k];

						if(pcc3->marker == pcc2->marker)
							pcc3->marker = pcc1->marker;
					}
				}
			}
		}
	}

	memset(g_super_cluster_coords1, 0, sizeof(g_super_cluster_coords1));

	g_super_cluster_count1 = 0;
	for(i = 1; i <= marker; i++)
	{
		count = 0;
		l = 0x7fff, t=0x7fff, r=0, b=0;

		for(j = 0; j < g_cluster_count1; j++)
		{
			CLUSTER_COORD *pcc = g_cluster_ptrs1[j];

			if(pcc->marker == i)
			{
				if(pcc->rect.left   < l) l = pcc->rect.left;
				if(pcc->rect.top    < t) t = pcc->rect.top;
				if(pcc->rect.right  > r) r = pcc->rect.right;
				if(pcc->rect.bottom > b) b = pcc->rect.bottom;

				count += pcc->count;
			}
		}

		if(count > 0)
		{
			g_super_cluster_coords1[g_super_cluster_count1].count       = count;
			g_super_cluster_coords1[g_super_cluster_count1].excluded    = __FALSE;
			g_super_cluster_coords1[g_super_cluster_count1].marker      = NO_MARKER;
			g_super_cluster_coords1[g_super_cluster_count1].rect.left   = l;
			g_super_cluster_coords1[g_super_cluster_count1].rect.top    = t;
			g_super_cluster_coords1[g_super_cluster_count1].rect.right  = r;
			g_super_cluster_coords1[g_super_cluster_count1].rect.bottom = b;
			g_super_cluster_count1++;
		}
	}
}

//----------------------------------------------------------------------------

static void build_clusters_array2(void)
{
	U8  marker;
	U16 i, j, k, count;
	U16 l, t, r, b;

	memset(g_cluster_coords2, 0, sizeof(g_cluster_coords2));

	for(i = 0; i < g_shot_count2; i++)
	{
		FIFO_ITEM			*fi = &g_shot_array2[i];
		U16					x = fi->x / CELL_SIZE;
		U16					y = fi->y / CELL_SIZE;
		CLUSTER_COORD		*pcc = &g_cluster_coords2[y*IMAGE_CELL_CX + x];
		pcc->count++;
		pcc->excluded		= __FALSE;
		pcc->marker			= NO_MARKER;
		pcc->rect.left		= (x    ) * CELL_SIZE;
		pcc->rect.top		= (y    ) * CELL_SIZE;
		pcc->rect.right		= (x + 1) * CELL_SIZE;
		pcc->rect.bottom	= (y + 1) * CELL_SIZE;
	}

	memset(g_cluster_ptrs2, 0, sizeof(g_cluster_ptrs2));

	g_cluster_count2 = 0;
	for(i = 0; i < IMAGE_CELL_CX*IMAGE_CELL_CY; i++)
	{
		if(g_cluster_coords2[i].count > 0)
			g_cluster_ptrs2[g_cluster_count2++] = &g_cluster_coords2[i];
	}

	marker = NO_MARKER;
	for(i = 0; i < g_cluster_count2; i++)
	{
		CLUSTER_COORD *pcc1 = g_cluster_ptrs2[i];
		S16 x1 = (pcc1->rect.left + pcc1->rect.right) / 2;
 		S16 y1 = (pcc1->rect.top + pcc1->rect.bottom) / 2;

		if(pcc1->marker == NO_MARKER)
			pcc1->marker = (++marker);

		for(j = i + 1; j < g_cluster_count2; j++)
		{
			CLUSTER_COORD *pcc2 = g_cluster_ptrs2[j];
			S16 x2 = (pcc2->rect.left + pcc2->rect.right) / 2;
 			S16 y2 = (pcc2->rect.top + pcc2->rect.bottom) / 2;

			if(abs(x1 - x2) <= CELL_SIZE && abs(y1 - y2) <= CELL_SIZE)
			{
				if(pcc2->marker == NO_MARKER)
				{
					pcc2->marker = pcc1->marker;
				}
				else
				{
					for(k = 0; k < g_cluster_count2; k++)
					{
						CLUSTER_COORD *pcc3 = g_cluster_ptrs2[k];

						if(pcc3->marker == pcc2->marker)
							pcc3->marker = pcc1->marker;
					}
				}
			}
		}
	}

	memset(g_super_cluster_coords2, 0, sizeof(g_super_cluster_coords2));

	g_super_cluster_count2 = 0;
	for(i = 1; i <= marker; i++)
	{
		count = 0;
		l = 0x7fff, t=0x7fff, r=0, b=0;

		for(j = 0; j < g_cluster_count2; j++)
		{
			CLUSTER_COORD *pcc = g_cluster_ptrs2[j];

			if(pcc->marker == i)
			{
				if(pcc->rect.left   < l) l = pcc->rect.left;
				if(pcc->rect.top    < t) t = pcc->rect.top;
				if(pcc->rect.right  > r) r = pcc->rect.right;
				if(pcc->rect.bottom > b) b = pcc->rect.bottom;

				count += pcc->count;
			}
		}

		if(count > 0)
		{
			g_super_cluster_coords2[g_super_cluster_count2].count       = count;
			g_super_cluster_coords2[g_super_cluster_count2].excluded    = __FALSE;
			g_super_cluster_coords2[g_super_cluster_count2].marker      = NO_MARKER;
			g_super_cluster_coords2[g_super_cluster_count2].rect.left   = l;
			g_super_cluster_coords2[g_super_cluster_count2].rect.top    = t;
			g_super_cluster_coords2[g_super_cluster_count2].rect.right  = r;
			g_super_cluster_coords2[g_super_cluster_count2].rect.bottom = b;
			g_super_cluster_count2++;
		}
	}
}

//----------------------------------------------------------------------------

static void exclude_intersected_clusters_from_array1(void)
{
	U16 i, j;

	for(i = 0; i < g_super_cluster_count1; i++)
	{
		CLUSTER_COORD *c1 = &g_super_cluster_coords1[i];

		for(j = 0; j < g_super_cluster_count2; j++)
		{
			CLUSTER_COORD *c2 = &g_super_cluster_coords2[j];

			if(IntersectRect(NULL, &c1->rect, &c2->rect))
				c1->excluded = __TRUE;
		}
	}
}

//----------------------------------------------------------------------------

static void exclude_intersected_clusters_from_array2(void)
{
	U16 i, j;

	for(i = 0; i < g_super_cluster_count2; i++)
	{
		CLUSTER_COORD *c2 = &g_super_cluster_coords2[i];

		for(j = 0; j < g_super_cluster_count1; j++)
		{
			CLUSTER_COORD *c1 = &g_super_cluster_coords1[j];

			if(IntersectRect(NULL, &c1->rect, &c2->rect))
				c2->excluded = __TRUE;
		}
	}
}

//----------------------------------------------------------------------------

static void enlarge_clusters_in_array1(void)
{
	U16 i;

	for(i = 0; i < g_super_cluster_count1; i++)
	{
		CLUSTER_COORD *pcc = &g_super_cluster_coords1[i];

		if(pcc->rect.left   >=          CELL_SIZE)	pcc->rect.left   -= CELL_SIZE;
		if(pcc->rect.top    >=          CELL_SIZE)	pcc->rect.top    -= CELL_SIZE;
		if(pcc->rect.right  <= IMAGE_CX-CELL_SIZE)	pcc->rect.right  += CELL_SIZE;
		if(pcc->rect.bottom <= IMAGE_CY-CELL_SIZE)	pcc->rect.bottom += CELL_SIZE;
	}
}

//----------------------------------------------------------------------------

static void enlarge_clusters_in_array2(void)
{
	U16 i;

	for(i = 0; i < g_super_cluster_count2; i++)
	{
		CLUSTER_COORD *pcc = &g_super_cluster_coords2[i];

		if(pcc->rect.left   >=          CELL_SIZE)	pcc->rect.left   -= CELL_SIZE;
		if(pcc->rect.top    >=          CELL_SIZE)	pcc->rect.top    -= CELL_SIZE;
		if(pcc->rect.right  <= IMAGE_CX-CELL_SIZE)	pcc->rect.right  += CELL_SIZE;
		if(pcc->rect.bottom <= IMAGE_CY-CELL_SIZE)	pcc->rect.bottom += CELL_SIZE;
	}
}

//----------------------------------------------------------------------------

static __weak BIT calculate_mass_centre_by_array1(float *px, float *py)
{
	U16 i, j, c = 0;
	float fx = 0, fy = 0;

	for(i = 0; i < g_super_cluster_count1; i++)
	{
		CLUSTER_COORD *pcc = &g_super_cluster_coords1[i];

		if(pcc->excluded == __FALSE)
		{
			for(j = 0; j < g_shot_count1; j++)
			{
				U16 x = g_shot_array1[j].x;
				U16 y = g_shot_array1[j].y;

				if(x >= pcc->rect.left && x < pcc->rect.right &&
					y >= pcc->rect.top && y < pcc->rect.bottom)
				{
					fx += x;
					fy += y;
					c++;
				}
			}
		}
	}

	if(c > 0)
	{
		*px = fx / (float)c;
		*py = fy / (float)c;

		return __TRUE;
	}

	return __FALSE;
}

//----------------------------------------------------------------------------

static __weak BIT calculate_mass_centre_by_array2(float *px, float *py)
{
	U16 i, j, c = 0;
	float fx = 0, fy = 0;

	for(i = 0; i < g_super_cluster_count2; i++)
	{
		CLUSTER_COORD *pcc = &g_super_cluster_coords2[i];

		if(pcc->excluded == __FALSE)
		{
			for(j = 0; j < g_shot_count2; j++)
			{
				U16 x = g_shot_array2[j].x;
				U16 y = g_shot_array2[j].y;

				if(x >= pcc->rect.left && x < pcc->rect.right &&
					y >= pcc->rect.top && y < pcc->rect.bottom)
				{
					fx += x;
					fy += y;
					c++;
				}
			}
		}
	}

	if(c > 0)
	{
		*px = fx / (float)c;
		*py = fy / (float)c;

		return __TRUE;
	}

	return __FALSE;
}

//----------------------------------------------------------------------------
static BIT calculate_mass_centre_1(void)
{
	U16 i, j, c = 0, cc = 0;
	float fx = 0, fy = 0;
	S16 angle1, angle2;
	COORDS_DATA coords;

	for(i = 0; i < g_super_cluster_count1; i++)
	{
		CLUSTER_COORD *pcc = &g_super_cluster_coords1[i];

		if(pcc->excluded == __FALSE)
		{
			for(j = 0; j < g_shot_count1; j++)
			{
				U16 x = g_shot_array1[j].x;
				U16 y = g_shot_array1[j].y;

				if(x >= pcc->rect.left && x < pcc->rect.right &&
					y >= pcc->rect.top && y < pcc->rect.bottom)
				{
					fx += x;
					fy += y;
					c++;
				}
			}
			cc++;
		}
	}

	if(c > 0 && cc > 0)
	{
		S16 _x = fx / (float)c;
		S16 _y = fy / (float)c;
		calculate_angles(_x, _y, &angle1, &angle2);
		coords.time = (U32)get_time_10us();
		coords.angle1 = angle1;
		coords.angle2 = angle2;
		push_coords_item(&coords);
		c = 0;
		fx = 0;
		fy = 0;
		return __TRUE;
	}
	return __FALSE;
}

//----------------------------------------------------------------------------
static BIT calculate_mass_centre_2(void)
{
	U16 i, j, c = 0, cc = 0;
	float fx = 0, fy = 0;
	S16 angle1, angle2;
	COORDS_DATA coords;

	for(i = 0; i < g_super_cluster_count2; i++)
	{
		CLUSTER_COORD *pcc = &g_super_cluster_coords2[i];

		if(pcc->excluded == __FALSE)
		{
			for(j = 0; j < g_shot_count2; j++)
			{
				U16 x = g_shot_array2[j].x;
				U16 y = g_shot_array2[j].y;

				if(x >= pcc->rect.left && x < pcc->rect.right &&
					y >= pcc->rect.top && y < pcc->rect.bottom)
				{
					fx += x;
					fy += y;
					c++;
				}
			}
			cc++;
		}
	}

	if(c > 0 && cc > 0)
	{
		S16 _x = fx / (float)c;
		S16 _y = fy / (float)c;
		calculate_angles(_x, _y, &angle1, &angle2);
		coords.time = (U32)get_time_10us();
		coords.angle1 = angle1;
		coords.angle2 = angle2;
		push_coords_item(&coords);
		c = 0;
		fx = 0;
		fy = 0;
		return __TRUE;
	}
	return __FALSE;
}

//----------------------------------------------------------------------------

static BIT load_application_settings(void)
{
	BIT result = __TRUE;

	memset(&g_bad_pixels1, 0, sizeof(g_bad_pixels1));
	memset(&g_bad_pixels2, 0, sizeof(g_bad_pixels2));
	memset(&g_config1    , 0, sizeof(g_config1    ));
	memset(&g_config2    , 0, sizeof(g_config2    ));

	if(finit("S:") != 0)
		return __FALSE;

	if(!load_bad_pixel_files())
		result = __FALSE;

	if(!load_config_files())
	{
		result = __FALSE;
	}
	else
	{
		dac_SetA(g_config1.nVinb);
		dac_SetB(g_config1.nVref);
		dac_SetC(g_config2.nVinb);
		dac_SetD(g_config2.nVref);

		CMOS1_POROG_REG = ((U16)g_config1.nLimit) << 2;
		CMOS2_POROG_REG = ((U16)g_config2.nLimit) << 2;
	}

	return result;
}

//----------------------------------------------------------------------------

static void calculate_angles(float x, float y, S16 *pAngle1, S16 *pAngle2)
{
	static float A, B, C;
	static float Xo, Yo;
	static float R, Alpha;
	static float sinB, cosB, sinA;
	static float Fi = 0, Tetta = 0;

	A = g_config1.fA;
	B = g_config1.fB;
	C = g_config1.fC;
	Xo = (float)g_config1.nXc;
	Yo = (float)g_config1.nYc;

	R = sqrt((x - Xo)*(x - Xo) + (y - Yo)*(y - Yo));
	if(R > 0.0)
	{
		float r = R;// * 122.0 / ((float)g_config1.nRs - 10.0);	// default: (Rs - 10.0)

		Alpha = (A*r*r*r + B*r*r + C*r) * M_PI / 180.0;
		if(Alpha > 90.0 * M_PI / 180.0)	Alpha = 90.0 * M_PI / 180.0;
		if(Alpha <  0.0               )	Alpha = 0.0;

		sinB = (y - Yo) / R;
		cosB = (x - Xo) / R;
		sinA = sin(Alpha);

		Tetta = asin(sinA * sinB);	//  Tetta: -PI/2...+PI/2

		if(fabs(Tetta) >= M_PI/2.0)	Fi = 0.0;
		else						Fi = asin(sinA * cosB / cos(Tetta));	// Fi: -PI/2...+PI/2
	}

	*pAngle1 = (S16)rint((-Fi * 180.0 / M_PI) * 60.0);
	*pAngle2 = (S16)rint((-Tetta * 180.0 / M_PI) * 60.0);
}

//----------------------------------------------------------------------------

BIT push_coords_item(const COORDS_DATA *coords)
{
	ENTER_CRITICAL_SECTION();

	// FIFO buffer is full? exit
	if(g_coords_index == COORDS_FIFO_SIZE)
	{
		LEAVE_CRITICAL_SECTION();
		return __FALSE;
	}

	g_coords[g_coords_index++] = *coords;

	LEAVE_CRITICAL_SECTION();

	return __TRUE;
}

//----------------------------------------------------------------------------

BIT pop_coords_item(COORDS_DATA *coords)
{
	ENTER_CRITICAL_SECTION();

	// FIFO buffer is empty? exit
	if(g_coords_index == 0)
	{
		LEAVE_CRITICAL_SECTION();
		return __FALSE;
	}

	*coords = g_coords[--g_coords_index];

	LEAVE_CRITICAL_SECTION();

	return __TRUE;
}

//----------------------------------------------------------------------------

U16 get_coords_count(void)
{
	U16 count;

	ENTER_CRITICAL_SECTION();
	count = g_coords_index;
	LEAVE_CRITICAL_SECTION();

	return count;
}

//----------------------------------------------------------------------------

static U64 get_time_10us(void)
{
	U64 init_time_10us;

	ENTER_CRITICAL_SECTION();
	init_time_10us = g_init_time_10us;
	LEAVE_CRITICAL_SECTION();

	return init_time_10us + rtc_GetTickCount_us() / 10;
}

//----------------------------------------------------------------------------

static BOOL CAN_safe_send(CAN_msg *msg, U16 timeout)
{
	U16 delay = 0;

	// RM (reset mode) bit might be set if 256 Tx errors
	// have occurred so we should clear it manually
	// regardless of Rx or Tx error counters
	CAN_hw_clear_reset_mode(CAN_CTRL);

	// reset RX error counter before transmittion
	if(CAN_hw_rx_get_err_counter(CAN_CTRL) >= 192)
		CAN_hw_rx_reset_err_counter(CAN_CTRL);
	
	if(CAN_send(CAN_CTRL, msg, timeout) != CAN_OK)
	{
		// nothing to do
	}

	while(!CAN_EX_tx_ready(CAN_CTRL) && CAN_hw_tx_get_err_counter(CAN_CTRL) == 0 && delay < timeout)
	{
		os_dly_wait(1);
		delay++;
	}

	// abort transmittion & clear TX error counter if not zero
	if(CAN_hw_tx_get_err_counter(CAN_CTRL) > 0)
	{
		CAN_hw_tx_abort(CAN_CTRL);
		CAN_hw_tx_reset_err_counter(CAN_CTRL);
		return __FALSE;
	}

	// abort transmittion on timeout
	if(delay >= timeout)
	{
		CAN_hw_tx_abort(CAN_CTRL);
		return __FALSE;
	}

	return __TRUE;
}

//----------------------------------------------------------------------------

static BOOL CAN_safe_receive(CAN_msg *msg, U16 timeout)
{
	CAN_ERROR result;
	result = CAN_receive(CAN_CTRL, msg, timeout);

	// RM (reset mode) bit might be set if 256 Tx errors
	// have occurred so we should clear it manually
	// regardless of Rx or Tx error counters
	CAN_hw_clear_reset_mode(CAN_CTRL);

	// reset RX error counter if limit reached
	if(CAN_hw_rx_get_err_counter(CAN_CTRL) >= 192)
		CAN_hw_rx_reset_err_counter(CAN_CTRL);

	if(result != CAN_OK)
		return __FALSE;

	return __TRUE;
}

//----------------------------------------------------------------------------

static CAN_ERROR CAN_init_controller(void)
{
	CAN_ERROR err;

	// initialize CAN controller
	if((err = CAN_init(CAN_CTRL, CAN_BAUDRATE)) != CAN_OK)
		return err;

	// store acceptance filter with broadcast IDs
	if((err = CAN_rx_object(CAN_CTRL, 0, CAN_MSG_ID_IN_BROADCAST_RESET     , STANDARD_FORMAT)) != CAN_OK)
		return err;
	if((err = CAN_rx_object(CAN_CTRL, 0, CAN_MSG_ID_IN_BROADCAST_CLOCK_SYNC, STANDARD_FORMAT)) != CAN_OK)
		return err;
	if((err = CAN_rx_object(CAN_CTRL, 0, CAN_MSG_ID_IN_BROADCAST_GET_STATUS, STANDARD_FORMAT)) != CAN_OK)
		return err;

	// store acceptance filter with individual IDs
	if((err = CAN_rx_object(CAN_CTRL, 0, CAN_MSG_ID_IN_RESET               , STANDARD_FORMAT)) != CAN_OK)
		return err;
	if((err = CAN_rx_object(CAN_CTRL, 0, CAN_MSG_ID_IN_CLOCK_SYNC          , STANDARD_FORMAT)) != CAN_OK)
		return err;
	if((err = CAN_rx_object(CAN_CTRL, 0, CAN_MSG_ID_IN_GET_STATUS          , STANDARD_FORMAT)) != CAN_OK)
		return err;

	return CAN_start(CAN_CTRL);
}

//----------------------------------------------------------------------------

static void bad_pixels_dynamic_calibration(void)
{
	//
	// TO DO (perform dynamic "new" bad pixel detection)
	//
}

//----------------------------------------------------------------------------
