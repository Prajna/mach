/*
 * Mach Operating System
 * Copyright (c) 1993-1989 Carnegie Mellon University
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
 * $Log:	trap.c,v $
 * Revision 2.25  93/05/15  19:13:40  mrt
 * 	machparam.h -> machspl.h
 * 
 * Revision 2.24  93/01/24  13:22:03  danner
 * 	Add include of mach_pcsample.h.
 * 
 * 
 * Revision 2.23  93/01/21  12:25:35  danner
 * 	Fast tas, pc sample support.
 * 	[93/01/20            bershad]
 * 
 * Revision 2.22  93/01/14  17:52:51  danner
 * 	Proper spl typing.
 * 	[92/12/01            af]
 * 
 * Revision 2.21  92/07/20  13:33:21  cmaeda
 * 	Turn off fast tas debugging messages.
 * 	[92/05/11  14:50:19  cmaeda]
 * 
 * 	Added recover_ras function.  Resets the pc of a thread
 * 	inside an ras to the beginning of the ras.
 * 	[92/05/11  14:39:46  cmaeda]
 * 
 * Revision 2.20  92/04/01  19:35:40  rpd
 * 	Changed trap to make all ADEL/ADES exceptions use
 * 	EXC_BAD_INSTRUCTION/EXC_MIPS_RESADDR.
 * 	[92/03/24            rpd]
 * 
 * Revision 2.19  91/11/12  11:17:10  rvb
 * 	Use the ss_count field as a flag to prevent re-entering
 * 	prepare_sstep() after, e.g. an AST go tus on the way out
 * 	to user space.  Looong overdue bugfix.
 * 	[91/10/13  17:21:59  af]
 * 
 * 	On breakpoint exceptions we were taking the instruction
 * 	from the wrong place.
 * 	[91/09/29            af]
 * 
 * Revision 2.18  91/10/09  16:15:09  af
 * 	On breakpoint exceptions we were taking the instruction
 * 	from the wrong place.
 * 	[91/09/29            af]
 * 
 * Revision 2.17  91/08/24  12:24:35  af
 * 	Spl defs, and fixed disguised splsoftclock().
 * 	[91/08/02  03:12:05  af]
 * 
 * Revision 2.16  91/05/18  14:37:14  rpd
 * 	Fixed double-miss handling in tlb_miss.  Now it maps an
 * 	invalid entry into the tlb instead of going to trap.
 * 	[91/04/04            rpd]
 * 
 * Revision 2.15  91/05/14  17:39:25  mrt
 * 	Correcting copyright
 * 
 * Revision 2.14  91/05/13  06:06:37  af
 * 	Removed optimization in interrupt() that would handle more
 * 	than one interrupt at a time.  See comments in interrupt()
 * 	for the reasons, and to decide wether you can still use this
 * 	optim or not, by default it is now disabled.
 * 	[91/05/12  15:57:19  af]
 * 
 * Revision 2.13  91/03/16  14:57:34  rpd
 * 	Added mips_sstep_state.
 * 	[91/02/18            rpd]
 * 	Added resume, continuation arguments to vm_fault.
 * 	Added user_page_fault_continue.
 * 	[91/02/04            rpd]
 * 	Removed EXC_AST.
 * 	[91/01/22  16:01:04  rpd]
 * 
 * Revision 2.12  91/02/14  14:37:56  mrt
 * 	Pass along the spl level to interrupt routines, which are now
 * 	invoked with interrupts DISABLED so that they can defend 
 * 	themselves from unwarranted multiple invocations.  It is
 * 	now the routine's responsibility to lower the spl at the indicated
 * 	priority level as soon as the interrupt cause has been cleared.
 * 	[91/02/12  12:28:43  af]
 * 
 * 	Activated mipsbox_memory_check() macro.
 * 	[91/01/03  02:08:14  af]
 * 
 * Revision 2.11  91/02/05  17:52:19  mrt
 * 	Added author notices
 * 	[91/02/04  11:25:35  mrt]
 * 
 * 	Changed to use new Mach copyright
 * 	[91/02/02  12:30:22  mrt]
 * 
 * Revision 2.10  91/01/09  19:54:20  rpd
 * 	Check mipsbox_memory_check return value.
 * 	[91/01/09            rpd]
 * 
 * Revision 2.9  91/01/08  15:52:10  rpd
 * 	Added a couple counters to tlb_miss.
 * 	[91/01/08  14:15:03  rpd]
 * 
 * 	Disabled pcache code.
 * 	[90/12/29            rpd]
 * 	Replaced thread_doexception with new exception interface.
 * 	Moved thread_exception_return here from mips/locore.s.
 * 	It must install single-step breakpoints in the user.
 * 	Added thread_kdb_return.
 * 	[90/12/23            rpd]
 * 	Added continuation argument to thread_block.
 * 	[90/12/08            rpd]
 * 
 * 	Added support for copymsgin, copymsgout.
 * 	[90/12/05            rpd]
 * 
 * 	Changed for new pcb organization.
 * 	[90/11/12            rpd]
 * 
 * Revision 2.7  90/12/05  20:50:19  af
 * 	Moved stray_intr() here, I had it in a machdep place.
 * 	Added pcache filling code that got lost in the woods.
 * 	[90/12/03            af]
 * 
 * Revision 2.6  90/10/25  14:47:07  rwd
 * 	Removed static from getreg_val.
 * 	[90/10/17            rpd]
 * 	Generalized the watchpoint support.
 * 	[90/10/16            rwd]
 * 	Added watchpoint support.
 * 	[90/10/16            rpd]
 * 
 * Revision 2.5  90/08/07  22:30:29  rpd
 * 	Support for 3max.  Added debugging of user traps with KDB.
 * 	removed some debug code.  Made clearing of bus errors box
 * 	specific, and hopefully done down in locore.
 * 	[90/08/07  15:20:01  af]
 * 
 * Revision 2.3  90/01/22  23:08:40  af
 * 	Only remove sstep breakpoints after hitting them, or an AST
 * 	would cancel.  Fixed typo in reading in the break-ed instruction.
 * 	[90/01/22            af]
 * 	Added full debugger support.  This includes implementing single-stepping
 * 	(exported via thread_set_status) and fixing various pending bugs and
 * 	misunderstandings. [I did not want to rewrite all the debuggers..]
 * 	After discussion with the Lisp folks at Franz, removed fixing of
 * 	unaligned memory accesses. It was just a mis-feature that noone used.
 * 	Invoke kdb or kill the task on exceptions that are not
 * 	handled by thread_doexception().
 * 	Clear thread->recover here after use.
 * 	[89/12/09  11:06:41  af]
 * 
 * Revision 2.2  89/11/29  14:15:39  af
 * 	Implemented for real fpa_intr, and moved it elsewhere.
 * 	[89/11/26  10:33:10  af]
 * 
 * 	Fixed bad interaction between thread_recover and copyout:
 * 	touching a user page for the first time would not work.
 * 	Scheduling of the FPA works, removed some dead code.
 * 	Still need to handle FPA exceptions.
 * 	[89/11/16  14:40:58  af]
 * 
 * 	Removed single step support: just deliver the breakpoint
 * 	exception with adequate information.  The U*x emulator
 * 	can do the rest all by itself.  Or the debugger.
 * 	[89/10/30            af]
 * 
 * 	Created.
 * 	[89/10/06            af]
 */
