/* 
 * Mach Operating System
 * Copyright (c) 1993-1987 Carnegie Mellon University
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
 * $Log:	db_trace.c,v $
 * Revision 2.13  93/11/17  17:54:06  dbg
 * 	Carried over mods made for alpha, db_all_cprocs(<task>) works.
 * 	Prototypes.
 * 	[93/09/24            af]
 * 
 * Revision 2.12  93/05/17  10:26:05  rvb
 * 	Type casts, etc to quiet gcc 2.3.3 warnings
 * 
 * Revision 2.11  93/01/14  17:51:30  danner
 * 	Fixed casts, extern/static complaints.
 * 	[93/01/14            danner]
 * 
 * 	db_symbol_values() changed its looks.
 * 	[92/12/01            af]
 * 
 * 	Fixes for ANSI C.
 * 	[92/08/22            jvh]
 * 	Added a declaration for getreg_val().
 * 	[92/11/06            cmaeda]
 * 
 * Revision 2.10  91/10/09  16:14:11  af
 * 	Fixed db_trace_cproc.
 * 	[91/09/19            rpd]
 * 
 * Revision 2.9.3.1  91/10/05  13:12:51  jeffreyh
 * 	Supported stack trace and register access of non current thread.
 * 	Supported user register access.
 * 	Fixed incorrect register reference.
 * 	Fixed guess_procedure to limit search and skip leading NOPs.
 * 	[91/09/05            tak]
 * 
 * Revision 2.9  91/05/14  17:33:59  mrt
 * 	Correcting copyright
 * 
 * Revision 2.8  91/03/16  14:55:54  rpd
 * 	Fixed db_trace_cproc to handle wired cthreads.
 * 	Fixed db_cproc_state.
 * 	[91/03/10            rpd]
 * 	Fixed tracing of swapped threads.
 * 	Removed unnecessary uses of db_get_value.
 * 	[91/03/01  17:49:16  rpd]
 * 
 * Revision 2.7  91/02/14  14:37:26  mrt
 * 	Split trace function from ddb interface.  Added (optional)
 * 	code to walk through a user-mode, optimized CThread program
 * 	like the U*x server.  Should make it MI someday.
 * 	[91/02/12  12:18:59  af]
 * 
 * Revision 2.6  91/02/05  17:48:14  mrt
 * 	Added author notices
 * 	[91/02/04  11:22:18  mrt]
 * 
 * 	Changed to use new Mach copyright
 * 	[91/02/02  12:25:54  mrt]
 * 
 * Revision 2.5  91/01/09  19:56:45  rpd
 * 	Fixed stack tracing for threads without kernel stacks.
 * 	[91/01/09            rpd]
 * 
 * Revision 2.4  91/01/08  15:49:38  rpd
 * 	Quit tracing after hitting a zero pc.
 * 	[90/12/31            rpd]
 * 	Changed for new pcb organization.
 * 	[90/11/12            rpd]
 * 
 * Revision 2.3  90/09/09  14:33:21  rpd
 * 	Fixed trace when hitting a breakpoint in user space.
 * 	Fixed to trace crossing over to user space for any
 * 	thread (e.g. "t/tu <thread>" command).
 * 	Documented magic adjustments of pc and ra in various places.
 * 	Locals work, provided you either do not strip or use
 * 	my fixed version of xstrip.
 * 	[90/08/30  16:56:46  af]
 * 
 * Revision 2.2  90/08/27  22:07:51  dbg
 * 	Fix some sloppy type (non)-declarations.
 * 	[90/08/27            dbg]
 * 
 * 	Got rid of ddb_regs: all lives on the stack now.
 * 	BEWARE: to make this possible the "valuep" field in
 * 	the db_regs array is _not_ a pointer, but a displacement
 * 	off the base of the exception frame.  Use _exclusively_
 * 	the (MI) db_read/write_variable functions to access registers.
 * 	Updated db_getreg_val accordingly.
 * 	Cleaned up a lot the trace functions, I hope they still work
 * 	without a valid symtab...
 * 	Made apparent that we cannot get a stack trace given _only_
 * 	a stack pointer, as the stack is not self-describing.
 * 	Not sure why local variables do not come out, xstrip problem ?
 * 	[90/08/20  10:09:54  af]
 * 
 * 	Turn debug off
 * 	[90/08/18  00:17:56  af]
 * 
 * 	Created, from my old KDB code.  History summary:
 * 		From jsb: better stack traces.
 * 		[90/04/23            rpd]
 * 		Changed tracing code to reflect 11 arguments for all traps.
 * 		[90/05/23            rwd]
 * 		Switch over to the MI KDB, fixes and cleanups.
 * 		[90/01/20            af]
 * 		Created.
 * 		[89/08/08            af]
 * 
 */
