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
 * HISTORY
 * $Log:	pcb.c,v $
 * Revision 2.4  93/05/15  19:11:18  mrt
 * 	machparam.h -> machspl.h
 * 
 * Revision 2.3  93/01/19  08:59:32  danner
 * 	Horrible mess to cope with cache-coherency bugs on ADU MP.
 * 	There is also some other bug at large that prevents proper
 * 	cleanup of the switch functions. Sigh.
 * 	[93/01/15            af]
 * 
 * Revision 2.2  93/01/14  17:13:36  danner
 * 	Added reference to documentation source(s).
 * 	[92/12/16  15:15:36  af]
 * 
 * 	Created.
 * 	[92/12/10  14:59:58  af]
 * 
 */
/*
 *	File: pcb.c
 * 	Author: Alessandro Forin, Carnegie Mellon University
 *	Date:	6/92
 *
 *	Alpha PCB management
 *
 *	This code was derived exclusively from information available in
 *	"Alpha Architecture Reference Manual", Richard L. Sites ed.
 *	Digital Press, Burlington, MA 01803
 *	ISBN 1-55558-098-X, Order no. EY-L520E-DP
 */

#include <cpus.h>
#include <mach_kdb.h>
#include <mach_debug.h>

#include <mach/std_types.h>
#include <machine/machspl.h>		/* spl definitions */
#include <mach/thread_status.h>
#include <mach/vm_param.h>
#include <kern/mach_param.h>
#include <kern/thread.h>
#include <kern/zalloc.h>
#include <kern/syscall_emulation.h>
#include <vm/vm_page.h>
#include <vm/pmap.h>
#include <alpha/alpha_cpu.h>
#include <alpha/thread.h>

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
	register pcb_t		pcb = thread->pcb;

extern boolean_t debug_verbose;
if (debug_verbose) db_printf("attach stack %x to thread %x (%x)\n",
				stack, thread, continuation);
	thread->kernel_stack = stack;

	/*
	 *	Setup the saved kernel state to run the continuation.
	 *	Point the exception link back at the exception frame.
	 */
	pcb->mss.hw_pcb.ksp = (vm_offset_t)&STACK_MEL(stack)->tf;

	STACK_MSB(stack)->pcb = pcb;
	STACK_MKS(stack)->pc = (vm_offset_t) continuation;
	STACK_MKS(stack)->sp = (vm_offset_t) STACK_MKS(stack);
	STACK_MEL(stack)->eframe = &pcb->mss;
	pcb->mss.saved_frame.saved_ps = alpha_initial_ps_value;
	STACK_MEL(stack)->tf = pcb->mss.saved_frame;
}

/*
 *	The stack_free_list can only be accessed at splsched,
 *	because stack_alloc_try/thread_swapin operate at splsched.
 */
decl_simple_lock_data(,stack_free_list_lock)
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

	simple_lock(&stack_free_list_lock);
	stack = stack_free_list;
	if (stack != 0) {
		stack_free_list = stack_next(stack);
		stack_free_count--;
	} else {
		stack = thread->stack_privilege;
		if (stack == 0) {
			stack_alloc_misses++;
			simple_unlock(&stack_free_list_lock);
			return FALSE;
		}
	}
	stack_alloc_hits++;
	simple_unlock(&stack_free_list_lock);

	stack_attach(thread, stack, continuation);
	return TRUE;
}

/*
 *	stack_alloc:
 *
 *	Allocate a kernel stack for a thread.
 *	May block.
 */

