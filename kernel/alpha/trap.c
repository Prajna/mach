/*
 * Mach Operating System
 * Copyright (c) 1994,1993,1992 Carnegie Mellon University
 * All Rights Reserved.
 * 
 * Permission to use, copy, modify and distribute this software and its
 * documentation is hereby granted, provided that both the copyright
 * notice and this permission notice appear in all copies of the
 * software, derivative works or modified versions, and any portions
 * thereof, and that both notices appear in supporting documentation.
 * 
 * CARNEGIE MELLON ALLOWS FREE USE OF THIS SOFTWARE IN ITS "AS-IS"
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
 * any improvements or extensions that they make and grant Carnegie Mellon the
 * the rights to redistribute these changes.
 */
/*
 * HISTORY
 *  6-Jul-94  David Golub (dbg) at Carnegie-Mellon University
 *	Added extern declaration of exception(), so that we do
 *	not lose the high 32 bits of a bad virtual address.
 *
 * $Log:	trap.c,v $
 * Revision 2.7  93/05/15  19:11:41  mrt
 * 	machparam.h -> machspl.h
 * 
 * Revision 2.6  93/03/09  10:51:42  danner
 * 	Proto for Thread_syscall_return with GCC.
 * 	[93/03/07            af]
 * 	Fixed pcsample botch.
 * 	[93/03/05            af]
 * 
 * Revision 2.5  93/02/05  08:00:14  danner
 * 	Added machine check handler (jeffreyh).
 * 	[93/02/04  00:45:20  af]
 * 
 * Revision 2.4  93/02/04  07:55:25  danner
 * 	Added pc_sampling support
 * 	[93/02/02            danner]
 * 
 * Revision 2.3  93/01/19  09:00:01  danner
 * 	Better MP printouts.  Save more state before getting to ddb
 * 	in crashes.  Still cannot continue from a crash, though.
 * 	[93/01/15            af]
 * 
 * Revision 2.2  93/01/14  17:14:37  danner
 * 	Added reference to documentation source(s).
 * 	[92/12/16  15:18:18  af]
 * 
 * 	Created.
 * 	[92/12/10  15:06:12  af]
 * 
 */
/*
 *	File: trap.c
 * 	Author: Alessandro Forin, Carnegie Mellon University
 *	Date:	6/92
 *
 *	Trap Handlers for ALPHA
 *
 *	This code was derived exclusively from information available in
 *	"Alpha Architecture Reference Manual", Richard L. Sites ed.
 *	Digital Press, Burlington, MA 01803
 *	ISBN 1-55558-098-X, Order no. EY-L520E-DP
 */

#include <mach_pcsample.h>
#include <mach_kdb.h>

#include <machine/machspl.h>		/* spl definitions */
#include <mach/exception.h>
#include <mach/vm_param.h>
#include <mach/alpha/alpha_instruction.h>
#include <kern/thread.h>
#include <kern/zalloc.h>
#include <vm/vm_kern.h>
#include <alpha/ast.h>
#include <alpha/alpha_cpu.h>
#include <alpha/trap.h>

#define	DEBUG	1

extern zone_t msss_zone;

extern char copymsg_start[], copymsg_end[];
extern int copymsg_error();

/*
 *	Parameters to exception() are 64 bits.
 */
extern void exception(
	integer_t	type,
	integer_t	code,
	integer_t	subcode);

#if	MACH_KDB
boolean_t	debug_all_traps_with_kdb = FALSE;
extern struct db_watchpoint *db_watchpoint_list;
extern boolean_t db_watchpoints_inserted;

#endif	MACH_KDB