/*
 *	File: db_trace.c
 * 	Author: Alessandro Forin, Carnegie Mellon University
 *	Date:	8/89
 *
 *	Stack traceback routines for MIPS
 */

#include <mach/boolean.h>
#include <machine/db_machdep.h>
#include <kern/thread.h>
#include <kern/task.h>
#include <ddb/db_access.h>
#include <ddb/db_variables.h>
#include <ddb/db_sym.h>
#include <ddb/db_task_thread.h>
#include <ddb/db_command.h>
#include <ddb/db_output.h>
#include <machine/db_trace.h>

#include <mips/mips_cpu.h>
#include <mach/vm_param.h>

#define	DEBUG 0

extern int start();	/* lowest kernel code address */
extern char end[];	/* highest kernel code address */
extern natural_t db_maxoff;
extern natural_t getreg_val();

#define REG_ARG(i)	(4+i)
#define SAVES_RA(x)	isa_spill((x),31)

#define KERN_SAVE_REG_IDX(vp)	( \
	((vp)->valuep >= (int *)(&((struct mips_saved_state *)0)->s0) &&    \
	 (vp)->valuep <= (int *)(&((struct mips_saved_state *)0)->s7))?	    \
		vp->valuep - (int *)(&((struct mips_saved_state *)0)->s0):  \
	((vp)->valuep >= (int *)(&((struct mips_saved_state *)0)->sp) &&    \
	 (vp)->valuep <= (int *)(&((struct mips_saved_state *)0)->ra))?	    \
		((vp)->valuep-(int *)(&((struct mips_saved_state *)0)->sp)) + \
		 ((int *)(&((struct mips_kernel_state *)0)->sp) - (int *)0):  \
	 -1)

extern	db_sym_t	localsym();

/*
 * Machine register set.
 */
struct mips_saved_state *db_cur_exc_frame = 0;

/* 
  forward declarations
*/
static void db_cproc_state(int ,char s[4]);
static int saves_arg(long, int, task_t);

static
db_setf_regs(
	struct db_variable	*vp,
	db_expr_t		*valuep,
	int			op,
	db_var_aux_param_t	ap)
{
	register		*regp = 0;
	int			null_reg = 0;
	register thread_t	thread = ap->thread;
	
	if (db_option(ap->modif, 'u')) {
		if (thread == THREAD_NULL) { 
			if ((thread = current_thread()) == THREAD_NULL)
				db_error("no user registers\n");
				/* NOTREACHED */
		}
		if (thread == current_thread() &&
			(db_cur_exc_frame->sr & SR_KUo))
			regp = (int *)((char *)db_cur_exc_frame + 
					(int) (vp->valuep));
	} else {
		if (thread == THREAD_NULL || thread == current_thread()) {
			regp = (int *)((char *) db_cur_exc_frame
					 + (int) (vp->valuep));
		} else if ((thread->state & TH_SWAPPED) == 0 &&
			    thread->kernel_stack) {
			int regno = KERN_SAVE_REG_IDX(vp);
			regp = (regno < 0)?
				 &null_reg:
				 ((int *) STACK_MKS(thread->kernel_stack))
						+ regno;
		} else if ((thread->state & TH_SWAPPED) &&
			    thread->swap_func != 
				(void (*)())thread_exception_return) {
			/* only PC is valid */
			if (vp->valuep == 
				(int *)&(((struct mips_saved_state *)0)->pc)) {
				regp = (int *) (&thread->swap_func);
			} else {
				regp = &null_reg;
			}
		}
	}
	if (regp == 0) {
		if (thread->pcb == 0)
			db_error("no pcb\n");
		regp = (int *)((char *)(&thread->pcb->mss) + (int)(vp->valuep));
	}
	if (op == DB_VAR_GET)
		*valuep = *regp;
	else if (op == DB_VAR_SET)
		*regp = *valuep;
}

