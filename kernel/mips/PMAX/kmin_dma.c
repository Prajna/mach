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
 * $Log:	kmin_dma.c,v $
 * Revision 2.12  93/08/03  12:31:41  mrt
 * 	If we have to panic in init, use dprintf.
 * 	[93/07/29  23:29:47  af]
 * 
 * Revision 2.11  93/05/30  21:08:18  rvb
 * 	Added kn03 (3max+) support, from John.Wroclawski@lcs.mit.edu
 * 	[93/05/09            af]
 * 
 * Revision 2.10  93/05/15  19:12:21  mrt
 * 	machparam.h -> machspl.h
 * 
 * Revision 2.9  93/05/10  21:20:26  rvb
 * 	Complete rewrite.  Now it makes sense to use tapes and such.
 * 	[93/04/08            af]
 * 
 * Revision 2.8  93/03/29  15:19:56  mrt
 * 	Quick fix to nasty bug triggered by GCC: there needs
 * 	to be at least one cycle between disabling DMA and
 * 	clearing the SCSI_SCR.
 * 	There are other problems with this file, but I'll merge
 * 	later. This one was urgent: it was causing bad SCSI commands
 * 	to go out to the bus in rare occasions, trashing disks.
 * 	[93/03/27            af]
 * 
 * Revision 2.7  93/01/14  17:49:57  danner
 * 	Proper spl typing.
 * 	[92/12/01            af]
 * 
 * Revision 2.6  92/03/02  18:34:10  rpd
 * 	Added support for MAXine.
 * 	Fixed "bus reset" bug, which was due to the interaction
 * 	of the chained fifos and prefetching in case the target
 * 	disconnected right away on a write command.
 * 	Also reduced to a minimum the spurious interrupts at
 * 	the end of a transfer.
 * 	[92/03/02  02:17:05  af]
 * 
 * Revision 2.5  92/01/03  20:41:03  dbg
 * 	Zero dma_ptr at end of xfers, else disconnects get confused.
 * 	On disconnect, fix case where (a) it happened before we got
 * 	a chance to start the dma operations going and (b) we restart
 * 	dma on following reconnect [cuz dma_offset==1 hack].
 * 	[91/12/26  11:05:33  af]
 * 
 * Revision 2.4  91/12/13  14:55:11  jsb
 * 	Fixed to flush the FIFO after sending the command
 * 	out and before sending more data from a different place.
 * 	This should not have been necessary, but it is.
 * 	Also fixed read case to manually pick up all remaining
 * 	bytes in the SCR regs, just causing it to flush was both
 * 	incorrect and unreliable.
 * 	[91/12/12  17:49:00  af]
 * 
 * 	In kmin_dma_end_msgin(), brutally copy four bytes off the IOCTL
 * 	buffers, flushing is asynchronous and does not work well enough.
 * 	This seems to have put an end to the "bad SDP" printouts.
 * 	[91/11/01  10:53:32  af]
 * 
 * Revision 2.2.1.1  91/10/30  13:43:47  af
 * 	Added kmin_dma_end_msgin().
 * 
 * Revision 2.2  91/08/24  12:21:26  af
 * 	Actually got it to boot multiuser with on-board SCSI.
 * 	[91/08/22  11:11:55  af]
 * 
 * 	Created.
 * 	[91/08/02  03:39:00  af]
 * 
 */
/*
 *	File: kmin_dma.c
 * 	Author: Alessandro Forin, Carnegie Mellon University
 *	Date:	7/91
 *
 *	DMA functions for the ASIC chip (3min maxine 3max+).
 *
 */

#include <mach/std_types.h>
#include <kern/assert.h>
#include <chips/busses.h>
#include <machine/machspl.h>		/* spl definitions */
#include <device/io_req.h>
#include <device/conf.h>

#include <mips/mips_box.h>
#include <mips/mips_cpu.h>
#include <mips/PMAX/kmin.h>
#include <mips/PMAX/maxine.h>
#include <mips/PMAX/kn03.h>

#define	private static

/*
 *  We only got machines with one of these chips so far
 */
#define NASIC	1
/*
 *  Each chip handles a number of DMA channels :
 *	- lance, 1 channel, full-duplex, unbuffered
 *	- scsi, 1 channel, half-duplex, buffered
 *	- serial, 4 channels, fixed dir, unbuffered, half-page interrupt
 *	- floppy, 1 channel, full-duplex, unbuffered
 *	- isdn, 2 channels, fixed dir, buffered
 *  Maxine has them all, Kmin/3max+ miss floppy and isdn.
 *  The lance is fixed at init time, so we do not count it.
 */
