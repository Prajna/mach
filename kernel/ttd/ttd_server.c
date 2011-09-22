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
 * TTD Communications parsing code for the kernel ttd server.
 *
 * HISTORY:
 * $Log:	ttd_server.c,v $
 * Revision 2.2  93/05/10  23:24:51  rvb
 * 	Checkin for MK80 branch.
 * 	[93/05/10  15:08:55  grm]
 * 
 * Revision 2.1.2.3  93/04/20  11:06:02  grm
 * 	Changed types for use with a more universal protocol.  Added code
 * 	to support the use of multiple endian machines.  Alignment code
 * 	added.
 * 	[93/04/20            grm]
 * 
 * Revision 2.1.2.2  93/03/29  16:30:24  grm
 * 	Version for protocol 2.2
 * 	[93/03/29            grm]
 * 
 * Revision 2.1.2.1  93/03/03  14:41:22  grm
 * 	Second version of code.  It works.
 * 	[93/03/03            grm]
 * 
 * Revision 2.1.1.8  93/01/28  15:14:51  grm
 * 	Added ttd_loop_type.  Last checkin before locore rewrite.
 * 
 * Revision 2.1.1.7  93/01/22  15:52:52  grm
 * 	Added request length checks.
 * 
 * Revision 2.1.1.6  93/01/21  13:03:49  grm
 * 	Changed to Ansi C prototypes.  Added kttd_debug statements.
 * 
 * Revision 2.1.1.5  92/10/30  12:44:18  grm
 * 	Fixed single stepping and set_state_action bug.
 * 	[92/10/30            grm]
 * 
 * Revision 2.1.1.4  92/10/23  21:21:39  grm
 * 	Added first pass at single stepping code.  Fixed bug in
 * 	stop/restart logic.
 * 	[92/10/23            grm]
 * 
 * Revision 2.1.1.3  92/10/08  14:29:28  grm
 * 	Fixed clear_breakpoint function.  Added some debugging code.
 * 	[92/10/08            grm]
 * 
 * Revision 2.1.1.2  92/10/01  15:36:54  grm
 * 	KTTD restructuring checkpoint.
 * 	[92/10/01            grm]
 * 
 * Revision 2.1.1.1  92/09/25  15:10:26  grm
 * 	Initial checkin.
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

#include <mach/boolean.h>
#include <mach/vm_param.h>

#include <ttd/ttd_comm.h>
#include <ttd/ttd_types.h>
#include <ttd/ttd_msg.h>
#include <ttd/kernel_ttd.h>
#include <ttd/ttd_stub.h>
#include <ttd/ttd_server.h>
#include <ttd/ttd_debug.h>

struct ttd_reply	prev_reply;         /* saved reply for idempotency */
natural_t		prev_reply_length;
ttd_operation		prev_operation;

static boolean_t	ttd_server_initialized;

#define END_ADDRESS(x) ((ttd_address)((natural_t)&(x) + sizeof(x)))

#define TTD_KMSG(kmsg)	((kmsg == NULL) ? ttd_request_msg : kmsg)

#define target_is_kernel(target) (target == TTD_KERNEL_MID)

#define target_stopped() kttd_target.is_stopped

#define valid_count(count) (count <= TTD_MAX_BLOCK_SIZE)

/*
 * Aligned TTD Request.
 */
struct ttd_request aligned_request;

/*
 * Kernel target state:
 */
boolean_t		kttd_single_stepping = FALSE;
ttd_seq			kttd_current_seq;

kttd_breakpoint		kttd_breaktable[KTTD_MAXBPT];
ttd_target_info		kttd_target;

ttd_machine_type	ttd_target_machine_type;

/*
 * Used by the ttd_stub code. 
 */
void kttd_stop_target(void)
{
	kttd_target.is_stopped = TRUE;
}

/*
 * Used by the machine dependent code for single stepping.
 */
boolean_t break_set(ttd_address addr,
		    ttd_saved_inst *inst)
{
	kttd_breakno b;
	
	for (b = 0; b < KTTD_MAXBPT; b++) {
		if ((!kttd_breaktable[b].free) &&
		    (kttd_breaktable[b].address == addr)) {
			*inst = kttd_breaktable[b].saved_inst;
			return TRUE; 
		}
	}
	return FALSE;
}

/*
 * Misc. routines for this module:
 */
static boolean_t find_break(ttd_address addr,
		     ttd_thread thread,
		     kttd_breakno *bn)
{
	kttd_breakno b;
	
	for (b = 0; b < KTTD_MAXBPT; b++) {
		if ((!kttd_breaktable[b].free) &&
		    (kttd_breaktable[b].address == addr) &&
		    (kttd_breaktable[b].thread == thread)) {
			*bn = b;
			return TRUE; 
		}
	}
	return FALSE;
}

static boolean_t find_free_breakentry(kttd_breakno *breakno)
{
	kttd_breakno	b;

	for(b = 0; b < KTTD_MAXBPT; b++) {
		if (kttd_breaktable[b].free) {
			*breakno = b;
			return TRUE;
		}
	}
	return FALSE;
}

static boolean_t valid_thread(ttd_thread thread)
{
	/* XXX Fix... */

	return TRUE;
}

static void init_break_table(void)
{
	kttd_breakno bn;
	
	for (bn = 0; bn < KTTD_MAXBPT; bn++)
		kttd_breaktable[bn].free = TRUE;
}

static void init_kernel_target(void)
{
	kttd_target.is_targeted = FALSE;
	kttd_target.is_stopped = FALSE;
	kttd_target.trapped_thread = 0;

	kttd_current_seq = 0x7fffffff;

	init_break_table();

	kttd_single_stepping = FALSE;
}

/*
 * Check to see if this message is a duplicate.  Updates
 * the current sequence number as a side effect.
 */