/*
 *	File: trap.c
 * 	Author: Alessandro Forin, Carnegie Mellon University
 *	Date:	10/89
 *
 *	Trap Handlers for MIPS
 */

#include <mach_kdb.h>
#include <fast_tas.h>
#include <mach_pcsample.h>

#include <machine/machspl.h>		/* spl definitions */
#include <mach/exception.h>
#include <mach/vm_param.h>
#include <mach/mips/mips_instruction.h>
#include <kern/thread.h>
#include <kern/zalloc.h>
#include <vm/vm_kern.h>
#include <mips/ast.h>
#include <mips/mips_box.h>
#include <mips/mips_cpu.h>

extern void exception();
extern void thread_exception_return();
extern void thread_bootstrap_return();

extern zone_t msss_zone;

extern void pmap_pte_fault();
extern char TRAP_tlb_umiss[], TRAP_tlb_umiss_load[];

#define real_tlb_umiss_load						\
	(VEC_TLB_UMISS +						\
	 ((unsigned)TRAP_tlb_umiss_load - (unsigned)TRAP_tlb_umiss))

int fast_thread_recover;
extern char copymsg_start[], copymsg_end[];
extern int copymsg_error();

int trap_debug = 1;
/*#define TRACE(lev,x) if (trap_debug>lev) x*/
#define TRACE(lev,x)