#define SCSI_CHANNEL	0
#define	SER0X_CHANNEL	1
#define	SER0R_CHANNEL	2
#define	SER1X_CHANNEL	3
#define	SER1R_CHANNEL	4
#define	FLOPPY_CHANNEL	5
#define	ISDNX_CHANNEL	6
#define	ISDNR_CHANNEL	7
#define	NCHANNELS	8	

/*
 * If we ever see a machine with more than one I/O ASIC
 * these register defines should be regmaps or offsets from base.
 */
#define phystoreg(x)		((volatile unsigned int *)PHYS_TO_K1SEG(x))

static struct {
    volatile unsigned int *ssr;
    volatile unsigned int *intr;
    volatile unsigned int *lance_dmap;
    volatile unsigned int *scsi_dmap;
    volatile unsigned int *scsi_nextp;
    volatile unsigned int *scsi_scr;
    volatile unsigned int *scsi_sdr0;
    volatile unsigned int *scsi_sdr1;
} asic_regs;

#define	CSR			asic_regs.ssr
#define	INTR			asic_regs.intr
#define	LANCE_DMAPTR		asic_regs.lance_dmap
#define	SCSI_DMAPTR		asic_regs.scsi_dmap
#define	SCSI_NEXT_DMAPTR	asic_regs.scsi_nextp
#define	SCSI_SCR		asic_regs.scsi_scr
#define	SCSI_SDR0		asic_regs.scsi_sdr0
#define	SCSI_SDR1		asic_regs.scsi_sdr1

#define	DMA_ADDR(p)		(((vm_offset_t)p) << (5-2))
#define	DMAR_READ		ASIC_CSR_SCSI_DIR	/* from SCSI to memory */

/*
 * Definition of the driver for the auto-configuration program.
 */

int	 	asic_intr();
private int	asic_probe();
private void	asic_attach();

vm_offset_t asic_std[NASIC] = { 0 };
struct	bus_device *asic_info[NASIC];
struct	bus_driver asic_driver = 
        { asic_probe, 0, asic_attach, 0, asic_std, "asic", asic_info,};

/*
 * Globals
 */
typedef struct {
	vm_offset_t	buffer_pointer;
	vm_offset_t	pointer;
	int		xfer_count;
	int		direction;
#	define	DIRECTION_NOWHERE	0
#	define	DIRECTION_INTO_MEMORY	1
#	define	DIRECTION_OUT_OF_MEMORY	2
	int		error_count;
} dma_softc, *dma_softc_t;

private dma_softc kmin_dma_softc_data[NASIC * NCHANNELS];

/*
 * Adapt/Probe/Attach functions
 */
private
asic_probe(
	int		  xxx,
	struct bus_device *ui)
{
	/* we are only called if hand-filled */
	return 1;
}

private void
asic_attach(
	register struct bus_device *ui)
{
	/* we got nothing to say */
}

/*
 * Called with the BRDTYPE_XXX constant describing the current machine
 */
