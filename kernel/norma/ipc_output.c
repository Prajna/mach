/* 
 * Mach Operating System
 * Copyright (c) 1990,1991,1992 Carnegie Mellon University
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
 * $Log:	ipc_output.c,v $
 * Revision 2.6  92/03/10  16:28:06  jsb
 * 	Merged in norma branch changes as of NORMA_MK7.
 * 	[92/03/09  12:50:12  jsb]
 * 
 * Revision 2.5.2.4  92/02/21  14:31:59  jsb
 * 	Removed code incorrectly duplicated by bmerge.
 * 
 * Revision 2.5.2.3  92/02/21  11:24:50  jsb
 * 	Moved ipc_kmsg_copyout_to_network here from ipc/ipc_kmsg.c.
 * 	Renamed norma_ipc_destroy_proxy to norma_ipc_dead_destination.
 * 	[92/02/21  10:36:14  jsb]
 * 
 * 	In norma_ipc_send, convert kmsg to network format.
 * 	[92/02/21  09:07:00  jsb]
 * 
 * 	Changed for norma_ipc_send_port now returning uid.
 * 	[92/02/20  17:15:23  jsb]
 * 
 * 	Added netipc_thread_wakeup to netipc_safe_vm_map_copy_invoke_cont.
 * 	Added logic to convert node number to netipc packet address in
 * 	netipc_packet_print.
 * 	[92/02/18  17:37:14  jsb]
 * 
 * 	Perform norma_ipc_send_*_dest only after successful acknowledgement.
 * 	This allows simplification of reference counting for destination
 * 	ports. The old scheme kept a single reference for the port while it
 * 	was queued. The new scheme keeps a reference for the port for every
 * 	kmsg it has queued, which is released in norma_ipc_send_*_dest.
 * 	The only explicit reference counting management required by the
 * 	new scheme is the acquisition of a port reference for a proxy before
 * 	calling norma_ipc_destroy_proxy, which expects the caller to supply
 * 	a reference. Eliminated netipc_port_release and netipc_free_port_list,
 * 	since norma_ipc_send_*_dest now handle releasing references after
 * 	message delivery. Changed safe kmsg freeing code to call
 * 	norma_ipc_send_*_dest (which must not be called at interrupt level).
 * 	Also changed usage of ip_norma_queue_next field to allow elimination
 * 	of ip_norma_queued field.
 * 	[92/02/18  09:14:14  jsb]
 * 
 * Revision 2.5.2.2  92/02/18  19:15:50  jeffreyh
 * 	iPSC changes from Intel.
 * 	[92/02/18            jeffreyh]
 * 	[intel] added debugging callhere stuff, routine for
 * 	netipc_vm_map_copy_cont_check(), all for iPSC.
 * 	[92/02/13  13:10:00  jeffreyh]
 * 
 * Revision 2.5.2.1  92/01/21  21:52:17  jsb
 * 	More de-linting.
 * 	[92/01/17  11:40:20  jsb]
 * 
 * 	Added definition of, and call to, netipc_safe_ikm_free.
 * 	[92/01/16  22:13:17  jsb]
 * 
 * 	Minor de-linting.
 * 	[92/01/14  22:01:43  jsb]
 * 
 * 	Reworked interface with underlying protocol module; see comment
 * 	that begins 'This is now how we cooperate...' below.
 * 	[92/01/14  09:29:51  jsb]
 * 
 * 	De-linted. Added netipc_packet_print.
 * 	[92/01/13  10:16:50  jsb]
 * 
 * 	Moved netipc_packet definitions, processing, allocation, to here.
 * 	Moved netipc_ack status demultiplexing here.
 * 	[92/01/11  17:35:35  jsb]
 * 
 * 	Moved old contents to norma/ipc_wire.c.
 * 	Now contains functions split from norma/ipc_net.c.
 * 	[92/01/10  20:40:51  jsb]
 * 
 */
/*
 *	File:	norma/ipc_output.c
 *	Author:	Joseph S. Barrera III
 *	Date:	1990
 *
 *	Functions to support ipc between nodes in a single Mach cluster.
 */

#include <norma/ipc_net.h>

/*
 * This is now how we cooperate with the reliable transmission module
 * underneath us:
 *	1. When we have something to send, we call netipc_start.
 *	2. When he can send it, he upcalls netipc_send_new.
 *	3. We then call netipc_send_{kmsg,page,...} as appropriate.
 *	   From these routines, we call netipc_send_with_timeout,
 *	   specifying a packetid and a seqid.
 *	4. If he decides a packet needs to be retransmitted, he calls
 *	   netipc_send_old with the same packetid and seqid.
 *	5. He receives acknowledgements, eliminates redundant ones,
 *	   and calls netipc_recv_ack_with_status with same packetid, seqid.
 *	6. From ack_with_status we multiplex on status and call one
 *	   of netipc_recv_{success,retarget,dead,not_found}.
 *	7. Netipc_recv_success calls netipc_start if there is more to send.
 *
 * The rationale here is
 *	1. Keep retransmission knowledge in protocol module
 *	2. Keep kmsg, copy object, etc. knowledge here.
 *	3. Do windowing based on limitations imposed by both
 *	   layers. E.g. continuations limit how much of a copy
 *	   object we can send at a time.
 *	4. Allow for smarter protocol modules in the future while
 *	   retaining the no-copy aspect provided by kmsg-walking
 *	   code in this module.
 *	5. Allow for simpler protocol modules for reliable interconnects.
 *
 * Things left to do:
 *	1. Actually add windowing
 *	2. Have this module do its own seqid allocation, separate from
 *	   protocol seqids. This eliminates need for Xnetipc_next_seqid.
 *	3. Eliminate array-based lookup of netipc_packets
 */

#define	NORMA_REALIGN_OOL_DATA	0

#define	DP_TYPE_KMSG		0L
#define	DP_TYPE_PAGE		1L
#define	DP_TYPE_KMSG_MORE	2L
#define	DP_TYPE_OOL_PORTS	3L

typedef struct netipc_packet	*netipc_packet_t;
#define	NETIPC_PACKET_NULL	((netipc_packet_t) 0)

struct netipc_packet {
	unsigned long	dp_type;
	unsigned long	dp_remote;
	unsigned long	dp_seqid;
	unsigned long	dp_first_seqid;
	unsigned long	dp_last_seqid;
	unsigned long	dp_last_unacked;
	ipc_kmsg_t	dp_kmsg;
	unsigned long	dp_offset;
	netipc_packet_t	dp_next;
	ipc_port_t	dp_remote_port;
	vm_map_copy_t	dp_copy;
	unsigned long	dp_copy_index;
	unsigned long	dp_copy_npages;
	unsigned long	dp_copy_last;
	unsigned long	dp_page_list_base;
	boolean_t	dp_has_continuation;
	boolean_t	dp_being_continued;
};

#define	MAX_NUM_NODES		256	/* XXX */
netipc_packet_t netipc_packet[MAX_NUM_NODES];

extern void netipc_start();
extern void norma_ipc_send_dest();
extern void norma_ipc_send_migrating_dest();
extern unsigned long norma_ipc_send_port();

netipc_packet_t netipc_packet_allocate();
void netipc_packet_deallocate();

void norma_ipc_queue_port();
#define	norma_ipc_queued(port)		((port)->ip_norma_queue_next != (port))
#define	norma_ipc_unqueue(port)		((port)->ip_norma_queue_next = (port))

/*
 * This should be conditionalized on machine and type of interconnect.
 * For now, we assume that everyone will be happy with 32 bit alignment.
 */
#define	WORD_SIZE	4
#define	WORD_MASK	(WORD_SIZE - 1)

#define ROUND_WORD(x)	((((unsigned long)(x)) + WORD_MASK) & ~WORD_MASK)
#define TRUNC_WORD(x)	(((unsigned long)(x)) & ~WORD_MASK)
#define	WORD_ALIGNED(x)	((((unsigned long)(x)) & WORD_MASK) == 0)

zone_t			netipc_packet_zone;

ipc_kmsg_t		netipc_kmsg_cache;
vm_size_t		netipc_kmsg_first_half;
int			netipc_kmsg_cache_hits;		/* debugging */
int			netipc_kmsg_cache_misses;	/* debugging */
int			netipc_kmsg_splits;		/* debugging */

struct netipc_hdr		send_hdr_p;
struct netipc_hdr		send_hdr_k;
struct netipc_hdr		send_hdr_m;
struct netipc_hdr		send_hdr_o;

struct netvec	netvec_p[3];
struct netvec	netvec_k[3];
struct netvec	netvec_m[3];
struct netvec	netvec_o[3];

struct vm_map_copy	netipc_kmsg_more_copy;
struct vm_map_copy	netipc_ool_ports_copy;

