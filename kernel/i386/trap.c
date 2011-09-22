/* 
 * Mach Operating System
 * Copyright (c) 1991,1990,1989,1988 Carnegie Mellon University
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
 * HISTORY
 * $Log:	trap.c,v $
 * Revision 2.23  93/05/15  19:30:47  mrt
 * 	machparam.h -> machspl.h
 * 
 * Revision 2.22  93/05/10  23:23:53  rvb
 * 	Checkin for MK80 branch.
 * 	[93/05/10  15:12:36  grm]
 * 
 * Revision 2.21.1.1  93/03/01  15:24:27  grm
 * 	Added TTD teledebug code to handle traps.  Code mirrors ddb.
 * 	[93/03/01            grm]
 * 
 * Revision 2.21  93/01/24  13:14:55  danner
 * 	Installed pc sampling from C Maeda; added interrupted_pc().
 * 	[93/01/12            rvb]
 * 
 * Revision 2.20  93/01/14  17:29:45  danner
 * 	Proper spl typing.
 * 	[92/11/30            af]
 * 
 * Revision 2.19  92/01/03  20:09:29  dbg
 * 	Build retry table for certain successful faults.
 * 	Enable IO instruction emulation in V86 mode.
 * 	[91/12/01            dbg]
 * 
 * 	Add i386_astintr to handle delayed floating-point exceptions.
 * 	[91/10/29            dbg]
 * 
 * 	Check for use of user FP register segment if floating-point
 * 	emulator present.  Pass i386 trap number as exception code
 * 	for all i386 exceptions.  Route i386 exceptions through
 * 	emulator fixup routine if exception taken within emulator.
 * 
 * 	Eliminate warning on 'ifdef'.  Remove offending type
 * 	declarations.
 * 	[91/10/19            dbg]
 * 
 * Revision 2.18  91/10/09  16:07:23  af
 * 	Checked kdb trap for user space T_DEBUG and T_INT3.
 * 	[91/08/29            tak]
 * 
 * Revision 2.17  91/08/28  21:37:16  jsb
 * 	Don't emulate IO instructions if in V86 mode.
 * 	[91/08/21            dbg]
 * 
 * Revision 2.16  91/08/24  11:57:09  af
 * 	Revision 2.15.3.1  91/08/19  13:45:20  danner
 * 	Make the file safe for gcc 1.36.  There is a really bizarro
 * 	structure assignment of an array that starts at zero that
 * 	nukes us.
 * 	[91/08/07            rvb]
 * 
 * Revision 2.15.3.1  91/08/19  13:45:20  danner
 * 	Make the file safe for gcc 1.36.  There is a really bizarro
 * 	structure assignment of an array that starts at zero that
 * 	nukes us.
 * 	[91/08/07            rvb]
 * 
 * Revision 2.15  91/07/31  17:42:21  dbg
 * 	Separate user and kernel trap cases.  Combine user and v86-mode
 * 	trap cases (except for calling instruction assist).
 * 
 * 	New v86 interrupt simulation.
 * 
 * 	Check for two copyout failure locations.
 * 	[91/07/30  17:01:10  dbg]
 * 
 * Revision 2.14  91/06/06  17:04:06  jsb
 * 	i386_read_fault is now intel_read_fault.
 * 	[91/05/13  16:56:39  jsb]
 * 
 * Revision 2.13  91/05/14  16:18:11  mrt
 * 	Correcting copyright
 * 
 * Revision 2.12  91/05/08  12:43:35  dbg
 * 	Correct calls to FPU error routines.
 * 	[91/04/26  14:39:33  dbg]
 * 
 * Revision 2.11  91/03/16  14:45:28  rpd
 * 	Added resume, continuation arguments to vm_fault.
 * 	Added user_page_fault_continue.
 * 	[91/02/05            rpd]
 * 	Removed astintr.
 * 	[91/01/22  15:53:33  rpd]
 * 
 * Revision 2.10  91/02/14  14:41:59  mrt
 * 	rfr's latest changes to v86 assist
 * 	[91/01/28  15:25:30  rvb]
 * 
 * Revision 2.9  91/02/05  17:15:21  mrt
 * 	Changed to new Mach copyright
 * 	[91/02/01  17:38:41  mrt]
 * 
 * Revision 2.8  91/01/09  22:41:55  rpd
 * 	Fixed a merge bug.
 * 	[91/01/09            rpd]
 * 
 * Revision 2.7  91/01/08  17:32:21  rpd
 * 	Add v86_hdw_assist().
 * 	[91/01/04  09:54:24  rvb]
 * 
 * 	Basically add trapv86()
 * 	[90/12/20  10:21:01  rvb]
 * 
 * Revision 2.6  91/01/08  15:11:18  rpd
 * 	Only need csw_needed in AST exit path.
 * 	[90/12/27            rpd]
 * 
 * 	Replaced thread_doexception with new exception interface.
 * 	[90/12/21            rpd]
 * 	Added continuation argument to thread_block.
 * 	[90/12/08            rpd]
 * 
 * Revision 2.5  90/10/25  14:44:56  rwd
 * 	Added watchpoint support.
 * 	[90/10/18            rpd]
 * 
 * Revision 2.4  90/06/02  14:48:58  rpd
 * 	Updated to new csw_needed macro.
 * 	[90/06/02            rpd]
 * 
 * Revision 2.3  90/05/21  13:26:49  dbg
 * 	Add hook for emulating IO instructions.
 * 	[90/05/17            dbg]
 * 
 * Revision 2.2  90/05/03  15:38:07  dbg
 * 	V86 mode is also user mode.
 * 	[90/04/26            dbg]
 * 
 * 	Created (from VAX version).
 * 	[90/02/08            dbg]
 * 
 */
