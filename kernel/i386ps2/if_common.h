/* 
 * Mach Operating System
 * Copyright (c) 1991 Carnegie Mellon University
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
 * $Log:	if_common.h,v $
 * Revision 2.2  93/02/04  08:00:13  danner
 * 	Integrate PS2 code from IBM.
 * 	[93/01/18            prithvi]
 * 
 */

/*
 * if_common.h
 * 
 *
 * Both the OSF/1 Ethernet and token ring drivers are derived from ACIS
 * sources and have source dependencies on files in the ACIS distribution
 * tree which have no equivalent in the OSF/1 tree.
 *
 * This file serves as a convient dumping ground for definitions for both
 * the Ungerman-Bass if_un.c and if_lan.c drivers.
 *
 */
#ifndef _I386PS2_IF_COMMON_H_
#define _I386PS2_IF_COMMON_H_

#include <mach/i386/vm_param.h>         /* for VM_MIN_KERNEL_ADDRESS */

#define KVBASE          VM_MIN_KERNEL_ADDRESS

/*
 * from /sys/caio/ioccvar.h
 */

#define DRIVER_SUSPEND  0x02		/* driver wants call on suspend */

#define SUSPEND_START 0			/* driver suspend start */
#define SUSPEND_DONE  1			/* driver suspend done */

#define PROBE_BAD	0		/* if the probe fails */
#define PROBE_NOINT	1		/* if probe ok but no interrupt */
#define PROBE_OK	2		/* if the probe was ok (interrupt caused) */
#define PROBE_BAD_INT	-1		/* we lost or didn't get interrupt */
#define PROBE_DELAY(n) 		\


/*
 * from /sys/ca/io.h
 */


/* Enable/Disable Interrupts from a Bus I/O level */
#define ENABLE	1
#define DISABLE	0

#define GEN_IN(cpu1,cpu2) (cpu1)
#define GEN_OUT(cpu1,cpu2) (cpu1)

/*
 * These Macros provide access to the PC's I/O space.
 *
 */

#define OUT(port, d)	outb(port, d)
#define IN(port)	inb(port)
#define OUTW(port, d)	outwb(port, d)
#define INW(port)	inwb(port)

#define MM_OUT(addr,d)	(*(unsigned char *)((int)addr+KVBASE) = (unsigned char)d)
#define MM_OUTW(addr,d)	(*(unsigned short *)((int)addr+KVBASE) = tr_swap(d))
#define MM_IN(addr)	(*(unsigned char *)((int)addr+KVBASE))
#ifdef notdef
#define MM_INW(addr)	(((*(unsigned char *)((int)addr+KVBASE)) << 8) | (*(unsigned char *)((int)addr+KVBASE+1)))
#else
#define MM_INW(addr)	tr_swap(*(unsigned short *)((int)addr+KVBASE))
#endif

/*
 *  Following macros do "in's and out's" for memory mapped devices
 *
 *  Must use set_128_window() before using these macros
 */

#define get_128_window() (1)
#define get_512_window() (1)

#define set_128_window(x) (x)	/* dummy */
#define set_512_window(x) (x)	/* dummy */

/*
 * from ca/debug.h
 */

#define SHOW_INTR	0x00000004
#define SHOW_IO		0x00000010
#define	SHOW_DATA	0x10000000

#define DEBUG

#ifdef DEBUG
#define DEBUGF(cond,stmt) if (cond) stmt	/* do the stmt (printf) */
#else
#define DEBUGF(cond,stmt) 		/* do nothing if not debugging */
#endif

/*
 * token ring specific definitions
 */
/*
 * from ca_atr/pcif.h
 */

int pc_copy_in;				/* bytes copied in from PC */
int pc_copy_out;			/* bytes copied out to PC */

#define TRIRQ 0x02

#endif  /* _I386PS2_IF_COMMON_H_ */