#if	iPSC386 || iPSC860
/* XXX debugging */
int			verbose_already_queued = 0;
int			norma_ipc_queue_port_on_list;
int			netipc_packet_list_empty;
netipc_packet_t		netipc_last_interesting_dp;
vm_map_copy_t		netipc_last_interesting_copy;
kern_return_t           (*netipc_last_dp_copy_continuation)();
#endif	iPSC386	|| iPSC860

/*
 * vm_map_copy_discard_cont is not an interesting continuation, that is,
 * it does not affect the way a copy object is sent, because it will
 * not result in any new page lists.
 */
extern kern_return_t vm_map_copy_discard_cont();
#define	vm_map_copy_has_interesting_cont(copy) \
  (vm_map_copy_has_cont(copy) && (copy)->cpy_cont != vm_map_copy_discard_cont)

netipc_output_init()
{
	/*
	 * Initialize send_hdr_k and netvec_k
	 */
	send_hdr_k.type = NETIPC_TYPE_KMSG;
	send_hdr_k.remote = node_self();
	netvec_k[0].addr = KVTODEV(&send_hdr_k);
	netvec_k[0].size = sizeof(struct netipc_hdr);

	/*
	 * Initialize send_hdr_p and netvec_p
	 */
	send_hdr_p.type = NETIPC_TYPE_PAGE;
	send_hdr_p.remote = node_self();
	netvec_p[0].addr = KVTODEV(&send_hdr_p);
	netvec_p[0].size = sizeof(struct netipc_hdr);

	/*
	 * Initialize send_hdr_m and netvec_m
	 */
	send_hdr_m.type = NETIPC_TYPE_PAGE;
	send_hdr_m.remote = node_self();
	netvec_m[0].addr = KVTODEV(&send_hdr_m);
	netvec_m[0].size = sizeof(struct netipc_hdr);

	/*
	 * Initialize send_hdr_o and netvec_o
	 */
	send_hdr_o.type = NETIPC_TYPE_PAGE;
	send_hdr_o.remote = node_self();
	netvec_o[0].addr = KVTODEV(&send_hdr_o);
	netvec_o[0].size = sizeof(struct netipc_hdr);

	netipc_packet_zone = zinit(sizeof(struct netipc_packet), 512*1024,
				   PAGE_SIZE, FALSE, "netipc packet");
}

/*
 * Called from netipc_recv_retarget and netipc_recv_dead.
 */
ipc_port_t
netipc_dequeue_port(dp)
	register netipc_packet_t dp;
{
	ipc_port_t remote_port;

	assert(netipc_locked());
	assert(dp);
	printf1("netipc_dequeue_port(%d)\n", dp->dp_remote);
	netipc_set_seqid(dp, dp->dp_first_seqid);
	assert(dp->dp_type == DP_TYPE_KMSG);	/* is at start of dp (kmsg) */

	/*
	 * Remove this port from this node's queue.
	 * Leave the port referenced.
	 */
	remote_port = dp->dp_remote_port;
	assert(remote_port->ip_norma_dest_node == dp->dp_remote);
	dp->dp_remote_port = dp->dp_remote_port->ip_norma_queue_next;
	norma_ipc_unqueue(remote_port);

	/*
	 * Move kmsg from dp back onto port.
	 */
	assert(dp->dp_kmsg != IKM_NULL);
	ipc_kmsg_unrmqueue_first(&remote_port->ip_messages.imq_messages,
				 dp->dp_kmsg);
	remote_port->ip_msgcount++;

	/*
	 * If there is another port, start it sending;
	 * otherwise, release dp.
	 */
	if (dp->dp_remote_port != IP_NULL) {
		printf1("== dequeue_port: advancing to port 0x%x\n",
			dp->dp_remote_port);
		(void) dp_advance(dp);
		netipc_start(dp->dp_remote);
	} else {
		unsigned long remote = dp->dp_remote;
		printf1("== dequeue_port: no more ports\n");
		netipc_packet_deallocate(dp);
		netipc_packet[remote] = (netipc_packet_t) 0;
	}
	return remote_port;
}

#define	kmsg_size(kmsg) (ikm_plus_overhead(kmsg->ikm_header.msgh_size))

boolean_t
is_one_page_kmsg(kmsg)
	ipc_kmsg_t kmsg;
{
	if (kmsg->ikm_header.msgh_bits & MACH_MSGH_BITS_COMPLEX_DATA) {
		return FALSE;
	}
	if (kmsg_size(kmsg) > PAGE_SIZE) {
		return FALSE;
	}
	return TRUE;
}

/*
 *	Routine:	ipc_kmsg_copyout_to_network
 *	Purpose:
 *		Prepare a copied-in message for norma_ipc_send.
 *		This means translating ports to uids, translating
 *		entry-list copy objects into page list copy objects,
 *		and setting MACH_MSG_BITS_COMPLEX_XXX bits.
 *		Derived from ipc_kmsg_copyin_from_kernel.
 *	Conditions:
 *		Nothing locked.
 */

void
ipc_kmsg_copyout_to_network(kmsg)
	ipc_kmsg_t kmsg;
{
	vm_offset_t saddr, eaddr;
	kern_return_t kr;

	if ((kmsg->ikm_header.msgh_bits & MACH_MSGH_BITS_COMPLEX) == 0) {
		return;
	}

	saddr = (vm_offset_t) (&kmsg->ikm_header + 1);
	eaddr = (vm_offset_t) &kmsg->ikm_header + kmsg->ikm_header.msgh_size;

	while (saddr < eaddr) {
		register mach_msg_type_long_t *type;
		mach_msg_type_name_t name;
		mach_msg_type_size_t size;
		mach_msg_type_number_t number;
		vm_size_t length;

		type = (mach_msg_type_long_t *) saddr;
		if (type->msgtl_header.msgt_longform) {
			name = type->msgtl_name;
			size = type->msgtl_size;
			number = type->msgtl_number;
			saddr += sizeof(mach_msg_type_long_t);
		} else {
			name = type->msgtl_header.msgt_name;
			size = type->msgtl_header.msgt_size;
			number = type->msgtl_header.msgt_number;
			saddr += sizeof(mach_msg_type_t);
		}

		/* calculate length of data in bytes, rounding up */
		length = ((number * size) + 7) >> 3;

		if (length == 0) {
			continue;
		}

		if (MACH_MSG_TYPE_PORT_ANY(name)) {
			register ipc_port_t *ports;
			mach_msg_type_number_t i;

			if (type->msgtl_header.msgt_inline) {
				ports = (ipc_port_t *) saddr;
				saddr += (length + 3) &~ 3;
			} else {
				ports = (ipc_port_t *) *(vm_offset_t *) saddr;
				saddr += sizeof(vm_offset_t);
				kmsg->ikm_header.msgh_bits |=
				    MACH_MSGH_BITS_COMPLEX_DATA;
			}
			kmsg->ikm_header.msgh_bits |=
			    MACH_MSGH_BITS_COMPLEX_PORTS;
			for (i = 0; i < number; i++) {
				if (! IO_VALID((ipc_object_t) ports[i])) {
					/* XXX clean??? */
					ports[i] = IP_NULL;
					continue;
				}
				ports[i] = (ipc_port_t)
				    norma_ipc_send_port(ports[i], name);
			}
			continue;
		}

		if (type->msgtl_header.msgt_inline) {
			saddr += (length + 3) &~ 3;
			continue;
		}

		kmsg->ikm_header.msgh_bits |= MACH_MSGH_BITS_COMPLEX_DATA;
		kr = vm_map_convert_to_page_list((vm_map_copy_t *) saddr);
		if (kr != KERN_SUCCESS) {
			/*
			 * XXX
			 * vm_map_convert_to_page_list has failed.
			 * Now what? If the error had been detected
			 * before the MiG stub was invoked
			 * the stub would do the right thing,
			 * but it's a bit late now.
			 * Probably the best we can do is return a null
			 * copy object. Need to adjust number accordingly.
			 *
			 * XXX
			 * Discard original copy object?
			 */
			printf("XXX convert_to_network: page_list: %d", kr);
			if (type->msgtl_header.msgt_longform) {
				type->msgtl_number = 0;
			} else {
				type->msgtl_header.msgt_number = 0;
			}
			* (vm_map_copy_t *) saddr = VM_MAP_COPY_NULL;
		}
		saddr += sizeof(vm_offset_t);
	}
}

/*
 * Main entry point from regular ipc code.
 */
