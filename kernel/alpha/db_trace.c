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
 * $Log:	db_trace.c,v $
 * Revision 2.5  93/03/11  13:57:41  danner
 * 	Debug off.
 * 
 * Revision 2.4  93/03/09  10:50:07  danner
 * 	Fix for findproc.  GCC lint.
 * 	[93/03/05            af]
 * 
 * Revision 2.3  93/01/19  08:59:20  danner
 * 	Locks are longs now.
 * 	[93/01/15            af]
 * 
 * Revision 2.2  93/01/14  17:12:51  danner
 * 	Now we can do a "!db_all_cprocs($task2)".
 * 	Also, we properly trace ALL cthreads, wired ones included.
 * 	[92/12/22  03:09:43  af]
 * 
 * 	Created, from mips version.
 * 	[92/06/01            af]
 * 
 */
/*
 *	File: db_trace.c
 * 	Author: Alessandro Forin, Carnegie Mellon University
 *	Date:	6/92
 *
 *	Stack traceback routines for ALPHA
 */

#include <mach/boolean.h>
#include <machine/db_machdep.h>
#include <kern/thread.h>
#include <kern/task.h>
#include <ddb/db_variables.h>
#include <ddb/db_sym.h>
#include <ddb/db_task_thread.h>
#include <machine/db_trace.h>

#include <alpha/alpha_cpu.h>
#include <mach/vm_param.h>

#define	DEBUG 0

#define	RA	26
#define	REG_ARGS 6
#define	A0	16
#define	RZERO	31

extern int start();	/* lowest kernel code address */
extern char end[];	/* highest kernel code address */
extern unsigned db_maxoff;

#define REG_ARG(i)	(A0+i)			/* from def of reg a0 */
#define SAVES_RA(x)	isa_spill((x),RA)

/* forward */
static db_cproc_state(
	int		state,
	char		s[4] );
static int saves_arg(
	db_addr_t	pc,
	int		n,
	task_t		task);


static long *
db_kern_saved_register_ptr(vp, thread)
	register struct db_variable	*vp;
	thread_t			thread;
{
	struct alpha_saved_state *s_base = (struct alpha_saved_state *) 0;
	struct alpha_kernel_state *k_zero = (struct alpha_kernel_state *) 0;
	struct alpha_kernel_state *k_base;

	k_base = STACK_MKS(thread->kernel_stack);

	if ( (vp->valuep >= (long *)(&s_base->s0)) &&
	     (vp->valuep <= (long *)(&s_base->s6)) )
		return (long *)k_base + (vp->valuep - (long *)(&s_base->s0));

	if ( vp->valuep == (long *)(&s_base->ra) )
		return (long *)(&k_base->pc);

	if ( vp->valuep == (long *)(&s_base->sp) )
		return (long *)(&k_base->sp);

	return 0;
}

extern	db_sym_t	localsym();

/*
 * Machine register set.
 */
struct alpha_saved_state *db_cur_exc_frame = 0;

db_sp()
{
	int i;
	db_printf("Stack now at approx %X\n", &i);
}

static int
db_setf_regs(vp, valuep, op, ap)
	struct db_variable	*vp;
	db_expr_t		*valuep;
	int			op;
	db_var_aux_param_t	ap;
{
	register long		*regp = 0;
	long			null_reg = 0;
	register thread_t	thread = ap->thread;
	
	if (db_option(ap->modif, 'u')) {
		if (thread == THREAD_NULL) { 
			if ((thread = current_thread()) == THREAD_NULL)
				db_error("no user registers\n");
				/* NOTREACHED */
		}
		if (thread == current_thread() &&
			(alpha_user_mode(db_cur_exc_frame->saved_frame.saved_ps)))
			regp = (long *)((char *)db_cur_exc_frame + 
					(long) (vp->valuep));
	} else {
		if (thread == THREAD_NULL || thread == current_thread()) {
			regp = (long *)((char *) db_cur_exc_frame
					 + (long) (vp->valuep));
		} else if ((thread->state & TH_SWAPPED) == 0 &&
			    thread->kernel_stack) {
			if ((regp = db_kern_saved_register_ptr(vp, thread)) == 0)
				regp = &null_reg;
		} else if ((thread->state & TH_SWAPPED) &&
			    thread->swap_func != 
				(void (*)())thread_exception_return) {
			/* only PC is valid */
			if (vp->valuep == 
				(long *)&(((struct alpha_saved_state *)0)->saved_frame.saved_pc)) {
				regp = (long *) (&thread->swap_func);
			} else {
				regp = &null_reg;
			}
		}
	}
	if (regp == 0) {
		if (thread->pcb == 0)
			db_error("no pcb\n");
		regp = (long *)((char *)(&thread->pcb->mss) + (long)(vp->valuep));
	}
	if (op == DB_VAR_GET)
		*valuep = *regp;
	else if (op == DB_VAR_SET)
		*regp = *valuep;
	return(0);
}