#define	ALLOCATE_STACK_WITH_GARD_PAGES	0
#include <vm/vm_kern.h>

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
	simple_lock(&stack_free_list_lock);
	stack = stack_free_list;
	if (stack != 0) {
		stack_free_list = stack_next(stack);
		stack_free_count--;
	}
	simple_unlock(&stack_free_list_lock);
	(void) splx(s);

	if (stack == 0) {
#if	ALLOCATE_STACK_WITH_GARD_PAGES
		vm_offset_t addr = 0;
		kmem_alloc_aligned( kernel_map, &addr, 4 * PAGE_SIZE);
		if (vm_protect( kernel_map, addr, PAGE_SIZE, 1, VM_PROT_READ))
			gimmeabreak();
		stack = addr + PAGE_SIZE;
		if (vm_protect( kernel_map, stack + PAGE_SIZE, 2*PAGE_SIZE, 1, VM_PROT_READ))
			gimmeabreak();
#else
		register vm_page_t m;

		while ((m = vm_page_grab()) == VM_PAGE_NULL)
			VM_PAGE_WAIT((void (*)()) 0);

		stack = PHYS_TO_K0SEG(m->phys_addr);
#if	MACH_DEBUG
		stack_init(stack);
#endif	MACH_DEBUG
		STACK_MSB(stack)->page = m;
#endif	/* ALLOCATE_STACK_WITH_GARD_PAGES */
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
		simple_lock(&stack_free_list_lock);
		stack_next(stack) = stack_free_list;
		stack_free_list = stack;
		if (++stack_free_count > stack_alloc_max)
			stack_alloc_max = stack_free_count;
		simple_unlock(&stack_free_list_lock);
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

#if	ALLOCATE_STACK_WITH_GARD_PAGES
#else
	s = splsched();
	simple_lock(&stack_free_list_lock);
	while (stack_free_count > stack_free_limit) {
		stack = stack_free_list;
		stack_free_list = stack_next(stack);
		stack_free_count--;
		simple_unlock(&stack_free_list_lock);
		(void) splx(s);

#if		MACH_DEBUG
		stack_finalize(stack);
#endif		MACH_DEBUG
		vm_page_release(STACK_MSB(stack)->page);

		s = splsched();
		simple_lock(&stack_free_list_lock);
	}
	simple_unlock(&stack_free_list_lock);
	(void) splx(s);
#endif	/* ALLOCATE_STACK_WITH_GARD_PAGES */
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

		simple_lock(&stack_free_list_lock);
		for (stack = stack_free_list;
		     stack != 0;
		     stack = stack_next(stack)) {
			vm_size_t usage = stack_usage(stack);

			if (usage > *maxusagep)
				*maxusagep = usage;
		}
		simple_unlock(&stack_free_list_lock);
	}

	*totalp = stack_free_count;
	(void) splx(s);
}
#endif	MACH_DEBUG

/* Cannot optimize this because multiP */
static
unload_fpa(pcb)
	pcb_t	pcb;	
{
	register struct alpha_float_state	*mfs;

	mfs = pcb->mms.mfs;
	/* Do we have state and did we use it this time around */
	if ((vm_offset_t)mfs & 1) {
		mfs = (struct alpha_float_state *)((vm_offset_t)mfs & ~1);
		pcb->mms.mfs = mfs;
		alpha_fpa_unload(mfs);	/* leaves fpa disabled */
	}
}

long show_handoff, show_ustack;

extern thread_t Switch_context();

thread_t switch_context(old, continuation, new)
	register thread_t	old;
	void (*continuation)();
	register thread_t	new;
{
	task_t old_task, new_task;
int mycpu = cpu_number();

	unload_fpa(old->pcb);

tbia(); alphacache_Iflush();
if (show_handoff) db_printf("[%d]switch(%x -> %x) %x\n", mycpu,
			old, new, continuation);
if (show_ustack) db_printf("[%d]USP %x (%x) -> %x \n", mycpu,
			old->pcb->mss.hw_pcb.usp, mfpr_usp(),
			new->pcb->mss.hw_pcb.usp);

	if ((old_task = old->task) == (new_task = new->task)) {
		register pcb_t	pcb = new->pcb;
if (new->pcb->mss.hw_pcb.ptbr != old->pcb->mss.hw_pcb.ptbr) gimmeabreak();
if (vm_map_pmap(new_task->map)->pid < 0) gimmeabreak();
		pcb->mss.hw_pcb.asn = vm_map_pmap(new_task->map)->pid;
		swpctxt(kvtophys((vm_offset_t) pcb), &(pcb)->mss.hw_pcb.ksp);
	} else {
		PMAP_DEACTIVATE_USER(vm_map_pmap(old_task->map),
				     old, cpu_number());
		PMAP_ACTIVATE_USER(vm_map_pmap(new_task->map),
				   new, cpu_number());
	}

	return Switch_context(old, continuation, new);
}