mach_msg_return_t
norma_ipc_send(kmsg)
	ipc_kmsg_t kmsg;
{
	register mach_msg_header_t *msgh;
	ipc_port_t local_port, remote_port;
	mach_msg_bits_t bits;

	ipc_kmsg_copyout_to_network(kmsg);

	msgh = (mach_msg_header_t *) &kmsg->ikm_header;
	remote_port = (ipc_port_t) msgh->msgh_remote_port;
	local_port = (ipc_port_t) msgh->msgh_local_port;
	bits = msgh->msgh_bits;

#if 1
	if (kmsg_size(kmsg) > PAGE_SIZE) {
		/*
		 * Now that we are actually using this bit in this case,
		 * we should simplify some of the tests below.
		 */
		msgh->msgh_bits = (bits |= MACH_MSGH_BITS_COMPLEX_DATA);
	}
#endif

	/*
	 * Get the receiver's uid.
	 */
	assert(remote_port->ip_norma_uid != 0);
	msgh->msgh_remote_port = (mach_port_t) remote_port->ip_norma_uid;

	/*
	 * If there is a local port, get its uid (creating one if necessary).
	 *
	 * Why do we only do this now?
	 * XXX
	 * Why copyin_type? Should we change the type in the message?
	 */
	if (local_port) {
		msgh->msgh_local_port = (mach_port_t)
		    norma_ipc_send_port(local_port, ipc_object_copyin_type(
				    MACH_MSGH_BITS_LOCAL(bits)));
	}

	/*
	 * Block interrupts while queueing message and port.
	 */
	netipc_thread_lock();

	/*
	 * XXX
	 * Should check to see whether this proxy died!?
	 */

	/*
	 * Enqueue the kmsg on the port.
	 */
	assert(remote_port->ip_pset == /*IPS_NULL*/0);	/* XXX ??? */
	remote_port->ip_msgcount++;
	ipc_kmsg_enqueue_macro(&remote_port->ip_messages.imq_messages, kmsg);

	/*
	 * Enqueue the port on the queue of ports with something to send.
	 */
	norma_ipc_queue_port(remote_port);

	/*
	 * Release spl, and return.
	 */
	netipc_thread_unlock();
	return KERN_SUCCESS;
}

/*
 * Put port on list of ports trying to send to port->ip_norma_det_node.
 * If there are no other ports, then start a send.
 */
void
norma_ipc_queue_port(remote_port)
	ipc_port_t remote_port;
{
	ipc_port_t port;
	ipc_kmsg_t kmsg;
	ipc_kmsg_queue_t kmsgs;
	unsigned long remote;
	netipc_packet_t dp;

	assert(netipc_locked());

	/*
	 * If it's on the list, return.
	 */
	if (norma_ipc_queued(remote_port)) {
		assert(netipc_packet[remote_port->ip_norma_dest_node] !=
		       NETIPC_PACKET_NULL);
#if	iPSC386 || iPSC860
		netipc_called_here(__FILE__, __LINE__, "norma_ipc_queue_port");
		if (verbose_already_queued) {
			printf("norma_ipc_queue_port(remote_port=%x) -- I saw one\n",
				remote_port);
		}
		norma_ipc_queue_port_on_list++;
#endif	iPSC386 || iPSC860
		return;
	}

	/*
	 * If there are other ports already on the list,
	 * then queue the port and return.
	 */
	remote = remote_port->ip_norma_dest_node;
	dp = netipc_packet[remote];
	if (dp != NETIPC_PACKET_NULL) {
		for (port = dp->dp_remote_port;
		     port->ip_norma_queue_next;
		     port = port->ip_norma_queue_next) {
			continue;
		}
		remote_port->ip_norma_queue_next = IP_NULL;
		port->ip_norma_queue_next = remote_port;
		return;
	}

	/*
	 * Pull first kmsg from port.
	 */
	kmsgs = &remote_port->ip_messages.imq_messages;
	kmsg = ipc_kmsg_queue_first(kmsgs);
	assert(kmsg != IKM_NULL);
	ipc_kmsg_rmqueue_first_macro(kmsgs, kmsg);
	remote_port->ip_msgcount--;

	/*
	 * Allocate and initialize a dp.
	 */
	dp = netipc_packet_allocate();
	if (dp == NETIPC_PACKET_NULL) {
		panic("netipc_packet_allocate. bogus panic.\n");
	}
	netipc_packet[remote] = dp;
	dp->dp_first_seqid = Xnetipc_next_seqid(remote);
	dp->dp_last_unacked = dp->dp_first_seqid;
	dp->dp_type = DP_TYPE_KMSG;
	dp->dp_remote = remote;
	dp->dp_remote_port = remote_port;
	dp->dp_remote_port->ip_norma_queue_next = IP_NULL;
	dp->dp_kmsg = kmsg;
	if (is_one_page_kmsg(kmsg)) {
		dp->dp_last_seqid = dp->dp_first_seqid;
	} else {
		dp->dp_last_seqid = 0;
	}

	/*
	 * Send it if we can.
	 */
	netipc_start(remote);
}

/*
 * Return to port message queue a kmsg removed via ipc_kmsg_rmqueue_first.
 */
ipc_kmsg_unrmqueue_first(queue, kmsg)
	ipc_kmsg_queue_t queue;
	ipc_kmsg_t kmsg;
{
	assert(netipc_locked());
	printf1("*** ipc_kmsg_unrmqueue_first(0x%x, 0x%x)\n", queue, kmsg);
	if (queue->ikmq_base == IKM_NULL) {
		kmsg->ikm_next = kmsg;
		kmsg->ikm_prev = kmsg;
	} else {
		register ipc_kmsg_t first = queue->ikmq_base;
		register ipc_kmsg_t last = first->ikm_prev;

		kmsg->ikm_next = first;
		kmsg->ikm_prev = last;
		first->ikm_prev = kmsg;
		last->ikm_next = kmsg;
	}
	queue->ikmq_base = kmsg;
}

ipc_port_t netipc_dead_port_list = IP_NULL;

/*
 * Called from netipc_send_dp.
 */
netipc_send_kmsg(remote, dp)
	unsigned long remote;
	register netipc_packet_t dp;
{
	unsigned int netvec_count;
	vm_offset_t length;

#if	iPSC386 || iPSC860
	netipc_called_here(__FILE__, __LINE__, "netipc_send_kmsg");
#endif	iPSC386 || iPSC860
	assert(netipc_locked());

	/*
	 * Kmsgs are word aligned.
	 */
	assert(WORD_ALIGNED(dp->dp_kmsg));

	/*
	 * Fill in send_hdr_k.
	 */
	send_hdr_k.seqid = dp->dp_seqid;

	/*
	 * Fill in netvec_k.
	 * Cache KVTODEV and page-splitting computations.
	 * (Kmsgs occasionally cross page boundaries, unfortunately.)
	 *
	 * This routine attempts to cache the results of KVTODEV
	 * since it is relatively expensive.
	 * This caching may be less effective now since we've added
	 * flow control, since we don't immediately stuff the kmsg back
	 * into the ikm_cache, which means it might not be the kmsg
	 * we see next. Perhaps we can use the kmsg->ikm_page field
	 * for caching physaddr?
	 */
	if (dp->dp_kmsg != netipc_kmsg_cache) {
		vm_offset_t data = (vm_offset_t) dp->dp_kmsg;
		netipc_kmsg_cache = dp->dp_kmsg;

		netipc_kmsg_first_half = round_page(data) - data;
			
		netvec_k[1].addr = KVTODEV(data);
		netvec_k[2].addr = KVTODEV(data + netipc_kmsg_first_half);
		netipc_kmsg_cache_misses++;
	} else {
		netipc_kmsg_cache_hits++;
	}

	/*
	 * Calculate how much of kmsg to send.
	 */
	length = kmsg_size(dp->dp_kmsg);
	length = ROUND_WORD(length);
	if (length > PAGE_SIZE) {
		length = PAGE_SIZE;
	}

	/*
	 * Set vector, with either one or two pieces for kmsg.
	 */
	if (length > netipc_kmsg_first_half) {
		netvec_k[1].size = netipc_kmsg_first_half;
		netvec_k[2].size = length - netipc_kmsg_first_half;
		netvec_count = 3;
		netipc_kmsg_splits++;
	} else {
		netvec_k[1].size = length;
		netvec_count = 2;
	}

	/*
	 * Start the send, and the associated timer.
	 */
	netipc_send_with_timeout(remote, netvec_k, netvec_count,
				 (unsigned long) dp, dp->dp_seqid);
}

/*
 * Called from netipc_send_dp.
 *
 * Derived from netipc_send_kmsg and netipc_send_page.
 * Sends from a kmsg but uses page packets.
 */