static boolean_t duplicate(ttd_seq s)
{
	boolean_t dup;
	
	dup = (kttd_current_seq == s);
	if (kttd_debug && (s < kttd_current_seq))
		printf("TTD:duplicate, SRC code bites us here.\n");
	kttd_current_seq = s;
				
	return dup;
}

/*
 * Note: since the kernel is the only kttd target, the target is
 * implicit.
 */
static void get_kernel_target_info(ttd_target_info *ti)
{
	ti->is_stopped = kttd_target.is_stopped;
	ti->is_targeted = kttd_target.is_targeted;
	ti->trapped_thread = 1;		/* XXX Fix */
	ti->debug_reason.length = 0;
#if	NOT_NOW
	strcpy("Hi!", ti->debug_reason.chars);
#endif	/* NOT_NOW */
}

/*******************/
/* Action Routines */
/*******************/

/*
 * Read and write to kernel VM.
 */
static ttd_code_t read_write_action(vm_prot_t	access,
				    ttd_address	mem_ptr,
				    ttd_address	buffer,
				    ttd_count	length)
{
	vm_size_t	size;

	/*
	 *	handle this in pages
	 */
	while (length > 0) {
		/*
		 *	get the number of bytes that we can
		 *	access in this page
		 */
		size = trunc_page(mem_ptr + PAGE_SIZE) - mem_ptr;
		if (length < size)
			size = length;
		
		if (!kttd_mem_access(mem_ptr, access)) {
			if (kttd_debug)
				printf("can't read memory\n");
			return MemoryReferenceFailed;
		}
		
		/*
		 *	we can move anything on this page
		 */
		if (access & VM_PROT_WRITE) {
			bcopy(buffer, mem_ptr, size);
			kttd_flush_cache(mem_ptr, size);
		}else if (access & VM_PROT_READ) {
			bcopy(mem_ptr, buffer, size);
		}else{
			return InvalidArgument;
		}

		mem_ptr += size;
		buffer += size;
		length -= size;
	}

	return Okay;
}

/*
 * XXX Fix it.
 */
static boolean_t get_next_thread_action(ttd_thread *thread)
{
#if	0
	boolean_t found_prev;
	hardware_processor_number p;
	
	found_prev = (*thread == 0);
	for (p = 0; p < hardware_max_processors; p++) {
		if (ISIN (heaP->up, p)) {
			if (found_prev) {
				*thread = current_thread(p);
				return TRUE;
			}
			found_prev = (*thread == Current_thread(p));
		}
	}
	*thread = 0;
	return found_prev;
#endif
}

static void get_state_action(ttd_thread		thread,
			     ttd_thread_info	*thread_info,
			     ttd_trap_info	*trap_info,
			     ttd_machine_state	*machine_state)
{
	/*
	 * Don't do the thread_info, or trap_info now. XXX Fix it?
	 */
	kttd_machine_getregs(machine_state);
}

static void set_state_action(ttd_thread		thread,
			     ttd_thread_info	*thread_info,
			     ttd_trap_info	*trap_info,
			     ttd_machine_state	*machine_state)
{
	/*
	 * Don't do the thread_info, or trap_info now either. XXX Fix it?
	 */

	kttd_machine_setregs(machine_state);
}

static void insert_break(kttd_breakno	bn,
			 ttd_address	addr,
			 ttd_thread	thread,
			 ttd_flavor	flavor,
			 ttd_saved_inst	saved_inst)
{
	kttd_breaktable[bn].free	= FALSE;
	kttd_breaktable[bn].address	= addr;
	kttd_breaktable[bn].thread	= thread;
	kttd_breaktable[bn].flavor	= flavor;
	kttd_breaktable[bn].saved_inst	= saved_inst;
}

static ttd_code_t set_break_action(ttd_address	addr,
				   ttd_thread	thread,
				   ttd_flavor	flavor,
				   ttd_saved_inst	*saved_inst)
{
	kttd_breakno	bn;
	boolean_t	found = FALSE;

	/*
	 * Check to see if a breakpoint is already set there for a
	 * different thread.
	 */
	for(bn = 0; bn < KTTD_MAXBPT; bn++) {
		if (!kttd_breaktable[bn].free &&
		    (kttd_breaktable[bn].address == addr)) {
			*saved_inst = kttd_breaktable[bn].saved_inst;
			found = TRUE;
			break;
		}
	}

	/*
	 * Get a free table entry.
	 */
	if (!find_free_breakentry(&bn))
		return TooManyBreakpoints;
	
	/*
	 * Insert into table if already have saved_inst.
	 */
	if (found) {
		insert_break(bn, addr, thread, flavor, *saved_inst);
		return Okay;
	}

	/*
	 * Fault in memory if not available.
	 */
	if (!kttd_mem_access(addr, VM_PROT_WRITE))
		return MemoryReferenceFailed;

	/*
	 * Insert the breakpoint into physical memory and flush the
	 * cache.
	 */
	if (!kttd_insert_breakpoint(addr, saved_inst))
		return MemoryReferenceFailed;

	kttd_flush_cache(addr, sizeof(ttd_saved_inst));
	
	/*
	 * Insert breakpoint into table.
	 */

	insert_break(bn, addr, thread, flavor, *saved_inst);

	if (kttd_debug) {
		printf("Inserting breakpoint into table at bn = %d\n",bn);
	}

	return Okay;
}

