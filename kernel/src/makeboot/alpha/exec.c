/* 
 * Mach Operating System
 * Copyright (c) 1993,1992 Carnegie Mellon University
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
 * Revision 2.5  93/05/10  17:48:42  rvb
 * 	Fix include to use < vs " for new ode shadowing
 * 	[93/05/10  10:40:31  rvb]
 * 
 * Revision 2.4  93/03/09  10:58:41  danner
 * 	Catched a bogus NMAGIC for a.out (quis ? quomodo ? quando ?),
 * 	as well as other peculiar little things derived from previous
 * 	a.out machines (their loaders dont look at headers, obviously).
 * 	[93/03/06            af]
 * 
 * Revision 2.3  93/01/19  09:01:41  danner
 * 	Forgot to bzero out_header to init properly when cross.
 * 	[93/01/15            af]
 * 
 * Revision 2.2  93/01/14  17:57:09  danner
 * 	Created, from mips version by dbg&rpd.
 * 	[92/05/31            af]
 * 
 */

/*
 * Executable file format for alpha.
 */
#include <cross_32_to_64.h>
#include <sys/types.h>
#include <sys/file.h>
#include <sys/time.h>

#include <alpha/coff.h>
#include <alpha/syms.h>
#include <alpha/exec.h>

#include <mach/boot_info.h>

extern int32 image_type;

/* COFF support */

#define	HDR_RND_SIZE ((sizeof(struct exechdr) + SCNROUND-1) & ~(SCNROUND-1))

struct hdr {
	struct exechdr	oh;
	char		filler[HDR_RND_SIZE - sizeof(struct exechdr)];
};

static void
write_coff_exec_header(out_file, kp, file_size)
	int32		   out_file;	/* output file */
	struct loader_info *kp;		/* kernel load info */
	off_t		   file_size;	/* size of output file */
{
	struct hdr	out_header;
	struct timeval	cur_time;

	gettimeofday(&cur_time, (struct timezone *)0);

	bzero(&out_header, sizeof out_header);

	out_header.oh.f.f_magic    = ALPHAMAGIC;
	out_header.oh.f.f_nscns    = 0;
	out_header.oh.f.f_timdat   = cur_time.tv_sec;
	out_header.oh.f.f_symptr   = zero;
	out_header.oh.f.f_nsyms    = 0;
	out_header.oh.f.f_opthdr   = sizeof(struct aouthdr);
	out_header.oh.f.f_flags	   = 7;			/* XX */

	out_header.oh.a.magic	   = OMAGIC;
	out_header.oh.a.vstamp	   = 0x600;		/* XX */
	out_header.oh.a.tsize	   = kp->text_size;
	low32(out_header.oh.a.dsize)   = (int32) file_size
					- sizeof(out_header)
					- low32(out_header.oh.a.tsize);
	out_header.oh.a.bsize	   = zero;
	out_header.oh.a.entry	   = kp->entry_1;
	out_header.oh.a.text_start = kp->text_start;
	out_header.oh.a.data_start = kp->data_start;
	out_header.oh.a.bss_start  = plus(out_header.oh.a.data_start,
				     	  out_header.oh.a.dsize);
	out_header.oh.a.gprmask    = 0xfffffffe;	/* XX */
	out_header.oh.a.gp_value   = kp->entry_2;

	lseek(out_file, (off_t) 0, L_SET);
	write(out_file, (char *)&out_header, sizeof(out_header));
}

static int32
ex_get_coff_header(in_file, is_kernel, lp,
		sym_header, sym_header_size)
	int32	in_file;
	int32	is_kernel;
	struct loader_info *lp;
	vm_size_t *sym_header;		/* OUT */
	int32	*sym_header_size;	/* OUT */
{
	struct exechdr	x;

	lseek(in_file, (off_t) 0, L_SET);
	read(in_file, (char *)&x, sizeof(x));

	if (x.f.f_magic != ALPHAMAGIC)
	    return (0);
	switch ((int32)x.a.magic) {
	    case OMAGIC:
	    case NMAGIC:
	    case ZMAGIC:
		if (neq(x.a.tsize,zero)) {
		    lp->text_start	= x.a.text_start;
		    lp->text_size	= x.a.tsize;
		    low32(lp->text_offset) = (N_TXTOFF(x.f,x.a) + 15) & ~15;
		    lp->data_start	= x.a.data_start;
		    lp->data_size	= x.a.dsize;
		    lp->data_offset	= plus(lp->text_offset,lp->text_size);
		    lp->bss_size	= x.a.bsize;
		} else {
		    return (0);
		}
		break;

	    default:
		return (0);
	}
	lp->entry_1 = x.a.entry;
	lp->entry_2 = x.a.gp_value;

	low32(lp->sym_size) = x.f.f_nsyms;
	lp->sym_offset = x.f.f_symptr;
	if (neq(lp->sym_offset,zero)) {
	    HDRR *hdrr = (HDRR *) sym_header;

	    lseek(in_file, (off_t) low32(lp->sym_offset), L_SET);
	    read(in_file, (char *)hdrr, sizeof(HDRR));
	
	    /*
	     * BUGFIX.  I have seen images where there was a
	     * hole (of 0x30 bytes, all zeroes) between the end
	     * of the HDRR and cbLineOffset.  This is fatal to
	     * the kernel, who has no notion of where the start
	     * of the file was.  Fix it here.
	     */
	    if ( neq( hdrr->cbLineOffset,
	    	      plus_a_32( lp->sym_offset, sizeof(HDRR)) )) {
		/*
		 * We fix it by removing the hole.  This of course
		 * would require fixing all the offsets in the hdrr
		 * to keep things right file-wise.  But since we do
		 * not care what happens file-wise we dont. Pun.
		 * [Someday we will want the kernel symtab at the
		 *  end of the file and we will have to cure it]
		 */
		lp->sym_offset = hdrr->cbLineOffset;
		*sym_header_size = sizeof(HDRR);
	    } else
		*sym_header_size = 0;

	    /*
	     * We assume (!) that hdrr->cbExtOffset points to the
	     * last items in the symbol table.
	     */
	    lp->sym_size = minus(
	    			plus_a_32( hdrr->cbExtOffset,
			    		   hdrr->iextMax * cbEXTR),
				lp->sym_offset);
	}
	else {
	    lp->sym_size = zero;
	    *sym_header_size = 0;
	}

	return 1;
}

