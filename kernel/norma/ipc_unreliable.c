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
 * $Log:	ipc_unreliable.c,v $
 * Revision 2.2  92/03/10  16:28:31  jsb
 * 	Merged in norma branch changes as of NORMA_MK7.
 * 	[92/03/09  12:50:36  jsb]
 * 
 * Revision 2.1.2.3  92/02/21  11:25:08  jsb
 * 	Reduced netipc_ticks to something more reasonable.
 * 	[92/02/20  14:01:35  jsb]
 * 
 * Revision 2.1.2.2  92/02/18  19:16:47  jeffreyh
 * 	[intel] added callhere debugging stuff for iPSC.
 * 	[92/02/13  13:07:35  jeffreyh]
 * 
 * Revision 2.1.2.1  92/01/21  21:53:02  jsb
 * 	Use softclock timeouts instead of call from clock_interrupt.
 * 	[92/01/17  12:20:09  jsb]
 * 
 * 	Added netipc_checksum flag for use under NETIPC_CHECKSUM conditional.
 * 	Never panic on checksum failure. Enter send queue when remembering
 * 	nack when netipc_sending is true. Forget nack when receiving ack.
 * 	Added netipc_receiving flag.
 * 	[92/01/16  22:27:47  jsb]
 * 
 * 	Added conditionalized checksumming code. Eliminated redundant and
 * 	unused pcs_last_unacked field. Added pcs_nacked field to avoid
 * 	throwing away nacks when netipc_sending is true. Removed panics
 * 	on obsolete seqids. Eliminated ancient obsolete comments.
 * 	Added netipc_start_receive call, which takes account of ipc_ether.c
 * 	now modifying vec[i].size to indicate actual received amounts.
 * 	[92/01/14  21:53:20  jsb]
 * 
 * 	Removed netipc_packet definitions and references. Changes for new
 * 	interface with norma/ipc_output.c (look there for explanation).
 * 	[92/01/13  20:21:54  jsb]
 * 
 * 	Changed to use pcs structures and queue macros.
 * 	Added netipc_pcs_print. De-linted.
 * 	[92/01/13  10:19:19  jsb]
 * 
 * 	First checkin. Simple protocol for unreliable networks.
 * 	[92/01/11  17:39:37  jsb]
 * 
 * 	First checkin. Contains functions moved from norma/ipc_net.c.
 * 	[92/01/10  20:47:49  jsb]
 * 
 */
/*
 *	File:	norma/ipc_send.c
 *	Author:	Joseph S. Barrera III
 *	Date:	1990
 *
 *	Functions to support ipc between nodes in a single Mach cluster.
 */

#include <kern/queue.h>
#include <kern/time_out.h>
#include <norma/ipc_net.h>

#define	CTL_NONE		0L
#define	CTL_ACK			1L
#define	CTL_NACK		2L
#define	CTL_SYNC		3L
#define	CTL_QUENCH		4L	/* not used yet */

#define	MAX_NUM_NODES		256	/* XXX */

int			netipc_self_stopped;
struct netipc_hdr	netipc_recv_hdr;

extern int Noise6;
extern int Noise7;

extern void netipc_recv_ack_with_status();

boolean_t		netipc_receiving = FALSE;
struct netvec		netvec_r[2];

vm_page_t		netipc_recv_page;
vm_page_t		netipc_fallback_page = VM_PAGE_NULL;
extern vm_page_t	netipc_page_grab();

int c_netipc_stop	= 0;
int c_netipc_unstop	= 0;
int c_netipc_old_recv	= 0;

#define	MAX_WINDOW_SIZE	1

#define	WX		(MAX_WINDOW_SIZE + 1)

/* XXX must make sure seqids never wrap to 0 */

extern unsigned long	node_incarnation;

/*
 * Protocol control structure
 * (struct pcb is already in use)
 */
typedef struct pcs	*pcs_t;
#define	PCS_NULL	((pcs_t) 0)
struct pcs {
	unsigned long	pcs_remote;
	unsigned long	pcs_last_received;
	unsigned long	pcs_last_sent;
	unsigned long	pcs_nacked;
	unsigned long	pcs_unacked_packetid[WX];
	unsigned long	pcs_incarnation;
	unsigned long	pcs_new_incarnation;
	unsigned long	pcs_ctl;
	unsigned long	pcs_ctl_seqid;
	kern_return_t	pcs_ctl_status;
	unsigned long	pcs_ctl_data;
	int		pcs_ticks;
	queue_chain_t	pcs_timer;
	queue_chain_t	pcs_unack;
	queue_chain_t	pcs_send;
};

queue_head_t netipc_timers;
queue_head_t netipc_unacks;
queue_head_t netipc_sends;

struct pcs netipc_pcs[MAX_NUM_NODES];

/*
 * Counters and such for debugging
 */
int c_netipc_timeout	= 0;
int c_netipc_retry_k	= 0;
int c_netipc_retry_m	= 0;
int c_netipc_retry_p	= 0;
int c_netipc_retry_o	= 0;

struct netipc_hdr	send_hdr_a;
unsigned long		send_data_a = 0xabcd9876;

struct netvec		netvec_a[2];

boolean_t		netipc_sending;

#if	NETIPC_CHECKSUM
int netipc_checksum = 1;
int netipc_checksum_print = 1;

#define	longword_aligned(x) ((((unsigned long)(x)) & (sizeof(long)-1)) == 0)