char *cause_fmt = CAUSE_FMT;
char *status_fmt = SR_FMT;

boolean_t tlb_miss_map_enable = TRUE;
int tlb_miss_next = 0;

void
tlb_miss_map(pid, vaddr, pte)
	tlbpid_t pid;
	vm_offset_t vaddr;
	unsigned int pte;
{
	if (tlb_miss_map_enable) {
		tlb_map(tlb_miss_next, pid, vaddr, pte);
		if (++tlb_miss_next == 8)
			tlb_miss_next = 0;
	} else
		tlb_map_random(pid, vaddr, pte);
}

/*
 *	Object:
 *		tlb_miss			EXPORTED function
 *
 *		Trap handler for TLB misses
 *
 *	This is the one that knows about the layout of the kernel
 *	virtual space. Depending on the virtual address, the miss
 *	could be on the kernel/user page tables, or on a kernel/user
 *	virtual address proper.
 *	The twist is to recognize TLB misses that happened within
 *	the tlb_umiss handler, and munge the stack appropriately
 *	in case, to return to the real place of the original miss.
 *
 */

int tlb_miss_bug = 0;
int tlb_miss_double = 0;	/* double misses */
int tlb_miss_double_punt = 0;
int tlb_miss_kernel_pt = 0;	/* misses on kernel page table pages */
int tlb_miss_user_pt = 0;	/* misses on user page table pages */
int tlb_miss_kernel = 0;	/* misses on kernel pages */
int tlb_miss_user = 0;		/* misses on user pages */

tlb_miss(ss_ptr, addr, cause)
	struct mips_saved_state *ss_ptr;
	vm_offset_t addr;
	unsigned cause;
{
	register pt_entry_t *pte;
	pmap_t pmap;

	TRACE(2,{printf("tlb_miss: %x %x cs=%b ucs=%b\n",	ss_ptr, addr,
			cause, cause_fmt, tlb_umiss_cause, cause_fmt);})

	cause &= CAUSE_EXC_MASK;

	if (addr == VEC_EXCEPTION) {
		tlb_miss_bug++;
		addr = ss_ptr->pc;
	}

	if (ss_ptr->pc == real_tlb_umiss_load) {
		vm_offset_t user_address;

		tlb_miss_double++;

		/*
		 *	Double miss.
		 */

		pmap = active_pmap;
		user_address = (addr & TLB_CXT_VPN_MASK) << (MIPS_PGSHIFT-2);

		/*
		 *	We set the exception PC to return to the place that
		 *	caused the original tlb_umiss [Kane] pag 5-24.
		 *	We never go to trap from here, so we don't bother
		 *	fixing ss_ptr->bad_address.
		 */

		ss_ptr->pc = ss_ptr->k1;

		/*
		 *	Catch quickly bogus pointers
		 */
		if (user_address < VM_MIN_ADDRESS) {
		    punt_double_miss:
			tlb_miss_double_punt++;

			/*
			 *	We don't go to trap/vm_fault, because
			 *	we have lost information about the miss.
			 *	We don't know the exact address or whether
			 *	it was a read or write.  So we map an
			 *	invalid entry into the tlb and retry,
			 *	to get this information.
			 */

			tlb_map_random(pmap->pid, user_address, 0);
			return 0;
		}

		/*
		 *	See if we only dropped the mapping from the tlb
		 */
		pte = pmap_pte(kernel_pmap, addr);
		if (!pte->pg_v)
			goto punt_double_miss;

		tlb_miss_map(0, addr, *(int*)pte);

		/*
		 *	Retry now
		 */
		pte = pmap_pte( pmap, user_address);
		if (!pte->pg_v)
			goto punt_double_miss;

		tlb_map_random(pmap->pid, user_address, *(int*)pte);

	} else if (addr >= KPTEADDR) {
		tlb_miss_kernel_pt++;

		/*
		 *	Miss on the kernel's page tables.
		 */
		pte = pmap_root_pte(addr);
		if (!pte->pg_v)
			pmap_pte_fault(kernel_pmap, pte);

		tlb_map_random(0, addr, *(int*)pte);

	} else if (addr >= VM_MAX_KERNEL_ADDRESS) {
		tlb_miss_user_pt++;

		/*
		 *	Miss on a user page table
		 *	We get here when the pmap module read/writes
		 *	a user pte which is either not there (page
		 *	is missing) or not in the tlb.
		 */
		pte = pmap_pte( kernel_pmap, addr);
		if (!pte->pg_v) {
			/*
			 *	The ptepage is missing, find out from
			 *	which pmap.
			 */
			extern pmap_t virt_to_pmap();
			pmap = virt_to_pmap(addr);
			pmap_pte_fault( pmap, pte);
		}

		tlb_miss_map(0, addr, *(int*)pte);

	} else if (addr >= VM_MIN_KERNEL_ADDRESS) {
		tlb_miss_kernel++;

		/*
		 * Miss on a kernel virtual address proper
		 */
		pte = pmap_pte(kernel_pmap, addr);
		if (!pte->pg_v)
			return cause;

		tlb_map_random(0, addr, *(int*)pte);

	} else {
		tlb_miss_user++;

		/*
		 * If we get here it's because the utlbmiss handler did
		 * find the pte for the page, but it was invalid.
		 * Therefore we go straight to the VM system.
		 */
		return cause;
	}

	return 0;	/* success */
}