/*
 * Hardware trap/fault handler.
 */

#include <cpus.h>
#include <fpe.h>
#include <mach_kdb.h>
#include <mach_ttd.h>
#include <mach_pcsample.h>

#include <sys/types.h>
#include <i386/eflags.h>
#include <i386/trap.h>
#include <machine/machspl.h>	/* for spl_t */

#include <mach/exception.h>
#include <mach/kern_return.h>
#include <mach/vm_param.h>
#include <mach/i386/thread_status.h>

#include <vm/vm_kern.h>
#include <vm/vm_map.h>

#include <kern/ast.h>
#include <kern/thread.h>
#include <kern/task.h>
#include <kern/sched.h>
#include <kern/sched_prim.h>

#include <i386/io_emulate.h>

extern void exception();
extern void thread_exception_return();

extern void i386_exception();

#if	MACH_KDB
boolean_t	debug_all_traps_with_kdb = FALSE;
extern struct db_watchpoint *db_watchpoint_list;
extern boolean_t db_watchpoints_inserted;

void
thread_kdb_return()
{
	register thread_t thread = current_thread();
	register struct i386_saved_state *regs = USER_REGS(thread);

	if (kdb_trap(regs->trapno, regs->err, regs)) {
		thread_exception_return();
		/*NOTREACHED*/
	}
}
#endif	MACH_KDB

#if	MACH_TTD
extern boolean_t kttd_enabled;
boolean_t debug_all_traps_with_kttd = TRUE;
#endif	MACH_TTD

void
user_page_fault_continue(kr)
	kern_return_t kr;
{
	register thread_t thread = current_thread();
	register struct i386_saved_state *regs = USER_REGS(thread);

	if (kr == KERN_SUCCESS) {
#if	MACH_KDB
		if (db_watchpoint_list &&
		    db_watchpoints_inserted &&
		    (regs->err & T_PF_WRITE) &&
		    db_find_watchpoint(thread->task->map,
				       (vm_offset_t)regs->cr2,
				       regs))
			kdb_trap(T_WATCHPOINT, 0, regs);
#endif	MACH_KDB
		thread_exception_return();
		/*NOTREACHED*/
	}

#if	MACH_KDB
	if (debug_all_traps_with_kdb &&
	    kdb_trap(regs->trapno, regs->err, regs)) {
		thread_exception_return();
		/*NOTREACHED*/
	}
#endif	MACH_KDB

	i386_exception(EXC_BAD_ACCESS, kr, regs->cr2);
	/*NOTREACHED*/
}

/*
 * Fault recovery in copyin/copyout routines.
 */
struct recovery {
	int	fault_addr;
	int	recover_addr;
};

