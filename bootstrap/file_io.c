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
 * $Log:	file_io.c,v $
 * Revision 2.8  93/05/10  19:40:24  rvb
 * 	Changed "" includes to <>
 * 	[93/04/30            mrt]
 * 
 * Revision 2.7  93/05/10  17:44:48  rvb
 * 	Include files specified with quotes dont work properly
 * 	when the C file in in the master directory but the
 * 	include file is in the shadow directory. Change to
 * 	using angle brackets.
 * 	Ian Dall <DALL@hfrd.dsto.gov.au>	4/28/93
 * 	[93/05/10  13:15:31  rvb]
 * 
 * Revision 2.6  93/01/14  17:09:10  danner
 * 	64bit clean.  Just type-bug fixes, actually.
 * 	[92/11/30            af]
 * 
 * Revision 2.5  92/03/01  00:39:32  rpd
 * 	Fixed device_get_status argument types.
 * 	[92/02/29            rpd]
 * 
 * Revision 2.4  92/02/23  22:25:43  elf
 * 	Removed debugging printf.
 * 	[92/02/23  13:16:45  af]
 * 
 * 	Added variation of file_direct to page on raw devices.
 * 	[92/02/22  18:53:04  af]
 * 
 * 	Added remove_file_direct().  Commented out some unused code.
 * 	[92/02/19  17:31:01  af]
 * 
 * Revision 2.3  92/01/24  18:14:31  rpd
 * 	Added casts to device_read dataCnt argument to mollify
 * 	buggy versions of gcc.
 * 	[92/01/24            rpd]
 * 
 * Revision 2.2  92/01/03  19:57:13  dbg
 * 	Make file_direct self-contained: add the few fields needed from
 * 	the superblock.
 * 	[91/10/17            dbg]
 * 
 * 	Deallocate unused superblocks when switching file systems.  Add
 * 	file_close routine to free space taken by file metadata.
 * 	[91/09/25            dbg]
 * 
 * 	Move outside of kernel.
 * 	Unmodify open_file to put arrays back on stack.
 * 	[91/09/04            dbg]
 * 
 * Revision 2.8  91/08/28  11:09:42  jsb
 * 	Added struct file_direct and associated functions.
 * 	[91/08/19            rpd]
 * 
 * Revision 2.7  91/07/31  17:24:07  dbg
 * 	Call vm_wire instead of vm_pageable.
 * 	[91/07/30  16:38:01  dbg]
 * 
 * Revision 2.6  91/05/18  14:28:45  rpd
 * 	Changed block_map to avoid blocking operations
 * 	while holding an exclusive lock.
 * 	[91/04/06            rpd]
 * 	Added locking in block_map.
 * 	[91/04/03            rpd]
 * 
 * Revision 2.5  91/05/14  15:22:53  mrt
 * 	Correcting copyright
 * 
 * Revision 2.4  91/02/05  17:01:23  mrt
 * 	Changed to new copyright
 * 	[91/01/28  14:54:52  mrt]
 * 
 * Revision 2.3  90/10/25  14:41:42  rwd
 * 	Modified open_file to allocate arrays from heap, not stack.
 * 	[90/10/23            rpd]
 * 
 * Revision 2.2  90/08/27  21:45:27  dbg
 * 	Reduce lint.
 * 	[90/08/13            dbg]
 * 
 * 	Created from include files, bsd4.3-Reno (public domain) source,
 * 	and old code written at CMU.
 * 	[90/07/17            dbg]
 * 
 */
/*
 * Stand-alone file reading package.
 */

#include <device/device_types.h>
#include <device/device.h>

#include <mach/mach_traps.h>
#include <mach/mach_interface.h>

#include <file_io.h>
#include <dir.h>

void	close_file();	/* forward */

/*
 * Free file buffers, but don't close file.
 */
void
free_file_buffers(fp)
	register struct file	*fp;
{
	register int level;

	/*
	 * Free the indirect blocks
	 */
	for (level = 0; level < NIADDR; level++) {
	    if (fp->f_blk[level] != 0) {
		(void) vm_deallocate(mach_task_self(),
				     fp->f_blk[level],
				     fp->f_blksize[level]);
		fp->f_blk[level] = 0;
	    }
	    fp->f_blkno[level] = -1;
	}

	/*
	 * Free the data block
	 */
	if (fp->f_buf != 0) {
	    (void) vm_deallocate(mach_task_self(),
				 fp->f_buf,
				 fp->f_buf_size);
	    fp->f_buf = 0;
	}
	fp->f_buf_blkno = -1;
}