struct db_variable db_regs[] = {
	{ "v0",	(long*)&(((struct alpha_saved_state *)0)->v0),  db_setf_regs },
	{ "t0",	(long*)&(((struct alpha_saved_state *)0)->t0),  db_setf_regs },
	{ "t1",	(long*)&(((struct alpha_saved_state *)0)->saved_frame.saved_r2),  db_setf_regs },
	{ "t2",	(long*)&(((struct alpha_saved_state *)0)->saved_frame.saved_r3),  db_setf_regs },
	{ "t3",	(long*)&(((struct alpha_saved_state *)0)->saved_frame.saved_r4),  db_setf_regs },
	{ "t4",	(long*)&(((struct alpha_saved_state *)0)->saved_frame.saved_r5),  db_setf_regs },
	{ "t5",	(long*)&(((struct alpha_saved_state *)0)->saved_frame.saved_r6),  db_setf_regs },
	{ "t6",	(long*)&(((struct alpha_saved_state *)0)->saved_frame.saved_r7),  db_setf_regs },
	{ "t7",	(long*)&(((struct alpha_saved_state *)0)->t7),  db_setf_regs },
	{ "s0",	(long*)&(((struct alpha_saved_state *)0)->s0),  db_setf_regs },
	{ "s1",	(long*)&(((struct alpha_saved_state *)0)->s1),  db_setf_regs },
	{ "s2",	(long*)&(((struct alpha_saved_state *)0)->s2),  db_setf_regs },
	{ "s3",	(long*)&(((struct alpha_saved_state *)0)->s3),  db_setf_regs },
	{ "s4",	(long*)&(((struct alpha_saved_state *)0)->s4),  db_setf_regs },
	{ "s5",	(long*)&(((struct alpha_saved_state *)0)->s5),  db_setf_regs },
	{ "s6",	(long*)&(((struct alpha_saved_state *)0)->s6),  db_setf_regs },
	{ "a0",	(long*)&(((struct alpha_saved_state *)0)->a0),  db_setf_regs },
	{ "a1",	(long*)&(((struct alpha_saved_state *)0)->a1),  db_setf_regs },
	{ "a2",	(long*)&(((struct alpha_saved_state *)0)->a2),  db_setf_regs },
	{ "a3",	(long*)&(((struct alpha_saved_state *)0)->a3),  db_setf_regs },
	{ "a4",	(long*)&(((struct alpha_saved_state *)0)->a4),  db_setf_regs },
	{ "a5",	(long*)&(((struct alpha_saved_state *)0)->a5),  db_setf_regs },
	{ "t8",	(long*)&(((struct alpha_saved_state *)0)->t8),  db_setf_regs },
	{ "t9",	(long*)&(((struct alpha_saved_state *)0)->t9),  db_setf_regs },
	{ "t10",(long*)&(((struct alpha_saved_state *)0)->t10),  db_setf_regs },
	{ "t11",(long*)&(((struct alpha_saved_state *)0)->t11),  db_setf_regs },
	{ "ra",	(long*)&(((struct alpha_saved_state *)0)->ra),  db_setf_regs },
	{ "t12",(long*)&(((struct alpha_saved_state *)0)->t12),  db_setf_regs },
	{ "at",	(long*)&(((struct alpha_saved_state *)0)->at),  db_setf_regs },
	{ "gp",	(long*)&(((struct alpha_saved_state *)0)->gp),  db_setf_regs },
	{ "sp",	(long*)&(((struct alpha_saved_state *)0)->sp),  db_setf_regs },
	{ "pc",	(long*)&(((struct alpha_saved_state *)0)->saved_frame.saved_pc),  db_setf_regs },
	{ "ps",	(long*)&(((struct alpha_saved_state *)0)->saved_frame.saved_ps),  db_setf_regs },
	{ "bad", (long*)&(((struct alpha_saved_state *)0)->bad_address), db_setf_regs },
	{ "cs",	(long*)&(((struct alpha_saved_state *)0)->cause),  db_setf_regs },
	{ "ai", (long*)&(((struct alpha_saved_state *)0)->t11),  db_setf_regs },
	{ "pv", (long*)&(((struct alpha_saved_state *)0)->t12),  db_setf_regs },
};
struct db_variable *db_eregs = db_regs + sizeof(db_regs)/sizeof(db_regs[0]);

