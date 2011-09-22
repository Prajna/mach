/* 
 * Mach Operating System
 * Copyright (c) 1991,1992 Carnegie Mellon University
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
 * $Log:	ipc_ether.c,v $
 * Revision 2.9  93/05/15  19:33:40  mrt
 * 	machparam.h -> machspl.h
 * 
 * Revision 2.8  93/01/14  17:53:37  danner
 * 	64bit cleanup. Proper spl typing.
 * 	[92/12/01            af]
 * 
 * Revision 2.7  92/03/10  16:27:23  jsb
 * 	Merged in norma branch changes as of NORMA_MK7.
 * 	[92/03/09  12:48:43  jsb]
 * 
 * Revision 2.5.3.3  92/01/21  21:51:03  jsb
 * 	Use a field in fragment header to record true length, since udp
 * 	length field can no longer be trusted due to ETHERMIN logic.
 * 	[92/01/21  21:30:47  jsb]
 * 
 * 	Don't send packets smaller than ETHERMIN.
 * 	[92/01/21  19:35:34  jsb]
 * 
 * 	Changed parameters to netipc_net_packet in preparation for saving
 * 	packets that arrive before a new recv_netvec has been supplied.
 * 	Added fragment information to debugging printfs.
 * 	[92/01/17  18:36:34  jsb]
 * 
 * 	Moved node_incarnation declaration from here to norma/ipc_net.c.
 * 	[92/01/17  14:38:17  jsb]
 * 
 * 	Fixed udp checksum bug. Drop packets whose length (as claimed by
 * 	driver) differs from length as recorded in udp header.
 * 	Set recv_netvec to zero before calling netipc_recv_intr
 * 	so that any new incoming packets won't corrupt netvec.
 * 	(This shouldn't happen anyway because interrupts will be blocked.)
 * 	Panic if fragment send completions occur out-of-order.
 * 	(This also shouldn't happen.)
 * 	[92/01/16  22:05:57  jsb]
 * 
 * 	Replaced small, incomprehensible, and incorrect loop in netipc_send
 * 	responsible for copying from fragments into vector elements with
 * 	comprehensible and correct code. De-linted.
 * 	[92/01/14  21:45:52  jsb]
 * 
 * 	Moved c_netipc_frag_drop* count definitions here from norma/ipc_net.c.
 * 	[92/01/10  20:37:07  jsb]
 * 
 * Revision 2.5.3.2  92/01/09  18:45:07  jsb
 * 	Added netipc_node_valid.
 * 	From durriya@ri.osf.org: added norma_ether_boot_info.
 * 	[92/01/08  16:45:45  jsb]
 * 
 * 	Added comment about reentrancy above netipc_send.
 * 	Replaced small, incomprehensible, and incorrect loop in netipc_send
 * 	responsible for copying from vector elements into fragments with
 * 	comprehensible and correct code.
 * 	[92/01/08  10:02:33  jsb]
 * 
 * 	Moved ntohl, etc. here from norma/ipc_net.c.
 * 	[92/01/04  22:11:30  jsb]
 * 
 * Revision 2.5.3.1  92/01/03  16:37:12  jsb
 * 	Made node_incarnation be unsigned long, and changed its name.
 * 	(It used to be node_instance. I also changed the log message below.)
 * 	[91/12/29  10:10:30  jsb]
 * 
 * 	Added code to set node_incarnation, using config info if available;
 * 	node_incarnation will allow norma ipc to cope with node crashes.
 * 	Fixed code that waits for nd table info to arrive from network;
 * 	it now detects and processes the first packet that arrives.
 * 	Split nd_reply routine into nd_reply, which calls either
 * 	nd_table_reply or nd_config_reply based on magic number.
 * 	Removed magic number checking code from the latter two routines,
 * 	and removed call to nd_config from netipc_net_packet.
 * 	[91/12/27  17:15:00  jsb]
 * 
 * 	Corrected log. Fixed merge error in nd_reply.
 * 	[91/12/24  14:34:55  jsb]
 * 
 * Revision 2.5  91/12/14  14:32:44  jsb
 * 	Picked up bootconfig code from sjs@osf.org.
 * 
 * Revision 2.4  91/11/14  16:52:20  rpd
 *	Added XLAS/XLA/XSA macros to solve alignment problems.
 * 	[91/11/00            jsb]
 * 
 * Revision 2.3  91/08/28  11:15:54  jsb
 * 	Generalized netipc_swap_device code.
 * 	[91/08/16  14:30:15  jsb]
 * 
 * 	Added fields to support nrp (node table resolution protocol),
 * 	allowing removal of hardwired table of node/ip/ether addresses.
 * 	[91/08/15  08:58:04  jsb]
 * 
 * Revision 2.2  91/08/03  18:19:14  jsb
 * 	Restructured to work with completely unmodified ethernet driver.
 * 	Added code to automatically find a usable ethernet device.
 * 	Removed NODE_BY_SUBNET case. Perform initialization earlier.
 * 	[91/08/02  14:21:08  jsb]
 * 
 * 	Use IO_LOANED technology.
 * 	[91/07/27  22:56:42  jsb]
 * 
 * 	Added fragmentation.
 * 	[91/07/27  18:54:37  jsb]
 * 
 * 	First checkin.
 * 	[91/07/24  23:38:03  jsb]
 * 
 */