static ttd_code_t clear_break_action(ttd_address addr, ttd_thread thread)
{
	kttd_breakno	b;
	kttd_breakno	bn = 0;
	ttd_count	count = 0;
	ttd_saved_inst	saved_inst;
	boolean_t	found = FALSE;
	
	/*
	 * Cycle through breaktable.  If more than one breakpoint
	 * at addr, we know not to replace memory with saved_inst.
	 */
	for(b = 0; b < KTTD_MAXBPT; b++) {
		if (!kttd_breaktable[b].free &&
		    (kttd_breaktable[b].address == addr)) {
			if (kttd_breaktable[b].thread == thread) {
				found = TRUE;
				bn = b;
			}
			if (++count > 1) {
				break;
			}
		}
	}

	/*
	 * Didn't find breakpoint in table, return
	 */
	if (!found) {
		if (kttd_debug) {
			printf("Couldn't find breakpoint in table.\n");
		}
		return InvalidArgument;
	}

	/*
	 * Only one breakpoint at that address, fault in memory
	 * and clear the breakpoint.
	 */
	if (count == 1) {
		if (!kttd_mem_access(addr, VM_PROT_WRITE))
			return MemoryReferenceFailed;
		
		saved_inst = kttd_breaktable[bn].saved_inst;

		if (!kttd_remove_breakpoint(addr, saved_inst))
			return MemoryReferenceFailed;
	}

	/*
	 * Free entry in breaktable.
	 */
	kttd_breaktable[bn].free = TRUE;

	return Okay;
}

static ttd_code_t get_next_break_action(boolean_t	all_breaks,
					ttd_address	*addr,
					ttd_thread	*thread,
					ttd_flavor	*flavor,
					ttd_saved_inst	*saved_inst)
{
	kttd_breakno bn;
	kttd_breakno start;

	/*
	 * If addr is zero, then start from beginning of list.
	 * Otherwise, start from the breakno after addr's breakno.
	 */
	if (*addr == 0) {
		start = 0;
	}else{
		/*
		 * Find the first matching breakpoint entry.
		 */
		for(bn = 0; bn < KTTD_MAXBPT; bn++) {
			if (!kttd_breaktable[bn].free &&
			    (kttd_breaktable[bn].address == *addr) &&
			    (kttd_breaktable[bn].thread == *thread))
				break;
		}

		/*
		 * If ran off the end, it's an invalid argument.
		 * Even all_breaks will have a valid addr and thread.
		 */
		if (!all_breaks && (bn > KTTD_MAXBPT))
			return InvalidArgument;

		if (bn < KTTD_MAXBPT - 1) {
			start = bn + 1;
		}else{
			/*
			 * Ran off the end.  No more breakpoints
			 * in the table.  Return zeros in vars.
			 */
			*addr = 0;
			*flavor = 0;
			bzero(saved_inst, sizeof(ttd_saved_inst));
			return Okay;
		}
	}

	for(bn = start; bn < KTTD_MAXBPT; bn ++) {
		if (!kttd_breaktable[bn].free &&
		    (all_breaks ||
		     (kttd_breaktable[bn].thread == *thread))) {
			*addr		= kttd_breaktable[bn].address;
			*thread		= kttd_breaktable[bn].thread;
			*flavor		= kttd_breaktable[bn].flavor;
			*saved_inst	= kttd_breaktable[bn].saved_inst;
			return Okay;
		}
	}
	*addr = 0;
	*flavor = 0;
	bzero(saved_inst, sizeof(ttd_saved_inst));

	return Okay;
}

/*
 * The Operations:
 */

/*
 * probe_server:
 *
 *  Probe server returns the version number of the KTTD implementation
 * and the machine type that it is executing on.
 *
 */
static void probe_server(ttd_request_t	request,
			 ttd_reply_t	reply,
			 ttd_address	*reply_end)
{
	if (kttd_debug) {
		printf("TTD:Probe Server, version = %d, machine_type = %d\n",
		       TTD_VERSION, ttd_target_machine_type);
	}

	reply->u.probe_server.version = netswap_4_bytes(TTD_VERSION);
	reply->u.probe_server.machine_type = netswap_4_bytes(ttd_target_machine_type);
	*reply_end = END_ADDRESS (reply->u.probe_server);
}

/*
 * get_target_info:
 *
 *  Get target info returns the target_info struct for the given target.
 * Since this is the kttd implementation, the only valid target is the
 * kernel target (ie. it only returns the target info for the kernel.
 *
 */
static void get_target_info(ttd_request_t	request,
			    ttd_reply_t	 	reply,
			    ttd_address		*reply_end)
{
	ttd_target	target;
	ttd_target_info	target_info;

	target = request->u.get_target_info.target;
	if (!target_is_kernel(target)) {
		reply->result.code = InvalidTarget;

		if (kttd_debug) {
			printf("TTD:get_target_info, invalid target %d\n", target);
		}

		return;
	}

	get_kernel_target_info(&target_info);

	reply->u.get_target_info.target_info = target_info;
	*reply_end = END_ADDRESS (reply->u.get_target_info);
	
	if (kttd_debug) {
		printf("TTD:get_target_info, target %d is %s, %s, th = 0x%x\n",
		       target,
		       target_info.is_stopped ? "stopped" : "running",
		       target_info.is_targeted ? "targeted" : "untargeted",
		       target_info.trapped_thread);
	}
}

/*
 * connect_to_target:
 *
 *  Connect to target connects to the target.  The only valid target
 * in the kttd implemetation is the kernel, all other targets are
 * invalid.  If the kernel is already targeted, it can only succeed
 * if the key value that is passed in the request message is valid.
 *
 * Note:  In this early implementation of kttd there is no security
 *        built into the ttd protocol (ie. a value of MASTER_KEY
 *        allows anyone to override the current connection).
 *
 */
static void connect_to_target(ttd_request_t	request,
			      ttd_reply_t	reply,
			      ttd_address	*reply_end)
{
	ttd_target	target;
	ttd_key		key;
	ttd_target_info	target_info;

	target = request->u.connect_to_target.target;
	key = request->u.connect_to_target.key;

	if (!target_is_kernel(target)) {
		reply->result.code = InvalidTarget;
		if (kttd_debug)
			printf("TTD:connect_to_target, invalid target %d\n", target);
		return;
	} else if (kttd_target.is_targeted && (key != MASTER_KEY)) {
		reply->result.code = TargetNotAvailable;
		if (kttd_debug)
			printf("TTD:connect_to_target, target %d unavailable\n", target);
		return;
	}

	kttd_target.is_targeted = TRUE;
	kttd_current_seq = 0;

	get_kernel_target_info(&target_info);

	reply->u.connect_to_target.target = TTD_KERNEL_MID;
	reply->u.connect_to_target.target_info = target_info;
	reply->result.argno = 2;
	*reply_end = END_ADDRESS (reply->u.connect_to_target);

	if (kttd_debug)
		printf("TTD:connect_to_target, connected kttd to target %d\n",target);
}

