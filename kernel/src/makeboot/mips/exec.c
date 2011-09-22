/* 
 * Mach Operating System
 * Copyright (c) 1991 Carnegie Mellon University
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
 * $Log:	exec.c,v $
 * Revision 2.3  92/04/01  19:36:09  rpd
 * 	Fixed to handle kernels with non-contiguous text and data.
 * 	[92/03/13            rpd]
 * 
 * Revision 2.2  92/01/03  20:28:52  dbg
 * 	Created.
 * 	[91/09/12            dbg]
 * 
 */

/*
 * Executable file format for mips.
 */
#include <sys/types.h>
#include <sys/file.h>
#include <sys/time.h>

#include <mips/coff.h>
#include <mips/syms.h>

#include <mach/machine/vm_types.h>
#include <mach/boot_info.h>

#define	HDR_RND_SIZE ((sizeof(struct exechdr) + SCNROUND-1) & ~(SCNROUND-1))

struct hdr {
	struct exechdr	oh;
	char		filler[HDR_RND_SIZE - sizeof(struct exechdr)];
};

off_t
exec_header_size()
{
	return sizeof(struct hdr);
}

void
write_exec_header(out_file, kp, file_size)
	int		   out_file;	/* output file */
	struct loader_info *kp;		/* kernel load info */
	off_t		   file_size;	/* size of output file */
{
	struct hdr	out_header;
	struct timeval	cur_time;

	gettimeofday(&cur_time, (struct timezone *)0);

	out_header.oh.f.f_magic    = MIPSMAGIC;
	out_header.oh.f.f_nscns    = 0;
	out_header.oh.f.f_timdat   = cur_time.tv_sec;
	out_header.oh.f.f_symptr   = 0;
	out_header.oh.f.f_nsyms    = 0;
	out_header.oh.f.f_opthdr   = sizeof(struct aouthdr);
	out_header.oh.f.f_flags	   = 7;			/* XX */

	out_header.oh.a.magic	   = NMAGIC;
	out_header.oh.a.vstamp	   = 0x20a;		/* XX */
	out_header.oh.a.tsize	   = kp->text_size;
	out_header.oh.a.dsize	   = (int) file_size - sizeof(out_header)
					- out_header.oh.a.tsize;
	out_header.oh.a.bsize	   = 0;
	out_header.oh.a.entry	   = kp->entry_1;
	out_header.oh.a.text_start = kp->text_start;
	out_header.oh.a.data_start = kp->data_start;
	out_header.oh.a.bss_start  = out_header.oh.a.data_start +
				     out_header.oh.a.dsize;
	out_header.oh.a.gprmask    = 0xfffffffe;	/* XX */
	out_header.oh.a.gp_value   = kp->entry_2;

	lseek(out_file, (off_t) 0, L_SET);
	write(out_file, (char *)&out_header, sizeof(out_header));
}

int
ex_get_header(in_file, is_kernel, lp,
		sym_header, sym_header_size)
	int	in_file;
	int	is_kernel;
	struct loader_info *lp;
	char	*sym_header;		/* OUT */
	int	*sym_header_size;	/* OUT */
{
	vm_offset_t	str_size;
	struct exechdr	x;

	lseek(in_file, (off_t) 0, L_SET);
	read(in_file, (char *)&x, sizeof(x));

	if (x.f.f_magic != MIPSMAGIC)
	    return (0);
	switch ((int)x.a.magic) {
	    case OMAGIC:
	    case NMAGIC:
	    case ZMAGIC:
		if (x.a.tsize == 0) {
		    return (0);
		}
		lp->text_start	= x.a.text_start;
		lp->text_size	= x.a.tsize;
		lp->text_offset	= N_TXTOFF(x.f,x.a);
		lp->data_start	= x.a.data_start;
		lp->data_size	= x.a.dsize;
		lp->data_offset	= lp->text_offset + lp->text_size;
		lp->bss_size	= x.a.bsize;
		break;

	    default:
		return (0);
	}
	lp->entry_1 = x.a.entry;
	lp->entry_2 = x.a.gp_value;

	lp->sym_size = x.f.f_nsyms;
	lp->sym_offset = x.f.f_symptr;
	if (lp->sym_offset == 0) {
	    lp->sym_size = 0;
	}
	else {
	    HDRR hdrr;

	    lseek(in_file, (off_t) lp->sym_offset, L_SET);
	    read(in_file, (char *)&hdrr, sizeof(hdrr));
	
	    /*
	     * We assume (!) that hdrr.cbExtOffset points to the
	     * last items in the symbol table.
	     */
	    lp->sym_size = hdrr.cbExtOffset + hdrr.iextMax * cbEXTR
			 - lp->sym_offset;
	}
	*sym_header_size = 0;

	return 1;
}