/*
 *	File:	norma/ipc_ether.c
 *	Author:	Joseph S. Barrera III
 *	Date:	1991
 *
 *	Functions for NORMA_IPC over ethernet.
 */

#include <machine/machspl.h>
#include <vm/vm_kern.h>
#include <vm/vm_page.h>
#include <mach/vm_param.h>
#include <mach/port.h>
#include <mach/message.h>
#include <kern/assert.h>
#include <kern/host.h>
#include <kern/sched_prim.h>
#include <kern/ipc_sched.h>
#include <kern/ipc_kobject.h>
#include <kern/zalloc.h>
#include <ipc/ipc_mqueue.h>
#include <ipc/ipc_thread.h>
#include <ipc/ipc_kmsg.h>
#include <ipc/ipc_port.h>
#include <ipc/ipc_pset.h>
#include <ipc/ipc_space.h>
#include <ipc/ipc_marequest.h>
#include <device/io_req.h>
#include <device/if_hdr.h>
#include <device/net_io.h>
#include <device/net_status.h>
#include <norma/ipc_net.h>
#include <norma/ipc_ether.h>

#define	MAXFRAG			8
#define	FHP_OVERHEAD		sizeof(struct netipc_fragment_header)

typedef struct netipc_fragment_header	*netipc_fragment_header_t;

struct netipc_fragment_header {
	unsigned short	f_offset;
	unsigned char	f_id;
	unsigned char	f_last;
	unsigned long	f_length;
};

int NoiseEther = 0;

struct io_req			netipc_ior[MAXFRAG];
char				netipc_ether_buf[MAXFRAG * ETHERMTU];

struct netvec			*recv_netvec = 0;
int				recv_netvec_count;

/*
 * Quick-and-dirty macros for getting around alignment problems
 * XSA/XLA -> Transfer Short/Long in an Alignment friendly way.
 * XLAS is statement form of XLA which includes a temporary declaration,
 * allowing second parameter to be an arbitary (aligned) expression.
 */
#define	XSA(a,b,o)	(*(o+(short *)&(a)) = *(o+(short *)&(b)))
#define XLA(a,b)	(XSA((a),(b),0),XSA((a),(b),1))
#define XLAS(a,b)	{long _l = *(long *)(b); XLA((a),_l);}

#define	BCOPY(src, dst, len)	bcopy((char *)(src), (char *)(dst), (int)(len))
#define	HTONS(s)		htons((unsigned short)(s))
#define	NTOHS(s)		ntohs((unsigned short)(s))
#define	HTONL(l)		htonl((unsigned long)(l))
#define	NTOHL(l)		ntohl((unsigned long)(l))

/*
 * Definitions for ntohl, etc. for architectures that don't otherwise
 * provide them.
 */
#if	mips || lint
unsigned long
ntohl(x)
	register unsigned long x;
{
	return (((x & 0x000000ff) << 24)|
		((x & 0x0000ff00) <<  8)|
		((x & 0x00ff0000) >>  8)|
		((x & 0xff000000) >> 24));
}

unsigned long
htonl(x)
	register unsigned long x;
{
	return (((x & 0x000000ff) << 24)|
		((x & 0x0000ff00) <<  8)|
		((x & 0x00ff0000) >>  8)|
		((x & 0xff000000) >> 24));
}

unsigned short
ntohs(x)
	register unsigned short x;
{
	return (((x & 0x00ff) << 8)|
		((x & 0xff00) >> 8));
}

unsigned short
htons(x)
	register unsigned short x;
{
	return (((x & 0x00ff) << 8)|
		((x & 0xff00) >> 8));
}
#endif

/*
 * Netipc_recv_fragment_ip_src is 0 if no current frag.
 * If we get packet from ip_src which is not a completing frag,
 * we should ask for missing frags. This is therefor a sender timeout driven
 * mechanism, tied in with higher level timeouts.
 *
 * We should have several pages for reassembly. This should be tied in with
 * the ability to subsitute buffers of same size or smaller.
 * E.g. substituting small kmsgs for pages.
 *
 * For now, if a frag assembly is in progress, we drop everything
 * else aimed at us. An immediate refinement would be to have one
 * page reserved for reassembly, leaving another for recording nacks.
 *
 * Maybe upper level should be allowed to provide several receive buffers
 * and we find best fit. That would seem to minimize our knowledge
 * of what buffers are all about.
 */