asic_init(
	int	brdtype)
{
	switch (brdtype) {
#if	KMIN
	case BRDTYPE_DEC5000_100:
	    asic_regs.ssr = phystoreg(KMIN_REG_CSR);
	    asic_regs.intr = phystoreg(KMIN_REG_INTR);
	    asic_regs.lance_dmap = phystoreg(KMIN_REG_LANCE_DMAPTR);
	    asic_regs.scsi_dmap = phystoreg(KMIN_REG_SCSI_DMAPTR);
	    asic_regs.scsi_nextp = phystoreg(KMIN_REG_SCSI_DMANPTR);
	    asic_regs.scsi_scr = phystoreg(KMIN_REG_SCSI_SCR);
	    asic_regs.scsi_sdr0 = phystoreg(KMIN_REG_SCSI_SDR0);
	    asic_regs.scsi_sdr1 = phystoreg(KMIN_REG_SCSI_SDR1);

	    *phystoreg(KMIN_REG_LANCE_DECODE) = KMIN_LANCE_CONFIG;
	    *phystoreg(KMIN_REG_SCSI_DECODE)  = KMIN_SCSI_CONFIG;
	    *phystoreg(KMIN_REG_SCC0_DECODE)  = KMIN_SCC0_CONFIG;
	    *phystoreg(KMIN_REG_SCC1_DECODE)  = KMIN_SCC1_CONFIG;

	    /* take all chips out of reset now */
	    *CSR = 0x00000f00;
	    break;
#endif
#if	KN03
	case BRDTYPE_DEC5000_240:
	    asic_regs.ssr = phystoreg(KN03_REG_CSR);
	    asic_regs.intr = phystoreg(KN03_REG_INTR);
	    asic_regs.lance_dmap = phystoreg(KN03_REG_LANCE_DMAPTR);
	    asic_regs.scsi_dmap = phystoreg(KN03_REG_SCSI_DMAPTR);
	    asic_regs.scsi_nextp = phystoreg(KN03_REG_SCSI_DMANPTR);
	    asic_regs.scsi_scr = phystoreg(KN03_REG_SCSI_SCR);
	    asic_regs.scsi_sdr0 = phystoreg(KN03_REG_SCSI_SDR0);
	    asic_regs.scsi_sdr1 = phystoreg(KN03_REG_SCSI_SDR1);

	    *phystoreg(KN03_REG_LANCE_DECODE) = KN03_LANCE_CONFIG;
	    *phystoreg(KN03_REG_SCSI_DECODE)  = KN03_SCSI_CONFIG;
	    *phystoreg(KN03_REG_SCC0_DECODE)  = KN03_SCC0_CONFIG;
	    *phystoreg(KN03_REG_SCC1_DECODE)  = KN03_SCC1_CONFIG;

	    /* take all chips out of reset now */
	    *CSR = 0x00000f00;
	    break;
#endif
#if	MAXINE
	case BRDTYPE_DEC5000_20:
	    asic_regs.ssr = phystoreg(XINE_REG_CSR);
	    asic_regs.intr = phystoreg(XINE_REG_INTR);
	    asic_regs.lance_dmap = phystoreg(XINE_REG_LANCE_DMAPTR);
	    asic_regs.scsi_dmap = phystoreg(XINE_REG_SCSI_DMAPTR);
	    asic_regs.scsi_nextp = phystoreg(XINE_REG_SCSI_DMANPTR);
	    asic_regs.scsi_scr = phystoreg(XINE_REG_SCSI_SCR);
	    asic_regs.scsi_sdr0 = phystoreg(XINE_REG_SCSI_SDR0);
	    asic_regs.scsi_sdr1 = phystoreg(XINE_REG_SCSI_SDR1);

	    *phystoreg(XINE_REG_LANCE_DECODE)  = XINE_LANCE_CONFIG;
	    *phystoreg(XINE_REG_SCSI_DECODE)   = XINE_SCSI_CONFIG;
	    *phystoreg(XINE_REG_SCC0_DECODE)   = XINE_SCC0_CONFIG;
	    *phystoreg(XINE_REG_DTOP_DECODE)   = XINE_DTOP_CONFIG;
	    *phystoreg(XINE_REG_FLOPPY_DECODE) = XINE_FLOPPY_CONFIG;

	    /* take all chips out of reset now */
	    *CSR = 0x00001fc1;
	    break;
#endif
	default:
	    dprintf("asic_init: unknown CPU type %x\n", brdtype);
	    halt();
	}

}

asic_enable_lance(
	vm_offset_t	phys)
{
	*LANCE_DMAPTR = (phys << 3) & 0xfff00000;

	*CSR |= ASIC_CSR_DMAEN_LANCE;
}

/*
 * Interrupt routine
 */
asic_intr(
	int	unit,
	spl_t	spllevel,
	int	intr)
{
	if (intr & ASIC_INTR_LANCE_READ_E)
		/* Just clear it and let the lance driver handle */
		*INTR = ~ASIC_INTR_LANCE_READ_E;

	else if (intr & (ASIC_INTR_SCSI_PTR_LOAD|
			 ASIC_INTR_SCSI_OVRUN	|
			 ASIC_INTR_SCSI_READ_E	))

		asic_scsi_intr(unit, spllevel, intr);

	else
		gimmeabreak();

}


/*
 *	Functions to support DMA for the on-board SCSI
 */

#include <scsi/compat_30.h>
#include <scsi/scsi.h>
#include <scsi/scsi_defs.h>
#include <scsi/adapters/scsi_dma.h>
#include <scsi/adapters/scsi_53C94.h>

/*
 * Some fields in the target structure are avail, use them.
 */
/* Indicate outgoing data does NOT follow cmd. */
#define	out_data_is_elsewhere	hba_dep[1]	/* overlaps oddb ! */
/* keep track of cache flushing */
#define cache_flushed		copy_count

/*
 * Utilities
 */

#if	1	/* DEBUG */
int		kmin_dma_debug;
#define	pr(x,y)		if (kmin_dma_debug > x) y
#define	log(a,b)	LOG(a,b)
#else
#define	pr(x,y)
#define	log(a,b)
#endif

#define	u_min(a,b)	(((a) < (b)) ? (a) : (b))

