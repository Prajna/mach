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
 * $Log:	c_misc.c,v $
 * Revision 2.8  91/08/24  12:18:18  af
 * 	Set up prom callback vector appropriately, added which_prom().
 * 	Now exec passes four arguments along.
 * 	[91/08/22  11:23:41  af]
 * 
 * Revision 2.7  91/06/26  12:37:28  rpd
 * 	Fixes for people who do not have dot in their paths.
 * 
 * Revision 2.6  91/06/20  22:14:33  rvb
 * 	Scsi include files have moved, plus we use BSD label defs
 * 	from disk_status.h
 * 	[91/06/20            af]
 * 
 * Revision 2.5  91/05/14  17:17:21  mrt
 * 	Correcting copyright
 * 
 * Revision 2.4  91/02/05  17:38:35  mrt
 * 	Added author notices
 * 	[91/02/04  11:09:27  mrt]
 * 
 * 	Changed to use new Mach copyright
 * 	[91/02/02  12:04:12  mrt]
 * 
 * Revision 2.3  90/12/05  23:29:25  af
 * 	Created.
 * 	[90/12/02            af]
 * 
 */
/*
 *	File: c_misc.c
 * 	Author: Alessandro Forin, Carnegie Mellon University
 *	Date:	12/90
 *
 *	Miscellaneous C code.
 */

#include <mach/std_types.h>
#include <sys/varargs.h>

#include <mips/coff.h>
#include <mips/syms.h>
#include "ufs.h"

#include <device/device_types.h>
#include <device/disk_status.h>
#include <scsi/rz_labels.h>


/*
 * Prom callback vector.
 */
#include <asm_misc.h>

extern int	pmax_open(), pmax_lseek(), pmax_read(), pmax_close(),
		pmax_gets(), pmax_printf();
extern char	*pmax_getenv();
extern void	pmax_halt();

struct _prom_call prom_call = {
	pmax_halt,
	pmax_open,
	pmax_lseek,
	pmax_read,
	pmax_close,
	pmax_gets,
	pmax_printf,
	pmax_getenv
};

which_prom(arg0,arg1,arg2,arg3)
{
	extern int	kmin_open(), kmin_lseek(),
			kmin_read(), kmin_close(),
			kmin_gets(), kmin_printf();
	extern char	*kmin_getenv();
	extern void	kmin_halt();
	extern int	kmin_prom_base;

	if (arg2 != KMIN_PROM_MAGIC)
		return;

	kmin_prom_base = arg3;

/*	prom_call.halt = kmin_halt;*/
	prom_call.open = kmin_open;
	prom_call.lseek = kmin_lseek;
	prom_call.read = kmin_read;
	prom_call.close = kmin_close;
	prom_call.gets = kmin_gets;
	prom_call.printf = kmin_printf;
	prom_call.getenv = kmin_getenv;
}

exit(v) { prom_halt(v); }


/*
 *	Utilities
 *	Note that SIZE is of the essence here, not speed
 */
bcopy(from, to, bcount)
	register char	*from, *to;
	register	bcount;
{
	if (from < to) {
		from += bcount;
		to += bcount;
		while (bcount-- > 0)
			*--to = *--from;
	} else {
		while (bcount-- > 0)
			*to++ = *from++;
	}
}

bzero(to, bcount)
	register char	*to;
	register	bcount;
{
	while (bcount-- > 0)
		*to++ = 0;
}


/*
 *	Dynamic memory allocator
 */
struct fl {
	struct fl	*next;
	int		size;
} *freelist = 0;
extern char end[];
char *top = end;

vm_allocate(x, ptr, size)
	struct fl	**ptr;
{
	register struct fl	*f = freelist,
				*prev;

	prev = (struct fl *)&freelist;
	while (f && f->size < size){
		prev = f;
		f = f->next;
	}
	if (f == 0) {
		f = (struct fl*)top;
		top += (size + 3) & ~3;
	} else
		prev->next = f->next;
	*ptr = f;
	bzero(f, size);
	return 0;
}

vm_deallocate(x, ptr, size)
	struct fl	*ptr;
{
	ptr->size = size;
	ptr->next = freelist;
	freelist = ptr;
	return 0;
}

/*
 * Character subroutines
 */

/*
 *	Object:
 *		strlen				EXPORTED function
 *
 *		String length
 *
 */
unsigned
strlen(str)
	char *str;
{
	register char  *s = str;

	while (*str++);

	return (str - s - 1);
}

/*
 * Compare strings:  s1>s2: >0  s1==s2: 0  s1<s2: <0
 */

strcmp(s1, s2)
register char *s1, *s2;
{

	while (*s1 == *s2++)
		if (*s1++=='\0')
			return(0);
	return(*s1 - *--s2);
}

/*
 *	Minimalistic File I/O package
 */

/*
 *	Load and transfer control to executable file image
 */