void
user_page_fault_continue(kr)
	kern_return_t kr;
{
	if (kr == KERN_SUCCESS) {
#if	MACH_KDB
		if (db_watchpoint_list &&
		    db_watchpoints_inserted) {
			register thread_t self = current_thread();
			register vm_map_t map = self->task->map;
			register struct alpha_saved_state *mss =
				&self->pcb->mss;

db_printf("Fix trap.c & watchpoints");
#if 0
			if (((mss->cause & CAUSE_EXC_MASK) == EXC_TLBS) &&
			    db_find_watchpoint(map, mss->bad_address, mss))
				(void) kdb_trap(mss, 2);
#endif
		}
#endif	MACH_KDB
		thread_exception_return();
		/*NOTREACHED*/
	}

#if	MACH_KDB
	if (debug_all_traps_with_kdb &&
	    kdb_trap(&current_thread()->pcb->mss, 1)) {
		thread_exception_return();
		/*NOTREACHED*/
	}
#endif	MACH_KDB

	exception(EXC_BAD_ACCESS, kr, current_thread()->pcb->mss.bad_address);
	/*NOTREACHED*/
}

/*
 *	Object:
 *		trap			EXPORTED function
 *
 *	Handle exceptions and faults.
 *
 */
boolean_t	syscalltrace = FALSE;
boolean_t	debug_verbose = FALSE;
void		(*alpha_machine_check)() = 0;