extern struct recovery	recover_table[];
extern struct recovery	recover_table_end[];

/*
 * Recovery from Successful fault in copyout does not
 * return directly - it retries the pte check, since
 * the 386 ignores write protection in kernel mode.
 */
extern struct recovery	retry_table[];
extern struct recovery	retry_table_end[];

char *	trap_type[] = {
	"Divide error",
	"Debug trap",
	"NMI",
	"Breakpoint",
	"Overflow",
	"Bounds check",
	"Invalid opcode",
	"No coprocessor",
	"Double fault",
	"Coprocessor overrun",
	"Invalid TSS",
	"Segment not present",
	"Stack bounds",
	"General protection",
	"Page fault",
	"(reserved)",
	"Coprocessor error"
};
int	TRAP_TYPES = sizeof(trap_type)/sizeof(trap_type[0]);

boolean_t	brb = TRUE;

/*
 * Trap from kernel mode.  Only page-fault errors are recoverable,
 * and then only in special circumstances.  All other errors are
 * fatal.
 */
void kernel_trap(regs)
	register struct i386_saved_state *regs;
{
	int	exc;
	int	code;
	int	subcode;
	register int	type;
	vm_map_t	map;
	kern_return_t	result;
	register thread_t	thread;

	type = regs->trapno;
	code = regs->err;
	thread = current_thread();

	switch (type) {
	    case T_NO_FPU:
		fpnoextflt();
		return;

	    case T_FPU_FAULT:
		fpextovrflt();
		return;

	    case T_FLOATING_POINT_ERROR:
		fpexterrflt();
		return;

	    case T_PAGE_FAULT:
		/*
		 * If the current map is a submap of the kernel map,
		 * and the address is within that map, fault on that
		 * map.  If the same check is done in vm_fault
		 * (vm_map_lookup), we may deadlock on the kernel map
		 * lock.
		 */
		subcode = regs->cr2;	/* get faulting address */

		if (thread == THREAD_NULL)
		    map = kernel_map;
		else {
		    map = thread->task->map;
		    if ((vm_offset_t)subcode < vm_map_min(map) ||
			(vm_offset_t)subcode >= vm_map_max(map))
			map = kernel_map;
		}

		/*
		 * Since the 386 ignores write protection in
		 * kernel mode, always try for write permission
		 * first.  If that fails and the fault was a
		 * read fault, retry with read permission.
		 */
		result = vm_fault(map,
				  trunc_page((vm_offset_t)subcode),
				  VM_PROT_READ|VM_PROT_WRITE,
				  FALSE,
				  FALSE,
				  (void (*)()) 0);
#if	MACH_KDB
		if (result == KERN_SUCCESS) {
		    /* Look for watchpoints */
		    if (db_watchpoint_list &&
			db_watchpoints_inserted &&
			(code & T_PF_WRITE) &&
			db_find_watchpoint(map,
				(vm_offset_t)subcode, regs))
			kdb_trap(T_WATCHPOINT, 0, regs);
		}
		else
#endif	MACH_KDB
		if ((code & T_PF_WRITE) == 0 &&
		    result == KERN_PROTECTION_FAILURE)
		{
		    /*
		     *	Must expand vm_fault by hand,
		     *	so that we can ask for read-only access
		     *	but enter a (kernel)writable mapping.
		     */
		    result = intel_read_fault(map,
					  trunc_page((vm_offset_t)subcode));
		}

		if (result == KERN_SUCCESS) {
		    /*
		     * Certain faults require that we back up
		     * the EIP.
		     */
		    register struct recovery *rp;

		    for (rp = retry_table; rp < retry_table_end; rp++) {
			if (regs->eip == rp->fault_addr) {
			    regs->eip = rp->recover_addr;
			    break;
			}
		    }
		    return;
		}

		/*
		 * If there is a failure recovery address
		 * for this fault, go there.
		 */
		{
		    register struct recovery *rp;

		    for (rp = recover_table;
			 rp < recover_table_end;
			 rp++) {
			if (regs->eip == rp->fault_addr) {
			    regs->eip = rp->recover_addr;
			    return;
			}
		    }
		}

		/*
		 * Check thread recovery address also -
		 * v86 assist uses it.
		 */
		if (thread->recover) {
		    regs->eip = thread->recover;
		    thread->recover = 0;
		    return;
		}

		/*
		 * Unanticipated page-fault errors in kernel
		 * should not happen.
		 */
		/* fall through */

	    default:
#if	MACH_TTD
		if (kttd_enabled && kttd_trap(type, code, regs))
			return;
#endif	/* MACH_TTD */
#if	MACH_KDB
		if (kdb_trap(type, code, regs))
		    return;
#endif	MACH_KDB
		printf("trap type %d, code = %x, pc = %x\n",
			type, code, regs->eip);
		panic("trap");
		return;
	}
}