unsigned long
netipc_compute_checksum(vec, count)
	register struct netvec *vec;
	unsigned int count;
{
	int i;
	register unsigned long checksum = 0;
	register unsigned long *data;
	register int j;
	
	if (! netipc_checksum) {
		return 0;
	}
	for (i = 0; i < count; i++) {
		data = (unsigned long *) DEVTOKV(vec[i].addr);
		assert(longword_aligned((unsigned long) data));
		assert(longword_aligned(vec[i].size));
		for (j = vec[i].size / sizeof(*data) - 1; j >= 0; j--) {
			checksum += data[j];
		}
	}
	return checksum;
}
#endif	NETIPC_CHECKSUM

#if	mips
#include <mips/mips_cpu.h>
/*
 * XXX
 * This works for write-through caches, albeit slowly.
 * Need a much better solution.
 */
vm_offset_t
phystokv(phys)
	vm_offset_t phys;
{
	return PHYS_TO_K1SEG(phys);	/* uncached */
}
#endif

/*
 * Use queue_chain_t to record whether we are already on the queue.
 */
#define queue_untable(head, elt, type, field)\
{\
	queue_remove(head, elt, type, field);\
	(elt)->field.prev = &((elt)->field);\
}
#define queue_tabled(q)		((q)->prev != (q))

netipc_cleanup_send_state(pcs)
	pcs_t pcs;
{
	int i;

	/*
	 * Clean up connection state.
	 */
	pcs->pcs_last_sent = 0;
	for (i = 0; i < WX; i++) {
		pcs->pcs_unacked_packetid[i] = 0;
	}
	if (queue_tabled(&pcs->pcs_timer)) {
		queue_untable(&netipc_timers, pcs, pcs_t, pcs_timer);
	}
	if (queue_tabled(&pcs->pcs_send)) {
		queue_untable(&netipc_sends, pcs, pcs_t, pcs_send);
	}
	if (queue_tabled(&pcs->pcs_unack)) {
		queue_untable(&netipc_unacks, pcs, pcs_t, pcs_unack);
	}

	/*
	 * XXX
	 * This should be called by norma ipc layer
	 */
	netipc_cleanup_incarnation_complete(pcs->pcs_remote);
}

/*
 * A timer for a remote node gets set to 1 when a message is sent
 * to that node. Every so many milliseconds, the timer value is
 * incremented. When it reaches a certain value (currently 2),
 * a sync message is sent to see whether we should retransmit.
 *
 * The timer SHOULD be set to 0 when the message is acknowledged.
 */

int
netipc_unacked_seqid(pcs)
	register pcs_t pcs;
{
	register unsigned long seqid;

	seqid = pcs->pcs_last_sent - (MAX_WINDOW_SIZE - 1);
	if ((long) seqid < 0) {
		seqid = 0;
	}
	for (; seqid <= pcs->pcs_last_sent; seqid++) {
		if (pcs->pcs_unacked_packetid[seqid % WX]) {
			return seqid;
		}
	}
	return 0;
}

extern int netipc_timeout_intr();

struct timer_elt netipc_timer_elt;

int netipc_ticks = 5;

netipc_set_timeout()
{
	netipc_timer_elt.fcn = netipc_timeout_intr;
	netipc_timer_elt.param = 0;
	set_timeout(&netipc_timer_elt, netipc_ticks);
}

_netipc_timeout_intr()
{
	register pcs_t pcs;
	register unsigned long seqid;

#if     iPSC386 || iPSC860
	netipc_called_here(__FILE__, __LINE__, "_netipc_timeout_intr (enter)");
#endif  iPSC386 || iPSC860
	queue_iterate(&netipc_timers, pcs, pcs_t, pcs_timer) {
		if (pcs->pcs_ticks++ == 0) {
#if     iPSC386 || iPSC860
			netipc_called_here(__FILE__, __LINE__, "{pcs->pcs_ticks++ == 0}");
#endif  iPSC386 || iPSC860
			continue;
		}
		assert(pcs->pcs_nacked == 0L);
		seqid = netipc_unacked_seqid(pcs);
		if (seqid == 0) {
			queue_untable(&netipc_timers, pcs, pcs_t, pcs_timer);
#if     iPSC386 || iPSC860
			netipc_called_here(__FILE__, __LINE__, "{seqid == 0}");
#endif  iPSC386 || iPSC860
			continue;
		}

		/*
		 * Something has timed out. Send a sync.
		 * XXX (for just first seqid? How about a bitmask?)
		 * XXX add exponential backoff here, perhaps
		 */
		c_netipc_timeout++;
#if     iPSC386 || iPSC860
		netipc_called_here(__FILE__, __LINE__, "{c_netipc_timeout++}");
#endif  iPSC386 || iPSC860
		printf2("timeout %d\n", pcs->pcs_remote);
		netipc_queue_ctl(pcs, CTL_SYNC, seqid, KERN_SUCCESS, 0L);
	}
	netipc_set_timeout();
#if     iPSC386 || iPSC860
	netipc_called_here(__FILE__, __LINE__, "_netipc_timeout_intr (leave)");
#endif  iPSC386 || iPSC860
}

/*
 * Should not panic, since a bad seqid is the sender's fault.
 */