/* AOUT support */

static void
write_aout_exec_header(out_file, kp, file_size)
	int32		   out_file;	/* output file */
	struct loader_info *kp;		/* kernel load info */
	off_t		   file_size;	/* size of output file */
{
	struct exec	out_header;

	low32(out_header.a_magic) = OMAGIC;
	low32(out_header.a_text)  = file_size - sizeof(struct exec);
	out_header.a_data = zero;
	out_header.a_syms = zero;
	out_header.a_bss  = zero;
	out_header.a_tstart = kp->text_start;
	out_header.a_dstart = zero/*kp->data_start*/;
	out_header.a_trsize = zero;
	out_header.a_drsize = zero;
	out_header.a_entry  = kp->entry_1;

	lseek(out_file, (off_t) 0, L_SET);
	write(out_file, (char *)&out_header, sizeof(out_header));
}

static int32
ex_get_aout_header(in_file, is_kernel, lp,
		sym_header, sym_header_size)
	int32	in_file;
	int32	is_kernel;
	struct loader_info *lp;
	char	*sym_header;		/* OUT */
	int32	*sym_header_size;	/* OUT */
{
	int32		str_size;
	struct exec	x;

	lseek(in_file, (off_t) 0, L_SET);
	read(in_file, (char *)&x, sizeof(x));

#define	LOADER_PAGE_SIZE	(8192)
#define loader_round_page(x)	((int32)((((int32)(x)) \
						+ LOADER_PAGE_SIZE - 1) \
					& ~(LOADER_PAGE_SIZE-1)))

	switch (low32(x.a_magic)) {
	    case 0407:
		lp->text_start  = x.a_tstart;
		lp->text_size   = zero;
		lp->text_offset = zero;
		lp->data_start  = x.a_dstart;
		lp->data_size	= plus(x.a_text, x.a_data);
		low32(lp->data_offset) = sizeof(struct exec);
		lp->bss_size	= x.a_bss;
		break;

	    case 0410:
		if (neq(x.a_text, zero)) {
		    lp->text_start	= x.a_tstart;
		    low32(lp->text_size)   = loader_round_page(low32(x.a_text));
		    low32(lp->text_offset) = sizeof(struct exec);
		    lp->data_start	= plus(lp->text_start, lp->text_size);
		    lp->data_size	= x.a_data;
		    lp->data_offset	= plus(lp->text_offset, x.a_text);
		    lp->bss_size	= x.a_bss;
		} else {
		    return (0);
		}
		break;

	    case 0413:
		if (neq(x.a_text, zero)) {
		    lp->text_start	= zero;
		    lp->text_size	= x.a_text;
		    low32(lp->text_offset) = 0; /* yes, header is included */
		    lp->data_start	= plus(lp->text_start, lp->text_size);
		    lp->data_size	= x.a_data;
		    lp->data_offset	= plus(lp->text_offset, lp->text_size);
		    lp->bss_size	= x.a_bss;
		} else {
		    return (0);
		}
		break;

	    default:
		return (0);
	}
	lp->entry_1 = x.a_entry;
	lp->entry_2 = zero;

	lp->sym_offset = plus(lp->data_offset, lp->data_size);

	/*
	 * Read string table size.
	 */
	lseek(in_file, (off_t) low32(plus(lp->sym_offset,x.a_syms)), L_SET);
	read(in_file, (char *)&str_size, sizeof(str_size));

	*(int32 *)sym_header = low32(x.a_syms);
#if	0
	/* this is right, but it would make all nlist structs unaligned.  Sigh */
	*sym_header_size = sizeof(int32);
#else
	*sym_header_size = sizeof(vm_size_t);
#endif

	lp->sym_size = plus_a_32(x.a_syms, str_size);

	return 1;
}

/* Interface routines */

off_t
exec_header_size()
{
	return (image_type) ? sizeof(struct hdr) : sizeof(struct exec);
}

void
write_exec_header(out_file, kp, file_size)
	int32		   out_file;	/* output file */
	struct loader_info *kp;		/* kernel load info */
	off_t		   file_size;	/* size of output file */
{
	if (image_type)
		write_coff_exec_header(out_file, kp, file_size);
	else
		write_aout_exec_header(out_file, kp, file_size);
}

int32
ex_get_header(in_file, is_kernel, lp,
		sym_header, sym_header_size)
	int32	in_file;
	int32	is_kernel;
	struct loader_info *lp;
	vm_size_t *sym_header;		/* OUT */
	int32	*sym_header_size;	/* OUT */
{
	if (ex_get_coff_header(in_file, is_kernel, lp,
			       sym_header, sym_header_size)) {
		image_type++;
		return 1;
	}
	return (ex_get_aout_header(in_file, is_kernel, lp,
			       sym_header, sym_header_size));
}
