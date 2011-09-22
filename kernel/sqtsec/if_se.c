/* 
 * Mach Operating System
 * Copyright (c) 1993, 1991 Carnegie Mellon University
 * Copyright (c) 1991 Sequent Computer Systems
 * All Rights Reserved.
 * 
 * Permission to use, copy, modify and distribute this software and its
 * documentation is hereby granted, provided that both the copyright
 * notice and this permission notice appear in all copies of the
 * software, derivative works or modified versions, and any portions
 * thereof, and that both notices appear in supporting documentation.
 * 
 * CARNEGIE MELLON AND SEQUENT COMPUTER SYSTEMS ALLOW FREE USE OF
 * THIS SOFTWARE IN ITS "AS IS" CONDITION.  CARNEGIE MELLON AND
 * SEQUENT COMPUTER SYSTEMS DISCLAIM ANY LIABILITY OF ANY KIND FOR
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
 * $Log:	if_se.c,v $
 * Revision 2.4  93/01/14  17:56:21  danner
 * 	Ansified preprocessor comments, fixed a static/extern problem
 * 	 with se_output.
 * 	[93/01/14            danner]
 * 
 * Revision 2.3  91/07/31  18:06:35  dbg
 * 	Changed copyright.
 * 	[91/07/31            dbg]
 * 
 * Revision 2.2  91/05/08  13:05:43  dbg
 * 	Changed net_filter to net_packet.
 * 
 * 	MACH_KERNEL conversion.
 * 	Added volatile declarations.
 * 	[91/03/22            dbg]
 * 
 */

#undef RAW_ETHER
/* #define RAW_ETHER	*/			/* Not in MACH */

#undef	PROMISCUOUS	/* UNDO promiscuous kernel */
/* #define	PROMISCUOUS */	/* promiscuous kernel */	/* Not in MACH*/

#ifndef lint
static char rcsid[] = "$Header: if_se.c,v 2.4 93/01/14 17:56:21 danner Exp $";
#endif

/*
 * SCSI/Ether Ethernet Communications Controller interface
 *
 * ETHER OUTPUT NOTES:
 * -------------------
 * Only one output request is active at a time.  A simple array
 * of iats holds the addresses of the mbuf data that get written.
 * We copy transmits into a single buffer because the higher-level
 * network code can generate mbufs too small for the DMA's to handle
 * (the firmware doesn't have enough time to turn around and reload).
 *
 * As a matter of convention, all SEC Ether ioctls are done with the
 * output device and output locks.
 * 
 * ETHER INPUT NOTES:
 * ------------------
 * Device programs for Ether read contain a pointer to an iat
 * and the number of data blocks in that iat.  The iat dp_data
 * fields give the physical addresses of the m_data field of 
 * a parallel array of mbufs.  Ether packets read from the net
 * are placed into these data blocks (and hence right into the
 * mbufs).
 *
 * Each controller has a circular queue of pointers to mbufs and
 * a circular queue of iats that are continually filled by the
 * SEC firmware with input packets.
 * Our job is to replace the used mbufs as quickly as possible
 * at interrupt time, and refill the iats.  We then add another
 * device program (or two if we wrap around the end of the iat
 * ring) for ether input.
 * 
 * Important hints:
 *	- There is an iat queue and an mbuf pointer queue for each
 *	  controller.
 *	- The iat queue and the mbuf queue have the same number
 *	  of elements.
 *	- Except when refilling the read programs (at interrupt time),
 *	  the heads of the iat queue and the mbuf queue should be the same.
 *	  Even here, they should only be different during the actual
 *	  refilling of the iat and mbuf queues.
 *	- All hell breaks loose if we run out of input programs
 *	  to replace the iats.  We can't sleep and wait for more at
 *	  interrupt level.
 *	- Overspecifying the size of the Ether read request and
 *	  done queues in the binary config file is a very good idea.
 *	- Refilling the queues after reading short packets will cause
 *	  each packet to have a single new device program added to the
 *	  Ether read device request queue.
 *	- No attempt is made to optimize these programs, as there is
 *	  no synchronization with the SEC firmware:  I can't ask him
 *	  to stop for a second while I increase the number of iats in
 *	  that last device program.
 *
 *
 *
 * TMP AND LOCKING NOTES:
 * ----------------------
 *
 * There is very little locking or synchronization needed at this
 * level of the software.  Most of it really goes on above when necessary.
 *
 * In general, we try to lock only the portions of the controller state
 * that we have to.  When changing "important" information (like fields
 * int the arp and/or ifnet structures), we lock everything.
 *
 * To lock everything, it is safe to lock structure from the inside out.
 * That is, lock either the input or output segment of the controller
 * state, then lock the common structure.  With the macros defined
 * below, the order OS_LOCK, then SS_LOCK should be safe.
 * See how se_init does locking for an example.
 */

/*
 * Revision 1.2  89/08/16  15:22:05  root
 * balance -> sqt
 * 
 * Revision 1.1  89/07/05  13:18:31  kak
 * Initial revision
 * 
 */

#ifdef	MACH_KERNEL
#include <device/device_types.h>
#include <device/io_req.h>
#include <device/net_io.h>

#include <sqt/vm_defs.h>
#include <sqt/intctl.h>
#include <sqt/ioconf.h>
#include <sqt/cfg.h>
#include <sqt/slic.h>
#include <sqt/mutex.h>

#include <sqtsec/sec.h>
#include <sqtsec/if_se.h>

/*
 * Convert to Mach style assert
 */
#define ASSERT(C,S)	assert(C)

#include <kern/assert.h>

#else	MACH_KERNEL
#include "sys/param.h"
#include "sys/systm.h"
#include "sys/mbuf.h"
#include "sys/buf.h"
#include "sys/protosw.h"
#include "sys/socket.h"
#include "sys/socketvar.h"
#include "sys/ioctl.h"
#include "sys/errno.h"
#include "sys/vm.h"
#include "sys/conf.h"

#include "net/if.h"
#include "net/netisr.h"
#include "net/route.h"

#include "netinet/in.h"
#include "netinet/in_systm.h"
#include "netinet/in_var.h"			/* MACH/4.3 */
#include "netinet/ip.h"
#include "netinet/ip_var.h"
#include "netinet/if_ether.h"

#include "sqt/pte.h"
#include "sqt/intctl.h"
#include "sqt/ioconf.h"
#include "sqt/cfg.h"
#include "sqt/slic.h"

#include "sqt/mutex.h"

#include "sqtsec/sec.h"

#include "sqtif/if_se.h"

#ifdef PROMISCUOUS
#include "net/promisc.h"
#endif PROMISCUOUS

#define	KVIRTTOPHYS(addr) \
	(PTETOPHYS(Sysmap[btop(addr)]) + ((int)(addr) & (NBPG-1)))

#ifdef	MACH
/*
 * Driver not yet converted to MACH/4.3 names.
 */

#define	ETHERPUP_PUPTYPE	ETHERTYPE_PUP
#define	ETHERPUP_IPTYPE		ETHERTYPE_IP
#define ETHERPUP_ARPTYPE	ETHERTYPE_ARP
#define	ETHERPUP_TRAIL		ETHERTYPE_TRAIL
#define	ETHERPUP_NTRAILER	ETHERTYPE_NTRAILER

/*
 * Convert to Mach style assert
 */
#define ASSERT(C,S)	assert(C)

#include "kern/assert.h"

#endif	/* MACH */
#endif	/* MACH_KERNEL */

/*
 * All procedures are referenced either through the se_driver structure,
 * or via the procedure handles in the ifnet structure.
 * Hence, everything but the se_driver structure should be able to be static.
 */

static int se_probe(), se_boot(), se_intr(), se_watch();
static int se_init(), se_ioctl(), se_reset();
#ifndef MACH_KERNEL
static int se_output();
#endif

#ifndef PROMISCUOUS

static struct mbuf *se_reorder_trailer_packet();

#else

/* N.B. this routine is not static so promiscq handler can call it */

struct mbuf *se_reorder_trailer_packet();

#endif /* PROMISCUOUS */

int se_handle_read(), se_add_read_progs();
int se_start(), se_set_addr();
int se_set_modes();

#ifdef	MACH_KERNEL
int	se_start_u();	/* takes unit number */
#endif	/* MACH_KERNEL */

struct sec_driver se_driver = {
	/* name	chan	flags		probe		boot		intr */
	"se",	1,	SED_TYPICAL,	se_probe,	se_boot,	se_intr
};

/*
 * SCSI-command template for Ether write.
 * These are placed in the write device programs,
 * and altered by the se_start routine just before
 * we write the packet.
 */

u_char se_scsi_cmd[10] = { SCSI_ETHER_WRITE, SCSI_ETHER_STATION };

#ifdef RAW_ETHER

#include "../net/raw_cb.h"

#endif /* RAW_ETHER */

/*
 * sec_init_iatq()
 *	initialize a ring of iat entries.
 *
 * If locking is needed, it is presumed to be done elsewhere.
 */

static void
sec_init_iatq(iq, count)
	register struct sec_iatq *iq;
	unsigned count;
{
	iq->iq_iats = (struct sec_iat *)
			calloc((int)(count*sizeof(struct sec_iat)));
	iq->iq_size = count;
	iq->iq_head = 0;
}

#ifndef	MACH_KERNEL
/*
 * sec_spray_mbuf_iatq - spray an mbuf chain into a queue of iats.
 *
 * The head of the iat queue is adjusted here.
 * It is illegal to wrap around the IAT ring.
 *
 * Returns pointer to first IAT if it worked.
 * Returns 0 otherwise.
 */

static struct sec_iat *
sec_spray_mbuf_iatq(m, iq)
	register struct mbuf *m;
	register struct sec_iatq *iq;
{
	register struct sec_iat *iat;
	int i, n;

	n = mbuf_chain_length(m);
	ASSERT(n > 0, "sec_spray_iatq: n <= 0");
	ASSERT(n <= iq->iq_size, "sec_spray_iatq: n > size");
	if (n > (iq->iq_size - iq->iq_head))
		return((struct sec_iat *)0);

	for (i = 0;  i < n;  ++i, m = m->m_next) {
		int k = (iq->iq_head + i) % iq->iq_size;

		ASSERT(m != (struct mbuf *)0, "sec_spray_iatq: m == 0");
		iat = &iq->iq_iats[k];
		iat->iat_data = (u_char *)KVIRTTOPHYS(mtod(m, int));
		iat->iat_count = m->m_len;
	}

	ASSERT(m == (struct mbuf *)0, "sec_spray_iatq: m != 0");
	iat = &iq->iq_iats[iq->iq_head];
	iq->iq_head = (iq->iq_head + n) % iq->iq_size;
	return(iat);
}
#endif	/* MACH_KERNEL */

