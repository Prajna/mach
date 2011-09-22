/* 
 * Mach Operating System
 * Copyright (c) 1991,1990,1989 Carnegie Mellon University
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
 * $Log:	pcb.c,v $
 * Revision 2.17  93/05/15  19:13:30  mrt
 * 	machparam.h -> machspl.h
 * 
 * Revision 2.16  93/01/14  17:52:23  danner
 * 	Proper spl typing.
 * 	[92/12/01            af]
 * 
 * Revision 2.15  92/03/03  12:28:50  rpd
 * 	Added syscall_emulation_sync.
 * 	[92/03/03            rpd]
 * 
 * Revision 2.14  92/01/03  20:25:39  dbg
 * 	Add user_stack_low and set_user_regs for passing control to
 * 	bootstrap in user space.
 * 	[91/11/26            dbg]
 * 
 * Revision 2.13  91/11/12  11:17:06  rvb
 * 	When allocating breakpoint structure zero the bp count,
 * 	this is now needed by prepare/did_sstep.
 * 	[91/10/13  17:19:49  af]
 * 
 * Revision 2.12  91/08/24  12:23:47  af
 * 	Missing include of Spl defs.
 * 	[91/08/02  03:04:04  af]
 * 
 * Revision 2.11  91/07/31  17:57:12  dbg
 * 	Add thread_set_syscall_return.
 * 	[91/07/26            dbg]
 * 
 * Revision 2.10  91/05/18  14:36:26  rpd
 * 	Replaced stack_free_reserved and swap_privilege with stack_privilege.
 * 	[91/03/30            rpd]
 * 
 * Revision 2.9  91/05/14  17:36:48  mrt
 * 	Correcting copyright
 * 
 * Revision 2.8  91/03/16  14:56:44  rpd
 * 	Pulled mips_float_state, mips_sstep_state out of mips_machine_state.
 * 	Added pcb_module_init.
 * 	[91/02/18            rpd]
 * 	Added continuation argument to VM_PAGE_WAIT.
 * 	Added stack_alloc_max.
 * 	[91/02/05            rpd]
 * 	Moved stack_switch to mips/context.s.
 * 	Added active_stacks.
 * 	[91/01/28            rpd]
 * 	Added stack_free_reserved.
 * 	[91/01/20            rpd]
 * 
 * Revision 2.7  91/02/05  17:50:17  mrt
 * 	Added author notices
 * 	[91/02/04  11:24:22  mrt]
 * 
 * 	Changed to use new Mach copyright
 * 	[91/02/02  12:28:14  mrt]
 * 
 * Revision 2.6  91/01/08  15:51:19  rpd
 * 	Removed pcb_synch.  Added pcb_collect.
 * 	[91/01/03            rpd]
 * 
 * 	Added mips_stack_base.
 * 	[91/01/02            rpd]
 * 
 * 	Split mips_machine_state off of mips_kernel_state.
 * 	Moved stack_switch here from mips/context.s.
 * 	[90/12/30            rpd]
 * 	Eliminated thread_bootstrap_user, thread_bootstrap_kernel.
 * 	[90/12/19            rpd]
 * 	Added pcb_alloc/pcb_free.  Use vm_page_grab/vm_page_free
 * 	for kernel stacks.  Reorganized the pcb layout.
 * 	Removed thread_private.
 * 	[90/11/11            rpd]
 * 
 * Revision 2.3  90/01/22  23:07:50  af
 * 	Added single-stepping interface, via setstatus of the exception
 * 	flavor.  Sounded like an "exception" state is the closest thing
 * 	to a vax with a T bit set.
 * 	[89/12/13            af]
 * 
 * Revision 2.2  89/11/29  14:15:00  af
 * 	Added exception flavor, read-only cuz setting it is useless.
 * 	Fixed get_status to return zeroes and not affect the thread
 * 	for floats flavor when the thread did not use the FPA.
 * 	[89/11/27            af]
 * 
 * 	User state is only identified by KUo=1.
 * 	[89/11/03  16:30:51  af]
 * 
 * 	Moved over to pure kernel, removed old history.
 * 	[89/10/29            af]
 */
