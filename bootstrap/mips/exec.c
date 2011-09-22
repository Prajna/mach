/* 
 * Mach Operating System
 * Copyright (c) 1993,1991,1990,1989 Carnegie Mellon University
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
 * Revision 2.4  93/11/17  16:00:43  dbg
 * 	De-linted.
 * 	[93/09/27            dbg]
 * 
 * Revision 2.3  93/02/04  09:58:59  mrt
 * 	Changed include of mach/mach.h to mach.h
 * 	[93/02/04            mrt]
 * 
 * Revision 2.2  92/01/03  19:57:58  dbg
 * 	Moved outside of kernel.
 * 	[91/09/04            dbg]
 * 
 * Revision 2.9  91/07/31  17:56:59  dbg
 * 	Added get_symtab.
 * 	[91/07/29            dbg]
 * 
 * Revision 2.5  90/12/05  20:49:47  af
 * 	Executable header is now defined in coff.h
 * 	[90/12/02            af]
 * 
 * Revision 2.4  90/08/27  22:08:04  dbg
 * 	Use new error names.  Use new file_io package.
 * 	[90/07/18            dbg]
 * 
 * Revision 2.3  90/06/02  15:02:03  rpd
 * 	Converted to new IPC.
 * 	[90/03/26  22:48:23  rpd]
 * 
 * 	Created, making the obvious mods to David Golub's vax code.
 * 	[89/10/09            af]
 * 
 */
/*
 *	File: exec.c
 * 	Author: Alessandro Forin, Carnegie Mellon University
 *	Date:	10/89
 *
 *	MIPS specific routines to read a COFF file, e.g.
 *	Machine-dependent portion of execve() for MIPS.
 *
 */

#include <mach.h>
#include <mach/machine/vm_param.h>

#include <file_io.h>
#include <loader_info.h>

#include <mips/coff.h>
#include <mips/syms.h>

/*
 *	Object:
 *		ex_get_header			EXPORTED function
 *
 *		Reads the exec header for the loader's benefit
 *
 */
int ex_get_header(
	struct file		*ip,
	struct loader_info	*lp)
{
	struct exechdr	x;
	register int	result;
	vm_size_t	resid;


	result = read_file(ip, 0, (vm_offset_t)&x, sizeof(x), &resid);
	if (result)
		return result;
	if (resid || (x.f.f_magic != MIPSMAGIC))
		return EX_NOT_EXECUTABLE;

	switch (x.a.magic) {

	    case 0407:
		lp->text_start  = 0;
		lp->text_size   = 0;
		lp->text_offset = 0;
		lp->data_start  = x.a.text_start;
		lp->data_size   = x.a.tsize + x.a.dsize;
		lp->data_offset = N_TXTOFF(x.f,x.a);
		lp->bss_size    = x.a.bsize;
		break;

	    case 0410:
	    case 0413:
		if (x.a.tsize == 0) {
			return EX_NOT_EXECUTABLE;
		}
		lp->text_start  = x.a.text_start;
		lp->text_size   = x.a.tsize;
		lp->text_offset = N_TXTOFF(x.f,x.a);
		lp->data_start  = x.a.data_start;
		lp->data_size   = x.a.dsize;
		lp->data_offset = lp->text_offset + lp->text_size;
		lp->bss_size    = x.a.bsize;
		break;

	    default:
		return EX_NOT_EXECUTABLE;
	}
	lp->entry_1 = x.a.entry;
	lp->entry_2 = x.a.gp_value;

	return 0;
}

/*
 *	Object:
 *		set_regs			EXPORTED function
 *
 *		Initialize enough state for a thread to run
 *
 */
#define STACK_SIZE (vm_size_t)(128*1024)

char *set_regs(
	mach_port_t	user_task,
	mach_port_t	user_thread,
	struct loader_info *lp,
	int		arg_size)
{
	vm_offset_t	stack_start;
	vm_offset_t	stack_end;
	struct mips_thread_state	regs;

	unsigned int		reg_size;

	/*
	 * Allocate stack.
	 */
	stack_end = VM_MAX_ADDRESS;
	stack_start = stack_end - STACK_SIZE;
	(void)vm_allocate(user_task,
			  &stack_start,
			  (vm_size_t)(STACK_SIZE),
			  FALSE);

	reg_size = MIPS_THREAD_STATE_COUNT;
	(void)thread_get_state(user_thread,
				MIPS_THREAD_STATE,
				(thread_state_t)&regs,
				&reg_size);

	regs.pc = lp->entry_1;
	regs.r28 = lp->entry_2;
	regs.r29 = (int)((stack_end - arg_size) & ~(sizeof(int)-1));

	(void)thread_set_state(user_thread,
				MIPS_THREAD_STATE,
				(thread_state_t)&regs,
				reg_size);

	return (char *)regs.r29;
}

boolean_t
get_symtab(
	struct file	*fp,
	vm_offset_t	*symoff_p,
	vm_size_t	*symsize_p,
	char		header[],
	vm_size_t	*header_size)
{
	vm_offset_t	st_filptr;
	vm_offset_t	st_end;

	union {
	    struct filehdr	filehdr;
	    HDRR		hdrr;
	} u0;

	kern_return_t	result;
	vm_size_t	resid;

	result = read_file(fp, 0, (vm_offset_t)&u0.filehdr,
			   sizeof(struct filehdr), &resid);
	if (result || resid)
	    return FALSE;

	st_filptr  = u0.filehdr.f_symptr;
	if (st_filptr == 0) {
	    *symsize_p = 0;
	    *header_size = 0;
	    return TRUE;
	}

	result = read_file(fp, st_filptr, (vm_offset_t)&u0.hdrr,
			   sizeof(HDRR), &resid);
	if (result || resid)
	    return FALSE;

	if (u0.hdrr.magic != magicSym)
	    return FALSE;

	/*
	 *	Assume that external symbol entries are the
	 *	last in the symbol table.
	 */
	st_end = u0.hdrr.cbExtOffset + u0.hdrr.iextMax * cbEXTR;

	*symoff_p = st_filptr;
	*symsize_p = st_end - st_filptr;

	*header_size = 0;

	return TRUE;
}