/*
 *	Object:
 *		tlb_mod				EXPORTED function
 *
 *		Trap handler for TLB MOD exceptions
 *
 *	This handler is used to maintain dirty bits in software.
 *	It's the call to pmap_set_modify() that matters.
 *
 */
tlb_mod(ss_ptr, addr)
	struct mips_saved_state *ss_ptr;
	vm_offset_t addr;
{
	register pmap_t map;
	register pt_entry_t *pte;

	TRACE(2,{printf("tlb_mod: %x %x\n", ss_ptr, addr);})

	if (ISA_K2SEG(addr))
		map = kernel_pmap;
	else
		map = current_thread()->task->map->pmap;

	pte = pmap_pte(map, addr);

	if ((pte->pg_prot & VM_PROT_WRITE) == 0)
		return EXC_TLBS;

	pte->pg_m = 1;
	pmap_set_modify( PTETOPHYS(pte));
	tlb_modify(map->pid, addr, 1);
	return 0;
}

#if	MACH_KDB
int debug_all_traps_with_kdb = 0;
extern struct db_watchpoint *db_watchpoint_list;
extern boolean_t db_watchpoints_inserted;

void
thread_kdb_return()
{
	if (kdb_trap(&current_thread()->pcb->mss, 1)) {
		thread_exception_return();
		/*NOTREACHED*/
	}
}
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
			register struct mips_saved_state *mss =
				&self->pcb->mss;

			if (((mss->cause & CAUSE_EXC_MASK) == EXC_TLBS) &&
			    db_find_watchpoint(map, mss->bad_address, mss))
				(void) kdb_trap(mss, 2);
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
 *		trap				EXPORTED function
 *
 *		Generic trap handler.
 *
 *	All exceptions that cannot be handled quickly endup here.
 *
 */