static struct alpha_saved_state alpha_reg_env;

struct alpha_saved_state *
db_get_reg_env(thread, user)
	thread_t	thread;
	boolean_t	user;
{
	register struct db_variable *dp;
	struct db_var_aux_param aux_param;

	aux_param.modif = (user)? "u": "";
	aux_param.level = 0;
	aux_param.thread = thread;
	for (dp = db_regs; dp < db_eregs; dp++) {
		db_read_write_variable(dp, 
				(int *)((db_expr_t)&alpha_reg_env + (db_expr_t)dp->valuep),
				DB_VAR_GET, &aux_param);
	}
	return(&alpha_reg_env);
}

/*
 * Stack trace.
 */

void db_alpha_stack_trace();

void
db_stack_trace_cmd(addr, have_addr, count, modif)
	db_expr_t	addr;
	boolean_t	have_addr;
	db_expr_t	count;
	char		*modif;
{
#define	F_KERNEL_ONLY	0x1
#define	F_TRACE_THREAD	0x2
#define	F_DO_LOCALS	0x4
#define	F_PRINT_FRAME	0x8
#define F_TRUST_USP	0x10
	unsigned	flags = F_KERNEL_ONLY;
	db_addr_t	the_pc, the_ra;
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
		th = (thread_t) addr;
		if (!db_check_thread_address_valid(th))
		    return;
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
		db_addr_t proc_off;

		sym = db_search_task_symbol( (db_addr_t)th->swap_func,
						DB_STGY_PROC, 
						&proc_off, th->task);
		if (sym == DB_SYM_NULL) {
			/* no procedures ? won't be able to do much */
			db_find_xtrn_task_sym_and_offset(
						(db_addr_t) th->swap_func,
						&name, &proc_off, th->task);
		} else {
			db_symbol_values(0, sym, &name, 0);
		}

		if (name == 0 || proc_off != 0) {
			db_printf("Continuation %x\n", th->swap_func);
		} else {
			db_printf("Continuation %s\n", name);
		}

		kstackp = 0;
		stackp = pcb->mss.sp;
		the_pc = pcb->mss.saved_frame.saved_pc;
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
	    the_pc = db_cur_exc_frame->saved_frame.saved_pc;
	    the_ra = db_cur_exc_frame->ra;
	} else {
	    db_error("You can only do this by fixing $sp, $pc and $ra");
	}
	db_alpha_stack_trace(count,stackp,the_pc,the_ra,flags,kstackp,th);
}

extern long getreg_val();

