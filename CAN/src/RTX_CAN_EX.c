#include <LPC22xx.H>
#include <RTL.h>                      /* RTX kernel functions & defines      */
#include <STRING.h>					  /* for memset fnc.                     */
#include "CAN_Cfg.h"                  /* CAN Configuration                   */
#include "RTX_CAN.h"                  /* CAN Generic functions & defines     */
#include "CAN_EX.h"                   /* CAN Extended functions & defines    */
/*---------------------------------------------------------------------------
 *
 *  Returns a number of free items in TX mailbox pool
 *
 *  Parameter:  ctrl:       Index of the hardware CAN controller (1 .. x)
 *
 *  Return:     OS_RESULT:  Number of free items (within 0 ... CAN_No_SendObjects)
 *---------------------------------------------------------------------------*/

OS_RESULT CAN_EX_tx_pool_free_items(U32 ctrl)
{
	U32 ctrl0 = ctrl - 1;               /* Controller index 0 .. x-1         */

	return os_mbx_check(MBX_tx_ctrl[ctrl0]);
}

/*---------------------------------------------------------------------------
 *
 *  Remove all messages stored into transmitter pool
 *
 *  Parameter:  ctrl:       Index of the hardware CAN controller (1 .. x)
 *
 *  Return:
 *---------------------------------------------------------------------------*/

void CAN_EX_tx_pool_cleanup(U32 ctrl)
{
	U32 ctrl0 = ctrl - 1;               /* Controller index 0 .. x-1         */
	CAN_msg *ptrmsg;

	while(os_mbx_wait(MBX_tx_ctrl[ctrl0], (void **)&ptrmsg, 0) == OS_R_OK)
		_free_box(CAN_mpool, ptrmsg);
}

/*---------------------------------------------------------------------------
 *
 *  Check if hardware is ready to transmit a message
 *
 *  Parameter:  ctrl:       Index of the hardware CAN controller (1 .. x)
 *
 *  Return:     BOOL:       __TRUE - h/w is ready, __FALSE - h/w is busy
 *---------------------------------------------------------------------------*/

BOOL CAN_EX_tx_ready(U32 ctrl)
{
	return (CAN_hw_tx_empty_keep_state(ctrl) == CAN_OK ? __TRUE : __FALSE);
}

/*----------------------------------------------------------------------------
 * end of file
 *---------------------------------------------------------------------------*/