/*
 *	File: pcb.c
 * 	Author: Alessandro Forin, Carnegie Mellon University
 *	Date:	10/89
 *
 *	Mips PCB management
 */

#include <mach_debug.h>

#include <machine/machspl.h>		/* spl definitions */
#include <mach/thread_status.h>
#include <mach/vm_param.h>
#include <kern/mach_param.h>
#include <kern/thread.h>
#include <kern/zalloc.h>
#include <kern/syscall_emulation.h>
#include <vm/vm_page.h>
#include <vm/pmap.h>
#include <mips/mips_cpu.h>
#include <mips/thread.h>

/*
 *	stack_attach:
 *
 *	Attach a kernel stack to a thread.
 */

void stack_attach(thread, stack, continuation)
	register thread_t thread;
	register vm_offset_t stack;
	register void (*continuation)();
{
	thread->kernel_stack = stack;

	/*
	 *	Setup the saved kernel state to run the continuation.
	 *	Point the exception link back at the exception frame.
	 */
	STACK_MKS(stack)->pc = (int) continuation;
	STACK_MKS(stack)->sp = (int) STACK_MEL(stack);
	STACK_MEL(stack)->eframe = &thread->pcb->mss;
}

/*
 *	The stack_free_list can only be accessed at splsched,
 *	because stack_alloc_try/thread_swapin operate at splsched.
 */
vm_offset_t stack_free_list;		/* splsched only */
unsigned int stack_free_count = 0;	/* splsched only */
unsigned int stack_free_limit = 1;	/* patchable */

unsigned int stack_alloc_hits = 0;	/* debugging */
unsigned int stack_alloc_misses = 0;	/* debugging */
unsigned int stack_alloc_max = 0;	/* debugging */

/*
 *	The next field is at the base of the stack,
 *	so the low end is left unsullied.  The page
 *	field must be preserved.
 */

#define stack_next(stack) STACK_MSB(stack)->next

/*
 *	stack_alloc_try:
 *
 *	Non-blocking attempt to allocate a kernel stack.
 *	Called at splsched with the thread locked.
 */

boolean_t stack_alloc_try(thread, continuation)
	register thread_t thread;
	void (*continuation)();
{
	register vm_offset_t stack;

	stack = stack_free_list;
	if (stack != 0) {
		stack_free_list = stack_next(stack);
		stack_free_count--;
	} else {
		stack = thread->stack_privilege;
		if (stack == 0) {
			stack_alloc_misses++;
			return FALSE;
		}
	}

	stack_attach(thread, stack, continuation);
	stack_alloc_hits++;
	return TRUE;
}

/*
 *	stack_alloc:
 *
 *	Allocate a kernel stack for a thread.
 *	May block.
 */

void stack_alloc(thread, continuation)
	register thread_t thread;
	void (*continuation)();
{
	register vm_offset_t stack;
	spl_t s;

	/*
	 *	We first try the free list.  It is probably empty,
	 *	or stack_alloc_try would have succeeded, but possibly
	 *	a stack was freed before the swapin thread got to us.
	 */

	s = splsched();
	stack = stack_free_list;
	if (stack != 0) {
		stack_free_list = stack_next(stack);
		stack_free_count--;
	}
	(void) splx(s);

	if (stack == 0) {
		register vm_page_t m;

		while ((m = vm_page_grab()) == VM_PAGE_NULL)
			VM_PAGE_WAIT((void (*)()) 0);

		stack = PHYS_TO_K0SEG(m->phys_addr);
#if	MACH_DEBUG
		stack_init(stack);
#endif	MACH_DEBUG
		STACK_MSB(stack)->page = m;
	}

	stack_attach(thread, stack, continuation);
}

/*
 *	stack_free:
 *
 *	Free a thread's kernel stack.
 *	Called at splsched with the thread locked.
 */

