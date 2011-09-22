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
 * $Log:	adu_se.c,v $
 * Revision 2.4  93/05/15  19:10:18  mrt
 * 	machparam.h -> machspl.h
 * 
 * Revision 2.3  93/03/26  17:55:01  mrt
 * 	Fixed some decls.
 * 	[93/03/23            af]
 * 
 * Revision 2.2  93/01/14  17:10:17  danner
 * 	Created, from the DEC specs:
 * 	"Alpha Demonstration Unit Specification"
 * 	V1.0, Aug 1990.
 * 	[92/06/02            af]
 * 
 */
/*
 *	File: adu_se.c
 * 	Author: Alessandro Forin, Carnegie Mellon University
 *	Date:	7/92
 *
 *	ADU's I/O module ethernet driver.
 */

#include <ase.h>
#if	NASE > 0

#include <mach_kdb.h>

#include <machine/machspl.h>		/* spl definitions */
#include <sys/ioctl.h>
#include <vm/vm_kern.h>
#include <sys/types.h>
#include <kern/time_out.h>
#include <sys/syslog.h>
#include <ipc/ipc_port.h>
#include <ipc/ipc_kmsg.h>

#include <device/device_types.h>
#include <device/errno.h>
#include <device/io_req.h>
#include <device/if_hdr.h>
#include <device/if_ether.h>
#include <device/net_status.h>
#include <device/net_io.h>

#include <chips/busses.h>

#include <alpha/alpha_cpu.h>
#include <alpha/DEC/tvbus.h>
extern vm_offset_t	kvtophys();

#define	private	static

#define	RSIZ		1536	/* 1514 rounded up to 32 */
#define	DUMMY_IOR	((io_req_t)-1)
#define	BUSY_IOR	((io_req_t)-2)

struct se_softc {
	struct ifnet		is_if;
	unsigned char			is_addr[6];

	decl_simple_lock_data(, xmt_lock)
	io_req_t		tpkt[SE_NRINGS];
	int	xmt_count;		/* Xmt queue size		*/
	int	xmt_next;		/* Xmt queue head (insert)	*/
	int	xmt_complete;		/* Xmt queue tail (remove)	*/

	int	rcv_next;		/* Rcv buffer next in (tail)	*/

#define	index_plus_one(ix)	(((ix) + 1) & (SE_NRINGS - 1))

	se_ringbuffer_t		*rb;
	volatile natural_t	*se_br;
	volatile natural_t	*se_icr;
	volatile natural_t	*se_db;
	integer_t		cpu_no_and_channel;
	se_init_packet_t	init_block;

} adu_se_softc_data[NASE];

typedef struct se_softc *se_softc_t;

se_softc_t adu_se_softc[NASE];

int adu_se_nin, adu_se_nout, adu_se_ncop, adu_se_nncop;

/*
 * Utilities
 */

/* To cope with alignemt on xmit */
vm_offset_t	freebufs;
decl_simple_lock_data(, freebuf_lock)

void put_freebuf(
	vm_offset_t b)
{
	simple_lock(&freebuf_lock);
	*(vm_offset_t *) b = (freebufs) ? freebufs : 0;
	freebufs = b;
	simple_unlock(&freebuf_lock);
}

vm_offset_t get_freebuf()
{
	register vm_offset_t b;

	simple_lock(&freebuf_lock);
	b = freebufs;
	if (b)
		freebufs = *(vm_offset_t *) b;
	simple_unlock(&freebuf_lock);

	if (b == 0) {
		vm_offset_t	p;
		register int    ps = PAGE_SIZE;

		if (kmem_alloc_aligned(kernel_map, &p, (vm_size_t) ps))
			panic("adu_get_freebuf");
		b = p;
		ps -= RSIZ;
		while (ps >= RSIZ) {
			put_freebuf(p);
			p += RSIZ;
			ps -= RSIZ;
		}
	}
	return b;
}