boolean_t
netipc_obsolete_seqid(pcs, seqid)
	register pcs_t pcs;
	unsigned long seqid;
{
	if (seqid > pcs->pcs_last_sent) {
		printf("premature seqid %d > %d\n", seqid, pcs->pcs_last_sent);
		return TRUE;
	}
	if (seqid <= pcs->pcs_last_sent - MAX_WINDOW_SIZE) {
		printf5("obsolete seqid %d <= %d\n",
			seqid, pcs->pcs_last_sent - MAX_WINDOW_SIZE);
		return TRUE;
	}
	if (pcs->pcs_unacked_packetid[seqid % WX] == 0) {
		printf5("seqid %d already acked\n", seqid);
		return TRUE;
	}
	return FALSE;
}

/*
 * Called with interrupts blocked, from pcs->pcs_recv_intr,
 * when a nack is received.
 * We will be stopped waiting for an ack; resending does not change this.
 */
netipc_recv_nack(pcs, seqid)
	register pcs_t pcs;
	unsigned long seqid;
{
	assert(netipc_locked());

	/*
	 * Ignore obsolete nacks.
	 */
	if (netipc_obsolete_seqid(pcs, seqid)) {
		return;
	}

	/*
	 * Even if we cannot retransmit right away, remember the nack
	 * so that we don't send another sync. I have seen sync-nack
	 * loops under certain conditions when nacks are simply dropped.
	 * We also remove ourselves from the timeout queue so that
	 * the timeout routine doesn't have to check for already-nacked
	 * packets. We have to add ourselves to the send queue because
	 * it is send_intr who is now responsible for sending the nack.
	 */
	if (netipc_sending) {
		pcs->pcs_nacked = seqid;
		queue_untable(&netipc_timers, pcs, pcs_t, pcs_timer);
		if (! queue_tabled(&pcs->pcs_send)) {
			queue_enter(&netipc_sends, pcs, pcs_t, pcs_send);
		}
		return;
	}

	/*
	 * We can retransmit now, so do so.
	 */
	assert(pcs->pcs_unacked_packetid[seqid % WX]);
	netipc_send_old(pcs->pcs_unacked_packetid[seqid % WX], seqid);
}

netipc_recv_ack(pcs, seqid, status, data)
	register pcs_t pcs;
	unsigned long seqid;
	kern_return_t status;
	unsigned long data;
{
	unsigned long packetid;

	assert(netipc_locked());
	/*
	 * Ignore obsolete acks.
	 */
	if (netipc_obsolete_seqid(pcs, seqid)) {
		return;
	}

	/*
	 * Acknowledge the seqid, possibly freeing the kmsg.
	 * Forget any recorded nack.
	 *
	 * XXX
	 * Should we cancel the timer? It's not really necessary.
	 * If we do, we should check to see whether there are
	 * any other unacked packets, and only cancel the timer
	 * if there aren't.
	 */
	packetid = pcs->pcs_unacked_packetid[seqid % WX];
	pcs->pcs_unacked_packetid[seqid % WX] = 0;
	if (pcs->pcs_nacked == seqid) {
		pcs->pcs_nacked = 0L;
	}

	/*
	 * Pass acknowledgement to upper level.
	 */
	netipc_recv_ack_with_status(packetid, seqid, status, data);
}

/*
 * Called by upper level to indicate that it has something to send,
 * and that we should make an upcall when we can perform that send.
 */
void
netipc_start(remote)
	unsigned long remote;
{
	register pcs_t pcs = &netipc_pcs[remote];

#if	iPSC386 || iPSC860
	netipc_called_here(__FILE__, __LINE__, "netipc_start (enter)");
#endif	iPSC386 || iPSC860
	assert(netipc_locked());
	if (! netipc_sending) {
		boolean_t sending;
#if	iPSC386 || iPSC860
		netipc_called_here(__FILE__, __LINE__, "{netipc_sending == FALSE}");
#endif	iPSC386 || iPSC860
		sending = netipc_send_new(remote, pcs->pcs_last_sent + 1);
		assert(sending);
	} else if (! queue_tabled(&pcs->pcs_send)) {
#if	iPSC386 || iPSC860
		netipc_called_here(__FILE__, __LINE__, "{queue_tabled==0}");
#endif	iPSC386 || iPSC860
		queue_enter(&netipc_sends, pcs, pcs_t, pcs_send);
	}
#if	iPSC386 || iPSC860
	else {
		netipc_called_here(__FILE__, __LINE__, "{neither!!}");
	}
	netipc_called_here(__FILE__, __LINE__, "netipc_start (leave)");
#endif	iPSC386 || iPSC860
}

Xnetipc_next_seqid(remote)
	unsigned long remote;
{
	pcs_t pcs = &netipc_pcs[remote];

	return pcs->pcs_last_sent + 1;
}

/*
 * XXX Still need to worry about running out of copy objects
 */

norma_kmsg_put(kmsg)
	ipc_kmsg_t kmsg;
{
	netipc_thread_lock();
	netipc_page_put(kmsg->ikm_page);
	netipc_thread_unlock();
}

netipc_cleanup_receive_state(pcs)
	register pcs_t pcs;
{
	pcs->pcs_last_received = 0;
}

/*
 * Called with interrupts blocked when a page becomes available.
 * Replaces current dummy page with new page, so that
 * any incoming page will be valid.
 * Note that with dma, a receiving may currently be happening.
 *
 * This has the bonus of saving a retransmit if we find a page
 * quickly enough. I don't know how often this will happen.
 *----
 * up to date comment:
 *	shouldn't transfer all at once since we might stop again
 *
 * XXX
 * Why don't we simply pass in a page instead of having this bogus assertion
 * assert(page != NULL) ?
 */
