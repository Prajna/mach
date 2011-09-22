/* 
 * Mach Operating System
 * Copyright (c) 1993,1992 Carnegie Mellon University
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
 * $Log:	alpha_cache.s,v $
 * Revision 2.3  93/01/19  08:56:59  danner
 * 	Added Dcache flushing.  Should have not been necessary.
 * 	[93/01/19            af]
 * 
 * Revision 2.2  93/01/14  17:10:52  danner
 * 	Added reference to documentation source(s).
 * 	[92/12/16  15:10:16  af]
 * 
 * 	Created.
 * 	[92/12/10  14:47:59  af]
 * 
 */
/*
 *	File: alpha_cache.s
 * 	Author: Alessandro Forin, Carnegie Mellon University
 *	Date:	6/92
 *
 *	Alpha cache control operations.
 *
 *	This code was derived exclusively from information available in
 *	"Alpha Architecture Reference Manual", Richard L. Sites ed.
 *	Digital Press, Burlington, MA 01803
 *	ISBN 1-55558-098-X, Order no. EY-L520E-DP
 */

#include <mach/alpha/asm.h>
#include <mach/alpha/alpha_instruction.h>

	.set	noreorder

/*
 *	Object:
 *		alphacache_Iflush		EXPORTED function
 *
 *		Flush (instruction) cache
 *
 *	Arguments:
 *		none
 *
 *	Just call the pal subrutine.
 */
LEAF(alphacache_Iflush,0)
	call_pal op_imb
	RET
	END(alphacache_Iflush)

/*
 *	Object:
 *		alphacache_Dflush		EXPORTED function
 *
 *		Flush (data) cache
 *
 *	Arguments:
 *		phys_addr			vm_offset_t
 *
 *	Turn the argument into a PFN and call the pal subrutine.
 */
LEAF(alphacache_Dflush,0)
	srl	a0,13,a0
	call_pal 1	/* cflush */
	RET
	END(alphacache_Dflush)

