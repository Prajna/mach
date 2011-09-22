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
 * CARNEGIE MELLON ALLOWS FREE USE OF THIS SOFTWARE IN ITS AS-IS
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
 * $Log:	sgmap.h,v $
 * Revision 2.2  93/03/09  10:48:44  danner
 * 	Created.
 * 	[93/02/18            jeffreyh]
 * 
 */
/*
 *	File: sgmap.h
 * 	Author: Jeffrey Heller, Kubota Pacific Computer 
 *	Date:	11/92
 *
 *	Scater/Gather DMA support.  See FMM 5.0.
 *
 */

#define SGMAP_START		KN15AA_REG_SGMAP /* Start of 128 map */

/*
 * The prom is able to use the first 24 entries
 */
#define SGMAP_NUM_ENTRIES		32768 /* 32K entries */
#define SGMAP_NUM_SYS_ENTRIES		24 /* The firmware may use upto 24 */
#define SGMAP_NUM_USABLE_ENTRIES	(SGMAP_NUM_ENTRIES - SGMAP_NUM_SYS_ENTRIES)


#	define SGMAP_ENTRY_VALID 	0x800000
#	define SGMAP_ENTRY_FUNNY	0x400000
#	define SGMAP_ENTRY_PARITY	0x200000

#	define SGMAP_ENTRY_MASK 	0x3fffffffU