/*
 * sec_start_prog - start a program on a SCSI/Ether device.
 */

static int
sec_start_prog(cmd, cib, slic, bin, vector, splok)
	struct sec_cib *cib;
	u_char slic, bin, vector;
{
	register volatile int *stat
		= PHYSTOKV(cib->cib_status, volatile int *);
	spl_t sipl;

	if (splok)
		sipl = splhi();

	cib->cib_inst = cmd;
	*stat = 0;
	mIntr(slic, bin, vector);
	if (splok)
		splx(sipl);

	while ((*stat & SINST_INSDONE) == 0)
		continue;
	return(*stat & ~SINST_INSDONE);
}

#ifdef	MACH_KERNEL
/*
 * Fill in the fields of a net-kmsg pointer queue.
 */
void
sec_init_msgq(mq, size)
	register struct sec_msgq *mq;
	unsigned int size;
{
	mq->mq_msgs =
	    (ipc_kmsg_t *) calloc((int) size * sizeof(ipc_kmsg_t));
	mq->mq_size = size;
	mq->mq_head = 0;
}

#else	MACH_KERNEL
/*
 * Fill in the fields of an mbuf pointer queue.
 *
 * We can't fill in the mbuf pointers yet, as it is too soon
 * to allocate mbufs yet.
 */

static
sec_init_mbufq(mq, size)
	register struct sec_mbufq *mq;
	unsigned size;
{
	mq->mq_mbufs =
		(struct mbuf **) calloc((int)(size*sizeof(struct mbuf *)));
	mq->mq_size = size;
	mq->mq_head = 0;
}

/*
 * sec_spray_mbuf_mbufq - spray an mbuf chain into a queue of mbuf pointers.
 *
 * The head of the mbuf queue is adjusted here.
 * It is illegal to wrap around the mbuf pointer ring.
 *
 * This work is done in parallel with the iat queue.
 *
 * Returns pointer to first mbuf pointer if it worked.
 * Returns 0 otherwise.
 */

static struct mbuf **
sec_spray_mbuf_mbufq(m, mq)
	register struct mbuf *m;
	register struct sec_mbufq *mq;
{
	register struct mbuf **mbufp;
	register int i, n;

	n = mbuf_chain_length(m);
	ASSERT(n > 0, "sec_spray_mbufq: n <= 0");
	ASSERT(n <= mq->mq_size, "sec_spray_mbufq: n > size");
	if (n > (mq->mq_size - mq->mq_head))
		return((struct mbuf **)0);

	for (i = 0;  i < n;  ++i, m = m->m_next) {
		ASSERT(m != (struct mbuf *)0, "sec_spray_mbufq: m == 0");
		mbufp = &mq->mq_mbufs[(mq->mq_head + i) % mq->mq_size];
		*mbufp = m;
	}
	ASSERT(m == (struct mbuf *)0, "sec_spray_mbufq: m != 0");
	mbufp = &mq->mq_mbufs[mq->mq_head];
	mq->mq_head = (mq->mq_head + n) % mq->mq_size;
	return(mbufp);
}

/*
 * sec_chain_mbufs - chain 'n' mbufs from the mbuf q and return the head.
 *
 * We don't touch the head pointer of the queue.
 * We set the length correctly for each mbuf.
 */

static struct mbuf *
sec_chain_mbufs(mq, n, length)
	register struct sec_mbufq *mq;
{
	register struct mbuf *m;
	register int k;

	ASSERT(n > 0, "sec_chain: n <= 0");
	ASSERT(n <= mq->mq_size, "sec_chain: n > size");

	/*
	 * build the chain from the back to the front.
	 * 'k' always refers to the entry cyclically after
	 * the one we want to chain next.
	 */

	m = 0;
	k = n + mq->mq_head;
	if (k >= mq->mq_size) k -= mq->mq_size;
	ASSERT(k >= 0, "sec_chain: k < 0");
	ASSERT(k < mq->mq_size, "sec_chain: k >= size");

	do {
		--k;
		if (k < 0) k = mq->mq_size - 1;
		mq->mq_mbufs[k]->m_next = m;
		m = mq->mq_mbufs[k];
		if (m->m_next == (struct mbuf *)0) {

			/*
			 * "last" mbuf.  Adjust it's length.
			 */

			m->m_len = MLEN - (n*MLEN - length);

		} else {
			m->m_len = MLEN;
		}
	} while (k != mq->mq_head);

	ASSERT(mbuf_chain_length(m) == n, "sec_chain: length");
	ASSERT(mbuf_chain_size(m) == length, "sec_chain: size");

	return(m);
}

/*
 * mbuf_chain_size - Determine the number of data bytes in a chain of mbufs.
 */

static int
mbuf_chain_size(m)
	register struct mbuf *m;
{
	register int count;

	for (count = 0; m != (struct mbuf *)0; m = m->m_next)
		count += m->m_len;
	return(count);
}

/*
 * mbuf_chain_length - Determine the number of mbufs in a chain.
 */

static int
mbuf_chain_length(m)
	register struct mbuf *m;
{
	register int count;

	for (count = 0; m != (struct mbuf *)0; m = m->m_next)
		++count;
	return(count);
}

#ifdef	MACH
/*
 * m_getm() -- get multiple mbuf's.
 *
 * This interface is used in DYNIX to use only one "gate" round-trip
 * to allocate a set of mbuf's.  This implementation is simplistic
 * mono-processor version.
 */

/*ARGSUSED*/
struct mbuf *
m_getm(canwait, type, n)			/* get n mbufs */
	register int n;
{
	register struct mbuf *m;
	register struct mbuf *msave = NULL;

	while (n-- > 0) {
		MGET(m, canwait, type);
		if (m == NULL) {
			m_freem(msave);
			return((struct mbuf *) NULL);
		}
		m->m_next = msave;
		msave = m;
	}

	return(msave);
}
#endif	/* MACH */
#endif	/* MACH_KERNEL */

#ifdef	MACH_KERNEL

/* lock the controller state */
#define	SS_LOCK(softp)	(p_lock(&(softp)->ss_lock, SPLIMP))
#define	SS_UNLOCK(softp, sipl)	(v_lock(&(softp)->ss_lock, sipl))

/* lock the output state */
#define	OS_LOCK(softp)	(p_lock(&(softp)->os_lock, SPLIMP))
#define	OS_UNLOCK(softp, sipl)	(v_lock(&(softp)->os_lock, sipl))

#else	MACH_KERNEL
#ifndef	MACH

/* lock the controller state */
#define	SS_LOCK(softp)	(p_lock(&(softp)->ss_lock, SPLIMP))
#define	SS_UNLOCK(softp, sipl)	(v_lock(&(softp)->ss_lock, sipl))

/* lock the output state */
#define	OS_LOCK(softp)	(p_lock(&(softp)->os_lock, SPLIMP))
#define	OS_UNLOCK(softp, sipl)	(v_lock(&(softp)->os_lock, sipl))

#else
/*
 * For now, MACH is "mono-processor" for all network code.
 */

/* lock the controller state */
#define	SS_LOCK(softp)		splimp()
#define	SS_UNLOCK(softp, sipl)	splx(sipl)

/* lock the output state */
#define	OS_LOCK(softp)		splimp()
#define	OS_UNLOCK(softp, sipl)	splx(sipl)

#endif	/* MACH */
#endif	/* MACH_KERNEL */

int se_max_unit = -1;		/* largest index of active ether controller */
u_char se_base_vec;		/* base interrupt vector */
struct se_state *se_state;	/* pointer to array of soft states */

/*
 * Probe an SEC for existence of Ether controller.
 *
 * There's some debate about what this means: presently
 * if the controller is there, so is the Ether part.
 * This is expected to be changed in the future,
 * when the world of depopulated boards arrives.
 * So let's look at the diagnostics flags, and make
 * the decision based on that.
 */

static
se_probe(probe)
	struct sec_probe *probe;
{
	if (probe->secp_desc->sec_diag_flags & CFG_S_ETHER)
		return(0);
	return(1);
}

/*
 * se_boot_one()
 *	boot procedure for a single device.
 *
 * Allocate the non-mbuf data structures for the device.
 * We shouldn't really talk to the device now either.
 *
 * For both ether read and ether write, the request and done queues
 * were allocated by autoconfig code.  We record handles to these
 * queues and fill in the actual device programs.
 *
 * The done queues should not need anything done to them, as they
 * never need programs of their own.
 *
 * The status pointers for each cib are set to point to local data
 * in the state structures.
 *
 * Iat queues are also allocated, but can't be filled in  yet
 * (no mbufs to allocate yet).  For input, the parallel array
 * of mbuf pointers is allocated as well.
 *
 * No locking needs to be done here, as we are still running config
 * code single-processor.
 */