/*
 *	Trap from user mode.
 */
void user_trap(regs)
	register struct i386_saved_state *regs;
{
	int	exc;
	int	code;
	int	subcode;
	register int	type;
	vm_map_t	map;
	kern_return_t	result;
	register thread_t thread = current_thread();

	if (regs->efl & EFL_VM) {
	    /*
	     * If hardware assist can handle exception,
	     * continue execution.
	     */
	    if (v86_assist(thread, regs))
		return;
	}

	type = regs->trapno;
	code = 0;
	subcode = 0;

	switch (type) {

	    case T_DIVIDE_ERROR:
		exc = EXC_ARITHMETIC;
		code = EXC_I386_DIV;
		break;

	    case T_DEBUG:
#if	MACH_TTD
		if (kttd_enabled && kttd_in_single_step()) {
			if (kttd_trap(type, regs->err, regs))
				return;
		}
#endif	/* MACH_TTD */
#if	MACH_KDB
		if (db_in_single_step()) {
		    if (kdb_trap(type, regs->err, regs))
			return;
		}
#endif
		exc = EXC_BREAKPOINT;
		code = EXC_I386_SGL;
		break;

	    case T_INT3:
#if	MACH_TTD
		if (kttd_enabled && kttd_trap(type, regs->err, regs))
			return;
		break;
#endif	/* MACH_TTD */
#if	MACH_KDB
	    {
		boolean_t db_find_breakpoint_here();

		if (db_find_breakpoint_here(
			(current_thread())? current_thread()->task: TASK_NULL,
			regs->eip - 1)) {
		    if (kdb_trap(type, regs->err, regs))
			return;
		}
	    }
#endif
		exc = EXC_BREAKPOINT;
		code = EXC_I386_BPT;
		break;

	    case T_OVERFLOW:
		exc = EXC_ARITHMETIC;
		code = EXC_I386_INTO;
		break;

	    case T_OUT_OF_BOUNDS:
		exc = EXC_SOFTWARE;
		code = EXC_I386_BOUND;
		break;

	    case T_INVALID_OPCODE:
		exc = EXC_BAD_INSTRUCTION;
		code = EXC_I386_INVOP;
		break;

	    case T_NO_FPU:
	    case 32:		/* XXX */
		fpnoextflt();
		return;

	    case T_FPU_FAULT:
		fpextovrflt();
		return;

	    case 10:		/* invalid TSS == iret with NT flag set */
		exc = EXC_BAD_INSTRUCTION;
		code = EXC_I386_INVTSSFLT;
		subcode = regs->err & 0xffff;
		break;

	    case T_SEGMENT_NOT_PRESENT:
#if	FPE
		if (fp_emul_error(regs))
		    return;
#endif	/* FPE */

		exc = EXC_BAD_INSTRUCTION;
		code = EXC_I386_SEGNPFLT;
		subcode = regs->err & 0xffff;
		break;

	    case T_STACK_FAULT:
		exc = EXC_BAD_INSTRUCTION;
		code = EXC_I386_STKFLT;
		subcode = regs->err & 0xffff;
		break;

	    case T_GENERAL_PROTECTION:
		if (!(regs->efl & EFL_VM)) {
		    if (check_io_fault(regs))
			return;
		}
		exc = EXC_BAD_INSTRUCTION;
		code = EXC_I386_GPFLT;
		subcode = regs->err & 0xffff;
		break;

	    case T_PAGE_FAULT:
		subcode = regs->cr2;
		(void) vm_fault(thread->task->map,
				trunc_page((vm_offset_t)subcode),
				(regs->err & T_PF_WRITE)
				  ? VM_PROT_READ|VM_PROT_WRITE
				  : VM_PROT_READ,
				FALSE,
				FALSE,
				user_page_fault_continue);
		/*NOTREACHED*/
		break;

	    case T_FLOATING_POINT_ERROR:
		fpexterrflt();
		return;

	    default:
#if	MACH_TTD
		if (kttd_enabled && kttd_trap(type, regs->err, regs))
			return;
#endif	/* MACH_TTD */
#if	MACH_KDB
		if (kdb_trap(type, regs->err, regs))
		    return;
#endif	MACH_KDB
		printf("trap type %d, code = %x, pc = %x\n",
		       type, regs->err, regs->eip);
		panic("trap");
		return;
	}

#if	MACH_TTD
	if (debug_all_traps_with_kttd && kttd_trap(type, regs->err, regs))
		return;
#endif	/* MACH_TTD */
#if	MACH_KDB
	if (debug_all_traps_with_kdb &&
	    kdb_trap(type, regs->err, regs))
		return;
#endif	MACH_KDB

	i386_exception(exc, code, subcode);
	/*NOTREACHED*/
}