trap(	struct alpha_saved_state	*ss_ptr,
	unsigned long			r4,
	unsigned long			r5,
	unsigned long			cause)
{
	thread_t	t;
	kern_return_t   ret;
	vm_map_t	map;
	vm_offset_t     vaddr;
	int		exc_type, exc_code;
	struct trap_frame	*tf;

if (ss_ptr == 0) gimmeabreak();
if ( (debug_verbose > 2) || ! (syscalltrace && (cause == T_CHMK)) )
if (debug_verbose) db_printf("{[%d]trap[%x](%x %x %x %x)}\n",
			     cpu_number(), &t, ss_ptr, r4, r5, cause);

	t = current_thread();

	tf = ss_ptr->framep;
	if (alpha_user_mode(tf->saved_ps))
		goto user_mode_traps;

	/*
	 *	Trap while in Kernel mode
	 */
	switch (cause) {
	case T_PROT_FAULT:
	case T_TRANS_INVALID:
	    vaddr = (vm_offset_t) r4;

		/*
		 * If the current map is a submap of the kernel map,
		 * and the address is within that map, fault on that
		 * map.  If the same check is done in vm_fault
		 * (vm_map_lookup), we may deadlock on the kernel map
		 * lock. [dbg]
		 */
		if (t == THREAD_NULL)	/* startup & sanity */
			map = kernel_map;
		else {
			map = t->task->map;
			if (vaddr < vm_map_min(map) ||
			    vaddr >= vm_map_max(map))
				map = kernel_map;
		}

		/*
		 * Register r5 contains the MMF flags:
		 *	8000000000000000 write fault
		 *	0000000000000000 read fault
		 *	0000000000000001 I-fetch fault
		 */
		ret = vm_fault(map, trunc_page(vaddr),
				(((long)r5) < 0)
			       		? VM_PROT_READ|VM_PROT_WRITE
					: ((r5) ?
						VM_PROT_READ|VM_PROT_EXECUTE:
						VM_PROT_READ),
			       FALSE, FALSE, (void (*)()) 0);

		if (ret == KERN_SUCCESS) {
#if	MACH_KDB
			if (db_watchpoint_list &&
			    db_watchpoints_inserted &&
			    (((long)r5) < 0) &&
			    db_find_watchpoint((VM_MIN_ADDRESS <= vaddr) &&
					       (vaddr < VM_MAX_ADDRESS) &&
					       (t != THREAD_NULL) ?
					       t->task->map : kernel_map,
					       vaddr,
					       ss_ptr))
				(void) kdb_trap(ss_ptr, 2);
#endif	MACH_KDB
			return;
		}

		if (((vm_offset_t) copymsg_start <= tf->saved_pc) &&
		    (tf->saved_pc < (vm_offset_t) copymsg_end)) {
			tf->saved_pc = (vm_offset_t) copymsg_error;
			return;
		}
		if (t->recover) {
			tf->saved_pc = t->recover;
			t->recover = 0;
			return;
		}

	    break;
	case T_SCHECK:
	case T_PCHECK:
	    if (alpha_machine_check){
		    (*alpha_machine_check)();	    /* XXXThis will need to 
						     * XXXtake args if it is 
						     * made real
						     */
		    return ;
	    }
	    else
		    panic ("Machine Check without a handler!\n");
	    break;
	case T_READ_FAULT:
	case T_WRITE_FAULT:
	case T_EXECUTE_FAULT:
	default:
	    break;
	}
#if	MACH_KDB
	/* locore did not record the faulting SP, for speed */
	ss_ptr->sp = (vm_offset_t)(tf + 1) + (tf->saved_ps >> 56);
#endif
	goto fatal;

	/*
	 *	Trap while in User mode
	 */
user_mode_traps:
	switch (cause) {
	case T_PROT_FAULT:
	case T_TRANS_INVALID:
	    vaddr = (vm_offset_t) r4;
	    ss_ptr->bad_address = vaddr;
	    ss_ptr->cause = cause;
		map = t->task->map;

	    ss_ptr->saved_frame = *tf;

		(void) vm_fault(map, trunc_page(vaddr),
				(((long)r5) < 0)
			       		? VM_PROT_READ|VM_PROT_WRITE
					: ((r5) ?
						VM_PROT_READ|VM_PROT_EXECUTE:
						VM_PROT_READ),
			       FALSE,
			       FALSE, user_page_fault_continue);
		/*NOTREACHED*/
		break;
	case T_CHMK:
		trap_syscall(ss_ptr, tf);
		return;
	case T_FPA_DISABLED:
{ static int memo = 0;
if (!memo++) db_printf("Remember to stress-test FPA usage\n");
}
	    {
		/*
		 *	Make sure the thread does have
		 *	a floating-point save area.
		 */
		register struct alpha_float_state	*mfs;

		mfs = t->pcb->mms.mfs;
		if (mfs == 0) {
			pcb_fpa_init(t);
			mfs = t->pcb->mms.mfs;
		}

		t->pcb->mms.mfs = (struct alpha_float_state *)
					((vm_offset_t)mfs | 1); /* inuse */
		alpha_fpa_loadup(mfs);	/* leaves fpa enabled */
		return;
	    }

	case T_BP: {		/* Breakpoint */
		register struct alpha_sstep_state *msss;

		/*
		 * If single stepping, remove breakpoints
		 * to minimize damage to other fellow threads.
		 * Otherwise it is a real breakpoint.
		 */
		msss = t->pcb->mms.msss;
		if (msss) {
			t->pcb->mms.msss = 0;
			exc_code = EXC_BREAK_SSTEP;
			did_sstep(msss);
			zfree(msss_zone, (vm_offset_t) msss);
		} else
			exc_code = EXC_BREAK_BPT;

		exc_type = EXC_BREAKPOINT;
		break;
	}

	case T_AST_K:
	case T_AST_E:
	case T_AST_S:
	case T_AST_U:
	case T_ILL:
	case T_PAL:
	case T_CHME:
	case T_CHMS:
	case T_CHMU:
		exc_type = EXC_BAD_INSTRUCTION;
		exc_code = EXC_ALPHA_RESOPND;
		break;

	case T_UNALIGNED:
		exc_type = EXC_BAD_INSTRUCTION;
		exc_code = EXC_ALPHA_RESADDR;
		break;

	case T_ARITHMETIC:
		exc_type = EXC_ARITHMETIC;
		exc_code = r5 & 0x7f;	/* exception summary param */
		cause = r4;		/* register write mask */
		break;

/*notyet*/
	case T_BUG:

	default:
		goto fatal;
	}
#if	MACH_KDB
	ss_ptr->saved_frame = *tf;
	ss_ptr->cause = cause;
	if (debug_all_traps_with_kdb && kdb_trap(ss_ptr, 1))
		return;
#endif	MACH_KDB

	/* Deliver the exception */
	exception(exc_type, exc_code, cause);
	/*NOTREACHED*/
	return; /* help for the compiler */

fatal:
	ss_ptr->saved_frame = *tf;
#if	MACH_KDB
	ss_ptr->cause = cause;
	ss_ptr->bad_address = r4;	/* most likely */
	/* XXX r5 ? */
	if (kdb_trap(ss_ptr, 1))
		return;
#endif
	splhigh();
	printf("Fatal kernel trap: ...\n");
	printf("pc = %x, va = %x, sp = %x\n",
			tf->saved_pc, ss_ptr->bad_address, ss_ptr->sp);
	halt_all_cpus(1);
}

