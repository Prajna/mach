/* 
 * Mach Operating System
 * Copyright (c) 1993 Carnegie Mellon University
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
 * Revision 2.7  93/11/17  15:56:35  dbg
 * 	Converted "" includes to <>.
 * 	[93/09/29            dbg]
 * 
 * Revision 2.6  93/08/31  15:15:33  mrt
 * 	Fixed type blooper in aout_get_symtab(), which now works.
 * 	[93/08/19            af]
 * 
 * Revision 2.5  93/03/09  13:06:15  danner
 * 	mach/mach.h -> mach.h (again).
 * 	[93/03/09            danner]
 * 
 * Revision 2.4  93/03/09  10:47:34  danner
 * 	Added support for a.out images.
 * 	[93/03/05            af]
 * 
 * Revision 2.3  93/02/01  09:57:09  danner
 * 	mach/mach.h -> mach.h
 * 	[93/01/28            danner]
 * 
 * Revision 2.2  93/01/14  17:08:34  danner
 * 	Created.
 * 	[92/05/31            af]
 * 
 */
/*
 *	File: exec.c
 * 	Author: Alessandro Forin, Carnegie Mellon University
 *	Date:	05/92
 *
 *	ALPHA specific routines to read a file image, e.g.
 *	Machine-dependent portion of execve() for ALPHA.
 *	Understands both COFF (OSF or cross) and A.OUT (gcc).
 *
 */

#include <mach.h>
#include <mach/machine/vm_param.h>

#include <file_io.h>
#include <loader_info.h>

#include <alpha/coff.h>
#include <alpha/syms.h>
#include <alpha/exec.h>

#define private static

/*
 * COFF support
 */

/*
 *	Object:
 *		coff_ex_get_header		LOCAL function
 *
 *	Extracts from a COFF header the info the loader wants.
 *
 */
private
int coff_ex_get_header(
	struct file		*ip,
	struct loader_info	*lp)
{
	struct exechdr	x;
	register int	result;
	vm_size_t	resid;


	result = read_file(ip, 0, (vm_offset_t)&x, sizeof(x), &resid);
	if (result)
		return (result);
	if (resid || (x.f.f_magic != ALPHAMAGIC))
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

	return(0);
}

/*
 *	Object:
 *		coff_get_symtab			LOCAL function
 *
 *	Look at a COFF symbol table, and let the loader know
 *	what data from the file it should send to a debugger.
 *
 */
private boolean_t
coff_get_symtab(
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
	if (result || resid || (u0.filehdr.f_magic != ALPHAMAGIC))
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

/*
 * AOUT support
 */

/*
 *	Object:
 *		aout_ex_get_header		LOCAL function
 *
 *	Extracts from a AOUT header the info the loader wants.
 *
 */
private
int aout_ex_get_header(
	struct file		*ip,
	struct loader_info	*lp)
{
	struct exec	x;
	register int	result;
	vm_size_t	resid;


	result = read_file(ip, 0, (vm_offset_t)&x, sizeof(x), &resid);
	if (result)
		return result;
	if (resid)
		return EX_NOT_EXECUTABLE;

	switch (x.a_magic) {

	    case 0407:
		lp->text_start  = 0;
		lp->text_size   = 0;
		lp->text_offset = 0;
		lp->data_start  = x.a_tstart;
		lp->data_size   = x.a_text + x.a_data;
		lp->data_offset = sizeof(struct exec);
		lp->bss_size    = x.a_bss;
		break;

	    case 0410:
	    case 0413:
		if (x.a_text == 0) {
			return EX_NOT_EXECUTABLE;
		}
		lp->text_start  = x.a_tstart;
		lp->text_size   = x.a_text;
		lp->text_offset = (x.a_magic == ZMAGIC) ?
					0 : sizeof(struct exec);
		lp->data_start  = x.a_dstart;
		lp->data_size   = x.a_data;
		lp->data_offset = lp->text_offset + lp->text_size;
		lp->bss_size    = x.a_bss;
		break;

	    default:
		return EX_NOT_EXECUTABLE;
	}
	lp->entry_1 = x.a_entry;
	/* quadrant pointer, but crt0 recomputes it */
	lp->entry_2 = (x.a_entry >> 32) << 32;

	return 0;
}

/*
 *	Object:
 *		aout_get_symtab			LOCAL function
 *
 *	Look at a AOUT symbol table, and let the loader know
 *	what data from the file it should send to a debugger.
 *
 */
private boolean_t
aout_get_symtab(
	struct file	*fp,
	vm_offset_t	*symoff_p,
	vm_size_t	*symsize_p,
	char		header[],
	vm_size_t	*header_size)
{
	register int		result;
	vm_offset_t		resid;
	vm_offset_t		sym_off;
	vm_offset_t		str_off;
	vm_size_t		sym_size;
	unsigned int		str_size;
	struct exec		x;

	result = read_file(fp, 0, (vm_offset_t)&x, sizeof(x), &resid);
	if (result || resid)
	    return FALSE;

	sym_off = ((x.a_magic == ZMAGIC) ? 0 : sizeof(struct exec))
		  + x.a_text + x.a_data + x.a_trsize + x.a_drsize;
	sym_size = x.a_syms;
	str_off  = sym_off + sym_size;
	result = read_file(fp, str_off, (vm_offset_t) &str_size,
			   sizeof(str_size), &resid);
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
	/* but to keep things aligned.. */
	*header_size = sizeof(vm_size_t);

	return TRUE;
}

/*
 * Interface functions proper
 */

/*
 *	Object:
 *		ex_get_header			EXPORTED function
 *
 *	Reads the exec header for the loader's benefit
 *
 */
int ex_get_header(
	struct file		*ip,
	struct loader_info	*lp)
{
	int result;

	/* Try coff first */
	result = coff_ex_get_header(ip, lp);
	if (result == EX_NOT_EXECUTABLE)
		result = aout_ex_get_header(ip, lp);
	return result;
}

/*
 *	Object:
 *		get_symtab			EXPORTED function
 *
 *	Check if an image has a symbol table or not, if so
 *	return some information about it.
 *
 */
boolean_t
get_symtab(
	struct file	*fp,
	vm_offset_t	*symoff_p,
	vm_size_t	*symsize_p,
	char		header[],
	vm_size_t	*header_size)
{
	boolean_t	result;

	result = coff_get_symtab(fp, symoff_p, symsize_p, header, header_size);
	if (result == FALSE)
		result = aout_get_symtab(fp, symoff_p, symsize_p, header, header_size);
	return result;
}

/*
 *	Object:
 *		set_regs			EXPORTED function
 *
 *	Initialize enough state for a thread to run, including
 *	stack memory and stack pointer, and program counter.
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
	struct alpha_thread_state	regs;

	natural_t	reg_size;

	/*
	 * Allocate stack.
	 */
	stack_end = VM_MAX_ADDRESS;
	stack_start = stack_end - STACK_SIZE;
	(void)vm_allocate(user_task,
			  &stack_start,
			  (vm_size_t)(STACK_SIZE),
			  FALSE);

	reg_size = ALPHA_THREAD_STATE_COUNT;
	(void)thread_get_state(user_thread,
				ALPHA_THREAD_STATE,
				(thread_state_t)&regs,
				&reg_size);

	regs.pc = lp->entry_1;
	regs.r29 = lp->entry_2;
	regs.r30 = (integer_t)((stack_end - arg_size) & ~(sizeof(integer_t)-1));

	(void)thread_set_state(user_thread,
				ALPHA_THREAD_STATE,
				(thread_state_t)&regs,
				reg_size);

	return (char *)regs.r30;
}