netipc_self_unstop()
{
	register pcs_t pcs;

	assert(netipc_locked());
	c_netipc_unstop++;
	assert(netipc_self_stopped);
	netipc_fallback_page = netipc_page_grab();
	assert(netipc_fallback_page != VM_PAGE_NULL);
	netipc_self_stopped = FALSE;

	queue_iterate(&netipc_unacks, pcs, pcs_t, pcs_unack) {
		netipc_queue_ctl(pcs, CTL_NACK, pcs->pcs_last_received,
				KERN_SUCCESS, 0L);
		queue_untable(&netipc_unacks, pcs, pcs_t, pcs_unack);
	}
}

int netipc_drop_freq = 0;
int netipc_drop_counter = 0;

/*
 * A general comment about acknowledgements and seqids, since
 * I've screwed this up in the past...
 *
 * If seqid > pcs->pcs_last_received + 1:
 *	Something is very wrong, since we are using stop-and-wait.
 *	The sender sent a packet before receiving an ack for the
 *	preceeding packet.
 * 
 * If seqid = pcs->pcs_last_received + 1:
 *	This is a packet we've not processed before.
 *	Pass it up to the ipc code, and ack it (now or later).
 * 
 * If seqid = pcs->pcs_last_received:
 *	We've seen this before and acked it, but the ack
 *	might have been lost. Ack it again.
 * 
 * If seqid < pcs->pcs_last_received:
 *	We don't need to ack this, because we know (from having
 *	received later packets) that the sender has already
 *	received an ack for this packet.
 *
 * Again... this code needs to be modified to deal with window sizes > 1.
 */