/*
 *	V86 mode assist for interrupt handling.
 */
boolean_t v86_assist_on = TRUE;
boolean_t v86_unsafe_ok = FALSE;
boolean_t v86_do_sti_cli = TRUE;
boolean_t v86_do_sti_immediate = FALSE;

#define	V86_IRET_PENDING 0x4000

int cli_count = 0;
int sti_count = 0;

boolean_t
v86_assist(thread, regs)
	thread_t	thread;
	register struct i386_saved_state *regs;
{
	register struct v86_assist_state *v86 = &thread->pcb->ims.v86s;

/*
 * Build an 8086 address.  Use only when off is known to be 16 bits.
 */
#define	Addr8086(seg,off)	((((seg) & 0xffff) << 4) + (off))

#define	EFL_V86_SAFE		(  EFL_OF | EFL_DF | EFL_TF \
				 | EFL_SF | EFL_ZF | EFL_AF \
				 | EFL_PF | EFL_CF )
	struct iret_32 {
		int		eip;
		int		cs;
		int		eflags;
	};
	struct iret_16 {
		unsigned short	ip;
		unsigned short	cs;
		unsigned short	flags;
	};
	union iret_struct {
		struct iret_32	iret_32;
		struct iret_16	iret_16;
	};

	struct int_vec {
		unsigned short	ip;
		unsigned short	cs;
	};

	if (!v86_assist_on)
	    return FALSE;

	/*
	 * If delayed STI pending, enable interrupts.
	 * Turn off tracing if on only to delay STI.
	 */
	if (v86->flags & V86_IF_PENDING) {
	    v86->flags &= ~V86_IF_PENDING;
	    v86->flags |=  EFL_IF;
	    if ((v86->flags & EFL_TF) == 0)
		regs->efl &= ~EFL_TF;
	}

	if (regs->trapno == T_DEBUG) {

	    if (v86->flags & EFL_TF) {
		/*
		 * Trace flag was also set - it has priority
		 */
		return FALSE;			/* handle as single-step */
	    }
	    /*
	     * Fall through to check for interrupts.
	     */
	}
	else if (regs->trapno == T_GENERAL_PROTECTION) {
	    /*
	     * General protection error - must be an 8086 instruction
	     * to emulate.
	     */
	    register int	eip;
	    boolean_t	addr_32 = FALSE;
	    boolean_t	data_32 = FALSE;
	    int		io_port;

	    /*
	     * Set up error handler for bad instruction/data
	     * fetches.
	     */
	    asm("movl $(addr_error), %0" : "=m" (thread->recover));

	    eip = regs->eip;
	    while (TRUE) {
		unsigned char	opcode;

		if (eip > 0xFFFF) {
		    thread->recover = 0;
		    return FALSE;	/* GP fault: IP out of range */
		}

		opcode = *(unsigned char *)Addr8086(regs->cs,eip);
		eip++;
		switch (opcode) {
		    case 0xf0:		/* lock */
		    case 0xf2:		/* repne */
		    case 0xf3:		/* repe */
		    case 0x2e:		/* cs */
		    case 0x36:		/* ss */
		    case 0x3e:		/* ds */
		    case 0x26:		/* es */
		    case 0x64:		/* fs */
		    case 0x65:		/* gs */
			/* ignore prefix */
			continue;

		    case 0x66:		/* data size */
			data_32 = TRUE;
			continue;

		    case 0x67:		/* address size */
			addr_32 = TRUE;
			continue;

		    case 0xe4:		/* inb imm */
		    case 0xe5:		/* inw imm */
		    case 0xe6:		/* outb imm */
		    case 0xe7:		/* outw imm */
			io_port = *(unsigned char *)Addr8086(regs->cs, eip);
			eip++;
			goto do_in_out;

		    case 0xec:		/* inb dx */
		    case 0xed:		/* inw dx */
		    case 0xee:		/* outb dx */
		    case 0xef:		/* outw dx */
		    case 0x6c:		/* insb */
		    case 0x6d:		/* insw */
		    case 0x6e:		/* outsb */
		    case 0x6f:		/* outsw */
			io_port = regs->edx & 0xffff;

		    do_in_out:
			if (!data_32)
			    opcode |= 0x6600;	/* word IO */

			switch (emulate_io(regs, opcode, io_port)) {
			    case EM_IO_DONE:
				/* instruction executed */
				break;
			    case EM_IO_RETRY:
				/* port mapped, retry instruction */
				thread->recover = 0;
				return TRUE;
			    case EM_IO_ERROR:
				/* port not mapped */
				thread->recover = 0;
				return FALSE;
			}
			break;

		    case 0xfa:		/* cli */
			if (!v86_do_sti_cli) {
			    thread->recover = 0;
			    return (FALSE);
			}

			v86->flags &= ~EFL_IF;
					/* disable simulated interrupts */
			cli_count++;
			break;

		    case 0xfb:		/* sti */
			if (!v86_do_sti_cli) {
			    thread->recover = 0;
			    return (FALSE);
			}

			if ((v86->flags & EFL_IF) == 0) {
			    if (v86_do_sti_immediate) {
				    v86->flags |= EFL_IF;
			    } else {
				    v86->flags |= V86_IF_PENDING;
				    regs->efl |= EFL_TF;
			    }
					/* single step to set IF next inst. */
			}
			sti_count++;
			break;

		    case 0x9c:		/* pushf */
		    {
			int	flags;
			vm_offset_t sp;
			int	size;

			flags = regs->efl;
			if ((v86->flags & EFL_IF) == 0)
			    flags &= ~EFL_IF;

			if ((v86->flags & EFL_TF) == 0)
			    flags &= ~EFL_TF;
			else flags |= EFL_TF;

			sp = regs->uesp;
			if (!addr_32)
			    sp &= 0xffff;
			else if (sp > 0xffff)
			    goto stack_error;
			size = (data_32) ? 4 : 2;
			if (sp < size)
			    goto stack_error;
			sp -= size;
			if (copyout((char *)&flags,
				    (char *)Addr8086(regs->ss,sp),
				    size))
			    goto addr_error;
			if (addr_32)
			    regs->uesp = sp;
			else
			    regs->uesp = (regs->uesp & 0xffff0000) | sp;
			break;
		    }

		    case 0x9d:		/* popf */
		    {
			vm_offset_t sp;
			int	nflags;

			sp = regs->uesp;
			if (!addr_32)
			    sp &= 0xffff;
			else if (sp > 0xffff)
			    goto stack_error;

			if (data_32) {
			    if (sp > 0xffff - sizeof(int))
				goto stack_error;
			    nflags = *(int *)Addr8086(regs->ss,sp);
			    sp += sizeof(int);
			}
			else {
			    if (sp > 0xffff - sizeof(short))
				goto stack_error;
			    nflags = *(unsigned short *)
					Addr8086(regs->ss,sp);
			    sp += sizeof(short);
			}
			if (addr_32)
			    regs->uesp = sp;
			else
			    regs->uesp = (regs->uesp & 0xffff0000) | sp;

			if (v86->flags & V86_IRET_PENDING) {
				v86->flags = nflags & (EFL_TF | EFL_IF);
				v86->flags |= V86_IRET_PENDING;
			} else {
				v86->flags = nflags & (EFL_TF | EFL_IF);
			}
			regs->efl = (regs->efl & ~EFL_V86_SAFE)
				     | (nflags & EFL_V86_SAFE);
			break;
		    }
		    case 0xcf:		/* iret */
		    {
			vm_offset_t sp;
			int	nflags;
			int	size;
			union iret_struct iret_struct;

			v86->flags &= ~V86_IRET_PENDING;
			sp = regs->uesp;
			if (!addr_32)
			    sp &= 0xffff;
			else if (sp > 0xffff)
			    goto stack_error;

			if (data_32) {
			    if (sp > 0xffff - sizeof(struct iret_32))
				goto stack_error;
			    iret_struct.iret_32 =
				*(struct iret_32 *) Addr8086(regs->ss,sp);
			    sp += sizeof(struct iret_32);
			}
			else {
			    if (sp > 0xffff - sizeof(struct iret_16))
				goto stack_error;
			    iret_struct.iret_16 =
				*(struct iret_16 *) Addr8086(regs->ss,sp);
			    sp += sizeof(struct iret_16);
			}
			if (addr_32)
			    regs->uesp = sp;
			else
			    regs->uesp = (regs->uesp & 0xffff0000) | sp;

			if (data_32) {
			    eip	      = iret_struct.iret_32.eip;
			    regs->cs  = iret_struct.iret_32.cs & 0xffff;
			    nflags    = iret_struct.iret_32.eflags;
			}
			else {
			    eip       = iret_struct.iret_16.ip;
			    regs->cs  = iret_struct.iret_16.cs;
			    nflags    = iret_struct.iret_16.flags;
			}

			v86->flags = nflags & (EFL_TF | EFL_IF);
			regs->efl = (regs->efl & ~EFL_V86_SAFE)
				     | (nflags & EFL_V86_SAFE);
			break;
		    }
		    default:
			/*
			 * Instruction not emulated here.
			 */
			thread->recover = 0;
			return FALSE;
		}
		break;	/* exit from 'while TRUE' */
	    }
	    regs->eip = (regs->eip & 0xffff0000 | eip);
	}
	else {
	    /*
	     * Not a trap we handle.
	     */
	    thread->recover = 0;
	    return FALSE;
	}

	if ((v86->flags & EFL_IF) && ((v86->flags & V86_IRET_PENDING)==0)) {

	    struct v86_interrupt_table *int_table;
	    int int_count;
	    int vec;
	    int i;

	    int_table = (struct v86_interrupt_table *) v86->int_table;
	    int_count = v86->int_count;

	    vec = 0;
	    for (i = 0; i < int_count; int_table++, i++) {
		if (!int_table->mask && int_table->count > 0) {
		    int_table->count--;
		    vec = int_table->vec;
		    break;
		}
	    }
	    if (vec != 0) {
		/*
		 * Take this interrupt
		 */
		vm_offset_t	sp;
		struct iret_16 iret_16;
		struct int_vec int_vec;

		sp = regs->uesp & 0xffff;
		if (sp < sizeof(struct iret_16))
		    goto stack_error;
		sp -= sizeof(struct iret_16);
		iret_16.ip = regs->eip;
		iret_16.cs = regs->cs;
		iret_16.flags = regs->efl & 0xFFFF;
		if ((v86->flags & EFL_TF) == 0)
		    iret_16.flags &= ~EFL_TF;
		else iret_16.flags |= EFL_TF;

#ifdef	gcc_1_36_worked
		int_vec = ((struct int_vec *)0)[vec];
#else
		bcopy((char *) (sizeof(struct int_vec) * vec),
		      (char *)&int_vec,
		      sizeof (struct int_vec));
#endif
		if (copyout((char *)&iret_16,
			    (char *)Addr8086(regs->ss,sp),
			    sizeof(struct iret_16)))
		    goto addr_error;
		regs->uesp = (regs->uesp & 0xFFFF0000) | (sp & 0xffff);
		regs->eip = int_vec.ip;
		regs->cs  = int_vec.cs;
		regs->efl  &= ~EFL_TF;
		v86->flags &= ~(EFL_IF | EFL_TF);
		v86->flags |= V86_IRET_PENDING;
	    }
	}

	thread->recover = 0;
	return TRUE;

	/*
	 *	On address error, report a page fault.
	 *	XXX report GP fault - we don`t save
	 *	the faulting address.
	 */
    addr_error:
	asm("addr_error:;");
	thread->recover = 0;
	return FALSE;

	/*
	 *	On stack address error, return stack fault (12).
	 */
    stack_error:
	thread->recover = 0;
	regs->trapno = T_STACK_FAULT;
	return FALSE;
}