struct db_variable db_regs[] = {
	{ "at",	(int*)&(((struct mips_saved_state *)0)->at),  db_setf_regs },
	{ "v0",	(int*)&(((struct mips_saved_state *)0)->v0),  db_setf_regs },
	{ "v1",	(int*)&(((struct mips_saved_state *)0)->v1),  db_setf_regs },
	{ "a0",	(int*)&(((struct mips_saved_state *)0)->a0),  db_setf_regs },
	{ "a1",	(int*)&(((struct mips_saved_state *)0)->a1),  db_setf_regs },
	{ "a2",	(int*)&(((struct mips_saved_state *)0)->a2),  db_setf_regs },
	{ "a3",	(int*)&(((struct mips_saved_state *)0)->a3),  db_setf_regs },
	{ "t0",	(int*)&(((struct mips_saved_state *)0)->t0),  db_setf_regs },
	{ "t1",	(int*)&(((struct mips_saved_state *)0)->t1),  db_setf_regs },
	{ "t2",	(int*)&(((struct mips_saved_state *)0)->t2),  db_setf_regs },
	{ "t3",	(int*)&(((struct mips_saved_state *)0)->t3),  db_setf_regs },
	{ "t4",	(int*)&(((struct mips_saved_state *)0)->t4),  db_setf_regs },
	{ "t5",	(int*)&(((struct mips_saved_state *)0)->t5),  db_setf_regs },
	{ "t6",	(int*)&(((struct mips_saved_state *)0)->t6),  db_setf_regs },
	{ "t7",	(int*)&(((struct mips_saved_state *)0)->t7),  db_setf_regs },
	{ "s0",	(int*)&(((struct mips_saved_state *)0)->s0),  db_setf_regs },
	{ "s1",	(int*)&(((struct mips_saved_state *)0)->s1),  db_setf_regs },
	{ "s2",	(int*)&(((struct mips_saved_state *)0)->s2),  db_setf_regs },
	{ "s3",	(int*)&(((struct mips_saved_state *)0)->s3),  db_setf_regs },
	{ "s4",	(int*)&(((struct mips_saved_state *)0)->s4),  db_setf_regs },
	{ "s5",	(int*)&(((struct mips_saved_state *)0)->s5),  db_setf_regs },
	{ "s6",	(int*)&(((struct mips_saved_state *)0)->s6),  db_setf_regs },
	{ "s7",	(int*)&(((struct mips_saved_state *)0)->s7),  db_setf_regs },
	{ "t8",	(int*)&(((struct mips_saved_state *)0)->t8),  db_setf_regs },
	{ "t9",	(int*)&(((struct mips_saved_state *)0)->t9),  db_setf_regs },
	{ "k0",	(int*)&(((struct mips_saved_state *)0)->k0),  db_setf_regs },
	{ "k1",	(int*)&(((struct mips_saved_state *)0)->k1),  db_setf_regs },
	{ "gp",	(int*)&(((struct mips_saved_state *)0)->gp),  db_setf_regs },
	{ "sp",	(int*)&(((struct mips_saved_state *)0)->sp),  db_setf_regs },
	{ "fp",	(int*)&(((struct mips_saved_state *)0)->fp),  db_setf_regs },
	{ "ra",	(int*)&(((struct mips_saved_state *)0)->ra),  db_setf_regs },
	{ "sr",	(int*)&(((struct mips_saved_state *)0)->sr),  db_setf_regs },
	{ "mdlo",(int*)&(((struct mips_saved_state *)0)->mdlo),  db_setf_regs },
	{ "mdhi",(int*)&(((struct mips_saved_state *)0)->mdhi),  db_setf_regs },
	{ "bad", (int*)&(((struct mips_saved_state *)0)->bad_address), db_setf_regs },
	{ "cs",	(int*)&(((struct mips_saved_state *)0)->cause),  db_setf_regs },
	{ "pc",	(int*)&(((struct mips_saved_state *)0)->pc),  db_setf_regs },
};
struct db_variable *db_eregs = db_regs + sizeof(db_regs)/sizeof(db_regs[0]);

static struct mips_saved_state mips_reg_env;

