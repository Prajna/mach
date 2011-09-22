/* 
 * Mach Operating System
 * Copyright (c) 1991 Carnegie Mellon University
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
 * $Log:	db_command.c,v $
 * Revision 2.22  93/03/09  10:53:31  danner
 * 	String protos.
 * 	[93/03/07            af]
 * 
 * Revision 2.21  93/02/01  09:55:01  danner
 * 	Removed unused (and incorrect) decl of db_machine_cmds.
 * 	[93/01/25            jfriedl]
 * 
 * Revision 2.20  93/01/14  17:24:43  danner
 * 	64bit cleanup.
 * 	[92/11/30            af]
 * 
 * Revision 2.19  92/08/03  17:30:46  jfriedl
 * 	removed silly prototypes
 * 	[92/08/02            jfriedl]
 * 
 * Revision 2.18  92/05/21  17:06:32  jfriedl
 * 	Cleanup to quiet gcc warnings.
 * 	[92/05/16            jfriedl]
 * 
 * Revision 2.17  92/03/10  16:25:07  jsb
 * 	Added show pset, show all vuids.
 * 	[92/02/20  10:52:22  jsb]
 * 	NORMA_VM: added show xmm_obj, show xmm_reply.
 * 	[92/01/21  18:13:32  jsb]
 * 
 * 	NORMA_IPC: added show all uids, proxies, principals.
 * 	[92/01/16  21:25:38  jsb]
 * 
 * 	Added show copy, show packet, show pcs.
 * 	[92/01/13  10:13:50  jsb]
 * 
 * Revision 2.16  91/10/09  15:58:27  af
 * 	Revision 2.15.1.1  91/10/05  13:05:19  jeffreyh
 * 	Added compound, macro, and conditional command support.
 * 	Added "xf" and "xb" command to examine forward and backward.
 * 	Added last command execution with "!!".
 * 	Added "show task" command.
 * 	Added "ipc_port" command to print all IPC ports in a task.
 * 	Added dot address and default thread indicator in prompt.
 * 	Changed error messages to output more information.
 * 	Moved "skip_to_eol()" to db_lex.c.
 * 	[91/08/29            tak]
 * 
 * Revision 2.15  91/08/24  11:55:24  af
 * 	Added optional funcall at ddb entry: similar to GDB's
 * 	"display"s in spirit.
 * 	[91/08/02  02:42:11  af]
 * 
 * Revision 2.14  91/08/03  18:17:16  jsb
 * 	Added `show kmsg' and `show msg'.
 * 	Replaced NORMA_BOOT conditionals with NORMA_IPC.
 * 	Use _node_self instead of node_self() for prompt,
 * 	since node_self() will panic if _node_self not intialized.
 * 	[91/07/24  22:48:48  jsb]
 * 
 * Revision 2.13  91/07/11  11:00:32  danner
 * 	Copyright Fixes
 * 
 * Revision 2.12  91/07/09  23:15:42  danner
 * 	Modified the command loop to include the cpu number in the
 * 	 command prompt on multicpu machines, and node number in norma
 * 	      systems.
 * 	[91/04/12            danner]
 *
 * Revision 2.11  91/07/01  08:24:04  jsb
 * 	Added support for 'show all slocks'.
 * 	[91/06/29  15:59:01  jsb]
 * 
 * Revision 2.10  91/06/17  15:43:46  jsb
 * 	Renamed NORMA conditionals.
 * 	[91/06/17  09:57:51  jsb]
 * 
 * 	Moved struct command definition to db_command.h, added include of
 * 	 db_command.h, renamed struct command to struct db_command. All
 * 	 in support of machine dependpent command extensions to db. 
 * 	[91/03/12            danner]
 * 
 * Revision 2.9  91/06/06  17:03:47  jsb
 * 	NORMA support: prompt includes node number, e.g., db4>.
 * 	[91/05/25  10:47:35  jsb]
 * 
 * Revision 2.8  91/05/14  15:32:51  mrt
 * 	Correcting copyright
 * 
 * Revision 2.7  91/03/16  14:42:30  rpd
 * 	Added db_recover.
 * 	[91/01/13            rpd]
 * 
 * Revision 2.6  91/02/05  17:06:10  mrt
 * 	Changed to new Mach copyright
 * 	[91/01/31  16:17:18  mrt]
 * 
 * Revision 2.5  91/01/08  17:31:54  rpd
 * 	Forward reference for db_fncall();
 * 	[91/01/04  12:35:17  rvb]
 * 
 * 	Add call as a synonym for ! and match for next
 * 	[91/01/04  12:14:48  rvb]
 * 
 * Revision 2.4  90/11/07  16:49:15  rpd
 * 	Added search.
 * 	[90/11/06            rpd]
 * 
 * Revision 2.3  90/10/25  14:43:45  rwd
 * 	Changed db_fncall to print the result unsigned.
 * 	[90/10/19            rpd]
 * 
 * 	Added CS_MORE to db_watchpoint_cmd.
 * 	[90/10/17            rpd]
 * 	Added watchpoint commands: watch, dwatch, show watches.
 * 	[90/10/16            rpd]
 * 
 * Revision 2.2  90/08/27  21:50:10  dbg
 * 	Remove 'listbreaks' - use 'show breaks' instead.  Change 'show
 * 	threads' to 'show all threads' to avoid clash with 'show thread'.
 * 	Set 'dot' here from db_prev or db_next, depending on 'db_ed_style'
 * 	flag and syntax table.
 * 	[90/08/22            dbg]
 * 	Reduce lint.
 * 	[90/08/07            dbg]
 * 	Created.
 * 	[90/07/25            dbg]
 * 
 */