/* temp */
#include <kern/syscall_sw.h>

#if	DEBUG
char	*syscall_names[77] = {
"kern_invalid",		/* 0 */		/* Unix */
"kern_invalid",		/* 1 */		/* Unix */
"kern_invalid",		/* 2 */		/* Unix */
"kern_invalid",		/* 3 */		/* Unix */
"kern_invalid",		/* 4 */		/* Unix */
"kern_invalid",		/* 5 */		/* Unix */
"kern_invalid",		/* 6 */		/* Unix */
"kern_invalid",		/* 7 */		/* Unix */
"kern_invalid",		/* 8 */		/* Unix */
"kern_invalid",		/* 9 */		/* Unix */
"task_self",		/* 10 */	/* obsolete */
"thread_reply",		/* 11 */	/* obsolete */
"task_notify",		/* 12 */	/* obsolete */
"thread_self",		/* 13 */	/* obsolete */
"kern_invalid",		/* 14 */
"kern_invalid",		/* 15 */
"kern_invalid",		/* 16 */
"evc_wait",		/* 17 */
"kern_invalid",		/* 18 */
"kern_invalid",		/* 19 */
"msg_send_trap",	/* 20 */	/* obsolete */
"msg_receive_trap",	/* 21 */	/* obsolete */
"msg_rpc_trap",		/* 22 */	/* obsolete */
"kern_invalid",		/* 23 */
"kern_invalid",		/* 24 */
"mach_msg_trap",	/* 25 */
"mach_reply_port",	/* 26 */
"mach_thread_self",	/* 27 */
"mach_task_self",	/* 28 */
"mach_host_self",	/* 29 */
"kern_invalid",		/* 30 */
"kern_invalid",		/* 31 */
"kern_invalid",		/* 32 */
"kern_invalid",		/* 33 */
"kern_invalid",		/* 34 */
"kern_invalid",		/* 35 */
"kern_invalid",		/* 36 */
"kern_invalid",		/* 37 */
"kern_invalid",		/* 38 */
"kern_invalid",		/* 39 */
"kern_invalid",		/* 40 */
"kern_invalid",		/* 41 */
"kern_invalid",		/* 42 */
"kern_invalid",		/* 43 */
"kern_invalid",		/* 44 */
"kern_invalid",		/* 45 */
"kern_invalid",		/* 46 */
"kern_invalid",		/* 47 */
"kern_invalid",		/* 48 */
"kern_invalid",		/* 49 */
"kern_invalid",		/* 50 */
"kern_invalid",		/* 51 */
"kern_invalid",		/* 52 */
"kern_invalid",		/* 53 */
"kern_invalid",		/* 54 */
"host_self",		/* 55 */
"null_port",		/* 56 */
"kern_invalid",		/* 57 */
"kern_invalid",		/* 58 */
 "swtch_pri",		/* 59 */
"swtch",		/* 60 */
"thread_switch",	/* 61 */
"kern_invalid",		/* 62 */
"kern_invalid",		/* 63 */
"syscall_vm_map",	/* 64 */
"syscall_vm_allocate",	/* 65 */
"syscall_vm_deallocate",/* 66 */
"kern_invalid",		/* 67 */
"syscall_task_create",	/* 68 */
"syscall_task_terminate",	/* 69 */
"syscall_task_suspend",		/* 70 */
"syscall_task_set_special_port",/* 71 */
"syscall_mach_port_allocate",	/* 72 */
"syscall_mach_port_deallocate",	/* 73 */
"syscall_mach_port_insert_right",	/* 74 */
"syscall_mach_port_allocate_name",	/* 75 */
"syscall_thread_depress_abort"	/* 76 */
};