struct mips_saved_state *
db_get_reg_env(
	thread_t	thread,
	boolean_t	user)
{
	register struct db_variable *dp;
	struct db_var_aux_param aux_param;

	aux_param.modif = (user)? "u": "";
	aux_param.level = 0;
	aux_param.thread = thread;
	for (dp = db_regs; dp < db_eregs; dp++) {
		db_read_write_variable(dp, 
				(int *)((int)&mips_reg_env + (int)dp->valuep),
				DB_VAR_GET, &aux_param);
	}
	return(&mips_reg_env);
}

/*
 * Stack trace.
 */

void
db_mips_stack_trace(
	int		count,
	vm_offset_t	stackp,
	int		the_pc,
	int		the_ra,
	int		flags,
	vm_offset_t	kstackp,
	thread_t	thread);		/* forward */

void print_exception_frame(
	struct mips_saved_state *fp,
	unsigned int	epc);

int guess_procedure(
	unsigned int	pc,
	task_t		task);

void guess_frame(
	db_sym_t	sym,
	long int		pc,
	frame_info_t	fr,
	task_t		task,
	struct mips_saved_state *reg_env);

void
db_stack_trace_cmd(
	db_expr_t	addr,
	int		have_addr,
	db_expr_t	count,
	char		*modif)
{
#define	F_KERNEL_ONLY	1
#define	F_TRACE_THREAD	2
#define	F_DO_LOCALS	4
#define	F_PRINT_FRAME	8
	unsigned	flags = F_KERNEL_ONLY;
	long int	the_pc, the_ra;
	register char 	c;
	thread_t	th;
	vm_offset_t kstackp, stackp;

	while ((c = *modif++) != 0) {
	    if (c == 't')
		flags |= F_TRACE_THREAD;
	    if (c == 'u')
		flags &= ~F_KERNEL_ONLY;
	    if (c == 'l')
		flags |= F_DO_LOCALS;
	    if (c == 'e')
		flags |= F_PRINT_FRAME;
	}

	if (flags & F_TRACE_THREAD) {
	    if (have_addr) {
		if (!db_check_thread_address_valid(addr))
		    return;
		th = (thread_t) addr;
	    } else {
		th = db_default_thread;
		if (th == THREAD_NULL)
		    th = current_thread();
		if (th == THREAD_NULL) {
		    db_printf("no active thread\n");
		    return;
		}
	    }
	    if (th->state & TH_SWAPPED) {
		register pcb_t pcb = th->pcb;
		db_sym_t sym;
		char *name;
		int proc_off;

		sym = db_search_task_symbol((db_addr_t) th->swap_func, 
					    DB_STGY_PROC, &proc_off, th->task);
		if (sym == DB_SYM_NULL) {
			/* no procedures ? won't be able to do much */
		  db_find_xtrn_task_sym_and_offset((db_addr_t) th->swap_func,
					       &name, &proc_off, th->task);
		} else 
		  db_symbol_values(0, sym, &name, 0);

		if (name == 0 || proc_off != 0) {
			db_printf("Continuation %x\n", th->swap_func);
		} else {
			db_printf("Continuation %s\n", name);
		}

		kstackp = 0;
		stackp = pcb->mss.sp;
		the_pc = pcb->mss.pc;
		the_ra = pcb->mss.ra;
	    } else {
		if (th == current_thread())
		    goto no_addr;

		kstackp = th->kernel_stack;
		stackp = STACK_MKS(kstackp)->sp;
		the_pc = STACK_MKS(kstackp)->pc;
		the_ra = the_pc; /* ??? */
	    }
	} else if (!have_addr) {
	no_addr:
	    th = current_thread();
	    kstackp = current_stack();
	    stackp = db_cur_exc_frame->sp;
	    the_pc = db_cur_exc_frame->pc;
	    the_ra = db_cur_exc_frame->ra;
	} else {
	    db_error("You can only do this by fixing $sp, $pc and $ra");
	}
	db_mips_stack_trace(count,stackp,the_pc,the_ra,flags,kstackp,th);
}