trap(ss_ptr, cause, status)
	struct mips_saved_state *ss_ptr;
	unsigned cause, status;
{
	thread_t	t;
	kern_return_t   ret;
	vm_map_t	map;
	vm_offset_t     vaddr;
	int		exc_type, exc_code;
	unsigned getreg_val();

	TRACE(1,{printf("trap: %x cs=%b st=%b\n", ss_ptr,
			cause, cause_fmt, status, status_fmt);})

	t = current_thread();

	if (status & SR_KUo)
		goto user_mode_traps;

	/*
	 *	Trap while in Kernel mode
	 */
	switch (cause & CAUSE_EXC_MASK) {
	case EXC_TLBL:
	case EXC_TLBS:	/* from tlbmiss/tlbmod */
		vaddr = trunc_page(ss_ptr->bad_address);
		cause &= CAUSE_EXC_MASK;

		TRACE(1,{ printf("trap kSegv at x%x\n", ss_ptr->bad_address);})

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

		ret = vm_fault(map, vaddr,
			       (cause == EXC_TLBS)
			       		? VM_PROT_READ|VM_PROT_WRITE
					: VM_PROT_READ,
			       FALSE, FALSE, (void (*)()) 0);

		if (ret == KERN_SUCCESS) {
#if	MACH_KDB
			if (db_watchpoint_list &&
			    db_watchpoints_inserted &&
			    ((cause & CAUSE_EXC_MASK) == EXC_TLBS) &&
			    db_find_watchpoint((VM_MIN_ADDRESS <= vaddr) &&
					       (vaddr < VM_MAX_ADDRESS) &&
					       (t != THREAD_NULL) ?
					       t->task->map : kernel_map,
					       ss_ptr->bad_address,
					       ss_ptr))
				(void) kdb_trap(ss_ptr, 2);
#endif	MACH_KDB
			return;
		}

		if (((unsigned) copymsg_start <= ss_ptr->pc) &&
		    (ss_ptr->pc < (unsigned) copymsg_end)) {
			ss_ptr->pc = (unsigned) copymsg_error;
			return;
		}
		if (fast_thread_recover) {
			ss_ptr->pc = fast_thread_recover;
			fast_thread_recover = 0;
			return;
		}
		if (t->recover) {
			ss_ptr->pc = t->recover;
			t->recover = 0;
			return;
		}
		break;

	case EXC_DBE:
		/*
		 *	[Kane] must look at instruction to find
		 *	faulting address.
		 */
		if (!isa_load_store( *(unsigned *)((cause&CAUSE_BD)?
					ss_ptr->pc + 4 : ss_ptr->pc),
				    &ss_ptr->bad_address,
				    getreg_val, ss_ptr))
			panic("DBE");
		/* fall through */
	case EXC_IBE:
		if (mipsbox_memory_check(ss_ptr))
			return;
	case EXC_ADEL:
	case EXC_ADES:
		if (((unsigned) copymsg_start <= ss_ptr->pc) &&
		    (ss_ptr->pc < (unsigned) copymsg_end)) {
			ss_ptr->pc = (unsigned) copymsg_error;
			return;
		}
		if (fast_thread_recover) {
			ss_ptr->pc = fast_thread_recover;
			fast_thread_recover = 0;
			return;
		}
		if (t->recover) {
			ss_ptr->pc = t->recover;
			t->recover = 0;
			return;
		}
		/* fall through */
	default:
		break;
	}

	goto fatal;

	/*
	 *	Trap while in User mode
	 */
user_mode_traps:

	switch (cause & CAUSE_EXC_MASK) {
	case EXC_TLBL:
	case EXC_TLBS:
		cause &= CAUSE_EXC_MASK;
		vaddr = trunc_page(ss_ptr->bad_address);

		TRACE(1,{ printf("trap uSegv at %x, badva 0x%x\n",
			ss_ptr->pc, ss_ptr->bad_address);})

		map = t->task->map;

		(void) vm_fault(map, vaddr,
				(cause == EXC_TLBS)
					? VM_PROT_READ|VM_PROT_WRITE
					: VM_PROT_READ,
				FALSE,
				FALSE, user_page_fault_continue);
		/*NOTREACHED*/
		break;

	case EXC_RI:		/* Illegal instr */
		exc_type = EXC_BAD_INSTRUCTION;
		exc_code = EXC_MIPS_RESOPND;
		break;
		
	case EXC_CU: {	/* coproc unusable */
		/*
		 *	We might be here because the thread is trying
		 *	to use floating point, and it doesn't have
		 *	a floating-point save area yet.
		 */

		if (t->pcb->mms.mfs == 0) {
			pcb_fpa_init(t);
			return;
		}

		exc_type = EXC_SOFTWARE;
		exc_code = EXC_MIPS_SOFT_CPU;
		break;
	}
	case EXC_OVF:		/* Overflow */
		exc_type = EXC_ARITHMETIC;
		exc_code = EXC_MIPS_FLT_OVERFLOW;
		break;

	case EXC_DBE: {		/* Data Bus Error */
		unsigned ins;

		/*
		 *	See EXC_DBE above.
		 */
		copyin( (cause&CAUSE_BD) ? ss_ptr->pc+4 : ss_ptr->pc,
			&ins, sizeof ins);
		isa_load_store(ins, &ss_ptr->bad_address,
				getreg_val, ss_ptr);
		/* fall through */
	}
	case EXC_IBE:		/* Instruction Bus Error */
		mipsbox_memory_check(ss_ptr);
		exc_type = EXC_BAD_ACCESS;
		exc_code = KERN_PROTECTION_FAILURE;
		cause = ss_ptr->bad_address;	/* GDB */
		break;

	case EXC_ADEL:	/* Read Address Error */
	case EXC_ADES:	/* Write Address Error */
		exc_type = EXC_BAD_INSTRUCTION;
		exc_code = EXC_MIPS_RESADDR;
		cause = ss_ptr->bad_address;
		break;

	case EXC_BP: {		/* Breakpoint */
		register struct mips_sstep_state *msss;

		/*
		 * If single stepping, remove breakpoints
		 * to minimize damage to other fellow threads.
		 * Tell the user whether this was a single-step or not.
		 * To make U*x compat easier, we pass along the instruction
		 * that faulted, for quick decoding of the break code.
		 */
		msss = t->pcb->mms.msss;
		if (msss) {
			t->pcb->mms.msss = 0;
			exc_code = BREAK_SSTEP;
			did_sstep(msss);
			zfree(msss_zone, (vm_offset_t) msss);
		} else
			exc_code = BREAK_USER;

		exc_type = EXC_BREAKPOINT;
		copyin(((cause&CAUSE_BD) ?
			ss_ptr->pc+4 : ss_ptr->pc),
			&cause, 4);
		break;
	}

	case EXC_SYS:

		exc_type = EXC_BAD_INSTRUCTION;
		exc_code = EXC_MIPS_RESOPND;
		break;

	default:
		goto fatal;
	}

#if	MACH_KDB
	if (debug_all_traps_with_kdb && kdb_trap(ss_ptr, 1))
		return;
#endif	MACH_KDB

	/* Deliver the exception */
	exception(exc_type, exc_code, cause);
	/*NOTREACHED*/
	return; /* help for the compiler */

fatal:
#if	MACH_KDB
	if (kdb_trap(ss_ptr, 1))
		return;
#endif	MACH_KDB
	(void) splhigh();
	dprintf("Fatal kernel trap:\n\tcause=%b, status=%b, ",
		cause, cause_fmt, status, status_fmt); 
	dprintf("pc = %x, va = %x, sp = %x\n",
			ss_ptr->pc, ss_ptr->bad_address, ss_ptr);
	halt_all_cpus(1);
	/* NOTREACHED */
}

