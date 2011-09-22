/* 
 * Mach Operating System
 * Copyright (c) 1993-1989 Carnegie Mellon University
 * All Rights Reserved.
 * 
 * Permission to use, copy, modify and distribute this software and its
 * documentation is hereby granted, provided that both the copyright
 * notice and this permission notice appear in all copies of the
 * software, derivative works or modified versions, and any portions
 * thereof, and that both notices appear in supporting documentation.
 * 
 * CARNEGIE MELLON ALLOWS FREE USE OF THIS SOFTWARE IN ITS AS-IS
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
 * $Log:	kn02_dma.c,v $
 * Revision 2.8  93/05/15  19:12:33  mrt
 * 	machparam.h -> machspl.h
 * 
 * Revision 2.7  93/05/10  21:20:34  rvb
 * 	Removed a couple of duplicated functions.
 * 	Fixed interface for dma_init().
 * 	[93/04/09            af]
 * 
 * Revision 2.6  93/01/14  17:50:12  danner
 * 	New dma_init routine (from af).
 * 	[93/01/14            danner]
 * 
 * 	Proper spl typing.
 * 	[92/12/01            af]
 * 
 * Revision 2.5  92/05/05  10:46:26  danner
 * 	Use aligned copies wherever possible.
 * 	[92/04/14  12:11:54  af]
 * 
 * Revision 2.4  91/11/12  11:16:57  rvb
 * 	Added kn02_dma_end_msgin() {null function}.
 * 	[91/10/30  13:44:31  af]
 * 
 * Revision 2.3  91/10/09  16:12:48  af
 * 	Made sure it works under 2.5.
 * 
 * Revision 2.2  91/08/24  12:21:33  af
 * 	Now end_xfer routine is called for writes too.
 * 	[91/08/22  11:12:51  af]
 * 
 * 	Created.
 * 	[91/08/02  03:39:32  af]
 * 
 */
/*
 *	File: kn02_dma.c
 * 	Author: Alessandro Forin, Carnegie Mellon University
 *	Date:	7/91
 *
 *	3max pseudo-DMA functions, to handle SCSI buffer.
 *
 */

#include <asc.h>
#define NDMA NASC		/* could be max 4: system + 3 option */

#include <machine/machspl.h>		/* spl definitions */
#include <mach/std_types.h>
#include <mach/vm_param.h>		/* PGBYTES on 2.5 */

#include <scsi/compat_30.h>
#include <scsi/scsi.h>
#include <scsi/scsi_defs.h>
#include <scsi/adapters/scsi_dma.h>

#include <mips/PMAX/pmaz_aa.h>

#define		ALIGNED(x)	((((int)(x)) & 0x3) == 0)


struct dma_softc {
	int		dev_unit;
	volatile int	*dmar;		/* DMA controller register */
	volatile char	*buff;		/* DMA memory (I/O space) */
	unsigned int	callback_arg1,
			callback_arg2,
			callback_arg3;
} kn02_softc_data[NDMA];

typedef struct dma_softc *dma_softc_t;

dma_softc_t	kn02_softc[NDMA];

/*
 * Statically partition the DMA buffer between targets.
 * This way we will eventually be able to attach/detach
 * drives on-fly.  And 18k/target is plenty for normal use.
 */
#define PER_TGT_DMA_SIZE		((ASC_RAM_SIZE/7) & ~(sizeof(int)-1))
#define	read_base(tgt)			(tgt->dma_ptr)
#define write_base(tgt)			(tgt->cmd_ptr + tgt->transient_state.cmd_count)

/*
 * Round to 4k to make debug easier
 */
#define	PER_TGT_BUFF_SIZE		((PER_TGT_DMA_SIZE >> 12) << 12)
#define	PER_TGT_BURST_SIZE		(PER_TGT_BUFF_SIZE/2)
#define	INITIAL_COPYOUT_CNT		128

#define	u_min(a,b)	(((a) < (b)) ? (a) : (b))


extern scsi_dma_ops_t	kn02_dma_ops;	/* at end */

/*
 * Cold, probe time init
 */