/*
 *	Author: David B. Golub, Carnegie Mellon University
 *	Date:	7/90
 */

/*
 * Command dispatcher.
 */
#include <cpus.h>
#include <norma_ipc.h>
#include <norma_vm.h>

#include <mach/boolean.h>
#include <kern/strings.h>
#include <machine/db_machdep.h>

#include <ddb/db_lex.h>
#include <ddb/db_output.h>
#include <ddb/db_command.h>
#include <ddb/db_task_thread.h>

#include <machine/setjmp.h>
#include <kern/thread.h>
#include <ipc/ipc_pset.h> /* 4proto */
#include <ipc/ipc_port.h> /* 4proto */



/*
 * Exported global variables
 */
boolean_t	db_cmd_loop_done;
jmp_buf_t	*db_recover = 0;
db_addr_t	db_dot;
db_addr_t	db_last_addr;
db_addr_t	db_prev;
db_addr_t	db_next;

/*
 * if 'ed' style: 'dot' is set at start of last item printed,
 * and '+' points to next line.
 * Otherwise: 'dot' points to next item, '..' points to last.
 */
boolean_t	db_ed_style = TRUE;

/*
 * Results of command search.
 */
#define	CMD_UNIQUE	0
#define	CMD_FOUND	1
#define	CMD_NONE	2
#define	CMD_AMBIGUOUS	3
#define	CMD_HELP	4

/*
 * Search for command prefix.
 */
int
db_cmd_search(name, table, cmdp)
	char *		name;
	struct db_command	*table;
	struct db_command	**cmdp;	/* out */
{
	struct db_command	*cmd;
	int		result = CMD_NONE;

	for (cmd = table; cmd->name != 0; cmd++) {
	    register char *lp;
	    register char *rp;
	    register int  c;

	    lp = name;
	    rp = cmd->name;
	    while ((c = *lp) == *rp) {
		if (c == 0) {
		    /* complete match */
		    *cmdp = cmd;
		    return (CMD_UNIQUE);
		}
		lp++;
		rp++;
	    }
	    if (c == 0) {
		/* end of name, not end of command -
		   partial match */
		if (result == CMD_FOUND) {
		    result = CMD_AMBIGUOUS;
		    /* but keep looking for a full match -
		       this lets us match single letters */
		}
		else {
		    *cmdp = cmd;
		    result = CMD_FOUND;
		}
	    }
	}
	if (result == CMD_NONE) {
	    /* check for 'help' */
	    if (!strncmp(name, "help", strlen(name)))
		result = CMD_HELP;
	}
	return (result);
}

void
db_cmd_list(table)
	struct db_command *table;
{
	register struct db_command *cmd;

	for (cmd = table; cmd->name != 0; cmd++) {
	    db_printf("%-12s", cmd->name);
	    db_end_line();
	}
}

void
db_command(last_cmdp, cmd_table)
	struct db_command	**last_cmdp;	/* IN_OUT */
	struct db_command	*cmd_table;
{
	struct db_command	*cmd;
	int		t;
	char		modif[TOK_STRING_SIZE];
	db_expr_t	addr, count;
	boolean_t	have_addr = FALSE;
	int		result;