netipc_send_kmsg_more(remote, dp)
	unsigned long remote;
	register netipc_packet_t dp;
{
	vm_size_t first_half, length;
	vm_offset_t data, offset;

#if	iPSC386 || iPSC860
	netipc_called_here(__FILE__, __LINE__, "netipc_send_kmsg_more");
#endif	iPSC386 || iPSC860
	assert(dp->dp_type == DP_TYPE_KMSG_MORE);

	assert(netipc_locked());

	/*
	 * Kmsgs are word aligned.
	 */
	assert(WORD_ALIGNED(dp->dp_kmsg));

	/*
	 * Calculate where in the kmsg to start,
	 * and how much to send.
	 */
	offset = PAGE_SIZE * dp->dp_copy_index + PAGE_SIZE;
	data = (vm_offset_t) dp->dp_kmsg + offset;
	length = kmsg_size(dp->dp_kmsg) - offset;
	length = ROUND_WORD(length);
	if (length > PAGE_SIZE) {
		length = PAGE_SIZE;
	}

	/*
	 * Fill in send_hdr_m.
	 */
	send_hdr_m.pg.pg_copy_offset = 0;
	send_hdr_m.pg.pg_msgh_offset = 0;
	send_hdr_m.pg.pg_page_first = (dp->dp_copy_index == 0);
	send_hdr_m.pg.pg_page_last = (dp->dp_copy_index ==
				      dp->dp_copy_npages - 1);
	send_hdr_m.pg.pg_copy_size = dp->dp_copy->size;
	send_hdr_m.pg.pg_copy_last = dp->dp_copy_last;
	send_hdr_m.seqid = dp->dp_seqid;

	/*
	 * If data crosses a page boundary, we need to point netvec_m
	 * to both physical pages involved.
	 */
	first_half = round_page(data) - data;
	if (length > first_half) {
		netvec_m[1].addr = KVTODEV(data);
		netvec_m[1].size = first_half;

		netvec_m[2].addr = KVTODEV(data + first_half);
		netvec_m[2].size = length - first_half;

		netipc_send_with_timeout(remote, netvec_m, 3,
					 (unsigned long) dp, dp->dp_seqid);
	} else {
		netvec_m[1].addr = KVTODEV(data);
		netvec_m[1].size = length;

		netipc_send_with_timeout(remote, netvec_m, 2,
					 (unsigned long) dp, dp->dp_seqid);
	}
}

/*
 * Called from netipc_send_dp.
 *
 * Derived from netipc_send_kmsg_more.
 * Sends from a kalloc'd region containing out-of-line ports,
 * but uses page packets.
 */
netipc_send_ool_ports(remote, dp)
	unsigned long remote;
	register netipc_packet_t dp;
{
	vm_size_t first_half, length;
	vm_offset_t data, offset;

#if	iPSC386 || iPSC860
	netipc_called_here(__FILE__, __LINE__, "netipc_send_ool_ports");
#endif	iPSC386 || iPSC860
	assert(netipc_locked());
	assert(dp->dp_type == DP_TYPE_OOL_PORTS);

	data = * (vm_offset_t *)
	    ((vm_offset_t) &dp->dp_kmsg->ikm_header + dp->dp_offset);

	/*
	 * Kalloc'd regions for out-of-line ports are word aligned.
	 */
	assert(WORD_ALIGNED(data));

	/*
	 * Calculate where in the kmsg to start,
	 * and how much to send.
	 */
	offset = PAGE_SIZE * dp->dp_copy_index;
	data += offset;
	length = dp->dp_copy->size - offset;
	length = ROUND_WORD(length);
	if (length > PAGE_SIZE) {
		length = PAGE_SIZE;
	}

	/*
	 * Fill in send_hdr_o.
	 */
	send_hdr_o.pg.pg_copy_offset = 0;
	send_hdr_o.pg.pg_msgh_offset = dp->dp_offset;
	send_hdr_o.pg.pg_page_first = (dp->dp_copy_index == 0);
	send_hdr_o.pg.pg_page_last = (dp->dp_copy_index ==
				      dp->dp_copy_npages - 1);
	send_hdr_o.pg.pg_copy_size = dp->dp_copy->size;
	send_hdr_o.pg.pg_copy_last = dp->dp_copy_last;
	send_hdr_o.seqid = dp->dp_seqid;

	/*
	 * If data crosses a page boundary, we need to point netvec_o
	 * to both physical pages involved.
	 */
	first_half = round_page(data) - data;
	if (length > first_half) {
		netvec_o[1].addr = KVTODEV(data);
		netvec_o[1].size = first_half;

		netvec_o[2].addr = KVTODEV(data + first_half);
		netvec_o[2].size = length - first_half;

		netipc_send_with_timeout(remote, netvec_o, 3,
					 (unsigned long) dp, dp->dp_seqid);
	} else {
		netvec_o[1].addr = KVTODEV(data);
		netvec_o[1].size = length;

		netipc_send_with_timeout(remote, netvec_o, 2,
					 (unsigned long) dp, dp->dp_seqid);
	}
}

/*
 * Called from netipc_send_dp.
 */
netipc_send_page(remote, dp)
	unsigned long remote;
	register netipc_packet_t dp;
{
	vm_page_t *page_list;
	unsigned long align;
	unsigned long length;
	unsigned long offset;

#if	iPSC386 || iPSC860
	netipc_called_here(__FILE__, __LINE__, "netipc_send_page");
#endif	iPSC386 || iPSC860
	if (dp->dp_copy_index == 0) {
		dp->dp_page_list_base = 0;
		dp->dp_has_continuation =
		    vm_map_copy_has_interesting_cont(dp->dp_copy);
		dp->dp_being_continued = FALSE;
#if	iPSC386 || iPSC860
		netipc_called_here(__FILE__, __LINE__, "netipc_send_page (dp_copy_index == 0)");
#endif	iPSC386 || iPSC860
	}
	if (dp->dp_has_continuation) {
#if	iPSC386 || iPSC860
		netipc_last_interesting_dp = dp;
		netipc_last_interesting_copy = dp->dp_copy;
		netipc_last_dp_copy_continuation = (dp->dp_copy)->cpy_cont;
		netipc_called_here(__FILE__, __LINE__, "netipc_send_page (has continuation)");
#endif	iPSC386 || iPSC860
		netipc_send_page_with_continuation(remote, dp);
		return;
	}

	assert(netipc_locked());
	assert(dp->dp_copy_index < dp->dp_copy_npages);

	/*
	 * Calculate length and offset.
	 * Round both to word boundaries.
	 */
#if	NORMA_REALIGN_OOL_DATA
	offset = (dp->dp_copy->offset & page_mask);
	align = (offset & WORD_MASK);
	if (dp->dp_copy_index < dp->dp_copy_npages - 1) {
		/*
		 * This is not the last page and therefore length will
		 * be a whole page. We just need to make offset word aligned.
		 */
		offset -= align;
		length = PAGE_SIZE;
	} else if (offset == 0) {
		/*
		 * Offset is page aligned and therefore word aligned.
		 * We just need to set length.
		 */
		length = (dp->dp_copy->size & page_mask);
		if (length == 0) {
			length = PAGE_SIZE;
		} else {
			length = ROUND_WORD(length);
		}
	} else {
		/*
		 * This is the last page, and this page list did not
		 * start on a page boundary.
		 *
		 * This code should correspond to the code in
		 * netipc_next_copy_object to calculate dp_copy_npages.
		 */
		vm_offset_t end = offset + dp->dp_copy->size;
		offset -= align;
		end = ROUND_WORD(end);
		length = ((end - offset) & page_mask);
		if (length == 0) {
			length = PAGE_SIZE;
		}
	}
#else	NORMA_REALIGN_OOL_DATA
	offset = 0;
	align = dp->dp_copy->offset & page_mask;
	if (dp->dp_copy_index < dp->dp_copy_npages - 1) {
		length = PAGE_SIZE;
	} else {
		length = ((dp->dp_copy->size + align) & page_mask);
		if (length == 0) {
			length = PAGE_SIZE;
		} else {
			length = ROUND_WORD(length);
		}
	}
#endif	NORMA_REALIGN_OOL_DATA

	assert(WORD_ALIGNED(offset));
	assert(WORD_ALIGNED(length));
	assert(length > 0);
	assert(length <= PAGE_SIZE);

	send_hdr_p.pg.pg_copy_offset = align;
	send_hdr_p.pg.pg_msgh_offset = dp->dp_offset;
	send_hdr_p.pg.pg_copy_size = dp->dp_copy->size;
	send_hdr_p.pg.pg_page_first = (dp->dp_copy_index == 0);
	send_hdr_p.pg.pg_page_last = (dp->dp_copy_index ==
				      dp->dp_copy_npages - 1);
	send_hdr_p.pg.pg_copy_last = dp->dp_copy_last;
	send_hdr_p.seqid = dp->dp_seqid;

	/*
	 * If data crosses a page boundary, we need to point netvec_p
	 * to both physical pages involved.
	 */
	page_list = &dp->dp_copy->cpy_page_list[dp->dp_copy_index];
	if (offset + length > PAGE_SIZE) {
		vm_offset_t first_half = PAGE_SIZE - offset;

		netvec_p[1].addr = VPTODEV(page_list[0]) + offset;
		netvec_p[1].size = first_half;

		netvec_p[2].addr = VPTODEV(page_list[1]);
		netvec_p[2].size = length - first_half;

		netipc_send_with_timeout(remote, netvec_p, 3,
					 (unsigned long) dp, dp->dp_seqid);
	} else {
		netvec_p[1].addr = VPTODEV(page_list[0]) + offset;
		netvec_p[1].size = length;

		netipc_send_with_timeout(remote, netvec_p, 2,
					 (unsigned long) dp, dp->dp_seqid);
	}
}

