/* 
 * Mach Operating System
 * Copyright (c) 1993-1990 Carnegie Mellon University
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
 * $Log:	db_run.c,v $
 * Revision 2.14  93/01/19  09:00:47  danner
 * 	64bit cleanup. software single step code revision.
 * 	comment correction (jfriedl).
 * 	[92/12/10  16:20:16  af]
 * 
 * Revision 2.13  93/01/14  17:25:38  danner
 * 	added use of db_branch_is_delayed in db_restart_at_pc. 
 * 	[92/11/28            jfriedl]
 * 	64bit cleanup.
 * 	[92/12/10  16:20:16  af]
 * 
 * Revision 2.12  92/08/03  17:31:57  jfriedl
 * 	removed silly prototypes
 * 	[92/08/02            jfriedl]
 * 
 * Revision 2.11  92/05/21  17:07:41  jfriedl
 * 	Removed unused variable 'bkpt' from db_clear_task_single_step().
 * 	[92/05/16            jfriedl]
 * 
 * Revision 2.10  92/04/01  10:54:05  rpd
 * 	Added missing newline print in n/p command.
 * 	[92/03/20            danner]
 * 
 * Revision 2.9  91/10/09  16:02:08  af
 * 	Added task parameter for user space break point, and changed
 * 	 db_find_breakpoint_here to db_find_thread_breakpoint_here
 * 	 to support thread based break point.
 * 	Changed db_{set,clear}_single_step to db_{set,clear}_task_single
 * 	 _step to add task paramter and to maintain compatibility.
 * 	[91/08/29            tak]
 * 
 * Revision 2.8  91/07/09  23:16:01  danner
 * 	Added logic to db_set_single_step not to set a breakpoint at the
 * 	 next sequential instruction if the current instruction is an
 * 	 unconditional transfer of flow of control instruction. This
 * 	 avoids problems with the debugger overwriting data or clobbering
 * 	 routines that the debugger itself might need. This is determined
 * 	 by calling the predicate inst_unconditional_flow_transfer. This
 * 	 predicate now needs to be defined for all architectures using
 * 	 the software single step. 
 * 
 * 	     Added include of ddb/db_run.h, where all the STEP defines have been
 * 	      moved.
 * 	 
 * 	[91/07/08            danner]
 * 
 * Revision 2.7  91/06/06  17:03:51  jsb
 * 	Removed redundant newlines and tabs.
 * 	Added delta to instruction count printf.
 * 	[91/05/25  10:55:01  jsb]
 * 
 * Revision 2.6  91/05/14  15:35:39  mrt
 * 	Correcting copyright
 * 
 * Revision 2.5  91/02/05  17:06:58  mrt
 * 	Changed to new Mach copyright
 * 	[91/01/31  16:19:05  mrt]
 * 
 * Revision 2.4  91/01/08  15:09:10  rpd
 * 	Fixed bug in db_restart_at_pc.
 * 	[90/12/07            rpd]
 * 	Added STEP_COUNT and count option to db_continue_cmd.
 * 	Changed db_stop_at_pc to return (modified) is_breakpoint.
 * 	Fixed db_stop_at_pc to print newlines in the right places.
 * 	[90/11/27            rpd]
 * 
 * Revision 2.3  90/10/25  14:43:59  rwd
 * 	Changed db_find_breakpoint to db_find_breakpoint_here.
 * 	[90/10/18            rpd]
 * 
 * 	Fixed db_set_single_step to pass regs to branch_taken.
 * 	Added watchpoint argument to db_restart_at_pc.
 * 	[90/10/17            rpd]
 * 	Generalized the watchpoint support.
 * 	[90/10/16            rwd]
 * 	Added watchpoint support.
 * 	[90/10/16            rpd]
 * 
 * Revision 2.2  90/08/27  21:51:59  dbg
 * 	Fixed names for single-step functions.
 * 	[90/08/20            af]
 * 	Reduce lint.
 * 	[90/08/07            dbg]
 * 	Created.
 * 	[90/07/25            dbg]
 * 
 */
/*
 * 	Author: David B. Golub, Carnegie Mellon University
 *	Date:	7/90
 */

