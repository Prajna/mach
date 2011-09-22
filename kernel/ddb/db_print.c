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
 * $Log:	db_print.c,v $
 * Revision 2.19  93/03/09  10:53:44  danner
 * 	String protos.
 * 	[93/03/07            af]
 * 
 * Revision 2.18  93/01/14  17:25:33  danner
 * 	64bit cleanup.
 * 	[92/11/30            af]
 * 
 * Revision 2.17  92/08/05  18:02:39  jfriedl
 * 	Removed silly prototypes.
 * 
 * Revision 2.16  92/05/21  17:07:35  jfriedl
 * 	Removed unused variable from db_show_regs().
 * 	[92/05/16            jfriedl]
 * 
 * Revision 2.15  92/05/04  11:24:08  danner
 * 	Converted some db_printsyms to db_task_printsyms.
 * 	[92/04/10            danner]
 * 
 * Revision 2.14  92/04/01  19:31:37  rpd
 * 	Changed db_print_thread so that both display formats
 * 	show the floating-point-used status of the thread.
 * 	[92/03/16            rpd]
 * 
 * Revision 2.13  92/02/20  18:34:28  elf
 * 	Fixed typo.
 * 	[92/02/20            elf]
 * 
 * Revision 2.12  92/02/19  15:07:47  elf
 * 	Added db_thread_fp_used, to avoid machine-dependent conditionals.
 * 	[92/02/19            rpd]
 * 
 * 	Added 'F' flag to db_thread_stat showing if the thread has a valid
 * 	FPU context. Tested on i386 and pmax.
 * 	[92/02/17            kivinen]
 * 
 * Revision 2.11  91/11/12  11:50:32  rvb
 * 	Added OPTION_USER ("/u") to db_show_all_threads, db_show_one_thread,
 * 	db_show_one_task.  Without it, we display old-style information.
 * 	[91/10/31            rpd]
 * 
 * Revision 2.10  91/10/09  16:01:48  af
 * 	Supported "show registers" for non current thread.
 * 	Changed display format of thread and task information.
 * 	Changed "show thread" to print current thread information 
 * 	  if no thread is specified.
 * 	Added "show_one_task" for "show task" command.
 * 	Added IPC port print routines for "show ipc_port" command.
 * 	[91/08/29            tak]
 * 
 * Revision 2.9  91/08/03  18:17:19  jsb
 * 	In db_print_thread, if the thread is swapped and there is a
 * 	continuation function, print the function name in parentheses
 * 	instead of '(swapped)'.
 * 	[91/07/04  09:59:27  jsb]
 * 
 * Revision 2.8  91/07/31  17:30:43  dbg
 * 	Revise scheduling state machine.
 * 	[91/07/30  16:43:42  dbg]
 * 
 * Revision 2.7  91/07/09  23:15:57  danner
 * 	Fixed a few printf that should be db_printfs. 
 * 	[91/07/08            danner]
 * 
 * Revision 2.6  91/05/14  15:35:25  mrt
 * 	Correcting copyright
 * 
 * Revision 2.5  91/02/05  17:06:53  mrt
 * 	Changed to new Mach copyright
 * 	[91/01/31  16:18:56  mrt]
 * 
 * Revision 2.4  90/10/25  14:43:54  rwd
 * 	Changed db_show_regs to print unsigned.
 * 	[90/10/19            rpd]
 * 	Generalized the watchpoint support.
 * 	[90/10/16            rwd]
 * 
 * Revision 2.3  90/09/09  23:19:52  rpd
 * 	Avoid totally incorrect guesses of symbol names for small values.
 * 	[90/08/30  17:39:08  af]
 * 
 * Revision 2.2  90/08/27  21:51:49  dbg
 * 	Insist that 'show thread' be called with an explicit address.
 * 	[90/08/22            dbg]
 * 
 * 	Fix type for db_maxoff.
 * 	[90/08/20            dbg]
 * 
 * 	Do not dereference the "valuep" field of a variable directly,
 * 	call the new db_read/write_variable functions instead.
 * 	Reflected changes in symbol lookup functions.
 * 	[90/08/20            af]
 * 	Reduce lint.
 * 	[90/08/10  14:33:44  dbg]
 * 
 * 	Created.
 * 	[90/07/25            dbg]
 * 
 */
/*
 * 	Author: David B. Golub, Carnegie Mellon University
 *	Date:	7/90
 */

/*
 * Miscellaneous printing.
 */
#include <mach/port.h>
#include <kern/strings.h>
#include <kern/task.h>
#include <kern/thread.h>
#include <kern/queue.h>
#include <ipc/ipc_port.h>
#include <ipc/ipc_space.h>