int				netipc_recv_fragment_ip_src;
int				netipc_recv_fragment_f_id;

int netipc_ip_id = 1;

#define	NODE_INVALID	-1
int _node_self = NODE_INVALID;
extern unsigned long node_incarnation;
struct node_addr *node_self_addr;

#define	MAX_NODE_TABLE_SIZE	256

struct node_addr	node_table[MAX_NODE_TABLE_SIZE];
int			node_table_size = 0;

#define	NODE_ID(index)	(index)

struct node_addr *
node_addr_by_node(node)
	unsigned long node;
{
	return &node_table[node];
}

boolean_t
netipc_node_valid(node)
	unsigned long node;
{
	return (node < node_table_size);
}

node_self()
{
	if (_node_self == NODE_INVALID) {
		printf("... premature node_self() call\n");
		netipc_network_init();
	}
	if (_node_self == NODE_INVALID) {
		panic("node_self: I don't know what node I am\n");
	}
	return _node_self;
}

#if 1
/* debugging */
node_id(node_ether_addr)
	unsigned short *node_ether_addr;
{
	int i;

	if (node_table_size == 0) {
		panic("bad node_table_size\n");
	}
	for (i = 0; i < node_table_size; i++) {
		if (node_ether_addr[0] == node_table[i].node_ether_addr[0] &&
		    node_ether_addr[1] == node_table[i].node_ether_addr[1] &&
		    node_ether_addr[2] == node_table[i].node_ether_addr[2]) {
			return NODE_ID(i);
		}
	}
	return NODE_INVALID;
}

node_ip_id(ip_addr)
	unsigned long ip_addr;
{
	int i;

	if (node_table_size == 0) {
		panic("bad node_table_size\n");
	}
	for (i = 0; i < node_table_size; i++) {
		if (node_table[i].node_ip_addr == ip_addr) {
			return NODE_ID(i);
		}
	}
	return NODE_INVALID;
}
#endif

struct dev_ops *netipc_device;
int netipc_device_unit = 0;

#define	is_nulldev(func)	((func)==nodev || (func)==nulldev)

unsigned long eaddr_l[2];	/* XXX */

char netipc_boot_config[BOOT_CONFIG_SIZE];	/* boot configuration block */

char *
norma_ether_boot_info()
{
	return netipc_boot_config;
}

netipc_network_init()
{
	unsigned long count;
	int unit = netipc_device_unit;	/* XXX */
	struct dev_ops *dp;

	/*
	 * Return if already initialized.
	 */
	if (netipc_device) {
		return;
	}

	/*
	 * Look for a network device.
	 */
	dev_search(dp) {
		if (is_nulldev(dp->d_write) ||
		    is_nulldev(dp->d_getstat) ||
		    ! is_nulldev(dp->d_read)) {
			continue;
		}
		if ((*dp->d_open)(unit, 0) != 0) {
			/*
			 * Open failed. Try another one.
			 */
			continue;
		}
		if ((*dp->d_getstat)(unit, NET_ADDRESS, eaddr_l, &count) == 0){
			/*
			 * Success!
			 */
			netipc_device = dp;
			printf("netipc: using %s%d\n",
			       netipc_device->d_name,
			       netipc_device_unit);
			break;
		}
		/*
		 * Opened the wrong device. Close and try another one.
		 */
		if (dp->d_close) {
			(*dp->d_close)(unit);
		}
	}

	if (! netipc_device) {
		panic("netipc_network_init: could not find network device\n");
	}

	/*
	 * Set address and calculate node number.
	 */
	eaddr_l[0] = NTOHL(eaddr_l[0]);
	eaddr_l[1] = NTOHL(eaddr_l[1]);
	printf("netipc: waiting for node table\n");
	for (;;) {
		spl_t s;

		nd_request();
		s = splsched();
		if (node_self_addr) {
			splx(s);
			break;
		}
		assert_wait((vm_offset_t) &node_self_addr, TRUE);
		thread_set_timeout(5 * hz);
		thread_block((void (*)()) 0);
		if (node_self_addr) {
			splx(s);
			break;
		}
		splx(s);
	}
	printf("netipc: node %d internet 0x%x ethernet %s\n",
	       node_self(),
	       NTOHL(node_self_addr->node_ip_addr),
	       ether_sprintf(node_self_addr->node_ether_addr));
	if (netipc_boot_config[0] != '\0') {
		printf("netipc: boot config:\n%s", netipc_boot_config);
	}
	printf("netipc: incarnation %d\n", node_incarnation);
}

