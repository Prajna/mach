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
 * $Log:	scsi_ADU_hdw.c,v $
 * Revision 2.5  93/05/15  19:10:10  mrt
 * 	machparam.h -> machspl.h
 * 
 * Revision 2.4  93/03/26  17:54:56  mrt
 * 	Fixed some decls.
 * 	[93/03/23            af]
 * 
 * Revision 2.3  93/01/19  09:00:39  danner
 * 	tbis does not fly, something broken cuz it should.
 * 	[93/01/16  12:22:27  af]
 * 
 * Revision 2.2  93/01/14  17:10:37  danner
 * 	Cleanup a bit: avoid copies when possible, support more
 * 	than one target, argue that it should be MP safe.
 * 	Still have to beat on it for real.
 * 	[92/12/25            af]
 * 
 * 	Created, from the DEC specs:
 * 	"Alpha Demonstration Unit Specification"
 * 	V1.0, Aug 1990.
 * 	[92/06/02            af]
 * 	Created.
 * 	[92/12/10  15:26:16  af]
 * 
 */
/*
 *	File: scsi_ADU_hdw.c
 * 	Author: Alessandro Forin, Carnegie Mellon University
 *	Date:	7/92
 *
 *	Bottom layer of the SCSI driver: chip-dependent functions
 *
 *	This file contains the code that is specific to the DEC
 *	ADU's I/O module SCSI interface.
 */

#include <cpus.h>

#include <asz.h>
#if	NASZ > 0

#include <mach/std_types.h>
#include <kern/kalloc.h>
#include <machine/machspl.h>		/* spls */

#include <sys/types.h>
#include <chips/busses.h>
#include <scsi/compat_30.h>

#include <sys/syslog.h>

#include <scsi/scsi.h>
#include <scsi/scsi2.h>
#include <scsi/scsi_defs.h>

/* This is a very specific interface, but there is no reason why it
   might not be used (as an example) somewhere else */

#include <alpha/DEC/scsi_ADU.h>

#include <platforms.h>

#ifdef	ADU
#include <alpha/alpha_cpu.h>
#include <alpha/DEC/tvbus.h>
#define	phystokvc	PHYS_TO_K0SEG
#ifdef	cpu_number	/* sanity */
#undef	cpu_number
#endif
#endif	/* ADU */

#ifndef	phystokvc
#define	phystokvc	phystokv
#endif

#define	MARK_XCOPIED(r)		(r)->cmdtag |= 1
#define	ISA_XCOPIED(r)		((r)->cmdtag & 1)
#define	UNMARK_XCOPIED(r)	((r)->cmdtag & ~1)

int adu_sz_ncop, adu_sz_nncop;

/*
 * State descriptor for this layer.  There is one such structure
 * per (enabled) board
 */
struct asz_softc {
	decl_simple_lock_data(, lock)
	scsi_softc_t	*sc;		/* HBA-indep info */

	sz_ringbuffer_t		*rb;
	volatile unsigned long	*sz_br;
	volatile unsigned long	*sz_icr;
	volatile unsigned long	*sz_db;

	int		ntargets;	/* how many alive on this scsibus */
	int		nxt_out;	/* next in cmd ring */
	int		nxt_in;		/* next in msg ring */
	int		my_scsi_id;

} asz_softc_data[NASZ];

typedef struct asz_softc *asz_softc_t;

asz_softc_t	asz_softc[NASZ];

/*
 * Definition of the controller for the auto-configuration program.
 */

int	asz_probe(), scsi_slave(), asz_go(), asz_intr();
void	scsi_attach();

vm_offset_t	asz_std[NASZ] = { 0 };
struct	bus_device *asz_dinfo[NASZ*8];
struct	bus_ctlr *asz_minfo[NASZ];
struct	bus_driver asz_driver = 
        { asz_probe, scsi_slave, scsi_attach, asz_go, asz_std, "rz", asz_dinfo,
	  "asz", asz_minfo, BUS_INTR_B4_PROBE};

asz_set_scsiid(
	int	unit,
	int	id)
{
	if ((unit < NASZ) && asz_softc[unit])
		asz_softc[unit]->my_scsi_id = id;
}

/*
 *	Probe/Slave/Attach functions
 */
boolean_t	asz_probe_target();