void stack_free(thread)
	register thread_t thread;
{
	register vm_offset_t stack;

	stack = thread->kernel_stack;

	if (stack != thread->stack_privilege) {
		stack_next(stack) = stack_free_list;
		stack_free_list = stack;
		if (++stack_free_count > stack_alloc_max)
			stack_alloc_max = stack_free_count;
	}
}

/*
 *	stack_collect:
 *
 *	Free excess kernel stacks.
 *	May block.
 */

void stack_collect()
{
	extern vm_page_t vm_page_array;
	extern int first_page;

	register vm_offset_t stack;
	spl_t s;

	s = splsched();
	while (stack_free_count > stack_free_limit) {
		stack = stack_free_list;
		stack_free_list = stack_next(stack);
		stack_free_count--;
		(void) splx(s);

#if		MACH_DEBUG
		stack_finalize(stack);
#endif		MACH_DEBUG
		vm_page_release(STACK_MSB(stack)->page);

		s = splsched();
	}
	(void) splx(s);
}

#if	MACH_DEBUG
extern boolean_t stack_check_usage;

/*
 *	stack_statistics:
 *
 *	Return statistics on cached kernel stacks
 *	kept by this machine-dependent module.
 *	*maxusagep must be initialized by the caller.
 */

void stack_statistics(totalp, maxusagep)
	unsigned int *totalp;
	vm_size_t *maxusagep;
{
	spl_t s;

	s = splsched();
	if (stack_check_usage) {
		register vm_offset_t stack;

		/*
		 *	This is pretty expensive to do at splsched,
		 *	but it only happens when someone makes
		 *	a debugging call, so it should be OK.
		 */

		for (stack = stack_free_list;
		     stack != 0;
		     stack = stack_next(stack)) {
			vm_size_t usage = stack_usage(stack);

			if (usage > *maxusagep)
				*maxusagep = usage;
		}
	}

	*totalp = stack_free_count;
	(void) splx(s);
}
#endif	MACH_DEBUG

extern thread_t Switch_context();

thread_t switch_context(old, continuation, new)
	register thread_t	old;
	void (*continuation)();
	register thread_t	new;
{
    {
	task_t old_task, new_task;

	if ((old_task = old->task) == (new_task = new->task)) {
		PMAP_CONTEXT(vm_map_pmap(new_task->map), new);
	} else {
		PMAP_DEACTIVATE_USER(vm_map_pmap(old_task->map),
				     old, cpu_number());
		PMAP_ACTIVATE_USER(vm_map_pmap(new_task->map),
				   new, cpu_number());
	}
    }

	return Switch_context(old, continuation, new);
}

pcb_t pcb_free_list;		/* list of unused pcb structures */
unsigned int pcb_free_count;	/* size of the list, for debugging */
unsigned int pcb_wasted_count;	/* number of unusable pcbs allocated */
zone_t pcb_zone;		/* used when free list is empty */

zone_t mfs_zone;
zone_t msss_zone;

void pcb_module_init()
{
	pcb_zone = zinit(sizeof(struct pcb),
			 THREAD_MAX * sizeof(struct pcb),
			 THREAD_CHUNK * sizeof(struct pcb),
			 FALSE, "mips pcb state");

	mfs_zone = zinit(sizeof(struct mips_float_state),
			 THREAD_MAX * sizeof(struct mips_float_state),
			 THREAD_CHUNK * sizeof(struct mips_float_state),
			 FALSE, "mips float state");

	msss_zone = zinit(sizeof(struct mips_sstep_state),
			  THREAD_MAX * sizeof(struct mips_sstep_state),
			  THREAD_CHUNK * sizeof(struct mips_sstep_state),
			  FALSE, "mips sstep state");
}