#if 1
void
stack_handoff(old,new)
	thread_t	old,new;
{
	task_t old_task, new_task;
int mycpu = cpu_number();

	unload_fpa(old->pcb);
tbia(); alphacache_Iflush();
if (show_handoff) db_printf("[%d]handoff(%x -> %x)\n", mycpu, old, new);
if (show_ustack) db_printf("[%d]USP %x (%x) -> %x \n", mycpu,
			old->pcb->mss.hw_pcb.usp, mfpr_usp(),
			new->pcb->mss.hw_pcb.usp);

	old_task = old->task;
	new_task = new->task;
	if (old_task != new_task) {
#if	NCPUS>1
		register int my_cpu = cpu_number();

		PMAP_DEACTIVATE_USER(vm_map_pmap(old_task->map),
				     old, my_cpu);
		PMAP_ACTIVATE_USER(vm_map_pmap(new_task->map),
				   new, my_cpu);
#else
		vm_map_pmap(old_task->map)->cpus_using = FALSE;
		vm_map_pmap(new_task->map)->cpus_using = TRUE;
#endif
	}

#if 1
	new->kernel_stack = current_stack();
	old->kernel_stack = 0;
#endif

	Stack_handoff(old,new,old_task,new_task);
}
#endif

decl_simple_lock_data(,pcb_free_list_lock)
pcb_t pcb_free_list;		/* list of unused pcb structures */
unsigned int pcb_free_count;	/* size of the list, for debugging */
unsigned int pcb_wasted_count;	/* number of unusable pcbs allocated */
zone_t pcb_zone;		/* used when free list is empty */

zone_t mfs_zone;
zone_t msss_zone;

void pcb_module_init()
{
	simple_lock_init(&stack_free_list_lock);
	simple_lock_init(&pcb_free_list_lock);

	pcb_zone = zinit(sizeof(struct pcb),
			 THREAD_MAX * sizeof(struct pcb),
			 THREAD_CHUNK * sizeof(struct pcb),
			 FALSE, "alpha pcb state");
	zcollectable(pcb_zone);	/* sigh, alignment */

	mfs_zone = zinit(sizeof(struct alpha_float_state),
			 THREAD_MAX * sizeof(struct alpha_float_state),
			 THREAD_CHUNK * sizeof(struct alpha_float_state),
			 FALSE, "alpha float state");

	msss_zone = zinit(sizeof(struct alpha_sstep_state),
			  THREAD_MAX * sizeof(struct alpha_sstep_state),
			  THREAD_CHUNK * sizeof(struct alpha_sstep_state),
			  FALSE, "alpha sstep state");
}