#include <machine/db_machdep.h>
#include <machine/thread.h>

#include <ddb/db_lex.h>
#include <ddb/db_variables.h>
#include <ddb/db_sym.h>
#include <ddb/db_task_thread.h>

extern unsigned int	db_maxoff;

/* ARGSUSED */
void
db_show_regs(addr, have_addr, count, modif)
	db_expr_t	addr;
	boolean_t	have_addr;
	db_expr_t	count;
	char		*modif;
{
	register struct db_variable *regp;
	db_expr_t	value;
	db_addr_t	offset;
	char *		name;
	register	i;
	struct db_var_aux_param aux_param;
	task_t		task = TASK_NULL;

	aux_param.modif = modif;
	aux_param.thread = THREAD_NULL;
	if (db_option(modif, 't')) {
	    if (have_addr) {
		if (!db_check_thread_address_valid((thread_t)addr))
		    return;
		aux_param.thread = (thread_t)addr;
	    } else
	        aux_param.thread = db_default_thread;
	    if (aux_param.thread != THREAD_NULL)
		task = aux_param.thread->task;
	}
	for (regp = db_regs; regp < db_eregs; regp++) {
	    if (regp->max_level > 1) {
		db_printf("bad multi-suffixed register %s\n", regp->name);
		continue;
	    }
	    aux_param.level = regp->max_level;
	    for (i = regp->low; i <= regp->high; i++) {
		aux_param.suffix[0] = i;
	        db_read_write_variable(regp, &value, DB_VAR_GET, &aux_param);
		if (regp->max_level > 0)
		    db_printf("%s%d%*s", regp->name, i, 
				12-strlen(regp->name)-((i<10)?1:2), "");
		else
		    db_printf("%-12s", regp->name);
		db_printf("%#*N", 2+2*sizeof(vm_offset_t), value);
		db_find_xtrn_task_sym_and_offset((db_addr_t)value, &name, 
							&offset, task);
		if (name != 0 && offset <= db_maxoff && offset != value) {
		    db_printf("\t%s", name);
		    if (offset != 0)
			db_printf("+%#r", offset);
	    	}
		db_printf("\n");
	    }
	}
}

#define OPTION_LONG		0x001		/* long print option */
#define OPTION_USER		0x002		/* print ps-like stuff */
#define OPTION_INDENT		0x100		/* print with indent */
#define OPTION_THREAD_TITLE	0x200		/* print thread title */
#define OPTION_TASK_TITLE	0x400		/* print thread title */

#ifndef	DB_TASK_NAME
#define DB_TASK_NAME(task)			/* no task name */
#define DB_TASK_NAME_TITLE	""		/* no task name */
#endif	DB_TASK_NAME

#ifndef	db_thread_fp_used
#define db_thread_fp_used(thread)	FALSE
#endif

char *
db_thread_stat(thread, status)
	register thread_t thread;
	char	 *status;
{
	register char *p = status;
	
	*p++ = (thread->state & TH_RUN)  ? 'R' : '.';
	*p++ = (thread->state & TH_WAIT) ? 'W' : '.';
	*p++ = (thread->state & TH_SUSP) ? 'S' : '.';
	*p++ = (thread->state & TH_SWAPPED) ? 'O' : '.';
	*p++ = (thread->state & TH_UNINT) ? 'N' : '.';
	/* show if the FPU has been used */
	*p++ = db_thread_fp_used(thread) ? 'F' : '.';
	*p++ = 0;
	return(status);
}