/*
 * Read a new inode into a file structure.
 */
int
read_inode(inumber, fp)
	ino_t			inumber;
	register struct file	*fp;
{
	vm_offset_t		buf;
	mach_msg_type_number_t	buf_size;
	register struct fs	*fs;
	daddr_t			disk_block;
	kern_return_t		rc;

	fs = fp->f_fs;
	disk_block = itod(fs, inumber);

	rc = device_read(fp->f_dev,
			 0,
			 (recnum_t) fsbtodb(fp->f_fs, disk_block),
			 (int) fs->fs_bsize,
			 (char **)&buf,
			 &buf_size);
	if (rc != KERN_SUCCESS)
	    return (rc);

	{
	    register struct dinode *dp;

	    dp = (struct dinode *)buf;
	    dp += itoo(fs, inumber);
	    fp->i_ic = dp->di_ic;
	}

	(void) vm_deallocate(mach_task_self(), buf, buf_size);

	/*
	 * Clear out the old buffers
	 */
	free_file_buffers(fp);

	return (0);	 
}

/*
 * Given an offset in a file, find the disk block number that
 * contains that block.
 */
int
block_map(fp, file_block, disk_block_p)
	struct file	*fp;
	daddr_t		file_block;
	daddr_t		*disk_block_p;	/* out */
{
	int		level;
	int		idx;
	daddr_t		ind_block_num;
	kern_return_t	rc;

	vm_offset_t	olddata[NIADDR+1];
	vm_size_t	oldsize[NIADDR+1];

	/*
	 * Index structure of an inode:
	 *
	 * i_db[0..NDADDR-1]	hold block numbers for blocks
	 *			0..NDADDR-1
	 *
	 * i_ib[0]		index block 0 is the single indirect
	 *			block
	 *			holds block numbers for blocks
	 *			NDADDR .. NDADDR + NINDIR(fs)-1
	 *
	 * i_ib[1]		index block 1 is the double indirect
	 *			block
	 *			holds block numbers for INDEX blocks
	 *			for blocks
	 *			NDADDR + NINDIR(fs) ..
	 *			NDADDR + NINDIR(fs) + NINDIR(fs)**2 - 1
	 *
	 * i_ib[2]		index block 2 is the triple indirect
	 *			block
	 *			holds block numbers for double-indirect
	 *			blocks for blocks
	 *			NDADDR + NINDIR(fs) + NINDIR(fs)**2 ..
	 *			NDADDR + NINDIR(fs) + NINDIR(fs)**2
	 *				+ NINDIR(fs)**3 - 1
	 */

	mutex_lock(&fp->f_lock);

	if (file_block < NDADDR) {
	    /* Direct block. */
	    *disk_block_p = fp->i_db[file_block];
	    mutex_unlock(&fp->f_lock);
	    return (0);
	}

	file_block -= NDADDR;

	/*
	 * nindir[0] = NINDIR
	 * nindir[1] = NINDIR**2
	 * nindir[2] = NINDIR**3
	 *	etc
	 */
	for (level = 0; level < NIADDR; level++) {
	    if (file_block < fp->f_nindir[level])
		break;
	    file_block -= fp->f_nindir[level];
	}
	if (level == NIADDR) {
	    /* Block number too high */
	    mutex_unlock(&fp->f_lock);
	    return (FS_NOT_IN_FILE);
	}

	ind_block_num = fp->i_ib[level];

	/*
	 * Initialize array of blocks to free.
	 */
	for (idx = 0; idx < NIADDR; idx++)
	    oldsize[idx] = 0;

	for (; level >= 0; level--) {

	    vm_offset_t	data;
	    mach_msg_type_number_t	size;

	    if (ind_block_num == 0)
		break;

	    if (fp->f_blkno[level] == ind_block_num) {
		/*
		 *	Cache hit.  Just pick up the data.
		 */

		data = fp->f_blk[level];
	    }
	    else {
		/*
		 *	Drop our lock while doing the read.
		 *	(The f_dev and f_fs fields don`t change.)
		 */
		mutex_unlock(&fp->f_lock);

		rc = device_read(fp->f_dev,
				0,
				(recnum_t) fsbtodb(fp->f_fs, ind_block_num),
				fp->f_fs->fs_bsize,
				(char **)&data,
				&size);
		if (rc != KERN_SUCCESS)
		    return (rc);

		/*
		 *	See if we can cache the data.  Need a write lock to
		 *	do this.  While we hold the write lock, we can`t do
		 *	*anything* which might block for memory.  Otherwise
		 *	a non-privileged thread might deadlock with the
		 *	privileged threads.  We can`t block while taking the
		 *	write lock.  Otherwise a non-privileged thread
		 *	blocked in the vm_deallocate (while holding a read
		 *	lock) will block a privileged thread.  For the same
		 *	reason, we can`t take a read lock and then use
		 *	lock_read_to_write.
		 */

		mutex_lock(&fp->f_lock);

		olddata[level] = fp->f_blk[level];
		oldsize[level] = fp->f_blksize[level];

		fp->f_blkno[level] = ind_block_num;
		fp->f_blk[level] = data;
		fp->f_blksize[level] = size;

		/*
		 *	Return to holding a read lock, and
		 *	dispose of old data.
		 */

	    }

	    if (level > 0) {
		idx = file_block / fp->f_nindir[level-1];
		file_block %= fp->f_nindir[level-1];
	    }
	    else
		idx = file_block;

	    ind_block_num = ((daddr_t *)data)[idx];
	}

	mutex_unlock(&fp->f_lock);

	/*
	 * After unlocking the file, free any blocks that
	 * we need to free.
	 */
	for (idx = 0; idx < NIADDR; idx++)
	    if (oldsize[idx] != 0)
		(void) vm_deallocate(mach_task_self(),
				     olddata[idx],
				     oldsize[idx]);

	*disk_block_p = ind_block_num;
	return (0);
}