boolean_t nd_request_in_progress = FALSE;

request_ior_done()
{
	nd_request_in_progress = FALSE;
}

nd_request()
{
	netipc_ether_header_t ehp;
	netipc_udpip_header_t uhp;
	struct nd_request *ndrq;
	int i, total_length, udp_length;
	unsigned long checksum;

	if (nd_request_in_progress) {
		return;
	}
	nd_request_in_progress = TRUE;

	ehp  = (netipc_ether_header_t) netipc_ether_buf;
	uhp  = (netipc_udpip_header_t) (((char *) ehp) + EHLEN);
	ndrq = (struct nd_request *) (uhp + 1);
	
	udp_length = UDP_OVERHEAD + sizeof(struct nd_request);
	total_length = EHLEN + IP_OVERHEAD + udp_length;

	netipc_ior[0].io_count = total_length;
	netipc_ior[0].io_data = (char *) ehp;
	netipc_ior[0].io_done = request_ior_done;
	netipc_ior[0].io_op = IO_LOANED;
	
	ehp->e_ptype = HTONS(ETHERTYPE_IP);
	BCOPY(eaddr_l, ehp->e_src, 6);
	ehp->e_dest[0] = 0xffff;
	ehp->e_dest[1] = 0xffff;
	ehp->e_dest[2] = 0xffff;
	
	uhp->ip_version = IPVERSION;
	uhp->ip_header_length = 5;
	uhp->ip_type_of_service = 0;
	uhp->ip_total_length = HTONS(udp_length + IP_OVERHEAD);
	uhp->ip_id = HTONS(netipc_ip_id++);
	uhp->ip_fragment_offset = HTONS(0);
	uhp->ip_time_to_live = 0xff;
	uhp->ip_protocol = UDP_PROTOCOL;
	uhp->ip_checksum = 0;

#if 0
	uhp->ip_src = 0;
	uhp->ip_dst = HTONL(0xffffffff);
#else
	{
		int ip_src = 0;
		int ip_dst = HTONL(0xffffffff);
		
		XLA(uhp->ip_src, ip_src);
		XLA(uhp->ip_dst, ip_dst);
	}
#endif		

	for (checksum = i = 0; i < 10; i++) {
		checksum += ((unsigned short *) uhp)[i];
	}
	checksum = (checksum & 0xffff) + (checksum >> 16);
	checksum = (checksum & 0xffff) + (checksum >> 16);
	uhp->ip_checksum = (~checksum & 0xffff);

	uhp->udp_source_port = HTONS(IPPORT_NDREPLY);
	uhp->udp_dest_port = HTONS(IPPORT_NDREQUEST);
	uhp->udp_length = HTONS(udp_length);
	uhp->udp_checksum = 0;

#if 0
	ndrq->ndrq_magic = HTONL(NDRQ_MAGIC);
	ndrq->ndrq_start = HTONS(0);
#else
	{
		int ndrq_magic = HTONL(NDRQ_MAGIC);
		int ndrq_start = HTONS(0);

		XLA(ndrq->ndrq_magic, ndrq_magic);
		XLA(ndrq->ndrq_start, ndrq_start);
	}
#endif
	BCOPY(eaddr_l, ndrq->ndrq_eaddr, 6);

	/*
	 * Send the packet.
	 */
	(*netipc_device->d_write)(netipc_device_unit, &netipc_ior[0]);
}

nd_config_parse_incarnation(s)
	char *s;
{
	char *incarnation_env = "INCARNATION=";
	unsigned long incarnation = 0;

	if (strncmp(s, incarnation_env, strlen(incarnation_env))) {
		return;
	}
	for (s += strlen(incarnation_env); *s >= '0' && *s <= '9'; s++) {
		incarnation = 10 * incarnation + (*s - '0');
	}
	if (incarnation != 0) {
		node_incarnation = incarnation;
	}
}

nd_config_reply(uhp, count)
	netipc_udpip_header_t uhp;
	unsigned long count;
{
	struct nd_config *ndrc = (struct nd_config *) (uhp + 1);
	int bootconfigsize, bootmsgcnt, bootoffset;

	bootmsgcnt = NTOHS(ndrc->ndrc_msg_size);
	bootoffset = NTOHS(ndrc->ndrc_offset);
	if ((bootoffset + bootmsgcnt) > sizeof(netipc_boot_config)) {
		printf("nd_config: Too much config data!");
		return;
	}
	bootconfigsize = NTOHS(ndrc->ndrc_config_size);
	BCOPY(ndrc->ndrc_boot_data, &netipc_boot_config[bootoffset],
	       bootmsgcnt);
	netipc_boot_config[bootconfigsize] = '\0';
	if (bootoffset + bootmsgcnt >= bootconfigsize) {
		nd_config_parse_incarnation(netipc_boot_config);
	}
}