void
db_mips_stack_trace(
	int		count,
	vm_offset_t	stackp,
	int		the_pc,
	int		the_ra,
	int		flags,
	vm_offset_t	kstackp,
	thread_t	thread)
{
	static char	*ubanner = "*** User stack:\n";

	char		*name;
	db_sym_t	sym;
	unsigned	framep = 0, argp;
	task_t		task = (thread == THREAD_NULL)? TASK_NULL: thread->task;
	unsigned	proc_pc;

	int		narg, nloc;
	unsigned	minpc,maxpc;
	boolean_t	top_frame = 1;
	struct frame_info f;
	int		proc_off;
	int 		Nest;
	struct mips_saved_state *reg_env;

	if (the_ra)
		the_ra -= 8;	/* backup jal */

	minpc = (unsigned)start;
	maxpc = (unsigned)end;

	if (ISA_KUSEG(the_pc)) {
	    if (flags & F_KERNEL_ONLY)
		goto out;
	    else {
		db_printf("%s", ubanner);
		minpc = VM_MIN_ADDRESS;
		maxpc = VM_MAX_ADDRESS;
	    }
	    reg_env = db_get_reg_env(thread, TRUE);
	    if (flags & F_PRINT_FRAME)
		print_exception_frame(reg_env, the_pc);
	} else
	    reg_env = db_get_reg_env(thread, FALSE);

	if (count == -1)
	    count = 65535;

	while (count--) {
		if (the_pc == 0)
			break;
		if ((the_pc < minpc) || (the_pc > maxpc) ||
		    (the_pc & 3) || (framep & 3)) {
			if (framep & 3)
				db_printf("bad frame pointer %X\n", framep);
			else
				db_printf("bad pc %X\n", the_pc);
			break;
		}

		bzero(&f, sizeof f);

		/* 1: find out where we are */
		sym = db_search_task_symbol(the_pc,DB_STGY_PROC,&proc_off,task);
		if (sym == DB_SYM_NULL) {
			/* no procedures ? wont be able to do much */
			db_find_xtrn_task_sym_and_offset(the_pc, &name,
							&proc_off, task);
			proc_pc = the_pc - proc_off;
		} else
			db_symbol_values(0, sym, &name, &proc_pc);

		if (name == 0 || proc_off > db_maxoff) {
			proc_off = -1;
			sym = 0;
			proc_pc = guess_procedure(the_pc, task);
			db_printf("%x(", proc_pc);
		} else {
			db_printf("%s(", name);
		}

		/* 2: find out more about it */
		guess_frame(sym, the_pc, &f, task, reg_env);
		nloc = f.nloc;
		narg = f.narg;

		/*
		 * Where are the arguments ?
		 * Where is the next frame ?
		 */
		if (top_frame) {
			if (f.at_entry)
				framep = stackp;

			if (f.mod_sp)
				/* New frame allocated */
				framep = stackp + f.framesize;

			argp = framep;

			/* Top procedure made any calls ? */
			if (!f.at_entry && f.mod_sp)
				goto saved_ra;
		} else if (f.isvector) {
			struct mips_exception_link *link;
			struct mips_saved_state *exc;

			db_printf(")\n");

			/* Mach syscalls are "varargs": see
			 * how we call them in locore.s
			 */
			if ((kstackp != 0) &&
			    (stackp == (unsigned)STACK_MEL(kstackp) - 28))
				stackp = (unsigned)STACK_MEL(kstackp);

			link = (struct mips_exception_link *)stackp;
			exc = link->eframe;
			the_pc = exc->pc;
			if (flags & F_PRINT_FRAME)
				print_exception_frame(exc, the_pc);
			the_ra = exc->ra;
			stackp = exc->sp;
			*reg_env = *exc;
#if	DEBUG
			db_printf("{Vector %x %x %x}\n",
				the_pc, the_ra, stackp);
#endif
			if (ISA_KUSEG(the_pc)) {
				if (flags & F_KERNEL_ONLY)
					goto out;
				db_printf("%s", ubanner);
				minpc = VM_MIN_ADDRESS;
				maxpc = VM_MAX_ADDRESS;
			}
			top_frame = TRUE;
			continue;
		} else {
			/* Frame is allocated, or else */
			framep = stackp + f.framesize;
			argp = framep;
			if (f.at_entry) {
				/* Not the top frame and RA not saved ?
				 * Must be start, or else we dont know
				 */
				the_ra = 0;
			} else {
saved_ra:
				the_ra = db_get_task_value(
						framep + f.saved_pc_off,
						4, FALSE, task);
#if	DEBUG
				db_printf("{%x}", the_ra);
#endif
				if (the_ra <= minpc || the_ra > maxpc)
					the_ra = 0;
				else
					/* backup before jal */
					the_ra -= 8;
			}
		}

		/* 2.5: Print arguments */
		if (top_frame) {
			register int i;
			for (i = 0; narg && i < 4; i++, argp += 4) {
				if ((sym = localsym(sym, 0, &Nest)) != 0) {
					db_symbol_values(0,sym,&name,0);
#if	DEBUG
					if (Nest != 1)
						db_printf("@"); /* oops! */
#endif
					db_printf("%s=", name);
				}
				if (saves_arg(the_pc, REG_ARG(i), task)) {
					db_printf("%x", db_get_task_value(argp,
							 4, FALSE, task));
				} else
					db_printf("%x", 
					    getreg_val(REG_ARG(i), reg_env));
				if (--narg)
					db_printf(",");
			}
		}
		for (; narg--; argp += 4) {
			if ((sym = localsym(sym, 0, &Nest)) != 0) {
				db_symbol_values(0,sym,&name, 0);
#if	DEBUG
				if (Nest != 1)
					db_printf("@"); /* XXX oops! */
#endif
				db_printf("%s=", name);
			}
			db_printf("%x", db_get_task_value(argp,4,FALSE,task));
			if (narg)
				db_printf(",");
		}
		db_printf(")");

		/* 3: give more detail about where we are */
		if (proc_off != -1)
			db_printf("+%x", proc_off);
		if (f.filename)
			db_printf(" [%s:%d]", f.filename, f.linenum);
		db_printf("\n");

		/* 4: possibly print local vars */
		if (flags & F_DO_LOCALS) {
			unsigned w;
			boolean_t isReg;

			while ((sym = localsym(sym, &isReg, &Nest)) != 0) {
				if (Nest != 2)	/* 2 = top-level locals */
					continue;

				db_symbol_values(0, sym, &name, &w);
				if (isReg)
					w = getreg_val(w, reg_env);
				else
					w = db_get_task_value(w + stackp, 4,
								 FALSE, task);
				db_printf("\t%s=%x\n", name, w);
			}
		}

		/* 5: find out where we go after this */

		db_restore_regs(reg_env, stackp, proc_pc, the_pc, task);
		if (f.mod_sp)
			stackp = framep;
		top_frame = FALSE;
		the_pc = the_ra;
	}
out:
#if	DEBUG
	db_printf("{fp=%x, esp=%x}\n", framep, db_cur_exc_frame->sp);
#endif
	return;
}

