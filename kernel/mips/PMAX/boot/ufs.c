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
 * $Log:	ufs.c,v $
 * Revision 2.6  91/06/26  12:37:37  rpd
 * 	Fixes for people who do not have dot in their paths.
 * 
 * Revision 2.5  91/05/14  17:18:56  mrt
 * 	Correcting copyright
 * 
 * Revision 2.4  91/02/05  17:39:26  mrt
 * 	Added author notices
 * 	[91/02/04  11:11:27  mrt]
 * 
 * 	Changed to use new Mach copyright
 * 	[91/02/02  12:06:45  mrt]
 * 
 * Revision 2.3  90/12/05  23:30:17  af
 * 	Created, from dbg's file I/O package.
 * 	[90/12/03            af]
 * 
 */
/*
 *	File: ufs.c
 * 	Author: David Golub, Carnegie Mellon University
 *	Date:	12/90
 *
 *	Stand-alone file reading package.
 */

#include "ufs.h"
#include "fs.h"
#include "dir.h"
#include "disk_inode.h"
#include "dev.h"

#include "asm_misc.h"	/* prom functions */

#include <device/device_types.h>

#define	ovbcopy	bcopy
#define mach_task_self()	0

/*
 * In-core open file.
 */
struct file {
	struct dev_t	f_dev;		/* device */
	unsigned int	f_seekp;	/* seek pointer */
	struct fs *	f_fs;		/* pointer to super-block */
	struct icommon	i_ic;		/* copy of on-disk inode */
	int		f_nindir[NIADDR+1];
					/* number of blocks mapped by
					   indirect block at level i */
	vm_offset_t	f_blk[NIADDR];	/* buffer for indirect block at
					   level i */
	vm_size_t	f_blksize[NIADDR];
					/* size of buffer */
	daddr_t		f_blkno[NIADDR];
					/* disk address of block in buffer */
	vm_offset_t	f_buf;		/* buffer for data block */
	vm_size_t	f_buf_size;	/* size of data block */
	daddr_t		f_buf_blkno;	/* block number of data block */
};


/*
 * Read a new inode into a file structure.
 */
