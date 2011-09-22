/* 
 * Mach Operating System
 * Copyright (c) 1991,1990 Carnegie Mellon University
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
 * Revision 2.14  93/01/14  17:29:21  danner
 * 	Added include of mach/std_types.h
 * 	[92/12/10  17:41:42  af]
 * 
 * Revision 2.13  92/01/03  20:08:34  dbg
 * 	Disable thread_set_state of ISA_PORT_MAP, but have it still
 * 	return KERN_SUCCESS (for DOS emulator compatibility).
 * 	[91/12/06            dbg]
 * 
 * 	Add user ldt management.  Move floating-point state
 * 	manipulation to i386/fpu.{c,h}.
 * 
 * 	Add user_stack_low and set_user_regs for passing control to
 * 	bootstrap in user space.
 * 	[91/10/30            dbg]
 * 
 * Revision 2.12  91/10/09  16:07:08  af
 * 	Set value of kernel_stack field in stack_handoff().
 * 	[91/08/29            tak]
 * 
 * Revision 2.11  91/07/31  17:39:56  dbg
 * 	Add thread_set_syscall_return.
 * 
 * 	Save user regs directly in PCB on trap, and switch to separate
 * 	kernel stack.
 * 
 * 	Add v8086 mode interrupt support.
 * 	[91/07/30  16:56:09  dbg]
 * 
 * Revision 2.10  91/05/14  16:13:06  mrt
 * 	Correcting copyright
 * 
 * Revision 2.9  91/05/08  12:40:34  dbg
 * 	Use iopb_tss_t for IO permission bitmap.
 * 	[91/03/21            dbg]
 * 
 * Revision 2.8  91/03/16  14:44:51  rpd
 * 	Pulled i386_fpsave_state out of i386_machine_state.
 * 	Added pcb_module_init.
 * 	[91/02/18            rpd]
 * 
 * 	Replaced stack_switch with stack_handoff and
 * 	switch_task_context with switch_context.
 * 	[91/02/18            rpd]
 * 	Added active_stacks.
 * 	[91/01/29            rpd]
 * 
 * Revision 2.7  91/02/05  17:13:19  mrt
 * 	Changed to new Mach copyright
 * 	[91/02/01  17:36:24  mrt]
 * 
 * Revision 2.6  91/01/09  22:41:41  rpd
 * 	Revised the pcb yet again.
 * 	Picked up i386_ISA_PORT_MAP_STATE flavors.
 * 	Added load_context, switch_task_context cover functions.
 * 	[91/01/09            rpd]
 * 
 * Revision 2.5  91/01/08  15:10:58  rpd
 * 	Removed pcb_synch.  Added pcb_collect.
 * 	[91/01/03            rpd]
 * 
 * 	Split i386_machine_state off of i386_kernel_state.
 * 	Set k_stack_top correctly for V8086 threads.
 * 	[90/12/31            rpd]
 * 	Added stack_switch.  Moved stack_alloc_try, stack_alloc,
 * 	stack_free, stack_statistics to kern/thread.c.
 * 	[90/12/14            rpd]
 * 
 * 	Reorganized the pcb.
 * 	Added stack_attach, stack_alloc, stack_alloc_try,
 * 	stack_free, stack_statistics.
 * 	[90/12/11            rpd]
 * 
 * Revision 2.4  90/08/27  21:57:34  dbg
 * 	Return correct count from thread_getstatus.
 * 	[90/08/22            dbg]
 * 
 * Revision 2.3  90/08/07  14:24:47  rpd
 * 	Include seg.h for segment names.
 * 	[90/07/17            dbg]
 * 
 * Revision 2.2  90/05/03  15:35:51  dbg
 * 	Created.
 * 	[90/02/08            dbg]
 * 
 */

#include <cpus.h>
#include <mach_debug.h>

#include <mach/std_types.h>
#include <mach/kern_return.h>
#include <mach/thread_status.h>
#include <mach/vm_param.h>