/*
 * Probe routine:
 *	Should find out (a) if the controller is
 *	present and (b) which/where slaves are present.
 *
 * Implementation:
 *	We never get here if the controller is not present.
 *	We delay the looking for slaves for later, when the
 *	VM system is up and running.
 */
asz_probe(
	vm_offset_t	base,
	struct bus_ctlr	*ui)
{
	asz_softc_t		asz;
	scsi_softc_t		*sc;
	sz_ringbuffer_t		*rb;
	vm_offset_t		addr;
	int			i;
	extern vm_offset_t	kvtophys();

	i = ui->unit;
	if (i >= NASZ) return 0;

	asz = &asz_softc_data[i];
	asz_softc[i] = asz;

	sc = scsi_master_alloc(i, asz);
	asz->sc = sc;

	simple_lock_init(&asz->lock);
	sc->go = asz_go;
	sc->probe = asz_probe_target;
	/* board handles timeouts */
	sc->max_dma_data = SZ_MAX_DMA;

	if (asz->my_scsi_id == 0) asz->my_scsi_id = 7;
	sc->initiator_id = asz->my_scsi_id;

	/*
	 * Grab a page for the ring. Cannot kalloc or anything yet.
	 */
	{
		vm_page_t	m = vm_page_grab();
		addr = m->phys_addr;
	}
	rb = (sz_ringbuffer_t *) (phystokvc(addr));		/* kseg0 */
	bzero(rb, PAGE_SIZE);
	asz->rb = rb;

	/* rings inited at zero is fine */

	asz->sz_br  = (volatile unsigned long*)(base + TV_IO_SCSI_BASE);
	asz->sz_icr = (volatile unsigned long*)(base + TV_IO_SCSI_ICR);
	asz->sz_db  = (volatile unsigned long*)(base + TV_IO_SCSI_S_BELL);

	*asz->sz_br = addr;
	wbflush();

#ifdef	ADU
	/* Interrupts */
	{
		int	cpu_no, chan;

		cpu_no = cpu_number();
		chan = tv_get_channel(cpu_no, asz_intr, asz);
		*asz->sz_icr =	(cpu_no << 1) & SZ_ICR_IRQ_NODE |
				(chan   << 5) & SZ_ICR_IRQ_CHAN |
				SZ_ICR_IE;
		wbflush();
	}
#endif
	printf("%s%d: SCSI id %d [will probe targets on demand]\n",
		ui->name, ui->unit, sc->initiator_id);

	return 1;
}

/* Dynprobe */
boolean_t
asz_probe_target(
	target_info_t		*tgt,
	io_req_t		ior)
{
	asz_softc_t     asz = asz_softc[tgt->masterno];
	boolean_t	newlywed;

	newlywed = (tgt->cmd_ptr == 0);
	if (newlywed) {
		vm_offset_t	buf;
		kern_return_t	ret;
		/* this array sized maximally, if we run with
		   page_size > hardware_page_size we need less */
		vm_page_t	pages[SZ_MAX_DMA / ALPHA_PGBYTES];

		ret = vm_page_grab_contiguous_pages( SZ_MAX_DMA / PAGE_SIZE,
						     pages,
						     0);

		if (ret == KERN_RESOURCE_SHORTAGE) {
			printf("asz_probe_target: Not enough contiguous memory\n");
			return FALSE;
		}
		/* pages are sorted */
		tgt->dma_ptr = (char *) pages[0]->phys_addr;

		/* we too might need to read/write the buffer */
		tgt->dma_ptr = (char *) phystokvc(tgt->dma_ptr); /* k0seg */

		buf = kalloc(512);
#ifdef	ADU
		buf = phystokvc(kvtophys(buf));
#endif
		tgt->cmd_ptr = (char*)buf;

	}

	if (scsi_inquiry(tgt, SCSI_INQ_STD_DATA) == SCSI_RET_DEVICE_DOWN)
		return FALSE;

	tgt->flags = TGT_ALIVE;
	return TRUE;
}


/*
 * Start a SCSI command on a target
 */