private void
asic_scsi_stop(
	register dma_softc_t	dma,
	boolean_t		flushit)
{
	*CSR &= ~ASIC_CSR_DMAEN_SCSI; wbflush();
	dma->direction = DIRECTION_NOWHERE;
	if (flushit) {
		*SCSI_SCR = 0;
		wbflush();
	}
	*INTR =	~(ASIC_INTR_SCSI_PTR_LOAD
		  | ASIC_INTR_SCSI_OVRUN
		  | ASIC_INTR_SCSI_READ_E);
	wbflush();
	log(0x22,"dma_stop");
}

asic_scsi_read(
	dma_softc_t	dma,
	vm_offset_t	dma_ptr,
	int		count)
{
	register vm_offset_t	phys, nphys;

	log(0x23,"dmaR");

	/* stop DMA engine first */
	*CSR &= ~ASIC_CSR_DMAEN_SCSI; wbflush();
	*SCSI_SCR = 0; wbflush();

	/*
	 * Deal with alignment issues, sigh.
	 */
	if (dma_ptr & 0x7) {
		log(0x3b,"UnR");
		/*
		 * Since we cannot handle at all odd bytes (grrrr!)
		 * we have to ask code higher up to backup one and
		 * re-dma in the odd byte that was previously xferred.
		 */
		pr(0,db_printf("{Ur %x}", dma_ptr));
		if (dma_ptr & 1) {
			dma_ptr--;
			count++;
		}
		/*
		 * And now.. two can play this silly game !
		 */
		if (dma_ptr & 0x4) {		/* load SCSI_SDR1 ? */
			register int *ip;
			ip = (int *) (dma_ptr & ~7);
			*SCSI_SDR0 = *ip++;
			if (dma_ptr & 2) {
				register short *sp = (short *) ip;
				*SCSI_SDR1 = *sp;
			}				
		} else if (dma_ptr & 0x2) {	/* load SCSI_SDR0 ? */
			register short *sp;
			sp = (short *) (dma_ptr & ~7);
			*SCSI_SDR0 = *sp;
		}
		wbflush();
		*SCSI_SCR = (dma_ptr & 0x7) >> 1;
		wbflush();
		dma_ptr &= ~0x7;
		/* do not touch count */
	}

	dma->pointer = dma_ptr;
	dma->xfer_count = count;
	dma->direction = DIRECTION_INTO_MEMORY;

	phys = kvtophys(dma_ptr);
	*SCSI_DMAPTR = DMA_ADDR(phys); wbflush();

	dma_ptr += MIPS_PGBYTES;
	dma_ptr = mips_trunc_page(dma_ptr);
	dma->buffer_pointer = dma_ptr;

	nphys = kvtophys(dma_ptr);
	*SCSI_NEXT_DMAPTR = DMA_ADDR(nphys); wbflush();

	*CSR |= DMAR_READ | ASIC_CSR_DMAEN_SCSI; wbflush();
}


asic_scsi_write(
	dma_softc_t	dma,
	vm_offset_t	dma_ptr,
	int		count)
{
	register vm_offset_t	phys, nphys;

	log(0x24,"dmaW");

	/* stop DMA engine first */
	*CSR &= ~ASIC_CSR_DMAEN_SCSI; wbflush();
	*SCSI_SCR = 0; wbflush();

	if (dma_ptr & 0x7) {
		register int *ip;

		pr(0,db_printf("{Uw %x}", dma_ptr));

		if (dma_ptr & 1) panic("kmin_dma_write");

		
		ip = (int *) (dma_ptr & ~7);	/* always load SDR1 */
		*SCSI_SDR1 = ip[1];

		if ((dma_ptr & 0x4) == 0)	/* load SRD0 too */
			*SCSI_SDR0 = ip[0];

		wbflush();
		*SCSI_SCR = ((dma_ptr & 0x7) >> 1) | 0x4;
		wbflush();
		dma_ptr = (dma_ptr & ~0x7) + 0x8;
		/* Aha! what if that crossed a page ? */
		if ((dma_ptr & (MIPS_PGBYTES - 1)) == 0)
			dma_ptr -= 4;	/* cause a ptr load right away */
		/* do not touch	count */
	}

	dma->pointer = dma_ptr;
	dma->xfer_count = count;
	dma->direction = DIRECTION_OUT_OF_MEMORY;

	phys = kvtophys(dma_ptr);
	*SCSI_DMAPTR = DMA_ADDR(phys); wbflush();

	dma_ptr += MIPS_PGBYTES;
	dma_ptr = mips_trunc_page(dma_ptr);
	dma->buffer_pointer = dma_ptr;

	nphys = kvtophys(dma_ptr);
	*SCSI_NEXT_DMAPTR = DMA_ADDR(nphys); wbflush();

	*CSR = (*CSR & ~DMAR_READ) | ASIC_CSR_DMAEN_SCSI; wbflush();
}

