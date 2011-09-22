/* 
 * Mach Operating System
 * Copyright (c) 1991,1990 Carnegie Mellon University
 * Copyright (c) 1991 IBM Corporation 
 * All Rights Reserved.
 * 
 * Permission to use, copy, modify and distribute this software and its
 * documentation is hereby granted, provided that both the copyright
 * notice and this permission notice appear in all copies of the
 * software, derivative works or modified versions, and any portions
 * thereof, and that both notices appear in supporting documentation,
 * and that the name IBM not be used in advertising or publicity 
 * pertaining to distribution of the software without specific, written
 * prior permission.
 * 
 * CARNEGIE MELLON AND IBM ALLOW FREE USE OF THIS SOFTWARE IN ITS "AS IS"
 * CONDITION.  CARNEGIE MELLON AND IBM DISCLAIM ANY LIABILITY OF ANY KIND FOR
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
 * $Log:	asm_startup.h,v $
 * Revision 2.2  93/02/04  07:58:24  danner
 * 	PS2 version
 * 	[92/02/24            dbg@ibm]
 * 
 * Revision 2.4  92/01/03  20:10:12  dbg
 * 	Symbols are no longer read separately by boot loader.
 * 	[91/08/02            dbg]
 * 
 * Revision 2.3  91/05/14  16:19:14  mrt
 * 	Correcting copyright
 * 
 * Revision 2.2  91/05/08  12:44:00  dbg
 * 	Created (from old i386/start.s).
 * 	[91/04/26  14:40:03  dbg]
 * 
 */

/*
 * Startup code for an i386 on a PS2.
 * Kernel is loaded starting at 1MB.
 * Protected mode, paging disabled.
 *
 * %esp ->	boottype
 *		size of extended memory (K)
 *		size of conventional memory (K)
 *		boothowto
 *              ABIOS data area
 *
 */

	popl	_boottype+KVTOPHYS	/ get boottype
	popl	_extmem+KVTOPHYS	/ extended memory, in K
	popl	_cnvmem+KVTOPHYS	/ conventional memory, in K
	popl	_boothowto+KVTOPHYS	/ boot flags
        popl    _abios_info+KVTOPHYS    / ABIOS info pointer