opaque_t
kn02_dma_init(dev_unit, base, dma_bsizep, oddbp)
	int		dev_unit;
	vm_offset_t	base;
	int		*dma_bsizep;
	boolean_t	*oddbp;
{
	static int	unit = 0;
	dma_softc_t	dma;

	dma		 =
	kn02_softc[unit] = &kn02_softc_data[unit];
	unit++;
	dma->dev_unit = dev_unit;
	dma->dmar = (volatile int *) (base + ASC_OFFSET_DMAR);	
	dma->buff = (volatile char*) (base + ASC_OFFSET_RAM);

	/*
	 * Clear out dma buffer
	 */
	bzero(dma->buff, ASC_RAM_SIZE);

	return (opaque_t)dma;
}

/*
 * A target exists
 */
void
kn02_dma_new_target(dma, tgt)
	dma_softc_t	dma;
	target_info_t	*tgt;
{
	/* desc was allocated afresh */
	char *dma_ptr = (char*)dma->buff;

	/*
	 * For writes I need cmd_ptr short aligned
	 * For reads I need dma_ptr word aligned,
	 * but dma_ptr must be cmd_ptr+sizeof(group0)
	 * (buffers are word aligned)
	 */
#define	tgt_allocate_buffer(t,d)	\
		(t)->cmd_ptr = (d) + 2; \
		(t)->dma_ptr = (d) + 8;

	dma_ptr += PER_TGT_DMA_SIZE * tgt->target_id;
	tgt_allocate_buffer(tgt,dma_ptr);

#ifdef	MACH_KERNEL
#else	/*MACH_KERNEL*/
	fdma_init(&tgt->fdma, scsi_per_target_virtual);
#endif	/*MACH_KERNEL*/

}

boolean_t kn02_dma_fast_writes = TRUE;
int kn02_dma_initial_copy_count = INITIAL_COPYOUT_CNT;

/*
 * Map, if necessary, user virtual addresses into
 * physical ones (or kernel virtuals if no true DMA)
 */
void
kn02_dma_map(dma, tgt)
	dma_softc_t	dma;
	target_info_t	*tgt;
{
	/*
	 * We cannot do real DMA.
	 */
#ifdef	MACH_KERNEL
#else	/*MACH_KERNEL*/
	if (tgt->ior)
		fdma_map(&tgt->fdma, tgt->ior);
#endif	/*MACH_KERNEL*/

	if ((tgt->cur_cmd == SCSI_CMD_WRITE) ||
	    (tgt->cur_cmd == SCSI_CMD_LONG_WRITE)){
		io_req_t	ior = tgt->ior;
		register int	len = ior->io_count;
		register char	*from, *to;

		tgt->transient_state.out_count = len;

		from = ior->io_data;
		to = write_base(tgt);

		if (kn02_dma_fast_writes && (ALIGNED(from)) && (ALIGNED(to))) {
			int cnt;
			cnt = u_min(len, kn02_dma_initial_copy_count);
			tgt->transient_state.copy_count = cnt;
			aligned_block_copy(from, to, cnt);
		} else {
			if (len > PER_TGT_BUFF_SIZE)
				len = PER_TGT_BUFF_SIZE;
			bcopy(from, to, len);
			tgt->transient_state.copy_count = len;
		}

		/* avoid leaks */
		if (len < tgt->block_size) {
			bzero(to + len, tgt->block_size - len);
			len = tgt->block_size;
			tgt->transient_state.out_count = len;
		}
	} else {
		tgt->transient_state.out_count = 0;
		tgt->transient_state.copy_count = 0;
	}
	tgt->transient_state.dma_offset = 0;

}

int
kn02_dma_start_cmd(dma, tgt)
	dma_softc_t	dma;
	register target_info_t	*tgt;
{
	register int	out_count;
	/*
	 * Note that the out_count does not include the
	 * (optional) identify message but does include
	 * both the data and cmd out count.
	 */
	out_count  = tgt->transient_state.cmd_count;
	out_count += u_min(tgt->transient_state.out_count,PER_TGT_BURST_SIZE);

	*(dma->dmar) = ASC_DMAR_WRITE | ASC_DMA_ADDR(tgt->cmd_ptr);
	return out_count;
}

void
kn02_dma_end_cmd(dma, tgt, ior)
	dma_softc_t	dma;
	target_info_t	*tgt;
	io_req_t	ior;
{
#ifdef	MACH_KERNEL
#else	/*MACH_KERNEL*/
	fdma_unmap(&tgt->fdma, ior);
#endif	/*MACH_KERNEL*/
}