/*
 * NOTE: Cache must be flushed !  But you can do it later.
 */
private void
empty_dma_buffer(
	vm_offset_t	virt)
{
	register int		nb;
	register int		w;
#if	MACH_KERNEL
	register unsigned short	*to;
#else
	register unsigned char	*to;
	register int		togo, i;
#endif

	/*
	 * Upto 6 bytes in buffer.  Just flushing the buffer
	 * (by writing a 4 in SCSI_SCR) triggers races.
	 *
	 * NOTE: we might write one-too-many bytes (for speed).
	 *	 This is ok in 3.0 cuz no DMA to userland.
	 */
	if (nb = *SCSI_SCR) {
		/* Bypass cache in case cache line is stale */
		virt = PHYS_TO_K1SEG(kvtophys(virt));

#if	MACH_KERNEL
		/* Last byte really xferred is.. */
		to = (unsigned short *)(virt & ~7);

		/* ..we need at least two more .. */
		w = *SCSI_SDR0;
		*to++ = w;

		/* ..possibly four more.. */
		if (--nb > 0) {
			w >>= 16;
			*to++ = w;
		}

		/* .. or maybe six more bytes. */
		if (--nb > 0) {
			w = *SCSI_SDR1;
			*to++ = w;
		}
#else
		/* This is correct but slightly slower */

		to = (unsigned char *)(virt & ~7);
		togo = virt & 7;
		/* first register */
		w = *SCSI_SDR0;
		i = (togo > 4) ? 4 : togo;
		while (i--) {
			*to++ = w;
			w >>= 8;	/* bigendian */
		}
		/* second register */
		if (togo > 4) {
			w = *SCSI_SDR1;
			i = togo - 4;
			while (i--) {
				*to++ = w;
				w >>= 8;	/* bigendian */
			}
		}
#endif
	}
}

private void asic_scsi_dflush( opaque_t dma_state, target_info_t *tgt );

private void
flush_the_damn_cache(
	vm_offset_t	p,
	int		nb)
{
	vm_size_t off;

	off = p & (MIPS_PGBYTES-1);
	off = MIPS_PGBYTES - off;
	while (nb > 0) {
		off = (nb < off) ? nb : off;
		mipscache_Dflush( PHYS_TO_K0SEG(kvtophys(p)), off);
		p += off;
		nb -= off;
		off = MIPS_PGBYTES;
	}
}

/*
 * Interrupt routine
 */
asic_scsi_intr(
	int	unit,
	spl_t	spllevel,
	int	intr)
{
	register vm_offset_t	next_phys;
	register int		togo;
	register dma_softc_t	dma;

	dma = &kmin_dma_softc_data[ /*unit*/0*NASIC + SCSI_CHANNEL ];

	if (intr & ASIC_INTR_SCSI_PTR_LOAD) {

		*INTR = ~ASIC_INTR_SCSI_PTR_LOAD;
		wbflush();

		/*
		 * We started with two good pointers, we just
		 * loaded one that is worth 1 page.  How much
		 * have we done and how much is there left to go ?
		 */
		togo = dma->buffer_pointer - dma->pointer;
		dma->pointer = dma->buffer_pointer;

		togo = dma->xfer_count - togo;
		dma->buffer_pointer += MIPS_PGBYTES;

		/*
		 * We are a long way from done, get next page ready.
		 */
		if (togo > MIPS_PGBYTES) {

			next_phys = kvtophys(dma->buffer_pointer);
			*SCSI_NEXT_DMAPTR =
				next_phys ? DMA_ADDR(next_phys) : -1;
			wbflush();
			log(0x25,"dmaIa");
		}
		/*
		 * The pointer just loaded is the last good one
		 * needed to cover the xfer.  Set sentinel.
		 * Could also be an extra interrupt at end.
		 */
		else {
			/*
			 * If we are storing to main memory, make it invalid
			 * to avoid runaways.
			 */
			*SCSI_NEXT_DMAPTR = -1;
			wbflush();
			log(0x27,"dmaIc");
		}

		dma->xfer_count = togo;
	}
	/*
	 * These interrupts are unavoidable.  There is a race between
	 * the scsi chip's interrupt and the DMA engine's one at the
	 * completion of a xfer.  Apparently, the DMA engine wins
	 * most of the times.
	 */
	if (intr & (ASIC_INTR_SCSI_OVRUN|ASIC_INTR_SCSI_READ_E)) {
		*INTR = ~(ASIC_INTR_SCSI_OVRUN|ASIC_INTR_SCSI_READ_E);
		dma->error_count++;
		log(0x28,"dmaId");
	}
}