asz_go(
	target_info_t		*tgt,
	int			cmd_count,
	int			in_count,
	boolean_t		cmd_only)
{
	asz_softc_t		asz;
	spl_t			s;
	sz_cmd_ring_t		*ring;
	int			len, i;
	vm_offset_t		virt, phys;

	asz = (asz_softc_t)tgt->hw_state;

	/* get a cmd desc */
	simple_lock(&asz->lock);
	i = asz->nxt_out;
	asz->nxt_out = (i == 31) ? 0 : i + 1;
	simple_unlock(&asz->lock);

	ring = &(asz->rb->cmds[i]);
	ring->cmdtag = (natural_t) tgt;
	ring->target = tgt->target_id;
	if (cmd_only) {
		ring->nmsg = 0;
		ring->msg = 0;
	} else {
		ring->nmsg = 1;
		ring->msg = SCSI_IDENTIFY|SCSI_IFY_ENABLE_DISCONNECT;
	}

	/*
	 * We can do real DMA.
	 */
	ring->dataaddr = kvtophys((vm_offset_t)tgt->dma_ptr);

/*	tgt->transient_state.copy_count = 0;	unused */
/*	tgt->transient_state.dma_offset = 0;	unused */

	tgt->transient_state.cmd_count = cmd_count;

	if ((tgt->cur_cmd == SCSI_CMD_WRITE) ||
	    (tgt->cur_cmd == SCSI_CMD_LONG_WRITE)){
		io_req_t	ior = tgt->ior;
		register int	len = ior->io_count;

		tgt->transient_state.out_count = len;

		/* Avoid leaks	 */
		if (len < tgt->block_size) {
			char *to = tgt->dma_ptr + len;
			bzero(to + len, tgt->block_size - len);
			len = tgt->block_size;
			tgt->transient_state.out_count = len;
		}
	} else {
		tgt->transient_state.out_count = 0;
	}

	/* See above for in_count < block_size */
	tgt->transient_state.in_count = in_count;

	/*
	 * Setup CCB state
	 */
	tgt->done = SCSI_RET_IN_PROGRESS;

	switch (tgt->cur_cmd) {
	    case SCSI_CMD_READ:
	    case SCSI_CMD_LONG_READ:
		virt = (vm_offset_t)tgt->ior->io_data;
		len = tgt->transient_state.in_count;
		break;
	    case SCSI_CMD_WRITE:
	    case SCSI_CMD_LONG_WRITE:
		virt = (vm_offset_t)tgt->ior->io_data;
		len = tgt->transient_state.out_count;
		break;
	    case SCSI_CMD_INQUIRY:
	    case SCSI_CMD_REQUEST_SENSE:
	    case SCSI_CMD_MODE_SENSE:
	    case SCSI_CMD_RECEIVE_DIAG_RESULTS:
	    case SCSI_CMD_READ_CAPACITY:
	    case SCSI_CMD_READ_BLOCK_LIMITS:
		virt = (vm_offset_t)tgt->cmd_ptr;
		len = tgt->transient_state.in_count;
		break;
	    case SCSI_CMD_MODE_SELECT:
	    case SCSI_CMD_REASSIGN_BLOCKS:
	    case SCSI_CMD_FORMAT_UNIT:
		tgt->transient_state.cmd_count = sizeof(scsi_command_group_0);
		len =
		tgt->transient_state.out_count = cmd_count - sizeof(scsi_command_group_0);
		virt = (vm_offset_t)tgt->cmd_ptr+sizeof(scsi_command_group_0);
		break;
	    default:
		virt = (vm_offset_t) tgt->dma_ptr;
		len = 0;
	}

	/* NOTE: tgt->cmd_ptr better be aligned!! */
	{
		register natural_t	*c = (natural_t*) tgt->cmd_ptr;

		ring->cmd[0] = c[0];
		ring->cmd[1] = c[1];
	}
	ring->ncmd = tgt->transient_state.cmd_count;
	ring->ndata = (len + 31) & ~31; /* roundup */
	ring->options = cmd_only ? SZ_O_NORETRY|SZ_O_NOSENSE : SZ_O_NOSENSE;
	ring->timeout = 0;	/* some say nowork */


	/* See if we can avoid copying.  This is iff the starting
	   address is aligned to 32 bytes, and the data fits all
	   in a single page.  Else we have to copy */

	phys = kvtophys((vm_offset_t)virt);

	/* Alignment restrictions, sigh */
#define	pageof(p)	((p) & ~(PAGE_SIZE-1))
	if ((phys & 0x1f) ||
	    ( pageof(phys) != pageof(phys+len-1) )) {
		/*
		 * Tough luck, must copy
		 */
		ring->dataaddr = kvtophys((vm_offset_t)tgt->dma_ptr);
		MARK_XCOPIED(ring);
adu_sz_ncop++;
	} else {
		ring->dataaddr = phys;
adu_sz_nncop++;
	}


	if ((ISA_XCOPIED(ring)) &&
	    (tgt->transient_state.out_count)) {
		register int slen = (len > 64) ? 64 : len;

		bcopy(virt, tgt->dma_ptr, slen);
		len -= slen;
		s = splhigh(); /* no kidding */
		ring->flag = SZ_F_SEND_COMMAND;
		wbflush();
		*asz->sz_db = 1;
		if (len > 0)
			bcopy(virt+slen, tgt->dma_ptr+slen, len);
		wbflush();
		splx(s);
	} else {
		ring->flag = SZ_F_SEND_COMMAND;
		wbflush();
		*asz->sz_db = 1;
	}
}