int
read_inode(inumber, fp)
	ino_t			inumber;
	register struct file	*fp;
{
	vm_offset_t		buf;
	vm_size_t		buf_size;
	register struct fs	*fs;
	daddr_t			disk_block;
	kern_return_t		rc;

	fs = fp->f_fs;
	disk_block = itod(fs, inumber);

	rc = device_read(&fp->f_dev,
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
	{
	    register int level;

	    for (level = 0; level < NIADDR; level++) {
		if (fp->f_blk[level] != 0) {
		    (void) vm_deallocate(mach_task_self(),
					 fp->f_blk[level],
					 fp->f_blksize[level]);
		    fp->f_blk[level] = 0;
		}
		fp->f_blkno[level] = -1;
	    }

	    if (fp->f_buf != 0) {
		(void) vm_deallocate(mach_task_self(),
				     fp->f_buf,
				     fp->f_buf_size);
		fp->f_buf = 0;
	    }
	    fp->f_buf_blkno = -1;
	}
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
	daddr_t		*ind_p;

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

	if (file_block < NDADDR) {
	    /* Direct block. */
	    *disk_block_p = fp->i_db[file_block];
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
	    return (FS_NOT_IN_FILE);
	}

	ind_block_num = fp->i_ib[level];

	for (; level >= 0; level--) {

	    if (ind_block_num == 0) {
		*disk_block_p = 0;	/* missing */
		return (0);
	    }

	    if (fp->f_blkno[level] != ind_block_num) {
		if (fp->f_blk[level] != 0) {
		    (void) vm_deallocate(mach_task_self(),
					 fp->f_blk[level],
					 fp->f_blksize[level]);
		}
		rc = device_read(&fp->f_dev,
				(recnum_t) fsbtodb(fp->f_fs, ind_block_num),
				fp->f_fs->fs_bsize,
				(char **)&fp->f_blk[level],
				&fp->f_blksize[level]);
		if (rc != KERN_SUCCESS)
		    return (rc);
		fp->f_blkno[level] = ind_block_num;
	    }

	    ind_p = (daddr_t *)fp->f_blk[level];

	    if (level > 0) {
		idx = file_block / fp->f_nindir[level-1];
		file_block %= fp->f_nindir[level-1];
	    }
	    else
		idx = file_block;

	    ind_block_num = ind_p[idx];
	}

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
		rc = device_read(&fp->f_dev,
				(recnum_t) fsbtodb(fs, disk_block),
				(int) block_size,
				(char **) &fp->f_buf,
				&fp->f_buf_size);
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
mount_fs(fp)
	register struct file	*fp;
{
	int	error;
	register struct fs *fs;
	vm_offset_t	buf;
	vm_size_t	buf_size;

	error = device_read(&fp->f_dev, (recnum_t) SBLOCK, SBSIZE,
			   (char **) &buf, &buf_size);
	if (error) {
	    return (error);
	}

	fs = (struct fs *)buf;
	if (fs->fs_magic != FS_MAGIC ||
	    fs->fs_bsize > MAXBSIZE ||
	    fs->fs_bsize < sizeof(struct fs)) {
		(void) vm_deallocate(mach_task_self(), buf, buf_size);
		return (FS_INVALID_FS);
	}
	/* don't read cylinder groups - we aren't modifying anything */

	fp->f_fs = fs;

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

/*
 * Open a file.
 */
int
ufs_open(path, mode, descp)
	char *		path;
	struct file	**descp;
{
	register char	*cp, *ncp;
	register int	c;	/* char */
	register int	rc;
	ino_t		inumber, parent_inumber;
	int		nlinks = 0;
	struct file	*fp;

	char	namebuf[MAXPATHLEN+1];
	char	component[MAXNAMLEN+1];
	
	if (path == 0 || *path == '\0') {
	    return (FS_NO_ENTRY);
	}

	vm_allocate(mach_task_self(), &fp, sizeof(struct file));

	/*
	 * Copy name into buffer to allow modifying it.
	 */
	bcopy(path, namebuf, (unsigned)(strlen(path) + 1));

	/*
	 * Look for 'dev(x,x,x)' at start of path, for
	 * root device.  Actually, check first for the
	 * stupid "5/xxx/" like form of the new proms.
	 */
	cp = namebuf;		/* device */
	if ((c = *cp) >= '0' && c <= '9') {
	    /* new form, only safe way out is getenv(boot) */
	    char	*dev = prom_getenv("boot");
	    int	len = strlen(dev);

	    bcopy(dev, component, len);
	    cp += 2;
	    while ((c = *cp++) != '\0' && c != '/')
	    	continue;
	    if (c != '/')
	        return (FS_NO_ENTRY);
	} else {
	    ncp = component;
	    while ((c = *cp) != '\0' && c != ')') {
	        *ncp++ = c;
	        cp++;
	    }
	    if (c != ')')
	        return (FS_NO_ENTRY);
	    *ncp++ = c;
	    *ncp = 0;
	    *cp = '/';
	}

	rc = device_open(component, &fp->f_dev);
	if (rc)
	    goto out;

	rc = mount_fs(fp);
	if (rc)
	    goto out;

	inumber = (ino_t) ROOTINO;
	if ((rc = read_inode(inumber, fp)) != 0) {
	    prom_printf("can't read root inode\n");
	    goto out;
	}

	while (*cp) {

	    /*
	     * Check that current node is a directory.
	     */
	    if ((fp->i_mode & IFMT) != IFDIR) {
		rc = FS_NOT_DIRECTORY;
		goto out;
	    }

	    /*
	     * Remove extra separators
	     */
	    while (*cp == '/')
		cp++;

	    /*
	     * Get next component of path name.
	     */
	    {
		register int	len = 0;

		ncp = component;
		while ((c = *cp) != '\0' && c != '/') {
		    if (len++ > MAXNAMLEN) {
			rc = FS_NAME_TOO_LONG;
			goto out;
		    }
		    if (c & 0200) {
			rc = FS_INVALID_PARAMETER;
			goto out;
		    }

		    *ncp++ = c;
		    cp++;
		}
		*ncp = 0;
	    }

	    /*
	     * Look up component in current directory.
	     * Save directory inumber in case we find a
	     * symbolic link.
	     */
	    parent_inumber = inumber;
	    rc = search_directory(component, fp, &inumber);
	    if (rc)
		goto out;

	    /*
	     * Open next component.
	     */
	    if ((rc = read_inode(inumber, fp)) != 0)
		goto out;

	    /*
	     * Check for symbolic link.
	     */
	    if ((fp->i_mode & IFMT) == IFLNK) {

		int	link_len = fp->i_size;
		int	len;

		len = strlen(cp) + 1;

		if (link_len + len >= MAXPATHLEN - 1) {
		    rc = FS_NAME_TOO_LONG;
		    goto out;
		}

		if (++nlinks > MAXSYMLINKS) {
		    rc = FS_SYMLINK_LOOP;
		    goto out;
		}

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
		    vm_size_t	buf_size;
		    daddr_t	disk_block;
		    register struct fs *fs = fp->f_fs;

		    (void) block_map(fp, (daddr_t)0, &disk_block);
		    rc = device_read(&fp->f_dev,
				     (recnum_t) fsbtodb(fs, disk_block),
				     (int) blksize(fs, fp, 0),
				     (char **) &buf,
				     &buf_size);
		    if (rc)
			goto out;

		    bcopy((char *)buf, namebuf, (unsigned)link_len);
		    (void) vm_deallocate(mach_task_self(), buf, buf_size);
		}

		/*
		 * If relative pathname, restart at parent directory.
		 * If absolute pathname, restart at root.
		 */
		cp = namebuf;
		if (*cp != '/') {
		    inumber = parent_inumber;
		}
		else
		    inumber = (ino_t)ROOTINO;

		if ((rc = read_inode(inumber, fp)) != 0)
		    goto out;
	    }
	}

	/*
	 * Found terminal component.
	 */

	rc = 0;
	*descp = fp;
out:
	if (rc)
		vm_deallocate(mach_task_self(), fp, sizeof(struct file));
	return (rc);
}

/*
 * Copy a portion of a file into kernel memory.
 * Cross block boundaries when necessary.
 */
int
ufs_read(fp, start, size, resid)
	register struct file	*fp;
	vm_offset_t		start;
	vm_size_t		size;
	vm_size_t		*resid;	/* out */
{
	int			rc;
	register vm_size_t	csize;
	vm_offset_t		buf;
	vm_size_t		buf_size;
	vm_offset_t		offset = fp->f_seekp;

	while (size != 0) {
	    rc = buf_read_file(fp, offset, &buf, &buf_size);
	    if (rc)
		goto out;

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

out:
	fp->f_seekp = offset;
	return (rc);
}


ufs_close(fp)
	register struct file	*fp;
{
	device_close(&fp->f_dev);
	vm_deallocate( mach_task_self(), fp, sizeof(struct file));
	return 0;
}

ufs_seek(fp, offset, mode)
	register struct file	*fp;
{
	switch (mode) {
	case 0:
		fp->f_seekp = offset;
		break;
	case 1:
		fp->f_seekp += offset;
		break;
	default:
		return FS_INVALID_PARAMETER;
	}
	return 0;
}

