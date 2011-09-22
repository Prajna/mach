/* 
 * Mach Operating System
 * Copyright (c) 1992 Carnegie Mellon University
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
 * $Log:	db_machdep.h,v $
 * Revision 2.2  93/02/05  07:58:10  danner
 * 	Fixed isa-breakpt macro.
 * 	[93/02/04  00:52:51  af]
 * 
 * 	Added reference to documentation source(s).
 * 	[92/12/16  15:14:10  af]
 * 
 * 	Added SAVE/RESTORE macros for multiP usage.
 * 	[92/12/16  12:43:09  af]
 * 
 * 	Created.
 * 	[92/05/31            af]
 * 
 */
/*
 *	File: db_machdep.h
 * 	Author: Alessandro Forin, Carnegie Mellon University
 *	Date:	5/92
 *
 *	Machine-dependent defines for kernel debugger.
 *
 *	This code was derived exclusively from information available in
 *	"Alpha Architecture Reference Manual", Richard L. Sites ed.
 *	Digital Press, Burlington, MA 01803
 *	ISBN 1-55558-098-X, Order no. EY-L520E-DP
 */

#ifndef	_ALPHA_DB_MACHDEP_H_
#define	_ALPHA_DB_MACHDEP_H_

#include <mach/alpha/vm_types.h>
#include <mach/alpha/vm_param.h>
#include <alpha/thread.h>		/* for thread_status */
#include <alpha/alpha_cpu.h>
#include <alpha/trap.h>
#include <mach/boolean.h>

typedef	vm_offset_t	db_addr_t;	/* address - unsigned */
typedef	long		db_expr_t;	/* expression - signed */

typedef struct alpha_saved_state db_regs_t;

db_regs_t		*db_cur_exc_frame;	/* register state */

#define	DDB_REGS	db_cur_exc_frame
#define	SAVE_DDB_REGS		db_regs_t *ddb_regs_save = db_cur_exc_frame
#define	RESTORE_DDB_REGS	db_cur_exc_frame = ddb_regs_save

#define	PC_REGS(regs)	((db_addr_t)(regs)->saved_frame.saved_pc)

#define	BKPT_INST	0x00000080	/* breakpoint instruction */
#define	BKPT_SIZE	(4)		/* size of breakpoint inst */
#define	BKPT_SET(inst)	(BKPT_INST)


#define	IS_BREAKPOINT_TRAP(type, code)	((type) == T_BP)
#define	IS_WATCHPOINT_TRAP(type, code)	((code) == 2)

/* #define	FIXUP_PC_AFTER_BREAK		DDB_REGS->pc -= 4; */

#define	SOFTWARE_SSTEP			1	/* no hardware support */

#define	inst_trap_return(ins)		isa_rei(ins)
#define	inst_return(ins)		isa_ret(ins)
#define	inst_call(ins)			isa_call(ins)
#define inst_branch(ins)		isa_branch(ins)
#define inst_load(ins)			isa_load(ins)
#define	inst_store(ins)			isa_store(ins)
#define next_instr_address(v,b,task)	((db_addr_t) ((b) ? (v): ((v)+4)))

#define	isa_kbreak(ins)			((ins) == BKPT_INST)

/* access capability and access macros */

#define	DB_ACCESS_LEVEL		2	/* access any space */
#define DB_CHECK_ACCESS(addr,size,task)				\
	db_check_access(addr,size,task)
#define DB_PHYS_EQ(task1,addr1,task2,addr2)			\
	db_phys_eq(task1,addr1,task2,addr2)
#define DB_VALID_KERN_ADDR(addr)				\
	(!ISA_KUSEG(addr))
#define DB_VALID_ADDRESS(addr,user)				\
	((!(user) && DB_VALID_KERN_ADDR(addr)) ||		\
	 ((user) && ISA_KUSEG(addr)))

boolean_t	db_check_access( /* vm_offset_t, int, task_t */ );
boolean_t	db_phys_eq( /* task_t, vm_offset_t, task_t, vm_offset_t */);

/* macros for printing OS server dependent task name */

#define DB_TASK_NAME(task)		db_task_name(task)
#define DB_TASK_NAME_TITLE		"COMMAND                "
#define DB_TASK_NAME_LEN		23
#define DB_NULL_TASK_NAME		"?                      "

/* debugger screen size definitions */

#define	DB_MAX_LINE	50	/* max output lines */
#define DB_MAX_WIDTH	132	/* max output colunms */

void		db_task_name(/* task_t */);

/* definitions of object format, we understand both */

/*#define	DB_NO_COFF	1	*/
/*#define	DB_NO_AOUT	1	*/

extern int db_alpha_symtab_type( char *st );
#define symtab_type(s)	db_alpha_symtab_type(s)


/* macro for checking if a thread has used floating-point */

#define db_thread_fp_used(thread)	((thread)->pcb->mms.mfs != 0)

#endif	/* _ALPHA_DB_MACHDEP_H_ */