/*
 * Interrupt routine, runs only on one node.
 */
asz_intr(
	asz_softc_t	asz)
{
	register sz_msg_ring_t	*ring;
	register int		i;

	i = asz->nxt_in;
	ring = &asz->rb->msgs[i];
	while (ring->flag == SZ_F_COMMAND_COMPLETE) {

		target_info_t	*tgt;
		int		cs, nd, ns, xcop;
		integer_t	st;

		xcop = ISA_XCOPIED(ring);
		tgt = (target_info_t *)UNMARK_XCOPIED(ring);
		cs = ring->cstatus;
		nd = ring->ndata;
		ns = ring->nsstatus;
		st = ring->sstatus[0];

		ring->cmdtag = 0;
		ring->flag = SZ_F_EMPTY;
#if 0
		/* spec sez do it, but seems unnecessary */
		wbflush();
		*asz->sz_db = 1;
#endif
		if (++i == 32) {
			i = 0;
			ring = &asz->rb->msgs[0];
		} else {
			ring++;
		}
		asz->nxt_in = i;

		asz_done(tgt, cs, nd, ns, st, xcop);
	}
}

asz_done(
	target_info_t	*tgt,
	int		cs,
	int		nd,
	int		ns,
	integer_t	st,
	int		xcop)
{
	switch (cs) {
	case SZ_S_OK:
	case SZ_S_OVERRUN:
	case SZ_S_UNDERRUN:
		st &= 0xff;
		if (st != SCSI_ST_GOOD) {
			scsi_error(tgt, SCSI_ERR_STATUS, st, 0);
			tgt->done = (st == SCSI_ST_BUSY) ?
				SCSI_RET_RETRY : SCSI_RET_NEED_SENSE;
		} else {
			if (xcop && tgt->transient_state.in_count)
				asz_copyin(tgt, nd);
			tgt->done = SCSI_RET_SUCCESS;
		}
		if (tgt->ior)
			(*tgt->dev_ops->restart)(tgt, TRUE);

		break;
	case SZ_S_SELECT:
	case SZ_S_TIMEOUT:
		tgt->flags = 0;
		tgt->done = SCSI_RET_DEVICE_DOWN;
		if (tgt->ior)
			(*tgt->dev_ops->restart)(tgt, TRUE);
		break;
	case SZ_S_REJECT:
	case SZ_S_BUS:
	case SZ_S_FIRMWARE:
		printf("asz: something screwy (%x)\n", cs);
		gimmeabreak();
	}
}

asz_copyin(
	target_info_t	*tgt,
	int		len)
{
	char	*to;

	if ((tgt->cur_cmd != SCSI_CMD_READ) &&
	    (tgt->cur_cmd != SCSI_CMD_LONG_READ)) {

		/* hba-indep code expects it there */
		to = tgt->cmd_ptr;

	} else {
		register io_req_t	ior = tgt->ior;

		assert(ior != 0);
		assert(ior->io_op & IO_READ);

		to = ior->io_data;
	}
	wbflush();	/* mb, actually */
#if	(NCPUS > 1)
	tbia(/*to*/);	/* how ugly... and tbis does not fly, either */
#endif
	bcopy(tgt->dma_ptr, to, len);
}


#endif	/* NASZ > 0 */