#endif

trap_syscall( struct alpha_saved_state	*ss_ptr,
	      struct trap_frame *tf)
{
	register mach_trap_t	*callp;
	natural_t	 	callno;
	register eml_dispatch_t	eml;
	register thread_t	t = current_thread();

	/*
	 * Syscall redirection
	 */
	eml = t->task->eml_dispatch;
	if (eml) {
		register natural_t	min, count;
		register vm_offset_t	eml_pc;

		callno = ss_ptr->v0;
		min = eml->disp_min;
		count = eml->disp_count;
		/* This math is tricky cuz unsigned & overflow */
		min = callno - min;
		if ((count >= min) &&
		    ((eml_pc = eml->disp_vector[min]) != 0)) {
#if 0
			ss_ptr->v0 = ss_ptr->saved_frame.saved_pc;
			ss_ptr->saved_frame.saved_pc = eml_pc;
#else
			register struct trap_frame	*tf;

			ss_ptr->v0 = ss_ptr->saved_frame.saved_pc;
			ss_ptr->saved_frame.saved_pc = eml_pc;

			tf = & STACK_MEL(active_stacks[cpu_number()])->tf;
			ss_ptr->v0 = tf->saved_pc;
			tf->saved_pc = eml_pc;
#endif
			return;
		}
	}

	/*
	 * Native syscall
	 */
	callno = -ss_ptr->v0;
	if (callno > mach_trap_count)
		goto invalid;

	callp = &mach_trap_table[callno];