/*
 * disconnect_from_target:
 *
 *  Disconnect from target disconnects from the specified target.
 * The kernel target is the only valid target in the kttd implemen-
 * tation.
 *
 * Note:  This routine does not implicitly restart the target.
 *
 */
static void disconnect_from_target(ttd_request_t request,
				   ttd_reply_t 	 reply,
				   ttd_address	 *reply_end)
{
	/*
	 * The target checking is done in the kttd_service_request
	 * routine.  We know the target is the kernel if we get this
	 * far.
	 */
	kttd_target.is_targeted = FALSE;

	if (kttd_debug)
		printf("TTD:disconnect_from_target, disconnected from kttd target\n");
}	

/*
 * read_from_target:
 *
 *  Read from target reads count number of bytes from the target's
 * address space (the kernel in this implementation) and places them
 * in the reply message's buffer.
 *
 */
static void read_from_target(ttd_request_t	request,
			     ttd_reply_t	reply,
			     ttd_address	*reply_end)
{
	ttd_address	start;
	ttd_count		count;
	ttd_address	buffer;

	/*
	 * Target must be stopped in order to do this operation.
	 */
	if (!target_stopped()) {
		reply->result.code = TargetNotStopped;
		if (kttd_debug)
			printf("TTD:read_from_target, target not stopped.\n");
		return;
	}

	start = (ttd_address) request->u.read_from_target.start;
	count = request->u.read_from_target.count;
	buffer = (ttd_address) &reply->u.read_from_target.data[0];

	if (!valid_count(count)) {
		reply->result.code = InvalidArgument;
		if (kttd_debug)
			printf("TTD:read_from_target, invalid count 0x%x!!\n",
			       count);
		return;
	}

#if	VERBOSE
	if (kttd_debug)
		printf("TTD:read_from_target, start= 0x%x count=0x%x,",
		       start, count);
#endif	/* VERBOSE */

	reply->result.code = read_write_action(VM_PROT_READ, start, buffer, count);

	if (reply->result.code == Okay) {
		reply->u.read_from_target.count = count;
		*reply_end = (ttd_address)
			&reply->u.read_from_target.data[count];
		reply->result.argno = 2;

#if	VERBOSE
		if (kttd_debug)
			printf("OK! readbytes=0x%x\n",count);
#endif	/* VERBOSE */
	}
}

/*
 * write_info_target:
 *
 *  Write into target writes count bytes into the target's address
 * space (the kernel's address space) at the address addr from the
 * request message's buffer.
 *
 */
static void write_into_target(ttd_request_t	request,
			      ttd_reply_t	reply,
			      ttd_address	*reply_end)
{
	ttd_address	start;
	ttd_count	count;
	ttd_address	buffer;

	/*
	 * Target must be stopped in order to do this operation.
	 */
	if (!target_stopped()) {
		reply->result.code = TargetNotStopped;
		if (kttd_debug)
			printf("TTD:write_into_target, target not stopped\n");
		return;
	}

	start = (ttd_address) request->u.write_into_target.start;
	count = request->u.write_into_target.count;
	buffer = (ttd_address) &request->u.write_into_target.data[0];

	if (!valid_count(count)) {
		reply->result.code = InvalidArgument;
		if (kttd_debug)
			printf("TTD:write_into_target, invalid count 0x%x\n",
			       count);
		return;
	}

	reply->result.code = read_write_action(VM_PROT_WRITE, start, buffer, count);

	if (kttd_debug)
		printf("TTD:write_into_target, start=0x%x count=0x%x, result = %s\n",
		       start, count, (reply->result.code == Okay) ? "OK" : "ERR");
}

/*
 * get_next_thread:
 *
 *  Get next thread returns the next thread in the taks's (target's)
 * thread list starting at the thread in the request message.  If the
 * request thread's value is NULL, the first thread in the task's
 * thread list is returned, otherwise we return the next thread.
 *
 */
static void get_next_thread(ttd_request_t	request,
			    ttd_reply_t		reply,
			    ttd_address		*reply_end)
{
	ttd_thread	thread;

	/*
	 * Target must be stopped in order to do this operation.
	 */
	if (!target_stopped()) {
		reply->result.code = TargetNotStopped;
		if (kttd_debug)
			printf("TTD:get_next_thread, target not stopped\n");
		return;
	}

	thread = request->u.get_next_thread.thread;

	if (!get_next_thread_action(&thread)) {
		reply->result.code = InvalidArgument;
		return;
	}

	reply->u.get_next_thread.next = thread;
	reply->result.argno = 1;
	*reply_end = END_ADDRESS (reply->u.get_next_thread);

	if (kttd_debug)
		printf("TTD:get_next_thread, orig= 0x%x, thread=0x%x\n",
		       request->u.get_next_thread.thread, thread);
}

/*
 * get_thread_info:
 *
 *  Get thread info returns information about the thread specified
 * in the request message.  There are three parts to the thread info:
 * 
 *	thread_info:	the thread's state (ie. what's returned by
 *			thread_getstatus).
 *
 *	trap_info:	information on which trap caused the thread
 *			to trap (if it is stopped).
 *
 *	machine_state:	the thread's register state.
 *
 * Note:  In this implementation, we only support the machine state.
 *
 */