static void
se_boot_one(softp, sd)
	register struct se_state *softp;
	register struct sec_dev *sd;
{
	int i;

	/*
	 * Controller info:  Can do this with
	 * either the input device or output device.
	 * Either way, we just do it once.
	 */

	if (!softp->ss_initted) {
		register struct ifnet *ifp;
		register struct sockaddr_in *sin;

#ifdef	MACH_KERNEL
		ifp = &softp->ss_if;
		ifp->if_unit = softp-se_state;
		ifp->if_header_size = sizeof(struct ether_header);
		ifp->if_header_format = HDR_ETHERNET;
		ifp->if_address_size = 6;
#else	MACH_KERNEL
		ifp = &softp->ss_arp.ac_if;
		ifp->if_unit = softp-se_state;
		ifp->if_name = se_driver.sed_name;
#endif	/* MACH_KERNEL */
		ifp->if_mtu = se_mtu;
#ifndef	MACH
		init_lock(&ifp->if_snd.ifq_lock, G_IFNET);
#endif	/* MACH */
#ifdef	MACH_KERNEL 
#else	/* MACH_KERNEL */
#ifndef	MACH
		sin = (struct sockaddr_in *)&ifp->if_addr;
		sin->sin_family = AF_INET;
		sin->sin_addr = arpmyaddr((struct arpcom *)0);
#endif	/* MACH */
		ifp->if_init = se_init;
		ifp->if_output = se_output;
		ifp->if_ioctl = se_ioctl;
		ifp->if_reset = se_reset;
#endif	/* MACH_KERNEL */
		init_lock(&softp->ss_lock, se_gate);
		bzero((caddr_t)&softp->ss_sum, sizeof(softp->ss_sum));
		softp->ss_scan_int = se_watch_interval;
#ifdef	MACH_KERNEL
		ifp->if_address = (char *)softp->ss_addr;
		bcopy((char *)sd->sd_desc->sec_ether_addr,
			(char *)softp->ss_addr, 6);
#else	/* MACH_KERNEL */
		bcopy((caddr_t)sd->sd_desc->sec_ether_addr,
			(caddr_t)softp->ss_arp.ac_enaddr, 6);
#endif	/* MACH_KERNEL */
		softp->ss_slic = sd->sd_desc->sec_slicaddr;
		softp->ss_bin = se_bin;
		softp->ss_ether_flags = SETHER_S_AND_B;
		softp->ss_alive = 1;
		softp->ss_initted = 1;
		softp->ss_init_called = 0;
#ifdef	MACH_KERNEL
		if_init_queues(ifp);
#else	/* MACH_KERNEL */
		if_attach(ifp);
#endif	/* MACH_KERNEL */
#ifndef	MACH
		pciattach(ifp, softp->ss_arp.ac_enaddr);
#endif	/* MACH */
		if ((int)(&softp->os_gmode + 1) > 4*1024*1024) {
			printf("%s%d: data structures above 4Mb!\n",
				se_driver.sed_name, softp-se_state);
			printf("     Ethernet function is unpredictable.\n");
		}
	}
	if (sd->sd_chan == SDEV_ETHERREAD) {
		init_lock(&softp->is_lock, se_gate);
		softp->is_cib = sd->sd_cib;
		softp->is_cib->cib_status =
			KVTOPHYS(&softp->is_status, int *);
		softp->is_reqq.sq_progq = sd->sd_requestq;
		softp->is_reqq.sq_size = sd->sd_req_size;
		softp->is_doneq.sq_progq = sd->sd_doneq;
		softp->is_doneq.sq_size = sd->sd_doneq_size;
		SEC_fill_progq(softp->is_reqq.sq_progq,
			(int)softp->is_reqq.sq_size,
			(int)sizeof(struct sec_edev_prog));
		sec_init_iatq(&softp->is_iatq, softp->is_reqq.sq_size-3);
#ifdef	MACH_KERNEL
		sec_init_msgq(&softp->is_msgq, softp->is_reqq.sq_size-3);
#else	/* MACH_KERNEL */
		sec_init_mbufq(&softp->is_mbufq, softp->is_reqq.sq_size-3);
#endif	/* MACH_KERNEL */
		softp->is_status = 0;
		softp->is_initted = 1;
	} else if (sd->sd_chan == SDEV_ETHERWRITE) {
		long cur_brk;

		init_lock(&softp->os_lock, se_gate);
		softp->os_cib = sd->sd_cib;
		softp->os_cib->cib_status =
			KVTOPHYS(&softp->os_status, int *);
		softp->os_status = 0;
		softp->os_reqq.sq_progq = sd->sd_requestq;
		softp->os_reqq.sq_size = sd->sd_req_size;
		softp->os_doneq.sq_progq = sd->sd_doneq;
		softp->os_doneq.sq_size = sd->sd_doneq_size;
		SEC_fill_progq(softp->os_reqq.sq_progq,
			(int)softp->os_reqq.sq_size,
			(int)sizeof(struct sec_dev_prog));
		for (i = 0; i < softp->os_reqq.sq_size; ++i) {
			register struct sec_dev_prog *dp;

			dp =
			  PHYSTOKV(softp->os_reqq.sq_progq->pq_un.pq_progs[i],
				   struct sec_dev_prog *);
			bcopy((caddr_t)se_scsi_cmd,
				(caddr_t)dp->dp_cmd, sizeof se_scsi_cmd);
			dp->dp_cmd_len = sizeof se_scsi_cmd;
		}

		/*
		 * transmit buffer can't cross 64K boundary
		 */

		cur_brk = (long)calloc(0);
		if ((cur_brk & 0xFFFF0000)
		    != ((cur_brk+OS_BUF_SIZE) & 0xFFFF0000)) {
			callocrnd((int)0x10000);
		}
		softp->os_buf = (u_char *)calloc(OS_BUF_SIZE);
		sec_init_iatq(&softp->os_iatq, (unsigned)se_write_iats);
#ifdef	MACH_KERNEL
		softp->os_pending = (io_req_t) 0;
#else	/* MACH_KERNEL */
		softp->os_pending = (struct mbuf *)0;
#endif	/* MACH_KERNEL */
		softp->os_initted = 1;
		softp->os_active = 0;
	} else {
		printf("%s%d: invalid device chan %d (0x%x) in boot routine\n",
			se_driver.sed_name, softp-se_state,
			sd->sd_chan, sd->sd_chan);
		panic("se_bootone");
	}
}

/*
 * se_boot - allocate data structures, etc at beginning of time.
 *
 * Called with an array of configured devices and the number
 * of ether devices.
 * We allocate the necessary soft descriptions, and fill them
 * in with info from the devs[] array.
 * Due to the dual-device-channel nature of the SEC description,
 * each entry in the devs[] array describes either the Ether input
 * device or the Ether output device.  We combine this into a
 * single device state per controller.
 *
 * The program queues are allocated by the routine that calls us,
 * but the device programs themselves are not allocated til now.
 *
 * Work related to allocating mbufs is done later at se_init time.
 */

static
se_boot(ndevs, devs)
	struct sec_dev devs[];
{
	register int i;

	/*
	 * First, allocate soft descriptions.
	 */

	se_max_unit = ndevs/2;
	se_state = (struct se_state *)calloc((se_max_unit+1)
		* sizeof(struct se_state));
	se_base_vec = devs[0].sd_vector;

	/*
	 * Now, boot each configured device.
	 */

	for (i = 0; i < ndevs; ++i) {
		register struct sec_dev *devp;
		register int unit;

		devp = &devs[i];
		if (devp->sd_alive == 0)
			continue;
		unit = i/2;
		se_boot_one(&se_state[unit], devp);
	}
}

/*
 * Initialization of interface; clear recorded pending
 * operations, and reinitialize SCSI/Ether usage.
 */

static
se_init(unit)
	int unit;
{
	register struct se_state *softp = &se_state[unit];
	register struct ifnet *ifp;
	register int i;
#ifdef	MACH_KERNEL
#else	/* MACH_KERNEL */
	struct mbuf *m;
	struct sockaddr_in *sin;
#endif	/* MACH_KERNEL */
	spl_t sipl;

	if (unit < 0 || unit > se_max_unit) {
		printf("%s%d: invalid unit in init\n",
			se_driver.sed_name, unit);
		return;
	}
	sipl = OS_LOCK(softp);
	(void) SS_LOCK(softp);

	if (!softp->ss_alive || !softp->ss_initted || !softp->is_initted
	    || !softp->os_initted)
		goto ret;

#ifdef	MACH_KERNEL
	ifp = &softp->ss_if;
#else	MACH_KERNEL
	ifp = &softp->ss_arp.ac_if;
#ifndef	MACH
	sin = (struct sockaddr_in *)&ifp->if_addr;
	if (sin->sin_addr.s_addr == 0)
		goto ret;	 /* address still unknown */
#else
	/* not yet, if address still unknown */
	if (ifp->if_addrlist == (struct ifaddr *)0)
		goto ret;
#endif	/* MACH */
#endif	/* MACH_KERNEL */

	if (ifp->if_flags & IFF_RUNNING)
		goto justarp;

	if (softp->ss_init_called)
		goto justarp;

#ifdef	MACH_KERNEL
	timeout(se_watch, (char *)0, softp->ss_scan_int);
#else	/* MACH_KERNEL */
	ifp->if_watchdog = se_watch;
	ifp->if_timer = softp->ss_scan_int;
#endif	/* MACH_KERNEL */

#ifdef	MACH_KERNEL
	/*
	 * Set the Ether modes before we add input programs,
	 * as the firmware will need to know the size of the
	 * input packets before we do the SINST_STARTIO.
	 */

	se_set_modes(softp);

	/*
	 * Allocate some net_kmsgs for input packets and fill in
	 * the iats.
	 */
	for (i = 0; i < softp->is_msgq.mq_size; ++i) {
	    register ipc_kmsg_t nk;

	    nk = net_kmsg_alloc();

	    se_add_read_progs(softp, nk);
	}
#else	MACH_KERNEL
	/*
	 * Allocate the mbufs parallel to the iat queue for input packets
	 * and fill in the iats.
	 * We allocate the mbufs one at a time because expansion happens
	 * slowly (a page cluster at a time) and we ask for a lot
	 * of mbufs at once.
	 */

	m = (struct mbuf *)0;
	for (i = 0; i < softp->is_mbufq.mq_size; ++i) {
		register struct mbuf *newm = m_getm(M_DONTWAIT, MT_DATA, 1);

		if (newm == (struct mbuf *)0) {
			printf("%s%d: can't allocate %d mbufs!\n",
				se_driver.sed_name, softp-se_state,
				softp->is_mbufq.mq_size);
			m_freem(m);
			goto ret;
		}
		newm->m_next = m;
		m = newm;
	}

	/*
	 * Set the Ether modes before we add input programs,
	 * as the firmware will need to know the size of the
	 * input packets before we do the SINST_STARTIO.
	 */

	se_set_modes(softp);
	se_add_read_progs(softp, m);
#endif	/* MACH_KERNEL */

	if (sec_start_prog(SINST_STARTIO, softp->is_cib, softp->ss_slic,
			   softp->ss_bin, SDEV_ETHERREAD, 1)
	    != SEC_ERR_NONE) {
		printf("%s%d: can't initialize.\n",
			se_driver.sed_name, softp-se_state);
		goto ret;
	}

	/*
	 * Shouldn't have to restart output to the device,
	 * as nothing was reset.
	 */

#ifndef	MACH
	ifp->if_flags |= IFF_UP|IFF_RUNNING;
#else
	ifp->if_flags |= IFF_RUNNING;
#endif	/* MACH */
	softp->ss_init_called = 1;

justarp:
#ifdef	MACH_KERNEL
	SS_UNLOCK(softp, SPLIMP);
	OS_UNLOCK(softp, sipl);
#else	MACH_KERNEL
#ifndef	MACH
	se_set_modes(softp);
	if_rtinit(ifp, RTF_UP);
	arpattach(&softp->ss_arp);
	SS_UNLOCK(softp, SPLIMP);
	OS_UNLOCK(softp, sipl);
	arpwhohas(&softp->ss_arp, &sin->sin_addr);
#endif	/* MACH */
#endif	/* MACH_KERNEL */
	return;
ret:
	SS_UNLOCK(softp, SPLIMP);
	OS_UNLOCK(softp, sipl);
}

#ifdef	MACH_KERNEL
se_open(unit, flag)
	int	unit;
	int	flag;
{
	if (unit < 0 || unit > se_max_unit)
	    return (D_NO_SUCH_DEVICE);

	se_state[unit].ss_if.if_flags |= IFF_UP;
	se_init(unit);		/* XXX should return status */
	return (D_SUCCESS);
}

