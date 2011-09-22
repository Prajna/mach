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
 * $Log:	pmaz_aa.h,v $
 * Revision 2.5  91/05/14  17:26:00  mrt
 * 	Correcting copyright
 * 
 * Revision 2.4  91/02/05  17:43:33  mrt
 * 	Added author notices
 * 	[91/02/04  11:16:21  mrt]
 * 
 * 	Changed to use new Mach copyright
 * 	[91/02/02  12:15:13  mrt]
 * 
 * Revision 2.3  90/12/05  23:33:44  af
 * 
 * 
 * Revision 2.1.1.1  90/11/01  02:48:49  af
 * 	Created, from the DEC specs:
 * 	"PMAZ-AA TURBOchannel SCSI Module Functional Specification"
 * 	Workstation Systems Engineering, Palo Alto, CA. Aug 27, 1990.
 * 	[90/09/03            af]
 */
/*
 *	File: pmaz_aa.h
 * 	Author: Alessandro Forin, Carnegie Mellon University
 *	Date:	9/90
 *
 *	Defines for the DS5000 PMAZ-AA option board (SCSI interface)
 */

#define ASC_OFFSET_53C94	0x0		/* from module base */
#define ASC_OFFSET_DMAR		0x40000		/* DMA Address Register */
#define ASC_OFFSET_RAM		0x80000		/* SRAM Buffer */
#define ASC_OFFSET_ROM		0xc0000		/* Diagnostic ROM */

#define	ASC_RAM_SIZE		0x20000		/* 128k (32k*32) */

/*
 * DMA Address Register
 */

#define ASC_DMAR_MASK		0x1ffff		/* 17 bits, 128k */
#define ASC_DMAR_WRITE		0x80000000	/* DMA direction bit */
#define	ASC_DMA_ADDR(x)		((unsigned)(x))&ASC_DMAR_MASK