/*
 * Read a portion of a file into an internal buffer.  Return
 * the location in the buffer and the amount in the buffer.
 */
int
buf_read_file(fp, offset, buf_p, size_p)
	register struct file	*fp;
	vm_offset_t		offset;
	vm_offset_t		*buf_p;		/* out */
	vm_size_t		*size_p;	/* out */
{
	register struct fs	*fs;
	vm_offset_t		off;
	register daddr_t	file_block;
	daddr_t			disk_block;
	int			rc;
	vm_offset_t		block_size;

	if (offset >= fp->i_size)
	    return (FS_NOT_IN_FILE);

	fs = fp->f_fs;

	off = blkoff(fs, offset);
	file_block = lblkno(fs, offset);
	block_size = blksize(fs, fp, file_block);

	if (file_block != fp->f_buf_blkno) {
	    rc = block_map(fp, file_block, &disk_block);
	    if (rc != 0)
		return (rc);

	    if (fp->f_buf)
		(void)vm_deallocate(mach_task_self(),
				    fp->f_buf,
				    fp->f_buf_size);

	    if (disk_block == 0) {
		(void)vm_allocate(mach_task_self(),
				  &fp->f_buf,
				  block_size,
				  TRUE);
		fp->f_buf_size = block_size;
	    }
	    else {
		rc = device_read(fp->f_dev,
				0,
				(recnum_t) fsbtodb(fs, disk_block),
				(int) block_size,
				(char **) &fp->f_buf,
				(mach_msg_type_number_t *)&fp->f_buf_size);
	    }
	    if (rc)
		return (rc);

	    fp->f_buf_blkno = file_block;
	}

	/*
	 * Return address of byte in buffer corresponding to
	 * offset, and size of remainder of buffer after that
	 * byte.
	 */
	*buf_p = fp->f_buf + off;
	*size_p = block_size - off;

	/*
	 * But truncate buffer at end of file.
	 */
	if (*size_p > fp->i_size - offset)
	    *size_p = fp->i_size - offset;

	return (0);
}

/*
 * Search a directory for a name and return its
 * i_number.
 */
int
search_directory(name, fp, inumber_p)
	char *		name;
	register struct file *fp;
	ino_t		*inumber_p;	/* out */
{
	vm_offset_t	buf;
	vm_size_t	buf_size;
	vm_offset_t	offset;
	register struct direct *dp;
	int		length;
	kern_return_t	rc;

	length = strlen(name);

	offset = 0;
	while (offset < fp->i_size) {
	    rc = buf_read_file(fp, offset, &buf, &buf_size);
	    if (rc != KERN_SUCCESS)
		return (rc);

	    dp = (struct direct *)buf;
	    if (dp->d_ino != 0) {
		if (dp->d_namlen == length &&
		    !strcmp(name, dp->d_name))
	    	{
		    /* found entry */
		    *inumber_p = dp->d_ino;
		    return (0);
		}
	    }
	    offset += dp->d_reclen;
	}
	return (FS_NO_ENTRY);
}