#define	MARK_XCOPIED(r)		(r) = (io_req_t)((vm_offset_t)(r) | 1)
#define	ISA_XCOPIED(r)		((vm_offset_t)(r) & 1)
#define	UNMARK_XCOPIED(r)	(r) = (io_req_t)((vm_offset_t)(r) & ~1)

private
int get_xmt_ring(
	se_softc_t	sc)
{
	spl_t	s;
	int	i;

	s = splimp();
	simple_lock(&sc->xmt_lock);
		if (sc->xmt_count++ >= SE_NRINGS) panic("adu_se");
		i = sc->xmt_next;
		sc->xmt_next = index_plus_one(i);
	simple_unlock(&sc->xmt_lock);
	splx(s);

	return i;
}

private
adu_se_sendit(
	se_softc_t	sc,
	io_req_t	ior,
	int		index)
{
	struct xmt_ring_t	*ring = &sc->rb->xmt_ring[index];
	vm_offset_t		phys;
	register int		count;

	phys = kvtophys((vm_offset_t)ior->io_data);
	count = ior->io_count;

	/* Alignment restrictions, sigh */
#define	pageof(p)	((p) & ~(PAGE_SIZE-1))
	if ((phys & 0x1f) ||
	    ( pageof(phys) != pageof(phys+count-1) )) {
		/*
		 * Tough luck, get a buf and copy
		 */
adu_se_ncop++;
		phys = get_freebuf();
		bcopy(ior->io_data, phys, count);
		phys = kvtophys(phys);
		MARK_XCOPIED(ior);
	}
else adu_se_nncop++;


	ring->bufp = phys;
	ring->ntbuf = (count < 64) ? 64 : ((count + 31) & ~31);

	/*
	 * Keep request around until transmission complete
	 */
	sc->tpkt[index] = ior;

	/*
	 * Now tell io board
	 */
	ring->flag = SE_F_XMT_PKT;
	wbflush();
	*sc->se_db = 1;
adu_se_nout++;
}

private
adu_se_rcvit(
	register struct ifnet	*ifp,
	char			*data,
	register struct rcv_ring_t	*r)
{
	unsigned int		len = r->nrbuf;
	register ipc_kmsg_t	new_kmsg;
	struct packet_header	*pkt;
	struct ether_header	*hdr;

adu_se_nin++;

	if (len <= sizeof(struct ether_header))
		goto dropit;	/* sanity */

	/*
	 * Get a new kmsg to put data into.
	 */
	new_kmsg = net_kmsg_get();
	if (new_kmsg == IKM_NULL) {
	    /*
	     * No room, drop the packet
	     */
dropit:
	    ifp->if_rcvdrops++;

	    /* give buffer back */
	    r->nrbuf = RSIZ;
	    wbflush();
	    r->flag = SE_F_EMPTY;

	    return;
	}

	hdr = (struct ether_header *) net_kmsg(new_kmsg)->header;
	pkt = (struct packet_header *) net_kmsg(new_kmsg)->packet;

	*hdr = *(struct ether_header *) data;
	len -= sizeof(*hdr);
	bcopy(data + sizeof(*hdr), pkt + 1, len);

	/* give buffer back */
	r->nrbuf = RSIZ;
	wbflush();
	r->flag = SE_F_EMPTY;

	pkt->type = hdr->ether_type;
	pkt->length = len;
		
	/*
	 * Hand the packet to the network module.
	 */
	net_packet(ifp, new_kmsg, len + sizeof(*pkt), ethernet_priority(new_kmsg));
}


/*
 * Definition of the driver for the auto-configuration program.
 */

private int	adu_se_probe();
private void	adu_se_attach();

vm_offset_t	adu_se_std[NASE];
struct	bus_device *adu_se_info[NASE];
struct	bus_driver adu_se_driver = 
        { adu_se_probe, 0, adu_se_attach, 0, adu_se_std, "ase", adu_se_info,};


/*
 * Externally visible functions
 */
int	adu_se_tintr(), adu_se_rintr();

int	adu_se_open(), adu_se_output(), adu_se_get_status(),	/* user */
	adu_se_set_status(), adu_se_setinput(), adu_se_restart();