pcb_t pcb_alloc()
{
	pcb_t pcb;

	if (pcb = pcb_free_list) {
		pcb_free_list = * (pcb_t *) pcb;
		pcb_free_count--;
		return pcb;
	}

	for (;;) {
		vm_offset_t va, pa;

		/*
		 *	We use the zone package to get more pcbs.
		 *	This is tricky, because we need k0seg memory.
		 */

		va = zalloc(pcb_zone);
		if (ISA_K0SEG(va))
			return (pcb_t) va;

		/*
		 *	We can convert the virtual address to a
		 *	physical address, if the pcb lies entirely in
		 *	one physical page.
		 */

		if (mips_trunc_page(va) + MIPS_PGBYTES ==
		    mips_round_page((vm_offset_t) ((pcb_t) va + 1))) {
			pa = pmap_resident_extract(kernel_pmap, va);
			if (pa == 0)
				panic("pcb_alloc", va);

			return (pcb_t) PHYS_TO_K0SEG(pa);
		}

		/*
		 *	Discard this pcb and try again.
		 */

		pcb_wasted_count++;
	}
}

void pcb_free(pcb)
	pcb_t pcb;
{
	pcb_free_count++;
	* (pcb_t *) pcb = pcb_free_list;
	pcb_free_list = pcb;
}

/*
 *	pcb_init:
 *
 *	Initialize the pcb for a thread.  For Mips,
 *	also initializes the coprocessor(s).
 *
 */
void pcb_init(thread)
	register thread_t	thread;
{
	register pcb_t pcb;

	/*
	 *	Allocate a pcb.
	 */
	pcb = pcb_alloc();
	thread->pcb = pcb;

	/*
	 *	We can't let the user see random values
	 *	in his registers.  They might not be so random.
	 */
	bzero(pcb, sizeof *pcb);

	/*
	 *	Make the thread run in user mode,
	 *	if it ever comes back out of the kernel.
	 */
	pcb->mss.sr = INT_LEV0|SR_KUo|SR_IEo;	/* user mode */

	/*
	 * Space for floating point and single-stepping state
	 * will be allocated as needed.
	 */
}

void pcb_fpa_init(thread)
	register thread_t thread;
{
	register pcb_t pcb = thread->pcb;

	pcb->mms.mfs = (struct mips_float_state *) zalloc(mfs_zone);

	/*
	 *	We can't let the user see random values
	 *	in his registers.  They might not be so random.
	 */
	bzero(pcb->mms.mfs, sizeof *pcb->mms.mfs);
}

/*
 *  	pcb_terminate:
 *
 *	Shutdown any state associated with a thread's pcb.
 *	Also, release any coprocessor(s) register state.
 */
void pcb_terminate(thread)
	register struct thread *thread;
{
	register pcb_t pcb = thread->pcb;

	check_fpa(thread, 1);
	if (pcb->mms.mfs != 0)
		zfree(mfs_zone, (vm_offset_t) pcb->mms.mfs);
	if (pcb->mms.msss != 0)
		zfree(msss_zone, (vm_offset_t) pcb->mms.msss);
	pcb_free(pcb);
	thread->pcb = 0;
}

/*
 *	pcb_collect:
 *
 *	Attempt to free excess pcb memory.
 */

void pcb_collect(thread)
	thread_t thread;
{
}

/*
 *	syscall_emulation_sync:
 *
 *	The task's emulation vector just changed.
 *	Perform any necessary synchronization.
 */

extern struct eml_dispatch *current_dispatch;

void syscall_emulation_sync(task)
	task_t task;
{
	if (task == current_task())
		current_dispatch = task->eml_dispatch;
}

/*
 *	thread_setstatus:
 *
 *	Set the status of the given thread.
 */