int
read_fs(dev, fsp)
	mach_port_t dev;
	struct fs **fsp;
{
	register struct fs *fs;
	vm_offset_t	buf;
	mach_msg_type_number_t	buf_size;
	int		error;

	error = device_read(dev, 0, (recnum_t) SBLOCK, SBSIZE,
			    (char **) &buf, &buf_size);
	if (error)
	    return (error);

	fs = (struct fs *)buf;
	if (fs->fs_magic != FS_MAGIC ||
	    fs->fs_bsize > MAXBSIZE ||
	    fs->fs_bsize < sizeof(struct fs)) {
		(void) vm_deallocate(mach_task_self(), buf, buf_size);
		return (FS_INVALID_FS);
	}
	/* don't read cylinder groups - we aren't modifying anything */

	*fsp = fs;
	return 0;
}

int
mount_fs(fp)
	register struct file	*fp;
{
	register struct fs *fs;
	int error;

	error = read_fs(fp->f_dev, &fp->f_fs);
	if (error)
	    return (error);
	fs = fp->f_fs;

	/*
	 * Calculate indirect block levels.
	 */
	{
	    register int	mult;
	    register int	level;

	    mult = 1;
	    for (level = 0; level < NIADDR; level++) {
		mult *= NINDIR(fs);
		fp->f_nindir[level] = mult;
	    }
	}

	return (0);
}

void
unmount_fs(fp)
	register struct file	*fp;
{
	if (file_is_structured(fp)) {
	    (void) vm_deallocate(mach_task_self(),
				 (vm_offset_t) fp->f_fs,
				 SBSIZE);
	    fp->f_fs = 0;
	}
}

/*
 * Open a file.
 */
int
open_file(master_device_port, path, fp)
	mach_port_t	master_device_port;
	char *		path;
	struct file	*fp;
{
#define	RETURN(code)	{ rc = (code); goto exit; }

	register char	*cp, *component;
	register int	c;	/* char */
	register int	rc;
	ino_t		inumber, parent_inumber;
	int		nlinks = 0;

	char	namebuf[MAXPATHLEN+1];
	
	if (path == 0 || *path == '\0') {
	    return FS_NO_ENTRY;
	}

	/*
	 * Copy name into buffer to allow modifying it.
	 */
	strcpy(namebuf, path);

	/*
	 * Look for '/dev/xxx' at start of path, for
	 * root device.
	 */
	if (!strprefix(namebuf, "/dev/")) {
	    printf("no device name\n");
	    return FS_NO_ENTRY;
	}

	cp = namebuf + 5;	/* device */
	component = cp;
	while ((c = *cp) != '\0' && c != '/') {
	    cp++;
	}
	*cp = '\0';

	rc = device_open(master_device_port,
			D_READ|D_WRITE,
			component,
			&fp->f_dev);
	if (rc)
	    return rc;

	if (c == 0) {
	    fp->f_fs = 0;
	    goto out_ok;
	}

	*cp = c;

	rc = mount_fs(fp);
	if (rc)
	    return rc;

	inumber = (ino_t) ROOTINO;
	if ((rc = read_inode(inumber, fp)) != 0) {
	    printf("can't read root inode\n");
	    goto exit;
	}