/*
 * Adapt/Probe/Attach functions
 */
adu_se_etheraddr(
	int	unit,
	char	*eth)
{
	if (unit <= NASE)
		bcopy(eth, adu_se_softc_data[unit].is_addr, 6);
}

adu_se_cold_init(
	int		unit,
	int		cpu_no,
	vm_offset_t	module_base)
{
	se_ringbuffer_t	*rb;
	se_softc_t	se;
	register int	n;
	extern vm_offset_t	pmap_steal_memory();
	int		adu_se_tintr(), adu_se_rintr();

	n  = unit;
	se = &adu_se_softc_data[n];
	adu_se_softc[n] = se;
	adu_se_std[n] = (vm_offset_t) module_base;
	se->se_br  = (volatile natural_t *)( module_base + TV_IO_NI_BASE );
	se->se_icr = (volatile natural_t *)( module_base + TV_IO_NI_ICR );
	se->se_db  = (volatile natural_t *)( module_base + TV_IO_NI_S_BELL );

	rb = (se_ringbuffer_t *) pmap_steal_memory(ALPHA_PGBYTES);
	se->rb = rb;

	bzero(rb, sizeof(se_ringbuffer_t));

	/* Allocate memory to the receive rings */
	{
		vm_offset_t	phys = 0;
		register struct rcv_ring_t	*r, *e;

		phys = pmap_steal_memory(RSIZ * SE_NRINGS);
		phys = kvtophys(phys);
		r = rb->rcv_ring;
		for (e = r + SE_NRINGS; r < e; r++) {
			r->bufp = phys;
			r->nrbuf = RSIZ;
			r->flag = SE_F_RCV_PKT;	/* Keep it !! */
			phys += RSIZ;
		}
	}

	/* Interrupts */
	{
		int tchan, rchan;
		tchan = tv_get_channel(cpu_no, adu_se_tintr, se);
		rchan = tv_get_channel(cpu_no, adu_se_rintr, se);
		se->cpu_no_and_channel = ((cpu_no << 2) & SE_ICR_IRQ_NODE) |
				       ((rchan << 6) & SE_ICR_RIRQ_CHAN) |
				       ((tchan << 11) & SE_ICR_TIRQ_CHAN);
	}

	*se->se_icr = se->cpu_no_and_channel;	/* enable intrs later */

	*se->se_br = K0SEG_TO_PHYS(rb);
	wbflush();
	*se->se_db = 1;

}

private
adu_se_probe(
	vm_offset_t	  base,
	struct bus_device *ui)
{
	if (ui->unit >= NASE) return 0;
	if (base != (vm_offset_t)adu_se_std[ui->unit]) gimmeabreak();
	return 1;
}

/*
 * Interface exists: make available by filling in network interface
 * record.  System will initialize the interface when it is ready
 * to accept packets.
 */
private void
adu_se_attach(
	register struct bus_device *ui)
{
	struct ifnet   *ifp;
	int unit = ui->unit;
	int i;
	se_softc_t	sc = adu_se_softc[unit];
	unsigned char	*enaddr;

	enaddr = sc->is_addr;
	printf(": %x-%x-%x-%x-%x-%x",
		(natural_t)enaddr[0], (natural_t)enaddr[1],
		(natural_t)enaddr[2], (natural_t)enaddr[3],
		(natural_t)enaddr[4], (natural_t)enaddr[5]);
	/*
	 * Initialize the standard interface descriptor 
	 */
	ifp = &sc->is_if;
	ifp->if_unit = unit;
	ifp->if_header_size = sizeof(struct ether_header);
	ifp->if_header_format = HDR_ETHERNET;
	ifp->if_address_size = 6;
	ifp->if_mtu = ETHERMTU;
	ifp->if_flags |= IFF_BROADCAST;

	ifp->if_address = (char *) enaddr;

	if_init_queues(ifp);

}

/*
 * Use a specific hardware address for interface
 */