_netipc_recv_intr()
{
	register struct netipc_hdr *hdr = &netipc_recv_hdr;
	register pcs_t pcs;
	register unsigned long seqid = hdr->seqid;
	register unsigned long type = hdr->type;
	register unsigned long incarnation = hdr->incarnation;
	register unsigned long ctl = hdr->ctl;
	register unsigned long ctl_seqid = hdr->ctl_seqid;
#if	NETIPC_CHECKSUM
	unsigned long checksum;
#endif	NETIPC_CHECKSUM

#if     iPSC386 || iPSC860
	netipc_called_here(__FILE__, __LINE__, "_netipc_recv_intr");
#endif  iPSC386 || iPSC860
	assert(netipc_locked());
	assert(netipc_receiving);
	netipc_receiving = FALSE;

	/*
	 * Netipc_drop_freq > 0 enables debugging code.
	 */
	if (netipc_drop_freq) {
		if (++netipc_drop_counter >= netipc_drop_freq) {
			/*
			 * Reset counter, drop packet, and rearm.
			 */
			netipc_drop_counter = 0;
			netipc_start_receive();
			return;
		}
	}

#if	NETIPC_CHECKSUM
	checksum = hdr->checksum;
	hdr->checksum = 0;
	hdr->checksum = netipc_compute_checksum(netvec_r, 2);
	if (hdr->checksum != checksum) {
		if (netipc_checksum_print) {
			extern int Noise0;
			int noise0 = Noise0;
			Noise0 = 1;
			netipc_print('r', type, hdr->remote, ctl, seqid,
				     ctl_seqid, hdr->ctl_status,
				     hdr->ctl_data, incarnation);
			Noise0 = noise0;
			printf("_netipc_recv_intr: checksum 0x%x != 0x%x!\n",
			       hdr->checksum, checksum);
		}
		netipc_start_receive();
		return;
	}
#endif	NETIPC_CHECKSUM

	/*
	 * XXX
	 * This isn't safe, but we'll fix it when we install a dynamic
	 * pcs table instead of a fixed size array.
	 */
	pcs = &netipc_pcs[hdr->remote];

	/*
	 * Print packet if so desired.
	 */
	netipc_print('r', type, hdr->remote, ctl, seqid, ctl_seqid,
		     hdr->ctl_status, hdr->ctl_data, incarnation);

	/*
	 * Check incarnation of sender with what we thought it was.
	 * This lets us detect old packets as well as newly rebooted senders.
	 * The incarnation value always increases with each reboot.
	 */
	if (hdr->incarnation != pcs->pcs_incarnation) {
		if (hdr->incarnation < pcs->pcs_incarnation) {
			/*
			 * This is an old packet from a previous incarnation.
			 * We should ignore it.
			 */
			netipc_start_receive();
			return;
		} else if (pcs->pcs_incarnation == 0L) {
			/*
			 * This is the first packet we have ever received
			 * from this node. If it looks like a first packet
			 * (an implicit connection open), then remember
			 * incarnation number; otherwise, tell sender our
			 * new incarnation number, which should force him
			 * to do a netipc_cleanup_incarnation.
			 *
			 * Valid first packets are:
			 *	non-control messages with seqid = 1
			 *	control messages with ctl_seqid = 1
			 * Any others???
			 */
			assert(pcs->pcs_last_received == 0L);
			if ((type == NETIPC_TYPE_KMSG ||
			     type == NETIPC_TYPE_PAGE) && seqid == 1L ||
			    type == NETIPC_TYPE_CTL && ctl_seqid == 1L) {
				/*
				 * A valid first packet.
				 */
				pcs->pcs_incarnation = incarnation;
			} else {
				/*
				 * Probably left over from a previous
				 * incarnation. Use a dummy ctl to
				 * tell sender our new incarnation.
				 */
				netipc_queue_ctl(pcs, CTL_NONE, seqid,
						KERN_SUCCESS, 0L);
				netipc_start_receive();
				return;
			}
		} else {
			/*
			 * This is a packet from a new incarnation.
			 * We don't change incarnation number or process
			 * any packets until we have finished cleaning up
			 * anything that depended on previous incarnation.
			 */
			assert(incarnation > pcs->pcs_incarnation);
			if (pcs->pcs_new_incarnation == 0L) {
				pcs->pcs_new_incarnation = incarnation;
				netipc_cleanup_incarnation(pcs);
			}
			netipc_start_receive();
			return;
		}
	}
	assert(incarnation == pcs->pcs_incarnation);

	/*
	 * Check type of packet.
	 * Discard or acknowledge old packets.
	 */
	if (type == NETIPC_TYPE_KMSG || type == NETIPC_TYPE_PAGE) {
		/*
		 * If this is an old packet, then discard or acknowledge.
		 */
		if (seqid <= pcs->pcs_last_received) {
			/*
			 * We have seen this packet before...
			 * but we might still need to reack it.
			 */
			c_netipc_old_recv++;
			if (seqid == pcs->pcs_last_received) {
				/*
				 * The sender may not yet have received an ack.
				 * Send the ack immediately.
				 * Should we use ackdelay logic here?
				 */
				netipc_queue_ctl(pcs, CTL_ACK, seqid,
						KERN_SUCCESS, 0L);
			}
			/*
			 * Rearm with same buffer, and return.
			 */
			netipc_start_receive();
			return;
		}
	} else if (type != NETIPC_TYPE_CTL) {
		printf("netipc_recv_intr: bad type 0x%x\n", type);
		netipc_start_receive();
		return;
	}

	/*
	 * Process incoming ctl, if any.
	 */
	if (ctl == CTL_NONE) {
		/* nothing to do */
	} else if (ctl == CTL_SYNC) {
		printf2("synch %d\n", ctl_seqid);
		if (ctl_seqid < pcs->pcs_last_received) {
			/* already acked, since sender sent newer packet. */
		} else if (ctl_seqid > pcs->pcs_last_received) {
			/* not yet seen; nack it. */
			/* use pending unacks? */
			assert(ctl_seqid == pcs->pcs_last_received + 1);
			if (ctl_seqid != pcs->pcs_last_received + 1) {
				printf("X%d: %d != %d\n",
				       ctl_seqid, pcs->pcs_last_received + 1);
			}
			netipc_queue_ctl(pcs, CTL_NACK, ctl_seqid,
					KERN_SUCCESS, 0L);
		} else {
			/* may not be acked; ack it. */
			/* use ackdelay? */
			assert(ctl_seqid == pcs->pcs_last_received);
			netipc_queue_ctl(pcs, CTL_ACK, ctl_seqid,
					KERN_SUCCESS, 0L);
		}
	} else if (ctl == CTL_ACK) {
		netipc_recv_ack(pcs, ctl_seqid, hdr->ctl_status,
				hdr->ctl_data);
	} else if (ctl == CTL_NACK) {
		netipc_recv_nack(pcs, ctl_seqid);
	} else {
		printf("%d: ctl?%d %d\n", node_self(), ctl, hdr->remote);
	}

	/*
	 * If this was just a ctl, then restart receive in same buffer.
	 */
	if (type == NETIPC_TYPE_CTL) {
		netipc_start_receive();
		return;
	}

	/*
	 * If we are stopped, set up an nack (since this was more than
	 * just a ctl) and restart receive in same buffer.
	 */
	if (netipc_self_stopped) {
		if (! queue_tabled(&pcs->pcs_unack)) {
			queue_enter(&netipc_unacks, pcs, pcs_t, pcs_unack);
		}
		netipc_start_receive();
		return;
	}

	/*
	 * At this point:
	 *	This is a previously unseen packet
	 *	We have room to keep it
	 *	It is either a kmsg or a page
	 *
	 * Deliver message according to its type.
	 */
	assert(pcs->pcs_last_received == seqid - 1);
	assert(! netipc_self_stopped);
	assert(type == NETIPC_TYPE_KMSG || type == NETIPC_TYPE_PAGE);
	if (type == NETIPC_TYPE_KMSG) {
		register ipc_kmsg_t kmsg;

		/*
		 * This is a kmsg. Kmsgs are word aligned,
		 * and contain their own length.
		 */
		kmsg = (ipc_kmsg_t) VPTOKV(netipc_recv_page);
		kmsg->ikm_size = IKM_SIZE_NORMA;
		kmsg->ikm_marequest = IMAR_NULL;
		kmsg->ikm_page = netipc_recv_page;
		printf6("deliver kmsg: remote=%d msgh_id=%d dest=%x\n",
			hdr->remote,
			kmsg->ikm_header.msgh_id,
			kmsg->ikm_header.msgh_remote_port);
		norma_deliver_kmsg(kmsg, hdr->remote);
	} else {
		/*
		 * This is out-of-line data, or out-of-line ports,
		 * or more of a bigger-than-page-size kmsg.
		 */
		printf6("deliver_page: remote=%d\n", hdr->remote);
		norma_deliver_page(netipc_recv_page,
				   hdr->pg.pg_msgh_offset,
				   hdr->remote,
				   hdr->pg.pg_page_first,
				   hdr->pg.pg_page_last,
				   hdr->pg.pg_copy_last,
				   hdr->pg.pg_copy_size,
				   hdr->pg.pg_copy_offset);
	}
}

