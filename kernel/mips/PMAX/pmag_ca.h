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
 * $Log:	pmag_ca.h,v $
 * Revision 2.2  92/05/22  15:49:04  jfriedl
 * 	Created, from Ultrix 4.2 code.
 * 	[92/05/08            af]
 * 
 */
/*
 *	File: pmag_ca.h
 * 	Author: Alessandro Forin, Carnegie Mellon University
 *	Date:	5/92
 *
 *	 Defines for the DS5000 PMAG-CA option board (Color Frame Buffer)
 */

#define	GA_OFFSET_POLL		0x000000	/* from module's base */
#define	GA_OFFSET_STAMP		0x0c0000	/* Stamp registers */
#define	GA_OFFSET_STIC		0x180000	/* Stic registers */
#define GA_OFFSET_BT459		0x200000	/* Bt459 registers */
#define GA_OFFSET_ROM		0x300000	/* Diagnostic ROM */

#define	GA_SLOT_SIZE		0x400000	/* total */

#define	GA_OFFSET_RESET_BT459	GA_OFFSET_ROM