	while (*cp) {

	    /*
	     * Check that current node is a directory.
	     */
	    if ((fp->i_mode & IFMT) != IFDIR)
		RETURN (FS_NOT_DIRECTORY);

	    /*
	     * Remove extra separators
	     */
	    while (*cp == '/')
		cp++;

	    /*
	     * Get next component of path name.
	     */
	    component = cp;
	    {
		register int	len = 0;

		while ((c = *cp) != '\0' && c != '/') {
		    if (len++ > MAXNAMLEN)
			RETURN (FS_NAME_TOO_LONG);
		    if (c & 0200)
			RETURN (FS_INVALID_PARAMETER);
		    cp++;
		}
		*cp = 0;
	    }

	    /*
	     * Look up component in current directory.
	     * Save directory inumber in case we find a
	     * symbolic link.
	     */
	    parent_inumber = inumber;
	    rc = search_directory(component, fp, &inumber);
	    if (rc) {
		printf("%s: not found\n", path);
		goto exit;
	    }
	    *cp = c;

	    /*
	     * Open next component.
	     */
	    if ((rc = read_inode(inumber, fp)) != 0)
		goto exit;

	    /*
	     * Check for symbolic link.
	     */
	    if ((fp->i_mode & IFMT) == IFLNK) {

		int	link_len = fp->i_size;
		int	len;

		len = strlen(cp) + 1;

		if (link_len + len >= MAXPATHLEN - 1)
		    RETURN (FS_NAME_TOO_LONG);

		if (++nlinks > MAXSYMLINKS)
		    RETURN (FS_SYMLINK_LOOP);

		ovbcopy(cp, &namebuf[link_len], len);

#ifdef	IC_FASTLINK
		if ((fp->i_flags & IC_FASTLINK) != 0) {
		    bcopy(fp->i_symlink, namebuf, (unsigned) link_len);
		}
		else
#endif	IC_FASTLINK
		{
		    /*
		     * Read file for symbolic link
		     */
		    vm_offset_t	buf;
		    mach_msg_type_number_t	buf_size;
		    daddr_t	disk_block;
		    register struct fs *fs = fp->f_fs;

		    (void) block_map(fp, (daddr_t)0, &disk_block);
		    rc = device_read(fp->f_dev,
				     0,
				     (recnum_t) fsbtodb(fs, disk_block),
				     (int) blksize(fs, fp, 0),
				     (char **) &buf,
				     &buf_size);
		    if (rc)
			goto exit;

		    bcopy((char *)buf, namebuf, (unsigned)link_len);
		    (void) vm_deallocate(mach_task_self(), buf, buf_size);
		}

		/*
		 * If relative pathname, restart at parent directory.
		 * If absolute pathname, restart at root.
		 * If pathname begins '/dev/<device>/',
		 *	restart at root of that device.
		 */
		cp = namebuf;
		if (*cp != '/') {
		    inumber = parent_inumber;
		}
		else if (!strprefix(cp, "/dev/")) {
		    inumber = (ino_t)ROOTINO;
		}
		else {
		    cp += 5;
		    component = cp;
		    while ((c = *cp) != '\0' && c != '/') {
			cp++;
		    }
		    *cp = '\0';

		    /*
		     * Unmount current file system and free buffers.
		     */
		    close_file(fp);

		    /*
		     * Open new root device.
		     */
		    rc = device_open(master_device_port,
				D_READ,
				component,
				&fp->f_dev);
		    if (rc)
			return (rc);

		    if (c == 0) {
			fp->f_fs = 0;
			goto out_ok;
		    }

		    *cp = c;

		    rc = mount_fs(fp);
		    if (rc)
			return (rc);

		    inumber = (ino_t)ROOTINO;
		}
		if ((rc = read_inode(inumber, fp)) != 0)
		    goto exit;
	    }
	}

	/*
	 * Found terminal component.
	 */
    out_ok:
	mutex_init(&fp->f_lock);
	return 0;

	/*
	 * At error exit, close file to free storage.
	 */
    exit:
	close_file(fp);
	return rc;
}

/*
 * Close file - free all storage used.
 */
void
close_file(fp)
	register struct file	*fp;
{
	register int	i;

	/*
	 * Free the disk super-block.
	 */
	unmount_fs(fp);

	/*
	 * Free the inode and data buffers.
	 */
	free_file_buffers(fp);
}

/*
 * Copy a portion of a file into kernel memory.
 * Cross block boundaries when necessary.
 */
int
read_file(fp, offset, start, size, resid)
	register struct file	*fp;
	vm_offset_t		offset;
	vm_offset_t		start;
	vm_size_t		size;
	vm_size_t		*resid;	/* out */
{
	int			rc;
	register vm_size_t	csize;
	vm_offset_t		buf;
	vm_size_t		buf_size;

	while (size != 0) {
	    rc = buf_read_file(fp, offset, &buf, &buf_size);
	    if (rc)
		return (rc);

	    csize = size;
	    if (csize > buf_size)
		csize = buf_size;
	    if (csize == 0)
		break;

	    bcopy((char *)buf, (char *)start, csize);

	    offset += csize;
	    start  += csize;
	    size   -= csize;
	}
	if (resid)
	    *resid = size;

	return (0);
}