	t = db_read_token();
	if (t == tEOL || t == tSEMI_COLON) {
	    /* empty line repeats last command, at 'next' */
	    cmd = *last_cmdp;
	    addr = (db_expr_t)db_next;
	    have_addr = FALSE;
	    count = 1;
	    modif[0]  = '\0';
	    if (t == tSEMI_COLON)
		db_unread_token(t);
	}
	else if (t == tEXCL) {
	    void db_fncall();
	    db_fncall();
	    return;
	}
	else if (t != tIDENT) {
	    db_printf("?\n");
	    db_flush_lex();
	    return;
	}
	else {
	    /*
	     * Search for command
	     */
	    while (cmd_table) {
		result = db_cmd_search(db_tok_string,
				       cmd_table,
				       &cmd);
		switch (result) {
		    case CMD_NONE:
			if (db_exec_macro(db_tok_string) == 0)
			    return;
			db_printf("No such command \"%s\"\n", db_tok_string);
			db_flush_lex();
			return;
		    case CMD_AMBIGUOUS:
			db_printf("Ambiguous\n");
			db_flush_lex();
			return;
		    case CMD_HELP:
			db_cmd_list(cmd_table);
			db_flush_lex();
			return;
		    default:
			break;
		}
		if ((cmd_table = cmd->more) != 0) {
		    t = db_read_token();
		    if (t != tIDENT) {
			db_cmd_list(cmd_table);
			db_flush_lex();
			return;
		    }
		}
	    }

	    if ((cmd->flag & CS_OWN) == 0) {
		/*
		 * Standard syntax:
		 * command [/modifier] [addr] [,count]
		 */
		t = db_read_token();
		if (t == tSLASH) {
		    t = db_read_token();
		    if (t != tIDENT) {
			db_printf("Bad modifier \"/%s\"\n", db_tok_string);
			db_flush_lex();
			return;
		    }
		    db_strcpy(modif, db_tok_string);
		}
		else {
		    db_unread_token(t);
		    modif[0] = '\0';
		}

		if (db_expression(&addr)) {
		    db_dot = (db_addr_t) addr;
		    db_last_addr = db_dot;
		    have_addr = TRUE;
		}
		else {
		    addr = (db_expr_t) db_dot;
		    have_addr = FALSE;
		}
		t = db_read_token();
		if (t == tCOMMA) {
		    if (!db_expression(&count)) {
			db_printf("Count missing after ','\n");
			db_flush_lex();
			return;
		    }
		}
		else {
		    db_unread_token(t);
		    count = -1;
		}
	    }
	}
	*last_cmdp = cmd;
	if (cmd != 0) {
	    /*
	     * Execute the command.
	     */
	    (*cmd->fcn)(addr, have_addr, count, modif);

	    if (cmd->flag & CS_SET_DOT) {
		/*
		 * If command changes dot, set dot to
		 * previous address displayed (if 'ed' style).
		 */
		if (db_ed_style) {
		    db_dot = db_prev;
		}
		else {
		    db_dot = db_next;
		}
	    }
	    else {
		/*
		 * If command does not change dot,
		 * set 'next' location to be the same.
		 */
		db_next = db_dot;
	    }
	}
}

void
db_command_list(last_cmdp, cmd_table)
	struct db_command	**last_cmdp;	/* IN_OUT */
	struct db_command	*cmd_table;
{
	void db_skip_to_eol();

	do {
	    db_command(last_cmdp, cmd_table);
	    db_skip_to_eol();
	} while (db_read_token() == tSEMI_COLON && db_cmd_loop_done == 0);
}

/*
 * 'show' commands
 */
extern void	db_listbreak_cmd();
extern void	db_listwatch_cmd();
extern void	db_show_regs(), db_show_one_thread(), db_show_one_task();
extern void	db_show_all_threads();
extern void	db_show_macro();
extern void	vm_map_print(), vm_object_print(), vm_page_print();
extern void	vm_map_copy_print();
extern void	ipc_port_print(), ipc_pset_print(), db_show_all_slocks();
extern void	ipc_kmsg_print(), ipc_msg_print();
extern void	db_show_port_id();
void		db_show_help();
#if	NORMA_IPC
extern void	netipc_packet_print(), netipc_pcs_print(), db_show_all_uids();
extern void	db_show_all_proxies(), db_show_all_principals();
extern void	db_show_all_uids_verbose();
#endif	NORMA_IPC
#if	NORMA_VM
extern void	xmm_obj_print(), xmm_reply_print();
#endif	NORMA_VM

