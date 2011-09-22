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
 * $Log:	pmag_aa.h,v $
 * Revision 2.2  91/08/24  12:21:58  af
 * 	Created, from the DEC specs:
 * 	"PMAG-AA TURBOchannel Monochrome Frame Buffer Functional Specification"
 * 	Workstation Systems Engineering, Palo Alto, CA. Aug 27, 1990.
 * 	[91/01/25            af]
 * 
 */
/*
 *	File: pmag_aa.h
 * 	Author: Alessandro Forin, Carnegie Mellon University
 *	Date:	7/91
 *
 *	 Defines for the DS5000 PMAG-AA option board (Monochrome Frame Buffer)
 */

#define FB_OFFSET_ROM		0x000000	/* Diagnostic ROM */
#define FB_OFFSET_IREQ		0x080000	/* Interrupt req. control */
#define FB_OFFSET_BT455		0x100000	/* Bt455 registers */
#define FB_OFFSET_BT431		0x180000	/* Bt431 registers */
#define	FB_OFFSET_VRAM		0x200000	/* from module's base */