#include <kern/counters.h>
#include <kern/mach_param.h>
#include <kern/thread.h>
#include <kern/sched_prim.h>
#include <vm/vm_kern.h>
#include <vm/pmap.h>

#include <i386/thread.h>
#include <i386/eflags.h>
#include <i386/proc_reg.h>
#include <i386/seg.h>
#include <i386/tss.h>
#include <i386/user_ldt.h>
#include <i386/fpu.h>

#if	NCPUS > 1
#include <i386/mp_desc.h>
#endif

extern thread_t	Switch_context();
extern void	Thread_continue();

extern struct i386_tss		ktss;
extern struct fake_descriptor	gdt[];

extern iopb_tss_t	iopb_create();
extern void		iopb_destroy();
extern void		user_ldt_free();

zone_t		pcb_zone;

vm_offset_t	kernel_stack[NCPUS];	/* top of active_stack */

/*
 *	stack_attach:
 *
 *	Attach a kernel stack to a thread.
 */

void stack_attach(thread, stack, continuation)
	register thread_t thread;
	register vm_offset_t stack;
	void (*continuation)();
{
	counter(if (++c_stacks_current > c_stacks_max)
			c_stacks_max = c_stacks_current);

	thread->kernel_stack = stack;

	/*
	 *	We want to run continuation, giving it as an argument
	 *	the return value from Load_context/Switch_context.
	 *	Thread_continue takes care of the mismatch between
	 *	the argument-passing/return-value conventions.
	 *	This function will not return normally,
	 *	so we don`t have to worry about a return address.
	 */
	STACK_IKS(stack)->k_eip = (int) Thread_continue;
	STACK_IKS(stack)->k_ebx = (int) continuation;
	STACK_IKS(stack)->k_esp = (int) STACK_IEL(stack);

	/*
	 *	Point top of kernel stack to user`s registers.
	 */
	STACK_IEL(stack)->saved_state = &thread->pcb->iss;
}

/*
 *	stack_detach:
 *
 *	Detaches a kernel stack from a thread, returning the old stack.
 */

vm_offset_t stack_detach(thread)
	register thread_t	thread;
{
	register vm_offset_t	stack;

	counter(if (--c_stacks_current < c_stacks_min)
			c_stacks_min = c_stacks_current);

	stack = thread->kernel_stack;
	thread->kernel_stack = 0;

	return stack;
}

#if	NCPUS > 1
#define	curr_gdt(mycpu)		(mp_gdt[mycpu])
#define	curr_ktss(mycpu)	(mp_ktss[mycpu])
#else
#define	curr_gdt(mycpu)		(gdt)
#define	curr_ktss(mycpu)	(&ktss)
#endif

#define	gdt_desc_p(mycpu,sel) \
	((struct real_descriptor *)&curr_gdt(mycpu)[sel_idx(sel)])