/*
 * Commands to run process.
 */
#include <mach/boolean.h>
#include <machine/db_machdep.h>

#include <ddb/db_lex.h>
#include <ddb/db_break.h>
#include <ddb/db_access.h>
#include <ddb/db_run.h>
#include <ddb/db_task_thread.h>

int	db_run_mode;

boolean_t	db_sstep_print;
int		db_loop_count;
int		db_call_depth;

int		db_inst_count;
int		db_last_inst_count;
int		db_load_count;
int		db_store_count;

#ifndef db_set_single_step
void		db_set_task_single_step(/* db_regs_t *, task_t */);/* forward */
#else
#define	db_set_task_single_step(regs,task)	db_set_single_step(regs)
#endif
#ifndef db_clear_single_step
void		db_clear_task_single_step(/* db_regs_t *, task_t */);
#else
#define db_clear_task_single_step(regs,task)	db_clear_single_step(regs)
#endif

boolean_t
db_stop_at_pc(is_breakpoint, task)
	boolean_t	*is_breakpoint;
	task_t		task;
{
	register  db_addr_t	pc;
	register  db_thread_breakpoint_t bkpt;
	boolean_t db_cond_check();

	db_clear_task_single_step(DDB_REGS, task);
	db_clear_breakpoints();
	db_clear_watchpoints();
	pc = PC_REGS(DDB_REGS);

#ifdef	FIXUP_PC_AFTER_BREAK
	if (*is_breakpoint) {
	    /*
	     * Breakpoint trap.  Fix up the PC if the
	     * machine requires it.
	     */
	    FIXUP_PC_AFTER_BREAK
	    pc = PC_REGS(DDB_REGS);
	}
#endif

	/*
	 * Now check for a breakpoint at this address.
	 */
	bkpt = db_find_thread_breakpoint_here(task, pc);
	if (bkpt) {
	    if (db_cond_check(bkpt)) {
		*is_breakpoint = TRUE;
		return (TRUE);	/* stop here */
	    }
	}
	*is_breakpoint = FALSE;

	if (db_run_mode == STEP_INVISIBLE) {
	    db_run_mode = STEP_CONTINUE;
	    return (FALSE);	/* continue */
	}
	if (db_run_mode == STEP_COUNT) {
	    return (FALSE); /* continue */
	}
	if (db_run_mode == STEP_ONCE) {
	    if (--db_loop_count > 0) {
		if (db_sstep_print) {
		    db_print_loc_and_inst(pc, task);
		}
		return (FALSE);	/* continue */
	    }
	}
	if (db_run_mode == STEP_RETURN) {
	    /* WARNING: the following assumes an instruction fits an int */
	    db_expr_t ins = db_get_task_value(pc, sizeof(int), FALSE, task);

	    /* continue until matching return */

	    if (!inst_trap_return(ins) &&
		(!inst_return(ins) || --db_call_depth != 0)) {
		if (db_sstep_print) {
		    if (inst_call(ins) || inst_return(ins)) {
			register int i;

			db_printf("[after %6d /%4d] ",
				  db_inst_count,
				  db_inst_count - db_last_inst_count);
			db_last_inst_count = db_inst_count;
			for (i = db_call_depth; --i > 0; )
			    db_printf("  ");
			db_print_loc_and_inst(pc, task);
			db_printf("\n");
		    }
		}
		if (inst_call(ins))
		    db_call_depth++;
		return (FALSE);	/* continue */
	    }
	}
	if (db_run_mode == STEP_CALLT) {
	    /* WARNING: the following assumes an instruction fits an int */
	    db_expr_t ins = db_get_task_value(pc, sizeof(int), FALSE, task);

	    /* continue until call or return */

	    if (!inst_call(ins) &&
		!inst_return(ins) &&
		!inst_trap_return(ins)) {
		return (FALSE);	/* continue */
	    }
	}
	if (db_find_breakpoint_here(task, pc))
		return(FALSE);
	db_run_mode = STEP_NONE;
	return (TRUE);
}

