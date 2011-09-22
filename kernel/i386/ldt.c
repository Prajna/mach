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
 * $Log:	ldt.c,v $
 * Revision 2.5  91/05/14  16:10:48  mrt
 * 	Correcting copyright
 * 
 * Revision 2.4  91/02/05  17:12:45  mrt
 * 	Changed to new Mach copyright
 * 	[91/02/01  17:35:36  mrt]
 * 
 * Revision 2.3  90/08/27  21:57:15  dbg
 * 	Use new segment definitions.
 * 	[90/07/25            dbg]
 * 
 * Revision 2.2  90/05/03  15:33:38  dbg
 * 	Created.
 * 	[90/02/15            dbg]
 * 
 */

/*
 * "Local" descriptor table.  At the moment, all tasks use the
 * same LDT.
 */
#include <i386/seg.h>
#include <mach/i386/vm_types.h>
#include <mach/i386/vm_param.h>

extern int	syscall();

struct fake_descriptor	ldt[LDTSZ] = {
/*007*/	{ (unsigned int)&syscall,
	  KERNEL_CS,
	  0,				/* no parameters */
	  ACC_P|ACC_PL_U|ACC_CALL_GATE
	},				/* call gate for system calls */
/*00F*/	{ 0, 0, 0, 0 },			/* unused */
/*017*/	{ 0,
	  (VM_MAX_ADDRESS-VM_MIN_ADDRESS-1)>>12,
	  SZ_32|SZ_G,
	  ACC_P|ACC_PL_U|ACC_CODE_R
	},				/* user code segment */
/*01F*/	{ 0,
	  (VM_MAX_ADDRESS-VM_MIN_ADDRESS-1)>>12,
	  SZ_32|SZ_G,
	  ACC_P|ACC_PL_U|ACC_DATA_W
	},				/* user data segment */
};
