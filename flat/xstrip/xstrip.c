/* 
 * Mach Operating System
 * Copyright (c) 1992,1991,1990 Carnegie Mellon University
 * All Rights Reserved.
 * 
 * Permission to use, copy, modify and distribute this software and its
 * documentation is hereby granted, provided that both the copyright
 * notice and this permission notice appear in all copies of the
 * software, derivative works or modified versions, and any portions
 * thereof, and that both notices appear in supporting documentation.
 * 
 * CARNEGIE MELLON ALLOWS FREE USE OF THIS SOFTWARE IN ITS 
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
 * any improvements or extensions that they make and grant Carnegie the
 * rights to redistribute these changes.
 */
/*
 * HISTORY
 * $Log:	xstrip.c,v $
 * Revision 2.1.1.1  94/06/01  10:24:41  rvb
 * 	From BSDSS
 * 
 * Revision 2.2  93/05/12  13:26:57  rvb
 * 	Stolen from user/xstrip
 * 
 * Revision 2.5  93/03/23  10:16:12  mrt
 * 	Moved definition of XSTRIP_AOUT_TOO before include of xstrip_alpha.c
 * 	which uses it.
 * 	[93/03/22            mrt]
 * 
 * Revision 2.4  93/03/21  18:12:42  mrt
 * 	Picked up Sandro's changes for alpha.
 * 	[93/03/20            mrt]
 * 
 * Revision 2.3  93/03/20  11:15:03  mrt
 * 	Added sun4
 * 	[93/02/05            mrt]
 * 
 * Revision 2.2  93/02/03  18:02:09  mrt
 * 	Moved from user, added pds's changes for gdb compatibility
 * 
 * Revision 2.5.2.1  92/12/23  17:46:46  pds
 * 	Converted file to ANSI C.
 * 	[92/12/23            pds]
 * 
 * Revision 2.5  92/01/22  22:52:59  rpd
 * 	Added luna88k to a.out case
 * 	[92/01/22            danner]
 * 
 * Revision 2.4  91/08/29  16:54:32  jsb
 * 	Moved mips functionality to xstrip_mips.c.
 * 	This is now a cover file that includes the right
 * 	file, or provides a failure stub if there is no
 * 	right file (e.g., unsupported machine type).
 * 
 */
/*
 *	File:	xstrip.c
 *	Author:	Joseph S. Barrera III
 *	Date:	1991
 */

#if	defined(mips)
#define	XSTRIP_IMPLEMENTATION	"xstrip_mips.c"
#endif

#if	defined(alpha)
#define	XSTRIP_AOUT_TOO
#include "xstrip_alpha.c"
#define XSTRIP_IMPLEMENTATION	"xstrip_aout.c"				   

/* because no more coff now */
#undef main
#define	main aout_main
#undef	N_BADMAG
#undef	N_TXTOFF
#endif

#if	defined(i386) || defined(vax) || defined(sun3) || defined(luna88k) || defined(sun4)
#define	XSTRIP_IMPLEMENTATION	"xstrip_aout.c"
#endif

#if	defined(XSTRIP_IMPLEMENTATION)
#include XSTRIP_IMPLEMENTATION
#else
void
main(void)
{
	printf("xstrip: not implemented for this architecture\n");
	exit(0);
}
#endif
