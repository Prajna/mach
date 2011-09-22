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
 * $Log:	alpha_instruction.c,v $
 * Revision 2.2  93/01/14  17:11:16  danner
 * 	Added reference to documentation source(s).
 * 	[92/12/16  15:11:14  af]
 * 
 * 	Created.
 * 	[92/06/01            af]
 * 
 */
/*
 *	File: alpha_instruction.c
 * 	Author: Alessandro Forin, Carnegie Mellon University
 *	Date:	6/92
 *
 *	Functions that operate on ALPHA instructions,
 *	such as branch prediction and opcode predicates.
 *
 *	This code was derived exclusively from information available in
 *	"Alpha Architecture Reference Manual", Richard L. Sites ed.
 *	Digital Press, Burlington, MA 01803
 *	ISBN 1-55558-098-X, Order no. EY-L520E-DP
 */

#include <mach_kdb.h>

#include <mach/alpha/alpha_instruction.h>
#include <mach/exception.h>
#include <alpha/alpha_cpu.h>
#include <mach/mach_types.h>
#include <kern/task.h>
#include <alpha/thread.h>

/*
 *	Object:
 *		isa_call			EXPORTED function
 *
 *		Function call predicate
 *
 */
boolean_t
isa_call(ins)
	register alpha_instruction ins;
{
	return ((ins.branch_format.opcode == op_bsr) ||
		((ins.jump_format.opcode == op_j) &&
		 (ins.jump_format.action & 1)));	/* covers jsr & jcr */
}


/*
 *	Object:
 *		isa_ret				EXPORTED function
 *
 *		Function return predicate
 *
 */
boolean_t
isa_ret(ins)
	register alpha_instruction ins;
{
	return	((ins.jump_format.opcode == op_j) &&
		 (ins.jump_format.action == op_ret));
}


/*
 *	Object:
 *		isa_rei				EXPORTED function
 *
 *		Return from interrupt predicate
 *
 */
boolean_t
isa_rei(ins)
	register alpha_instruction ins;
{
	return ((ins.pal_format.opcode == op_pal) &&
		(ins.pal_format.function == op_rei));
}

/*
 *	Object:
 *		isa_branch			EXPORTED function
 *
 *		Branch predicate
 *
 *	Does NOT include function calls, use isa_call() for that.
 *	Includes all other jump and branch instructions (ret included)
 */
boolean_t
isa_branch(ins)
	register alpha_instruction ins;
{
	switch (ins.branch_format.opcode) {

	    case op_j:
	    case op_br:
	    case op_fbeq:
	    case op_fblt:
	    case op_fble:
	    case op_fbne:
	    case op_fbge:
	    case op_fbgt:
	    case op_blbc:
	    case op_beq:
	    case op_blt:
	    case op_ble:
	    case op_blbs:
	    case op_bne:
	    case op_bge:
	    case op_bgt:
		return TRUE;

	    default:
		return FALSE;
	}
}

/*
 *	Object:
 *		inst_unconditonal_flow_transfer		EXPORTED function
 *
 *		return true for instructions that result in
 *              unconditional transfers of the flow of control.
 *
 */
boolean_t
inst_unconditional_flow_transfer(ins)
	register alpha_instruction ins;
{
	switch (ins.branch_format.opcode) {

	    case op_j:
	    case op_br:
		return TRUE;

	    case op_pal:
		return ((ins.pal_format.function == op_rei) ||
			(ins.pal_format.function == op_chmk));

	}

	return FALSE;

}

/*
 *	Object:
 *		isa_spill			EXPORTED function
 *
 *		Register save (spill) predicate
 *
 */
boolean_t
isa_spill(ins,regn)
	register alpha_instruction ins;
	register unsigned regn;
{
	return ((ins.mem_format.opcode == op_stq) &&
		(ins.mem_format.rd == regn));
}

/*
 *	Object:
 *		isa_load			EXPORTED function
 *
 *		Memory load predicate.
 *
 */
boolean_t
isa_load(ins)
	register alpha_instruction ins;
{
	return
		/* loads */
		(ins.mem_format.opcode == op_ldq_u) ||
		((op_ldf <= ins.mem_format.opcode) &&
		 (ins.mem_format.opcode <= op_ldt)) ||
		((op_ldl <= ins.mem_format.opcode) &&
		 (ins.mem_format.opcode <= op_ldq_l)) ||
		/* prefetches */
		((ins.mem_format.opcode == op_special) &&
		 ((ins.mem_format.displacement == (short)op_fetch) ||
		  (ins.mem_format.displacement == (short)op_fetch_m))) ;
		/* note: MB is treated as a store */
}

/*
 *	Object:
 *		isa_store			EXPORTED function
 *
 *		Memory store predicate.
 *
 */
boolean_t
isa_store(ins)
	register alpha_instruction ins;
{
	return
		/* loads */
		(ins.mem_format.opcode == op_stq_u) ||
		((op_stf <= ins.mem_format.opcode) &&
		 (ins.mem_format.opcode <= op_stt)) ||
		((op_stl <= ins.mem_format.opcode) &&
		 (ins.mem_format.opcode <= op_stq_c)) ||
		/* barriers */
		((ins.mem_format.opcode == op_special) &&
		 (ins.mem_format.displacement == op_mb));
}