#define	CTHREADS_SUPPORT	1

#if	CTHREADS_SUPPORT

thread_t
db_find_kthread(
	vm_offset_t	ustack_base,
	vm_size_t	ustack_top,
	task_t		task)
{
	thread_t thread;

	queue_iterate(&task->thread_list, thread, thread_t, thread_list) {
		vm_offset_t	usp = thread->pcb->mss.sp;
		if (usp >= ustack_base && usp < ustack_top)
			return thread;
	}
	return THREAD_NULL;
}

/* offsets in a cproc structure */
int db_cproc_next_offset = 0 * 4;
int db_cproc_incarnation_offset = 1 * 4;
int db_cproc_list_offset = 2 * 4;
int db_cproc_wait_offset = 3 * 4;
int db_cproc_context_offset = 5 * 4;
int db_cproc_state_offset = 7 * 4;
int db_cproc_stack_base_offset = 10 * 4 + sizeof(mach_msg_header_t);
int db_cproc_stack_size_offset = 11 * 4 + sizeof(mach_msg_header_t);

/* offsets in a cproc_switch context structure */
int db_cproc_pc_offset = 13 * 4;
int db_cproc_stack_offset = 14 * 4;

#include <machine/setjmp.h>

extern jmp_buf_t *db_recover;

void
db_trace_cproc(
	vm_offset_t	cproc,
	thread_t	thread)
{
	jmp_buf_t	db_jmpbuf;
	jmp_buf_t	*prev = db_recover;
	task_t		task;

	task = (thread == THREAD_NULL)? TASK_NULL: thread->task;

	if (!_setjmp(db_recover = &db_jmpbuf)) {
		char pstate[4];
		unsigned int s, w, n, c, cth;

		s = db_get_task_value(cproc + db_cproc_state_offset, 4,
				      FALSE, task);
		w = db_get_task_value(cproc + db_cproc_wait_offset, 4,
				      FALSE, task);
		n = db_get_task_value(cproc + db_cproc_next_offset, 4,
				      FALSE, task);
		c = db_get_task_value(cproc + db_cproc_context_offset, 4,
				      FALSE, task);
		cth = db_get_task_value(cproc + db_cproc_incarnation_offset, 4,
					FALSE, task);

		db_cproc_state(s, pstate);

		db_printf("CThread %x (cproc %x) %s", cth, cproc, pstate);
		if (w) db_printf(" awaits %x", w);
		if (n) db_printf(" next %x", n);
		db_printf("\n");

		if ((s != 0) && (c != 0)) {
			unsigned int pc;

			pc = db_get_task_value(c + db_cproc_pc_offset, 4,
					       FALSE, task);
			db_mips_stack_trace(-1, c + db_cproc_stack_offset,
					    pc, pc, F_DO_LOCALS, 0, thread);
		} else {
			db_addr_t sb;
			vm_size_t ss;

			sb = db_get_task_value(cproc + db_cproc_stack_base_offset, sizeof(db_expr_t), FALSE, task);
			ss = db_get_task_value(cproc + db_cproc_stack_size_offset, sizeof(db_expr_t), FALSE, task);
			db_printf(" Stack base: %x\n", sb);
			/*
			 *  Lessee now..
			 */
			thread = db_find_kthread(sb, sb+ss, task);
			if ((thread != THREAD_NULL)) {
			    db_addr_t pc, ra, usp;
			    pc = thread->pcb->mss.pc;
			    ra = thread->pcb->mss.ra;
			    usp = thread->pcb->mss.sp;
			    db_mips_stack_trace(-1, usp, pc, ra,
			    		F_DO_LOCALS, 0, thread);
			}
		}
	}

	db_recover = prev;
}

