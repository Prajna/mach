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
 * TTD Stub code for the kernel.  Misc, LowfaceTTD code.
 *
 * HISTORY:
 * $Log:	ttd_stub.c,v $
 * Revision 2.2  93/05/10  23:25:04  rvb
 * 	Turned kttd debugging output off by default.
 * 	[93/05/10  15:15:11  grm]
 * 
 * 	Checkin for MK80 branch.
 * 	[93/05/10  15:09:57  grm]
 * 
 * Revision 2.1.2.2  93/04/20  10:59:12  grm
 * 	Changed the types for protocol version 2.4.  Changed the logic
 * 	for dropping packets.
 * 	[93/04/20            grm]
 * 
 * Revision 2.1.2.1  93/03/03  14:35:18  grm
 * 	Second version of code.  It works.
 * 	[93/03/03            grm]
 * 
 * Revision 2.1.1.9  93/01/28  15:19:13  grm
 * 	Added ttd_loop_status.  Last checkin before locore rewrite.
 * 
 * Revision 2.1.1.8  93/01/22  15:53:23  grm
 * 	Added request length checks.
 * 
 * Revision 2.1.1.7  93/01/21  13:05:26  grm
 * 	Changed to Ansi C prototypes.  Modified code for single stepping.
 * 
 * Revision 2.1.1.6  92/10/23  21:23:09  grm
 * 	First pass at single stepping.  Left over code from debugging.
 * 	[92/10/23            grm]
 * 
 * Revision 2.1.1.5  92/10/08  14:30:40  grm
 * 	Small changes.  Checkin before ttd-ether rewrite.
 * 	[92/10/08            grm]
 * 
 * Revision 2.1.1.4  92/10/01  15:37:20  grm
 * 	KTTD restructuring checkpoint.
 * 	[92/10/01            grm]
 * 
 * Revision 2.1.1.3  92/09/30  13:31:16  grm
 * 	Added sync and async routines.  Filled in kttd_trap.
 * 	[92/09/30            grm]
 * 
 * Revision 2.1.1.2  92/09/25  15:12:12  grm
 * 	checkpointing...
 * 	[92/09/25            grm]
 * 
 * Revision 2.1.1.1  92/09/09  14:44:34  grm
 * 	Initial Checkin.
 * 
 */
/***********************************************************
Copyright 1992 by Digital Equipment Corporation, Maynard, Massachusetts,

                        All Rights Reserved

Permission to use, copy, modify, and distribute this software and its 
documentation for any purpose and without fee is hereby granted, provided 
that the above copyright notice appear in all copies and that both that 
copyright notice and this permission notice appear in supporting 
documentation, and that the name of Digital not be used in advertising 
or publicity pertaining to distribution of the software without specific, 
written prior permission.  Digital makes no representations about the 
suitability of this software for any purpose.  It is provided "as is"
without express or implied warranty.

DIGITAL DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING
ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL
DIGITAL BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR
ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
SOFTWARE.

******************************************************************/

#include <sys/types.h>
#include <mach/mach_types.h>
#include <sys/reboot.h>
#include <ipc/ipc_kmsg.h>
#include <mach_kdb.h>
#include <ttd/ttd_types.h>
#include <ttd/ttd_msg.h>
#include <ttd/ttd_stub.h>

/*
 *	Status and Debugging flags
 */
boolean_t 	kttd_enabled = FALSE;	/* are we catching traps with ttd? */
integer_t 	kttd_active = MIN_KTTD_ACTIVE;	/* are we in ttd NOW? */
boolean_t 	kttd_debug_init = FALSE; /* != 0 waits for host at bootstrap */
boolean_t	kttd_debug = FALSE;	/* are we debugging kttd? */

/*
 * Pointer to the current request, and its length.  It must be a
 * global since it must be set in the async entry point which may
 * indirectly jump to the kttd_task_trap routine (as in the i386at
 * version).
 */
vm_offset_t	kttd_current_request;
natural_t	kttd_current_length;
ipc_kmsg_t	kttd_current_kmsg;

ttd_status_t	kttd_run_status;

/*
 * Allocate a static net_kmsg_get buffer for receives
 * 
 * ttd_request_msg is really used as a net_kmsg_t
 *
 * ttd_reply_msg is NOT used as a net_kmsg_t, but only as
 *               the actual msg!!!
 *
 * Added four bytes so that we can shift alignment around.
 */
char ttd_request_msg_array[MAX_TTD_MSG_SIZE + BYTE_ALIGNMENT];
char ttd_reply_msg_array[MAX_TTD_MSG_SIZE + BYTE_ALIGNMENT];

char * ttd_request_msg = NULL;
char * ttd_reply_msg = NULL;

/*
 * The low level ethernet driver vars:
 */
integer_t ttd_device_unit = -1;
int (*ttd_get_packet)() = NULL;
int (*ttd_send_packet)() = NULL;
struct ether_hardware_address ttd_host_ether_id;


/*
 * Initialize the TTD code.
 * 
 * Allocate static buffer, etc.
 *
 */