norma_ipc_ack(status, data)
	kern_return_t status;
	unsigned long data;
{
	register int remote = netipc_recv_hdr.remote;
	register unsigned long seqid = netipc_recv_hdr.seqid;
	register pcs_t pcs = &netipc_pcs[remote];

	assert(netipc_locked());
	/*
	 * Send acknowledgements.
	 * Eventually, we should wait and try to piggyback these ctls.
	 * This could lead to deadlock if we aren't tricky.
	 * We should for example send acks as soon as we are idle.
	 */
	pcs->pcs_last_received = seqid;
	netipc_queue_ctl(pcs, CTL_ACK, seqid, status, data);

	/*
	 * Start a new receive.
	 * If status is success, the buffer is being kept,
	 * so allocate a new one.
	 * XXX should we have done this before the deliver calls above?
	 */
	if (status == KERN_SUCCESS) {
		netipc_recv_hdr.type = NETIPC_TYPE_INVALID;
		netipc_recv_page = netipc_page_grab();
		if (netipc_recv_page == VM_PAGE_NULL) {
			c_netipc_stop++;
			netipc_self_stopped = TRUE;
			netipc_recv_page = netipc_fallback_page;
		}
		netvec_r[1].addr = VPTODEV(netipc_recv_page);
	}
	netipc_start_receive();
}

/*
 * Drop the packet; pretend that we never saw it.
 * Start a new receive.
 *
 * In a reliable interconnect module, we would register a nack here.
 */
norma_ipc_drop()
{
	netipc_start_receive();
}

/*
 * Called with interrupts blocked, from netipc_self_unstop and
 * netipc_recv_intr.  Ctl may be either CTL_ACK or CTL_NACK.  If we are
 * currently sending, use netipc_pending_ctl to have netipc_send_intr do
 * the send when the current send completes.
 *
 * The netipc_pending_ctl mechanism should be generalized to allow for
 * arbitrary pending send operations, so that no one needs to spin on
 * netipc_sending.
 */
netipc_queue_ctl(pcs, ctl, ctl_seqid, ctl_status, ctl_data)
	register pcs_t pcs;
	unsigned long ctl;
	unsigned long ctl_seqid;
	kern_return_t ctl_status;
	unsigned long ctl_data;
{
	assert(netipc_locked());

	/*
	 * If net is busy sending, let netipc_send_intr send the ctl.
	 */
	if (netipc_sending) {
		/*
		 * We may blow away a preceeding ctl, which is unfortunate
		 * but not fatal.
		 */
		pcs->pcs_ctl = ctl;
		pcs->pcs_ctl_seqid = ctl_seqid;
		pcs->pcs_ctl_status = ctl_status;
		pcs->pcs_ctl_data = ctl_data;
		if (! queue_tabled(&pcs->pcs_send)) {
			queue_enter(&netipc_sends, pcs, pcs_t, pcs_send);
		}
		return;
	}

	/*
	 * Fill in send_hdr_a.
	 */
	send_hdr_a.ctl = ctl;
	send_hdr_a.ctl_seqid = ctl_seqid;
	send_hdr_a.ctl_status = ctl_status;
	send_hdr_a.ctl_data = ctl_data;

	/*
	 * Start the send.
	 */
	netipc_send_ctl(pcs->pcs_remote);
}

/*
 * Net send interrupt routine: called when a send completes.
 * If there is something else to send (currently, only ctls),
 * send it; otherwise, set netipc_sending FALSE.
 */
_netipc_send_intr()
{
	register pcs_t pcs;

	assert(netipc_locked());
	assert(netipc_sending);
	netipc_sending = FALSE;
#if     iPSC386 || iPSC860
	netipc_called_here(__FILE__, __LINE__, "_netipc_send_intr");
#endif  iPSC386 || iPSC860

	/*
	 * Scan the pending list, looking for something to send.
	 * If something is on the list but doesn't belong, remove it.
	 */
	queue_iterate(&netipc_sends, pcs, pcs_t, pcs_send) {
		if (pcs->pcs_ctl != CTL_NONE) {
			/*
			 * There is a ctl to send; send it.
			 */
			send_hdr_a.ctl = pcs->pcs_ctl;
			send_hdr_a.ctl_seqid = pcs->pcs_ctl_seqid;
			send_hdr_a.ctl_status = pcs->pcs_ctl_status;
			send_hdr_a.ctl_data = pcs->pcs_ctl_data;
			pcs->pcs_ctl = CTL_NONE;
			netipc_send_ctl(pcs->pcs_remote);
			return;
		}

		/*
		 * There may be something to retransmit.
		 */
		if (pcs->pcs_nacked != 0L) {
			register unsigned long seqid = pcs->pcs_nacked;
			pcs->pcs_nacked = 0L;
			assert(pcs->pcs_unacked_packetid[seqid % WX]);
			netipc_send_old(pcs->pcs_unacked_packetid[seqid % WX],
					seqid);
		}

		/*
		 * There may be something new to send.
		 */
		if (netipc_send_new(pcs->pcs_remote, pcs->pcs_last_sent + 1)) {
			return;
		}

		/*
		 * Nothing to send -- remove from queue.
		 */
		queue_untable(&netipc_sends, pcs, pcs_t, pcs_send);
	}
}

