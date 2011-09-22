/* 
 * Mach Operating System
 * Copyright (c) 1991,1990,1989,1988,1987 Carnegie Mellon University
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
 * $Log:	time_stamp.c,v $
 * Revision 2.6  93/01/14  17:37:21  danner
 * 	64bit cleanup.
 * 	[92/12/10  18:14:01  af]
 * 
 * Revision 2.5  92/08/03  17:40:20  jfriedl
 * 	removed silly prototypes
 * 	[92/08/02            jfriedl]
 * 
 * Revision 2.4  92/05/21  17:16:57  jfriedl
 * 	Cleanup to quiet gcc warnings.
 * 	[92/05/16            jfriedl]
 * 
 * Revision 2.3  91/05/14  16:49:34  mrt
 * 	Correcting copyright
 * 
 * Revision 2.2  91/02/05  17:30:55  mrt
 * 	Changed to new Mach copyright
 * 	[91/02/01  16:20:38  mrt]
 * 
 * Revision 2.1  89/08/03  15:53:16  rwd
 * Created.
 * 
 * 16-Jun-87  David Black (dlb) at Carnegie-Mellon University
 *	machtimer.h --> timer.h  Changed to cpp symbols for multimax.
 *
 *  5-Apr-87  Avadis Tevanian (avie) at Carnegie-Mellon University
 *	ts.h -> time_stamp.h
 *
 * 30-Mar-87  David Black (dlb) at Carnegie-Mellon University
 *	Created.
 */ 

#include <mach/std_types.h>
#include <sys/time.h>
#include <kern/time_stamp.h>

/*
 *	ts.c - kern_timestamp system call.
 */
#ifdef	multimax
#include <mmax/timer.h>
#endif	multimax



kern_return_t
kern_timestamp(tsp)
struct	tsval	*tsp;
{
#ifdef	multimax
	struct	tsval	temp;
	temp.low_val = FRcounter;
	temp.high_val = 0;
#else	multimax
/*
	temp.low_val = 0;
	temp.high_val = ts_tick_count;
*/
	time_value_t temp;
	temp = time;
#endif	multimax

	if (copyout((char *)&temp,
		    (char *)tsp,
		    sizeof(struct tsval)) != KERN_SUCCESS)
	    return(KERN_INVALID_ADDRESS);
	return(KERN_SUCCESS);
}

/*
 *	Initialization procedure.
 */

void timestamp_init()
{
#ifdef	multimax
#else	multimax
	ts_tick_count = 0;
#endif	multimax
}