se_setinput(unit, receive_port, priority, filter, filter_count)
	int		unit;
	mach_port_t	receive_port;
	int		priority;
	filter_t	filter[];
	unsigned int	filter_count;
{
	if (unit < 0 || unit > se_max_unit)
	    return (D_NO_SUCH_DEVICE);

	return (net_set_filter(&se_state[unit].ss_if,
			receive_port, priority,
			filter, filter_count));
}


#endif	/* MACH_KERNEL */
/*
 * Ethernet interface interrupt routine.
 * Could be output or input interrupt.
 * We determine the source and call the appropriate routines
 * to decode the done queue programs.
 */

static
se_intr(vector)
	int vector;
{
	int unit = (vector - se_base_vec)/2;
	int is_read = (vector - 2*unit) == 0;
	register struct se_state *softp = &se_state[unit];
	register struct sec_pq *sq;
	spl_t sipl;

	if (unit < 0 || unit > se_max_unit) {
		printf("%s%d: invalid interrupt vector %d\n",
			se_driver.sed_name, unit, vector);
		return;
	}

	if (is_read) { 	/* Receiver interrupt.  */
		register struct sec_eprogq *epq;
#ifdef DEBUG
		if (se_ibug)
			printf("R%d ", unit);
#endif /* DEBUG */
		ASSERT(softp->ss_alive, "se_intr: alive");
		ASSERT(softp->ss_init_called, "se_intr: initted");
		sq = &softp->is_doneq;
		epq = (struct sec_eprogq *)sq->sq_progq;
		ASSERT(epq->epq_tail < sq->sq_size, "se_intr: tail");
		ASSERT(epq->epq_head < sq->sq_size, "se_intr: head");

		/*
		 * Lock the input state before we test for work.
		 * This keeps other processors out of the way once
		 * we commit to entering the loop and doing work.
		 * There is a race here between the decision to
		 * leave the loop and the v_lock that can cause an
		 * interrupt from the SCSI/Ether controller to be
		 * missed, but we say this is acceptable, as the
		 * net should always be busy, and we will eventually
		 * see the packet on the next interrupt.
		 */

		sipl = cp_lock(&softp->is_lock, SPLIMP);
		if (sipl == CPLOCKFAIL)
			return;
		while (epq->epq_tail != epq->epq_head) { /* work to do */
			se_handle_read(softp, &epq->epq_status[epq->epq_tail]);
			epq->epq_tail = (epq->epq_tail + 1) % sq->sq_size;
		}
		v_lock(&softp->is_lock, sipl);

	} else { 	/* Transmitter interrupt. */

		register struct sec_progq *pq;
		register struct sec_dev_prog *dp;
#ifdef DEBUG
		if (se_obug)
			printf("X%d ", unit);
#endif
		/*
		 * Since there is only one device program active
		 * at a time on ether output, it makes more sense to
		 * spin on the output side lock rather than conditionally
		 * lock an interrupt lock here.
		 * When we switch to multiple active device outputs
		 * per SCSI/Ether, this decision should be reversed,
		 * as there is the potential for idling an unbounded
		 * number of processors while Ether transmit interrupts
		 * occur.
		 */

		sipl = OS_LOCK(softp);
		if (!softp->ss_alive || !softp->ss_init_called) {
			OS_UNLOCK(softp, sipl);
			return;
		}
		sq = &softp->os_doneq;
		pq = sq->sq_progq;

		ASSERT(pq->pq_tail < sq->sq_size, "se_intr: tail 2");
		ASSERT(pq->pq_head < sq->sq_size, "se_intr: head 2");
		if (!softp->os_active) {	 /* spurious interrupt */
			OS_UNLOCK(softp, sipl);
			return;
		}
		while (pq->pq_tail != pq->pq_head) {
			dp = PHYSTOKV(pq->pq_un.pq_progs[pq->pq_tail],
				      struct sec_dev_prog *);
			if (dp->dp_status1 != 0) {
				int status = sec_start_prog(SINST_RESTARTIO,
						   softp->os_cib,
						   softp->ss_slic,
						   softp->ss_bin,
						   SDEV_ETHERWRITE, 1);
				if (status != SEC_ERR_NONE
				    && status != SEC_ERR_NO_MORE_IO) {
					printf("%s%d: se_intr: status 0x%x\n",
						se_driver.sed_name,
						softp-se_state,
						softp->os_status);
				}
			}
			pq->pq_tail = (pq->pq_tail + 1) % sq->sq_size;
			softp->os_active = 0;
			ASSERT(softp->os_pending != (struct mbuf *)0,
				"se_intr: os_pending");
#ifdef PROMISCUOUS
		if (promiscon) {
			struct mbuf * xm;
			struct promiscif * xpm;

			/*
			 * manage monitor receipt of transmitted packets.
			 */

			xm = m_getm(M_DONTWAIT, MT_DATA, 1);
			(void) IF_LOCK(&promiscq);
			if (!xm || IF_QFULL(&promiscq)) {
				IF_DROP(&promiscq);
				IF_UNLOCK(&promiscq, SPLIMP);
				m_freem(softp->os_pending);
				if(xm) (void) m_free(xm);
			}else{
				xm->m_next = softp->os_pending;
				xm->m_len = sizeof(struct promiscif);
				xpm = mtod(xm, struct promiscif *);
				xpm->promiscif_ifnet = (caddr_t) softp;
				xpm->promiscif_flag = PROMISC_XMIT;

				IF_ENQUEUE(&promiscq, xm);
				if (!promiscq.ifq_busy) {
					schednetisr(NETISR_PROMISC);
				}
				IF_UNLOCK(&promiscq, SPLIMP);
			}

		} else		/* not monitoring */

#endif /* PROMISCUOUS */
#ifdef	MACH_KERNEL
			iodone(softp->os_pending);
			softp->os_pending = 0;
#else	
			m_freem(softp->os_pending);
			softp->os_pending = (struct mbuf *)0;
#endif	/* MACH_KERNEL */
			/* Should only be one program in the queue */
			ASSERT(pq->pq_tail == pq->pq_head,"se_intr: head/tail");
		}
		OS_UNLOCK(softp, sipl);
		se_start(softp);
	}
}

#ifdef	MACH_KERNEL

/*
 * We set the offset for receives so that the data portion of a packet
 * lands at sizeof(struct packet_header) into the data portion
 * of a network message.  That leaves the Ethernet type word at
 * the 'type' field of the packet_header.
 */
#define	ETHER_HDR_OFF \
	(sizeof(struct packet_header) - sizeof(struct ether_header))

se_handle_read(softp, statp)
	register struct se_state *softp;
	struct sec_ether_status *statp;
{
	struct sec_msgq *mq = &softp->is_msgq;
	ipc_kmsg_t	old_kmsg, new_kmsg;
	struct ifnet	*ifp = &softp->ss_if;
	int		len;
	char		*old_addr;

	old_kmsg = mq->mq_msgs[mq->mq_head];
	old_addr = &net_kmsg(old_kmsg)->packet[ETHER_HDR_OFF];

	if (KVTOPHYS(old_addr, u_char *) != statp->es_data) {
		struct sec_iatq *iq = &softp->is_iatq;
		struct sec_iat *iat = &iq->iq_iats[iq->iq_head];
		register int i;

		printf("%s%d: botch: statp 0x%x from mq 0x%x; es_data 0x%x\n",
			se_driver.sed_name, softp-se_state, statp, mq,
			statp->es_data);
		printf("mq->mq_head %d KVTOPHYS(old_kmsg) 0x%x\n",
			mq->mq_head, KVTOPHYS(old_kmsg, int));
		printf("iatq 0x%x iq->iq_head %d iat 0x%x addr 0x%x count %d\n",
			iq, iq->iq_head, iat, iat->iat_data, iat->iat_count);
		for (i = 0; i < 500000; ++i) {
			if (KVTOPHYS(old_kmsg, int) == (int)statp->es_data)
				break;
		}
	}

	len = (int)statp->es_count;

#ifdef DEBUG
	if (se_ibug) {
		printf("got %d ", statp->es_count);
	}
#endif 

	softp->ss_if.if_ipackets++;

	new_kmsg = net_kmsg_get();
	if (new_kmsg == 0) {
		/*
		 * Cannot allocate replacement message.
		 * Use the old one again.
		 */
		softp->ss_if.if_rcvdrops++;

		se_add_read_progs(softp, old_kmsg);
		if (sec_start_prog(SINST_STARTIO, softp->is_cib,
				   softp->ss_slic, softp->ss_bin,
				   SDEV_ETHERREAD, 1)
		    != SEC_ERR_NONE) {
			printf("%s%d: se_handle_read: status 0x%x\n",
				se_driver.sed_name, softp-se_state,
				softp->is_status);
		}
#ifdef DEBUG
		if (se_ibug > 1)
			printf("lose%d ", n);
#endif
		return;
	}

	/*
	 *	Replace the kmsg in the queue.
	 */

	se_add_read_progs(softp, new_kmsg);

	if (sec_start_prog(SINST_STARTIO, softp->is_cib, softp->ss_slic,
			   softp->ss_bin, SDEV_ETHERREAD, 1)
	    != SEC_ERR_NONE) {
		printf("%s%d: se_handle_read: status 0x%x\n",
			se_driver.sed_name, softp-se_state, softp->is_status);
	}
#ifdef DEBUG
	if (se_ibug > 1)
		printf("repl%d ", n);
#endif

	/*
	 * Fill in the missing fields of the old kmsg.
	 */
	{
	    register struct ether_header *eh;
	    register struct packet_header *ph;

	    eh = (struct ether_header *) &net_kmsg(old_kmsg)->header[0];
	    ph = (struct packet_header *) &net_kmsg(old_kmsg)->packet[0];

	    /*
	     * Copy the Ethernet header from where it was received.
	     */
	    *eh = *(struct ether_header *)old_addr;

	    /*
	     * Set up the type and length fields in the packet header.
	     */
	    ph->type = eh->ether_type;
	    ph->length = len - sizeof(struct ether_header)
			     + sizeof(struct packet_header);

	    /*
	     * Hand the packet to the network module.
	     */
	    net_packet(&softp->ss_if, old_kmsg, ph->length,
		       ethernet_priority(old_kmsg));
	}
#ifdef DEBUG
	if (se_ibug) printf("\n");
#endif
}

/*
 * se_add_read_progs - Add a read program to the request queue.
 *
 * We replace the net_kmsgs in the kmsg queue 'mq' here.
 * 'm' is a net_kmsg.
 */