/*
 * Like netipc_send_page, but can deal with copy objects with continuations.
 * Does not try to be tricky about changing allignment, which is okay, because
 * beginning/end page fragementation is less significant for the large copy
 * objects that typically have continuations.
 *
 * XXX
 * This turns out not to be that different from netipc_send_page,
 * so should probably remerge the two.
 */
netipc_send_page_with_continuation(remote, dp)
	unsigned long remote;
	register netipc_packet_t dp;
{
	unsigned long align;
	unsigned long length;
	int index;

#if	iPSC386 || iPSC860
	netipc_called_here(__FILE__, __LINE__, "netipc_send_page_with_continuation");
#endif	iPSC386 || iPSC860
	assert(netipc_locked());
	assert(dp->dp_has_continuation);

	/*
	 * If we are currently being continued, return right away.
	 */
	if (dp->dp_being_continued) {
		printf3("being continued\n");
#if	iPSC386 || iPSC860
		netipc_called_here(__FILE__, __LINE__, "{continued...}");
#endif	iPSC386 || iPSC860
		return;
	}

	/*
	 * Calculate index into current page list from
	 * dp_copy_index, the current number of pages sent.
	 * If dp_copy is a continuation, these numbers won't be the same.
	 */
	index = dp->dp_copy_index - dp->dp_page_list_base;
	printf3("send_page_with_cont dp=0x%x c_idx=%d c_npages=%d idx=%d\n",
		dp, dp->dp_copy_index, dp->dp_copy_npages, index);
	assert(index >= 0 && index <= dp->dp_copy->cpy_npages);

	/*
	 * We may be at the end of the current page list,
	 * in which case we need to call the copy continuation.
	 * We cannot do this ourselves, since the operation might
	 * block. We therefore let the netipc thread do it.
	 * It will call this routine again with dp_page_list_base reset.
	 */
	if (index == dp->dp_copy->cpy_npages) {
		netipc_safe_vm_map_copy_invoke_cont(dp);
#if	iPSC386 || iPSC860
		netipc_called_here(__FILE__, __LINE__, "{netipc_safe_vm_map_copy_invoke_cont");
#endif	iPSC386 || iPSC860
		return;
	}

	/*
	 * Calculate length. Round to word boundary.
	 */
	align = dp->dp_copy->offset & page_mask;
	if (index < dp->dp_copy_npages - 1) {
		length = PAGE_SIZE;
	} else {
		length = ((dp->dp_copy->size + align) & page_mask);
		if (length == 0) {
			length = PAGE_SIZE;
		} else {
			length = ROUND_WORD(length);
		}
	}

	assert(WORD_ALIGNED(length));
	assert(length > 0);
	assert(length <= PAGE_SIZE);

	send_hdr_p.pg.pg_copy_offset = align;
	send_hdr_p.pg.pg_msgh_offset = dp->dp_offset;
	send_hdr_p.pg.pg_copy_size = dp->dp_copy->size;
	send_hdr_p.pg.pg_page_first = (dp->dp_copy_index == 0);
	send_hdr_p.pg.pg_page_last = (dp->dp_copy_index ==
				      dp->dp_copy_npages - 1);
	send_hdr_p.pg.pg_copy_last = dp->dp_copy_last;
	send_hdr_p.seqid = dp->dp_seqid;

	netvec_p[1].addr = VPTODEV(dp->dp_copy->cpy_page_list[index]);
	netvec_p[1].size = length;

	netipc_send_with_timeout(remote, netvec_p, 2,
				 (unsigned long) dp, dp->dp_seqid);
}

/*
 * Advance dp->dp_copy to the next copy object in the list.
 */
netipc_next_copy_object(dp)
	register netipc_packet_t dp;
{
	vm_offset_t saddr, eaddr;
	ipc_kmsg_t kmsg = dp->dp_kmsg;

	assert(netipc_locked());
	assert(dp->dp_offset >= sizeof(kmsg->ikm_header));
	assert(dp->dp_type == DP_TYPE_PAGE ||
	       dp->dp_type == DP_TYPE_KMSG_MORE ||
	       dp->dp_type == DP_TYPE_OOL_PORTS);

	saddr = (vm_offset_t) &kmsg->ikm_header + dp->dp_offset;
	eaddr = (vm_offset_t) &kmsg->ikm_header + kmsg->ikm_header.msgh_size;

	dp->dp_copy = VM_MAP_COPY_NULL;
	dp->dp_copy_index = 0;
	dp->dp_copy_last = TRUE;

	if (dp->dp_type == DP_TYPE_KMSG_MORE) {
		dp->dp_copy = &netipc_kmsg_more_copy;
		dp->dp_copy->size = kmsg_size(kmsg) - PAGE_SIZE;
		dp->dp_copy_npages = atop(round_page(dp->dp_copy->size));
		assert(dp->dp_copy->size > 0);
		assert(dp->dp_copy_npages > 0);
	} else if (dp->dp_offset > sizeof(kmsg->ikm_header)) {
		printf4("nextipc_next_copy_object: multiple copy objects\n");
		/* skip copy object to get to next type record */
		saddr += sizeof(vm_offset_t);
		printf4("nextipc_next_copy_object: saddr=0x%x, eaddr=0x%x\n",
		       saddr, eaddr);
	}

	while (saddr < eaddr) {
		mach_msg_type_long_t *type;
		mach_msg_type_name_t name;
		mach_msg_type_size_t size;	/* XXX */
		mach_msg_type_number_t number;	/* XXX */
		vm_size_t length;

		type = (mach_msg_type_long_t *) saddr;
		if (type->msgtl_header.msgt_longform) {
			name = type->msgtl_name;
			size = type->msgtl_size;
			number = type->msgtl_number;
			saddr += sizeof(mach_msg_type_long_t);
		} else {
			name = type->msgtl_header.msgt_name;
			size = type->msgtl_header.msgt_size;
			number = type->msgtl_header.msgt_number;
			saddr += sizeof(mach_msg_type_t);
		}

		/* calculate length of data in bytes, rounding up */

		length = ((number * size) + 7) >> 3;

		if (type->msgtl_header.msgt_inline) {
			/* inline data sizes round up to int boundaries */
			saddr += (length + 3) &~ 3;
			continue;
		}

		/*
		 * XXX This is required because net_deliver does
		 * XXX funny rounding to msgh_size.
		 * XXX Why doesn't anything in ipc/ipc_kmsg.c need this?
		 */
		if (saddr >= eaddr) {
			printf4("nextipc_next_co: saddr=0x%x, eaddr=0x%x\n",
			       saddr, eaddr);
			break;
		}

		if (* (vm_map_copy_t *) saddr == VM_MAP_COPY_NULL) {
			saddr += sizeof(vm_offset_t);
			continue;
		}

		if (dp->dp_copy) {
			printf4("setting dp_copy_last false!\n");
			dp->dp_copy_last = FALSE;
			break;
		}

		if (MACH_MSG_TYPE_PORT_ANY(name)) {
			dp->dp_type = DP_TYPE_OOL_PORTS;
			dp->dp_copy = &netipc_ool_ports_copy;
			dp->dp_copy->size = length;
			dp->dp_copy_npages = atop(round_page(length));
			dp->dp_offset = saddr - (vm_offset_t)&kmsg->ikm_header;
			assert(dp->dp_copy->size > 0);
			saddr += sizeof(vm_offset_t);
			continue;
		}

		dp->dp_copy = * (vm_map_copy_t *) saddr;
		dp->dp_offset = saddr - (vm_offset_t) &kmsg->ikm_header;

		assert(dp->dp_copy != VM_MAP_COPY_NULL);
		assert(dp->dp_copy->type == VM_MAP_COPY_PAGE_LIST);
		assert(dp->dp_copy->size == length);

		if (vm_map_copy_has_interesting_cont(dp->dp_copy)) {
			/*
			 * This copy object has a continuation,
			 * which means that we won't change alignment,
			 * thus dp_copy_npages, the number of pages that
			 * the copy object will have on the destination,
			 * is the same as the number of pages that it
			 * has here. We cannot use dp->dp_copy->cpy_npages
			 * since that is just the number of pages in the
			 * first page list in the copy object.
			 */
/*			panic("0x%x has continuation\n", dp->dp_copy);*/
			dp->dp_copy_npages =
			    atop(round_page(dp->dp_copy->offset +
					    dp->dp_copy->size) -
				 trunc_page(dp->dp_copy->offset));
			assert(dp->dp_copy_npages >= dp->dp_copy->cpy_npages);
		} else {
			/*
			 * This copy object does not have a continuation,
			 * and therefore things are simple enough that we
			 * will bother to change alignment if we can send
			 * the copy object in one fewer pages than it
			 * currently occupies, which is possible when the
			 * total amount used by the first and last pages
			 * is no larger than a page, after taking word
			 * alignment into account.
			 */
			vm_offset_t offset = dp->dp_copy->offset & page_mask;
			vm_offset_t end = offset + dp->dp_copy->size;
#if	NORMA_REALIGN_OOL_DATA
			offset = TRUNC_WORD(offset);
			end = ROUND_WORD(end);
			dp->dp_copy_npages = atop(round_page(end - offset));
#else	NORMA_REALIGN_OOL_DATA
			dp->dp_copy_npages = atop(round_page(end) -
						  trunc_page(offset));
#endif	NORMA_REALIGN_OOL_DATA
			assert(dp->dp_copy_npages ==
			       dp->dp_copy->cpy_npages ||
			       dp->dp_copy_npages ==
			       dp->dp_copy->cpy_npages - 1);
		}
		saddr += sizeof(vm_offset_t);
	}
	assert(dp->dp_copy);
}