kern_return_t thread_setstatus(thread, flavor, tstate, count)
	thread_t		thread;
	int			flavor;
	thread_state_t		tstate;
	unsigned int		count;
{
	switch (flavor) {
	case MIPS_THREAD_STATE: {
		register struct mips_saved_state *mss;
		register struct mips_thread_state *mts;

		if (count != MIPS_THREAD_STATE_COUNT)
			return(KERN_INVALID_ARGUMENT);

		mss = USER_REGS(thread);
		mts = (struct mips_thread_state *) tstate;

		mss->at = mts->r1;
		mss->v0 = mts->r2;
		mss->v1 = mts->r3;
		mss->a0 = mts->r4;
		mss->a1 = mts->r5;
		mss->a2 = mts->r6;
		mss->a3 = mts->r7;
		mss->t0 = mts->r8;
		mss->t1 = mts->r9;
		mss->t2 = mts->r10;
		mss->t3 = mts->r11;
		mss->t4 = mts->r12;
		mss->t5 = mts->r13;
		mss->t6 = mts->r14;
		mss->t7 = mts->r15;
		mss->s0 = mts->r16;
		mss->s1 = mts->r17;
		mss->s2 = mts->r18;
		mss->s3 = mts->r19;
		mss->s4 = mts->r20;
		mss->s5 = mts->r21;
		mss->s6 = mts->r22;
		mss->s7 = mts->r23;
		mss->t8 = mts->r24;
		mss->t9 = mts->r25;
		mss->k0 = mts->r26;
		mss->k1 = mts->r27;
		mss->gp = mts->r28;
		mss->sp = mts->r29;
		mss->fp = mts->r30;
		mss->ra = mts->r31;
		mss->mdlo = mts->mdlo;
		mss->mdhi = mts->mdhi;
		mss->pc = mts->pc;
		break;
	}

	case MIPS_FLOAT_STATE: {
		register pcb_t pcb;
		register struct mips_float_state *mfs;

		if (count != MIPS_FLOAT_STATE_COUNT)
			return(KERN_INVALID_ARGUMENT);

		pcb = thread->pcb;
		mfs = (struct mips_float_state *) tstate;

		if (pcb->mms.mfs == 0)
			pcb->mms.mfs = (struct mips_float_state *)
				zalloc(mfs_zone);

		bcopy(mfs, pcb->mms.mfs, sizeof *mfs);

		disable_fpa(thread);	/* ensure we'll reload fpu */
		break;
	}

	case MIPS_EXC_STATE: {
		register pcb_t pcb;
		register struct mips_exc_state *mes;

		if (count != MIPS_EXC_STATE_COUNT)
			return(KERN_INVALID_ARGUMENT);

		mes = (struct mips_exc_state *) tstate;

		if (mes->cause != EXC_SST)
			return(KERN_INVALID_ARGUMENT);

		pcb = thread->pcb;
		if (pcb->mms.msss == 0) {
			pcb->mms.msss = (struct mips_sstep_state *)
				zalloc(msss_zone);
			pcb->mms.msss->ss_count = 0;
		}
		break;
	}

	default:
		return(KERN_INVALID_ARGUMENT);
	}

	return(KERN_SUCCESS);

}

/*
 *	thread_getstatus:
 *
 *	Get the status of the specified thread.
 */