void
thread_exception_return()
{
	register thread_t t = current_thread();
	register pcb_t pcb = t->pcb;
	register struct mips_sstep_state *msss = pcb->mms.msss;

	/*
	 *	If single stepping, install breakpoints before
	 *	getting back to user.  This should be done as
	 *	late as possible, to minimize the (unavoidable)
	 *	interference with other threads in the same task.
	 *	It would be nice to do it _after_ ASTs are taken.
	 */
	if (msss)
	  {
	    prepare_sstep(msss, &pcb->mss);
#if FAST_TAS		
	    if (recover_ras(t))  
	      printf("Warning: single stepping through RAS (0x%x)\n",
		     t->pcb->mss.pc);
#endif		    
	  } 
#if FAST_TAS 
         else (void)recover_ras(t);
#endif FAST_TAS

	thread_bootstrap_return();
	/*NOTREACHED*/
}

/*
 *	Object:
 *		interrupt			EXPORTED function
 *
 *		Trap handler for external interrupts
 *
 *	Dispatch to device handler asap.
 *
 */
interrupt(ss_ptr, cause, status)
	struct mips_saved_state *ss_ptr;
	register unsigned	 cause;
	register unsigned	 status;
{
	/*
	 *	Only take those that are not masked
	 */
	cause &= (status & SR_INT_MASK);

#if 0
	/* NOTE: this optimized scheme of dispatching interrupts
	 * might or might not be acceptable depending on your
	 * device drivers.  In general, it might fail and it is
	 * therefore not enabled.
	 * The (very infrequent) cases to watch for are like:
	 * handler A is running at spl N, interrupt N+1 comes in
	 * before the handler removes the cause for its interrupt,
	 * handler B runs and returns, we call handler A
	 * --> we call handler A twice for the same interrupt.
	 * A similar situation might arise if interrupts A and B
	 * are both pending and enabled and the higher priority
	 * one (A) lowers its priority below that of B thereby
	 * letting the interrupt for B in again.
	 * [On a pmax this happens frequently with A==clock and
	 *  B==scsi.]  Some drivers might be able to quickly
	 * dismiss the interrupt bit and solve the problem that
	 * way, in which case this code can profitably be enabled.
	 * That is the case for a 3max with no SII option board, sigh.
	 */

	/*
	 *	In decreasing priority order, enabling the next
	 *	higher priority interrupts, call the interrupt
	 *	handler for the device that is connected to this
	 *	interrupt level on this box.
	 */
	if (cause & IP_LEV7) {
		mipsbox_intr7(ss_ptr, cause, status, SR_IEc);
	}
	if (cause & IP_LEV6) {
		mipsbox_intr6(ss_ptr, cause, status, SR_IEc|INT_LEV7);
	}
	if (cause & IP_LEV5) {
		mipsbox_intr5(ss_ptr, cause, status, SR_IEc|INT_LEV6);
	}
	if (cause & IP_LEV4) {
		mipsbox_intr4(ss_ptr, cause, status, SR_IEc|INT_LEV5);
	}
	if (cause & IP_LEV3) {
		mipsbox_intr3(ss_ptr, cause, status, SR_IEc|INT_LEV4);
	}
	if (cause & IP_LEV2) {
		mipsbox_intr2(ss_ptr, cause, status, SR_IEc|INT_LEV3);
	}
	if (cause & IP_LEV1) {
		clearsoftclock();
		splx(SR_IEc|INT_LEV2);
		softclock();
	}
#else
	/*
	 * Only take the highest priority one
	 */
	if (cause & IP_LEV7) {
		mipsbox_intr7(ss_ptr, cause, status, SR_IEc);
		return;
	}
	if (cause & IP_LEV6) {
		mipsbox_intr6(ss_ptr, cause, status, SR_IEc|INT_LEV7);
		return;
	}
	if (cause & IP_LEV5) {
		mipsbox_intr5(ss_ptr, cause, status, SR_IEc|INT_LEV6);
		return;
	}
	if (cause & IP_LEV4) {
		mipsbox_intr4(ss_ptr, cause, status, SR_IEc|INT_LEV5);
		return;
	}
	if (cause & IP_LEV3) {
		mipsbox_intr3(ss_ptr, cause, status, SR_IEc|INT_LEV4);
		return;
	}
	if (cause & IP_LEV2) {
		mipsbox_intr2(ss_ptr, cause, status, SR_IEc|INT_LEV3);
		return;
	}
	if (cause & IP_LEV1) {
		spl_t	s;

		clearsoftclock();
		s = splsoftclock();
		softclock();
		splx(s);
		return;
	}
#endif
	if (cause & IP_LEV0)
		panic("ip_lev0");
}