static void get_thread_info(ttd_request_t	request,
			    ttd_reply_t		reply,
			    ttd_address		*reply_end)
{
	ttd_thread	thread;
	ttd_thread_info	thread_info;
	ttd_trap_info	trap_info;
	ttd_machine_state machine_state;

	/*
	 * Target must be stopped in order to do this operation.
	 */
	if (!target_stopped()) {
		reply->result.code = TargetNotStopped;
		if (kttd_debug)
			printf("TTD:get_thread_info, target not stopped.\n");
		return;
	}

	thread = request->u.get_thread_info.thread;

	if (!valid_thread(thread)) {
		reply->result.code = InvalidArgument;
		if (kttd_debug)
			printf("TTD:get_thread_info, invalid thread.\n");
		return;
	}

	get_state_action(thread, &thread_info, &trap_info, &machine_state);

	if (kttd_debug)
		printf("TTD:get_thread_info, thread= 0x%x, ...\n", thread);
		
	reply->u.get_thread_info.thread_info = thread_info;
	reply->u.get_thread_info.trap_info = trap_info;
	reply->u.get_thread_info.machine_state = machine_state;
	reply->result.argno = 3;
	*reply_end = END_ADDRESS (reply->u.get_thread_info);
}

/*
 * set_thread_info:
 *
 *  Set thread info sets the specified thread's three states to
 * parameters in the request message.  These thread states are the
 * ones outlined above in the get_thread_info call.
 *
 * Note:  Only the thread's machine_state is set in this implemen-
 *        tation.
 *
 */
static void set_thread_info(ttd_request_t	request,
			    ttd_reply_t		reply,
			    ttd_address		*reply_end)
{
	ttd_thread	thread;
	ttd_thread_info	*thread_info;
	ttd_trap_info	*trap_info;
	ttd_machine_state *machine_state;

	/*
	 * Target must be stopped in order to do this operation.
	 */
	if (!target_stopped()) {
		reply->result.code = TargetNotStopped;
		if (kttd_debug)
			printf("TTD:set_thread_info, target not stopped.\n");
		return;
	}

#if	FUTURE
	thread = request->u.set_thread_info.thread;
	thread_info = &(request->u.set_thread_info.thread_info);
	trap_info = &(request->u.set_thread_info.trap_info);
#endif	/* FUTURE */
	machine_state = &(request->u.set_thread_info.machine_state);

#if	FUTURE
	if (!valid_thread(thread)) {
		reply->result.code = InvalidArgument;
		if (kttd_debug)
			printf("TTD:set_thread_info, invalid thread.\n");
		return;
	}
#endif	/* FUTURE */

	set_state_action(thread, thread_info, trap_info, machine_state);

	if (kttd_debug)
		printf("TTD:set_thread_info, thread= 0x%x, ...\n");
}

/*
 * stop_target:
 *
 *  Stop target stops the target specified in the request message.
 * In order to issue this command, the client must have successfull
 * issued an attach_to_target request previous to this request.
 *
 * Note:  this command was not directly supported by the original
 *        NubTTD implementation.  It is supported by kttd since
 *        the kttd client can communicate with the kttd server
 *        asynchronously.
 *
 * Note:  This implementation stops all threads in a task.  Future
 *        versions will work on a per-thread basis.
 *
 */
static void stop_target(ttd_request_t	request,
			ttd_reply_t	reply,
			ttd_address	*reply_end)
{
	/*
	 * Return error message if already stopped.
	 */
	if (target_stopped()) {
		reply->result.code = TargetStopped;
		if (kttd_debug)
			printf("TTD:stop_target, target ALREADY stopped.\n");
		return;
	}

	/*
	 * All we need to do to stop the kernel is call
	 * kttd_break.  This will cause this "thread" to
	 * enter the kttd_handle_sync() routine which will:
	 *
	 * 1.  Halt all the processors.
	 * 
	 * 2.  Stop the kernel (kttd_target.is_stopped = TRUE).
	 *
	 */

	if (kttd_debug)
		printf("TTD:stop_target, stopping kernel.\n");

	kttd_halt_processors();

	kttd_target.is_stopped = TRUE;

#if	PRE_MIPS_CODE
	kttd_stop_status = FULL_STOP;
#else
	kttd_run_status = FULL_STOP;
#endif	/* PRE_MIPS_CODE */
}

/*
 * probe_target:
 *
 *  Probe target returns the target info of the current target.  This
 * is used extensively by the asynhronous client.  In general the client
 * will poll the kttd target until it is stopped, and only then issue
 * requests.
 *
 */
static void probe_target(ttd_request_t	request,
			 ttd_reply_t	reply,
			 ttd_address	*reply_end)
{
	ttd_target_info	target_info;

	get_kernel_target_info(&target_info);
	reply->u.probe_target.target_info = target_info;
	reply->result.argno = 1;
	*reply_end = END_ADDRESS (reply->u.probe_target);

	if (kttd_debug)
		printf("TTD:probe_target, Kernel target is %s, %s, th = 0x%x\n",
		       target_info.is_stopped ? "stopped" : "running",
		       target_info.is_targeted ? "targeted" : "untargeted",
		       target_info.trapped_thread);
}

/*
 * restart_target:
 *
 *  Restart target resume's the kttd target's (the kernel's) execution.
 *
 * Note:  The current implementation restarts all a tasks threads at once.
 *        Future versions will work on a per thread basis.
 *
 */
static ttd_response_t restart_target(ttd_request_t	request,
				     ttd_reply_t	reply,
				     ttd_address	*reply_end)
{
	ttd_thread	thread;

	/*
	 * If target already running return error code.
	 */
	if (!target_stopped()) {
		reply->result.code = TargetNotStopped;
		if (kttd_debug)
			printf("TTD:restart_target, target not stopped.\n");
		return SEND_REPLY;
	}

	if (kttd_debug)
		printf("TTD:restart_target, restarting target....\n");

	/*
	 * We don't need to save a message for duplicate replies since
	 * restart doesn't send a reply.  We have a check when we check
	 * for duplicates that determines if it's a duplicate restart packet.
	 * If it is, then we just ignore it.
	 *
	 * The same holds for duplicate single_step packets.
	 */

	/* This will make it restart. */
#if	PRE_MIPS_CODE
	kttd_stop_status = ONE_STOP;
#else
	kttd_run_status = ONE_STOP;
#endif	/* PRE_MIPS_CODE */

	kttd_target.is_stopped = FALSE;

#if	SECOND_ATTEMPT
	/* Restart operations do not expect a reply. */
	return NO_REPLY;
#else
	/*
	 * Second way didn't send reply now, but third way will.
	 * We'll cache the reply and send it whenever we receive
	 * a duplicate!  Duhhhhhh....
	 */
	return SEND_REPLY;
#endif	/* SECOND_ATTEMPT */
}