#if	0
/*
 * Special read and write routines for default pager.
 * Assume that all offsets and sizes are multiples
 * of DEV_BSIZE.
 */

/*
 * Read all or part of a data block, and
 * return a pointer to the appropriate part.
 * Caller must deallocate the block when done.
 */
int
page_read_file(fp, offset, size, addr, size_read)
	register struct file	*fp;
	vm_offset_t		offset;
	vm_size_t		size;
	vm_offset_t		*addr;		/* out */
	mach_msg_type_number_t	*size_read;	/* out */
{
	register struct fs	*fs;
	vm_offset_t		off;
	register daddr_t	file_block;
	daddr_t			disk_block;
	register int		rc;
	vm_size_t		block_size;

	if (offset % DEV_BSIZE != 0 ||
	    size % DEV_BSIZE != 0)
	    panic("page_read_file");

	if (offset >= fp->i_size)
	    return (FS_NOT_IN_FILE);

	fs = fp->f_fs;

	off = blkoff(fs, offset);
	file_block = lblkno(fs, offset);
	block_size = blksize(fs, fp, file_block);

	rc = block_map(fp, file_block, &disk_block);
	if (rc != 0)
	    return (rc);
	if (disk_block == 0)
	    return (FS_NOT_IN_FILE);

	if (size > block_size)
	    size = block_size;

	return (device_read(fp->f_dev,
			0,
			(recnum_t) (fsbtodb(fs, disk_block) + btodb(off)),
			(int) size,
			(char **) addr,
			size_read));
}

/*
 * Write all or part of a data block, and
 * return the amount written.
 */
int
page_write_file(fp, offset, addr, size, size_written)
	register struct file	*fp;
	vm_offset_t		offset;
	vm_offset_t		addr;
	vm_size_t		size;
	vm_offset_t		*size_written;	/* out */
{
	register struct fs	*fs;
	vm_offset_t		off;
	register daddr_t	file_block;
	daddr_t			disk_block;
	int			rc, num_written;
	vm_offset_t		block_size;

	if (offset % DEV_BSIZE != 0 ||
	    size % DEV_BSIZE != 0)
	    panic("page_write_file");

	if (offset >= fp->i_size)
	    return (FS_NOT_IN_FILE);

	fs = fp->f_fs;

	off = blkoff(fs, offset);
	file_block = lblkno(fs, offset);
	block_size = blksize(fs, fp, file_block);

	rc = block_map(fp, file_block, &disk_block);
	if (rc != 0)
	    return (rc);
	if (disk_block == 0)
	    return (FS_NOT_IN_FILE);

	if (size > block_size)
	    size = block_size;

	/*
	 * Write the data.  Wait for completion to keep
	 * reads from getting ahead of writes and reading
	 * stale data.
	 */
	rc = device_write(
			fp->f_dev,
			0,
			(recnum_t) (fsbtodb(fs, disk_block) + btodb(off)),
			(char *) addr,
			size,
			&num_written));
	*size_written = num_written;
	return rc;
}

/*
 * Wire down file buffers if caller needs wired memory.
 */
int
file_wire(master_host_port, fp)
	mach_port_t	master_host_port;
	register struct file	*fp;
{
	register int	rc;
	register int	i;

	/*
	 * The disk super-block...
	 */
	rc = vm_wire(master_host_port,
			mach_task_self(),
			(vm_offset_t) fp->f_fs,
			SBSIZE,
			VM_PROT_READ|VM_PROT_WRITE);
	if (rc != KERN_SUCCESS)
	    return (rc);

	/*
	 * The inode buffer
	 */
	if (fp->f_buf) {
	    rc = vm_wire(master_host_port,
			mach_task_self(),
			fp->f_buf,
			fp->f_buf_size,
			VM_PROT_READ|VM_PROT_WRITE);
	    if (rc != KERN_SUCCESS)
		return (rc);
	}

	/*
	 * And the data blocks.
	 */
	for (i = 0; i < NIADDR; i++) {
	    if (fp->f_blk[i]) {
		rc = vm_wire(master_host_port,
			mach_task_self(),
			fp->f_blk[i],
			fp->f_blksize[i],
			VM_PROT_READ|VM_PROT_WRITE);
		if (rc != KERN_SUCCESS)
		    return (rc);
	    }
	}
	return (0);
}
#endif	0

