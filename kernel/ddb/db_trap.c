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
 * $Log:	db_trap.c,v $
 * Revision 2.11  93/01/14  17:26:00  danner
 * 	64bit cleanup.
 * 	[92/12/10  16:22:07  af]
 * 
 * Revision 2.10  92/08/03  17:32:15  jfriedl
 * 	removed silly prototypes
 * 	[92/08/02            jfriedl]
 * 
 * Revision 2.9  92/05/21  17:08:04  jfriedl
 * 	tried prototypes.
 * 	[92/05/20            jfriedl]
 * 
 * Revision 2.8  91/10/09  16:03:34  af
 * 	Replaced db_trap function with db_task_trap, and changed
 * 	 db_trap to call db_task_trap with user_space parameter.
 * 	 db_trap exists only for backward compatibility.
 * 	Passed task parameter for user space break point support.
 * 	[91/08/29            tak]
 * 
 * Revision 2.7  91/05/18  14:29:20  rpd
 * 	Protect against faults in db_print_loc_and_inst.
 * 	[91/05/14            rpd]
 * 
 * Revision 2.6  91/05/14  15:36:39  mrt
 * 	Correcting copyright
 * 
 * Revision 2.5  91/02/05  17:07:16  mrt
 * 	Changed to new Mach copyright
 * 	[91/01/31  16:19:35  mrt]
 * 
 * Revision 2.4  91/01/08  15:09:17  rpd
 * 	Changed db_stop_at_pc's arguments.
 * 	Print db_inst_count, db_load_count, db_store_count.
 * 	[90/11/27            rpd]
 * 
 * Revision 2.3  90/10/25  14:44:11  rwd
 * 	From rpd.
 * 	[90/10/19  17:03:17  rwd]
 * 
 * 	Generalized the watchpoint support.
 * 	[90/10/16            rwd]
 * 	Added watchpoint support.
 * 	[90/10/16            rpd]
 * 
 * Revision 2.2  90/08/27  21:52:52  dbg
 * 	Assign to db_dot before calling the print function.
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
 * Trap entry point to kernel debugger.
 */
#include <mach/boolean.h>
#include <machine/db_machdep.h>
#include <ddb/db_command.h>
#include <ddb/db_access.h>
#include <ddb/db_break.h>
#include <ddb/db_task_thread.h>



extern jmp_buf_t *db_recover;

extern void		db_restart_at_pc();
extern boolean_t	db_stop_at_pc();

extern int		db_inst_count;
extern int		db_load_count;
extern int		db_store_count;

void
db_task_trap(type, code, user_space)
	int	  type, code;
	boolean_t user_space;
{
	jmp_buf_t db_jmpbuf;
	jmp_buf_t *prev;
	boolean_t	bkpt;
	boolean_t	watchpt;
	void		db_init_default_thread();
	void		db_check_breakpoint_valid();
	task_t		task_space;

	task_space = db_target_space(current_thread(), user_space);
	bkpt = IS_BREAKPOINT_TRAP(type, code);
	watchpt = IS_WATCHPOINT_TRAP(type, code);

	db_init_default_thread();
	db_check_breakpoint_valid();
	if (db_stop_at_pc(&bkpt, task_space)) {
	    if (db_inst_count) {
		db_printf("After %d instructions (%d loads, %d stores),\n",
			  db_inst_count, db_load_count, db_store_count);
	    }
	    if (bkpt)
		db_printf("Breakpoint at  ");
	    else if (watchpt)
		db_printf("Watchpoint at  ");
	    else
		db_printf("Stopped at  ");
	    db_dot = PC_REGS(DDB_REGS);

	    prev = db_recover;
	    if (_setjmp(db_recover = &db_jmpbuf) == 0)
		db_print_loc_and_inst(db_dot, task_space);
	    else
		db_printf("Trouble printing location %#X.\n", db_dot);
	    db_recover = prev;

	    db_command_loop();
	}

	db_restart_at_pc(watchpt, task_space);
}

void
db_trap(type, code)
	int	type, code;
{
	db_task_trap(type, code, !DB_VALID_KERN_ADDR(PC_REGS(DDB_REGS)));
}