/*
 * This routine is overly general because it was written before it was clear
 * that copy object page lists cannot be backed up once their continuation
 * has been called.
 *
 * Note: dp_last_seqid is set when we first visit it.
 */
netipc_set_seqid(dp, seqid)
	register netipc_packet_t dp;
	register unsigned long seqid;
{
	assert(netipc_locked());

	/*
	 * Are we there already?
	 */
	if (dp->dp_seqid == seqid) {
		return;
	}

	/*
	 * We must be in the correct dp.
	 */
	assert(dp->dp_first_seqid <= seqid);
	assert(dp->dp_last_seqid == 0 || seqid <= dp->dp_last_seqid);

	/*
	 * If we want to be at the kmsg, go there.
	 */
	if (dp->dp_first_seqid == seqid) {
		dp->dp_type = DP_TYPE_KMSG;
		dp->dp_seqid = seqid;
		return;
	}
	  
	/*
	 * If we are in the right copy object, just change the index.
	 */
	if (dp->dp_type != DP_TYPE_KMSG) {
		int index = dp->dp_copy_index + (seqid - dp->dp_seqid);
		if (index >= 0 && index < dp->dp_copy_npages) {
			dp->dp_copy_index = index;
			dp->dp_seqid = seqid;
			return;
		}
	}

	/*
	 * We might be too far forward and thus need to back up.
	 * The easiest way of backing up is to start at the kmsg
	 * and walk forward. This isn't necessary the most efficient way!
	 *
	 * Note that this cannot happen if this is a simple message.
	 *
	 * XXX
	 * Page-list continuations limit how far we can back up.
	 */
	if (dp->dp_seqid > seqid) {
		dp->dp_seqid = dp->dp_first_seqid;
		assert(dp->dp_first_seqid < seqid);
	}

	/*
	 * If we are currently at the kmsg, advance to the first copy object.
	 * Otherwise, advance seqid to next copy object.
	 *
	 * XXX where do we make a fast check for simple messages?
	 */
	if (dp->dp_seqid == dp->dp_first_seqid) {
		dp->dp_seqid++;
		if (kmsg_size(dp->dp_kmsg) > PAGE_SIZE) {
			dp->dp_type = DP_TYPE_KMSG_MORE;
		} else {
			dp->dp_type = DP_TYPE_PAGE;
		}
		dp->dp_offset = sizeof(dp->dp_kmsg->ikm_header);
	} else {
		printf4(">> %d ", dp->dp_seqid);
		dp->dp_seqid -= dp->dp_copy_index; /* beginning of this obj */
		printf4(">> %d ", dp->dp_seqid);
		dp->dp_seqid += dp->dp_copy_npages; /* begin of next */
		printf4("-> %d\n", dp->dp_seqid);
	}

	/*
	 * Examine each copy object to see whether it contains seqid.
	 * If it does, set index appropriately and return.
	 *
	 * XXX
	 * This should no longer be a for loop. We should only
	 * need to walk to the next copy object.
	 *
	 * XXX
	 * Should discard current copy object?!
	 */
	for (;;) {
		netipc_next_copy_object(dp);
		if (dp->dp_copy_last && dp->dp_last_seqid == 0) {
			dp->dp_last_seqid =
			    dp->dp_seqid + dp->dp_copy_npages - 1;
		}
		assert(seqid >= dp->dp_seqid);
		if (seqid < dp->dp_seqid + dp->dp_copy_npages) {
			dp->dp_copy_index = seqid - dp->dp_seqid;
			dp->dp_seqid = seqid;
			return;
		}
		assert(! dp->dp_copy_last);
		dp->dp_seqid += dp->dp_copy_npages;
	}
}

/*
 * Called when dp->dp_kmsg has been completely sent, and it's time
 * to move to the next kmsg destined for dp->dp_remote, either from
 * the current port or from the next one.
 *
 * Returns true if there is still something to send.
 */
boolean_t
dp_advance(dp)
	register netipc_packet_t dp;
{
	ipc_kmsg_t kmsg;
	struct ipc_kmsg_queue *kmsgs;

	assert(netipc_locked());

	/*
	 * Find next kmsg on the current port.
	 */
	kmsgs = &dp->dp_remote_port->ip_messages.imq_messages;
	kmsg = ipc_kmsg_queue_first(kmsgs);

	/*
	 * If there are no more kmsgs on this port,
	 * move to the next port and check there.
	 */
	if (kmsg == IKM_NULL) {
		ipc_port_t port;

		/*
		 * If there are no more ports waiting to send
		 * to this node, reset and return.
		 */
		port = dp->dp_remote_port->ip_norma_queue_next;
		norma_ipc_unqueue(dp->dp_remote_port);
		if (port == IP_NULL) {
			unsigned long remote = dp->dp_remote;
			netipc_packet_deallocate(dp);
			netipc_packet[remote] = (netipc_packet_t) 0;
			return FALSE;
		}
		dp->dp_remote_port = port;

		/*
		 * Find first kmsg on the new port.
		 */
		kmsgs = &port->ip_messages.imq_messages;
		kmsg = ipc_kmsg_queue_first(kmsgs);
	}

	/*
	 * Remove the kmsg from the port.
	 */
	assert(kmsg != IKM_NULL);
	ipc_kmsg_rmqueue_first_macro(kmsgs, kmsg);
	dp->dp_remote_port->ip_msgcount--;

	/*
	 * Reset dp and return success.
	 */
	dp->dp_kmsg = kmsg;
	dp->dp_type = DP_TYPE_KMSG;
	dp->dp_first_seqid = Xnetipc_next_seqid(dp->dp_remote);
	dp->dp_last_unacked = dp->dp_first_seqid;
	if (is_one_page_kmsg(kmsg)) {
		dp->dp_last_seqid = dp->dp_first_seqid;
	} else {
		dp->dp_last_seqid = 0;
	}
	return TRUE;
}

/*
 * Free kmsg and last copy object associated with dp->dp_kmsg.
 */
dp_finish(dp)
	netipc_packet_t dp;
{
	register ipc_kmsg_t kmsg = dp->dp_kmsg;
	mach_msg_bits_t bits;
	
	/*
	 * Queue kmsg to be freed, after getting bits and storing remote port.
	 */
	printf2("-free %d..%d\n", dp->dp_first_seqid, dp->dp_last_seqid);
	bits = kmsg->ikm_header.msgh_bits;
	kmsg->ikm_header.msgh_remote_port = (mach_port_t) dp->dp_remote_port;
	netipc_safe_ikm_free(kmsg);

	/*
	 * Discard last copy object.
	 *
	 * XXX Should have discarded all previous copy objects.
	 * XXX Netipc_next_copy_object obvious place to do so.
	 */
	if (bits & MACH_MSGH_BITS_COMPLEX_DATA) {
		if (! dp->dp_copy) {
			netipc_set_seqid(dp, dp->dp_last_seqid);
			assert(dp->dp_copy);
		}
		if (dp->dp_copy != &netipc_kmsg_more_copy &&
		    dp->dp_copy != &netipc_ool_ports_copy) {
			netipc_safe_vm_map_copy_discard(dp->dp_copy);
		}
	}
	
}

