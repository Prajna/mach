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
 * $Log:	pmad_aa.h,v $
 * Revision 2.5  91/05/14  17:25:44  mrt
 * 	Correcting copyright
 * 
 * Revision 2.4  91/02/05  17:43:24  mrt
 * 	Added author notices
 * 	[91/02/04  11:16:07  mrt]
 * 
 * 	Changed to use new Mach copyright
 * 	[91/02/02  12:15:01  mrt]
 * 
 * Revision 2.3  90/12/05  23:33:29  af
 * 
 * 
 * Revision 2.1.1.1  90/11/01  02:43:20  af
 * 	Created, from the DEC specs:
 * 	"PMAD-AA TURBOchannel Ethernet Module Functional Specification"
 * 	Workstation Systems Engineering, Palo Alto, CA. Aug 27, 1990.
 * 	[90/09/03            af]
 */
/*
 *	File: pmad_aa.h
 * 	Author: Alessandro Forin, Carnegie Mellon University
 *	Date:	9/90
 *
 *	Defines for the DS5000 PMAD-AA option board (Ethernet Controller)
 */

#define	PMAD_OFFSET_RAM		0x0		/* from module's base */
#define PMAD_OFFSET_LANCE	0x100000	/* Lance registers */
#define PMAD_OFFSET_ROM		0x1c0000	/* Diag ROM and Address */

#define PMAD_RAM_SIZE		0x20000		/* 128k (32k*32) */

#define PMAD_ROM_SIZE		0x8000		/* 32k, appears on LSB */

#define PMAD_ROM_ESAR		0x1c0000	/* Station Address */
						/* appears on third byte */

#define PMAD_ESAR_ADDR0		0x1c0002
#define PMAD_ESAR_ADDR1		0x1c0006
#define PMAD_ESAR_ADDR2		0x1c000a
#define PMAD_ESAR_ADDR3		0x1c000e
#define PMAD_ESAR_ADDR4		0x1c0012
#define PMAD_ESAR_ADDR5		0x1c0016
						/* chksum+.. follows */

#define PMAD_ESAR_TEST		0x1c0062	/* should read:
						/* ff-00-55-aa-ff-00-55-aa */