void
db_alpha_stack_trace(count, stackp, the_pc, the_ra, flags, kstackp, thread)
	vm_offset_t	stackp, kstackp;
	db_addr_t	the_pc, the_ra;
	thread_t	thread;
{
	static char	*ubanner = "*** User stack:\n";

	char		*name;
	db_sym_t	sym;
	vm_offset_t	framep = 0, argp;
	task_t		task = (thread == THREAD_NULL)? TASK_NULL: thread->task;
	db_expr_t	proc_pc;

	int		narg, nloc;
	db_addr_t	minpc,maxpc;
	boolean_t	top_frame = 1;
	struct frame_info f;
	db_addr_t	proc_off;
	int 		Nest;
	struct alpha_saved_state *reg_env;

	if (the_ra)
		the_ra -= sizeof(alpha_instruction);	/* backup jsr */

	minpc = (db_addr_t)start;
	maxpc = (db_addr_t)end;

	if (ISA_KUSEG(the_pc)) {
	    if (flags & F_KERNEL_ONLY)
		goto out;
	    else {
		if ((flags & F_TRUST_USP) == 0)
			stackp = thread->pcb->mss.hw_pcb.usp;
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
			db_symbol_values( 0, sym, &name, &proc_pc);

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
#if	DEBUG
		db_printf("{f: %x %x %x %x",
			f.narg, f.nloc, f.framesize, f.regmask);
		db_printf(" %x %x %x %x",
			f.saved_pc_off,	f.isleaf, f.isvector, f.mod_sp);
		db_printf(" %x %x}",
			f.at_entry, f.linenum);
#endif	/* DEBUG */

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

			/* argsave is callee-alloced (mips was caller) */
			argp = framep - (REG_ARGS * sizeof(db_expr_t));

			/* Top procedure made any calls ? */
			if (!f.at_entry && f.mod_sp)
				goto saved_ra;
		} else if (f.isvector) {
			struct alpha_saved_state *exc;

			db_printf(")\n");
#if 0
	notyet
			/* Mach syscalls are "varargs": see
			 * how we call them in locore.s
			 */
#endif
#if	DEBUG
db_printf("[V %x (%x %x) %x", kstackp, STACK_MEL(kstackp), STACK_MSB(kstackp), stackp);
#endif
			if ((kstackp != 0) &&
			    (stackp == (vm_offset_t)STACK_MEL(kstackp) - sizeof(struct alpha_kernel_state))) {
				/*
				 * Top exc frame, probably came from user mode
				 */
				exc = &STACK_MSB(kstackp)->pcb->mss;
#if	DEBUG
db_printf(" %x]", exc);
#endif
				/* move what locore did not */
				*reg_env = *exc;
				reg_env->saved_frame = STACK_MEL(kstackp)->tf;
				/* user sp in hw_pcb */
				reg_env->sp = exc->hw_pcb.usp;

				the_pc = reg_env->saved_frame.saved_pc;
				the_ra = reg_env->ra;
				stackp = reg_env->sp;

				if (flags & F_PRINT_FRAME)
					print_exception_frame(reg_env, the_pc);

				if (flags & F_KERNEL_ONLY)
					goto out;

				db_printf("%s", ubanner);
				minpc = VM_MIN_ADDRESS;
				maxpc = VM_MAX_ADDRESS;

			} else {
				/*
				 * Came from kernel mode
				 */
				struct alpha_exception_link *link;

				stackp += sizeof(struct alpha_saved_state);
				link = (struct alpha_exception_link *)stackp;
#if	DEBUG
db_printf(" %x]", link);
#endif
				exc = link->eframe;

				/* move what locore did not */
				*reg_env = *exc;
				reg_env->saved_frame = link->tf;

				the_pc = reg_env->saved_frame.saved_pc;
				the_ra = reg_env->ra;
				/* reconstruct sp from alignment in PS */
				{
					natural_t ps = link->tf.saved_ps;
					stackp = (vm_offset_t)&link->tf;/*trapped*/
					stackp += sizeof(struct trap_frame) + (ps>>56);
				}
				if (flags & F_PRINT_FRAME)
					print_exception_frame(reg_env, the_pc);
			}
			/*
			 * Now restart from top
			 */
#if	DEBUG
			db_printf("{Vector %x %x %x}\n",
				the_pc, the_ra, stackp);
#endif	/* DEBUG */
			top_frame = TRUE;
			continue;

		} else {
			/* Frame is allocated, or else */
			framep = stackp + f.framesize;

			/* argsave is callee-alloced (mips was caller) */
			argp = framep - (REG_ARGS * sizeof(db_expr_t));

			if (f.at_entry) {
				/* Not the top frame and RA not saved ?
				 * Must be start, or else we dont know
				 */
				the_ra = 0;
			} else {
saved_ra:
				the_ra = db_get_task_value(
						framep + f.saved_pc_off,
						sizeof the_ra, FALSE, task);
#if	DEBUG
				db_printf("{%x}", the_ra);
#endif	/* DEBUG */
				if (the_ra <= minpc || the_ra > maxpc)
					the_ra = 0;
				else
					/* backup before jsr */
					the_ra -= sizeof(alpha_instruction);
			}
		}

		/* 2.5: Print arguments */
		if (top_frame) {
			register int i;
			for (i = 0; narg && i < REG_ARGS; i++, argp += sizeof(db_expr_t)) {
				if (sym = localsym(sym, 0, &Nest)) {
					db_symbol_values(0,sym,&name,0);
#if	DEBUG
					if (Nest != 1)
						db_printf("@"); /* oops! */
#endif	/* DEBUG */
					db_printf("%s=", name);
				}
				if (saves_arg(the_pc, REG_ARG(i), task)) {
					db_printf("%x", db_get_task_value(argp,
							 sizeof(db_expr_t), FALSE, task));
				} else
					db_printf("%x", 
					    getreg_val(REG_ARG(i), reg_env));
				if (--narg)
					db_printf(",");
			}
		}
		for (; narg--; argp += sizeof(db_expr_t)) {
			if (sym = localsym(sym, 0, &Nest)) {
				db_symbol_values(0,sym,&name, 0);
#if	DEBUG
				if (Nest != 1)
					db_printf("@"); /* XXX oops! */
#endif	/* DEBUG */
				db_printf("%s=", name);
			}
			db_printf("%x", db_get_task_value(argp,sizeof(db_expr_t),FALSE,task));
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
			db_expr_t	w;
			boolean_t	isReg;

#if	DEBUG
			db_printf("framep==%x\n", framep);
#endif	/* DEBUG */
			while (sym = localsym(sym, &isReg, &Nest)) {
				if (Nest != 2)	/* 2 = top-level locals */
					continue;

				db_symbol_values(0,sym, &name, &w);
#if	DEBUG
				db_printf("{%X %d}", w, isReg);
#endif	/* DEBUG */
				if (isReg)
					w = getreg_val(w, reg_env);
				else
					w = db_get_task_value(w + framep,
							    sizeof(db_expr_t),
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
#endif	/* DEBUG*/
	return;
}

#define	EXTRA_UX_SUPPORT	1

#ifdef	EXTRA_UX_SUPPORT

thread_t
db_find_kthread(ustack_base, ustack_top, task)
	vm_offset_t	ustack_base;
	vm_size_t	ustack_top;
	task_t		task;
{
	thread_t thread;

	queue_iterate(&task->thread_list, thread, thread_t, thread_list) {
		vm_offset_t	usp = thread->pcb->mss.hw_pcb.usp;
		if (usp >= ustack_base && usp < ustack_top)
			return thread;
	}
	return THREAD_NULL;
}

/* offsets in a cproc structure */
int db_cproc_next_offset = 0 * sizeof(db_expr_t);
int db_cproc_incarnation_offset = 1 * sizeof(db_expr_t);
int db_cproc_list_offset = 2 * sizeof(db_expr_t);
int db_cproc_wait_offset = 3 * sizeof(db_expr_t);
int db_cproc_context_offset = 5 * sizeof(db_expr_t);
int db_cproc_state_offset = 7 * sizeof(db_expr_t);
int db_cproc_stack_base_offset = 10 * sizeof(db_expr_t) + sizeof(mach_msg_header_t);
int db_cproc_stack_size_offset = 11 * sizeof(db_expr_t) + sizeof(mach_msg_header_t);

/* offsets relative to a cproc_switch context structure */
int db_cprocsw_pc_offset = 14 * sizeof(db_expr_t);
int db_cprocsw_framesize = 15 * sizeof(db_expr_t);	/* see alpha/csw.s */

#include <machine/setjmp.h>

extern jmp_buf_t *db_recover;

db_trace_cproc(cproc, thread)
	vm_offset_t	cproc;
	thread_t	thread;
{
	jmp_buf_t db_jmpbuf;
	jmp_buf_t *prev = db_recover;
	task_t task = (thread == THREAD_NULL)? TASK_NULL: thread->task;

	if (!_setjmp(db_recover = &db_jmpbuf)) {
		char pstate[4];
		db_addr_t s, w, n, c, cth;

		s = db_get_task_value(cproc + db_cproc_state_offset, sizeof(int), FALSE, task);
		w = db_get_task_value(cproc + db_cproc_wait_offset, sizeof(db_expr_t), FALSE, task);
		n = db_get_task_value(cproc + db_cproc_next_offset, sizeof(db_expr_t), FALSE, task);
		c = db_get_task_value(cproc + db_cproc_context_offset, sizeof(db_expr_t), FALSE, task);
		cth = db_get_task_value(cproc + db_cproc_incarnation_offset, sizeof(db_expr_t), FALSE, task);

		db_cproc_state(s, pstate);

		db_printf("CThread %x (cproc %x) %s", cth, cproc, pstate);
		if (w) db_printf(" awaits %x", w);
		if (n) db_printf(" next %x", n);
		db_printf("\n");

		if ((s != 0) && (c != 0)) {
			db_addr_t pc, stk;

			pc = db_get_task_value(c + db_cprocsw_pc_offset, sizeof(db_expr_t), FALSE, task);
			stk = c + db_cprocsw_framesize;
			db_alpha_stack_trace(-1, stk, pc, pc,
					    F_TRUST_USP|F_DO_LOCALS, 0,
					    thread);
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
			    db_addr_t pc, ra;
			    pc = thread->pcb->mss.saved_frame.saved_pc;
			    ra = thread->pcb->mss.ra;
			    db_alpha_stack_trace(-1, sb/*ignored*/, pc, ra,
			    		F_DO_LOCALS, 0, thread);
			}
		}
	}

	db_recover = prev;
}

db_all_cprocs(task, cproc_list)
	task_t		task;
	db_expr_t	cproc_list;
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

static db_cproc_state(
	int	state,
	char	s[4])
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

#endif	EXTRA_UX_SUPPORT

guess_procedure(pc, task)
	db_addr_t pc;
	task_t	 task;
{
	alpha_instruction w;
	db_addr_t         bw_pc, fw_pc;

	if (pc < (db_addr_t) start)
		fw_pc = VM_MIN_ADDRESS;
	else
		fw_pc = (db_addr_t) start;
	if (fw_pc + db_maxoff < pc)
		fw_pc = pc - db_maxoff;

	for (bw_pc = pc; bw_pc >= fw_pc; bw_pc -= sizeof(w)) {
		w.bits = db_get_task_value(bw_pc, sizeof(w), FALSE, task);
		if (isa_ret(w))
			break;
	}
	if (bw_pc < fw_pc)
		return (fw_pc);
	/*
	 * skip "ret" and skip padding zeroes
	 */
	for (bw_pc += sizeof(w); bw_pc < pc; bw_pc += sizeof(w)) {
		if (db_get_task_value(bw_pc, sizeof(w), FALSE, task))
			break;
	}
	return (bw_pc);
}

db_expr_t curproc;

guess_frame(sym, pc, fr, task, reg_env)
	db_sym_t	sym;
	db_addr_t	pc;
	frame_info_t	fr;
	task_t		task;
	struct alpha_saved_state *reg_env;
{
	int             inc;
	db_addr_t	bw_pc, fw_pc, binc, finc;
	char           *name;
	alpha_instruction	w;

	if (sym != DB_SYM_NULL)
		db_symbol_values(0,sym, &name, &curproc);
	else
		curproc = guess_procedure(pc, task);

	if (curproc == 0)
		curproc = pc;

	/*
	 * Given the pc, figure out how we might have modified the sp. 
	 */
	if (findproc(db_last_symtab, sym, fr, pc)) {
		fr->at_entry = 1;
		if (fr->mod_sp) {
			/* maybe the sp has not yet been modified */
			inc = 0;
			for (fw_pc = curproc; fw_pc < pc;
			     fw_pc += sizeof(alpha_instruction)) {
				w.bits = db_get_task_value(fw_pc,
							   sizeof(alpha_instruction),
							   FALSE, task);
				inc -= stack_modified(w, getreg_val,
						      (vm_offset_t) reg_env);
				if (SAVES_RA(w.bits))
					fr->at_entry = 0;
			}
			fr->mod_sp = (inc > 0);
			if (inc && inc != fr->framesize) {
#if	DEBUG
				db_printf("[?frame %x != %x]", inc, fr->framesize);
#endif	/* DEBUG */
			}
		}
		return;
	}
	/* Not found in symtab, play guessing games */

	/* Guess 1: did we save the RA and where, or not */
	fr->at_entry = 1;
	inc = 0;
	for (fw_pc = curproc; fw_pc < pc; fw_pc += sizeof(w)) {
		w.bits = db_get_task_value(fw_pc, sizeof(w), FALSE, task);
		inc -= stack_modified(w, getreg_val, (vm_offset_t) reg_env);
		if (SAVES_RA(w.bits))
			fr->at_entry = 0;
	}

	/* Guess 2: did we alter the SP */
	fr->mod_sp = (inc > 0);
	fr->framesize = inc;
	fr->nloc = inc / sizeof(long);

	/* Defaults for the unknowns */
	fr->narg = 5;
	fr->isleaf = 0;
	fr->isvector = 0;
	fr->saved_pc_off = -sizeof(db_addr_t);
}

db_expr_t
db_getreg_val(r)
	db_expr_t	r;
{
	if (r == RZERO)
		return 0;
	db_read_write_variable(&db_regs[r], &r, DB_VAR_GET, 0);
	return r;
}


static int saves_arg(
	db_addr_t	pc,
	int		n,
	task_t		task)
{
	db_addr_t 		fw_pc;
	alpha_instruction	ins;
	boolean_t		ret = FALSE;

	for (fw_pc = curproc; fw_pc < pc; fw_pc += sizeof(ins)) {
		ins.bits = db_get_task_value(fw_pc, sizeof(ins), FALSE, task);
		if (isa_spill(ins,n)) {
			ret = TRUE;
			break;
		}
	}
#if	DEBUG
db_printf("[saves %x %x %x -> %x]", pc, n, task, ret);
#endif
	return ret;
}

print_exception_frame(fp, epc)
	register struct alpha_saved_state *fp;
	db_addr_t epc;
{
	db_printf("\t Exception: taken at pc=%x, frame (at %x) :\n", epc, fp);
	db_printf("\t\t v0=%-16X t0=%-16X t1=%-16X t2=%-16X\n",
		  fp->v0, fp->t0,
		  fp->saved_frame.saved_r2,
		  fp->saved_frame.saved_r3);
	db_printf("\t\t t3=%-16X t4=%-16X t5=%-16X t6=%-16X\n",
		  fp->saved_frame.saved_r4, fp->saved_frame.saved_r5,
		  fp->saved_frame.saved_r6, fp->saved_frame.saved_r7);
	db_printf("\t\t t7=%-16X s0=%-16X s1=%-16X s2=%-16X\n",
		  fp->t7, fp->s0, fp->s1, fp->s2);
	db_printf("\t\t s3=%-16X s4=%-16X s5=%-16X s6=%-16X\n",
		  fp->s3, fp->s4, fp->s5, fp->s6);
	db_printf("\t\t a0=%-16X a1=%-16X a2=%-16X a3=%-16X\n",
		  fp->a0, fp->a1, fp->a2, fp->a3);
	db_printf("\t\t a4=%-16X a5=%-16X t8=%-16X t9=%-16X\n",
		  fp->a4, fp->a5, fp->t8, fp->t9);
	db_printf("\t\t t10=%-15X t11=%-15X ra=%-16X pv=%-16X\n",
		  fp->t10, fp->t11, fp->ra, fp->t12);
	db_printf("\t\t at=%-16X gp=%-16X sp=%-16X ps=%-16X\n",
		  fp->at, fp->gp, fp->sp, fp->saved_frame.saved_ps);
	db_printf("\t\t bad=%-15X cs=%-16X pc=%-16X\n",
		  fp->bad_address, fp->cause, fp->saved_frame.saved_pc);
}