/*
 *	Object:
 *		isa_load_store			EXPORTED function
 *
 *		Memory load/store predicate
 *
 *	If the instruction is a load or store instruction
 *	returns the destination address.
 */
boolean_t
isa_load_store(ins, dest_addr, getreg, arg)
	register alpha_instruction   ins;
	vm_offset_t		   *dest_addr;
	vm_offset_t		  (*getreg)();
	vm_offset_t		    arg;
{
	if ((ins.mem_format.opcode == op_ldq_u) ||
	    (ins.mem_format.opcode == op_stq_u) ||
	    ((ins.mem_format.opcode >= op_ldf) &&
	     (ins.mem_format.opcode <= op_stq_c))) {

		/*
		 * The only address calculation is register+displacement 
		 */
		*dest_addr = (vm_offset_t) (ins.mem_format.displacement +
					 (*getreg) (ins.mem_format.rs, arg));

		return TRUE;
	}
	return FALSE;
}

/*
 *	Object:
 *		branch_taken			EXPORTED function
 *
 *		Branch prediction
 *
 *	Returns the address where the instruction might branch,
 *	if the branch is taken.
 *	Needs the address where the instruction is located and
 *	a function returning the current value of some register.
 *
 *	The instruction must be a call or branch, or we panic.
 */
vm_offset_t
branch_taken(ins, addr, getreg, arg)
	register alpha_instruction ins;
	vm_offset_t	addr;
	vm_offset_t	(*getreg)();
	vm_offset_t	 arg;
{
	switch (ins.branch_format.opcode) {

	    case op_j:
		return (*getreg) (ins.jump_format.rs, arg) & ~3;

	    case op_br:
	    case op_fbeq:
	    case op_fblt:
	    case op_fble:
	    case op_bsr:
	    case op_fbne:
	    case op_fbge:
	    case op_fbgt:
	    case op_blbc:
	    case op_beq:
	    case op_blt:
	    case op_ble:
	    case op_blbs:
	    case op_bne:
	    case op_bge:
	    case op_bgt:
		return ((ins.branch_format.displacement << 2) + (addr + 4));

	}

	panic("branch_taken");
}


#if	MACH_KDB
/*
 *	Object:
 *		stack_modified			EXPORTED function
 *
 *		Does the instruction affect the stack pointer and how
 *
 *	Returns the amount by which the instruction changes the
 *	stack pointer, or 0.
 *	Needs a function returning the current value of some
 *	register, but this needs not be precise as the C compiler
 *	typically uses immediate values.
 */
int stack_modified(ins, getreg, arg)
	register alpha_instruction ins;
	vm_offset_t	(*getreg)();
	vm_offset_t	arg;
{
#define SP 30
	/* frame is mods only by lda. else you lose */
	if ((ins.mem_format.opcode == op_lda) &&
	    (ins.mem_format.rd == SP)) {

		if (ins.mem_format.rs == SP)
			return ins.mem_format.displacement;
		return ins.mem_format.displacement +
			(*getreg)(ins.mem_format.rs, arg) -
			(*getreg)(SP, arg);
	}
	return 0;
}

#define	ALPHA_REG_SAVE_SEARCH	((32+3)*4)	/* ???? what is this ???? */

/*
 *	Object:
 *		db_restore_regs			EXPORTED function
 *
 *		restore register environment
 *
 *	This code assumes that all register saves are made at the prolog
 *	code, and only "lda[h]" and "stq xx,yy(sp)" appear in
 *	in it.
 *
 *	Which is of course pure idiocy, unless you ever only saw GCC.
 *	Anyways, OSF gaveth and the code that uses it can tolerate it.
 */
void
db_restore_regs(ssp, sp, proc_pc, cur_pc, task)
	struct		alpha_saved_state *ssp;
	vm_offset_t	sp;
	vm_offset_t	proc_pc;
	vm_offset_t	cur_pc;
	task_t		task;
{
	register vm_offset_t	pc, epc;
	alpha_instruction	ins;
	extern vm_size_t	*addrof_alpha_reg();

	epc = proc_pc + ALPHA_REG_SAVE_SEARCH;
	if (epc > cur_pc)
		epc = cur_pc;
	for (pc = proc_pc; pc < epc; pc += sizeof(alpha_instruction)) {

		ins.bits = db_get_task_value(pc, sizeof(alpha_instruction), FALSE, task);

		if (ins.mem_format.opcode == op_lda
			|| ins.mem_format.opcode == op_ldah) 
			continue;

		if ((ins.mem_format.rs == SP) &&
		    (ins.mem_format.opcode == op_stq)) {
			if (ins.mem_format.rd != 31) {
				vm_size_t	*p = addrof_alpha_reg(ins.mem_format.rd,ssp);

				*p = db_get_task_value(sp + ins.mem_format.displacement,
							 8, FALSE, task);
			}
			continue;
		} 
	end_prolog:
		/*
		 * end of prolog code.  look one more instruction.
		 */
		if (epc > pc + 4)
			epc = pc + 8;
	}
}

#endif	MACH_KDB