struct db_command db_show_all_cmds[] = {
	{ "threads",	db_show_all_threads,	0,	0 },
	{ "slocks",	db_show_all_slocks,	0,	0 },
#if	NORMA_IPC
	{ "uids",	db_show_all_uids,	0,	0 },
	{ "proxies",	db_show_all_proxies,	0,	0 },
	{ "principals",	db_show_all_principals,	0,	0 },
	{ "vuids",	db_show_all_uids_verbose, 0,	0 },
#endif	NORMA_IPC
	{ (char *)0 }
};

struct db_command db_show_cmds[] = {
	{ "all",	0,			0,	db_show_all_cmds },
	{ "registers",	db_show_regs,		0,	0 },
	{ "breaks",	db_listbreak_cmd, 	0,	0 },
	{ "watches",	db_listwatch_cmd, 	0,	0 },
	{ "thread",	db_show_one_thread,	0,	0 },
	{ "task",	db_show_one_task,	0,	0 },
	{ "macro",	db_show_macro,		CS_OWN, 0 },
	{ "map",	vm_map_print,		0,	0 },
	{ "object",	vm_object_print,	0,	0 },
	{ "page",	vm_page_print,		0,	0 },
	{ "copy",	vm_map_copy_print,	0,	0 },
	{ "port",	ipc_port_print,		0,	0 },
	{ "pset",	ipc_pset_print,		0,	0 },
	{ "kmsg",	ipc_kmsg_print,		0,	0 },
	{ "msg",	ipc_msg_print,		0,	0 },
	{ "ipc_port",	db_show_port_id,	0,	0 },
#if	NORMA_IPC
	{ "packet",	netipc_packet_print,	0,	0 },
	{ "pcs",	netipc_pcs_print,	0,	0 },
#endif	NORMA_IPC
#if	NORMA_VM
	{ "xmm_obj",	xmm_obj_print,		0,	0 },
	{ "xmm_reply",	xmm_reply_print,	0,	0 },
#endif	NORMA_VM
	{ (char *)0, }
};

extern void	db_print_cmd(), db_examine_cmd(), db_set_cmd();
extern void	db_examine_forward(), db_examine_backward();
extern void	db_search_cmd();
extern void	db_write_cmd();
extern void	db_delete_cmd(), db_breakpoint_cmd();
extern void	db_deletewatch_cmd(), db_watchpoint_cmd();
extern void	db_single_step_cmd(), db_trace_until_call_cmd(),
		db_trace_until_matching_cmd(), db_continue_cmd();
extern void	db_stack_trace_cmd(), db_cond_cmd();
void		db_help_cmd();
void		db_def_macro_cmd(), db_del_macro_cmd();
void		db_fncall();

struct db_command db_command_table[] = {
#ifdef DB_MACHINE_COMMANDS
  /* this must be the first entry, if it exists */
	{ "machine",    0,                      0,     		0},
#endif
	{ "print",	db_print_cmd,		CS_OWN,		0 },
	{ "examine",	db_examine_cmd,		CS_MORE|CS_SET_DOT, 0 },
	{ "x",		db_examine_cmd,		CS_MORE|CS_SET_DOT, 0 },
	{ "xf",		db_examine_forward,	CS_SET_DOT,	0 },
	{ "xb",		db_examine_backward,	CS_SET_DOT,	0 },
	{ "search",	db_search_cmd,		CS_OWN|CS_SET_DOT, 0 },
	{ "set",	db_set_cmd,		CS_OWN,		0 },
	{ "write",	db_write_cmd,		CS_MORE|CS_SET_DOT, 0 },
	{ "w",		db_write_cmd,		CS_MORE|CS_SET_DOT, 0 },
	{ "delete",	db_delete_cmd,		CS_OWN,		0 },
	{ "d",		db_delete_cmd,		CS_OWN,		0 },
	{ "break",	db_breakpoint_cmd,	CS_MORE,	0 },
	{ "dwatch",	db_deletewatch_cmd,	CS_MORE,	0 },
	{ "watch",	db_watchpoint_cmd,	CS_MORE,	0 },
	{ "step",	db_single_step_cmd,	0,		0 },
	{ "s",		db_single_step_cmd,	0,		0 },
	{ "continue",	db_continue_cmd,	0,		0 },
	{ "c",		db_continue_cmd,	0,		0 },
	{ "until",	db_trace_until_call_cmd,0,		0 },
	{ "next",	db_trace_until_matching_cmd,0,		0 },
	{ "match",	db_trace_until_matching_cmd,0,		0 },
	{ "trace",	db_stack_trace_cmd,	0,		0 },
	{ "cond",	db_cond_cmd,		CS_OWN,	 	0 },
	{ "call",	db_fncall,		CS_OWN,		0 },
	{ "macro",	db_def_macro_cmd,	CS_OWN,	 	0 },
	{ "dmacro",	db_del_macro_cmd,	CS_OWN,		0 },
	{ "show",	0,			0,	db_show_cmds },
	{ (char *)0, }
};