/*
 * Received successful ack of seqid.
 */
void
netipc_recv_success(dp, seqid)
	netipc_packet_t dp;
	unsigned long seqid;
{
	assert(netipc_locked());
	assert(seqid == dp->dp_last_unacked);
	dp->dp_last_unacked++;
	if (dp->dp_last_seqid && dp->dp_last_seqid == seqid) {
		dp_finish(dp);
		if (dp_advance(dp)) {
			netipc_start(dp->dp_remote);
		}
	} else {
		netipc_start(dp->dp_remote);
	}
}

void
netipc_recv_retarget(dp, seqid, new_remote)
	netipc_packet_t dp;
	unsigned long seqid;
	unsigned long new_remote;
{
	ipc_port_t remote_port;

	assert(netipc_locked());
	assert(seqid == dp->dp_first_seqid);

	/*
	 * Handle acknowledgement stuff, and find port.
	 */
	remote_port = netipc_dequeue_port(dp);
	if (remote_port == IP_NULL) {
		return;
	}

	/*
	 * Reset destination node field of destination port.
	 * If new destination is still remote, then start a send;
	 * otherwise, queued messages will be absorbed automatically by
	 * norma_ipc_receive_rright. XXX (that part does not work yet) XXX
	 */
	remote_port->ip_norma_dest_node = new_remote;
	if (new_remote != node_self()) {
		norma_ipc_queue_port(remote_port);
	} else {
		printf("*** TELL JOE: retarget to node self.\n");
	}
}

void
netipc_recv_dead(dp, seqid)
	netipc_packet_t dp;
	unsigned long seqid;
{
	ipc_port_t remote_port;

	assert(netipc_locked());
	assert(seqid == dp->dp_first_seqid);

	/*
	 * Handle acknowledgement stuff, and find port.
	 */
	remote_port = netipc_dequeue_port(dp);
	if (remote_port == IP_NULL) {
		return;
	}
	printf1("*** netipc_recv_dead! 0x%x:%x\n",
	       remote_port, remote_port->ip_norma_uid);

	/*
	 * Put the port on the dead port list, so that the netipc thread
	 * can find it and call norma_ipc_destroy_proxy.
	 *
	 * Using ip_norma_queue_next makes the port look
	 * like it's already queued. This will prevent norma_ipc_queue_port
	 * from sticking it on the queue again and starting another send.
	 */
	assert(! norma_ipc_queued(remote_port));
	remote_port->ip_norma_queue_next = netipc_dead_port_list;
	netipc_dead_port_list = remote_port;
}

void
netipc_recv_not_found(dp, seqid)
	netipc_packet_t dp;
	unsigned long seqid;
{
	assert(netipc_locked());
	/*
	 * XXX For now, we handle this as if the port had died.
	 */
	printf1("netipc_recv_not_found!\n");
	netipc_recv_dead(dp, seqid);
}

void
netipc_recv_ack_with_status(packetid, seqid, status, data)
	unsigned long packetid;
	unsigned long seqid;
	kern_return_t status;
	unsigned long data;
{
	netipc_packet_t dp;

	dp = (netipc_packet_t) packetid;
	assert(dp);
	if (status == KERN_SUCCESS) {
		netipc_recv_success(dp, seqid);
	} else if (status == KERN_NOT_RECEIVER) {
		netipc_recv_retarget(dp, seqid, data);
	} else if (status == KERN_INVALID_RIGHT) {
		netipc_recv_dead(dp, seqid);
	} else if (status == KERN_INVALID_NAME) {
		netipc_recv_not_found(dp, seqid);
	} else {
		panic("status %d from receive_dest\n", status);
	}
}

netipc_send_dp(remote, dp)
	unsigned long remote;
	netipc_packet_t dp;
{
#if	iPSC386 || iPSC860
	netipc_called_here(__FILE__, __LINE__, "netipc_send_dp (enter)");
#endif	iPSC386 || iPSC860
	if (dp->dp_type == DP_TYPE_KMSG) {
		netipc_send_kmsg(remote, dp);
	} else if (dp->dp_type == DP_TYPE_PAGE) {
		netipc_send_page(remote, dp);
	} else if (dp->dp_type == DP_TYPE_KMSG_MORE) {
		netipc_send_kmsg_more(remote, dp);
	} else {
		assert(dp->dp_type == DP_TYPE_OOL_PORTS);
		netipc_send_ool_ports(remote, dp);
	}
#if	iPSC386 || iPSC860
	netipc_called_here(__FILE__, __LINE__, "netipc_send_dp (leave)");
#endif	iPSC386 || iPSC860
}

/*
 * Called from lower level when we have previously stated that we
 * have more to send and when the send interface is not busy.
 *
 * Seqid is the new seqid that should be used if there is something to send.
 */
boolean_t
netipc_send_new(remote, seqid)
	unsigned long remote;
	unsigned long seqid;
{
	register netipc_packet_t dp;

#if	iPSC386 || iPSC860
	netipc_called_here(__FILE__, __LINE__, "netipc_send_new (enter)");
#endif	iPSC386 || iPSC860
	assert(netipc_locked());
	dp = netipc_packet[remote];
	if (dp == NETIPC_PACKET_NULL) {
#if	iPSC386 || iPSC860
		netipc_called_here(__FILE__, __LINE__, "{dp == 0, leave}");
#endif	iPSC386 || iPSC860
		return FALSE;
	}
	assert(dp->dp_remote == remote);
	if (dp->dp_last_seqid && seqid > dp->dp_last_seqid) {
#if	iPSC386 || iPSC860
		netipc_called_here(__FILE__, __LINE__, "{seqid > last, leave}");
#endif	iPSC386 || iPSC860
		return FALSE;
	}
	if (seqid > dp->dp_last_unacked) {
#if	iPSC386 || iPSC860
		netipc_called_here(__FILE__, __LINE__, "{seqid > last_unacked, leave}");
#endif	iPSC386 || iPSC860
		return FALSE;	/* stop-and-wait */
	}
	netipc_set_seqid(dp, seqid);
	netipc_send_dp(remote, dp);
#if	iPSC386 || iPSC860
		netipc_called_here(__FILE__, __LINE__, "netipc_send_new (leave)");
#endif	iPSC386 || iPSC860
	return TRUE;
}

/*
 * Called from lower level when we have to retransmit something that
 * we have already sent.
 */
netipc_send_old(packetid, seqid)
	unsigned long packetid;
	unsigned long seqid;
{
	netipc_packet_t dp;

	dp = (netipc_packet_t) packetid;
	assert(dp);
	netipc_set_seqid(dp, seqid);
	netipc_send_dp(dp->dp_remote, dp);
}

netipc_packet_t	netipc_packet_list = NETIPC_PACKET_NULL;
int		netipc_packet_count = 0;

netipc_packet_t netipc_continuing_packet_list = NETIPC_PACKET_NULL;

netipc_packet_t
netipc_packet_allocate()
{
	netipc_packet_t dp;

	assert(netipc_locked());
	dp = netipc_packet_list;
	if (dp != NETIPC_PACKET_NULL) {
		netipc_packet_list = dp->dp_next;
		netipc_packet_count--;
	}
#if	iPSC386 || iPSC860
	else {
		/*
		 * must be netipc_output_replenish() hasn't tried
		 * to grab it's 300 packets yet...
		 */
		netipc_packet_list_empty++;
		dp = (netipc_packet_t) zalloc(netipc_packet_zone);
	}
#endif	iPSC386 || iPSC860
	return dp;
}

void
netipc_packet_deallocate(dp)
	netipc_packet_t dp;
{
	assert(netipc_locked());

	dp->dp_next = netipc_packet_list;
	netipc_packet_list = dp;
	netipc_packet_count++;
}

/*
 * Currently requires a thread wakeup every VM_MAP_COPY_PAGE_LIST_MAX pages.
 * Does this matter? Can we do better?
 */
netipc_safe_vm_map_copy_invoke_cont(dp)
	netipc_packet_t dp;
{
	assert(netipc_locked());
	assert(! dp->dp_being_continued);
	dp->dp_being_continued = TRUE;
	printf3("netipc_safe_vm_map_copy_invoke_cont(dp=0x%x)\n", dp);
	dp->dp_next = netipc_continuing_packet_list;
	netipc_continuing_packet_list = dp;
	netipc_thread_wakeup();
}