void switch_ktss(pcb)
	register pcb_t	pcb;
{
	int			mycpu = cpu_number();
    {
	register iopb_tss_t	tss = pcb->ims.io_tss;
	vm_offset_t		pcb_stack_top;

	/*
	 *	Save a pointer to the top of the "kernel" stack -
	 *	actually the place in the PCB where a trap into
	 *	kernel mode will push the registers.
	 *	The location depends on V8086 mode.  If we are
	 *	not in V8086 mode, then a trap into the kernel
	 *	won`t save the v86 segments, so we leave room.
	 */

	pcb_stack_top = (pcb->iss.efl & EFL_VM)
			? (int) (&pcb->iss + 1)
			: (int) (&pcb->iss.v86_segs);

	if (tss == 0) {
	    /*
	     *	No per-thread IO permissions.
	     *	Use standard kernel TSS.
	     */
	    if (!(gdt_desc_p(mycpu,KERNEL_TSS)->access & ACC_TSS_BUSY))
		set_tr(KERNEL_TSS);
	    curr_ktss(mycpu)->esp0 = pcb_stack_top;
	}
	else {
	    /*
	     * Set the IO permissions.  Use this thread`s TSS.
	     */
	    *gdt_desc_p(mycpu,USER_TSS)
	    	= *(struct real_descriptor *)tss->iopb_desc;
	    tss->tss.esp0 = pcb_stack_top;
	    set_tr(USER_TSS);
	    gdt_desc_p(mycpu,KERNEL_TSS)->access &= ~ ACC_TSS_BUSY;
	}
    }

    {
	register user_ldt_t	ldt = pcb->ims.ldt;
	/*
	 * Set the thread`s LDT.
	 */
	if (ldt == 0) {
	    /*
	     * Use system LDT.
	     */
	    set_ldt(KERNEL_LDT);
	}
	else {
	    /*
	     * Thread has its own LDT.
	     */
	    *gdt_desc_p(mycpu,USER_LDT) = ldt->desc;
	    set_ldt(USER_LDT);
	}
    }
	/*
	 * Load the floating-point context, if necessary.
	 */
	fpu_load_context(pcb);

}

/*
 *	stack_handoff:
 *
 *	Move the current thread's kernel stack to the new thread.
 */

void stack_handoff(old, new)
	register thread_t	old;
	register thread_t	new;
{
	register int		mycpu = cpu_number();
	register vm_offset_t	stack;

	/*
	 *	Save FP registers if in use.
	 */
	fpu_save_context(old);

	/*
	 *	Switch address maps if switching tasks.
	 */
    {
	task_t old_task, new_task;

	if ((old_task = old->task) != (new_task = new->task)) {
		PMAP_DEACTIVATE_USER(vm_map_pmap(old_task->map),
				     old, mycpu);
		PMAP_ACTIVATE_USER(vm_map_pmap(new_task->map),
				   new, mycpu);
	}
    }

	/*
	 *	Load the rest of the user state for the new thread
	 */
	switch_ktss(new->pcb);

	/*
	 *	Switch to new thread
	 */
	stack = current_stack();
	old->kernel_stack = 0;
	new->kernel_stack = stack;
	active_threads[mycpu] = new;

	/*
	 *	Switch exception link to point to new
	 *	user registers.
	 */

	STACK_IEL(stack)->saved_state = &new->pcb->iss;

}

/*
 * Switch to the first thread on a CPU.
 */
void load_context(new)
	register thread_t	new;
{
	switch_ktss(new->pcb);
	Load_context(new);
}

/*
 * Switch to a new thread.
 * Save the old thread`s kernel state or continuation,
 * and return it.
 */
thread_t switch_context(old, continuation, new)
	register thread_t	old;
	void (*continuation)();
	register thread_t	new;
{
	/*
	 *	Save FP registers if in use.
	 */
	fpu_save_context(old);

	/*
	 *	Switch address maps if switching tasks.
	 */
    {
	task_t old_task, new_task;
	int	mycpu = cpu_number();

	if ((old_task = old->task) != (new_task = new->task)) {
		PMAP_DEACTIVATE_USER(vm_map_pmap(old_task->map),
				     old, mycpu);
		PMAP_ACTIVATE_USER(vm_map_pmap(new_task->map),
				   new, mycpu);
	}
    }

	/*
	 *	Load the rest of the user state for the new thread
	 */
	switch_ktss(new->pcb);

	return Switch_context(old, continuation, new);
}

void pcb_module_init()
{
	pcb_zone = zinit(sizeof(struct pcb),
			 THREAD_MAX * sizeof(struct pcb),
			 THREAD_CHUNK * sizeof(struct pcb),
			 FALSE, "i386 pcb state");

	fpu_module_init();
	iopb_init();
}