pcb_t pcb_alloc()
{
	pcb_t pcb;

	simple_lock(&pcb_free_list_lock);
	if (pcb = pcb_free_list) {
		pcb_free_list = * (pcb_t *) pcb;
		pcb_free_count--;
	}
	simple_unlock(&pcb_free_list_lock);

	if (pcb)
		return pcb;

	for (;;) {
		vm_offset_t va, pa;

		/*
		 *	We use the zone package to get more pcbs.
		 *	This is tricky, because we need k0seg memory.
		 */

		va = zalloc(pcb_zone);
		/* must be aligned or else */
		if (va & (sizeof(struct pcb)-1))
			panic("pcb_alloc");
		if (ISA_K0SEG(va))
			return (pcb_t) va;

		/*
		 *	We can convert the virtual address to a
		 *	physical address, if the pcb lies entirely in
		 *	one physical page.  It should.
		 */

		if (alpha_trunc_page(va) + ALPHA_PGBYTES ==
		    alpha_round_page((vm_offset_t) ((pcb_t) va + 1))) {
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
	simple_lock(&pcb_free_list_lock);
	pcb_free_count++;
	* (pcb_t *) pcb = pcb_free_list;
	pcb_free_list = pcb;
	simple_unlock(&pcb_free_list_lock);
}

/*
 *	pcb_init:
 *
 *	Initialize the pcb for a thread.  For Alpha,
 *	also (lazily) initialize the FPA state.
 *
 */
void pcb_init(thread)
	register thread_t	thread;
{
	register pcb_t	pcb;

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

	pcb->mss.framep = &pcb->mss.saved_frame;
	set_ptbr(thread->task->map->pmap, pcb, FALSE);

	/*
	 *	Make the thread run in user mode,
	 *	if it ever comes back out of the kernel.
	 *	This is done in thread_bootstrap_return().
	 */

	/*
	 * Space for floating point and single-stepping state
	 * will be allocated as needed.
	 */
}

void pcb_fpa_init(thread)
	register thread_t thread;
{
	register pcb_t pcb = thread->pcb;

	pcb->mms.mfs = (struct alpha_float_state *) zalloc(mfs_zone);

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

	if (pcb->mms.mfs != 0)
		/* fpa disabled at ctxt switch time */
		zfree(mfs_zone, (vm_offset_t)pcb->mms.mfs & ~1);

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

extern struct eml_dispatch *current_dispatch[];

void syscall_emulation_sync(task)
	task_t task;
{
	if (task == current_task())
		current_dispatch[cpu_number()] = task->eml_dispatch;
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
	natural_t		count;
{
	switch (flavor) {
	case ALPHA_THREAD_STATE: {
		register struct alpha_saved_state *mss;
		register struct alpha_thread_state *mts;

		if (count != ALPHA_THREAD_STATE_COUNT)
			return(KERN_INVALID_ARGUMENT);

		mss = & USER_REGS(thread)->mss;
		mts = (struct alpha_thread_state *) tstate;

		mss->v0 = mts->r0;
		mss->t0 = mts->r1;
		mss->saved_frame.saved_r2 = mts->r2;
		mss->saved_frame.saved_r3 = mts->r3;
		mss->saved_frame.saved_r4 = mts->r4;
		mss->saved_frame.saved_r5 = mts->r5;
		mss->saved_frame.saved_r6 = mts->r6;
		mss->saved_frame.saved_r7 = mts->r7;
		mss->t7 = mts->r8;
		mss->s0 = mts->r9;
		mss->s1 = mts->r10;
		mss->s2 = mts->r11;
		mss->s3 = mts->r12;
		mss->s4 = mts->r13;
		mss->s5 = mts->r14;
		mss->s6 = mts->r15;
		mss->a0 = mts->r16;
		mss->a1 = mts->r17;
		mss->a2 = mts->r18;
		mss->a3 = mts->r19;
		mss->a4 = mts->r20;
		mss->a5 = mts->r21;
		mss->t8 = mts->r22;
		mss->t9 = mts->r23;
		mss->t10 = mts->r24;
		mss->t11 = mts->r25;
		mss->ra = mts->r26;
		mss->t12 = mts->r27;
		mss->at = mts->r28;
		mss->gp = mts->r29;
		mss->sp = mts->r30;
		mss->hw_pcb.usp = mts->r30;
/* XXXX if thread == current_thread mtpr_usp */
		mss->saved_frame.saved_pc = mts->pc;
		break;
	}

	case ALPHA_FLOAT_STATE: {
		register pcb_t pcb;
		register struct alpha_float_state *mfs;

		if (count != ALPHA_FLOAT_STATE_COUNT)
			return(KERN_INVALID_ARGUMENT);

		pcb = thread->pcb;
		mfs = (struct alpha_float_state *) tstate;

		if (pcb->mms.mfs == 0)
			pcb->mms.mfs = (struct alpha_float_state *)
				zalloc(mfs_zone);

		bcopy(mfs, pcb->mms.mfs, sizeof *mfs);

		break;
	}

	case ALPHA_EXC_STATE: {
		register pcb_t pcb;
		register struct alpha_exc_state *mes;

		if (count != ALPHA_EXC_STATE_COUNT)
			return(KERN_INVALID_ARGUMENT);

		mes = (struct alpha_exc_state *) tstate;

		if (mes->cause != ALPHA_EXC_SET_SSTEP)
			return(KERN_INVALID_ARGUMENT);

		pcb = thread->pcb;
		if (pcb->mms.msss == 0) {
			pcb->mms.msss = (struct alpha_sstep_state *)
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
	natural_t		*count;		/* IN/OUT */
{
	switch (flavor) {
	case THREAD_STATE_FLAVOR_LIST:
		if (*count < 3)
			return(KERN_INVALID_ARGUMENT);

		tstate[0] = ALPHA_THREAD_STATE;
		tstate[1] = ALPHA_FLOAT_STATE;
		tstate[2] = ALPHA_EXC_STATE;

		*count = 3;
		break;

	case ALPHA_THREAD_STATE: {
		register struct alpha_saved_state *mss;
		register struct alpha_thread_state *mts;

		if (*count < ALPHA_THREAD_STATE_COUNT)
			return(KERN_INVALID_ARGUMENT);

		mss = & USER_REGS(thread)->mss;
		mts = (struct alpha_thread_state *) tstate;

		mts->r0 = mss->v0;
		mts->r1 = mss->t0;
		mts->r2 = mss->saved_frame.saved_r2;
		mts->r3 = mss->saved_frame.saved_r3;
		mts->r4 = mss->saved_frame.saved_r4;
		mts->r5 = mss->saved_frame.saved_r5;
		mts->r6 = mss->saved_frame.saved_r6;
		mts->r7 = mss->saved_frame.saved_r7;
		mts->r8 = mss->t7;
		mts->r9 = mss->s0;
		mts->r10 = mss->s1;
		mts->r11 = mss->s2;
		mts->r12 = mss->s3;
		mts->r13 = mss->s4;
		mts->r14 = mss->s5;
		mts->r15 = mss->s6;
		mts->r16 = mss->a0;
		mts->r17 = mss->a1;
		mts->r18 = mss->a2;
		mts->r19 = mss->a3;
		mts->r20 = mss->a4;
		mts->r21 = mss->a5;
		mts->r22 = mss->t8;
		mts->r23 = mss->t9;
		mts->r24 = mss->t10;
		mts->r25 = mss->t11;
		mts->r26 = mss->ra;
		mts->r27 = mss->t12;
		mts->r28 = mss->at;
		mts->r29 = mss->gp;
		mts->r30 = mss->hw_pcb.usp;
		mts->pc = mss->saved_frame.saved_pc;

		*count = ALPHA_THREAD_STATE_COUNT;
		break;
	}

	case ALPHA_FLOAT_STATE: {
		register pcb_t pcb;
		register struct alpha_float_state *mfs;

		if (*count < ALPHA_FLOAT_STATE_COUNT)
			return(KERN_INVALID_ARGUMENT);

		pcb = thread->pcb;
		mfs = (struct alpha_float_state *) tstate;

		if (pcb->mms.mfs) {
			/* fpa state dumped at ctxt switch time */
			bcopy(pcb->mms.mfs, mfs, sizeof *mfs);
		} else
			bzero(mfs, sizeof *mfs);

		*count = ALPHA_FLOAT_STATE_COUNT;
		break;
	}

	case ALPHA_EXC_STATE: {
		register struct alpha_saved_state *mss;
		register struct alpha_exc_state *mes;

		if (*count < ALPHA_EXC_STATE_COUNT)
			return(KERN_INVALID_ARGUMENT);

		mss = & USER_REGS(thread)->mss;
		mes = (struct alpha_exc_state *) tstate;

		mes->cause = mss->cause;
		mes->address = mss->bad_address;
		mes->used_fpa = (thread->pcb->mms.mfs != 0);

		*count = ALPHA_EXC_STATE_COUNT;
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
#if MACH_KDB
	extern boolean_t syscalltrace;
	if (syscalltrace) db_printf("-> %x\n", retval);
#endif
	USER_REGS(thread)->mss.v0 = retval;
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
	vm_offset_t	*entry;
	vm_size_t	arg_size;
{
	vm_offset_t	arg_addr;
	register struct alpha_saved_state *saved_state;

	arg_size = (arg_size + sizeof(vm_size_t) - 1) & ~(sizeof(vm_size_t)-1);
	arg_addr = stack_base + stack_size - arg_size;

	saved_state = & USER_REGS(current_thread())->mss;
	saved_state->saved_frame.saved_pc = entry[0];
	saved_state->framep->saved_pc     = entry[0];
	saved_state->gp 	= entry[1];
	saved_state->sp = arg_addr;
	saved_state->hw_pcb.usp = arg_addr;

	return (arg_addr);
}
