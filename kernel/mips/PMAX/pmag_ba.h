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
 * $Log:	pmag_ba.h,v $
 * Revision 2.5  91/05/14  17:25:52  mrt
 * 	Correcting copyright
 * 
 * Revision 2.4  91/02/05  17:43:29  mrt
 * 	Added author notices
 * 	[91/02/04  11:16:16  mrt]
 * 
 * 	Changed to use new Mach copyright
 * 	[91/02/02  12:15:07  mrt]
 * 
 * Revision 2.3  90/12/05  23:33:36  af
 * 
 * 
 * Revision 2.1.1.1  90/11/01  03:35:57  af
 * 	Created, from the DEC specs:
 * 	"PMAG-BA TURBOchannel Color Frame Buffer Functional Specification"
 * 	Workstation Systems Engineering, Palo Alto, CA. Aug 27, 1990.
 * 	[90/09/03            af]
 */
/*
 *	File: pmag_ba.h
 * 	Author: Alessandro Forin, Carnegie Mellon University
 *	Date:	9/90
 *
 *	 Defines for the DS5000 PMAG-BA option board (Color Frame Buffer)
 */

#define	CFB_OFFSET_VRAM		0x0		/* from module's base */
						/* Replicated at x100000 */
#define CFB_OFFSET_BT459	0x200000	/* Bt459 registers */
#define CFB_OFFSET_IREQ		0x300000	/* Interrupt req. control */
#define CFB_OFFSET_ROM		0x380000	/* Diagnostic ROM */
#define CFB_OFFSET_RESET	0x3c0000	/* Bt459 resets on writes */

