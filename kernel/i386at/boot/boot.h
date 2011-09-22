/*
 * Mach Operating System
 * Copyright (c) 1992, 1991 Carnegie Mellon University
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
 * $Log:	boot.h,v $
 * Revision 2.4  93/08/10  15:56:51  mrt
 * 	DEBUG -> LABEL_DEBUG
 * 	[93/08/02            rvb]
 * 	Under #ifdef DEBUG allow for FILE, LABEL and parition printout
 * 	[93/07/09  15:16:23  rvb]
 * 
 * Revision 2.3  93/05/10  17:47:01  rvb
 * 	bsdss has a different fs layout
 * 	[93/05/04  17:33:14  rvb]
 * 
 * Revision 2.2  92/04/04  11:35:03  rpd
 * 	Fabricated from 3.0 bootstrap.  But too many things are global.
 * 	[92/03/30            mg32]
 * 
 */

#include <sys/param.h>
#ifdef	__386BSD__
#include <ufs/quota.h>
#include <ufs/fs.h>
#include <ufs/inode.h>
#else	/* __386BSD__ */
#include <sys/fs.h>
#include <sys/inode.h>
#endif	/* __386BSD__ */

extern char *devs[], *name, *iodest;
extern struct fs *fs;
extern struct inode inode;
extern int unit, part, maj, boff, poff, bnum, cnt;

#ifdef	LABEL_DEBUG
extern int iflag;

#define I_DOS	1
#define I_LABEL	2
#define I_FILE	4
#endif	/* LABEL_DEBUG */