void
exec(file, argc, argv, arg2, arg3)
	char	**argv;
{
	struct exechdr		hdr;
	register struct exechdr	*h;
	register int		symsize, rsize;
	HDRR			*symptr;

	/*
	 *	Read in and check header
	 */
	if ((read(file, &hdr, sizeof(hdr)) != sizeof(hdr)) ||
	    N_BADMAG(hdr.a)) {
		prom_printf("?aout?\n");
		return;
	}

	/*
	 *	Text
	 */
	lseek(file, N_TXTOFF(hdr.f, hdr.a), 0);
	prom_printf("%d", hdr.a.tsize);
	if (read(file, hdr.a.text_start, hdr.a.tsize) != hdr.a.tsize) {
		prom_printf("?txt?\n");
		return;
	}

	/*
	 *	Data
	 */
	prom_printf("+%d", hdr.a.dsize);
	if (read(file, hdr.a.data_start, hdr.a.dsize) != hdr.a.dsize) {
		prom_printf("?dat?\n");
		return;
	}

	/*
	 *	Bss
	 */
	prom_printf("+%d", hdr.a.bsize);
	bzero(hdr.a.bss_start, hdr.a.bsize);
	
	/*
	 *	Symbol table, and related info
	 */

	/* exec header will be copied right after bss */
	h = (struct exechdr *) (hdr.a.bss_start + hdr.a.bsize);
	bzero( h, sizeof(hdr));

	if (hdr.f.f_symptr > 0) {

		/* symtab header right after exec header */
		symptr = (HDRR*) (h + 1);

		lseek(file, hdr.f.f_symptr, 0);
		if (read(file, symptr, hdr.f.f_nsyms) != hdr.f.f_nsyms)
			goto badsymtab;


		/* symbol table proper after that */
		symsize = (symptr->cbExtOffset + symptr->iextMax * cbEXTR) -
			  hdr.f.f_symptr;
		rsize = symsize - hdr.f.f_nsyms;

		if (read(file, (char*)symptr + hdr.f.f_nsyms, rsize) != rsize) {
badsymtab:		prom_printf("?smt?");
			goto execit;
		}

		/* all is well */
		bcopy( &hdr, h, sizeof(hdr) );

		prom_printf("[+%d]", symsize);
	}
execit:
	prom_printf(" start x%x\n\n", hdr.a.entry);
	(*((int (*) ()) hdr.a.entry)) (argc, argv, arg2, arg3);
}

/*
 *	File primitives proper
 */
#define	NFSYS	1

struct file_ops {
	int	(*open)();
	int	(*close)();
	int	(*read)();
	int	(*write)();
	int	(*seek)();
} file_system[NFSYS] = {
	{ ufs_open, ufs_close, ufs_read, 0, ufs_seek}
};

#define NFILES 1

struct open_file {
	struct file_ops	*f_ops;
	char		*f_des;
} files[NFILES];

typedef struct open_file *open_file_t;


unsigned int
open(path, mode)
	char	*path;
{
	register int	i;
	char		*desc;

	for (i = 0; i < NFSYS; i++)
		if ((file_system[i].open)(path, mode, &desc) == 0)
			break;
	if (i == NFSYS)
		return -1;

	files[0].f_ops = &file_system[i];
	files[0].f_des = desc;
	return 0;
}

close(file)
	unsigned int	file;
{
	register open_file_t	f;

	if (file >= NFILES)
		return -1;
	f = &files[file];
	return (f->f_ops->close)(f->f_des);
}

int
read(file, dest, bcount)
	unsigned int	file;
{
	register open_file_t	f;
	int			resid = bcount;

	if (file >= NFILES)
		return -1;
	f = &files[file];
	if ((f->f_ops->read)(f->f_des, dest, bcount, &resid) &&
	    (resid == bcount))
		return -1;
	return bcount - resid;
}

int
lseek(file, offset, mode)
	unsigned int	file;
{
	register open_file_t	f;

	if (file >= NFILES)
		return -1;
	f = &files[file];
	return (f->f_ops->seek)(f->f_des, offset, mode);
}

/*
 *	Device interface (between filesystems and PROM)
 *	NOTE that PROM is busted and leaks iob[]s.
 */
#include "dev.h"

static char last_devname[256];
int prom_handle = -1;

device_open(devname, dev)
	char		*devname;
	struct dev_t	*dev;
{
	struct disklabel	*l;
	int			resid;

	/* do not open if you can, only two descs available */
	if (strcmp(devname, last_devname)) {
		prom_handle = prom_open(devname, 0);
		if (prom_handle >= 0)
			bcopy(devname, last_devname, strlen(devname) + 1);
	}
	if ((dev->handle = prom_handle) < 0)
		return -1;

	/* partitioning */
	dev->first_block = 0;
	dev->last_block = -1;
	device_read(dev, 0, DEV_BSIZE, &l, &resid);

	l = (struct disklabel*)((char*)l + LABELOFFSET);
	if (l->d_magic == DISKMAGIC) {
		register char	*cp = devname, c;

		while ((c = *cp) != ',' && c != 0) cp++;
		if (c != ',') return 0;
		cp++;
		while ((c = *cp) != ',' && c != 0) cp++;
		if (c != ',') return 0;
		c = cp[1];
		if (c < '0' || c > '9')
			return 0;
		dev->first_block = l->d_partitions[c - '0'].p_offset;
		dev->last_block  = dev->first_block + l->d_partitions[c - '0'].p_size;
	}

	return 0;
}

device_close(dev)
	struct dev_t	*dev;
{
	dev->first_block = -1;
	return( prom_close(dev->handle) );
}

device_read(dev, block, size, ptr, rsize)
	struct dev_t	*dev;
	char		**ptr;	/* out */
	int		*rsize;	/* out */
{
	if (block < dev->first_block || block > dev->last_block)
		return FS_INVALID_PARAMETER;
	vm_allocate(0, ptr, size);
	prom_lseek( dev->handle, block * DEV_BSIZE, 0);
	*rsize = prom_read( dev->handle, *ptr, size);
	return 0;
}