/* simple utility: only works for 2^n */
log2(n)
	register unsigned int n;
{
	register int i = 0;

	while ((n & 1) == 0) {
		i++;
		n >>= 1;
	}
	return i;
}

/*
 * Make an empty file_direct for a device.
 */
int
open_file_direct(dev, fdp, is_structured)
	mach_port_t	dev;
	register struct file_direct *fdp;
	boolean_t	is_structured;
{
	struct fs *fs;
	int rc;

	if (!is_structured) {
		fdp->fd_dev     = dev;
		fdp->fd_blocks  = (daddr_t *) 0;
		fdp->fd_bsize   = vm_page_size;
		fdp->fd_bshift  = log2(vm_page_size);
		fdp->fd_fsbtodb = 0;	/* later */
		fdp->fd_size    = 0;	/* later */
		return 0;
	}

	rc = read_fs(dev, &fs);
	if (rc)
		return rc;

	fdp->fd_dev = dev;
	fdp->fd_blocks = (daddr_t *) 0;
	fdp->fd_size = 0;
	fdp->fd_bsize = fs->fs_bsize;
	fdp->fd_bshift = fs->fs_bshift;
	fdp->fd_fsbtodb = fs->fs_fsbtodb;

	(void) vm_deallocate(mach_task_self(),
			     (vm_offset_t) fs,
			     SBSIZE);

	return 0;
}

/*
 * Add blocks from a file to a file_direct.
 */
int
add_file_direct(fdp, fp)
	register struct file_direct *fdp;
	register struct file *fp;
{
	register struct fs *fs;
	long num_blocks, i;
	vm_offset_t buffer;
	vm_size_t size;
	int rc;

	/* the file must be on the same device */

	if (fdp->fd_dev != fp->f_dev)
		return FS_INVALID_FS;

	if (!file_is_structured(fp)) {
		int	result[DEV_GET_SIZE_COUNT];
		natural_t count;

		count = DEV_GET_SIZE_COUNT;
		rc = device_get_status(	fdp->fd_dev, DEV_GET_SIZE,
					result, &count);
		if (rc)
			return rc;
		fdp->fd_size = result[DEV_GET_SIZE_DEVICE_SIZE] >> fdp->fd_bshift;
		fdp->fd_fsbtodb = log2(fdp->fd_bsize/result[DEV_GET_SIZE_RECORD_SIZE]);
		return 0;
	}

	/* it must hold a file system */

	fs = fp->f_fs;
	if (fdp->fd_bsize != fs->fs_bsize ||
	    fdp->fd_fsbtodb != fs->fs_fsbtodb)
		return FS_INVALID_FS;

	/* calculate number of blocks in the file, ignoring fragments */

	num_blocks = lblkno(fs, fp->i_size);

	/* allocate memory for a bigger array */

	size = (num_blocks + fdp->fd_size) * sizeof(daddr_t);
	rc = vm_allocate(mach_task_self(), &buffer, size, TRUE);
	if (rc != KERN_SUCCESS)
		return rc;

	/* lookup new block addresses */

	for (i = 0; i < num_blocks; i++) {
		daddr_t disk_block;

		rc = block_map(fp, (daddr_t) i, &disk_block);
		if (rc != 0) {
			(void) vm_deallocate(mach_task_self(), buffer, size);
			return rc;
		}

		((daddr_t *) buffer)[fdp->fd_size + i] = disk_block;
	}

	/* copy old addresses and install the new array */

	if (fdp->fd_blocks != 0) {
		bcopy((char *) fdp->fd_blocks, (char *) buffer,
		      fdp->fd_size * sizeof(daddr_t));

		(void) vm_deallocate(mach_task_self(),
				(vm_offset_t) fdp->fd_blocks,
				(vm_size_t) (fdp->fd_size * sizeof(daddr_t)));
	}
	fdp->fd_blocks = (daddr_t *) buffer;
	fdp->fd_size += num_blocks;

	/* deallocate cached blocks */

	free_file_buffers(fp);

	return 0;
}

