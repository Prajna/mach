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
 * $Log:	debugf.h,v $
 * Revision 2.2  93/02/04  07:59:15  danner
 * 	Integrate PS2 code from IBM.
 * 	[93/01/18            prithvi]
 * 
 */

#ifndef _H_DEBUGF
#define _H_DEBUGF
/*
 * COMPONENT_NAME: INC/SYS debugf.h Debugging Utility Define's
 *
 * FUNCTIONS:
 *
 * ORIGINS: 27
 */                                                                   

#ifdef DEBUG
#define DEBUGF(cond,stmt) if (cond) stmt	/* do the stmt (printf) */
#else
#define DEBUGF(cond,stmt) 		/* do nothing if not debugging */
#endif
/*
 * Usage:
 * DEBUGF(fddebug & DEBUG_OPEN,
 *	printf("fdopen: Entering, Hi, I am %s, %d 0x%x\n", here, dec, hex));
 */
#define DEBUG_ALL		0xffffffff
#define DEBUG_NONE		0x00000000

/*
 * Bit 0-15 are used to debug entry points found in most device drivers.
 * Bit 16-30 are used to debug component specific routines.
 * Bit 31 is used to debug error conditions.
 */
#define DEBUG_CONFIG		BIT0
#define DEBUG_OPEN		BIT1
#define DEBUG_CLOSE		BIT2
#define DEBUG_READ		BIT3
#define DEBUG_WRITE		BIT4
#define DEBUG_IOCTL		BIT5
#define DEBUG_STRATEGY		BIT6
#define DEBUG_SELECT		BIT7
#define DEBUG_PRINT		BIT8
#define DEBUG_DUMP		BIT9
#define DEBUG_MPX		BIT10
#define DEBUG_REVOKE		BIT11
#define DEBUG_INTR		BIT12
#define DEBUG_CALL		BIT13		/* still free */
#define DEBUG_BIT14		BIT14		/* still free */
#define DEBUG_ABIOS		BIT15

#define DEBUG_ERR_COND		BIT31

#ifdef DEBUG
extern void debug_handy_brkpt(char * msg);
#else
#define debug_handy_brkpt(msg)
#endif
/*
 * Usage: use debugger to set a break point at debug_handy_brkpt(),
 *	  put this line in your program where you wish to stop:
 * debug_handy_brkpt("I am here");
 */

#ifdef DEBUG
extern void dump_abios_rb(caddr_t addr, int flag);
#else
#define dump_abios_rb(addr, flag)
#endif
/*
 * Usage: dump content of abios request block.
 * dump_abios_rb((caddr_t)&this_request_block, DUMP_HEADER_ONLY);
 * or
 * dump_abios_rb((caddr_t)&this_request_block, DUMP_ENTIRE_BLOCK);
 */
#define DUMP_HEADER_ONLY		BIT0
#define DUMP_ENTIRE_BLOCK		BIT1

/*
 * Little-Endian bit convention
 */
#define BIT0				0x00000001
#define BIT1				0x00000002
#define BIT2				0x00000004
#define BIT3				0x00000008
#define BIT4				0x00000010
#define BIT5				0x00000020
#define BIT6				0x00000040
#define BIT7				0x00000080
#define BIT8				0x00000100
#define BIT9				0x00000200
#define BIT10				0x00000400
#define BIT11				0x00000800
#define BIT12				0x00001000
#define BIT13				0x00002000
#define BIT14				0x00004000
#define BIT15				0x00008000
#define BIT16				0x00010000
#define BIT17				0x00020000
#define BIT18				0x00040000
#define BIT19				0x00080000
#define BIT20				0x00100000
#define BIT21				0x00200000
#define BIT22				0x00400000
#define BIT23				0x00800000
#define BIT24				0x01000000
#define BIT25				0x02000000
#define BIT26				0x04000000
#define BIT27				0x08000000
#define BIT28				0x10000000
#define BIT29				0x20000000
#define BIT30				0x40000000
#define BIT31				0x80000000

#endif /* _H_DEBUGF */