#ifdef DB_MACHINE_COMMANDS

/* this function should be called to install the machine dependent
   commands. It should be called before the debugger is enabled  */
void db_machine_commands_install(ptr)
struct db_command *ptr;
{
  db_command_table[0].more = ptr;
  return;
}

#endif


struct db_command	*db_last_command = 0;

void
db_help_cmd()
{
	struct db_command *cmd = db_command_table;

	while (cmd->name != 0) {
	    db_printf("%-12s", cmd->name);
	    db_end_line();
	    cmd++;
	}
}

int	(*ddb_display)();

void
db_command_loop()
{
	jmp_buf_t db_jmpbuf;
	jmp_buf_t *prev = db_recover;
	extern int db_output_line;
	extern int db_macro_level;
#if	NORMA_IPC
	extern int _node_self;	/* node_self() may not be callable yet */
#endif	NORMA_IPC

	/*
	 * Initialize 'prev' and 'next' to dot.
	 */
	db_prev = db_dot;
	db_next = db_dot;

	if (ddb_display)
		(*ddb_display)();

	db_cmd_loop_done = 0;
	while (!db_cmd_loop_done) {
	    (void) _setjmp(db_recover = &db_jmpbuf);
	    db_macro_level = 0;
	    if (db_print_position() != 0)
		db_printf("\n");
	    db_output_line = 0;
	    db_printf("db%s", (db_default_thread)? "t": "");
#if	NORMA_IPC
	    db_printf("%d", _node_self);
#endif
#if	NCPUS > 1
	    db_printf("{%d}", cpu_number());
#endif
	    db_printf("> ");

	    (void) db_read_line("!!");
	    db_command_list(&db_last_command, db_command_table);
	}

	db_recover = prev;
}

boolean_t
db_exec_cmd_nest(cmd, size)
	char *cmd;
	int  size;
{
	struct db_lex_context lex_context;

	db_cmd_loop_done = 0;
	if (cmd) {
	    db_save_lex_context(&lex_context);
	    db_switch_input(cmd, size /**OLD, &lex_context OLD**/);
	}
	db_command_list(&db_last_command, db_command_table);
	if (cmd)
	    db_restore_lex_context(&lex_context);
	return(db_cmd_loop_done == 0);
}

#ifdef __GNUC__
extern __volatile__ void _longjmp();
#endif

void db_error(s)
	char *s;
{
	extern int db_macro_level;

	db_macro_level = 0;
	if (db_recover) {
	    if (s)
		db_printf(s);
	    db_flush_lex();
	    _longjmp(db_recover, 1);
	}
	else
	{
	    if (s)
	        db_printf(s);
	    panic("db_error");
	}
}

/*
 * Call random function:
 * !expr(arg,arg,arg)
 */
void
db_fncall()
{
	db_expr_t	fn_addr;
#define	MAXARGS		11
	db_expr_t	args[MAXARGS];
	int		nargs = 0;
	db_expr_t	retval;
	db_expr_t	(*func)();
	int		t;

	if (!db_expression(&fn_addr)) {
	    db_printf("Bad function \"%s\"\n", db_tok_string);
	    db_flush_lex();
	    return;
	}
	func = (db_expr_t (*) ()) fn_addr;

	t = db_read_token();
	if (t == tLPAREN) {
	    if (db_expression(&args[0])) {
		nargs++;
		while ((t = db_read_token()) == tCOMMA) {
		    if (nargs == MAXARGS) {
			db_printf("Too many arguments\n");
			db_flush_lex();
			return;
		    }
		    if (!db_expression(&args[nargs])) {
			db_printf("Argument missing\n");
			db_flush_lex();
			return;
		    }
		    nargs++;
		}
		db_unread_token(t);
	    }
	    if (db_read_token() != tRPAREN) {
		db_printf("?\n");
		db_flush_lex();
		return;
	    }
	}
	while (nargs < MAXARGS) {
	    args[nargs++] = 0;
	}

	retval = (*func)(args[0], args[1], args[2], args[3], args[4],
			 args[5], args[6], args[7], args[8], args[9] );
	db_printf(" %#N\n", retval);
}

boolean_t
db_option(modif, option)
	char	*modif;
	int	option;
{
	register char *p;

	for (p = modif; *p; p++)
	    if (*p == option)
		return(TRUE);
	return(FALSE);
}