adu_se_setaddr(
	register se_softc_t 	sc,
	unsigned char			eaddr[6])
{
	se_init_packet_t	*ini = &sc->init_block;

	/*
	 * Modify initialization block accordingly
	 */
	bcopy(eaddr, ini->rxaddr, sizeof(eaddr));

	/*
	 * Make a note of it
	 */
	if (eaddr != sc->is_addr)
		bcopy(eaddr, sc->is_addr, sizeof(eaddr));

	/*
	 * Tell the interface
	 */
	adu_se_ship_init_block(sc);
}

adu_se_ship_init_block(
	register se_softc_t sc)
{
	se_init_packet_t	*ini = &sc->init_block;
	struct xmt_ring_t	*r;
	int			index;

	/* address must be set already */
	ini->flag = SE_F_EMPTY;
	ini->rxmode = SE_R_NORMAL;
	bzero(ini->lamask, sizeof(*ini->lamask));
	index = get_xmt_ring(sc);
	r = &sc->rb->xmt_ring[index];
	bcopy(ini, r, sizeof(*ini));
	sc->tpkt[index] = DUMMY_IOR;
	r->flag = SE_F_INIT_PKT;
	wbflush();
	*sc->se_db = 1;
}

adu_se_init(
	se_softc_t	sc)
{
	register struct rcv_ring_t	*r, *e;

	adu_se_setaddr(sc, sc->is_addr);

	r = sc->rb->rcv_ring;
	for (e = r + SE_NRINGS; r < e; r++) {
		r->flag = SE_F_EMPTY;
	}
	wbflush();

	sc->is_if.if_flags |= IFF_UP | IFF_RUNNING;

	*sc->se_icr = sc->cpu_no_and_channel | SE_ICR_RE | SE_ICR_TE;
	wbflush();

	*sc->se_db = 1;
}

/*
 * Open the device, declaring the interface up
 * and enabling lance interrupts.
 */
/*ARGSUSED*/
int adu_se_rcv_chunk	=  8;	/* arbitrary */

adu_se_open(
	int	dev,
	int	flag)
{
	register int		unit = dev, i;
	register se_softc_t	sc = adu_se_softc[unit];

	if (unit >= NASE)
		return EINVAL;
	if (sc == 0)
		return ENXIO;

	sc->is_if.if_flags |= IFF_UP;

	adu_se_init(sc);
	return (0);
}

/*
 * Start output on interface.
 *
 */
adu_se_start(
	int	unit)
{
	register se_softc_t sc = adu_se_softc[unit];
	io_req_t        request;
	spl_t		s;
	register int    index;
	boolean_t	ring_it = FALSE;

	while (TRUE) {

		s = splimp();
		simple_lock(&sc->xmt_lock);

		if (sc->xmt_count >= SE_NRINGS)
			break;	/* full */

		/*
		 * Dequeue the next transmit request, if any. 
		 */
		IF_DEQUEUE(&sc->is_if.if_snd, request);
		if (request == 0)
			break;	/* empty */

		index = sc->xmt_next;
		sc->xmt_next = index_plus_one(index);
		sc->xmt_count++;

		sc->tpkt[index] = BUSY_IOR;	/* busy, sanity */

		/*
		 * Let other processors queue up packets
		 */
		simple_unlock(&sc->xmt_lock);
		splx(s);

		/*
		 * .. while we ship this one
		 */
		adu_se_sendit( sc, request, index);
		ring_it = TRUE;
	}

	if (ring_it)
		*sc->se_db = 1;

	simple_unlock(&sc->xmt_lock);
	splx(s);
}

/*
 * Output routine.
 * Call common function for wiring memory,
 * come back later (to adu_se_start) to get
 * things going.
 */
io_return_t
adu_se_output(
	int		dev,
	io_req_t	ior)
{
	return (net_write(&adu_se_softc[dev]->is_if, adu_se_start, ior));
}

/*
 * Set/Get status functions
 */