se_add_read_progs(softp, m)
	register struct se_state *softp;
	register ipc_kmsg_t m;
{
	struct sec_msgq *mq;
	struct sec_iatq *iq;
	struct sec_pq *sq;
	struct sec_progq *pq;
	struct sec_edev_prog *dp;
	struct sec_iat *iat;

	mq = &softp->is_msgq;
	iq = &softp->is_iatq;
	sq = &softp->is_reqq;
	pq = sq->sq_progq;

	/* We do the entire msg in one piece. */

	assert(iq->iq_size - iq->iq_head >= 1);
	assert(mq->mq_size - mq->mq_head >= 1);

	/*
	 * Point iats at the net_kmsg.
	 * It is illegal to wrap around the ring.
	 */
	iat = &iq->iq_iats[iq->iq_head];

	iat->iat_data = KVTOPHYS(&net_kmsg(m)->packet[ETHER_HDR_OFF],
				 u_char *);
	iat->iat_count = sizeof(struct ether_header) + ETHERMTU;

	iat = &iq->iq_iats[iq->iq_head];
	iq->iq_head = (iq->iq_head + 1) % iq->iq_size;

	mq->mq_msgs[mq->mq_head] = m;
	mq->mq_head = (mq->mq_head + 1) % mq->mq_size;

	/*
	 * Add the device program.
	 */
	assert((pq->pq_head + 1) % sq->sq_size != pq->pq_tail);
	dp = PHYSTOKV(pq->pq_un.pq_eprogs[pq->pq_head],
		      struct sec_edev_prog *);
	assert(dp != 0);
	dp->edp_iat_count = 1;
	dp->edp_iat = SEC_IATIFY(KVTOPHYS(iat, vm_offset_t));
	pq->pq_head = (pq->pq_head + 1) % sq->sq_size;

}

#else	MACH_KERNEL
/*
 * Handle read interrupt requests.
 * This includes recognizing trailer protocol,
 * and passing up to the higher level software.
 *
 * This is called with the input state locked.
 */

extern struct custom_client custom_clients[];

static
se_handle_read(softp, statp)
	register struct se_state *softp;
	struct sec_ether_status *statp;
{
	struct sec_mbufq *mq = &softp->is_mbufq;
	struct mbuf *m, *mnew;
#ifdef PROMISCUOUS
	struct mbuf *mpromisc;
	struct promiscif * mp;
#endif 
	int len, n, int_to_sched;
	struct ether_header *hp;
	struct ifqueue *inq;
	spl_t sipl;
	struct ifnet *ifp = &softp->ss_arp.ac_if;
#ifdef RAW_ETHER
	struct raw_header * rh;
	struct mbuf * mrh;
#endif 
	int	trailer = 0;
	int	ci;

	m = mq->mq_mbufs[mq->mq_head];
	if ((u_char *)KVIRTTOPHYS(mtod(m, int)) != statp->es_data) {
		struct sec_iatq *iq = &softp->is_iatq;
		struct sec_iat *iat = &iq->iq_iats[iq->iq_head];
		register int i;

		printf("%s%d: botch: statp 0x%x from mq 0x%x; es_data 0x%x\n",
			se_driver.sed_name, softp-se_state, statp, mq,
			statp->es_data);
		printf("mq->mq_head %d KVIRTTOPHYS(m) 0x%x\n",
			mq->mq_head, KVIRTTOPHYS(mtod(m, int)));
		printf("iatq 0x%x iq->iq_head %d iat 0x%x addr 0x%x count %d\n",
			iq, iq->iq_head, iat, iat->iat_data, iat->iat_count);
		for (i = 0; i < 500000; ++i) {
			if (KVIRTTOPHYS(mtod(m, int)) == (int)statp->es_data)
				break;
		}
	}

	n = howmany(statp->es_count, MLEN);
	m = sec_chain_mbufs(mq, n, (int)statp->es_count);

#ifdef DEBUG
	if (se_ibug) {
		printf("got%d ", statp->es_count);
	}
#endif

	softp->ss_arp.ac_if.if_ipackets++;

#ifdef PROMISCUOUS

	/*
	 * get an mbuf for passing softp to promiscintr.
	 */

	mnew = m_getm(M_DONTWAIT, MT_DATA, n+1);
#else
	mnew = m_getm(M_DONTWAIT, MT_DATA, n);

#endif /* PROMISCUOUS */

	if (mnew == 0) {

		/*
		 * Can't allocate replacement mbufs.
		 * Go ahead and use the old ones again.
		 */

		se_add_read_progs(softp, m);
		if (sec_start_prog(SINST_STARTIO, softp->is_cib,
				   softp->ss_slic, softp->ss_bin,
				   SDEV_ETHERREAD, 1)
		    != SEC_ERR_NONE) {
			printf("%s%d: se_handle_read: status 0x%x\n",
				se_driver.sed_name, softp-se_state,
				softp->is_status);
		}
#ifdef DEBUG
		if (se_ibug > 1)
			printf("lose%d ", n);
#endif 
		softp->ss_arp.ac_if.if_ierrors++;
		return;
	}

	/*
	 * Replace the mbufs in the circular mbuf queue.
	 */

#ifdef PROMISCUOUS

	/*
	 * strip off promiscif mbuf
	 */

	mpromisc = mnew;
	mnew = mnew->m_next;
	mpromisc->m_next = m;
	mpromisc->m_len = sizeof(struct promiscif);

#endif 

	se_add_read_progs(softp, mnew);
	if (sec_start_prog(SINST_STARTIO, softp->is_cib, softp->ss_slic,
			   softp->ss_bin, SDEV_ETHERREAD, 1)
	    != SEC_ERR_NONE) {
		printf("%s%d: se_handle_read: status 0x%x\n",
			se_driver.sed_name, softp-se_state, softp->is_status);
	}
#ifdef DEBUG
	if (se_ibug > 1)
		printf("repl%d ", n);
#endif

	/*
	 * m now contains a packet from the interface.
	 * Check for trailer protocol.
	 */

	hp = mtod(m, struct ether_header *);
	len = statp->es_count;
	ASSERT(len == mbuf_chain_size(m), "se_handle_read: size");
	len -= sizeof(struct ether_header);
	if (len < ETHERMIN) {

#ifdef DEBUG
		if (se_ibug)
			printf("packet returned of size %d\n", len);
#endif
		++softp->ss_arp.ac_if.if_ierrors;

#ifdef PROMISCUOUS
		m_freem(mpromisc);
#else
		m_freem(m);
#endif

		return;
	}

#ifdef PROMISCUOUS
	if (promiscon) { /* promiscon => give promiscintr the packet */
		inq = &promiscq;
		int_to_sched = NETISR_PROMISC;
		mp = mtod(mpromisc, struct promiscif *);
		mp->promiscif_ifnet = (caddr_t)softp;
		mp->promiscif_flag = PROMISC_RCVD;
		sipl = IF_LOCK(inq);
		if (IF_QFULL(inq)) {
			IF_DROP(inq);
			IF_UNLOCK(inq, sipl);
			m_freem(mpromisc);
			return;
		}
		IF_ENQUEUE(inq, mpromisc);
		if (!inq->ifq_busy) {
			schednetisr(int_to_sched);
		}
		IF_UNLOCK(inq, sipl);
		return;
	}

#endif /* PROMISCUOUS */

	/*
	 * check for SETHER_PROMISCUOUS but !promiscon
	 */

	if (softp->ss_ether_flags == SETHER_PROMISCUOUS) {
		if (bcmp((char *)etherbroadcastaddr,
			 (char *)hp->ether_dhost, 6) != 0
		    && bcmp((char *)softp->ss_arp.ac_enaddr,
			    (char *)hp->ether_dhost, 6) != 0) {

#ifdef PROMISCUOUS
			m_freem(mpromisc);
#else
			m_freem(m); 	/* throw promiscuous packets away. */
#endif 
			return;
		}
	}

#ifdef PROMISCUOUS

	(void) m_free(mpromisc); /* promiscoff, no need for promiscif buffer */

#endif 

	m->m_off += sizeof(struct ether_header);
	m->m_len -= sizeof(struct ether_header);
	hp->ether_type = ntohs((u_short)hp->ether_type);
	if (hp->ether_type >= ETHERPUP_TRAIL
	    && hp->ether_type < ETHERPUP_TRAIL + ETHERPUP_NTRAILER) {
		mnew = se_reorder_trailer_packet(hp, m);
		if (mnew == (struct mbuf *)0) {
			m_freem(m);
			return;
		}
		m = mnew;
		trailer++;
	}
#ifdef	MACH
	/*
	 * 4.3 wants the interface pointer inserted in front of the
	 * data -- do this.  For now, just tack on an mbuf to front
	 * of the mbuf chain (this works for both trailers and non-trailer
	 * packets).  This can likely be done more efficiently.
	 * 
	 * See IF_ADJ()/IF_DEQUEUEIF().
	 */
	MGET(mnew, M_DONTWAIT, MT_DATA);
	if (mnew == (struct mbuf *) NULL) {
		m_freem(m);
		return;
	}
	mnew->m_len = sizeof (struct ifnet *);
	*(mtod(mnew, struct ifnet **)) = ifp;
	mnew->m_next = m;
	m = mnew;
#endif	/* MACH */

#ifdef DEBUG
	if (se_ibug)
		printf("type0x%x ", hp->ether_type);
#endif 

