/* 
 * Mach Operating System
 * Copyright (c) 1991,1990 Carnegie Mellon University
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
 * $Log:	ufs.h,v $
 * Revision 2.5  91/05/14  17:19:08  mrt
 * 	Correcting copyright
 * 
 * Revision 2.4  91/02/05  17:39:33  mrt
 * 	Changed to use new Mach copyright
 * 	[91/02/02  12:06:56  mrt]
 * 
 * Revision 2.3  90/12/05  23:30:20  af
 * 	Copied over.
 * 	[90/12/04            af]
 * 
 * Revision 2.2  90/08/27  21:45:05  dbg
 * 	Created.
 * 	[90/07/16            dbg]
 * 
 */

/*
 * Common definitions for Berkeley Fast File System.
 */
#include <sys/types.h>
#include <device/param.h>

typedef	unsigned short	uid_t;
typedef	unsigned short	gid_t;
typedef	unsigned long	ino_t;

#define	NBBY	8

/*
 * The file system is made out of blocks of at most MAXBSIZE units,
 * with smaller units (fragments) only in the last direct block.
 * MAXBSIZE primarily determines the size of buffers in the buffer
 * pool.  It may be made larger without any effect on existing
 * file systems; however, making it smaller may make some file
 * systems unmountable.
 *
 * Note that the disk devices are assumed to have DEV_BSIZE "sectors"
 * and that fragments must be some multiple of this size.
 */
#define	MAXBSIZE	8192
#define	MAXFRAG		8

/*
 * MAXPATHLEN defines the longest permissible path length
 * after expanding symbolic links.
 *
 * MAXSYMLINKS defines the maximum number of symbolic links
 * that may be expanded in a path name.  It should be set
 * high enough to allow all legitimate uses, but halt infinite
 * loops reasonably quickly.
 */

#define	MAXPATHLEN	1024
#define	MAXSYMLINKS	8


extern int	ufs_open(), ufs_close(), ufs_read(), ufs_seek();

/*
 * Error codes for file system errors.
 */

#define	FS_NOT_DIRECTORY	5000		/* not a directory */
#define	FS_NO_ENTRY		5001		/* name not found */
#define	FS_NAME_TOO_LONG	5002		/* name too long */
#define	FS_SYMLINK_LOOP		5003		/* symbolic link loop */
#define	FS_INVALID_FS		5004		/* bad file system */
#define	FS_NOT_IN_FILE		5005		/* offset not in file */
#define	FS_INVALID_PARAMETER	5006		/* bad parameter to
						   a routine */
