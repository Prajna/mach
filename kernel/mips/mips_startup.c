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
 * $Log:	mips_startup.c,v $
 * Revision 2.6  91/08/24  12:23:34  af
 * 	Removed obsolete piece of wisdom.
 * 	[91/08/02  03:15:03  af]
 * 
 * Revision 2.5  91/05/14  17:36:32  mrt
 * 	Correcting copyright
 * 
 * Revision 2.4  91/02/05  17:50:05  mrt
 * 	Added author notices
 * 	[91/02/04  11:24:08  mrt]
 * 
 * 	Changed to use new Mach copyright
 * 	[91/02/02  12:27:57  mrt]
 * 
 * Revision 2.3  90/06/02  15:02:57  rpd
 * 	Removed include of confdep.h.
 * 	[90/03/26  22:52:36  rpd]
 * 
 * Revision 2.2  89/11/29  14:14:54  af
 * 	Do not print cache tests results: if they fail we never get here.
 * 	[89/11/26  10:30:22  af]
 * 
 * 	Modified cache state printout.
 * 	[89/11/16  14:37:43  af]
 * 
 * 	Created.
 * 	[89/10/29            af]
 */
/*
 *	File: mips_startup.c
 * 	Author: Alessandro Forin, Carnegie Mellon University
 *	Date:	10/89
 *
 *	Machine dependent startup code.
 */

#include <mach_kdb.h>

#include <sys/reboot.h>
#include <mips/prom_interface.h>
#include <mach/vm_param.h>
#include <vm/vm_kern.h>

/*
 * Machine-dependent startup code.
 */
machine_startup()
{
	extern unsigned icache_size, dcache_size;
	extern char 	version[];

	/*
	 * Initialization message print. 
	 */
	printf(version);
#define KBYTE	1024
#define MEG	(KBYTE*KBYTE)
	printf("cache : %d+%d kilobytes.\n",
		icache_size/KBYTE, dcache_size/KBYTE);
	printf("memory: %d.%d%d megabytes.\n", mem_size / MEG,
	       ((mem_size % MEG) * 10) / MEG,
	       ((mem_size % (MEG / 10)) * 100) / MEG);

	/*
	 * Initialize restart block
	 */
#if	MACH_KDB
	kdb_enable();
#else	MACH_KDB
	init_restartblk();
#endif	MACH_KDB

	/*
	 *	Start the system up
	 */
	setup_main();
}




init_restartblk()
{
	register struct restart_blk *rb = (struct restart_blk *)RESTART_ADDR;
	register i, sum;
	extern int halt_cpu();

	rb->rb_magic = RESTART_MAGIC;
	rb->rb_occurred = 0;

	rb->rb_restart = halt_cpu;

	sum = 0;
	for (i = 0; i < RESTART_CSUMCNT; i++)
		sum += ((int *)rb->rb_restart)[i];

	rb->rb_checksum = sum;
}