	switch (hp->ether_type) {
#ifdef INET
	case ETHERPUP_IPTYPE:
#ifdef DEBUG
		if (se_ibug) {
			printf("ip ");
			if (hp->ether_dhost[0] & 0x01)
				printf("broad ");
			else	printf("station ");
		}
#endif /* DEBUG */
		int_to_sched = NETISR_IP;
		inq = &ipintrq;
		break;

	case ETHERPUP_ARPTYPE:
#ifdef DEBUG
		if (se_ibug)
			printf("arp\n");
#endif /* DEBUG */
		arpinput(&softp->ss_arp, m);
		return;
#endif /* INET */

#ifndef	MACH
	case PCI_TYPE:
		pcirint(hp, m);
		return;
#endif	MACH

	default:

#ifndef	MACH
		/* do not queue reordered trailer to rawif or custom */

		if(trailer) {
			m_freem(m);
			return;
		}

	 	/* allow for custom ether_read device drivers */

		for(ci = 0; ci < 4; ci++) {
		  if(custom_clients[ci].custom_devno
			&& custom_clients[ci].custom_type == hp->ether_type)
		  {

	    ASSERT(cdevsw[major(custom_clients[ci].custom_devno)].d_read,
			"no custom_client cdevsw.d_read!");

		        (*cdevsw[major(custom_clients[ci].custom_devno)].d_read)
				(hp, m, ifp);

		  	custom_clients[ci].custom_count++;
		  	return;
		  }
		}

#ifdef RAW_ETHER

		/*
		 * reput the ether header into the lead data buffer
		 * *and* copy a Unix4.2 raw_header for compatibility
		 */
	
		m->m_off -= sizeof(struct ether_header);
		m->m_len += sizeof(struct ether_header);
		int_to_sched = NETISR_RAW;
		inq = &rawif.if_snd;
		mrh = m_getclrm(M_DONTWAIT, MT_DATA, 1);
		if(mrh == (struct mbuf *) NULL) {
			m_freem(m);
			return;
		}
	
		/*
		 * link the raw_header into the ether packet for 4.2
		 * compatibility (?)
		 *
		 * set up raw header, using type as sa_data for bind.
		 * raw_input() could do this if static struct set up.
		 * 	- for now assign AF_UNSPEC for protocol
		 */

		mrh->m_next = m;
		m = mrh;
		rh = mtod(mrh, struct raw_header*);
		rh->raw_proto.sp_family = AF_RAWE;
		rh->raw_proto.sp_protocol = AF_UNSPEC;

		/*
		 * copy AF_RAWE and ether_type in for dst addr
		 */

		rh->raw_dst.sa_family = AF_RAWE;
		bcopy((caddr_t)&hp->ether_type,
			(caddr_t)rh->raw_dst.sa_data, 2);
		bcopy((caddr_t)&hp->ether_type,
			(caddr_t)rh->raw_src.sa_data, 2);

		/*
		 * put type back into net order
		 */

		hp->ether_type = htons(hp->ether_type);

		/*
		 * copy AF_RAWE and if_unit # in for src addr
		 */

		rh->raw_src.sa_family = AF_RAWE;
		bcopy((caddr_t)&ifp->if_unit,
			(caddr_t)&rh->raw_src.sa_data[2], sizeof(short));

#else		/* not RAW_ETHER */

		m_freem(m);
		return;

#endif RAW_ETHER
#else	MACH
		m_freem(m);
		return;
#endif	MACH

	}	/* end switch */

#ifndef	MACH
	sipl = IF_LOCK(inq);
#else
	sipl = splimp();
#endif	MACH
	if (IF_QFULL(inq)) {
		IF_DROP(inq);
#ifndef	MACH
		IF_UNLOCK(inq, sipl);
#else
		splx(sipl);
#endif	MACH
		m_freem(m);
		return;
	}
	IF_ENQUEUE(inq, m);
#ifndef	MACH
	if (!inq->ifq_busy) {
		schednetisr(int_to_sched);
	}
	IF_UNLOCK(inq, sipl);
#else
	schednetisr(int_to_sched);
	splx(sipl);
#endif	MACH
#ifdef DEBUG
	if (se_ibug) printf("\n");
#endif DEBUG
}



/*
 * se_reorder_trailer_packet - return a real mbuf chain after noticing trailer
 *	protocol is being used.
 *
 * Return the new chain, and modify the header to reflect the real type.
 * Return a null mbuf if we couldn't do it.
 * It is the responsibility of the caller to free the original, if necessary.
 */

struct trailer {
	u_short tl_type;
	u_short tl_count;
};

#ifdef PROMISCUOUS

/* N.B. this routine is not static so promiscq handler can call it */

struct mbuf *se_reorder_trailer_packet(hp, m)

#else

static struct mbuf *se_reorder_trailer_packet(hp, m)

#endif PROMISCUOUS

	struct ether_header *hp;
	register struct mbuf *m;
{
	register struct mbuf *mnew, *split;
	int trail_off;
	struct trailer *trailerp;

	/*
	 * find mbuf where we have to split things.
	 */

	trail_off = (hp->ether_type - ETHERPUP_TRAIL)*512;
	if (trail_off != 512 && trail_off != 1024) {
		printf("%s: ignore trailer with type 0x%x\n",
			se_driver.sed_name, hp->ether_type);
		return((struct mbuf *)0);
	}
	split = m;
	for (; split != 0 && split->m_len <= trail_off; split = split->m_next)
		trail_off -= split->m_len;
#ifdef DEBUG
	if (se_ibug > 1)
		printf("split 0x%x trail_off %d ", split, trail_off);
#endif DEBUG

	if (split == (struct mbuf *)0)
		return((struct mbuf *)0);

	/*
	 * trail_off has the index into 'split' of the trailer.
	 * Lots of potential boundary conditions here that should
	 * be checked, but since we know the size of data blocks
	 * in trailer-protocol packets  == 512 or 1024 and MLEN == 112,
	 * we are guaranteed that the trailer header is completely
	 * embedded in a single mbuf.
	 */

	trailerp = (struct trailer *)(mtod(split, int) + trail_off);
	if (trail_off + sizeof(struct trailer) > split->m_len)
		return((struct mbuf *)0);

	MGET(mnew, M_DONTWAIT, MT_DATA);
	if (mnew == 0)
		return((struct mbuf *)0);

	/*
	 * Know where to split, and have place for start of header.
	 * Build real header by copying and chaining.
	 */

	mnew->m_off = MMINOFF;
	mnew->m_len = split->m_len - trail_off - sizeof(struct trailer);
	mnew->m_next = split->m_next;
	bcopy((caddr_t)(trailerp+1), mtod(mnew, caddr_t), (u_int)mnew->m_len);
	hp->ether_type = ntohs(trailerp->tl_type);

#ifdef DEBUG
	if (se_ibug > 1)
		printf("new len %d tlr len %d trtype0x%x ",
			mnew->m_len, ntohs(trailerp->tl_count), hp->ether_type);
#endif DEBUG

	split->m_len = trail_off;
	split->m_next = 0;

#ifdef DEBUG
	if (ntohs(trailerp->tl_count) !=
	    sizeof(struct trailer) + mbuf_chain_size(mnew)) {
		printf("%s: odd trailer count %d, expected %d+%d\n",
			se_driver.sed_name, ntohs(trailerp->tl_count),
			sizeof(struct trailer), mbuf_chain_size(mnew));
	}
#endif DEBUG

	mnew->m_next = m;
	return(mnew);
}


/*
 * se_add_read_progs - Add one or two read programs to the request queue.
 *
 * We replace the mbufs in the mbuf queue 'mq' here.  'm' is an
 * mbuf chain of the appropriate length.
 */

static
se_add_read_progs(softp, m)
	register struct se_state *softp;
	register struct mbuf *m;
{
	struct sec_mbufq *mq;
	struct sec_iatq *iq;
	struct sec_pq *sq;
	struct sec_progq *pq;
	struct sec_edev_prog *dp;
	int n;

	mq = &softp->is_mbufq;
	iq = &softp->is_iatq;
	n = mbuf_chain_length(m);
	sq = &softp->is_reqq;
	pq = sq->sq_progq;

	while (n > 0) {
		int nnow = MIN(n, iq->iq_size - iq->iq_head);
		struct sec_iat *iat;
		struct mbuf *mnext;

#ifdef DEBUG
		if (se_ibug > 1)
			printf("add%d ", nnow);
#endif DEBUG
		ASSERT(nnow >= 1, "se_add_read: nnow 1");
		ASSERT(nnow == MIN(n, mq->mq_size - mq->mq_head),
			"se_add_read: nnow 2");
		if (nnow != n) {
			register struct mbuf *mprev;
			int i;

			for (mprev = m, i = 0;  i < nnow-1;  ++i, mprev = mprev->m_next)
				continue;
			mnext = mprev->m_next;
			mprev->m_next = (struct mbuf *)0;
			ASSERT(mbuf_chain_length(m) == nnow, "se_add_read: length");
			ASSERT(mbuf_chain_length(mnext) == n - nnow,
				"se_add_read: length 2");
		} else {
			mnext = (struct mbuf *)0;
		}

		/*
		 * m now has chain to spray into iats.
		 * mnext has the rest of the chain.
		 */

		iat = sec_spray_mbuf_iatq(m, iq);
		if (iat == 0)
			panic("se_add_read_progs");

		(void) sec_spray_mbuf_mbufq(m, mq);

		/*
		 * add the device program.
		 */

		ASSERT((pq->pq_head + 1) % sq->sq_size != pq->pq_tail,
			"se_add_read: bad head");
		dp = pq->pq_un.pq_eprogs[pq->pq_head];
		ASSERT(dp != (struct sec_edev_prog *)0, "se_add_read: dp 0");
		dp->edp_iat_count = nnow;
		dp->edp_iat = SEC_IATIFY(iat);
		pq->pq_head = (pq->pq_head + 1) % sq->sq_size;

		n -= nnow;
		m = mnext;
	}
}
#endif	MACH_KERNEL
/*
 * Ethernet output routine.
 * Encapsulate a packet of type family for the local net.
 * If this packet is a broadcast packet or is destined for
 * ourselves, we pass a copy of it through the loopback
 * interface, as the SEEQ chip is not capable of hearing
 * its own transmissions.
 */
#ifdef	MACH_KERNEL
se_output(dev, ior)
	dev_t	dev;
	io_req_t	ior;
{
	register int	unit = minor(dev);
	register struct se_state *softp = &se_state[unit];

	return (net_write(&softp->ss_if, se_start_u, ior));
}

se_start_u(unit)
	int	unit;
{
	se_start(&se_state[unit]);
}