nd_table_reply(uhp, count)
	netipc_udpip_header_t uhp;
	unsigned long count;
{
	struct nd_reply *ndrp = (struct nd_reply *) (uhp + 1);
	int i;

	_node_self = NTOHS(ndrp->ndrp_node_self);
	node_table_size = NTOHS(ndrp->ndrp_table_size);
	if (node_table_size < 0 || node_table_size > MAX_NODE_TABLE_SIZE) {
		panic("nd_table_reply: bad table size %d\n", node_table_size);
	}
	for (i = 0; i < node_table_size; i++) {
		struct nd *nd = &ndrp->ndrp_table[i];
		struct node_addr *na = &node_table[i];

		if (! nd->nd_valid) {
			node_table[i].node_ip_addr = 0;
			bzero(node_table[i].node_ether_addr, 6);
			continue;
		} else {
			na->node_ip_addr = nd->nd_iaddr;
			BCOPY(nd->nd_eaddr, na->node_ether_addr, 6);
		}
	}
	node_self_addr = &node_table[_node_self];
	thread_wakeup((vm_offset_t) &node_self_addr);
}

nd_reply(uhp, count)
	netipc_udpip_header_t uhp;
	unsigned long count;
{
	struct nd_reply *ndrp = (struct nd_reply *) (uhp + 1);

	if (node_self_addr) {
		/*
		 * Table and config info already initialized.
		 */
		return;
	}
	if (ndrp->ndrp_magic == HTONL(NDRP_MAGIC)) {
		nd_table_reply(uhp, count);
	} else if (ndrp->ndrp_magic == HTONL(NDRC_MAGIC)) {
		nd_config_reply(uhp, count);
	} else {
		printf("nd_reply: bad magic\n");
	}
}

char netipc_swap_device[3] = "hd";
int netipc_swap_node_1 = 1;
int netipc_swap_node_2 = 1;
int netipc_swap_xor = 1;

char *
dev_forward_name(name, namebuf, namelen)
	char *name;
	char *namebuf;
	int namelen;
{
	if (netipc_swap_node_1 == netipc_swap_node_2) {
		return name;
	}
	if (name[0] == netipc_swap_device[0] &&
	    name[1] == netipc_swap_device[1]) {
		namebuf[0] = '<';
		if (node_self() == netipc_swap_node_1) {
			namebuf[1] = '0' + netipc_swap_node_2;
		} else {
			namebuf[1] = '0' + netipc_swap_node_1;
		}
		namebuf[2] = '>';
		namebuf[3] = name[0];			/* e.g. 'h' */
		namebuf[4] = name[1];			/* e.g. 'd' */
		namebuf[5] = name[2] ^ netipc_swap_xor;	/* major */
		namebuf[6] = name[3];			/* minor */
		namebuf[7] = '\0';
		printf("[ %s -> %s ]\n", name, namebuf);
		return namebuf;
	}
	return name;
}

int c_netipc_frag_drop0	= 0;
int c_netipc_frag_drop1	= 0;
int c_netipc_frag_drop2	= 0;
int c_netipc_frag_drop3	= 0;
int c_netipc_frag_drop4	= 0;

netipc_net_packet(kmsg, count)
	ipc_kmsg_t kmsg;
	unsigned long count;
{
	register struct netipc_ether_header *ehp;
	register struct netipc_udpip_header *uhp;

	ehp = (netipc_ether_header_t) &net_kmsg(kmsg)->header[0];
	uhp = (netipc_udpip_header_t) &net_kmsg(kmsg)->packet[sizeof(char *)];
	count = count - sizeof(struct packet_header);

	/*
	 * Only consider udp packets
	 */
	if (ehp->e_ptype != HTONS(ETHERTYPE_IP) ||
/*	    node_ip_id(uhp->ip_src) == -1 ||		ignore broadcasts? */
	    uhp->ip_protocol != UDP_PROTOCOL) {
		return FALSE;
	}

	/*
	 * Ignore packets whose length (as claimed by driver) differs from
	 * length as recorded in udp header.
	 */
	if (count != NTOHS(uhp->udp_length) + IP_OVERHEAD) {
		c_netipc_frag_drop0++;
		return FALSE;
	}

	/*
	 * Steal the kmsg iff we recognize the udp port.
	 */
	if (uhp->udp_dest_port == NETIPC_UDPPORT) {
		if (NoiseEther) {
			netipc_fragment_header_t fhp =
			    (netipc_fragment_header_t) (uhp + 1);
			printf("R s=%d.%x, d=%d.%x f=%04d/%d/%d len=%d\n",
			       node_ip_id(uhp->ip_src),
			       NTOHL(uhp->ip_src),
			       node_ip_id(uhp->ip_dst),
			       NTOHL(uhp->ip_dst),
			       NTOHS(fhp->f_offset),
			       fhp->f_id,
			       fhp->f_last,
			       NTOHS(uhp->udp_length) + EHLEN + IP_OVERHEAD);
		}
		netipc_recv_copy(kmsg);
		return TRUE;
	} else if (NTOHS(uhp->udp_dest_port) == IPPORT_NDREPLY) {
		nd_reply(uhp, count);
		net_kmsg_put(kmsg);
		return TRUE;
	} else {
		return FALSE;
	}
}