netipc_protocol_init()
{
	register unsigned long remote;
	register pcs_t pcs;

	/*
	 * Initialize pcs structs, queues, and enable timer.
	 */
	for (remote = 0; remote < MAX_NUM_NODES; remote++) {
		pcs = &netipc_pcs[remote];
		pcs->pcs_remote = remote;
		queue_init(&pcs->pcs_timer);
		queue_init(&pcs->pcs_unack);
		queue_init(&pcs->pcs_send);
	}
	queue_init(&netipc_timers);
	queue_init(&netipc_unacks);
	queue_init(&netipc_sends);
	netipc_set_timeout();

	/*
	 * Initialize netipc_recv_hdr and netvec_r
	 */
	netipc_recv_hdr.type = NETIPC_TYPE_INVALID;
	netvec_r[0].addr = KVTODEV(&netipc_recv_hdr);
	netvec_r[0].size = sizeof(struct netipc_hdr);
	netvec_r[1].size = PAGE_SIZE;

	/*
	 * Initialize send_hdr_a and netvec_a
	 */
	send_hdr_a.type = NETIPC_TYPE_CTL;
	send_hdr_a.remote = node_self();
	netvec_a[0].addr = KVTODEV(&send_hdr_a);
	netvec_a[0].size = sizeof(struct netipc_hdr);
	netvec_a[1].addr = KVTODEV(&send_data_a);
	netvec_a[1].size = sizeof(unsigned long);

	/*
	 * Start receiving.
	 */
	netipc_recv_page = netipc_fallback_page = vm_page_grab();
	assert(netipc_recv_page != VM_PAGE_NULL);
	netipc_self_stopped = TRUE;
	netvec_r[1].addr = VPTODEV(netipc_recv_page);
	netipc_start_receive();
}

netipc_start_receive()
{
	assert(! netipc_receiving);
	netipc_receiving = TRUE;
	netvec_r[0].size = sizeof(struct netipc_hdr);
	netvec_r[1].size = PAGE_SIZE;
	netipc_recv(netvec_r, 2);
}

netipc_init()
{
	netipc_network_init();
	netipc_output_init();
	netipc_protocol_init();
}

netipc_cleanup_incarnation(pcs)
	register pcs_t pcs;
{
	printf1("netipc_cleanup_incarnation(%d)\n", pcs->pcs_remote);

	/*
	 * Clean up connection state and higher-level ipc state.
	 */
	netipc_cleanup_send_state(pcs);
	netipc_cleanup_receive_state(pcs);
	netipc_cleanup_ipc_state(pcs->pcs_remote);
}

#if 666
/*
 * This be in the norma ipc layer and should do something real
 */
netipc_cleanup_ipc_state(remote)
	unsigned long remote;
{
	/* XXX */
	netipc_cleanup_incarnation_complete(remote);
}
#endif

netipc_cleanup_incarnation_complete(remote)
	unsigned long remote;
{
	register pcs_t pcs = &netipc_pcs[remote];

	pcs->pcs_incarnation = pcs->pcs_new_incarnation;
	pcs->pcs_new_incarnation = 0;
}

netipc_send_ctl(remote)
	unsigned long remote;
{
	register struct netipc_hdr *hdr = &send_hdr_a;

	assert(netipc_locked());
	assert(! netipc_sending);
	netipc_sending = TRUE;

	hdr->incarnation = node_incarnation;
	if (remote == node_self()) {
		panic("netipc_send_ctl: sending to node_self!\n");
	}
	netipc_print('s', NETIPC_TYPE_CTL, remote, hdr->ctl, 0L,
		     hdr->ctl_seqid, hdr->ctl_status, hdr->ctl_data,
		     node_incarnation);
#if	NETIPC_CHECKSUM
	hdr->checksum = 0;
	hdr->checksum = netipc_compute_checksum(netvec_a, 2);
#endif	NETIPC_CHECKSUM
	netipc_send(remote, netvec_a, 2);
}

netipc_send_with_timeout(remote, vec, count, packetid, seqid)
	unsigned long remote;
	register struct netvec *vec;
	unsigned int count;
	unsigned long packetid;
	unsigned long seqid;
{
	struct netipc_hdr *hdr = (struct netipc_hdr *) DEVTOKV(vec[0].addr);
/*	register unsigned long seqid = hdr->seqid;*/
	register unsigned long type = hdr->type;
	register unsigned long ctl;
	register unsigned long ctl_seqid = hdr->ctl_seqid;
	register kern_return_t ctl_status = hdr->ctl_status;
	register unsigned long ctl_data = hdr->ctl_data;
	register pcs_t pcs = &netipc_pcs[remote];

#if	iPSC386 || iPSC860
	netipc_called_here(__FILE__, __LINE__, "netipc_send_with_timeout");
#endif	iPSC386 || iPSC860
	assert(netipc_locked());
	assert(! netipc_sending);
	assert(seqid = hdr->seqid);
	netipc_sending = TRUE;

	ctl = hdr->ctl = CTL_NONE;

	hdr->incarnation = node_incarnation;
	while (pcs->pcs_new_incarnation != 0L) {
		/*
		 * Shouldn't get this far!
		 */
		panic("netipc_send_with_timeout: incarnation clean!!!\n");
	}
	if (pcs->pcs_remote == node_self()) {
		panic("netipc_send_with_timeout: sending to node_self!\n");
	}
	netipc_print('s', type, pcs->pcs_remote, ctl, seqid, ctl_seqid,
		     ctl_status, ctl_data, node_incarnation);
	if (pcs->pcs_unacked_packetid[seqid % WX] == 0) {
		pcs->pcs_unacked_packetid[seqid % WX] = packetid;
	}
	if (seqid > pcs->pcs_last_sent) {
		pcs->pcs_last_sent = seqid;
	}
	if (! queue_tabled(&pcs->pcs_timer)) {
		pcs->pcs_ticks = 0;
		queue_enter(&netipc_timers, pcs, pcs_t, pcs_timer);
	}
#if	NETIPC_CHECKSUM
	hdr->checksum = 0;
	hdr->checksum = netipc_compute_checksum(vec, count);
#endif	NETIPC_CHECKSUM
	netipc_send(pcs->pcs_remote, vec, count);
}