/*
 * Cold, probe time init
 */
private opaque_t
asic_scsi_init(
	int		dev_unit,
	vm_offset_t	base,
	int		*dma_bsizep,
	boolean_t	*oddbp)
{
	*SCSI_DMAPTR = -1;
	*SCSI_NEXT_DMAPTR = -1;
	*SCSI_SCR = 0;
	*oddbp = TRUE;
	return (opaque_t) &kmin_dma_softc_data[ /*dev_unit*/0*NCHANNELS
						+ SCSI_CHANNEL];
}

/*
 * A target exists
 */
private void
asic_scsi_new_target(
	opaque_t	dma_state,
	target_info_t	*tgt)
{
	extern vm_offset_t	vm_page_grab_phys_addr();
	vm_offset_t		buf;

	buf = vm_page_grab_phys_addr();
	if (buf == -1) panic("asic_scsi_new_target");

	/* make uncached */
	tgt->cmd_ptr = (char*)PHYS_TO_K1SEG(buf);
	tgt->dma_ptr = 0;

#ifdef	MACH_KERNEL
#else	/*MACH_KERNEL*/
	fdma_init(&tgt->fdma, scsi_per_target_virtual);
#endif	/*MACH_KERNEL*/

}

/*
 * Map virtual addresses onto physical ones
 * (and/or user virtuals on 2.5)
 */
private void
asic_scsi_map(
	opaque_t	dma_state,
	target_info_t	*tgt)
{
	log(0x29,"dmaM");
	/*
	 * We do real DMA.
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

		tgt->transient_state.out_count = len;

		/* avoid leaks ?? */
		if (len < tgt->block_size) {
/*			bzero(to + len, tgt->block_size - len);*/
			len = tgt->block_size;
			tgt->transient_state.out_count = len;
		}
		tgt->transient_state.out_data_is_elsewhere = TRUE;
	} else {
		tgt->transient_state.out_data_is_elsewhere = FALSE;
	}
	tgt->transient_state.dma_offset = 0;
	tgt->transient_state.cache_flushed = FALSE;
}

private int
asic_scsi_start_cmd(
	opaque_t	dma_state,
	register target_info_t	*tgt)
{
	register int	out_now, out_tot;

	log(0x2a,"dmacS");
	/*
	 * Note that the out_count does not include the
	 * (optional) identify message but does include
	 * both the cmd count and any out data after it.
	 */
	out_now = tgt->transient_state.cmd_count;
	out_tot = tgt->transient_state.out_count;
	out_tot = u_min(out_tot, ASC_TC_MAX);
	out_tot += out_now;

	/* loadup DMA engine with DMAOUT from phys of tgt->cmd_ptr; */
	asic_scsi_write( (dma_softc_t) dma_state,
			 (vm_offset_t) tgt->cmd_ptr,
			 (tgt->transient_state.out_data_is_elsewhere) ?
			    out_now : out_tot );

	return out_tot;
}

/*
 * WARNING: The DMA engine might have been
 * rescheduled to someone else (collisions).
 */
private void
asic_scsi_end_cmd(
	opaque_t	dma_state,
	target_info_t	*tgt,
	io_req_t	ior)
{
#ifdef	MACH_KERNEL
#else	/*MACH_KERNEL*/
	fdma_unmap(&tgt->fdma, ior);
#endif	/*MACH_KERNEL*/
	tgt->dma_ptr = 0;
	/* tgt->transient_state.dma_offset = 0;	dont: helps debugging */
	log(0x2b,"dmacE");
}

/*
 * End of a command that involved data xfers 
 */
private void
asic_scsi_end_xfer(
	opaque_t	dma_state,
	target_info_t	*tgt,
	int		bytes_read)
{
	log(0x2c,"xferE");

	/*
	 * Was this a read op ? if not, just stop dma engine
	 */
	if (bytes_read) {

		register volatile char	*ptr;

		ptr = tgt->dma_ptr + tgt->transient_state.dma_offset;
		empty_dma_buffer( (vm_offset_t) ptr + bytes_read );

		/* If we did not get a chance to do this before */
		if (! tgt->transient_state.cache_flushed) {
			asic_scsi_dflush( dma_state, tgt );
			assert(bytes_read <= tgt->transient_state.in_count);
		}

	}
	asic_scsi_stop( (dma_softc_t) dma_state, TRUE);

	* SCSI_DMAPTR = -1;
	* SCSI_NEXT_DMAPTR = -1;
}