void
db_restart_at_pc(watchpt, task)
	boolean_t watchpt;
	task_t	  task;
{
	register db_addr_t pc = PC_REGS(DDB_REGS), brpc;

	if ((db_run_mode == STEP_COUNT) ||
	    (db_run_mode == STEP_RETURN) ||
	    (db_run_mode == STEP_CALLT)) {
	    db_expr_t		ins;

	    /*
	     * We are about to execute this instruction,
	     * so count it now.
	     */

	    ins = db_get_task_value(pc, sizeof(int), FALSE, task);
	    db_inst_count++;
	    db_load_count += inst_load(ins);
	    db_store_count += inst_store(ins);
#ifdef	SOFTWARE_SSTEP
	    /* Account for instructions in delay slots */
	    brpc = next_instr_address(pc,1,task);
	    if ((brpc != pc) && (inst_branch(ins) || inst_call(ins))) {
		/* Note: this ~assumes an instruction <= sizeof(int) */
		ins = db_get_task_value(brpc, sizeof(int), FALSE, task);
		db_inst_count++;
		db_load_count += inst_load(ins);
		db_store_count += inst_store(ins);
	    }
#endif	/* SOFTWARE_SSTEP */
	}

	if (db_run_mode == STEP_CONTINUE) {
	    if (watchpt || db_find_breakpoint_here(task, pc)) {
		/*
		 * Step over breakpoint/watchpoint.
		 */
		db_run_mode = STEP_INVISIBLE;
		db_set_task_single_step(DDB_REGS, task);
	    } else {
		db_set_breakpoints();
		db_set_watchpoints();
	    }
	} else {
	    db_set_task_single_step(DDB_REGS, task);
	}
}

void
db_single_step(regs, task)
	db_regs_t *regs;
	task_t	  task;
{
	if (db_run_mode == STEP_CONTINUE) {
	    db_run_mode = STEP_INVISIBLE;
	    db_set_task_single_step(regs, task);
	}
}

#ifdef	SOFTWARE_SSTEP
/*
 *	Software implementation of single-stepping.
 *	If your machine does not have a trace mode
 *	similar to the vax or sun ones you can use
 *	this implementation, done for the mips.
 *	Just define the above conditional and provide
 *	the functions/macros defined below.
 *
 * extern boolean_t
 *	inst_branch(),		returns true if the instruction might branch
 * extern unsigned
 *	branch_taken(),		return the address the instruction might
 *				branch to
 *	db_getreg_val();	return the value of a user register,
 *				as indicated in the hardware instruction
 *				encoding, e.g. 8 for r8
 *			
 * next_instr_address(pc,bd,task) returns the address of the first
 *				instruction following the one at "pc",
 *				which is either in the taken path of
 *				the branch (bd==1) or not.  This is
 *				for machines (mips) with branch delays.
 *
 *	A single-step may involve at most 2 breakpoints -
 *	one for branch-not-taken and one for branch taken.
 *	If one of these addresses does not already have a breakpoint,
 *	we allocate a breakpoint and save it here.
 *	These breakpoints are deleted on return.
 */			
db_breakpoint_t	db_not_taken_bkpt = 0;
db_breakpoint_t	db_taken_bkpt = 0;

db_breakpoint_t
db_find_temp_breakpoint(task, addr)
	task_t		   task;
	db_addr_t	   addr;
{
	if (db_taken_bkpt && (db_taken_bkpt->address == addr) &&
	    db_taken_bkpt->task == task)
		return db_taken_bkpt;
	if (db_not_taken_bkpt && (db_not_taken_bkpt->address == addr) &&
	    db_not_taken_bkpt->task == task)
		return db_not_taken_bkpt;
	return 0;
}