adu_se_get_status(
	int		 dev,
	dev_flavor_t	 flavor,
	dev_status_t	 status,	/* pointer to OUT array */
	natural_t	*status_count)		/* out */
{
	return (net_getstat(&adu_se_softc[dev]->is_if,
			    flavor, status, status_count));
}

adu_se_set_status(
	int		dev,
	dev_flavor_t	flavor,
	dev_status_t	status,
	natural_t	status_count)
{
	register se_softc_t	sc;
	int			unit = dev;

	sc = adu_se_softc[unit];

	switch (flavor) {
	case NET_STATUS:
		{
			/* XXX */
			break;
		}

	case NET_ADDRESS:
		{
			register union ether_cvt {
				char            addr[6];
				int             lwd[2];
			}              *ec = (union ether_cvt *) status;

			if (status_count < sizeof(*ec) / sizeof(int))
				return (D_INVALID_SIZE);

			ec->lwd[0] = ntohl(ec->lwd[0]);
			ec->lwd[1] = ntohl(ec->lwd[1]);

			adu_se_setaddr(sc, ec->addr);

			break;
		}
	default:
		return (D_INVALID_OPERATION);
	}
	return (D_SUCCESS);
}


/*
 * Install new filter.
 * Nothing special needs to be done here.
 */
io_return_t
adu_se_setinput(
	int		dev,
	ipc_port_t	receive_port,
	int		priority,
	filter_t	*filter,
	natural_t	filter_count)
{
	return (net_set_filter(&adu_se_softc[dev]->is_if,
			       receive_port, priority,
			       filter, filter_count));
}

/*
 * Interrupt routines
 */
adu_se_rintr(
	se_softc_t	sc)
{
	int             index, next;
	se_ringbuffer_t *rb = sc->rb;

	/*
	 * Scan all rings, starting from expected one
	 * (XXX Opt: stop at first empty. beware of re-inits)
	 */
	next = index = sc->rcv_next;
	do {
		register struct rcv_ring_t	*r;

		r = &rb->rcv_ring[index];
		index = index_plus_one(index);
		if (r->flag != SE_F_EMPTY) {
if (r->status != SE_S_OK) gimmeabreak(r->status);
			next = index;
			adu_se_rcvit(&sc->is_if,
				     PHYS_TO_K0SEG(r->bufp),
				     r);
/*			*sc->sc_db = 1;*/
			/* which also gives the buffer back asap */
		}
#if notyet
		else break;
#endif
	} while (index != sc->rcv_next);

	sc->rcv_next = next;
}

int adu_se_in_tint;

adu_se_tintr(
	se_softc_t	sc)
{
	int             index, next;
	se_ringbuffer_t *rb = sc->rb;
	register boolean_t	done = FALSE;

if (adu_se_in_tint) gimmeabreak();
adu_se_in_tint++;

	/* NB: Only one cpu can take this interrupt */

	/* Scan from the last one incomplete */
	next = index = sc->xmt_complete;
	do {
		register struct xmt_ring_t	*r;
		io_req_t			*iorp;

		r = &rb->xmt_ring[index];
		iorp = &sc->tpkt[index];
		index = index_plus_one(index);
		if (r->flag == SE_F_EMPTY) {
			register io_req_t	ior = *iorp;

			if (ior && (ior != BUSY_IOR)) {
				*iorp = 0;
if (r->status != SE_S_OK) gimmeabreak(r->status);
				next = index;

				simple_lock(&sc->xmt_lock);
				if (--sc->xmt_count == 0) done = TRUE;
				simple_unlock(&sc->xmt_lock);

				if (ior != DUMMY_IOR) {
					if (ISA_XCOPIED(ior)) {
						UNMARK_XCOPIED(ior);
						put_freebuf(PHYS_TO_K0SEG(r->bufp));
					}
					iodone(ior);
				}
			}
		}
		else
			done = TRUE;

	} while (!done && (index != sc->xmt_complete));

	sc->xmt_complete = next;
adu_se_in_tint--;
}

adu_se_restart()
{
	panic("adu_se_restart");
}

#endif	/* NASE > 0 */