	switch (callp->mach_trap_arg_count) {
	case 0:
#if	DEBUG
		if (syscalltrace)
		db_printf("%s() ", syscall_names[callno]);
#endif
		ss_ptr->v0 = (*callp->mach_trap_function)();
		break;
	case 1:
#if	DEBUG
		if (syscalltrace)
		db_printf("%s(%x) ", syscall_names[callno],
			  ss_ptr->a0);
#endif
		ss_ptr->v0 = (*callp->mach_trap_function)
			(ss_ptr->a0);
		break;
	case 2:
#if	DEBUG
		if (syscalltrace)
		db_printf("%s(%x,%x) ", syscall_names[callno],
			  ss_ptr->a0, ss_ptr->a1);
#endif
		ss_ptr->v0 = (*callp->mach_trap_function)
			(ss_ptr->a0, ss_ptr->a1);
		break;
	case 3:
#if	DEBUG
		if (syscalltrace)
		db_printf("%s(%x,%x,%x) ", syscall_names[callno],
			  ss_ptr->a0, ss_ptr->a1, ss_ptr->a2);
#endif
		ss_ptr->v0 = (*callp->mach_trap_function)
			(ss_ptr->a0, ss_ptr->a1, ss_ptr->a2);
		break;
	case 4:
#if	DEBUG
		if (syscalltrace)
		db_printf("%s(%x,%x,%x,%x) ", syscall_names[callno],
			  ss_ptr->a0, ss_ptr->a1, ss_ptr->a2, ss_ptr->a3);
#endif
		ss_ptr->v0 = (*callp->mach_trap_function)
			(ss_ptr->a0, ss_ptr->a1, ss_ptr->a2, ss_ptr->a3);
		break;
	case 5:
#if	DEBUG
		if (syscalltrace)
		db_printf("%s(%x,%x,%x,%x,%x) ", syscall_names[callno],
			  ss_ptr->a0, ss_ptr->a1, ss_ptr->a2, ss_ptr->a3,
			  ss_ptr->a4);
#endif
		ss_ptr->v0 = (*callp->mach_trap_function)
			(ss_ptr->a0, ss_ptr->a1, ss_ptr->a2, ss_ptr->a3,
			 ss_ptr->a4);
		break;
	case 6:
#if	DEBUG
		if (syscalltrace)
		db_printf("%s(%x,%x,%x,%x,%x,%x) ", syscall_names[callno],
			  ss_ptr->a0, ss_ptr->a1, ss_ptr->a2, ss_ptr->a3,
			  ss_ptr->a4, ss_ptr->a5);
#endif
		ss_ptr->v0 = (*callp->mach_trap_function)
			(ss_ptr->a0, ss_ptr->a1, ss_ptr->a2, ss_ptr->a3,
			 ss_ptr->a4, ss_ptr->a5);
		break;
	case 7:
#if	DEBUG
		if (syscalltrace)
		db_printf("%s(%x,%x,%x,%x,%x,%x,%x) ", syscall_names[callno],
			  ss_ptr->a0, ss_ptr->a1, ss_ptr->a2, ss_ptr->a3,
			  ss_ptr->a4, ss_ptr->a5, ss_ptr->t0);
#endif
		ss_ptr->v0 = (*callp->mach_trap_function)
			(ss_ptr->a0, ss_ptr->a1, ss_ptr->a2, ss_ptr->a3,
			 ss_ptr->a4, ss_ptr->a5, ss_ptr->t0);
		break;
	case 11:
#if	DEBUG
		if (syscalltrace)
		db_printf("%s(%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x) ", syscall_names[callno],
			  ss_ptr->a0, ss_ptr->a1, ss_ptr->a2, ss_ptr->a3,
			  ss_ptr->a4, ss_ptr->a5, ss_ptr->t0,
			  tf->saved_r2, tf->saved_r3, tf->saved_r4,
			  tf->saved_r5 );
#endif
		ss_ptr->v0 = (*callp->mach_trap_function)
			(ss_ptr->a0, ss_ptr->a1, ss_ptr->a2, ss_ptr->a3,
			 ss_ptr->a4, ss_ptr->a5, ss_ptr->t0,
			 tf->saved_r2, tf->saved_r3, tf->saved_r4,
			 tf->saved_r5 );
		break;
	default:
invalid:
#if	DEBUG
		if (syscalltrace)
		db_printf("invalid syscallno -%d\n", callno);
#endif
		ss_ptr->v0 = KERN_FAILURE;
	}
#if	DEBUG
	if (syscalltrace) {
		db_printf("-> %x\n", ss_ptr->v0);
		if (ss_ptr->v0 && (syscalltrace > 1)) gimmeabreak();
	}
#endif
}

void thread_syscall_return(ret)
	kern_return_t	ret;
{
#if	__GNUC__
	extern void __volatile__ Thread_syscall_return( kern_return_t );
#endif
#if	DEBUG
	if (syscalltrace) {
		db_printf("-> %x\n", ret);
		if (ret && (syscalltrace > 1)) gimmeabreak();
	}
#endif
	Thread_syscall_return(ret);
	/* NOTREACHED */
}

/* --- */
#define	MAX_CHANS	32	/* grow as needed */
struct {
	void		(*routine)();
	natural_t	argument;
} interrupt_vector[MAX_CHANS];

interrupt_dispatch(chan, routine, argument)
	void		(*routine)();
	natural_t	argument;
{
	if (chan >= MAX_CHANS) panic("interrupt_dispatch");
	interrupt_vector[chan].routine = routine;
	interrupt_vector[chan].argument = argument;
}