void ttd_init(void)
{
	extern int boothowto;

	if (boothowto & RB_KDB) {

		/*
		 * Set 'em up so that bootp can work.
		 * Must be redone before high level protocols with
		 * 32 bit structure members can be used.
		 */
		ttd_request_msg = &ttd_request_msg_array[0];
		ttd_reply_msg = &ttd_reply_msg_array[0];
		
		boothowto &= ~RB_KDB;
		kttd_enabled = TRUE;

		/*
		 * Init console kgdb device.
		 */
		if(!kttd_console_init()) {
			kttd_active = FALSE;
			kttd_enabled = FALSE;
			return;
		}

		/*
		 * Init the ttd_server.
		 */
		ttd_server_initialize();

		printf("TTD enabled, protocol version: %d.%d.\n",
		       TTD_VERSION/10, TTD_VERSION%10);

		if (kttd_debug_init) {
			/*
			 * Debugging init procedure, break here.
			 */
			printf("TTD waiting...");
			kttd_break();
			printf(" connected.\n");

		}
	} else
		kttd_enabled = FALSE;
}

/*
 * kttd_handle_async:
 *
 *  This routine deals with asynchronous ttd requests.  These requests are
 * obtained via interrupts when the kernel is running.  Normally this is
 * when an ethernet packet has just arrived.
 *
 *  If the current ethernet packet is a valid ttd request, we service it.
 * It returns FALSE if the kernel should send the packet to the filter,
 * or TRUE if it was a ttd packet and has been handled already.
 *
 *  The main difference between this routine and the sychronous one, is
 * that the kernel target is not stopped before service, which allows
 * only a small set of possible ttd operations to occur on the kernel
 * (get_info, probe_server, stop_target).
 *
 */
boolean_t kttd_handle_async(ipc_kmsg_t kmsg)
{
	vm_offset_t	ttd_msg;
	natural_t	ttd_length;

	/*
	 * Return if not supported.
	 */
	if (!kttd_supported()) {
		kttd_enabled = FALSE;
		if (kttd_debug)
			printf("kttd_handle_async: kttd not supported!\n");
		return FALSE;
	}

	if (!kttd_valid_request(kmsg, FALSE, &ttd_msg, &ttd_length))
		return FALSE;

	/*
	 * If we are already servicing a TTD interrupt,
	 * we just drop this packet.
	 */
	if ((kttd_run_status != RUNNING) || (kttd_single_stepping)) {

		if (kttd_debug)
			printf("kttd_handle_async: dropping packet, !run or sstep.\n");
		
		/*
		 * Put the kmsg back in the free queue:
		 */
		net_kmsg_put(kmsg);

		/*
		 * Tell it we handled it.
		 */
		return TRUE;
	}

	/*
	 * Set up variables for use after we enter the
	 * Teledebug service code.  Teledebug service
	 * code will net_put_kmsg of the kmsg after the
	 * request has been processed.
	 */
	kttd_current_request = ttd_msg;
	kttd_current_length = ttd_length;
	kttd_current_kmsg = kmsg;
	kttd_run_status = ONE_STOP;

	if (kttd_debug)
		printf("kttd_async: calling kttd_intr()\n");

	/*
	 * "Call teledebugger"
	 */
	kttd_intr();

	if (kttd_debug)
		printf("kttd_handle_async: returning\n");

	return TRUE;
}

/*
 * kttd_task_trap:
 *
 *  This routine is called by the kttd_trap routine in the
 * kttd_interface.c file.  Both asyncronous and syncronous entries call
 * this routine.
 *
 */
void kttd_task_trap(int type, int code, boolean_t user_space)
{
	if (kttd_debug)
		printf("kttd_task_trap entered. type = %d, code = %x, from %s space\n",
		       type, code, (user_space ? "USER" : "KERNEL"));

	if (kttd_run_status == FULL_STOP) {

		/*
		 * We're at a full stop.  So:
		 * 
		 * -  Halt the other processors
		 * -  Take us out of single stepping mode
		 *
		 */
		kttd_halt_processors();

		/*
		 * Update the kernel state. ie. we're stopped now...
		 */
		kttd_stop_target();

		if (kttd_debug)
			printf("kttd_task_trap: stopping kernel target.\n");
	
		/*
		 * Turn off single stepping if this is a
		 * single step trap/breakpoint.
		 */
		if (kttd_single_stepping) {
			if (!kttd_clear_single_step(NULL))
				printf("ttd_trap: Couldn't clear single stepping!!\n");
		}

		/*
		 * Async entry already has request, so we need to get
		 * one to catch up.
		 */
		(void) kttd_get_request(&kttd_current_request,
					&kttd_current_length);

		/*
		 * Fill in the kttd globals to deal with this
		 * msg.  A NULL ptr means that we should use the
		 * ttd_request_msg.
		 */
		kttd_current_kmsg = NULL;
	}

	/*
	 * Update kernel target info (ie. why we're stopped, etc.)
	 */
	kttd_type_to_ttdtrap(type);

	/*
	 * The packet "command" loop, where we respond to remote
	 * debugging commands.
	 *
	 * Service request(s) now...
	 */

	if (kttd_debug)
		printf("kttd_task_trap: servicing requests\n");

	for(;;) {
		ttd_service_request();

		/*
		 * If the request came from a kmsg, we need
		 * to put it back onto the net_queue_free list.
		 */
		if (kttd_current_kmsg != NULL) {
			net_kmsg_put(kttd_current_kmsg);
			kttd_current_kmsg = NULL;
		}

		/*
		 * If we are in ONE_STOP mode, we're done
		 * with processing packets, so we should exit
		 * the "command" loop.
		 */
		if (kttd_run_status == ONE_STOP)
			break;

		/*
		 * We must be at a FULL_STOP, so we need to get
		 * another ttd packet, and fill in the globals.
		 *
		 * kttd_get_request sets the first parameter to point
		 * at the struct ttd_request in the ttd_request_msg.
		 */
		kttd_get_request(&kttd_current_request, &kttd_current_length);
		kttd_current_kmsg = NULL;
	}
}
