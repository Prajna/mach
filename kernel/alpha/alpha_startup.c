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
 * $Log:	alpha_startup.c,v $
 * Revision 2.2  93/01/14  17:11:39  danner
 * 	Created.
 * 	[92/06/05            af]
 * 
 */
/*
 *	File: alpha_startup.c
 * 	Author: Alessandro Forin, Carnegie Mellon University
 *	Date:	6/92
 *
 *	Machine dependent startup code.
 */

#include <mach/vm_param.h>

/*
 * Machine-dependent startup code.
 */
machine_startup()
{
	extern char 	version[];

	/*
	 * Initialization message print. 
	 */
	printf(version);
#define KBYTE	1024
#define MEG	(KBYTE*KBYTE)
	printf("memory: %d.%d%d megabytes.\n", mem_size / MEG,
	       ((mem_size % MEG) * 10) / MEG,
	       ((mem_size % (MEG / 10)) * 100) / MEG);

	/*
	 *	Start the system up
	 */
	setup_main();
}