void
db_print_thread(thread, thread_id, flag)
	thread_t thread;
	int	 thread_id;
	int	 flag;
{
	if (flag & OPTION_USER) {
	    char status[8];
	    char *indent = "";

	    if (flag & OPTION_LONG) {
		if (flag & OPTION_INDENT)
		    indent = "    ";
		if (flag & OPTION_THREAD_TITLE) {
		    db_printf("%s ID: THREAD   STAT   STACK    PCB", indent);
		    db_printf("      SUS PRI CONTINUE,WAIT_FUNC\n");
		}
		db_printf("%s%3d%c %0*X %s %0*X %0*X %3d %3d ",
		    indent, thread_id,
		    (thread == current_thread())? '#': ':',
		    2*sizeof(vm_offset_t), thread,
		    db_thread_stat(thread, status),
		    2*sizeof(vm_offset_t), thread->kernel_stack,
		    2*sizeof(vm_offset_t), thread->pcb,
		    thread->suspend_count, thread->sched_pri);
		if ((thread->state & TH_SWAPPED) && thread->swap_func) {
		    db_task_printsym((db_addr_t)thread->swap_func,
				     DB_STGY_ANY, kernel_task);
		    db_printf(", ");
		}
		if (thread->state & TH_WAIT)
		    db_task_printsym((db_addr_t)thread->wait_event,
				     DB_STGY_ANY, kernel_task);
		db_printf("\n");
	    } else {
		if (thread_id % 3 == 0) {
		    if (flag & OPTION_INDENT)
			db_printf("\n    ");
		} else
		    db_printf(" ");
		db_printf("%3d%c(%0*X,%s)", thread_id, 
		    (thread == current_thread())? '#': ':',
		    2*sizeof(vm_offset_t), thread,
		    db_thread_stat(thread, status));
	    }
	} else {
	    if (flag & OPTION_INDENT)
		db_printf("            %3d (%0*X) ", thread_id,
			  2*sizeof(vm_offset_t), thread);
	    else
		db_printf("(%0*X) ", 2*sizeof(vm_offset_t), thread);
	    db_printf("%c%c%c%c%c",
		      (thread->state & TH_RUN)  ? 'R' : ' ',
		      (thread->state & TH_WAIT) ? 'W' : ' ',
		      (thread->state & TH_SUSP) ? 'S' : ' ',
		      (thread->state & TH_UNINT)? 'N' : ' ',
		      db_thread_fp_used(thread) ? 'F' : ' ');
	    if (thread->state & TH_SWAPPED) {
		if (thread->swap_func) {
		    db_printf("(");
		    db_task_printsym((db_addr_t)thread->swap_func, 
				     DB_STGY_ANY, kernel_task);
		    db_printf(")");
		} else {
		    db_printf("(swapped)");
		}
	    }
	    if (thread->state & TH_WAIT) {
		db_printf(" ");
		db_task_printsym((db_addr_t)thread->wait_event, 
			    DB_STGY_ANY, kernel_task);
	    }
	    db_printf("\n");
	}
}

void
db_print_task(task, task_id, flag)
	task_t	task;
	int	task_id;
	int	flag;
{
	thread_t thread;
	int thread_id;

	if (flag & OPTION_USER) {
	    if (flag & OPTION_TASK_TITLE) {
		db_printf(" ID: TASK     MAP      THD SUS PR %s", 
			  DB_TASK_NAME_TITLE);
		if ((flag & OPTION_LONG) == 0)
		    db_printf("  THREADS");
		db_printf("\n");
	    }
	    db_printf("%3d: %0*X %0*X %3d %3d %2d ",
			    task_id, 2*sizeof(vm_offset_t), task,
			    2*sizeof(vm_offset_t), task->map, task->thread_count,
			    task->suspend_count, task->priority);
	    DB_TASK_NAME(task);
	    if (flag & OPTION_LONG) {
		if (flag & OPTION_TASK_TITLE)
		    flag |= OPTION_THREAD_TITLE;
		db_printf("\n");
	    } else if (task->thread_count <= 1)
		flag &= ~OPTION_INDENT;
	    thread_id = 0;
	    queue_iterate(&task->thread_list, thread, thread_t, thread_list) {
		db_print_thread(thread, thread_id, flag);
		flag &= ~OPTION_THREAD_TITLE;
		thread_id++;
	    }
	    if ((flag & OPTION_LONG) == 0)
		db_printf("\n");
	} else {
	    if (flag & OPTION_TASK_TITLE)
		db_printf("    TASK        THREADS\n");
	    db_printf("%3d (%0*X): ", task_id, 2*sizeof(vm_offset_t), task);
	    if (task->thread_count == 0) {
		db_printf("no threads\n");
	    } else {
		if (task->thread_count > 1) {
		    db_printf("%d threads: \n", task->thread_count);
		    flag |= OPTION_INDENT;
		} else
		    flag &= ~OPTION_INDENT;
		thread_id = 0;
		queue_iterate(&task->thread_list, thread,
			      thread_t, thread_list)
		    db_print_thread(thread, thread_id++, flag);
	    }
	}
}

/*ARGSUSED*/
void
db_show_all_threads(addr, have_addr, count, modif)
	db_expr_t	addr;
	boolean_t	have_addr;
	db_expr_t	count;
	char *		modif;
{
	task_t task;
	int task_id;
	int flag;
	processor_set_t pset;

	flag = OPTION_TASK_TITLE|OPTION_INDENT;
	if (db_option(modif, 'u'))
	    flag |= OPTION_USER;
	if (db_option(modif, 'l'))
	    flag |= OPTION_LONG;

	task_id = 0;
	queue_iterate(&all_psets, pset, processor_set_t, all_psets) {
	    queue_iterate(&pset->tasks, task, task_t, pset_tasks) {
		db_print_task(task, task_id, flag);
		flag &= ~OPTION_TASK_TITLE;
		task_id++;
	    }
	}
}