netipc_recv_copy(kmsg)
	ipc_kmsg_t kmsg;
{
	netipc_udpip_header_t uhp;
	netipc_fragment_header_t fhp;
	char *buf;
	unsigned long count;
	int i, offset;

	uhp = (netipc_udpip_header_t) &net_kmsg(kmsg)->packet[sizeof(char *)];
	fhp = (netipc_fragment_header_t) (uhp + 1);
	count = NTOHL(fhp->f_length);
	buf = (char *) (fhp + 1);
	offset = NTOHS(fhp->f_offset);

	if (recv_netvec == 0) {
		c_netipc_frag_drop1++;
		net_kmsg_put(kmsg);
		return;
	}

	if (netipc_recv_fragment_ip_src) {
		if (netipc_recv_fragment_ip_src != uhp->ip_src) {
			/*
			 * Someone has a frag in progress. Drop this packet.
			 */
			c_netipc_frag_drop2++;
			net_kmsg_put(kmsg);
			return;
		}
		if (fhp->f_last == 0) {
			/*
			 * New nonfragged message replaces old frag.
			 * XXX clearly suboptimal, see discussion above
			 * XXX assumes only one fragged message in transit
			 */
			c_netipc_frag_drop3++;
			netipc_recv_fragment_ip_src = 0;
		}
	}

	/*
	 * First check for non fragmented packets.
	 */
	if (fhp->f_last == 0) {
		for (i = 0; i < recv_netvec_count; i++) {
			if (count <= recv_netvec[i].size) {
				BCOPY(buf, recv_netvec[i].addr, count);
				recv_netvec[i].size = count;
				for (i++; i < recv_netvec_count; i++) {
					recv_netvec[i].size = 0;
				}
				break;
			}
			BCOPY(buf, recv_netvec[i].addr, recv_netvec[i].size);
			buf += recv_netvec[i].size;
			count -= recv_netvec[i].size;
		}
		recv_netvec = 0;
		netipc_recv_intr();
		net_kmsg_put(kmsg);
		return;
	}

	/*
	 * The packet was fragmented.
	 * If this is the first frag of a message, set up frag info.
	 */
	if (netipc_recv_fragment_ip_src == 0) {
		netipc_recv_fragment_ip_src = uhp->ip_src;
		netipc_recv_fragment_f_id = 0;
	}

	/*
	 * If we missed a frag, drop the whole thing!
	 */
	if (netipc_recv_fragment_f_id != fhp->f_id) {
		netipc_recv_fragment_ip_src = 0;
		c_netipc_frag_drop4++;
		net_kmsg_put(kmsg);
		return;
	}

	/*
	 * Skip to the right offset in the receive buffer,
	 * and copy it into the the appropriate vectors.
	 *
	 * First skip past all elements that we won't touch.
	 */
	for (i = 0; offset >= recv_netvec[i].size; i++) {
		offset -= recv_netvec[i].size;
	}
	assert(offset >= 0);
	assert(i < recv_netvec_count);

	/*
	 * A non-zero offset will affect at most one element.
	 * Deal with it now so that we can stop worrying about offset.
	 */
	if (offset > 0) {
		/*
		 * If this is the last element, and the last fragment,
		 * then set size.
		 */
		if (count <= recv_netvec[i].size - offset) {
			BCOPY(buf, recv_netvec[i].addr + offset, count);
			if (fhp->f_id == fhp->f_last) {
				recv_netvec[i].size = count + offset;
			}
			goto did_copy;
		}
		/*
		 * This is not the last element. Copy and continue.
		 */
		BCOPY(buf, recv_netvec[i].addr + offset,
		      recv_netvec[i].size - offset);
		buf += recv_netvec[i].size - offset;
		count -= recv_netvec[i].size - offset;
		i++;
	}

	/*
	 * Fill all the middle elements.
	 */
	for (; count > recv_netvec[i].size; i++) {
		BCOPY(buf, recv_netvec[i].addr, recv_netvec[i].size);
		buf += recv_netvec[i].size;
		count -= recv_netvec[i].size;
	}

	/*
	 * Fill the last element; if it's the last fragment, set size.
	 */
	BCOPY(buf, recv_netvec[i].addr, count);
	if (fhp->f_id == fhp->f_last) {
		recv_netvec[i].size = count;
	}

did_copy:
	/*
	 * If this is the last frag, hand it up, after setting the
	 * size for all remaining elements to zero.
	 * Otherwise, simply note that we received this frag.
	 */
	if (fhp->f_id == fhp->f_last) {
		for (i++; i < recv_netvec_count; i++) {
			recv_netvec[i].size = 0;
		}
		netipc_recv_fragment_ip_src = 0;
		recv_netvec = 0;
		netipc_recv_intr();
	} else {
		netipc_recv_fragment_f_id++;
	}
	net_kmsg_put(kmsg);
}

