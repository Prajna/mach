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
 * The TTD message format.
 *
 * HISTORY:
 * $Log:	ttd_msg.h,v $
 * Revision 2.2  93/05/10  23:24:47  rvb
 * 	Checkin for MK80 branch.
 * 	[93/05/10  15:08:31  grm]
 * 
 * Revision 2.1.2.2  93/04/20  10:53:06  grm
 * 	Changed the types for a more universal protocol.  This checkin
 * 	contains protocol version 2.4.
 * 	[93/04/20            grm]
 * 
 * Revision 2.1.2.1  93/03/03  14:40:20  grm
 * 	Changed the structures and types.
 * 	[93/03/03            grm]
 * 
 * Revision 2.1.1.4  92/10/23  21:24:28  grm
 * 	Added single stepping error code.
 * 	[92/10/23            grm]
 * 
 * Revision 2.1.1.3  92/10/08  14:28:07  grm
 * 	Cosmetic changes.
 * 	[92/10/08            grm]
 * 
 * Revision 2.1.1.2  92/10/01  15:37:41  grm
 * 	KTTD restructuring checkpoint.
 * 	[92/10/01            grm]
 * 
 * Revision 2.1.1.1  92/09/25  15:11:04  grm
 * 	Initial Checkin.
 * 	[92/09/25            grm]
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

#ifndef	_TTD_MSG_H_
#define	_TTD_MSG_H_

#include "ttd_types.h"

/*********************************************/
/*  Topaz TeleDebug Protocol Message Format  */
/*********************************************/

typedef ttd_byte ttd_protocol_version_t;

/*
 * We got this from SRC as version 2, so I called this 21 (or 2.1)
 */
#define TTD_VERSION 24

/*
 * Made these 32 bit values for easier life with risc and 64bit archs.
 */
typedef uint32 ttd_operation;
typedef uint32 ttd_protocol_version;

enum {
	Okay,			/* Success                                */
	InvalidOperation,	/* No such operation                      */
	InvalidTarget,		/* Target does not exist                  */
	InvalidArgument,	/* Result.argNo identifies bad arg        */
	ServerNotAvailable,	/* Can't contact desired TTD server       */
	TargetNotAvailable,	/* Target in use or otherwise unavailable */
	MemoryReferenceFailed,	/* Can't make requested memory reference  */
	TooManyBreakpoints,	/* Can't set any more breakpoints         */
	OperationNotApplicable,	/* Can't apply operation in current state */
	TargetStopped,		/* 					  */
	TargetNotStopped,	/* 					  */
	SynchError,		/* 					  */
	TargetInLimbo,		/* Target doing VFork -- come back later  */
	TargetTimedOut,		/* Target held by stopped/crashed Taos    */
	ThreadInKernelCall,	/* Thread can't be modified during call   */ 
	SingleSteppingError	/* Problem with Single Stepping Mechanism */
};
typedef u_short ttd_code_t;

/*
 * Must be multiple of word (4 bytes) size.  Someday 8, 16, ...?
 */
typedef struct {
	ttd_code_t	code;
	u_short		argno;
} ttd_result;

enum ttd_operations {
	PROBE_SERVER			= 1,
	GET_TARGET_INFO			= 4,
	CONNECT_TO_TARGET		= 5,

	DISCONNECT_FROM_TARGET		= 6,
	READ_FROM_TARGET		= 7,
	WRITE_INTO_TARGET		= 8,
	GET_NEXT_THREAD			= 9,
	GET_THREAD_INFO			= 10,
	SET_THREAD_INFO			= 11,
	STOP_TARGET			= 12,
	PROBE_TARGET			= 13,
	RESTART_TARGET			= 14,
	SET_BREAKPOINT_IN_TARGET	= 15,
	CLEAR_BREAKPOINT_IN_TARGET	= 16,
	GET_NEXT_BREAKPOINT_IN_TARGET	= 17,
	SINGLE_STEP_THREAD		= 18
};

/*
 * The TTD Request Message Structure:
 */
struct ttd_request {
	ttd_server	server;
	ttd_seq		seq;
	ttd_target	target;
	ttd_operation	operation;
	union {
		/*** UNTARGETED ***/

		/* probe_server */

		struct {
			ttd_target	target;
		} get_target_info;

		struct { 
			ttd_target	target;
			ttd_key		key;
		} connect_to_target;

		/*** TARGETED ***/

		/* disconnect_from_target */

		struct { 
			ttd_address	start;
			ttd_count	count;
		} read_from_target;

		struct { 
			ttd_address	start;
			ttd_count	count;
			ttd_data_block	data;
		} write_into_target;

		struct {
			ttd_thread	thread;
		} get_next_thread;

		struct {
			ttd_thread	thread;
		} get_thread_info;

		struct {
			ttd_thread		thread;
			ttd_thread_info		thread_info;
			ttd_trap_info		trap_info;
			ttd_machine_state	machine_state;
		} set_thread_info;

		struct {
			ttd_thread	thread;
		} stop_target;

		/* probe_target */
		
		struct {
			ttd_thread	thread;
		} restart_target;
		
		struct {
			ttd_address	address;
			ttd_thread	thread;		/* Mach Addition */
			ttd_flavor	flavor;
		} set_breakpoint_in_target;
		
		struct {
			ttd_address	address;
			ttd_thread	thread;		/* Mach Addition */
		} clear_breakpoint_in_target;
		
		struct {
			ttd_address	address;
			ttd_boolean	all_breaks;	/* Mach Addition */
			ttd_thread	thread;		/* Mach Addition */
		} get_next_breakpoint_in_target;
		
		struct {
			ttd_thread	thread;
		} single_step_thread;

	} u;
};

typedef struct ttd_request * ttd_request_t;

/*
 * The Reply Message Structure:
 */
struct ttd_reply {
	ttd_server	server;
	ttd_seq		seq;
	ttd_target	target;
	ttd_result	result;
	ttd_operation	operation;
	union {
		/*** UNTARGETED ***/
		
		struct {
			ttd_protocol_version	version;
			ttd_machine_type	machine_type;
		} probe_server;

		struct { 
			ttd_target_info	target_info;
		} get_target_info;
		
		struct { 
			ttd_target	target;		/* Necessary? XXX */
			ttd_target_info	target_info;
		} connect_to_target;
		
		/*** TARGETED ***/
		
		/* disconnect_from_target */
		
		struct { 
			ttd_count	count;
			ttd_data_block	data;
		} read_from_target;
		
		/* write_into_target */
		
		struct {
			ttd_thread	next;
		} get_next_thread;
		
		struct {
			ttd_thread_info		thread_info;
			ttd_trap_info		trap_info;	/* Hmmm XXX */
			ttd_machine_state	machine_state;
		} get_thread_info;
		
		/* set_thread_info */
		
		/* stop_target */
		
		struct {
			ttd_target_info target_info;
		} probe_target;
		
		/* restart_target */
		
		struct {
			ttd_saved_inst	saved_inst;
		} set_breakpoint_in_target;
		
		/* clear_breakpoint_in_target */
		
		struct {
			ttd_address	address;
			ttd_flavor	flavor;
			ttd_saved_inst	saved_inst;
		} get_next_breakpoint_in_target;
		
		/* single_step_thread */

	} u;
};

typedef struct ttd_reply * ttd_reply_t;

#endif	/* _TTD_MSG_H_ */
