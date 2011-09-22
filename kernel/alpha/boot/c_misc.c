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
 * $Log:	c_misc.c,v $
 * Revision 2.3  93/03/09  10:49:35  danner
 * 	Prom dispatching, protos, now can ask for file and read labels.
 * 	Which means booting from any partition. Also does a.out images.
 * 	[93/03/05            af]
 * 
 * Revision 2.2  93/02/05  08:00:26  danner
 * 	Created a while back.
 * 	[93/02/04            af]
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

#include <alpha/coff.h>
#include <alpha/exec.h>
#include <alpha/syms.h>
#include "ufs.h"
#include "min_io.h"

#include "dev.h"
#include "../../device/disk_status.h"
#include "../../scsi/rz_labels.h"


/*
 * Prom callback setup.
 */
#include "prom_routines.h"

int console;

void
init_prom_calls()
{
	struct restart_blk	*alpha_hwrpb;
	struct restart_blk	*r;
	struct console_routine_blk *c;
	vm_offset_t	addr;

	r = (struct restart_blk *)RESTART_ADDR;

	c = (struct console_routine_blk *)
		((char*)r + r->console_routine_block_offset);

	prom_dispatch_v.routine_arg = c->dispatch_func_desc;
	prom_dispatch_v.routine = c->dispatch_func_desc->code;

	/*
	 * Now look for console tty
	 */
	{
		char buf[4];
		prom_getenv( PROM_E_TTY_DEV, buf, 4);
		console = buf[0] - '0';
	}

	/*
	 * Turn on superpage
	 */
	{
		struct per_cpu_slot	*p;
		int			offset;

		offset = r->percpu_slot_size * cpu_number();
		p = (struct per_cpu_slot *) ((char*)r +
						r->percpu_slots_offset +
						offset);
		addr = enable_suppage(p->palcode_memory);
	}
}

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
		top += (size + 7) & ~7;
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

unsigned long
strlen(	register const char *str )
{
	register const char  *s = str;

	while (*str++);

	return (str - s - 1);
}

/*
 * Compare strings:  s1>s2: >0  s1==s2: 0  s1<s2: <0
 */

strcmp(s1, s2)
register const char *s1, *s2;
{
	while (*s1 == *s2++)
		if (*s1++=='\0')
			return(0);
	return(*s1 - *--s2);
}

void putnum( unsigned long int n)
{
	char	digits[24], *p = &digits[23];

	while (n) {
		*p-- = ((n & 0xf) > 9) ?
			  ((n & 0xf) + 'a'-10)
			  : ((n & 0xf) + '0');
		n >>= 4;
	}
	*p = '0';
	prom_puts(console, p, &digits[24] - p);
}


#ifdef	INTERACTIVE

int gets( unsigned char *buf )
{
	register unsigned int c;
	register unsigned long ret;
	register unsigned char *cp = buf;

	do {
		ret = prom_getc(console);
		c = ret & 0xff;
		ret >>= 61;
		if ((ret == 0) || (ret == 1)) {
			*cp = c;
			prom_puts(console, cp++, 1);
		}
	} while (c != '\r');
	*cp = '\n';
	prom_puts(console, cp, 1);
	cp[-1] = 0; /* Squash CR-LF */
	return (cp - 1) - buf;
}

#endif	/* INTERACTIVE */

/*
 *	Load and transfer control to executable file image
 */
void
aout_exec(	open_file_t	file,
		struct exec	*hdr,
		long		arg0)
{
	/* lets not kid ourselves */
	if (hdr->a_magic != OMAGIC) return;

	/* text, all there is */
	lseek( file, sizeof(*hdr), 0);
	putnum(hdr->a_text);
	if (read(file, (char *)hdr->a_tstart, hdr->a_text) != hdr->a_text)
		return;

	/* if data */
	if (hdr->a_data) {
		puts("+");
		putnum(hdr->a_data);
		if (read(file, (char *)hdr->a_dstart, hdr->a_data) != hdr->a_data)
			return;
	}

	/* if bss */
	if (hdr->a_bss) {
		puts("+");
		putnum(hdr->a_bss);
		bzero(hdr->a_dstart + hdr->a_data, hdr->a_bss);
	}

	puts(" start ");
	putnum(hdr->a_entry);
	puts("\r\n");
	(*((int (*) ()) hdr->a_entry)) (arg0);
}