void
db_all_cprocs(
	task_t		task,
	db_expr_t	cproc_list)
{
	jmp_buf_t	db_jmpbuf;
	jmp_buf_t 	*prev = db_recover;
	thread_t	thread;
	db_expr_t	cproc, next;

	if (task != TASK_NULL) {
		thread = (thread_t) queue_first(&task->thread_list);
	} else
		thread = current_thread();

	if (cproc_list != 0)
		next = cproc_list;
	else
		if (!db_value_of_name("unix::cproc_list", &next)) {
			db_printf("No cprocs.\n");
			return;
		}

	while (next) {
		if (_setjmp(db_recover = &db_jmpbuf))
			break;

		cproc = db_get_task_value(next, sizeof(db_expr_t), FALSE, task);
		if (cproc == 0) break;
		next = cproc + db_cproc_list_offset;

		db_trace_cproc(cproc, thread);
	}

	db_recover = prev;
}

static void db_cproc_state(int state,char s[4])
{
	if (state == 0) {
		*s++ = 'R';
	} else {
		if (state & 1) *s++ = 'S';
		if (state & 2) *s++ = 'B';
		if (state & 4) *s++ = 'C';
	}
	*s = 0;
}

#endif	/* CTHREADS_SUPPORT */

int
guess_procedure(
	unsigned int pc,
	task_t	 task)
{
    unsigned	    w;
    int             bw_pc, fw_pc;

    if (pc < (unsigned)start)
    	fw_pc = VM_MIN_ADDRESS;
    else
        fw_pc = (unsigned)start;
    if (fw_pc + db_maxoff < pc)
	fw_pc = pc - db_maxoff;

    for (bw_pc = pc; bw_pc >= fw_pc; bw_pc -= 4) {
	w = db_get_task_value(bw_pc, 4, FALSE, task);
	if (isa_ret(w))
	    break;
    }
    if (bw_pc < fw_pc)
	return fw_pc;
    /*
     * skip "j ra" and delayed slot, and skip leading NOPs
     */
    for (bw_pc += 8; bw_pc < pc; bw_pc += 4) {
	if (db_get_task_value(bw_pc, 4, FALSE, task))
	    break;
    }
    return bw_pc;
}

int curproc;

void guess_frame(
	db_sym_t sym,
	long int pc,
	frame_info_t fr,
	task_t task,
	struct mips_saved_state *reg_env)
{
	int inc;
	unsigned w;
	int fw_pc;
	char *name;

	if (sym != DB_SYM_NULL)
		db_symbol_values(0, sym, &name, &curproc);
	else
		curproc = guess_procedure(pc, task);