#if	1/*DEBUG*/
interrupt(struct alpha_saved_state	*ss_ptr, int chan)
{
	(*interrupt_vector[chan].routine)(interrupt_vector[chan].argument,
					  ss_ptr->framep->saved_ps);
}
#else
interrupt(xx, chan)
{
	(*interrupt_vector[chan].routine)(interrupt_vector[chan].argument);
}
#endif
/* --- */

stray_interrupt( struct alpha_saved_state	*ss_ptr,
		 unsigned long			r4,
		 unsigned long			r5,
		 unsigned long			cause)
{
#if	MACH_KDB
	gimmeabreak();
#endif
}

stray_trap( struct alpha_saved_state	*ss_ptr,
	    unsigned long		r4,
	    unsigned long		r5,
	    unsigned long		cause)
{
#if	MACH_KDB
	gimmeabreak();
#endif
	panic("Unexpected trap");
}


/*
 *	Object:
 *		getreg_val			EXPORTED function
 *
 *	Return the value of a register in the exception frame
 *
 */
vm_size_t *addrof_alpha_reg(regn, ss_ptr)
	register unsigned regn;
	struct alpha_saved_state *ss_ptr;
{
	switch (regn) {
	case 0:		return &ss_ptr->v0;
	case 1:		return &ss_ptr->t0;
	case 2:		return &ss_ptr->saved_frame.saved_r2;
	case 3:		return &ss_ptr->saved_frame.saved_r3;
	case 4:		return &ss_ptr->saved_frame.saved_r4;
	case 5:		return &ss_ptr->saved_frame.saved_r5;
	case 6:		return &ss_ptr->saved_frame.saved_r6;
	case 7:		return &ss_ptr->saved_frame.saved_r7;
	case 8:		return &ss_ptr->t7;
	case 9:		return &ss_ptr->s0;
	case 10:	return &ss_ptr->s1;
	case 11:	return &ss_ptr->s2;
	case 12:	return &ss_ptr->s3;
	case 13:	return &ss_ptr->s4;
	case 14:	return &ss_ptr->s5;
	case 15:	return &ss_ptr->s6;
	case 16:	return &ss_ptr->a0;
	case 17:	return &ss_ptr->a1;
	case 18:	return &ss_ptr->a2;
	case 19:	return &ss_ptr->a3;
	case 20:	return &ss_ptr->a4;
	case 21:	return &ss_ptr->a5;
	case 22:	return &ss_ptr->t8;
	case 23:	return &ss_ptr->t9;
	case 24:	return &ss_ptr->t10;
	case 25:	return &ss_ptr->t11;
	case 26:	return &ss_ptr->ra;
	case 27:	return &ss_ptr->t12;
	case 28:	return &ss_ptr->at;
	case 29:	return &ss_ptr->gp;
	case 30:	return &ss_ptr->sp;
	default: return 0;
	}
}

vm_size_t
getreg_val(regn, ss_ptr)
	register unsigned regn;
	struct alpha_saved_state *ss_ptr;
{
	if (regn >= 31)
		return 0;
	return *addrof_alpha_reg(regn,ss_ptr);
}

/*
 *	Object:
 *		thread_exception_return		EXPORTED function
 *
 *
 */
void
thread_exception_return()
{
	register thread_t t = current_thread();
	register pcb_t pcb = t->pcb;
	register struct alpha_sstep_state *msss = pcb->mms.msss;

	/*
	 *	If single stepping, install breakpoints before
	 *	getting back to user.  This should be done as
	 *	late as possible, to minimize the (unavoidable)
	 *	interference with other threads in the same task.
	 *	It would be nice to do it _after_ ASTs are taken.
	 */
	if (msss)
		prepare_sstep(msss, &pcb->mss);

	thread_bootstrap_return();
	/*NOTREACHED*/
}

/*
 *	Object:
 *		prepare_sstep			LOCAL function
 *
 *	Install breakpoints to realize single stepping in
 *	software.  Either one or two will be needed, depending
 *	on whether we are at a branching instruction or not.
 *	Note that only the target thread should execute this.
 *
 */