netipc_print(c, type, remote, ctl, seqid, ctl_seqid, ctl_status, ctl_data,
	     incarnation)
	char c;
	unsigned long type;
	unsigned long remote;
	unsigned long ctl;
	unsigned long seqid;
	unsigned long ctl_seqid;
	kern_return_t ctl_status;
	unsigned long ctl_data;
	unsigned long incarnation;
{
	char *ts;
	char *cs;
	extern int Noise0;

	assert(netipc_locked());
	if (Noise0 == 2) {
		printf("%c", c);
		return;
	}
	if (Noise0 == 0) {
		return;
	}
	if (type == NETIPC_TYPE_KMSG) {
		ts = "kmsg";
	} else if (type == NETIPC_TYPE_PAGE) {
		ts = "page";
	} else if (type == NETIPC_TYPE_CTL) {
		ts = "ctrl";
		seqid = 0;
	} else {
		ts = "????";
	}
	if (ctl == CTL_NONE) {
		cs = "none";
		ctl_seqid = 0;
	} else if (ctl == CTL_ACK) {
		cs = "ack ";
	} else if (ctl == CTL_NACK) {
		cs = "nack";
	} else if (ctl == CTL_SYNC) {
		cs = "sync";
	} else if (ctl == CTL_QUENCH) {
		cs = "qnch";
	} else {
		cs = "????";
	}
	printf("%c remote=%d type=%s.%04d ctl=%s.%04d kr=%2d data=%2d %10d\n",
	       c, remote, ts, seqid, cs, ctl_seqid, ctl_status, ctl_data,
	       incarnation);
}

#include <mach_kdb.h>
#if	MACH_KDB

#define	printf	kdbprintf

/*
 *	Routine:	netipc_pcs_print
 *	Purpose:
 *		Pretty-print a netipc protocol control structure for ddb.
 */

netipc_pcs_print(pcs)
	pcs_t pcs;
{
	extern int indent;
	int i;

	if ((unsigned int) pcs < MAX_NUM_NODES) {
		pcs = &netipc_pcs[(unsigned int) pcs];
	}

	printf("netipc pcs 0x%x\n", pcs);

	indent += 2;

	iprintf("remote=%d", pcs->pcs_remote);
	printf(", last_received=%d", pcs->pcs_last_received);
	printf(", last_sent=%d", pcs->pcs_last_sent);
	printf(", nacked=%d\n", pcs->pcs_nacked);

	iprintf("sent_packet[0..%d]={", WX - 1);
	for (i = 0; i < WX - 1; i++) {
		printf("0x%x, ", pcs->pcs_unacked_packetid[i]);
	}
	printf("0x%x}\n", pcs->pcs_unacked_packetid[WX - 1]);

	iprintf("incarnation=%d", pcs->pcs_incarnation);
	printf(", new_incarnation=%d\n", pcs->pcs_new_incarnation);

	iprintf("ctl=%d", pcs->pcs_ctl);
	switch ((int) pcs->pcs_ctl) {
		case CTL_NONE:
		printf("[none]");
		break;

		case CTL_ACK:
		printf("[ack]");
		break;

		case CTL_NACK:
		printf("[nack]");
		break;

		case CTL_SYNC:
		printf("[sync]");
		break;

		case CTL_QUENCH:
		printf("[quench]");
		break;

		default:
		printf("[bad type]");
		break;
	}
	printf(", ctl_seqid=%d", pcs->pcs_ctl_seqid);
	printf(", ctl_status=%ld", pcs->pcs_ctl_status);
	printf(", ctl_data=%ld\n", pcs->pcs_ctl_data);

	iprintf("ticks=%d", pcs->pcs_ticks);
	printf(", timer=[%x,%x t=%d]",
	       pcs->pcs_timer.prev,
	       pcs->pcs_timer.next,
	       queue_tabled(&pcs->pcs_timer));
	printf(", unack=[%x,%x t=%d]",
	       pcs->pcs_unack.prev,
	       pcs->pcs_unack.next,
	       queue_tabled(&pcs->pcs_unack));
	printf(", send=[%x,%x t=%d]",
	       pcs->pcs_send.prev,
	       pcs->pcs_send.next,
	       queue_tabled(&pcs->pcs_send));
	printf("\n");

	indent -=2;
}
#endif	MACH_KDB