private int
asic_scsi_start_datain(
	opaque_t	dma_state,
	register target_info_t	*tgt)
{
	register char	*dma_ptr;
	register int	count;

	log(0x2d,"dmaDI");
	/*
	 * Setup tgt->dma_ptr to tgt->cmd_ptr or to ior->io_data, 
	 * must flush cache (later) only in the second case since
	 * cmd_ptr is uncached.
	 */
	if (tgt->cur_cmd == SCSI_CMD_READ ||
	    tgt->cur_cmd == SCSI_CMD_LONG_READ)
		dma_ptr = tgt->ior->io_data;
	else
		/*
		 * Swear it won't disconnect midway an in-xfer or else
		 * any input data would be clobbered by the disconnect
		 * message(s)
		 */
		dma_ptr = tgt->cmd_ptr;
	tgt->dma_ptr = dma_ptr;

	assert( ((vm_offset_t)dma_ptr & 7) == 0);	/* aligned or else */

	count = tgt->transient_state.in_count;
	count = u_min(count, ASC_TC_MAX);

	asic_scsi_read( (dma_softc_t) dma_state, (vm_offset_t)dma_ptr, count);

	pr(1,db_printf("{DI%x}", count));

	return count;
}

private int
asic_scsi_restart_datain_1(
	opaque_t	dma_state,
	target_info_t	*tgt)
{
	register char	*dma_ptr;
	register int	count;

	log(0x2e,"dmaDI1");

	/* This is the case we disconnected before
	   actually ever starting the data xfer */
	if (tgt->dma_ptr == 0)
		return asic_scsi_start_datain(dma_state, tgt);

	dma_ptr = tgt->dma_ptr + tgt->transient_state.dma_offset;

	count = tgt->transient_state.in_count;
	count = u_min(count, ASC_TC_MAX);

	asic_scsi_read( (dma_softc_t)dma_state, (vm_offset_t)dma_ptr, count);

	pr(1, db_printf("{Di%x}", count));
	return count;
}

private int
asic_scsi_restart_datain_2(
	opaque_t	dma_state,
	target_info_t	*tgt,
	int		xferred)
{
	log(0x2f,"dmaDI2");

	tgt->transient_state.dma_offset += xferred;

	return asic_scsi_restart_datain_1(dma_state, tgt);
}

#define asic_scsi_restart_datain_3	asic_scsi_dflush

/* Flush Dcache in parallel with DMA xfer */
private void
asic_scsi_dflush(
	opaque_t	dma_state,
	target_info_t	*tgt)
{
	register volatile char	*ptr;

	log(0x30,"FDcache");

	if (tgt->transient_state.cache_flushed)
		return;
	tgt->transient_state.cache_flushed = TRUE;

	if (ISA_K1SEG(tgt->dma_ptr))
		return;

	flush_the_damn_cache(
		(vm_offset_t) tgt->dma_ptr,
		tgt->transient_state.dma_offset + tgt->transient_state.in_count);
}


private boolean_t
asic_scsi_start_dataout(
	opaque_t		dma_state,
	register target_info_t	*tgt,
	volatile unsigned	*regp,
	unsigned		cmd,
	unsigned char		*prefetch_count)
{
	register char	*dma_ptr;

	log(0x31,"dmaDO");
	if (tgt->transient_state.out_data_is_elsewhere) {

		*regp = 1;	/*flush fifo*/
		cmd = *regp;	/*readback*/
		delay(1);
		*prefetch_count = 0;

		dma_ptr = tgt->ior->io_data;
		/* tgt->transient_state.dma_offset = 0; done at start */

		asic_scsi_write( (dma_softc_t) dma_state,
				 (vm_offset_t) dma_ptr,
				 tgt->transient_state.out_count);

		pr(1,db_printf("{DO%x}", tgt->transient_state.out_count));
	} else {
		/* do not change the current DMA pointer, which is ok */
		dma_ptr = tgt->cmd_ptr;
		tgt->transient_state.dma_offset = tgt->transient_state.cmd_count;
	}

	tgt->dma_ptr = dma_ptr;

	return TRUE;
}

private int
asic_scsi_restart_dataout_1(
	opaque_t	dma_state,
	target_info_t	*tgt)
{
	int	count;

	log(0x32,"dmaDO1");

	/* did we disconnect before starting any xfer ?
	   this "should not happen" on commands that do dma */
	if (tgt->dma_ptr == 0) {
		tgt->dma_ptr = tgt->ior->io_data;
	}

	count = tgt->transient_state.out_count;
	count = u_min(count, ASC_TC_MAX);

	/* DMA is started later, in dataout_3 */
	return count;
}