#else	MACH_KERNEL
static
se_output(ifp, m, dest)
	struct ifnet *ifp;
	register struct mbuf *m;
	struct sockaddr *dest;
{
	register struct se_state *softp = &se_state[ifp->if_unit];
	u_char ether_dest[6];
	register struct ether_header *header;
	int type;
	spl_t sipl;
	extern struct ifnet loif;
#ifdef	MACH
	int	usetrailers;
#endif	MACH

#ifdef	MACH
	if ((ifp->if_flags & (IFF_UP|IFF_RUNNING)) != (IFF_UP|IFF_RUNNING)) {
		m_freem(m);
		return(ENETDOWN);
	}
#endif	MACH

#ifdef DEBUG
	if (se_obug) {
		printf("O%d ", ifp->if_unit);
		if (se_obug > 1) {
			printf("se_output called with...\n");
			dump_mbuf_chain(m);
		}
	}
#endif DEBUG
	switch (dest->sa_family) {
#ifdef INET
	case AF_INET: {
		struct in_addr inet_dest;
		register struct mbuf *m0 = m;
		int off;
		struct trailer *tl;

		inet_dest = ((struct sockaddr_in *)dest)->sin_addr;
#ifndef	MACH
		if (!arpresolve(&softp->ss_arp, m, &inet_dest, ether_dest))
			return(0);	/* Not yet resolved */
#else
		/*
		 * New parameter, tells if should use trailers.
		 * Need the paramater, but can ignore the result.
		 */
		if (!arpresolve(&softp->ss_arp, m, &inet_dest, ether_dest, &usetrailers))
			return(0);	/* Not yet resolved */
#endif	MACH
		if (in_lnaof(inet_dest) == INADDR_ANY) {
			struct mbuf *copy = (struct mbuf *)0;

			copy = m_copy(m, 0, (int)M_COPYALL);
			if (copy != (struct mbuf *)0)
				(void) looutput(&loif, copy, dest);
		}

		/* Generate trailer protocol? */

		off = ntohs((u_short)mtod(m, struct ip *)->ip_len) - m->m_len;
		if ((ifp->if_flags & IFF_NOTRAILERS) == 0
		    && off > 0 && (off & 0x1FF) == 0
		    && m->m_off >= MMINOFF + sizeof(struct trailer)) {
			type = ETHERPUP_TRAIL + (off >> 9);
			m->m_off -= sizeof(struct trailer);
			m->m_len += sizeof(struct trailer);
			tl = mtod(m, struct trailer *);
			tl->tl_type = htons((u_short)ETHERPUP_IPTYPE);
			tl->tl_count = htons((u_short)m->m_len);

			/*
			 * Move first packet (control information)
			 * to end of chain.
			 */

			while (m0->m_next)
				m0 = m0->m_next;
			m0->m_next = m;
			m0 = m->m_next;
			m->m_next = (struct mbuf *)0;
			m = m0;
		} else {
			type = ETHERPUP_IPTYPE;
		}
		}
		break;
#endif INET

	case AF_UNSPEC:
		header = (struct ether_header *)dest->sa_data;
		bcopy((caddr_t)header->ether_dhost, (caddr_t)ether_dest,
			sizeof(ether_dest));
		type = header->ether_type;
		break;

	default:
		printf("%s%d: can't handle address family %d\n",
			se_driver.sed_name, softp-se_state, dest->sa_family);
		m_freem(m);
		return(EAFNOSUPPORT);
	}

	/*
	 * Add the local header.
	 * Always add a new mbuf for the header so that we can compress
	 * short mbufs at the front easily.
	 */
	{
		register struct mbuf *m0;

		MGET(m0, M_DONTWAIT, MT_HEADER);
		if (m0 == (struct mbuf *)0) {
			m_freem(m);
			return(ENOBUFS);
		}
		m0->m_next = m;
		m0->m_off = MMINOFF;
		m0->m_len = sizeof(struct ether_header);
		m = m0;
	}

	header = mtod(m, struct ether_header *);
	header->ether_type = htons((u_short)type);
	bcopy((caddr_t)ether_dest, (caddr_t)header->ether_dhost,
		sizeof(ether_dest));
	sipl = OS_LOCK(softp);
	bcopy((caddr_t)softp->ss_arp.ac_enaddr,
		(caddr_t)header->ether_shost, 6);

	/*
	 * The SCSI/Ether interface is not very good at handling short
	 * output packets, so try to condense the first few mbufs
	 * together.
	 * Note that we are guaranteed that m->m_off == MMINOFF, as
	 * we just placed the 14-byte Ethernet header there.
	 */

	while (m->m_next && (m->m_len + m->m_next->m_len) <= MLEN) {
		register struct mbuf *mn = m->m_next;

		bcopy(mtod(mn, caddr_t), (caddr_t)(mtod(m, int)+m->m_len), (u_int)mn->m_len);
		m->m_len += mn->m_len;
		ASSERT(m->m_len <= MLEN, "se_output: MLEN");
		m->m_next = mn->m_next;
		mn->m_next = (struct mbuf *)0;
		m_freem(mn);
	}

	/*
	 * Queue message on interface, and start output if interface not active.
	 */

#ifndef	MACH
	(void) IF_LOCK(&ifp->if_snd);
#endif	MACH
	if (IF_QFULL(&ifp->if_snd)) {
#ifdef DEBUG
		if (se_obug)
			printf("dropo\n");
#endif DEBUG
		IF_DROP(&ifp->if_snd);
#ifndef	MACH
		IF_UNLOCK(&ifp->if_snd, SPLIMP);
#endif	MACH
		OS_UNLOCK(softp, sipl);
		m_freem(m);
		return(ENETDOWN);
	}
	IF_ENQUEUE(&ifp->if_snd, m);
#ifndef	MACH
	IF_UNLOCK(&ifp->if_snd, SPLIMP);
#endif	MACH
	OS_UNLOCK(softp, sipl);
	se_start(softp);
	return(0);
}
#endif	MACH_KERNEL


/*
 * se_start - start output on the interface.
 *
 * First we make sure it is idle and that there is work to do.
 *
 * We spray the mbuf into the output iat queue,
 * build the device program and start the program running.
 *
 * EMERGENCY FIX: Since someone is putting tiny mbufs in the
 * middle of the mbuf chain, we must copy mbufs into an output
 * buffer until we understand the problem better.
 */

se_start(softp)
	register struct se_state *softp;
{
	spl_t sipl;

	if (softp-se_state > se_max_unit)
		return;
#ifdef DEBUG
	if (se_obug)
		printf("S%d ", softp-se_state);
#endif DEBUG
	sipl = OS_LOCK(softp);
	if (softp->os_active) {
#ifdef DEBUG
		if (se_obug)
			printf("active ");
#endif DEBUG
		goto ret;
	}

	/*
	 * Device not busy.  Is there something in the queue?
	 */

	for (;;) {
#ifdef	MACH_KERNEL
		struct ifqueue *ifq = &softp->ss_if.if_snd;
		register io_req_t	m;
#else	MACH_KERNEL
		struct ifqueue *ifq = &softp->ss_arp.ac_if.if_snd;
		register struct mbuf *m, *n;
#endif	MACH_KERNEL
		register struct sec_pq *sq = &softp->os_reqq;
		register struct sec_progq *pq = sq->sq_progq;
		register struct sec_dev_prog *dp;
		u_char *cp;
		struct ether_header *header;
		int packetsize, padcount;

		dp = PHYSTOKV(pq->pq_un.pq_progs[pq->pq_head],
				struct sec_dev_prog *);
#ifndef	MACH
		(void) IF_LOCK(ifq);
#endif	MACH
		IF_DEQUEUE(ifq, m);
#ifndef	MACH
		IF_UNLOCK(ifq, SPLIMP);
#endif	MACH
#ifdef	MACH_KERNEL
		if (m == 0)
#else	MACH_KERNEL
		if (m == (struct mbuf *)0)
#endif	MACH_KERNEL
			break;

		/*
		 * m is a nonempty chain of mbufs
		 * corresponding to a packet.
		 * Flush the iat queue to empty, and
		 * place the mbufs there.
		 */

		ASSERT(sq->sq_size != 0, "se_start: size");
		ASSERT(pq->pq_head < sq->sq_size, "se_start: head");
		ASSERT(pq->pq_tail < sq->sq_size, "se_start: tail");
		ASSERT((pq->pq_head + 1) % sq->sq_size != pq->pq_tail,
			"se_start: head+1");
		ASSERT(pq->pq_tail == pq->pq_head, "se_start: head/tail");

#ifdef	MACH_KERNEL
		softp->ss_if.if_opackets++;
#else	MACH_KERNEL
		softp->ss_arp.ac_if.if_opackets++;
#endif	MACH_KERNEL
		softp->os_active = 1;
		softp->os_pending = m;
#ifdef	MACH_KERNEL
		packetsize = m->io_count;
#else	MACH_KERNEL
		packetsize = mbuf_chain_size(m);
#endif	MACH_KERNEL
		padcount = ETHERMIN - (packetsize - sizeof(struct ether_header));
		if (padcount > 0)
			packetsize += padcount;
#ifdef	MACH_KERNEL
		bcopy(m->io_data, softp->os_buf, m->io_count);
		cp = softp->os_buf + m->io_count;
#else	MACH_KERNEL
		for (cp = softp->os_buf, n = m; n != 0;  n = n->m_next) {
			bcopy(mtod(n, caddr_t), (caddr_t)cp, (u_int)n->m_len);
			cp += n->m_len;
		}
#endif	MACH_KERNEL
		ASSERT(cp >= softp->os_buf, "se_start: cp < os_buf");
		ASSERT(cp <= softp->os_buf + OS_BUF_SIZE,
			"se_start: cp > os_buf");
		dp->dp_un.dp_data = KVTOPHYS(softp->os_buf, unsigned char *);
		dp->dp_data_len = packetsize;
		dp->dp_cmd_len = 0;
		dp->dp_next = (struct sec_dev_prog *)0;
#ifdef	DEBUG
		if (se_obug > 1) {
			printf("se_start: starting...");
			dump_bytes((char *) softp->os_buf, packetsize);
		}
#endif	DEBUG

		/*
		 * If the packet is a multicast or broadcast
		 * packet, place an indicator in the dp_cmd[]
		 * so that the firmware knows to turn off the
		 * receiver.  The SCSI/Ether firmware can't look
		 * at the packet itself, as the mbuf might not
		 * be within its 4MB window.
		 */

		dp->dp_cmd[0] = SCSI_ETHER_WRITE;
#ifdef	MACH_KERNEL
		header = (struct ether_header *)m->io_data;
#else	MACH_KERNEL
		header = mtod(m, struct ether_header *);
#endif	MACH_KERNEL

		if ((header->ether_dhost[0] & 0x01)
#ifdef PROMISCUOUS
		   || (softp->ss_ether_flags == SETHER_PROMISCUOUS)
#endif PROMISCUOUS
		)
		{
			dp->dp_cmd[1] = SCSI_ETHER_MULTICAST;
		} else {
			dp->dp_cmd[1] = SCSI_ETHER_STATION;
		}

#ifdef DEBUG
		if (se_obug)
			printf("sio ");
#endif DEBUG
		pq->pq_head = (pq->pq_head + 1) % sq->sq_size;
		if (sec_start_prog(SINST_STARTIO, softp->os_cib,
				   softp->ss_slic, softp->ss_bin,
				   SDEV_ETHERWRITE, 1)
		    != SEC_ERR_NONE) {
			printf("%s%d: se_start: status 0x%x\n",
				se_driver.sed_name, softp-se_state,
				softp->os_status);
		}
		break;
	}
ret:

#ifdef DEBUG
	if (se_obug)
		printf("\n");
#endif DEBUG
	OS_UNLOCK(softp, sipl);
}

#ifndef	MACH
/*
 * se_ioctl
 */

