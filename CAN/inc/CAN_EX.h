#include <RTL.h>                      /* RTX kernel functions & defines      */
#include "CAN_Cfg.h"                  /* CAN Configuration                   */

#ifndef _RTX_CAN_EX_H
#define _RTX_CAN_EX_H

/* Functions defined in module RTX_CAN_EX.c                                  */
OS_RESULT	CAN_EX_tx_pool_free_items(U32 ctrl);
void		CAN_EX_tx_pool_cleanup(U32 ctrl);
BOOL		CAN_EX_tx_ready(U32 ctrl);

/* Helper functionality */
#define		CAN_EX_can_send_to_hw(ctrl)		((CAN_EX_tx_pool_free_items(ctrl) == CAN_No_SendObjects) && CAN_EX_tx_ready(ctrl))
#define		CAN_EX_can_send_to_fifo(ctrl)	(CAN_EX_tx_pool_free_items(ctrl) > 0)

#endif /* _RTX_CAN_EX_H */

/*----------------------------------------------------------------------------
 * end of file
 *---------------------------------------------------------------------------*/