#if	iPSC386 || iPSC860
netipc_vm_map_copy_cont_check(copy)
	vm_map_copy_t	copy;
{
	extern kern_return_t vm_map_copy_discard_cont();
	extern kern_return_t vm_map_copyin_page_list_cont();
	extern kern_return_t norma_deliver_page_continuation();

	if ((copy->cpy_cont != vm_map_copy_discard_cont) &&
	    (copy->cpy_cont != vm_map_copyin_page_list_cont) &&
	    (copy->cpy_cont != norma_deliver_page_continuation)) {
		printf("Unknown continuation: copy=%x, cpy_cont=%x\n",
			copy, copy->cpy_cont);
		assert(0);
	}
}
#endif	iPSC386 || iPSC860


netipc_vm_map_copy_invoke(dp)
	netipc_packet_t dp;
{
	kern_return_t kr;
	vm_map_copy_t old_copy, new_copy;

	/*
	 * Get the old copy object and save its npages value.
	 */
	assert(netipc_locked());
	printf3("netipc_vm_map_copy_invoke_cont(dp=0x%x)\n", dp);
	old_copy = dp->dp_copy;

	/*
	 * Unlock, and invoke the continuation.
	 * If the continuation succeeds, discard the old copy object, and lock.
	 * If it fails... not sure what to do.
	 */
	netipc_thread_unlock();
#if	iPSC386 || iPSC860
	netipc_vm_map_copy_cont_check(old_copy);
#endif	iPSC386 || iPSC860
	vm_map_copy_invoke_cont(old_copy, &new_copy, &kr);
	if (kr != KERN_SUCCESS) {
		/*
		 * XXX
		 * What do we do here?
		 * What should appear in the receiver's address space?
		 *
		 * Should we abort the send at this point?
		 * We cannot, really, since we let the sender
		 * continue... didn't we?
		 * I guess we shouldn't.
		 */
		netipc_thread_lock();
		panic("netipc_vm_map_copy_invoke: kr=%d%x\n", kr, kr);
		return;
	}
	vm_map_copy_discard(old_copy);
	netipc_thread_lock();

	/*
	 * The continuation invocation succeeded.
	 * Adjust page_list_base and reset being_continued flag.
	 */
	dp->dp_page_list_base = dp->dp_copy_index;
	dp->dp_copy = new_copy;
	dp->dp_being_continued = FALSE;
}

/*
 * XXX
 * Use the int type field to implement a linked list.
 *
 * XXX
 * It's really quite unfortunate to have to do a wakeup each time
 * we want to discard a copy. It would be much better for the sending
 * thread -- if he's still waiting -- to do the discard.
 * We could also check to see whether the pages were stolen, in
 * which case it's not as important to release the pages quickly.
 */
vm_map_copy_t	netipc_safe_vm_map_copy_discard_list = VM_MAP_COPY_NULL;

netipc_safe_vm_map_copy_discard(copy)
	vm_map_copy_t copy;
{
	assert(netipc_locked());
	assert(copy->type == VM_MAP_COPY_PAGE_LIST);
	copy->type = (int) netipc_safe_vm_map_copy_discard_list;
	netipc_safe_vm_map_copy_discard_list = copy;
	netipc_thread_wakeup();
}

ipc_kmsg_t netipc_safe_ikm_free_list = IKM_NULL;

netipc_safe_ikm_free(kmsg)
	ipc_kmsg_t kmsg;
{
	kmsg->ikm_next = netipc_safe_ikm_free_list;
	netipc_safe_ikm_free_list = kmsg;
}

netipc_output_replenish()
{
	assert(netipc_unlocked());
	while (netipc_continuing_packet_list != NETIPC_PACKET_NULL) {
		netipc_packet_t dp;

		netipc_thread_lock();
		dp = netipc_continuing_packet_list;
		netipc_continuing_packet_list = dp->dp_next;
		netipc_vm_map_copy_invoke(dp);
		printf3("netipc_replenish: send_page_with_c 0x%x\n", dp);
		netipc_start(dp->dp_remote);
		netipc_thread_unlock();
	}
	while (netipc_dead_port_list != IP_NULL) {
		ipc_port_t port;

		netipc_thread_lock();
		port = netipc_dead_port_list;
		netipc_dead_port_list = port->ip_norma_queue_next;
		netipc_thread_unlock();
		ip_reference(port);
		norma_ipc_dead_destination(port);
	}
	while (netipc_safe_vm_map_copy_discard_list != VM_MAP_COPY_NULL) {
		vm_map_copy_t copy;

		netipc_thread_lock();
		copy = netipc_safe_vm_map_copy_discard_list;
		netipc_safe_vm_map_copy_discard_list
		    = (vm_map_copy_t) copy->type;
		netipc_thread_unlock();
		copy->type = VM_MAP_COPY_PAGE_LIST;
		vm_map_copy_discard(copy);
	}
	while (netipc_safe_ikm_free_list != IKM_NULL) {
		ipc_kmsg_t kmsg;
		ipc_port_t dest;
		mach_msg_bits_t bits;

		/*
		 * Lock, grab kmsg, and grab dest and bits from kmsg
		 * before it is freed.
		 */
		netipc_thread_lock();
		kmsg = netipc_safe_ikm_free_list;
		netipc_safe_ikm_free_list = kmsg->ikm_next;
		dest = (ipc_port_t) kmsg->ikm_header.msgh_remote_port;
		bits = kmsg->ikm_header.msgh_bits;

		/*
		 * Free kmsg under lock or not, as appropriate. 
		 */
		if (kmsg->ikm_size == IKM_SIZE_NORMA) {
			netipc_page_put(kmsg->ikm_page);
			netipc_thread_unlock();
		} else {
			netipc_thread_unlock();
			if (ikm_cache() == IKM_NULL &&
			    kmsg->ikm_size == IKM_SAVED_KMSG_SIZE) {
				ikm_cache() = kmsg;
			} else {
				ikm_free(kmsg);
			}
		}
		/*
		 * Perform deferred copyout (including release) of dest.
		 */
		assert(dest->ip_references > 0);
		if (bits & MACH_MSGH_BITS_MIGRATED) {
			norma_ipc_send_migrating_dest(dest);
		} else {
			norma_ipc_send_dest(dest, MACH_MSGH_BITS_REMOTE(bits));
		}
	}
	while (netipc_packet_count < 300) { /* XXX ??? ever alloced at int? */
		netipc_packet_t dp;

		dp = (netipc_packet_t) zalloc(netipc_packet_zone);
		netipc_thread_lock();
		dp->dp_next = netipc_packet_list;
		netipc_packet_list = dp;
		netipc_packet_count++;
		netipc_thread_unlock();
	}
}

#include <mach_kdb.h>
#if	MACH_KDB

#define	printf	kdbprintf

/*
 *	Routine:	netipc_packet_print
 *	Purpose:
 *		Pretty-print a netipc packet for ddb.
 */

netipc_packet_print(dp)
	netipc_packet_t dp;
{
	extern int indent;

	if ((unsigned int) dp < MAX_NUM_NODES) {
		dp = netipc_packet[(unsigned int) dp];
		if (dp == NETIPC_PACKET_NULL) {
			printf("null netipc packet\n");
			return;
		}
	}
  
	printf("netipc packet 0x%x\n", dp);

	indent += 2;

	iprintf("type=%d", dp->dp_type);
	switch ((int) dp->dp_type) {
		case DP_TYPE_KMSG:
		printf("[kmsg]");
		break;

		case DP_TYPE_PAGE:
		printf("[page]");
		break;

		case DP_TYPE_KMSG_MORE:
		printf("[kmsg_more]");
		break;

		case DP_TYPE_OOL_PORTS:
		printf("[ool_ports]");
		break;

		default:
		printf("[bad type]");
		break;
	}
	printf(", remote=%d", dp->dp_remote);
	printf(", seqid=%d", dp->dp_seqid);
	printf(", first_seqid=%d", dp->dp_first_seqid);
	printf(", last_seqid=%d\n", dp->dp_last_seqid);

	iprintf("kmsg=0x%x", dp->dp_kmsg);
	printf(", offset=%d", dp->dp_offset);
	printf("[0x%x]", dp->dp_offset + (char *) dp->dp_kmsg);
	printf(", next=0x%x\n", dp->dp_next);

	iprintf("copy=0x%x", dp->dp_copy);
	printf(", index=%d", dp->dp_copy_index);
	printf(", npages=%d", dp->dp_copy_npages);
	printf(", base=%d", dp->dp_page_list_base);
	printf(", has_cont=%d", dp->dp_page_list_base);
	printf(", being_cont=%d\n", dp->dp_being_continued);

	indent -=2;
}
#endif	MACH_KDB