#define SSTEP_INSTRUCTION (0x00000080)

prepare_sstep(msss, mss)
	register struct alpha_sstep_state *msss;
	register struct alpha_saved_state *mss;
{
	extern vm_offset_t	branch_taken();
	alpha_instruction	ins;
	alpha_instruction	*brpc, *pc;
	register struct breakpoint *bp = msss->ss_bp;

	pc = (alpha_instruction *) mss->saved_frame.saved_pc;

	/*
	 * NOTE: we might reenter because an AST stopped us on the way out
	 */
	if (msss->ss_count)
		return;

	if (copyin(pc, &ins, sizeof ins))
		return;			/* no harm done */

	if (isa_call(ins) || isa_branch(ins)) {
		brpc = (alpha_instruction *)
				branch_taken(ins, pc, getreg_val, mss);
		if (brpc != pc) {
			bp->address = (vm_offset_t) brpc;
			if (copyin(brpc, &bp->instruction, sizeof ins))
				goto seq;	/* he'll get hurt */	
			if (poke_instruction(brpc, SSTEP_INSTRUCTION))
				goto seq;	/* ditto */
			msss->ss_count++, bp++;
		}
	}
seq:
	pc += 1;
	bp->address = (vm_offset_t) pc;
	if (copyin(pc, &bp->instruction, sizeof ins))
		return;			/* he'll get hurt */	
	if (poke_instruction(pc, SSTEP_INSTRUCTION))
		return;
	msss->ss_count++;
}

/*
 *	Object:
 *		did_sstep			LOCAL function
 *
 *	Remove the breakpoints installed by the above function,
 *	pay attention to what other threads might have done to
 *	the task's memory in the meantime.  Yes, there _are_
 *	problems for this on a multiprocessor.
 */
did_sstep(msss)
	register struct alpha_sstep_state *msss;
{
	alpha_instruction	 ins, *pc;
	register struct breakpoint *bp = msss->ss_bp;

	for (; msss->ss_count-- > 0; bp++) {
		pc = (alpha_instruction *) bp->address;
		if (copyin(pc, &ins, sizeof ins))
			continue;
		if (ins.bits != SSTEP_INSTRUCTION)
			continue;
		poke_instruction(pc, bp->instruction);
	}
	msss->ss_count = 0;
}

/*
 *	Object:
 *		poke_instruction		LOCAL function
 *
 *	Put an instruction in my address space.  Does not
 *	change protections, but might fault in text pages.
 */
poke_instruction(loc, ins)
	alpha_instruction	*loc, ins;
{
	vm_map_t	 map = current_thread()->task->map;
	vm_offset_t	 pa;
	kern_return_t	 ret;	
again:
	pa = pmap_extract(map->pmap, (vm_offset_t) loc);
	if (pa == 0) {
		ret = vm_fault(map, trunc_page(loc), VM_PROT_READ,
			       FALSE, FALSE, (void (*)()) 0);
		if (ret != KERN_SUCCESS)
			return 1;
		goto again;
	}
	loc = (alpha_instruction *)(PHYS_TO_K0SEG(pa));
	*loc = ins;
	alphacache_Iflush();
	return 0;
}

#if	MACH_KDB
/*
 *	Object:
 *		thread_kdb_return		EXPORTED function
 *
 *	Try for debugger, to user if ok.
 *
 */
void
thread_kdb_return()
{
	if (kdb_trap(&current_thread()->pcb->mss, 1)) {
		thread_exception_return();
		/*NOTREACHED*/
	}
}
#endif	MACH_KDB

#if	MACH_PCSAMPLE
/*
 * return saved state for interrupted user thread
 */
vm_offset_t
interrupted_pc(thread_t t)
{
	register struct alpha_saved_state *mss;

 	mss = &(USER_REGS(t)->mss);
 	return mss->framep->saved_pc;
}
#endif	/*MACH_PCSAMPLE*/