	if (curproc == 0)
		curproc = pc;

	/*
	 * Given the pc, figure out how we might have
	 * modified the sp.
	 */
	if (findproc(sym, fr, pc)) {
		fr->at_entry = 1;
		if (fr->mod_sp) {
			/* maybe the sp has not yet been modified */
			inc = 0;
			for (fw_pc = curproc; fw_pc < pc; fw_pc += 4){
				w = db_get_task_value(fw_pc, 4, FALSE, task);
				inc -= stack_modified(w, getreg_val, 
							(unsigned)reg_env);
				if (SAVES_RA(w))
					fr->at_entry = 0;
			}
			fr->mod_sp = (inc > 0);
			if (inc && inc != fr->framesize) {
#if	DEBUG
				db_printf("[?frame %x != %x]", inc, fr->framesize);
#endif
			}
		}
		return;
	}
	/* Not found in symtab, play guessing games */

	/* Guess 1: did we save the RA and where, or not */
	fr->at_entry = 1;
	inc = 0;
	for (fw_pc = curproc; fw_pc < pc; fw_pc += 4){
		w = db_get_task_value(fw_pc, 4, FALSE, task);
		inc -= stack_modified(w, getreg_val, (unsigned)reg_env);
		if (SAVES_RA(w))
			fr->at_entry = 0;
	}

	/* Guess 2: did we alter the SP */
	fr->mod_sp = (inc > 0);
	fr->framesize = inc;
	fr->nloc = inc / 4;

	/* Defaults for the unknowns */
	fr->narg = 5;
	fr->isleaf = 0;
	fr->isvector = 0;
	fr->saved_pc_off = -4;
}

db_expr_t
db_getreg_val(
	db_expr_t	r)
{
	if (r == 0)
		return 0;
	db_read_write_variable(&db_regs[r - 1], &r, DB_VAR_GET, 0);
	return r;
}


static int saves_arg(
	long  		pc,
        int		n,
	task_t		task)
{
	long fw_pc;
	unsigned w;

	for (fw_pc = curproc; fw_pc < pc; fw_pc += 4) {
		w = db_get_task_value(fw_pc, 4, FALSE, task);
		if (isa_spill(w,n))
			return 1;
	}
	return 0;
}

void print_exception_frame(
	register struct mips_saved_state *fp,
	unsigned int epc)
{
	db_printf("\t Exception: taken at pc=%x, frame (at %x) :\n", epc, fp);
	db_printf("\t\t tlblo=%-13X tlbhi=%-13X tlbix=%-13X tlbcx=%-13X\n",
		  fp->tlb_low, fp->tlb_high,
		  fp->tlb_index, fp->tlb_context);
	db_printf("\t\t at=%-16X v0=%-16X v1=%-16X a0=%-16X\n",
		  fp->at, fp->v0, fp->v1, fp->a0);
	db_printf("\t\t a1=%-16X a2=%-16X a3=%-16X t0=%-16X\n",
		  fp->a1, fp->a2, fp->a3, fp->t0);
	db_printf("\t\t t1=%-16X t2=%-16X t3=%-16X t4=%-16X\n",
		  fp->t1, fp->t2, fp->t3, fp->t4);
	db_printf("\t\t t5=%-16X t6=%-16X t7=%-16X s0=%-16X\n",
		  fp->t5, fp->t6, fp->t7, fp->s0);
	db_printf("\t\t s1=%-16X s2=%-16X s3=%-16X s4=%-16X\n",
		  fp->s1, fp->s2, fp->s3, fp->s4);
	db_printf("\t\t s5=%-16X s6=%-16X s7=%-16X t8=%-16X\n",
		  fp->s5, fp->s6, fp->s7, fp->t8);
	db_printf("\t\t t9=%-16X k0=%-16X k1=%-16X gp=%-16X\n",
		  fp->t9, fp->k0, fp->k1, fp->gp);
	db_printf("\t\t sp=%-16X fp=%-16X ra=%-16X sr=%-16X\n",
		  fp->sp, fp->fp, fp->ra, fp->sr);
	db_printf("\t\t lo=%-16X hi=%-16X bad=%-15X cs=%-16X\n",
		  fp->mdlo, fp->mdhi, fp->bad_address, fp->cause);
	db_printf("\t\t pc=%-16X\n", fp->pc);
}