/*
 *	Object:
 *		getreg_val			EXPORTED function
 *
 *	Return the value of register in the exception frame
 *
 */
unsigned
getreg_val(regn, ss_ptr)
	register unsigned regn;
	struct mips_saved_state *ss_ptr;
{
	if (regn == 0)
		return 0;
	return *(&ss_ptr->at + regn - 1);
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
#define SSTEP_INSTRUCTION (0x0005000D)

prepare_sstep(msss, mss)
	register struct mips_sstep_state *msss;
	register struct mips_saved_state *mss;
{
	extern unsigned		branch_taken();
	unsigned		ins;
	unsigned		*brpc, *pc = (unsigned *) mss->pc;
	register struct breakpoint *bp = msss->ss_bp;

	/*
	 * NOTE: we might reenter because an AST stopped us on the way out
	 */
	if (msss->ss_count)
		return;

	/* We don't put bp in bdslots */
	if (copyin(pc, &ins, sizeof ins))
		return;			/* no harm done */

	if (isa_call(ins) || isa_branch(ins)) {
		brpc = (unsigned *) branch_taken(ins, pc, getreg_val, mss);
		if (brpc != pc) {
			bp->address = brpc;
			if (copyin(brpc, &bp->instruction, sizeof ins))
				goto seq;	/* he'll get hurt */	
			if (poke_instruction(brpc, SSTEP_INSTRUCTION))
				goto seq;	/* ditto */
			msss->ss_count++, bp++;
		}
		pc += 1;		/* skip bdslot */
	}
seq:
	bp->address = pc += 1;
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
	register struct mips_sstep_state *msss;
{
	register int 		 i;
	register unsigned	*pc;
	unsigned		 ins;
	register struct breakpoint *bp = msss->ss_bp;

	for (; msss->ss_count-- > 0; bp++) {
		pc = bp->address;
		if (copyin(pc, &ins, sizeof ins))
			continue;
		if (ins != SSTEP_INSTRUCTION)
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
poke_instruction(loc, val)
	unsigned	*loc;
	unsigned	 val;
{
	vm_map_t	 map = current_thread()->task->map;
	vm_offset_t	 pa;
	kern_return_t	 ret;	
again:
	pa = pmap_extract(map->pmap, loc);
	if (pa == 0) {
		ret = vm_fault(map, trunc_page(loc), VM_PROT_READ,
			       FALSE, FALSE, (void (*)()) 0);
		if (ret != KERN_SUCCESS)
			return 1;
		goto again;
	}
	loc = (unsigned*)(PHYS_TO_K0SEG(pa));
	*loc = val;
	mipscache_Iflush(loc, 4);
	return 0;
}

/*
 *	Object:
 *		fpa_broken			EXPORTED function
 *
 *	Panic when FPA not working, as we do not have
 *	a software emulator.
 *
 */
fpa_broken(ss_ptr, cause, status, fpa_type)
	struct mips_saved_state *ss_ptr;
{
	extern pcb_t current_pcb;

	printf("fpa_broken: fpa_type x%x pcb at x%x status %b\n",
		fpa_type, current_pcb, status, status_fmt);
	panic("Missing FPA");
}


/*
 *	Object:
 *		stray_intr			EXPORTED function
 *
 *	Handle unexpected interrupts
 *
 */
int log_stray_interrupts = 1;

stray_intr(st)
{
	if (log_stray_interrupts)
		printf("Stray interrupt!\n");
}

#if	FAST_TAS
int
recover_ras(thread_t t)
{
    register unsigned int i;
    for (i = 0; i < TASK_FAST_TAS_NRAS; i++)  {
	if (t->task->fast_tas_base[i] == 0 && t->task->fast_tas_end[i] == 0) 
		break;
	else if ((t->pcb->mss.pc > t->task->fast_tas_base[i])
		  && (t->pcb->mss.pc <= t->task->fast_tas_end[i])) {
			t->pcb->mss.pc = t->task->fast_tas_base[i];
			return 1;
	}
    }
    return 0;
}
#endif	FAST_TAS


#if	MACH_PCSAMPLE
/*
 * return saved state for interrupted user thread
 */
unsigned
interrupted_pc(thread_t t)
{
	register struct mips_saved_state *mss;

 	mss = USER_REGS(t);
 	return mss->pc;
}
#endif	/*MACH_PCSAMPLE*/