static
se_ioctl(ifp, cmd, data)
	register struct ifnet *ifp;
	int cmd;
	caddr_t data;
{
	register struct ifreq *ifr = (struct ifreq *)data;
	register struct se_state *softp = &se_state[ifp->if_unit];
	spl_t sipl;

	switch (cmd) {
	case SIOCSIFADDR:
		sipl = SS_LOCK(softp);
		if (ifp->if_flags & IFF_RUNNING)
			if_rtinit(ifp, -1);

		se_set_addr(ifp, (struct sockaddr_in *)&ifr->ifr_addr);
		SS_UNLOCK(softp, sipl);
		se_init(ifp->if_unit);
		return(0);

	default:

#ifdef PROMISCUOUS
	if(promiscdev)
		return((*cdevsw[major(promiscdev)].d_ioctl)(ifp, cmd, data));
	else
		return(EINVAL);
#else
		return(EINVAL);
#endif PROMISCUOUS
	}
}

#else
/*
 * MACH/4.3 changed this a bunch.
 */
#ifdef	MACH_KERNEL
se_getstat(dev, flavor, status, count)
	dev_t	dev;
	int	flavor;
	dev_status_t	status;		/* pointer to OUT array */
	unsigned int	count;		/* out */
{
	register int	unit = minor(dev);
	register struct se_state *softp = &se_state[unit];

	return (net_getstat(&softp->ss_if, flavor, status, count));
}

se_setstat(dev, flavor, status, count)
	dev_t	dev;
	int	flavor;
	dev_status_t	status;
	unsigned int	count;
{
	register int	unit = minor(dev);
	register struct se_state *softp = &se_state[unit];

	switch (flavor) {
	    case NET_STATUS:
	    {
		/*
		 * All we can change are flags, and not many of those.
		 */
		register struct net_status *ns = (struct net_status *)status;
		int	mode = 0;

		if (count < NET_STATUS_COUNT)
		    return (D_INVALID_OPERATION);

		/*
		 * XXX This cannot be right-
		 *	the multicast and promiscuous flags
		 *	seem to be mutually exclusive!
		 */
		if (ns->flags & IFF_ALLMULTI)
		    mode |= SETHER_MULTICAST;
		if (ns->flags & IFF_PROMISC)
		    mode |= SETHER_PROMISCUOUS;

		/*
		 * Force a complete reset if the receive mode changes
		 * so that these take effect immediately.
		 */
		if (softp->ss_ether_flags != mode) {
		    softp->ss_ether_flags = mode;
		    se_set_modes(softp);
		}
		break;
	    }
	    case NET_ADDRESS:
	    {
		register union ether_cvt {
		    char	addr[6];
		    int		lwd[2];
		} *ec = (union ether_cvt *)status;

		if (count < sizeof(*ec)/sizeof(int))
		    return (D_INVALID_SIZE);
		ec->lwd[0] = ntohl(ec->lwd[0]);
		ec->lwd[1] = ntohl(ec->lwd[1]);

		bcopy((char *)ec->addr, (char *)softp->ss_addr, 6);
		se_set_modes(softp);
		break;
	    }

	    default:
		return (D_INVALID_OPERATION);
	}
	return (D_SUCCESS);

}
#else	MACH_KERNEL
/*
 * se_ioctl
 */

static
se_ioctl(ifp, cmd, data)
	register struct ifnet *ifp;
	int cmd;
	caddr_t data;
{
	register struct ifaddr *ifa = (struct ifaddr *)data;
	int s = splimp(), error = 0;

	switch (cmd) {

	case SIOCSIFADDR:
		ifp->if_flags |= IFF_UP;
		se_init(ifp->if_unit);

		switch (ifa->ifa_addr.sa_family) {
#ifdef INET
		case AF_INET:
			((struct arpcom *)ifp)->ac_ipaddr =
				IA_SIN(ifa)->sin_addr;
			arpwhohas((struct arpcom *)ifp, &IA_SIN(ifa)->sin_addr);
			break;
#endif
#ifdef NS
		ERROR -- this case not really here yet
		case AF_NS:
		    {
			register struct ns_addr *ina = &(IA_SNS(ifa)->sns_addr);
			
			if (ns_nullhost(*ina))
				ina->x_host = *(union ns_host *)(ds->ds_addr);
			else
				se_setaddr(ina->x_host.c_host,ifp->if_unit);
			break;
		    }

#endif
		default:
			break;
		}
		break;

	default:
		error = EINVAL;
	}
	splx(s);
	return (error);
}
#endif	MACH_KERNEL
#endif	MACH

#ifndef	MACH
static
se_set_addr(ifp, sin)
	register struct ifnet *ifp;
	register struct sockaddr_in *sin;
{
	ifp->if_addr = *(struct sockaddr *)sin;
	ifp->if_net = in_netof(sin->sin_addr);
	ifp->if_host[0] = in_lnaof(sin->sin_addr);
	sin = (struct sockaddr_in *)&ifp->if_broadaddr;
	sin->sin_family = AF_INET;
	sin->sin_addr = if_makeaddr(ifp->if_net, INADDR_ANY);
	ifp->if_flags |= IFF_BROADCAST;
}
#endif	MACH

/*
 * se_watch - watchdog routine, request statistics from board.
 *
 * The cib's status pointer must have an address that is physical == virtual,
 * and must reside within the SEC's 4MB window.
 */

static
se_watch(unit)
	int unit;
{
	register struct se_state *softp = &se_state[unit];
	register struct sec_cib *cib;
#ifdef	MACH_KERNEL
	struct ifnet *ifp = &softp->ss_if;
#else	MACH_KERNEL
	struct ifnet *ifp = &softp->ss_arp.ac_if;
#endif	MACH_KERNEL
	volatile int *saved_status;
	spl_t sipl;

	if (unit < 0 || unit > se_max_unit)
		return;

	sipl = OS_LOCK(softp);
	cib = softp->os_cib;
	saved_status = cib->cib_status;
	cib->cib_status = KVTOPHYS(&softp->os_gmode, int *);

	if (sec_start_prog(SINST_GETMODE, cib, softp->ss_slic,
			   softp->ss_bin, SDEV_ETHERWRITE, 1)
	    != SEC_ERR_NONE) {
		printf("%s%d: se_watch: status 0x%x\n",
			se_driver.sed_name, softp-se_state,
			softp->os_gmode.gm_status);
	}

	cib->cib_status = saved_status;

	(void) SS_LOCK(softp);

#define INCR(field1, field2) \
	softp->ss_sum.field1 += softp->os_gmode.gm_un.gm_ether.field2

	INCR(ec_rx_ovfl, egm_rx_ovfl);
	INCR(ec_rx_crc, egm_rx_crc);
	INCR(ec_rx_dribbles, egm_rx_dribbles);
	INCR(ec_rx_short, egm_rx_short);
	INCR(ec_rx_good, egm_rx_good);

	INCR(ec_tx_unfl, egm_tx_unfl);
	INCR(ec_tx_coll, egm_tx_coll);
	INCR(ec_tx_16xcoll, egm_tx_16x_coll);
	INCR(ec_tx_good, egm_tx_good);
#undef INCR

#ifdef	MACH_KERNEL
	softp->ss_if.if_ierrors +=
#else	MACH_KERNEL
	softp->ss_arp.ac_if.if_ierrors +=
#endif	MACH_KERNEL
		softp->os_gmode.gm_un.gm_ether.egm_rx_ovfl
		+ softp->os_gmode.gm_un.gm_ether.egm_rx_crc
		+ softp->os_gmode.gm_un.gm_ether.egm_rx_dribbles;
#ifdef	MACH_KERNEL
	softp->ss_if.if_oerrors +=
#else	MACH_KERNEL
	softp->ss_arp.ac_if.if_oerrors +=
#endif	MACH_KERNEL
		softp->os_gmode.gm_un.gm_ether.egm_tx_unfl;

#ifdef	MACH_KERNEL
	softp->ss_if.if_collisions = softp->ss_sum.ec_tx_coll;
#else	MACH_KERNEL
	softp->ss_arp.ac_if.if_collisions = softp->ss_sum.ec_tx_coll;
#endif	MACH_KERNEL

#ifdef	MACH_KERNEL
#else	MACH_KERNEL
	ifp->if_timer = softp->ss_scan_int;
#endif	MACH_KERNEL
	SS_UNLOCK(softp, SPLIMP);
	OS_UNLOCK(softp, sipl);

#ifdef	MACH_KERNEL
	timeout(se_watch, (char *)0, softp->ss_scan_int);
#endif	MACH_KERNEL

}



/*
 * reset: not necessary on sequent hardware.
 */

static
se_reset()
{
	panic("se_reset");
}



/*
 * se_set_modes - set the Ethernet modes based upon the soft state.
 *
 * Called with all pieces of the state locked.
 *
 * When we do the SINST_SETMODE, we use the get_mode structure
 * in the output state.  This is fair as everyone else is locked
 * out and the first part of the get_mode structure is a set_mode
 * piece.
 */

se_set_modes(softp)
	register struct se_state *softp;
{
	register volatile struct sec_ether_smodes *esm;
	register struct sec_cib *cib = softp->os_cib;
	volatile int *saved_status = cib->cib_status;

	cib->cib_status = KVTOPHYS(&softp->os_gmode, volatile int *);

	esm = &softp->os_gmode.gm_un.gm_ether.egm_sm;
#ifdef	MACH_KERNEL
	bcopy((caddr_t)softp->ss_addr, (caddr_t)esm->esm_addr, 6);
#else	MACH_KERNEL
	bcopy((caddr_t)softp->ss_arp.ac_enaddr, (caddr_t)esm->esm_addr, 6);
#endif	MACH_KERNEL
	esm->esm_flags = softp->ss_ether_flags;
#ifdef	MACH_KERNEL
	esm->esm_size = sizeof(struct ether_header) + ETHERMTU;
		/* we receive entire packet at once */
#else	MACH_KERNEL
	esm->esm_size = MLEN;
#endif	MACH_KERNEL

	if (sec_start_prog(SINST_SETMODE, cib, softp->ss_slic,
			   softp->ss_bin, SDEV_ETHERWRITE, 1)
	    != SEC_ERR_NONE) {
		printf("%s%d: se_set_mode: status 0x%x\n",
			se_driver.sed_name, softp-se_state,
			softp->os_gmode.gm_status);
	}
	cib->cib_status = saved_status;
}

#ifdef	DEBUG
static	char	hex[] = "0123456789abcdef";

dump_mbuf_chain(m)
	register struct mbuf *m;
{
	register int	mcnt;

	for (mcnt = 0; m != NULL; mcnt++, m = m->m_next) {
		printf("mbuf[%d]:", mcnt);
		dump_bytes(mtod(m, char *), m->m_len);
	}
}

dump_bytes(cp, len)
	register char	*cp;
	register int	len;
{
	register int	cnt;

	for (cnt = 0; cnt < len; cnt++, cp++) {
		if ((cnt % 20) == 0)
			printf("\n\t");
		printf(" %c%c", hex[((int)(*cp) >> 4) & 0xf], hex[(*cp) & 0xf]);
	}
	printf("\n");
}
#endif	DEBUG