void
kn02_dma_end_xfer(dma, tgt, bytes_read)
	dma_softc_t	dma;
	target_info_t	*tgt;
	int		bytes_read;
{
	register char		*to, *from;

	/* nothing needed on writes */
	if (bytes_read == 0) return;

	if ((tgt->cur_cmd != SCSI_CMD_READ) &&
	    (tgt->cur_cmd != SCSI_CMD_LONG_READ)) {

		/* hba-indep code expects it there */
		to = tgt->cmd_ptr;

	} else {
		register io_req_t	ior = tgt->ior;

		assert(ior != 0);
		assert(ior->io_op & IO_READ);

		to = ior->io_data + tgt->transient_state.copy_count;
	}

	from = read_base(tgt) + tgt->transient_state.dma_offset;
	if (ALIGNED(from) && ALIGNED(to))
		aligned_block_copy(from, to, bytes_read);
	else
		bcopy(from, to, bytes_read);

}

int
kn02_dma_start_datain(dma, tgt)
	dma_softc_t	dma;
	register target_info_t	*tgt;
{
	register char	*dma_ptr;
	register int	count, avail;

	count = tgt->transient_state.in_count;
	count = u_min(count, (PER_TGT_BURST_SIZE));
	avail = PER_TGT_BUFF_SIZE - tgt->transient_state.dma_offset;
	count = u_min(count, avail);

	dma_ptr = read_base(tgt) + tgt->transient_state.dma_offset;
	(*dma->dmar) = ASC_DMA_ADDR(dma_ptr);

	return count;
}

#define	kn02_dma_restart_datain_1	kn02_dma_start_datain

int
kn02_dma_restart_datain_2(dma, tgt, xferred)
	dma_softc_t	dma;
	target_info_t	*tgt;
	int		xferred;
{
	register char	*dma_ptr;
	register int	count;

	dma->callback_arg2 = (unsigned int)tgt->transient_state.dma_offset;

	/* See comment above about OBB */
	if ((tgt->transient_state.dma_offset += xferred) & 1) {
		tgt->transient_state.dma_offset++;
	}
	count = u_min(tgt->transient_state.in_count, (PER_TGT_BURST_SIZE));
	if (tgt->transient_state.dma_offset >= PER_TGT_BUFF_SIZE) {
		tgt->transient_state.dma_offset = 0;
	} else {
		register int avail;
		avail = PER_TGT_BUFF_SIZE - tgt->transient_state.dma_offset;
		count = u_min(count, avail);
	}

	dma->callback_arg1 = (unsigned int)xferred;

	/* get some more */
	dma_ptr = read_base(tgt) + tgt->transient_state.dma_offset;
	(*dma->dmar) = ASC_DMA_ADDR(dma_ptr);
	return count;
}

void
kn02_dma_restart_datain_3(dma, tgt)
	dma_softc_t	dma;
	target_info_t	*tgt;
{
	register int	xferred = dma->callback_arg1;

	/* copy what we got */
	bcopy(	read_base(tgt) + dma->callback_arg2,
		tgt->ior->io_data + tgt->transient_state.copy_count,
		xferred);
	tgt->transient_state.copy_count += xferred;

}

int asc_fast_write_count;

boolean_t
kn02_dma_start_dataout(dma, tgt, regp, cmd)
	dma_softc_t	dma;
	register target_info_t	*tgt;
	volatile char	*regp, cmd;
{
	if ((tgt->cur_cmd == SCSI_CMD_WRITE) ||
	    (tgt->cur_cmd == SCSI_CMD_LONG_WRITE)){
		register int	could_copy, copied, copy_now;

		/*
		 * See if he have only partially loaded the first buffer.
		 * Most of the times this is the *only* buffer involved.
		 */
		could_copy = u_min(PER_TGT_BUFF_SIZE,tgt->transient_state.out_count);
		copied = tgt->transient_state.copy_count;
		copy_now = could_copy - copied;

		if (copy_now > 0) {
			register volatile char	*from, *to, c;
			spl_t			s;

			/*
			 * Start dma on the whole thing even if we have copied only
			 * a tiny bit of it.  Clearly, we cannot be distracted while
			 * doing the remaining copy.  Not even tlb misses!
			 */
	
asc_fast_write_count++;
			s = splhigh();/*honest!*/

			from = tgt->ior->io_data + copied;
			to = write_base(tgt) + copied;

			/* This knows too much, Caveat Maintainer! */

			/* Take all misses now: max three cuz buffer is 8k */
			{
			    register volatile char *p = from;
			    c = *p;
			    p = (volatile char *)mips_round_page(p);
			    if ((p - from) < copy_now) c = *p;
			    p += MIPS_PGBYTES;
			    if ((p - from) < copy_now) c = *p;
			}
			/* Miss again, if necessary, on the very first page */
			c = *from;

			/* Ready.. Get Set.. Go! */
			*regp = cmd;

			tgt->transient_state.copy_count = copied + copy_now;

			(void) aligned_block_copy( from, to, copy_now);

			splx(s);
			return FALSE;
		}
	}
	return TRUE;
}

