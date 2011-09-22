/* 
 * Mach Operating System
 * Copyright (c) 1993-1989 Carnegie Mellon University
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
 * $Log:	exec.c,v $
 * Revision 2.5  93/11/17  15:59:22  dbg
 * 	De-linted.  Changed "" includes to <>.  Added ANSI
 * 	function prototypes.
 * 	[93/09/28            dbg]
 * 
 * Revision 2.4  93/02/04  10:40:52  mrt
 * 	Changed mach/mach.h to mach.h.
 * 	[93/02/04            mrt]
 * 
 * Revision 2.3  92/03/01  15:14:39  rpd
 * 	Changed a_info to a_magic.
 * 	[92/03/01            rpd]
 * 
 * Revision 2.2  92/01/03  19:57:28  dbg
 * 	Moved outside of kernel.
 * 	[91/09/04            dbg]
 * 
 * Revision 2.8  91/07/31  17:35:40  dbg
 * 	Changed get_symtab to copy out the size of the symbol table and
 * 	return the symbol table size + string table size.
 * 	[91/07/31            dbg]
 * 
 * Revision 2.6  91/02/05  17:11:29  mrt
 * 	Changed x.a_magic to x.a_info to work with FSF's version of the
 * 	exec structure definition.
 * 
 * 	Added new copyright.
 * 	[91/01/28            mrt]
 * 
 * Revision 2.5  90/08/27  21:56:28  dbg
 * 	Add get_symtab to find symbol table in file.
 * 	[90/08/20            dbg]
 * 	Use new error return names.  Use new file_io package.
 * 	[90/07/17            dbg]
 * 
 * Revision 2.4  90/06/02  14:48:29  rpd
 * 	Converted to new IPC.
 * 	[90/06/01            rpd]
 * 
 * Revision 2.3  90/05/21  13:26:28  dbg
 * 	i386 a.out files DO NOT round data_size to loader_page...
 * 	[90/05/15            dbg]
 * 
 * Revision 2.2  90/05/03  15:25:07  dbg
 * 	Converted for i386 a.out format.
 * 	[90/02/15            dbg]
 * 
 * Revision 2.2  90/01/11  11:45:26  dbg
 * 	De-linted.
 * 	[90/01/03            dbg]
 * 
 * Revision 2.1  89/08/03  16:32:06  rwd
 * Created.
 * 
 *  3-Mar-89  David Golub (dbg) at Carnegie-Mellon University
 *	Created.
 *
 */
/*
 * i386-specific routines for loading a.out files.
 */

#include <mach.h>
#include <mach/machine/vm_param.h>

#include <file_io.h>
#include <loader_info.h>

#include <i386/exec.h>

/*
 *	Machine-dependent portions of execve() for the i386.
 */

int ex_get_header(
	struct file *fp,
	register struct loader_info *lp)
{
	struct exec	x;
	register int	result;
	vm_size_t	resid;

	result = read_file(fp, 0, (vm_offset_t)&x, sizeof(x), &resid);
	if (result)
		return result;
	if (resid)
		return EX_NOT_EXECUTABLE;

	switch ((int)x.a_magic) {

	    case 0407:
		lp->text_start  = 0;
		lp->text_size   = 0;
		lp->text_offset = 0;
		lp->data_start  = 0x10000;
		lp->data_size   = x.a_text + x.a_data;
		lp->data_offset = sizeof(struct exec);
		lp->bss_size    = x.a_bss;
		break;

	    case 0410:
		if (x.a_text == 0) {
			return EX_NOT_EXECUTABLE;
		}
		lp->text_start  = 0x10000;
		lp->text_size   = x.a_text;
		lp->text_offset = sizeof(struct exec);
		lp->data_start  = lp->text_start + lp->text_size;
		lp->data_size   = x.a_data;
		lp->data_offset = lp->text_offset + lp->text_size;
		lp->bss_size    = x.a_bss;
		break;

	    case 0413:
		if (x.a_text == 0) {
			return EX_NOT_EXECUTABLE;
		}
		lp->text_start  = 0x10000;
		lp->text_size   = sizeof(struct exec) + x.a_text;
		lp->text_offset = 0;
		lp->data_start  = lp->text_start + lp->text_size;
		lp->data_size   = x.a_data;
		lp->data_offset = lp->text_offset + lp->text_size;
		lp->bss_size    = x.a_bss;
		break;
	    default:
		return EX_NOT_EXECUTABLE;
	}
	lp->entry_1 = x.a_entry;
	lp->entry_2 = 0;

	return 0;
}

#define	STACK_SIZE	(64*1024)

char *set_regs(
	mach_port_t	user_task,
	mach_port_t	user_thread,
	struct loader_info *lp,
	int		arg_size)
{
	vm_offset_t	stack_start;
	vm_offset_t	stack_end;
	struct i386_thread_state	regs;
	unsigned int		reg_size;

	/*
	 * Add space for 5 ints to arguments, for
	 * PS program. XXX
	 */
	arg_size += 5 * sizeof(int);

	/*
	 * Allocate stack.
	 */
	stack_end = VM_MAX_ADDRESS;
	stack_start = VM_MAX_ADDRESS - STACK_SIZE;
	(void)vm_allocate(user_task,
			  &stack_start,
			  (vm_size_t)(stack_end - stack_start),
			  FALSE);

	reg_size = i386_THREAD_STATE_COUNT;
	(void)thread_get_state(user_thread,
				i386_THREAD_STATE,
				(thread_state_t)&regs,
				&reg_size);

	regs.eip = lp->entry_1;
	regs.uesp = (int)((stack_end - arg_size) & ~(sizeof(int)-1));

	(void)thread_set_state(user_thread,
				i386_THREAD_STATE,
				(thread_state_t)&regs,
				reg_size);

	return (char *)regs.uesp;
}



boolean_t
get_symtab(
	struct file	*fp,
	vm_offset_t	*symoff_p,	/* out */
	vm_size_t	*symsize_p,	/* out */
	char		header[],	/* out array */
	vm_size_t	*header_size)	/* out */
{
	register int		result;
	vm_offset_t		resid;
	vm_offset_t		sym_off;
	vm_offset_t		str_off;
	vm_size_t		sym_size;
	vm_size_t		str_size;
	struct exec		x;

	result = read_file(fp, 0, (vm_offset_t)&x, sizeof(x), &resid);
	if (result || resid)
	    return FALSE;

	sym_off = sizeof(struct exec)
		  + x.a_text + x.a_data + x.a_trsize + x.a_drsize;
	sym_size = x.a_syms;
	str_off  = sym_off + sym_size;
	result = read_file(fp, str_off,
			(vm_offset_t) &str_size, sizeof(int), &resid);
	if (result || resid)
	    return FALSE;

	/*
	 * Return the entire symbol table + string table.
	 * Add a header:
	 *   size of symbol table
	 */

	*symoff_p = sym_off;
	*symsize_p = sym_size + str_size;
	*(int *)header = sym_size;
	*header_size = sizeof(int);

	return TRUE;
}
