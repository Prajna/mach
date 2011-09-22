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
 * $Log:	db_machdep.h,v $
 * Revision 2.10  93/01/14  17:51:18  danner
 * 	We only do COFF symtabs.
 * 	[92/12/10  20:07:48  af]
 * 
 * Revision 2.9  92/02/19  16:46:52  elf
 * 	Fixed bogus address check macro.
 * 	[92/02/10  17:30:20  af]
 * 
 * Revision 2.8  92/02/19  15:08:51  elf
 * 	Added db_thread_fp_used.
 * 	[92/02/19            rpd]
 * 
 * Revision 2.7  91/10/09  16:13:49  af
 * 	Added access and task name macros and screen size definitions.
 * 	[91/09/05            tak]
 * 
 * Revision 2.6  91/05/14  17:33:38  mrt
 * 	Correcting copyright
 * 
 * Revision 2.5  91/02/05  17:48:00  mrt
 * 	Added author notices
 * 	[91/02/04  11:22:03  mrt]
 * 
 * 	Changed to use new Mach copyright
 * 	[91/02/02  12:25:35  mrt]
 * 
 * Revision 2.4  91/01/08  15:49:31  rpd
 * 	Added inst_load, inst_store.
 * 	[90/11/27            rpd]
 * 
 * Revision 2.3  90/10/25  14:46:42  rwd
 * 	Generalized the watchpoint support.
 * 	[90/10/16            rwd]
 * 	Added watchpoint support.
 * 	[90/10/16  21:09:42  rpd]
 * 
 * Revision 2.2  90/08/27  22:07:31  dbg
 * 	Define DB_NO_AOUT to let MIPS use its own symbol table.
 * 	[90/08/27            dbg]
 * 
 * 	Got rid of ddb_regs, take PC off exception frame.
 * 	[90/08/20  10:23:52  af]
 * 
 * 	Adapted for mips boxes.
 * 	[90/08/14            af]
 * 
 * 	Created.
 * 	[90/07/25            dbg]
 * 
 */
/*
 *	File: db_machdep.h
 * 	Author: Alessandro Forin, Carnegie Mellon University
 *	Date:	8/90
 *
 *	Machine-dependent defines for new kernel debugger.
 */

#ifndef	_MIPS_DB_MACHDEP_H_
#define	_MIPS_DB_MACHDEP_H_

#include <mach/mips/vm_types.h>
#include <mach/mips/vm_param.h>
#include <mips/thread.h>		/* for thread_status */
#include <mips/mips_cpu.h>
#include <mach/boolean.h>

typedef	vm_offset_t	db_addr_t;	/* address - unsigned */
typedef	int		db_expr_t;	/* expression - signed */

typedef struct mips_saved_state db_regs_t;
db_regs_t	*db_cur_exc_frame;	/* register state */
#define	DDB_REGS	db_cur_exc_frame

#define	PC_REGS(regs)	((db_addr_t)(regs)->pc)

#define	BKPT_INST	0x0001000D	/* breakpoint instruction */
#define	BKPT_SIZE	(4)		/* size of breakpoint inst */
#define	BKPT_SET(inst)	(BKPT_INST)

#define	IS_BREAKPOINT_TRAP(type, code)	((type) == EXC_BP)
#define	IS_WATCHPOINT_TRAP(type, code)	((code) == 2)

#define	SOFTWARE_SSTEP			1	/* no hardware support */

#define	inst_trap_return(ins)		isa_rei(ins)
#define	inst_return(ins)		isa_ret(ins)
#define	inst_call(ins)			isa_call(ins)
#define inst_branch(ins)		isa_branch(ins)
#define inst_load(ins)			isa_load(ins)
#define	inst_store(ins)			isa_store(ins)
#define next_instr_address(v,b,task)	((unsigned)(v)+4)

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

boolean_t	db_check_access(/* vm_offset_t, int, task_t */);
boolean_t	db_phys_eq(/* task_t, vm_offset_t, task_t, vm_offset_t */);

/* macros for printing OS server dependent task name */

#define DB_TASK_NAME(task)		db_task_name(task)
#define DB_TASK_NAME_TITLE		"COMMAND                "
#define DB_TASK_NAME_LEN		23
#define DB_NULL_TASK_NAME		"?                      "

/* debugger screen size definitions */

#define	DB_MAX_LINE	50	/* max output lines */
#define DB_MAX_WIDTH	132	/* max output colunms */

void		db_task_name(/* task_t */);

/* definitions of object format */

#define	symtab_type(s)			SYMTAB_COFF
#define	DB_NO_AOUT			1

/* macro for checking if a thread has used floating-point */

#define db_thread_fp_used(thread)	((thread)->pcb->mms.mfs != 0)

#endif	/* _MIPS_DB_MACHDEP_H_ */