int
kn02_dma_restart_dataout_1(dma, tgt)
	dma_softc_t	dma;
	target_info_t	*tgt;
{
	register int	count, remains;

	count = tgt->transient_state.out_count;
	count = u_min(count, (PER_TGT_BURST_SIZE));
	remains = PER_TGT_BUFF_SIZE - tgt->transient_state.dma_offset;
	count = u_min(count, remains);

	return count;
}

int
kn02_dma_restart_dataout_2(dma, tgt, xferred)
	dma_softc_t	dma;
	register target_info_t	*tgt;
	int		xferred;
{
	register int	count;

	dma->callback_arg2 = (unsigned int)tgt->transient_state.dma_offset;
	tgt->transient_state.dma_offset += xferred;
	count = u_min(tgt->transient_state.out_count, (PER_TGT_BURST_SIZE));
	if (tgt->transient_state.dma_offset >= PER_TGT_BUFF_SIZE) {
		tgt->transient_state.dma_offset = 0;
	} else {
		register int remains;
		remains = PER_TGT_BUFF_SIZE - tgt->transient_state.dma_offset;
		count = u_min(count, remains);
	}

	dma->callback_arg1 = (unsigned int)xferred;

	return count;
}

int
kn02_dma_restart_dataout_3(dma, tgt, regp)
	dma_softc_t	dma;
	target_info_t	*tgt;
	volatile char	*regp;
{
	register char	*dma_ptr;

	/* ship some more */
	dma_ptr = write_base(tgt) + tgt->transient_state.dma_offset;
	*(dma->dmar) = ASC_DMAR_WRITE | ASC_DMA_ADDR(dma_ptr);
	/*
	 * See NCR spec, Pag. 12 about 'preloading the FIFO'
	 */
	if ((int)dma_ptr & 1) {
		*regp = *dma_ptr++;
		return 1;
	} else
		return 0;
}

void
kn02_dma_restart_dataout_4(dma, tgt)
	dma_softc_t	dma;
	target_info_t	*tgt;
{
	register int	count;

	/* copy some more data */
	count = tgt->ior->io_count - tgt->transient_state.copy_count;
	if (count > 0) {
		register unsigned int	xferred = dma->callback_arg1,
					offset  = dma->callback_arg2;

		xferred = u_min(count, xferred);
		count = tgt->transient_state.copy_count;
		tgt->transient_state.copy_count = count + xferred;

		bcopy(	tgt->ior->io_data + count,
			write_base(tgt) + offset,
			xferred);
	}
}

int
kn02_dma_start_msgin(dma, tgt)
	dma_softc_t	dma;
	target_info_t	*tgt;
{
	register char	*dma_ptr;

	dma_ptr = tgt->cmd_ptr;
	(*dma->dmar) = ASC_DMA_ADDR(dma_ptr);
}

void
kn02_dma_end_msgin()
{
	/* nothing needed */
}


boolean_t
kn02_dma_disconn_1(dma, tgt, xferred)
	dma_softc_t	dma;
	target_info_t	*tgt;
	int		xferred;
{
	register int	count, offset;

	offset = tgt->transient_state.dma_offset;
	tgt->transient_state.dma_offset += xferred;
	if (tgt->transient_state.dma_offset >= PER_TGT_BUFF_SIZE)
		tgt->transient_state.dma_offset = 0;

	count = tgt->ior->io_count - tgt->transient_state.copy_count;

	if ((xferred > 0) && (count > 0)) {
		xferred = u_min(count, xferred);
		count = tgt->transient_state.copy_count;
		tgt->transient_state.copy_count = count + xferred;

		dma->callback_arg1 = (unsigned int)tgt->ior->io_data + count;
		dma->callback_arg2 = (unsigned int)write_base(tgt) + offset;
		dma->callback_arg3 = (unsigned int)xferred;
		return TRUE;
	}
	return FALSE;
}