void pcb_init(thread)
	register thread_t	thread;
{
	register pcb_t		pcb;

	pcb = (pcb_t) zalloc(pcb_zone);
	if (pcb == 0)
		panic("pcb_init");

	counter(if (++c_threads_current > c_threads_max)
			c_threads_max = c_threads_current);

	/*
	 *	We can't let random values leak out to the user.
	 */
	bzero((char *) pcb, sizeof *pcb);
	simple_lock_init(&pcb->lock);

	/*
	 *	Guarantee that the bootstrapped thread will be in user
	 *	mode.
	 */
	pcb->iss.cs = USER_CS;
	pcb->iss.ss = USER_DS;
	pcb->iss.ds = USER_DS;
	pcb->iss.es = USER_DS;
	pcb->iss.fs = USER_DS;
	pcb->iss.gs = USER_DS;
	pcb->iss.efl = EFL_USER_SET;

	thread->pcb = pcb;
}

void pcb_terminate(thread)
	register thread_t	thread;
{
	register pcb_t		pcb = thread->pcb;

	counter(if (--c_threads_current < c_threads_min)
			c_threads_min = c_threads_current);

	if (pcb->ims.io_tss != 0)
		iopb_destroy(pcb->ims.io_tss);
	if (pcb->ims.ifps != 0)
		fp_free(pcb->ims.ifps);
	if (pcb->ims.ldt != 0)
		user_ldt_free(pcb->ims.ldt);
	zfree(pcb_zone, (vm_offset_t) pcb);
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
 *	thread_setstatus:
 *
 *	Set the status of the specified thread.
 */

kern_return_t thread_setstatus(thread, flavor, tstate, count)
	thread_t		thread;
	int			flavor;
	thread_state_t		tstate;
	unsigned int		count;
{
	switch (flavor) {
	    case i386_THREAD_STATE:
	    case i386_REGS_SEGS_STATE:
	    {
		register struct i386_thread_state	*state;
		register struct i386_saved_state	*saved_state;

		if (count < i386_THREAD_STATE_COUNT) {
		    return(KERN_INVALID_ARGUMENT);
		}

		if (flavor == i386_REGS_SEGS_STATE) {
		    /*
		     * Code and stack selectors must not be null,
		     * and must have user protection levels.
		     * Only the low 16 bits are valid.
		     */
		    state->cs &= 0xffff;
		    state->ss &= 0xffff;
		    state->ds &= 0xffff;
		    state->es &= 0xffff;
		    state->fs &= 0xffff;
		    state->gs &= 0xffff;

		    if (state->cs == 0 || (state->cs & SEL_PL) != SEL_PL_U
		     || state->ss == 0 || (state->ss & SEL_PL) != SEL_PL_U)
			return KERN_INVALID_ARGUMENT;
		}

		state = (struct i386_thread_state *) tstate;

		saved_state = USER_REGS(thread);

		/*
		 * General registers
		 */
		saved_state->edi = state->edi;
		saved_state->esi = state->esi;
		saved_state->ebp = state->ebp;
		saved_state->uesp = state->uesp;
		saved_state->ebx = state->ebx;
		saved_state->edx = state->edx;
		saved_state->ecx = state->ecx;
		saved_state->eax = state->eax;
		saved_state->eip = state->eip;
		saved_state->efl = (state->efl & ~EFL_USER_CLEAR)
				    | EFL_USER_SET;

		/*
		 * Segment registers.  Set differently in V8086 mode.
		 */
		if (state->efl & EFL_VM) {
		    /*
		     * Set V8086 mode segment registers.
		     */
		    saved_state->cs = state->cs & 0xffff;
		    saved_state->ss = state->ss & 0xffff;
		    saved_state->v86_segs.v86_ds = state->ds & 0xffff;
		    saved_state->v86_segs.v86_es = state->es & 0xffff;
		    saved_state->v86_segs.v86_fs = state->fs & 0xffff;
		    saved_state->v86_segs.v86_gs = state->gs & 0xffff;

		    /*
		     * Zero protected mode segment registers.
		     */
		    saved_state->ds = 0;
		    saved_state->es = 0;
		    saved_state->fs = 0;
		    saved_state->gs = 0;

		    if (thread->pcb->ims.v86s.int_table) {
			/*
			 * Hardware assist on.
			 */
			thread->pcb->ims.v86s.flags =
			    state->efl & (EFL_TF | EFL_IF);
		    }
		}
		else if (flavor == i386_THREAD_STATE) {
		    /*
		     * 386 mode.  Set segment registers for flat
		     * 32-bit address space.
		     */
		    saved_state->cs = USER_CS;
		    saved_state->ss = USER_DS;
		    saved_state->ds = USER_DS;
		    saved_state->es = USER_DS;
		    saved_state->fs = USER_DS;
		    saved_state->gs = USER_DS;
		}
		else {
		    /*
		     * User setting segment registers.
		     * Code and stack selectors have already been
		     * checked.  Others will be reset by 'iret'
		     * if they are not valid.
		     */
		    saved_state->cs = state->cs;
		    saved_state->ss = state->ss;
		    saved_state->ds = state->ds;
		    saved_state->es = state->es;
		    saved_state->fs = state->fs;
		    saved_state->gs = state->gs;
		}
		break;
	    }

	    case i386_FLOAT_STATE: {

		if (count < i386_FLOAT_STATE_COUNT)
			return(KERN_INVALID_ARGUMENT);

		return fpu_set_state(thread,
				(struct i386_float_state *) tstate);
	    }

	    /*
	     * Temporary - replace by i386_io_map
	     */
	    case i386_ISA_PORT_MAP_STATE: {
		register struct i386_isa_port_map_state *state;
		register iopb_tss_t	tss;

		if (count < i386_ISA_PORT_MAP_STATE_COUNT)
			return(KERN_INVALID_ARGUMENT);

#if 0
		/*
		 *	If the thread has no ktss yet,
		 *	we must allocate one.
		 */

		state = (struct i386_isa_port_map_state *) tstate;
		tss = thread->pcb->ims.io_tss;
		if (tss == 0) {
			tss = iopb_create();
			thread->pcb->ims.io_tss = tss;
		}

		bcopy((char *) state->pm,
		      (char *) tss->bitmap,
		      sizeof state->pm);
#endif
		break;
	    }

	    case i386_V86_ASSIST_STATE:
	    {
		register struct i386_v86_assist_state *state;
		vm_offset_t	int_table;
		int		int_count;

		if (count < i386_V86_ASSIST_STATE_COUNT)
		    return KERN_INVALID_ARGUMENT;

		state = (struct i386_v86_assist_state *) tstate;
		int_table = state->int_table;
		int_count = state->int_count;

		if (int_table >= VM_MAX_ADDRESS ||
		    int_table +
			int_count * sizeof(struct v86_interrupt_table)
			    > VM_MAX_ADDRESS)
		    return KERN_INVALID_ARGUMENT;

		thread->pcb->ims.v86s.int_table = int_table;
		thread->pcb->ims.v86s.int_count = int_count;

		thread->pcb->ims.v86s.flags =
			USER_REGS(thread)->efl & (EFL_TF | EFL_IF);
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
	thread_state_t		tstate;	/* pointer to OUT array */
	unsigned int		*count;		/* IN/OUT */
{
	switch (flavor)  {
	    case THREAD_STATE_FLAVOR_LIST:
		if (*count < 4)
		    return (KERN_INVALID_ARGUMENT);
		tstate[0] = i386_THREAD_STATE;
		tstate[1] = i386_FLOAT_STATE;
		tstate[2] = i386_ISA_PORT_MAP_STATE;
		tstate[3] = i386_V86_ASSIST_STATE;
		*count = 4;
		break;

	    case i386_THREAD_STATE:
	    case i386_REGS_SEGS_STATE:
	    {
		register struct i386_thread_state	*state;
		register struct i386_saved_state	*saved_state;

		if (*count < i386_THREAD_STATE_COUNT)
		    return(KERN_INVALID_ARGUMENT);

		state = (struct i386_thread_state *) tstate;
		saved_state = USER_REGS(thread);

		/*
		 * General registers.
		 */
		state->edi = saved_state->edi;
		state->esi = saved_state->esi;
		state->ebp = saved_state->ebp;
		state->ebx = saved_state->ebx;
		state->edx = saved_state->edx;
		state->ecx = saved_state->ecx;
		state->eax = saved_state->eax;
		state->eip = saved_state->eip;
		state->efl = saved_state->efl;
		state->uesp = saved_state->uesp;

		state->cs = saved_state->cs;
		state->ss = saved_state->ss;
		if (saved_state->efl & EFL_VM) {
		    /*
		     * V8086 mode.
		     */
		    state->ds = saved_state->v86_segs.v86_ds & 0xffff;
		    state->es = saved_state->v86_segs.v86_es & 0xffff;
		    state->fs = saved_state->v86_segs.v86_fs & 0xffff;
		    state->gs = saved_state->v86_segs.v86_gs & 0xffff;

		    if (thread->pcb->ims.v86s.int_table) {
			/*
			 * Hardware assist on
			 */
			if ((thread->pcb->ims.v86s.flags &
					(EFL_IF|V86_IF_PENDING))
				== 0)
			    state->efl &= ~EFL_IF;
		    }
		}
		else {
		    /*
		     * 386 mode.
		     */
		    state->ds = saved_state->ds & 0xffff;
		    state->es = saved_state->es & 0xffff;
		    state->fs = saved_state->fs & 0xffff;
		    state->gs = saved_state->gs & 0xffff;
		}
		*count = i386_THREAD_STATE_COUNT;
		break;
	    }

	    case i386_FLOAT_STATE: {

		if (*count < i386_FLOAT_STATE_COUNT)
			return(KERN_INVALID_ARGUMENT);

		*count = i386_FLOAT_STATE_COUNT;
		return fpu_get_state(thread,
				(struct i386_float_state *)tstate);
	    }

	    /*
	     * Temporary - replace by i386_io_map
	     */
	    case i386_ISA_PORT_MAP_STATE: {
		register struct i386_isa_port_map_state *state;
		register iopb_tss_t tss;

		if (*count < i386_ISA_PORT_MAP_STATE_COUNT)
			return(KERN_INVALID_ARGUMENT);

		state = (struct i386_isa_port_map_state *) tstate;
		tss = thread->pcb->ims.io_tss;

		if (tss == 0) {
		    int i;

		    /*
		     *	The thread has no ktss, so no IO permissions.
		     */

		    for (i = 0; i < sizeof state->pm; i++)
			state->pm[i] = 0xff;
		} else {
		    /*
		     *	The thread has its own ktss.
		     */

		    bcopy((char *) tss->bitmap,
			  (char *) state->pm,
			  sizeof state->pm);
		}

		*count = i386_ISA_PORT_MAP_STATE_COUNT;
		break;
	    }

	    case i386_V86_ASSIST_STATE:
	    {
		register struct i386_v86_assist_state *state;

		if (*count < i386_V86_ASSIST_STATE_COUNT)
		    return KERN_INVALID_ARGUMENT;

		state = (struct i386_v86_assist_state *) tstate;
		state->int_table = thread->pcb->ims.v86s.int_table;
		state->int_count = thread->pcb->ims.v86s.int_count;

		*count = i386_V86_ASSIST_STATE_COUNT;
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
	thread->pcb->iss.eax = retval;
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
	register struct i386_saved_state *saved_state;

	arg_size = (arg_size + sizeof(int) - 1) & ~(sizeof(int)-1);
	arg_addr = stack_base + stack_size - arg_size;

	saved_state = USER_REGS(current_thread());
	saved_state->uesp = (int)arg_addr;
	saved_state->eip = entry[0];

	return (arg_addr);
}
