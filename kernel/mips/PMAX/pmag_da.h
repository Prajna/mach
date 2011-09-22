/* 
 * Mach Operating System
 * Copyright (c) 1991,1990,1989 Carnegie Mellon University
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
 * $Log:	pmag_da.h,v $
 * Revision 2.2  92/05/22  15:49:09  jfriedl
 * 	Corrected some offsets. The bt459 one was actually right,
 * 	but we offset it by two bytes so that we get to read
 * 	off byte 0 as expected by existing cursor code.
 * 	[92/05/20  22:39:37  af]
 * 
 * 	Created, from Ultrix 4.2 code.
 * 	[92/05/08            af]
 * 
 */
/*
 *	File: pmag_da.h
 * 	Author: Alessandro Forin, Carnegie Mellon University
 *	Date:	5/92
 *
 *	 Defines for the DS5000 PMAG-DA/FA option boards (3D)
 */

#define	GQ_OFFSET_POLL		0x000000	/* from module's base */
#define	GQ_OFFSET_STAMP		0x0c0000	/* Stamp registers */
#define	GQ_OFFSET_STIC		0x180000	/* Stic registers */
#define	GQ_OFFSET_SRAM		0x200000	/* Packet area */
#define	GQ_OFFSET_INT_TO_HOST	0x284820	/* N10 intr, host clears */
#define	GQ_OFFSET_INT_TO_N10	0x2c4824	/* host intr, N10 clear */
#define GQ_OFFSET_BT459		0x300002	/* Bt459 registers */
#define GQ_OFFSET_RESET_BT459	0x340000	/* Bt459 registers */
#define GQ_OFFSET_START_N10	0x380000	/* Coproc start */
#define GQ_OFFSET_RESET_N10	0x3c0000	/* Coproc reset */

#define	GQ_SLOT_SIZE		0x400000	/* total */

