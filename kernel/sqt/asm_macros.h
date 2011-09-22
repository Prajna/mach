/* 
 * Mach Operating System
 * Copyright (c) 1991 Carnegie Mellon University
 * Copyright (c) 1991 Sequent Computer Systems
 * All Rights Reserved.
 * 
 * Permission to use, copy, modify and distribute this software and its
 * documentation is hereby granted, provided that both the copyright
 * notice and this permission notice appear in all copies of the
 * software, derivative works or modified versions, and any portions
 * thereof, and that both notices appear in supporting documentation.
 * 
 * CARNEGIE MELLON AND SEQUENT COMPUTER SYSTEMS ALLOW FREE USE OF
 * THIS SOFTWARE IN ITS "AS IS" CONDITION.  CARNEGIE MELLON AND
 * SEQUENT COMPUTER SYSTEMS DISCLAIM ANY LIABILITY OF ANY KIND FOR
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
 * $Log:	asm_macros.h,v $
 * Revision 2.3  91/07/31  17:59:07  dbg
 * 	Changed copyright.
 * 	[91/07/31            dbg]
 * 
 * Revision 2.2  91/05/08  12:51:37  dbg
 * 	Created from Sequent file.
 * 	[91/04/26  14:46:43  dbg]
 * 
 */

#ifndef	_SQT_ASM_MACROS_H_
#define	_SQT_ASM_MACROS_H_

/*
 * get the CPU number
 */
#define CPU_NUMBER(arg) \
	movzbl	VA_SLIC+SL_PROCID, arg; \
	movzbl	_slic_to_cpu(arg), arg


/*
 * SPL_ASM(new,old)	raise SPL to "new", put old value in "old" (mod's %ah).
 * SPLX_ASM(old)	lower SPL back to "old".
 *			Both change %edx.
 *
 * See machine/intctl.h for detail on spl synch with SLIC.
 * need 8 clocks at 16Mhz 10 at 20 Mhz  12 at 24 Mhz etc
 * modelC requires an extra 50 ns
 * all machines may run at 10% margins.
 * Note: movb X,r	will contribute 2 cycles
 *	 nop	        is 3 cycles
 *	 movl	r,r	is 2 cycles
 */

#ifndef	MHz
#define	MHz	20
#endif	MHz

/***************SLICSYNC 2 ***************************************/
#if MHz == 16
#define	SPL_ASM(new,old) \
	movb	VA_SLIC+SL_LMASK, old; \
	movb	new, VA_SLIC+SL_LMASK; \
 	movb	VA_SLIC+SL_LMASK, %ah; \
	nop; nop;
#else
#if MHz == 20
#define	SPL_ASM(new,old) \
	movb	VA_SLIC+SL_LMASK, old; \
	movb	new, VA_SLIC+SL_LMASK; \
 	movb	VA_SLIC+SL_LMASK, %ah; \
	movl	%eax,%eax; \
	movl	%eax,%eax; \
	nop; nop; 
#else
	ERROR not 16Mhz nor 20Hz
#endif
#endif

#define	SPLX_ASM(old) \
	movb	old, VA_SLIC+SL_LMASK

#endif	/* _SQT_ASM_MACROS_H_ */