/*
 * set_breakpoint_in_target
 *
 *  Set breakpoint in target sets a breakpoint for a specified thread in
 * in the current task's (the kernel task) at a specified address with
 * a specified flavor.  If the thread is NULL, it applies to all threads
 * in the target's task (all kernel threads).
 *
 */
static void set_breakpoint_in_target(ttd_request_t	request,
				     ttd_reply_t	reply,
				     ttd_address	*reply_end)
{
	ttd_address	addr;
	ttd_thread	thread;
	ttd_flavor	flavor;
	kttd_breakno	bn;
	ttd_saved_inst	saved_inst;

	/*
	 * Target must be stopped in order to do this operation.
	 */
	if (!target_stopped()) {
		reply->result.code = TargetNotStopped;
		if (kttd_debug)
			printf("TTD:set_breakpoint, target not stopped.\n");
		return;
	}

	addr = (ttd_address) request->u.set_breakpoint_in_target.address;
	thread = (ttd_thread) request->u.set_breakpoint_in_target.thread;
	flavor = request->u.set_breakpoint_in_target.flavor;

	if (thread && !valid_thread(thread)) {
		reply->result.code = InvalidArgument;
		if (kttd_debug) {
			printf("TTD:set_breakpoint, Invalid thread.\n");
		}
		return;
	}
		
	if (kttd_debug) {
		printf("TTD:set_breakpoint, addr= 0x%x, thread= 0x%x, flavor= %d, ",
		       addr, thread, flavor);
	}

	if (find_break(addr, thread, &bn)) {
		kttd_breaktable[bn].flavor = flavor;
		saved_inst = kttd_breaktable[bn].saved_inst;
	}else{
		reply->result.code = set_break_action(addr, thread,
						      flavor, &saved_inst);
	}

	if (!reply->result.code == Okay) {
		if (kttd_debug)
			printf("ERR\n");
		return;
	}

	if (kttd_debug)
		printf("OK\n");

	reply->u.set_breakpoint_in_target.saved_inst = saved_inst;
	reply->result.argno = 1;
	*reply_end = END_ADDRESS(reply->u.set_breakpoint_in_target);
}

/*
 * clear_breakpoint_in_target:
 *
 *  Clear breakpoint in target removes the breakpoint from target's
 * breakpoint table.  Like the set breakpoint request above, it takes
 * an address and a thread.  If the thread is NULL, it only clears
 * the breakpoints that apply to all threads.
 *
 */
static void clear_breakpoint_in_target(ttd_request_t	request,
				       ttd_reply_t	reply,
				       ttd_address	*reply_end)
{
	ttd_address	addr;
	ttd_thread	thread;

	/*
	 * Target must be stopped in order to do this operation.
	 */
	if (!target_stopped()) {
		reply->result.code = TargetNotStopped;
		if (kttd_debug)
		       printf("TTD:clear_break, target not stopped.\n");
		return;
	}

	addr = (ttd_address)request->u.clear_breakpoint_in_target.address;
	thread = (ttd_thread)request->u.clear_breakpoint_in_target.thread;

	if (kttd_debug)
		printf("TTD:clear_breakpoint, addr= 0x%x, thread= 0x%x\n",
		       addr, thread);

	/*
	 * Doesn't have to be a legal thread.  Might want to clear a
	 * breakpoint for a thread that has been destroyed.  The
	 * clear break action procedure will ignore threads that don't
	 * have breakpoints in the table.
	 */
	reply->result.code = clear_break_action(addr, thread);
}

/*
 * get_next_breakpoint_in_target:
 *
 *  Get next breakpoint in target returns the next breakpoint in the
 * target's (the kernel's) breakpoint list.  This request returns the
 * next breakpoint with respect to the breakpoint passed in the request.
 * If the request breakpoint is NULL, it starts at the beginning of the
 * breakpoint list.  If all_breaks is TRUE it returns the next breakpoint
 * without regard to the breakpoint's associated thread.  If all_breaks
 * is FALSE it only returns the next breakpoint that is associated with
 * the thread specified in the request message.
 *
 * Note:  This request can be issued on a running target.
 *
 */
static void get_next_breakpoint_in_target(ttd_request_t	request,
					  ttd_reply_t	reply,
					  ttd_address	*reply_end)
{
	ttd_address	addr;
	ttd_thread	thread;
	ttd_flavor	flavor;
	ttd_saved_inst	saved_inst;
	boolean_t	all_breaks;
		
	addr = (ttd_address)request->u.get_next_breakpoint_in_target.address;
	thread = (ttd_thread)request->u.get_next_breakpoint_in_target.thread;
	all_breaks = request->u.get_next_breakpoint_in_target.all_breaks;

	if (kttd_debug)
		printf("TTD:get_break, address= 0x%x, thread= 0x%x, allbreaks= 0x%x\n",
		       addr, thread, all_breaks);

	reply->result.code = get_next_break_action(all_breaks, &addr, &thread,
						   &flavor, &saved_inst);

	if (reply->result.code != Okay)
		return;

	reply->u.get_next_breakpoint_in_target.address = (ttd_address) addr;
	reply->u.get_next_breakpoint_in_target.flavor = flavor;
	reply->u.get_next_breakpoint_in_target.saved_inst = saved_inst;
	*reply_end = END_ADDRESS (reply->u.get_next_breakpoint_in_target);
}