netipc_recv(vec, count)
	register struct netvec *vec;
	int count;
{
	recv_netvec = vec;
	recv_netvec_count = count;
}

/* (net as in usable, not as in network) */
#define	NETMTU	(ETHERMTU - EHLEN - IP_OVERHEAD - UDP_OVERHEAD - FHP_OVERHEAD)

int netipc_fragment_last_uncompleted = 0;

netipc_ether_send_complete(ior)
	register io_req_t ior;
{
	register netipc_udpip_header_t uhp;
	register netipc_fragment_header_t fhp;

	uhp = (netipc_udpip_header_t) (((char *) ior->io_data) + EHLEN);
	fhp = (netipc_fragment_header_t) (uhp + 1);
	if (fhp->f_id != netipc_fragment_last_uncompleted) {
		panic("netipc_ether_send_complete: f_id %d != %d!\n",
		      fhp->f_id, netipc_fragment_last_uncompleted);
	}
	netipc_fragment_last_uncompleted++;
	if (fhp->f_id == fhp->f_last) {
		netipc_send_intr();
	}
}

/*
 * This routine is not reentrant and is not expected to be.
 * norma/ipc_net.c uses netipc_sending to prevent multiple entrances.
 * If we have a reentrant form, we can upcall on netipc_send_intr
 * to indicate that we can accept another netipc_send call.
 */
