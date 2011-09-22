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
 * CARNEGIE MELLON ALLOWS FREE USE OF THIS SOFTWARE IN ITS "AS-IS"
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
 * any improvements or extensions that they make and grant Carnegie Mellon the
 * the rights to redistribute these changes.
 */
/*
 * HISTORY
 * $Log:	trap.h,v $
 * Revision 2.2  93/01/14  17:14:42  danner
 * 	Created.
 * 	[92/12/10            af]
 * 
 */
/*
 *	File: trap.h
 * 	Author: Alessandro Forin, Carnegie Mellon University
 *	Date:	6/92
 *
 *	Trap defined for ALPHA
 */
				/* pc incremented ? */
#define	T_FPA_DISABLED	1
#define	T_PROT_FAULT	2
#define	T_TRANS_INVALID	3
#define	T_READ_FAULT	4
#define	T_WRITE_FAULT	5
#define	T_EXECUTE_FAULT	6
#define	T_ARITHMETIC	7	/* yes */
#define	T_AST_K	8
#define	T_AST_E	9
#define	T_AST_S	10
#define	T_AST_U	11
#define	T_UNALIGNED	12	/* yes */
#define	T_BP	13		/* yes */
#define	T_BUG	14		/* yes */
#define	T_ILL	15		/* yes */
#define	T_PAL	16		/* yes */
#define T_CHMK	17		/* yes */
#define	T_CHME	18		/* yes */
#define	T_CHMS	19		/* yes */
#define	T_CHMU	20		/* yes */
#define	T_SOFT_INT	21
#define	T_SCE	22
#define	T_PCE	23
#define	T_PFAIL	24
#define	T_SCHECK	25
#define	T_PCHECK	26
					/* other, software defined */
#define	T_UNEXPECTED	27
#define	T_BAD_INTERRUPT	28

#define	TRAP_TYPES	28