kern_return_t thread_getstatus(thread, flavor, tstate, count)
	register thread_t	thread;
	int			flavor;
	thread_state_t		tstate;		/* pointer to OUT array */
	unsigned int		*count;		/* IN/OUT */
{
	switch (flavor) {
	case THREAD_STATE_FLAVOR_LIST:
		if (*count < 3)
			return(KERN_INVALID_ARGUMENT);

		tstate[0] = MIPS_THREAD_STATE;
		tstate[1] = MIPS_FLOAT_STATE;
		tstate[2] = MIPS_EXC_STATE;

		*count = 3;
		break;

	case MIPS_THREAD_STATE: {
		register struct mips_saved_state *mss;
		register struct mips_thread_state *mts;

		if (*count < MIPS_THREAD_STATE_COUNT)
			return(KERN_INVALID_ARGUMENT);

		mss = USER_REGS(thread);
		mts = (struct mips_thread_state *) tstate;

		mts->r1 = mss->at;
		mts->r2 = mss->v0;
		mts->r3 = mss->v1;
		mts->r4 = mss->a0;
		mts->r5 = mss->a1;
		mts->r6 = mss->a2;
		mts->r7 = mss->a3;
		mts->r8 = mss->t0;
		mts->r9 = mss->t1;
		mts->r10 = mss->t2;
		mts->r11 = mss->t3;
		mts->r12 = mss->t4;
		mts->r13 = mss->t5;
		mts->r14 = mss->t6;
		mts->r15 = mss->t7;
		mts->r16 = mss->s0;
		mts->r17 = mss->s1;
		mts->r18 = mss->s2;
		mts->r19 = mss->s3;
		mts->r20 = mss->s4;
		mts->r21 = mss->s5;
		mts->r22 = mss->s6;
		mts->r23 = mss->s7;
		mts->r24 = mss->t8;
		mts->r25 = mss->t9;
		mts->r26 = mss->k0;
		mts->r27 = mss->k1;
		mts->r28 = mss->gp;
		mts->r29 = mss->sp;
		mts->r30 = mss->fp;
		mts->r31 = mss->ra;
		mts->mdlo = mss->mdlo;
		mts->mdhi = mss->mdhi;
		mts->pc  = mss->pc;

		*count = MIPS_THREAD_STATE_COUNT;
		break;
	}

	case MIPS_FLOAT_STATE: {
		register pcb_t pcb;
		register struct mips_float_state *mfs;

		if (*count < MIPS_FLOAT_STATE_COUNT)
			return(KERN_INVALID_ARGUMENT);

		pcb = thread->pcb;
		mfs = (struct mips_float_state *) tstate;

		if (pcb->mms.mfs) {
			check_fpa(thread, 0);
			bcopy(pcb->mms.mfs, mfs, sizeof *mfs);
		} else
			bzero(mfs, sizeof *mfs);

		*count = MIPS_FLOAT_STATE_COUNT;
		break;
	}

	case MIPS_EXC_STATE: {
		register struct mips_saved_state *mss;
		register struct mips_exc_state *mes;

		if (*count < MIPS_EXC_STATE_COUNT)
			return(KERN_INVALID_ARGUMENT);

		mss = USER_REGS(thread);
		mes = (struct mips_exc_state *) tstate;

		mes->cause = mss->cause;
		mes->address = mss->bad_address;
		mes->coproc_state = MIPS_STATUS_USE_COP0;
		if (thread->pcb->mms.mfs)
			mes->coproc_state |= MIPS_STATUS_USE_COP1;

		*count = MIPS_EXC_STATE_COUNT;
		break;
	}

	default:
		return(KERN_INVALID_ARGUMENT);
	}
	return(KERN_SUCCESS);
}

/*
 * Alter the thread`s state so that a following thread_exception_return
 * will make the thread return 'retval' from a syscall.
 */
void
thread_set_syscall_return(thread, retval)
	thread_t	thread;
	kern_return_t	retval;
{
	USER_REGS(thread)->v0 = retval;
}


/*
 * Return prefered address of user stack.
 * Always returns low address.  If stack grows up,
 * the stack grows away from this address;
 * if stack grows down, the stack grows towards this
 * address.
 */
vm_offset_t
user_stack_low(stack_size)
	vm_size_t	stack_size;
{
	return (VM_MAX_ADDRESS - stack_size);
}

/*
 * Allocate argument area and set registers for first user thread.
 */
vm_offset_t
set_user_regs(stack_base, stack_size, entry, arg_size)
	vm_offset_t	stack_base;	/* low address */
	vm_offset_t	stack_size;
	int		*entry;
	vm_size_t	arg_size;
{
	vm_offset_t	arg_addr;
	register struct mips_saved_state *saved_state;

	arg_size = (arg_size + sizeof(int) - 1) & ~(sizeof(int)-1);
	arg_addr = stack_base + stack_size - arg_size;

	saved_state = USER_REGS(current_thread());
	saved_state->pc = entry[0];
	saved_state->gp = entry[1];
	saved_state->sp = arg_addr;

	return (arg_addr);
}
