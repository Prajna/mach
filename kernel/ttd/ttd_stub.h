/* 
 * Mach Operating System
 * Copyright (c) 1993,1992 Carnegie Mellon University
 * All Rights Reserved.
 * 
 * Permission to use, copy, modify and distribute this software and its
 * documentation is hereby granted, provided that both the copyright
 * notice and this permission notice appear in all copies of the
 * software, derivative works or modified versions, and any portions
 * thereof, and that both notices appear in supporting documentation.
 * 
 * CARNEGIE MELLON ALLOWS FREE USE OF THIS SOFTWARE IN ITS "AS IS"
 * CONDITION.  CARNEGIE MELLON DISCLAIMS ANY LIABILITY OF ANY KIND FOR
 * ANY DAMAGES WHATSOEVER RESULTING FROM THE USE OF THIS SOFTWARE.
 * 
 * Carnegie Mellon requests users of this software to return to
 * 
 *  Software Distribution Coordinator  or  Software.Distribution@CS.CMU.EDU
 *  School of Computer Science
 *  Carnegie Mellon University
 *  Pittsburgh PA 15213-3890
 * 
 * any improvements or extensions that they make and grant Carnegie Mellon
 * the rights to redistribute these changes.
 */
/*
 * TTD externs.
 *
 * HISTORY
 * $Log:	ttd_stub.h,v $
 * Revision 2.2  93/05/10  23:25:08  rvb
 * 	Checkin for MK80 branch.
 * 	[93/05/10  15:10:15  grm]
 * 
 * Revision 2.1.2.2  93/04/20  11:01:18  grm
 * 	Changed types for use with version 2.4 of the protocol.
 * 	[93/04/20            grm]
 * 
 * Revision 2.1.2.1  93/03/03  14:35:28  grm
 * 	Changed interface.  Version works.
 * 	[93/03/03            grm]
 * 
 * Revision 2.1.1.6  93/01/28  15:19:49  grm
 * 	Last checkin before locore rewrite.
 * 
 * Revision 2.1.1.5  92/10/23  21:23:49  grm
 * 	Added single stepping boolean.
 * 	[92/10/23            grm]
 * 
 * Revision 2.1.1.4  92/10/08  14:31:56  grm
 * 	Added kttd_debug.
 * 	[92/10/08            grm]
 * 
 * Revision 2.1.1.3  92/10/01  15:36:27  grm
 * 	KTTD Restructuring checkpoint.
 * 	[92/10/01            grm]
 * 
 * Revision 2.1.1.2  92/09/25  15:15:11  grm
 * 	checkpointing...
 * 	[92/09/25            grm]
 * 
 * Revision 2.1.1.1  92/09/09  14:45:23  grm
 * 	Initial checkin.
 * 
 */

#ifndef	_TTD_STUB_H_
#define	_TTD_STUB_H_

#include <mach/boolean.h>
#include <mach/vm_param.h>
#include <ttd/ttd_comm.h>
#include <ttd/ttd_types.h>
#include <ipc/ipc_kmsg.h>

extern integer_t kttd_active;

extern boolean_t kttd_enabled;
extern boolean_t kttd_debug;
extern boolean_t kttd_debug_init;
extern boolean_t kttd_single_stepping;

extern vm_offset_t	kttd_current_request;
extern natural_t	kttd_current_length;
extern ipc_kmsg_t	kttd_current_kmsg;
extern ttd_status_t	kttd_run_status;

extern boolean_t	kttd_servicing_async;

#define	BYTE_ALIGNMENT		4
#define MAX_TTD_MSG_SIZE	2048

extern char *ttd_request_msg;
extern char *ttd_reply_msg;
extern char ttd_request_msg_array[];
extern char ttd_reply_msg_array[];


extern integer_t ttd_device_unit;
extern int (*ttd_get_packet)();
extern int (*ttd_send_packet)();
extern struct ether_hardware_address ttd_host_ether_id;

/*
 * Prototypes:
 */
extern void ttd_init(void);
extern boolean_t kttd_handle_async(ipc_kmsg_t kmsg);
extern void kttd_task_trap(int type, int code, boolean_t user_space);

#endif /* _TTD_STUB_H_ */