/*
 * Handle AST traps for i386.
 * Check for delayed floating-point exception from
 * AT-bus machines.
 */
void
i386_astintr()
{
	int	mycpu = cpu_number();

	(void) splsched();	/* block interrupts to check reasons */
	if (need_ast[mycpu] & AST_I386_FP) {
	    /*
	     * AST was for delayed floating-point exception -
	     * FP interrupt occured while in kernel.
	     * Turn off this AST reason and handle the FPU error.
	     */
	    ast_off(mycpu, AST_I386_FP);
	    (void) spl0();

	    fpexterrflt();
	}
	else {
	    /*
	     * Not an FPU trap.  Handle the AST.
	     * Interrupts are still blocked.
	     */
	    ast_taken();
	}
}

/*
 * Handle exceptions for i386.
 *
 * If we are an AT bus machine, we must turn off the AST for a
 * delayed floating-point exception.
 *
 * If we are providing floating-point emulation, we may have
 * to retrieve the real register values from the floating point
 * emulator.
 */
void
i386_exception(exc, code, subcode)
	int	exc;
	int	code;
	int	subcode;
{
	spl_t	s;

	/*
	 * Turn off delayed FPU error handling.
	 */
	s = splsched();
	ast_off(cpu_number(), AST_I386_FP);
	splx(s);

#if	FPE
	fpe_exception_fixup(exc, code, subcode);
#else
	exception(exc, code, subcode);
#endif
	/*NOTREACHED*/
}