netipc_send(remote, vec, count)
	unsigned long remote;
	register struct netvec *vec;
	unsigned int count;
{
	int i, f, frag_count, data_len;
	extern struct node_addr *node_self_addr, *node_addr_by_node();
	struct node_addr *node_dest_addr;
	char *buf;
	netipc_ether_header_t ehp;
	netipc_udpip_header_t uhp;
	netipc_fragment_header_t fhp;
	int total_length, udp_length;

	/*
	 * Silently drop packets destined for invalid nodes.
	 * Sender should previously have called netipc_node_valid.
	 * XXX
	 * We should have sender check our return value.
	 * If this were done, we wouldn't have to call netipc_send_intr.
	 */
	if (! netipc_node_valid(remote)) {
		netipc_send_intr();
		return;
	}

	/*
	 * Compute total size and number of fragments
	 */
	data_len = 0;
	for (i = 0; i < count; i++) {
		data_len += vec[i].size;
	}
	frag_count = (data_len + NETMTU - 1) / NETMTU;
	if (frag_count > MAXFRAG) {
		panic("netipc_send: size %d: too many (%d) fragments\n",
		       data_len, frag_count);
	}

	/*
	 * Get destination address
	 */
	node_dest_addr = node_addr_by_node(remote);

	/*
	 * Construct headers for each fragment; copy data into a contiguous
	 * chunk (yuck). Use loaned ior's.
	 */
	netipc_fragment_last_uncompleted = 0;
	for (f = 0; f < frag_count; f++) {
		ehp = (netipc_ether_header_t) &netipc_ether_buf[f * ETHERMTU];
		uhp = (netipc_udpip_header_t) (((char *) ehp) + EHLEN);
		fhp = (netipc_fragment_header_t) (uhp + 1);
		
		if (data_len > NETMTU) {
			udp_length = ETHERMTU - EHLEN - IP_OVERHEAD;
			fhp->f_length = HTONL(udp_length - UDP_OVERHEAD -
					      FHP_OVERHEAD);
			total_length = ETHERMTU;
			data_len -= NETMTU;
		} else {
			udp_length = UDP_OVERHEAD + FHP_OVERHEAD + data_len;
			fhp->f_length = HTONL(data_len);
			if (udp_length < ETHERMIN - IP_OVERHEAD) {
				udp_length = ETHERMIN - IP_OVERHEAD;
			}
			total_length = EHLEN + IP_OVERHEAD + udp_length;
			data_len = 0;
		}

		netipc_ior[f].io_count = total_length;
		netipc_ior[f].io_data = (char *) ehp;
		netipc_ior[f].io_done = netipc_ether_send_complete;
		netipc_ior[f].io_op = IO_LOANED;

		ehp->e_ptype = HTONS(ETHERTYPE_IP);
		ehp->e_src[0] = node_self_addr->node_ether_addr[0];
		ehp->e_src[1] = node_self_addr->node_ether_addr[1];
		ehp->e_src[2] = node_self_addr->node_ether_addr[2];
		ehp->e_dest[0] = node_dest_addr->node_ether_addr[0];
		ehp->e_dest[1] = node_dest_addr->node_ether_addr[1];
		ehp->e_dest[2] = node_dest_addr->node_ether_addr[2];
		
		uhp->ip_version = IPVERSION;
		uhp->ip_header_length = 5;
		uhp->ip_type_of_service = 0;
		uhp->ip_total_length = HTONS(total_length);
		uhp->ip_id = HTONS(netipc_ip_id++);
		uhp->ip_fragment_offset = HTONS(0);
		uhp->ip_time_to_live = 0xff;
		uhp->ip_protocol = UDP_PROTOCOL;
		uhp->ip_checksum = -1;
#if 0
		uhp->ip_src = node_self_addr->node_ip_addr;
		uhp->ip_dst = node_dest_addr->node_ip_addr;
#else
		XLA(uhp->ip_src, node_self_addr->node_ip_addr);
		XLA(uhp->ip_dst, node_dest_addr->node_ip_addr);
#endif		
		uhp->udp_source_port = HTONS(NETIPC_UDPPORT);
		uhp->udp_dest_port = HTONS(NETIPC_UDPPORT);
		uhp->udp_length = HTONS(udp_length);
		uhp->udp_checksum = -1;

		buf = (char *) (fhp + 1);

		if (frag_count == 0) {
			fhp->f_offset = 0;
			fhp->f_id = 0;
			fhp->f_last = 0;

			/*
			 * Everything will fit into one fragment,
			 * so just copy everything into it.
			 */
			for (i = 0; i < count; i++) {
				BCOPY(vec[i].addr, buf, vec[i].size);
				buf += vec[i].size;
			}
		} else {
			unsigned short offset = f * NETMTU;
			unsigned short remain = NETMTU;

			fhp->f_offset = HTONS(offset);
			fhp->f_id = f;
			fhp->f_last = frag_count - 1;

			/*
			 * First skip all elements which have
			 * already been completely sent.
			 */
			for (i = 0; offset >= vec[i].size; i++) {
				assert(i < count);
				offset -= vec[i].size;
			}
			assert((long) offset >= 0);
			assert((long) offset < vec[i].size);

			/*
			 * Offset is now the offset into the current
			 * element at which we begin copying.
			 * If this element is all we need, then copy
			 * as much of this element as will fit.
			 */
			assert(i < count);
			if (vec[i].size - offset >= remain) {
				BCOPY(vec[i].addr + offset, buf, remain);
				goto done_copying_into_fragments;
			}

			/*
			 * Copy all of this element, starting at offset.
			 * Move to the next element.
			 */
			BCOPY(vec[i].addr + offset, buf, vec[i].size - offset);
			buf += (vec[i].size - offset);
			remain -= (vec[i].size - offset);
			i++;

			/*
			 * For each remaining element: if it's all we need,
			 * then copy all that will fit and stop. Otherwise,
			 * copy all of it, and continue.
			 */
			for (; i < count; i++) {
				if (vec[i].size >= remain) {
					BCOPY(vec[i].addr, buf, remain);
					break;
				}
				BCOPY(vec[i].addr, buf, vec[i].size);
				buf += vec[i].size;
				remain -= vec[i].size;
			}
done_copying_into_fragments:
			;
		}

		if (NoiseEther) {
			printf("S s=%d.%x, d=%d.%x f=%04d/%d/%d len=%d\n",
			       node_ip_id(uhp->ip_src),
			       NTOHL(node_self_addr->node_ip_addr),
			       node_ip_id(uhp->ip_dst),
			       NTOHL(node_dest_addr->node_ip_addr),
			       NTOHS(fhp->f_offset),
			       fhp->f_id,
			       fhp->f_last,
			       udp_length + EHLEN + IP_OVERHEAD);
		}

		/*
		 * Send the packet.
		 */
		(*netipc_device->d_write)(netipc_device_unit, &netipc_ior[f]);
	}
}