private int
asic_scsi_restart_dataout_2(
	opaque_t	dma_state,
	register target_info_t	*tgt,
	int		xferred)
{
	log(0x33,"dmaDO2");
	tgt->transient_state.dma_offset += xferred;

	return asic_scsi_restart_dataout_1(dma_state, tgt);
}

private int
asic_scsi_restart_dataout_3(
	opaque_t	dma_state,
	target_info_t	*tgt,
	volatile unsigned *regp)
{
	register char	*dma_ptr;
	int		ret;

	log(0x34,"dmaDO3");

	dma_ptr = tgt->dma_ptr + tgt->transient_state.dma_offset;

	/*
	 * See NCR spec, Pag. 12 about 'preloading the FIFO'
	 */
	if ((int)dma_ptr & 1) {
		*regp = *dma_ptr++;
		ret = 1;
	} else
		ret = 0;

	asic_scsi_write( (dma_softc_t) dma_state,
			 (vm_offset_t) dma_ptr,
			 tgt->transient_state.out_count - ret);

	pr(1,db_printf("{D%x}", tgt->transient_state.out_count-ret));
	return ret;
}

private void
asic_scsi_restart_dataout_4(
	opaque_t	dma_state,
	target_info_t	*tgt)
{
	log(0x35,"dmaDO4");
	/* Nothing needed here */
}

private int
asic_scsi_start_msgin(
	opaque_t	dma_state,
	target_info_t	*tgt)
{
	log(0x36,"dmaMI");
	asic_scsi_read( (dma_softc_t) dma_state, (vm_offset_t) tgt->cmd_ptr, 6);
}

private void
asic_scsi_end_msgin(
	opaque_t	dma_state,
	target_info_t	*tgt)
{
	register int		*msgp;

	log(0x37,"dmaMIE");
	/*
	 * Must look at the IOCTL buffer to see if anything left
	 * Obviously yes there is stuff, else it would not be fun
	 * Note however that we expect two lonely bytes at most,
	 * so we just pick four.
	 */
	asic_scsi_stop( (dma_softc_t) dma_state, FALSE);
	if (*SCSI_SCR == 0)
		return;
	msgp = (int *)tgt->cmd_ptr;
	*msgp = *SCSI_SDR0;
	*SCSI_SCR = 0; wbflush();
}


private boolean_t
asic_scsi_disconn_1(
	opaque_t	dma_state,
	target_info_t	*tgt,
	int		xferred)
{
	log(0x38,"dmad1");

	asic_scsi_stop( (dma_softc_t) dma_state, TRUE);

	tgt->transient_state.dma_offset += xferred;
	pr(0,db_printf("{X%x}", xferred));
	return FALSE;
}

private boolean_t
asic_scsi_disconn_2(
	opaque_t	dma_state,
	target_info_t	*tgt)
{
	/* Nothing needed */
	log(0x39,"dmad2");
	return FALSE;
}


private boolean_t
asic_scsi_disconn_3(
	opaque_t	dma_state,
	target_info_t	*tgt,
	int		xferred)
{
	register volatile char	*ptr;

	log(0x3a,"dmad3");

	if (xferred > 0) {

		ptr = tgt->dma_ptr + tgt->transient_state.dma_offset;
		empty_dma_buffer( (vm_offset_t) ptr + xferred );

	}

	asic_scsi_stop( (dma_softc_t) dma_state, TRUE);

	tgt->transient_state.dma_offset += xferred;

	pr(0,db_printf("{Xi%x}", xferred));

	/*
	 * If we have not flushed the Dcache yet this is a good chance
	 * to do it in parallel with some other activity.
	 */
	return (! tgt->transient_state.cache_flushed);
}

scsi_dma_ops_t kmin_dma_ops = {
	asic_scsi_init,
	asic_scsi_new_target,
	asic_scsi_map,
	asic_scsi_start_cmd,
	asic_scsi_end_xfer,
	asic_scsi_end_cmd,
	asic_scsi_start_datain,
	asic_scsi_start_msgin,
	asic_scsi_end_msgin,
	asic_scsi_start_dataout,
	asic_scsi_restart_datain_1,
	asic_scsi_restart_datain_2,
	asic_scsi_dflush,
	asic_scsi_restart_dataout_1,
	asic_scsi_restart_dataout_2,
	asic_scsi_restart_dataout_3,
	asic_scsi_restart_dataout_4,
	asic_scsi_disconn_1,
	asic_scsi_disconn_2,
	asic_scsi_disconn_3,
	asic_scsi_disconn_3,
	asic_scsi_disconn_1,
	asic_scsi_dflush,
};