/*
 * kttd_single_step:
 *
 *  KTTD Single Step sets the machine independent single stepping
 * values to single stepping state.  It then calls the machine 
 * dependent code to turn the machine into single stepping mode.
 *
 */
boolean_t kttd_single_step(void)
{
	if (kttd_single_stepping) {
		printf("kttd_single_step:  Already Single stepping!!!\n");
		return FALSE;
	}

	kttd_single_stepping = TRUE;

	/*
	 * In the current implementation, we can only set and
	 * clear single step in the kernel task.
	 */
	return(kttd_set_machine_single_step(NULL));
}

/*
 * kttd_clear_single_step:
 *
 *  KTTD Clear Single Step clear the machine independent single stepping
 * mechanism, and takes the machine out of single stepping mode.
 *
 */
boolean_t kttd_clear_single_step(void)
{
	if (!kttd_single_stepping) {
		printf("kttd_clear_single_step: Already out of single stepping!!\n");
	}

	kttd_single_stepping = FALSE;

	/*
	 * In the current implementation, we can only set and
	 * clear single step in the kernel task.
	 */
	return (kttd_clear_machine_single_step(NULL));
}

boolean_t kttd_in_single_step(void)
{
	return (kttd_single_stepping);
}

/*
 * single_step_thread:
 *
 *  Single step thread starts a thread specified in the request message
 * executing in single step mode.
 *
 * Note:  In this implementation you can only single step the current
 *        thread.
 *
 */
static ttd_response_t single_step_thread(ttd_request_t	request,
					 ttd_reply_t	reply,
					 ttd_address	*reply_end)
{
	ttd_thread	thread;

	/*
	 * Target must be stopped in order to do this operation.
	 */
	if (!target_stopped()) {
		reply->result.code = TargetNotStopped;
		if (kttd_debug)
			printf("TTD:single_step, target not stopped.\n");
		return SEND_REPLY;
	}

#if	DO_LATER
	thread = request->u.single_step_thread.thread;

	if (!valid_thread(thread)) {
		reply->result.code = InvalidArgument;
		return SEND_REPLY;
	}
#endif	/* DO_LATER */

	/*
	 * Note:  in this implementation a thread is assumed to
	 *        be stopped if it's target (task) is not running,
	 *        make sure the check that the thread is stopped
	 *        when the implementation is changed to allow 
	 *        thread stoppages.
	 */

	/*
	 * What should happen here:
	 *
	 * 1. swap in the specified thread.
	 *
	 */

	if (!kttd_single_step()) {
		reply->result.code = SingleSteppingError;
		if (kttd_debug)
			printf("TTD:single_step, single stepping ERROR!\n");
		return SEND_REPLY;
	}

	if (kttd_debug)
		printf("TTD:single_step, single step.\n");


	/* This will restart the kernel */
#if	PRE_MIPS_CODE
	kttd_stop_status = ONE_STOP;
#else
	kttd_run_status = ONE_STOP;
#endif	/* PRE_MIPS_CODE */

	kttd_target.is_stopped = FALSE;

#if	SECOND_ATTEMPT
	return NO_REPLY;
#else
	/* See comments in restart_target. */
	return SEND_REPLY;
#endif	/* SECOND_ATTEMPT */
}

/*
 * ttd_decode_request:
 *
 *  Returns TRUE if a reply should be sent, FALSE if no reply
 * should be sent.
 *
 */