/*ARGSUSED*/
void
db_show_one_thread(addr, have_addr, count, modif)
	db_expr_t	addr;
	boolean_t	have_addr;
	db_expr_t	count;
	char *		modif;
{
	int		flag;
	int		thread_id;
	thread_t	thread;

	flag = OPTION_THREAD_TITLE;
	if (db_option(modif, 'u'))
	    flag |= OPTION_USER;
	if (db_option(modif, 'l'))
	    flag |= OPTION_LONG;

	if (!have_addr) {
	    thread = current_thread();
	    if (thread == THREAD_NULL) {
		db_error("No thread\n");
		/*NOTREACHED*/
	    }
	} else
	    thread = (thread_t) addr;

	if ((thread_id = db_lookup_thread(thread)) < 0) {
	    db_printf("bad thread address %#X\n", addr);
	    db_error(0);
	    /*NOTREACHED*/
	}

	if (flag & OPTION_USER) {
	    db_printf("TASK%d(%0*X):\n",
		      db_lookup_task(thread->task),
		      2*sizeof(vm_offset_t), thread->task);
	    db_print_thread(thread, thread_id, flag);
	} else {
	    db_printf("task %d(%0*X): thread %d",
		      db_lookup_task(thread->task),
		      2*sizeof(vm_offset_t), thread->task, thread_id);
	    db_print_thread(thread, thread_id, flag);
	}
}

/*ARGSUSED*/
void
db_show_one_task(addr, have_addr, count, modif)
	db_expr_t	addr;
	boolean_t	have_addr;
	db_expr_t	count;
	char *		modif;
{
	int		flag;
	int		task_id;
	task_t		task;

	flag = OPTION_TASK_TITLE;
	if (db_option(modif, 'u'))
	    flag |= OPTION_USER;
	if (db_option(modif, 'l'))
	    flag |= OPTION_LONG;

	if (!have_addr) {
	    task = db_current_task();
	    if (task == TASK_NULL) {
		db_error("No task\n");
		/*NOTREACHED*/
	    }
	} else
	    task = (task_t) addr;

	if ((task_id = db_lookup_task(task)) < 0) {
	    db_printf("bad task address %#X\n", addr);
	    db_error(0);
	    /*NOTREACHED*/
	}

	db_print_task(task, task_id, flag);
}

int
db_port_iterate(thread, func)
	thread_t thread;
	void (*func)();
{
	ipc_entry_t entry;
	int index;
	int n = 0;
	int size;
	ipc_space_t space;

	space = thread->task->itk_space;
	entry = space->is_table;
	size = space->is_table_size;
	for (index = 0; index < size; index++, entry++) {
	    if (entry->ie_bits & MACH_PORT_TYPE_PORT_RIGHTS)
		(*func)(index, (ipc_port_t) entry->ie_object,
			entry->ie_bits, n++);
	}
	return(n);
}

ipc_port_t
db_lookup_port(thread, id)
	thread_t thread;
	int id;
{
	register ipc_space_t space;
	register ipc_entry_t entry;

	if (thread == THREAD_NULL)
	    return(0);
	space = thread->task->itk_space;
	if (id < 0 || id >= space->is_table_size)
	    return(0);
	entry = &space->is_table[id];
	if (entry->ie_bits & MACH_PORT_TYPE_PORT_RIGHTS)
	    return((ipc_port_t)entry->ie_object);
	return(0);
}

static void
db_print_port_id(id, port, bits, n)
	int id;
	ipc_port_t port;
	unsigned bits;
	int n;
{
	if (n != 0 && n % 3 == 0)
	    db_printf("\n");
	db_printf("\tport%d(%s,%x)", id,
		(bits & MACH_PORT_TYPE_RECEIVE)? "r":
		(bits & MACH_PORT_TYPE_SEND)? "s": "S", port);
}

/* ARGSUSED */
void
db_show_port_id(addr, have_addr, count, modif)
	db_expr_t	addr;
	boolean_t	have_addr;
	db_expr_t	count;
	char *		modif;
{
	thread_t thread;

	if (!have_addr) {
	    thread = current_thread();
	    if (thread == THREAD_NULL) {
		db_error("No thread\n");
		/*NOTREACHED*/
	    }
	} else
	    thread = (thread_t) addr;
	if (db_lookup_thread(thread) < 0) {
	    db_printf("Bad thread address %#X\n", addr);
	    db_error(0);
	    /*NOTREACHED*/
	}
	if (db_port_iterate(thread, db_print_port_id))
	    db_printf("\n");
}
