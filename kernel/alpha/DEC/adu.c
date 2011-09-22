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
 * CARNEGIE MELLON ALLOWS FREE USE OF THIS SOFTWARE IN ITS "AS-IS"
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
 * any improvements or extensions that they make and grant Carnegie Mellon the
 * the rights to redistribute these changes.
 */
/*
 * HISTORY
 * $Log:	adu.c,v $
 * Revision 2.2  93/02/05  07:56:45  danner
 * 	Dropped ISP hacking.
 * 	[93/02/04  01:19:18  af]
 * 
 * 	Added C version of simple_lock to deal with ADU bug.
 * 	[93/01/15            af]
 * 	Created, from the DEC specs:
 * 	"Alpha Demonstration Unit Specification"
 * 	V1.0, Aug 1990.
 * 	[92/06/02            af]
 * 	Created.
 * 	[92/12/10            af]
 * 
 */

#include <platforms.h>
#include <cpus.h>

#include <mach/std_types.h>

#if (NCPUS>1)

extern vm_offset_t kvtophys();

int use_Simple_lock = 0;

void simple_lock( long *l )
{
	int             nloops = 0;

	if (simple_lock_try(l))
		return;

	if (use_Simple_lock)
		Simple_lock(l);
	else
		while (!simple_lock_try(l)) {
			vm_offset_t     phys = kvtophys((vm_offset_t)l);

			/* flush Dcache every now and then */
			if ((nloops & 0x1f) == 0)
				alphacache_Dflush(phys);

			if (++nloops > 100000) {
				gimmeabreak();
				nloops = 0;
			}
		}
}

#endif