boolean_t
kn02_dma_disconn_2(dma, tgt)
	dma_softc_t	dma;
	target_info_t	*tgt;
{
	/* check if we postponed the initial copy */
	if ((tgt->cur_cmd == SCSI_CMD_WRITE) ||
	    (tgt->cur_cmd == SCSI_CMD_LONG_WRITE)){
		register int	could_copy, copied, copy_now;

		/*
		 * See if he have only partially loaded the first buffer.
		 * Most of the times this is the *only* buffer involved.
		 */
		could_copy = u_min(PER_TGT_BUFF_SIZE,tgt->transient_state.out_count);
		copied = tgt->transient_state.copy_count;
		copy_now = could_copy - copied;

		if (copy_now > 0) {
			register volatile char	*from, *to;

			from = tgt->ior->io_data + copied;
			to = write_base(tgt) + copied;
			tgt->transient_state.copy_count = copied + copy_now;

			(void) aligned_block_copy( from, to, copy_now);
		}
	}			
	return FALSE;
}


boolean_t
kn02_dma_disconn_3(dma, tgt, xferred)
	dma_softc_t	dma;
	target_info_t	*tgt;
	int		xferred;
{
	register int	offset;

	offset = tgt->transient_state.dma_offset;

	/* See comment in dma_in_r() */
	if ((tgt->transient_state.dma_offset += xferred) & 1) {
		tgt->transient_state.dma_offset++;
	}
	if (tgt->transient_state.dma_offset >= PER_TGT_BUFF_SIZE)
		tgt->transient_state.dma_offset = 0;

	dma->callback_arg1 = (unsigned int)read_base(tgt) + offset;
	dma->callback_arg2 = (unsigned int)tgt->ior->io_data + tgt->transient_state.copy_count;
	dma->callback_arg3 = (unsigned int)xferred;

	tgt->transient_state.copy_count += xferred;
	return TRUE;
}

boolean_t
kn02_dma_disconn_4(dma, tgt, xferred)
	dma_softc_t	dma;
	target_info_t	*tgt;
	int		xferred;
{
	register int	offset;

	offset = tgt->transient_state.dma_offset;
	/* See comment in dma_in_r() */
	if ((tgt->transient_state.dma_offset += xferred) & 1) {
		tgt->transient_state.dma_offset++;
	}
	if (tgt->transient_state.dma_offset >= PER_TGT_BUFF_SIZE)
		tgt->transient_state.dma_offset = 0;

	dma->callback_arg1 = (unsigned int)read_base(tgt) + offset;
	dma->callback_arg2 = (unsigned int)tgt->ior->io_data + tgt->transient_state.copy_count;
	dma->callback_arg3 = (unsigned int)xferred;

	tgt->transient_state.copy_count += xferred;
	return TRUE;
}

#define	kn02_dma_disconn_5	kn02_dma_disconn_1

void
kn02_dma_disconn_callback(dma)
	dma_softc_t	dma;
/*	target_info_t	*tgt; */
{
	bcopy(dma->callback_arg1, dma->callback_arg2, dma->callback_arg3);
}

scsi_dma_ops_t kn02_dma_ops = {
	kn02_dma_init,
	kn02_dma_new_target,
	kn02_dma_map,
	kn02_dma_start_cmd,
	kn02_dma_end_xfer,
	kn02_dma_end_cmd,
	kn02_dma_start_datain,
	kn02_dma_start_msgin,
	kn02_dma_end_msgin,
	kn02_dma_start_dataout,
	kn02_dma_restart_datain_1,
	kn02_dma_restart_datain_2,
	kn02_dma_restart_datain_3,
	kn02_dma_restart_dataout_1,
	kn02_dma_restart_dataout_2,
	kn02_dma_restart_dataout_3,
	kn02_dma_restart_dataout_4,
	kn02_dma_disconn_1,
	kn02_dma_disconn_2,
	kn02_dma_disconn_3,
	kn02_dma_disconn_4,
	kn02_dma_disconn_5,
	kn02_dma_disconn_callback,
};

