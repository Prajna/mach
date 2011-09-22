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
 * $Log:	disk_inode.h,v $
 * Revision 2.3  93/01/14  17:09:06  danner
 * 	64bit clean. Assumes 32bit filesystem. Breaks 16bit systems(PDP-11).
 * 	[92/11/30            af]
 * 
 * Revision 2.2  92/01/03  19:56:42  dbg
 * 	Moved out of kernel.
 * 	[91/12/23  17:59:11  dbg]
 * 
 * Revision 2.4  91/05/14  15:22:38  mrt
 * 	Correcting copyright
 * 
 * Revision 2.3  91/02/05  17:01:14  mrt
 * 	Changed to new copyright
 * 	[91/01/28  14:54:49  mrt]
 * 
 * Revision 2.2  90/08/27  21:45:18  dbg
 * 	Took new definition from BSD 4.3 Reno release.
 * 	[90/07/16            dbg]
 * 
 */
/*
 * Copyright (c) 1982, 1989 The Regents of the University of California.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms are permitted
 * provided that the above copyright notice and this paragraph are
 * duplicated in all such forms and that any documentation,
 * advertising materials, and other materials related to such
 * distribution and use acknowledge that the software was developed
 * by the University of California, Berkeley.  The name of the
 * University may not be used to endorse or promote products derived
 * from this software without specific prior written permission.
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 *
 *	@(#)inode.h	7.5 (Berkeley) 7/3/89
 */

#ifndef	_BOOT_UFS_DISK_INODE_H_
#define	_BOOT_UFS_DISK_INODE_H_

/*
 * The I node is the focus of all file activity in the BSD Fast File System.
 * There is a unique inode allocated for each active file,
 * each current directory, each mounted-on file, text file, and the root.
 * An inode is 'named' by its dev/inumber pair. (iget/iget.c)
 * Data in icommon is read in from permanent inode on volume.
 */

#define	NDADDR	12		/* direct addresses in inode */
#define	NIADDR	3		/* indirect addresses in inode */

#define	MAX_FASTLINK_SIZE	((NDADDR + NIADDR) * sizeof(daddr_t))

struct 	icommon {
	u_short	ic_mode;	/*  0: mode and type of file */
	short	ic_nlink;	/*  2: number of links to file */
	uid_t	ic_uid;		/*  4: owner's user id */
	gid_t	ic_gid;		/*  6: owner's group id */
	quad	ic_size;	/*  8: number of bytes in file */
	time_t	ic_atime;	/* 16: time last accessed */
	int	ic_atspare;
	time_t	ic_mtime;	/* 24: time last modified */
	int	ic_mtspare;
	time_t	ic_ctime;	/* 32: last time inode changed */
	int	ic_ctspare;
	union {
	    struct {
		daddr_t	Mb_db[NDADDR];	/* 40: disk block addresses */
		daddr_t	Mb_ib[NIADDR];	/* 88: indirect blocks */
	    } ic_Mb;
	    char	ic_Msymlink[MAX_FASTLINK_SIZE];
					/* 40: symbolic link name */
	} ic_Mun;
#define	ic_db		ic_Mun.ic_Mb.Mb_db
#define	ic_ib		ic_Mun.ic_Mb.Mb_ib
#define	ic_symlink	ic_Mun.ic_Msymlink
	int	ic_flags;	/* 100: status, currently unused */
#define	IC_FASTLINK	0x0001		/* Symbolic link in inode */
	int	ic_blocks;	/* 104: blocks actually held */
	int	ic_gen;		/* 108: generation number */
	int	ic_spare[4];	/* 112: reserved, currently unused */
} i_ic;

#define	i_mode		i_ic.ic_mode
#define	i_nlink		i_ic.ic_nlink
#define	i_uid		i_ic.ic_uid
#define	i_gid		i_ic.ic_gid
#if	BYTE_MSF
#define	i_size		i_ic.ic_size.val[1]
#else	/* BYTE_LSF */
#define	i_size		i_ic.ic_size.val[0]
#endif
#define	i_db		i_ic.ic_db
#define	i_ib		i_ic.ic_ib
#define	i_atime		i_ic.ic_atime
#define	i_mtime		i_ic.ic_mtime
#define	i_ctime		i_ic.ic_ctime
#define i_blocks	i_ic.ic_blocks
#define	i_rdev		i_ic.ic_db[0]
#define	i_symlink	i_ic.ic_symlink
#define i_flags		i_ic.ic_flags
#define i_gen		i_ic.ic_gen

/* modes */
#define	IFMT		0xf000		/* type of file */
#define	IFCHR		0x2000		/* character special */
#define	IFDIR		0x4000		/* directory */
#define	IFBLK		0x6000		/* block special */
#define	IFREG		0x8000		/* regular */
#define	IFLNK		0xa000		/* symbolic link */
#define	IFSOCK		0xc000		/* socket */

#define	ISUID		0x0800		/* set user id on execution */
#define	ISGID		0x0400		/* set group id on execution */
#define	ISVTX		0x0200		/* save swapped text even after use */
#define	IREAD		0x0100		/* read, write, execute permissions */
#define	IWRITE		0x0080
#define	IEXEC		0x0040

/*
 *	Same structure, but on disk.
 */
struct dinode {
	union {
	    struct icommon	di_com;
	    char		di_char[128];
	} di_un;
};
#define	di_ic	di_un.di_com

#endif	/* _BOOT_UFS_DISK_INODE_H_ */