remove_file_direct(fdp)
	struct file_direct	*fdp;
{
	if (fdp->fd_blocks)
	(void) vm_deallocate(mach_task_self(),
			     (vm_offset_t) fdp->fd_blocks,
			     (vm_size_t) (fdp->fd_size * sizeof(daddr_t)));
	fdp->fd_blocks = 0; /* sanity */
	/* xxx should lose a ref to fdp->fd_dev here (and elsewhere) xxx */
}

#if 0
/*
 * Wire down the disk address array for callers that need to use wired memory.
 */
int
file_wire_direct(master_host_port, fdp)
	mach_port_t master_host_port;
	register struct file_direct *fdp;
{
	int rc;

	/* wire the block addresses */

	rc = vm_wire(master_host_port, mach_task_self(),
		     (vm_offset_t) fdp->fd_blocks,
		     (vm_size_t) (fdp->fd_size * sizeof(daddr_t)),
		     VM_PROT_READ|VM_PROT_WRITE);
	return (rc);
}
#endif

/*
 * Special read and write routines for default pager.
 * Assume that all offsets and sizes are multiples
 * of DEV_BSIZE.
 */

#define	fdir_blkoff(fdp, offset)	/* offset % fd_bsize */ \
	((offset) & ((fdp)->fd_bsize - 1))
#define	fdir_lblkno(fdp, offset)	/* offset / fd_bsize */ \
	((offset) >> (fdp)->fd_bshift)

#define	fdir_fsbtodb(fdp, block)	/* offset * fd_bsize / DEV_BSIZE */ \
	((block) << (fdp)->fd_fsbtodb)

/*
 * Read all or part of a data block, and
 * return a pointer to the appropriate part.
 * Caller must deallocate the block when done.
 */
int
page_read_file_direct(fdp, offset, size, addr, size_read)
	register struct file_direct *fdp;
	vm_offset_t offset;
	vm_size_t size;
	vm_offset_t *addr;			/* out */
	mach_msg_type_number_t *size_read;	/* out */
{
	vm_offset_t		off;
	register daddr_t	file_block;
	daddr_t			disk_block;

	if (offset % DEV_BSIZE != 0 ||
	    size % DEV_BSIZE != 0)
	    panic("page_read_file_direct");

	if (offset >= (fdp->fd_size << fdp->fd_bshift))
	    return (FS_NOT_IN_FILE);

	off = fdir_blkoff(fdp, offset);
	file_block = fdir_lblkno(fdp, offset);

	if (file_is_device(fdp)) {
	    disk_block = file_block;
	} else {
	    disk_block = fdp->fd_blocks[file_block];
	    if (disk_block == 0)
		return (FS_NOT_IN_FILE);
	}

	if (size > fdp->fd_bsize)
	    size = fdp->fd_bsize;

	return (device_read(fdp->fd_dev,
			0,
			(recnum_t) (fdir_fsbtodb(fdp, disk_block) + btodb(off)),
			(int) size,
			(char **) addr,
			size_read));
}

/*
 * Write all or part of a data block, and
 * return the amount written.
 */
int
page_write_file_direct(fdp, offset, addr, size, size_written)
	register struct file_direct *fdp;
	vm_offset_t offset;
	vm_offset_t addr;
	vm_size_t size;
	vm_offset_t *size_written;	/* out */
{
	vm_offset_t		off;
	register daddr_t	file_block;
	daddr_t			disk_block;
	int			rc, num_written;
	vm_offset_t		block_size;

	if (offset % DEV_BSIZE != 0 ||
	    size % DEV_BSIZE != 0)
	    panic("page_write_file");

	if (offset >= (fdp->fd_size << fdp->fd_bshift))
	    return (FS_NOT_IN_FILE);

	off = fdir_blkoff(fdp, offset);
	file_block = fdir_lblkno(fdp, offset);

	if (file_is_device(fdp)) {
	    disk_block = file_block;
	} else {
	    disk_block = fdp->fd_blocks[file_block];
	    if (disk_block == 0)
		return (FS_NOT_IN_FILE);
	}

	if (size > fdp->fd_bsize)
	    size = fdp->fd_bsize;

	/*
	 * Write the data.  Wait for completion to keep
	 * reads from getting ahead of writes and reading
	 * stale data.
	 */
	rc = device_write(
			fdp->fd_dev,
			0,
			(recnum_t) (fdir_fsbtodb(fdp, disk_block) + btodb(off)),
			(char *) addr,
			size,
			&num_written);
	*size_written = num_written;
	return rc;
}