boolean_t
check_io_fault(regs)
	struct i386_saved_state *regs;
{
	int		eip, opcode, io_port;
	boolean_t	data_16 = FALSE;

	/*
	 * Get the instruction.
	 */
	eip = regs->eip;

	for (;;) {
	    opcode = inst_fetch(eip, regs->cs);
	    eip++;
	    switch (opcode) {
		case 0x66:	/* data-size prefix */
		    data_16 = TRUE;
		    continue;

		case 0xf3:	/* rep prefix */
		case 0x26:	/* es */
		case 0x2e:	/* cs */
		case 0x36:	/* ss */
		case 0x3e:	/* ds */
		case 0x64:	/* fs */
		case 0x65:	/* gs */
		    continue;

		case 0xE4:	/* inb imm */
		case 0xE5:	/* inl imm */
		case 0xE6:	/* outb imm */
		case 0xE7:	/* outl imm */
		    /* port is immediate byte */
		    io_port = inst_fetch(eip, regs->cs);
		    eip++;
		    break;

		case 0xEC:	/* inb dx */
		case 0xED:	/* inl dx */
		case 0xEE:	/* outb dx */
		case 0xEF:	/* outl dx */
		case 0x6C:	/* insb */
		case 0x6D:	/* insl */
		case 0x6E:	/* outsb */
		case 0x6F:	/* outsl */
		    /* port is in DX register */
		    io_port = regs->edx & 0xFFFF;
		    break;

		default:
		    return FALSE;
	    }
	    break;
	}

	if (data_16)
	    opcode |= 0x6600;		/* word IO */

	switch (emulate_io(regs, opcode, io_port)) {
	    case EM_IO_DONE:
		/* instruction executed */
		regs->eip = eip;
		return TRUE;

	    case EM_IO_RETRY:
		/* port mapped, retry instruction */
		return TRUE;

	    case EM_IO_ERROR:
		/* port not mapped */
		return FALSE;
	}
}

#if	MACH_PCSAMPLE > 0
/*
 * return saved state for interrupted user thread
 */
unsigned
interrupted_pc(t)
	thread_t t;
{
	register struct i386_saved_state *iss;

 	iss = USER_REGS(t);
 	return iss->eip;
}
#endif	/* MACH_PCSAMPLE > 0*/