void
exec(	open_file_t	file,
	long		arg0)
{
	union {
		struct exechdr		coff;
		struct exec		aout;
	} hdr;
	register struct exechdr	*h;
	register int		symsize, rsize;
	HDRR			*symptr;

	/*
	 *	Read in and check header
	 */
	if (read(file, (char *)&hdr, sizeof(hdr)) != sizeof(hdr))
		return;

	if (N_BADMAG(hdr.coff.a)) {
		aout_exec(file, &hdr.aout, arg0);
		return;
	}

	/*
	 *	Text
	 */
	lseek(file, N_TXTOFF(hdr.coff.f, hdr.coff.a), 0);
	putnum(hdr.coff.a.tsize);
	if (read(file, (char *)hdr.coff.a.text_start, hdr.coff.a.tsize) != hdr.coff.a.tsize) {
		return;
	}

	/*
	 *	Data
	 */
	puts("+");
	putnum(hdr.coff.a.dsize);
	if (read(file, (char *)hdr.coff.a.data_start, hdr.coff.a.dsize) != hdr.coff.a.dsize) {
		return;
	}

	/*
	 *	Bss
	 */
	puts("+");
	putnum(hdr.coff.a.bsize);
	bzero(hdr.coff.a.bss_start, hdr.coff.a.bsize);
	
	/*
	 *	Symbol table is loaded by cheating on data segment size
	 */

	/*
	 *	Entry and go
	 */
	puts(" start ");
	putnum(hdr.coff.a.entry);
	puts("\r\n");
	(*((int (*) ()) hdr.coff.a.entry)) (arg0);
}

/*
 *	Device interface (between filesystems and PROM)
 */

boot_device_open(dev)
	struct dev_t	*dev;
{
	unsigned char		devname[32];
	struct disklabel	*l;
	vm_size_t		resid;
	prom_return_t		ret;
	int			devlen;

	ret.bits = prom_getenv( PROM_E_BOOTED_DEV, devname, sizeof devname);
	devlen = ret.u.retval;

	ret.bits = prom_open(devname, devlen);
	if (ret.u.status)
		return -1;

	dev->handle = ret.u.retval;

	/* partitioning */
	dev->first_block = 0;
	dev->last_block = -1;

#if	USE_LABEL
	device_read(dev, 0, 512, &l, &resid);

	l = (struct disklabel*)((char*)l + LABELOFFSET);
	if (l->d_magic == DISKMAGIC) {
		register unsigned char	*cp;
		int c;

		/* Another shot of genius from the DEC folks. Now
		   the syntax of boot devices is per-system type.
		   This code assumes something like
		   "<junk> upp N <syntax qualifier>"
		   	<syntax qualifier> ::= non-blanks
			N ::= single digit
			pp ::= partition, 2 digits, we take the second
			u ::= unit number
			junk ::= anything
		 */

		cp = &devname[devlen];
		while (*(--cp) != ' ') ;
		while (*(--cp) != ' ') ;
		c = cp[-1] - '0';

		if (c < 0 || c >= l->d_npartitions)
			return 0;

		dev->first_block = l->d_partitions[c].p_offset;
		dev->last_block  = dev->first_block + l->d_partitions[c].p_size;
	}
#endif	/* USE_LABEL */
	return 0;
}

device_close(dev)
	struct dev_t	*dev;
{
	dev->first_block = -1;
	return prom_close(dev->handle);
}

device_read(dev, block, size, ptr, rsize)
	struct dev_t	*dev;
	char		**ptr;	/* out */
	vm_size_t	*rsize;	/* out */
{
	prom_return_t	ret;

	if (block < dev->first_block || block > dev->last_block)
		return FS_INVALID_PARAMETER;
	vm_allocate(0, ptr, size);
	ret.bits = prom_read( dev->handle, size, *ptr, block);
	*rsize = ret.u.retval;
	return ret.u.status;
}