void
db_set_task_single_step(regs, task)
	register db_regs_t *regs;
	task_t		   task;
{
	db_addr_t pc = PC_REGS(regs), brpc;
	register unsigned int	 inst;
	register boolean_t       unconditional;

	/*
	 *	User was stopped at pc, e.g. the instruction
	 *	at pc was not executed.
	 */
	inst = db_get_task_value(pc, sizeof(int), FALSE, task);
	if (inst_branch(inst) || inst_call(inst)) {
	    extern db_expr_t getreg_val();

	    brpc = branch_taken(inst, pc, getreg_val, regs);
	    if (brpc != pc) {	/* self-branches are hopeless */
		db_taken_bkpt = db_set_temp_breakpoint(task, brpc);
	    } else
	        db_taken_bkpt = 0;
	    pc = next_instr_address(pc,1,task);
	}
	
	/* check if this control flow instruction is an unconditional transfer */
	unconditional = inst_unconditional_flow_transfer(inst);

	pc = next_instr_address(pc,0,task);
	/* 
	  We only set the sequential breakpoint if previous instruction was not
	  an unconditional change of flow of control. If the previous instruction
	  is an unconditional change of flow of control, setting a breakpoint in the
	  next sequential location may set a breakpoint in data or in another routine,
	  which could screw up either the program or the debugger. 
	  (Consider, for instance, that the next sequential instruction is the 
	  start of a routine needed by the debugger.)
	*/
	if (!unconditional && db_find_breakpoint_here(task, pc) == 0) {
	    db_not_taken_bkpt = db_set_temp_breakpoint(task, pc);
	}
	else
	    db_not_taken_bkpt = 0;
}

void
db_clear_task_single_step(regs, task)
	db_regs_t *regs;
	task_t	  task;
{
	if (db_taken_bkpt != 0) {
	    db_delete_temp_breakpoint(task, db_taken_bkpt);
	    db_taken_bkpt = 0;
	}
	if (db_not_taken_bkpt != 0) {
	    db_delete_temp_breakpoint(task, db_not_taken_bkpt);
	    db_not_taken_bkpt = 0;
	}
}

#endif	/* SOFTWARE_SSTEP */


extern int	db_cmd_loop_done;

/* single-step */
/*ARGSUSED*/
void
db_single_step_cmd(addr, have_addr, count, modif)
	db_expr_t	addr;
	int		have_addr;
	db_expr_t	count;
	char *		modif;
{
	boolean_t	print = FALSE;

	if (count == -1)
	    count = 1;

	if (modif[0] == 'p')
	    print = TRUE;

	db_run_mode = STEP_ONCE;
	db_loop_count = count;
	db_sstep_print = print;
	db_inst_count = 0;
	db_last_inst_count = 0;
	db_load_count = 0;
	db_store_count = 0;

	db_cmd_loop_done = 1;
}

/* trace and print until call/return */
/*ARGSUSED*/
void
db_trace_until_call_cmd(addr, have_addr, count, modif)
	db_expr_t	addr;
	int		have_addr;
	db_expr_t	count;
	char *		modif;
{
	boolean_t	print = FALSE;

	if (modif[0] == 'p')
	    print = TRUE;

	db_run_mode = STEP_CALLT;
	db_sstep_print = print;
	db_inst_count = 0;
	db_last_inst_count = 0;
	db_load_count = 0;
	db_store_count = 0;

	db_cmd_loop_done = 1;
}

/*ARGSUSED*/
void
db_trace_until_matching_cmd(addr, have_addr, count, modif)
	db_expr_t	addr;
	int		have_addr;
	db_expr_t	count;
	char *		modif;
{
	boolean_t	print = FALSE;

	if (modif[0] == 'p')
	    print = TRUE;

	db_run_mode = STEP_RETURN;
	db_call_depth = 1;
	db_sstep_print = print;
	db_inst_count = 0;
	db_last_inst_count = 0;
	db_load_count = 0;
	db_store_count = 0;

	db_cmd_loop_done = 1;
}

/* continue */
/*ARGSUSED*/
void
db_continue_cmd(addr, have_addr, count, modif)
	db_expr_t	addr;
	int		have_addr;
	db_expr_t	count;
	char *		modif;
{
	if (modif[0] == 'c')
	    db_run_mode = STEP_COUNT;
	else
	    db_run_mode = STEP_CONTINUE;
	db_inst_count = 0;
	db_last_inst_count = 0;
	db_load_count = 0;
	db_store_count = 0;

	db_cmd_loop_done = 1;
}

boolean_t
db_in_single_step()
{
	return(db_run_mode != STEP_NONE && db_run_mode != STEP_CONTINUE);
}