static ttd_response_t ttd_decode_request(ttd_request_t	request,
					 ttd_reply_t	reply,
					 natural_t *	reply_length)
{
	ttd_address	reply_end;
	boolean_t	targeted_op;
	ttd_response_t	sr;

	/*
	 * Convert from network to host byte ordering.
	 */
	request->server = netswap_4_bytes(request->server);
	request->seq = netswap_4_bytes(request->seq);
	request->target = netswap_4_bytes(request->target);
	request->operation = netswap_4_bytes(request->operation);

	targeted_op = request->operation > CONNECT_TO_TARGET;

	/*
	 * Setup the reply's return values that are common
	 * to all operations:
	 */
	reply->server = netswap_4_bytes(KERNEL_TTD);
	reply->target = netswap_4_bytes(TTD_KERNEL_MID);
	reply->result.code = Okay;
	reply->result.argno = 0;
	reply->operation = netswap_4_bytes(request->operation);

	reply_end = END_ADDRESS(reply->operation);

	/*
	 * Set this now for those requests that return before the end.	
	 */
	*reply_length = (natural_t)reply_end - (natural_t)reply;
	
	/*
	 * For untargeted operations, set the sequence number of the
	 * reply to the seq # of the last valid targeted operation.
	 * This is used by the client end to determine restart and single
	 * stepping reception.
	 */
	reply->seq = netswap_4_bytes(kttd_current_seq);

	if (request->server != KERNEL_TTD) {
		reply->result.code = ServerNotAvailable;

		if (kttd_debug)
			printf("KTTD Server not available.\n");
		
		return SEND_REPLY;
	}

	/*
	 * Make sure that there aren't any idempotent operations
	 * that aren't targeted operations, since the duplicate
	 * sequence checking takes place in this if-clause!
	 */
	if (targeted_op) {
		if (!target_is_kernel(request->target)) {
			reply->result.code = InvalidTarget;

			if (kttd_debug)
				printf("Invalid KTTD target.\n");

			return SEND_REPLY;
		}

		if (!kttd_target.is_targeted) {
			reply->result.code = TargetNotAvailable;
			
			if (kttd_debug)
				printf("Kernel not targeted.\n");

			return SEND_REPLY;
		}

		if (duplicate(request->seq)) {
#if	SECOND_ATTEMPT
			/*
			 * The problem is, what happens when we get a duplicate
			 * restart or single step message?  There wasn't a
			 * reply for it, so we can't resend it.  So we just 
			 * act like we're ignoring it (like we did the first
			 * message), and "know" that we've already restarted
			 * or single stepped the kernel.
			 */
			if ((prev_operation == RESTART_TARGET) ||
			    (prev_operation == SINGLE_STEP_THREAD))
				return NO_REPLY;
#endif	/* SECOND_ATTEMPT */

			bcopy(&prev_reply, reply, prev_reply_length);

			if (kttd_debug)
#if	KTTD_VERBOSE
				printf("Duplicate, resending last reply.\n");
#else
			        printf("<D>");
#endif	/* KTTD_VERBOSE */
			
			return SEND_REPLY;
		}
	}
	
	switch(request->operation) {
	    case PROBE_SERVER:
		probe_server(request, reply, &reply_end);
		sr = SEND_REPLY;
		break;

	    case GET_TARGET_INFO:
		get_target_info(request, reply, &reply_end);
		sr = SEND_REPLY;
		break;

	    case CONNECT_TO_TARGET:
		connect_to_target(request, reply, &reply_end);
		sr = SEND_REPLY;
		break;

	    case DISCONNECT_FROM_TARGET:
		disconnect_from_target(request, reply, &reply_end);
		sr = SEND_REPLY;
		break;

	    case READ_FROM_TARGET:
		read_from_target(request, reply, &reply_end);
		sr = SEND_REPLY;
		break;

	    case WRITE_INTO_TARGET:
		write_into_target(request, reply, &reply_end);
		sr = SEND_REPLY;
		break;

	    case GET_NEXT_THREAD:
		get_next_thread(request, reply, &reply_end);
		sr = SEND_REPLY;
		break;

	    case GET_THREAD_INFO:
		get_thread_info(request, reply, &reply_end);
		sr = SEND_REPLY;
		break;

	    case SET_THREAD_INFO:
		set_thread_info(request, reply, &reply_end);
		sr = SEND_REPLY;
		break;

	    case STOP_TARGET:
		stop_target(request, reply, &reply_end);
		sr = SEND_REPLY;
		break;

	    case PROBE_TARGET:
		probe_target(request, reply, &reply_end);
		sr = SEND_REPLY;
		break;

	    case RESTART_TARGET:
		sr = restart_target(request, reply, &reply_end);
		break;

	    case SET_BREAKPOINT_IN_TARGET:
		set_breakpoint_in_target(request, reply, &reply_end);
		sr = SEND_REPLY;
		break;

	    case CLEAR_BREAKPOINT_IN_TARGET:
		clear_breakpoint_in_target(request, reply, &reply_end);
		sr = SEND_REPLY;
		break;

	    case GET_NEXT_BREAKPOINT_IN_TARGET:
		get_next_breakpoint_in_target(request, reply, &reply_end);
		sr = SEND_REPLY;
		break;

	    case SINGLE_STEP_THREAD:
		sr = single_step_thread(request, reply, &reply_end);
		break;

	    default:
		sr = SEND_REPLY;
		reply->result.code = InvalidOperation;
		
		/*
		 * Don't return here, we might've already set up our
		 * seq # as the duplicate seq #, and we therefore need
		 * to make sure that the prev_reply_length field is set
		 * properly.
		 */
	}

	/*
	 * See above comment dealing with the reply->seq.
	 */
	if (targeted_op)
		reply->seq = netswap_4_bytes(request->seq);

	*reply_length = (natural_t)reply_end - (natural_t)reply;

	prev_operation = request->operation;

	return (sr);
}

/*
 * ttd_service_request:
 *
 */
void ttd_service_request(void)
{
	natural_t	kttd_reply_length;
	ttd_request_t	request;
	ttd_reply_t	reply;

	/*
	 * If the length is bad, just drop the packet.
	 */
	if (kttd_current_length > sizeof(struct ttd_request)) {
		if (kttd_debug)
			printf("INVALID TTD Request Size!! 0x%x > 0x%x\n",
			       kttd_current_length, sizeof(struct ttd_request));
		return;
	}

	request = (ttd_request_t)kttd_current_request;
	reply = skip_net_headers(ttd_reply_msg);

	if ((int)request % BYTE_ALIGNMENT) {
		request = &aligned_request;
		bcopy(kttd_current_request, request,
		      ((kttd_current_length) > sizeof(struct ttd_request) ?
		       sizeof(struct ttd_request) : kttd_current_length));
	}

	if (ttd_decode_request(request, reply, &kttd_reply_length) == SEND_REPLY) {

		/*
		 * We've only built the ttd_reply_msg portion of the
		 * reply.  Now build the ether/ip/udp parts and cache
		 * the reply for duplicate retransmissions.
		 *
		 * Save this reply so that duplicate requests
		 * receive the right reply (idempotent requests).
		 *
		 * Remember that kttd_reply_msg is the
		 * skip_net_headers(ttd_reply_msg)!
		 *
		 * Just save the struct ttd_reply_msg, we'll build the
		 * reply header and ip/udp contents later.
		 *
		 */
		bcopy(skip_net_headers(ttd_reply_msg),
		      &prev_reply,
		      kttd_reply_length);
		prev_reply_length = kttd_reply_length;

		/*
		 * Build the Full reply msg, then send it.
		 */

		complete_and_send_ttd_reply(kttd_reply_length);
	}
}

void ttd_server_initialize(void)
{
	if (ttd_server_initialized)
		return;

	init_kernel_target();

	ttd_target_machine_type = get_ttd_machine_type();

	ttd_request_msg = &ttd_request_msg_array[0];
	ttd_reply_msg = &ttd_reply_msg_array[0];

	/*
	 * Get it to line up on the last 2 byte boundary before a
	 * BYTE_ALIGNMENT boundary.
	 */
	ttd_reply_msg = (char *)((int)ttd_reply_msg +
				 (int)ttd_reply_msg % BYTE_ALIGNMENT +
				 (BYTE_ALIGNMENT - 2));;
	
	ttd_server_initialized = TRUE;
}
