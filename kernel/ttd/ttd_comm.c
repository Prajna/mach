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
 * TTD Communications parsing code for the kernel ttd server.
 *
 * HISTORY:
 * $Log:	ttd_comm.c,v $
 * Revision 2.2  93/05/10  23:24:28  rvb
 * 	Checkin for MK80 branch.
 * 	[93/05/10  15:05:52  grm]
 * 
 * Revision 2.1.2.2  93/04/20  10:51:43  grm
 * 	Changed for use with mips and machines of both endian types.
 * 	Changed many of the types so that the same protocol will be
 * 	decodable by many machine types.
 * 	[93/04/20            grm]
 * 
 * Revision 2.1.2.1  93/03/03  14:35:46  grm
 * 	Second version of code.  It works.
 * 	[93/03/03            grm]
 * 
 * Revision 2.1.1.10  93/01/22  15:51:24  grm
 * 	Added request pkt length checks.
 * 
 * Revision 2.1.1.9  93/01/21  13:02:00  grm
 * 	Changed to ansi prototypes.
 * 
 * Revision 2.1.1.8  92/10/23  21:16:53  grm
 * 	Fixed bug in kttd_valid_request so that it looks at correct
 * 	incoming packet.
 * 	[92/10/23            grm]
 * 
 * Revision 2.1.1.7  92/10/08  14:27:10  grm
 * 	Now sends and receives packets.
 * 	[92/10/08            grm]
 * 
 * Revision 2.1.1.6  92/10/01  15:35:50  grm
 * 	Restructuring of ttd code checkpoint.
 * 	[92/10/01            grm]
 * 
 * Revision 2.1.1.5  92/09/30  13:32:31  grm
 * 	Changed for use with Mach specific kttd routines (sync and async).
 * 	Added get_request, and valid_request.
 * 	[92/09/30            grm]
 * 
 * Revision 2.1.1.4  92/09/25  15:15:34  grm
 * 	checkpointing...
 * 	[92/09/25            grm]
 * 
 * Revision 2.1.1.3  92/09/21  13:22:17  grm
 * 	This version uses bootp, and responds to arp requests.
 * 	[92/09/21            grm]
 * 
 * Revision 2.1.1.2  92/09/15  18:27:57  grm
 * 	Checkpoint version with bootp working.
 * 
 * Revision 2.1.1.1  92/09/09  14:44:07  grm
 * 	Initial checkin.
 * 
 */
/***********************************************************
Copyright 1992 by Digital Equipment Corporation, Maynard, Massachusetts,

                        All Rights Reserved

Permission to use, copy, modify, and distribute this software and its 
documentation for any purpose and without fee is hereby granted, provided 
that the above copyright notice appear in all copies and that both that 
copyright notice and this permission notice appear in supporting 
documentation, and that the name of Digital not be used in advertising 
or publicity pertaining to distribution of the software without specific, 
written prior permission.  Digital makes no representations about the 
suitability of this software for any purpose.  It is provided "as is"
without express or implied warranty.

DIGITAL DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING
ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL
DIGITAL BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR
ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
SOFTWARE.

******************************************************************/

/*****************************************************/
/*                    ttd_comm.c                     */
/* Simple TTD/UDP/IP/ARP protocol package for lowttd */
/*****************************************************/

#include <device/if_ether.h>
#include <device/net_status.h>
#include <ttd/ttd_stub.h>
#include <ttd/ttd_comm.h>
#include <ttd/ttd_types.h>
#include <ttd/ttd_msg.h>
#include <ttd/ttd_debug.h>


static struct ether_hardware_address broadcast = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff};

static struct ip_address my_ip_addr;
static struct ip_address ip_broadcast = {0xff, 0xff, 0xff, 0xff};
static struct ip_address ip_undefined = {0x00, 0x00, 0x00, 0x00};

#define skip_pkt_header(pkt)	(arp_packet_t)((natural_t)pkt + sizeof(struct packet_header))

/*
 * My own damn byte swapping routines:
 */
#if	BYTE_MSF
#else
u_short netswap_2_bytes(u_short n)
{
	u_short	tmp0;

	tmp0 = (n << 8);
	tmp0 |= (n >> 8);
		
	return tmp0;
}

uint32 netswap_4_bytes(uint32 n)
{
	uint32 tmp0, tmp1;

	tmp0 = (n << 24) | (n >> 24);
	tmp1 = (n & 0xff00) << 8;
	tmp0 |= tmp1;
	tmp1 = (n >> 8) & 0xff00;
	return tmp0 | tmp1;
}
#endif	/* BYTE_MSF */

/***************************************/
/*        Checksum routines            */
/***************************************/

/*
 * finish_checksum converts a two's complement accumulation of
 * HalfWords into the one's complement checksum.
 */
static int finish_checksum(int w)
{
	if (w == 0)
		return 0xffff;
	while (w > 0xffff)
		w = (w & 0xffff) + (w >> 16);
	return w ^ 0xffff;
}

/*
 * checksum_ip_header is applied to an IP direct from the
 * network interface, before any byteswapping.
 * Note that this means that the addition is
 * performed with the bytes of each halfword
 * reversed, but this is OK due to the magic
 * nature of ones complement arithmetic!
 * A checksum in network order is returned.
 */
static int checksum_ip_header(struct ip_header *ip)
{
	half_word_t *wp;
	int all, i, ip_header_halfwords;

	all = - ip->header_checksum;     /* don't count checksum field */
	wp = (half_word_t *) ip;
	ip_header_halfwords = ip->header_words * 2;
	for (i = 0; i < ip_header_halfwords; i++)
		all += wp[i];
	return (finish_checksum(all));
}

static boolean_t ip_checksum_ok(struct ip_header *ip)
{
	int net_check;

	net_check = ip->header_checksum;
	return ((net_check == 0) || (checksum_ip_header(ip) == net_check));
}

static void set_ip_checksum(struct ip_header *ip)
{
	ip->header_checksum = checksum_ip_header(ip);
}

static void byteswap_ip_header(struct ip_header *ip_hdr)
{
	ip_hdr->length         	= netswap_2_bytes(ip_hdr->length);
	ip_hdr->id             	= netswap_2_bytes(ip_hdr->id);
	ip_hdr->fragment_stuff  = netswap_2_bytes(ip_hdr->fragment_stuff);
	ip_hdr->header_checksum = netswap_2_bytes(ip_hdr->header_checksum);
}

/*
 * Use this after checking the IP header checksum.
 * A checksum in network order is returned.
 */
static int checksum_udp(struct udp_header	*udp,
			struct udp_pseudo_header pseudo_hdr)
{
	half_word_t *wp;
	byte_t *bp;
	int all, i, udp_bytes, udp_halfwords;

	all = - udp->checksum;     /* don't count checksum field */
	wp = (half_word_t *) &pseudo_hdr;
	for (i = 0; i < sizeof(struct udp_pseudo_header) / sizeof(half_word_t); i++)
		all += wp[i];
	udp_bytes = netswap_2_bytes(udp->length);
	udp_halfwords = udp_bytes / 2;
	wp = (half_word_t *) udp;
	for (i = 0; i <= udp_halfwords - 1; i++)
		all += wp[i];
	if ((udp_bytes % 2) == 1) {
		bp = (byte_t *) wp;
		all += bp[udp_bytes - 1];
	}
	return (finish_checksum (all));
}

static boolean_t udp_checksum_ok(struct udp_header		*udp,
				 struct udp_pseudo_header	pseudo_hdr)
{
	int net_check;

	net_check = udp->checksum;
	return ((net_check == 0) || 
		(checksum_udp(udp, pseudo_hdr) == net_check));
}

static void set_udp_checksum(struct udp_header		*udp,
			     struct udp_pseudo_header	ph)
{
	udp->checksum = checksum_udp(udp, ph);
}

/*****************************************/
/*       Simple-minded ARP Server        */
/*****************************************/

static boolean_t eq_ip(struct ip_address a, struct ip_address b)
{
	int i;
	
	for (i = 0; i < IP_ADDRESS_LENGTH; i++)
		if (a.array[i] != b.array[i])
			return FALSE;
	return TRUE;
}

static foocount = 10;

void dump_arp_packet(struct ttd_ether_header * e, arp_packet_t p)
{
	printf("\neh.dest %x:%x:%x:%x:%x:%x, ",
	       e->dest.array[0],
	       e->dest.array[1],
	       e->dest.array[2],
	       e->dest.array[3],
	       e->dest.array[4],
	       e->dest.array[5]);
	printf("eh.source %x:%x:%x:%x:%x:%x, ",
	       e->source.array[0],
	       e->source.array[1],
	       e->source.array[2],
	       e->source.array[3],
	       e->source.array[4],
	       e->source.array[4]);
	printf("eh.protocol = %x, ", e->protocol);
	printf("arp.hat %x, ", p->hardware_addr_type);
	printf("arp.pat %x, ", p->protocol_addr_type);
	printf("arp.hal %x, ", p->hardware_addr_length);
	printf("arp.pal %x, ", p->protocol_addr_length);
	printf("arp.aop %x, ", p->arp_opcode);
	printf("arp.sha %x:%x:%x:%x:%x:%x",
	       p->ip.source_hardware_addr.array[0],
	       p->ip.source_hardware_addr.array[1],
	       p->ip.source_hardware_addr.array[2],
	       p->ip.source_hardware_addr.array[3],
	       p->ip.source_hardware_addr.array[4],
	       p->ip.source_hardware_addr.array[5]);
	printf("arp.spa %d.%d.%d.%d, ",
	       p->ip.source_protocol_addr.array[0],
	       p->ip.source_protocol_addr.array[1],
	       p->ip.source_protocol_addr.array[2],
	       p->ip.source_protocol_addr.array[3]);
	printf("arp.dha %x:%x:%x:%x:%x:%x",
	       p->ip.dest_hardware_addr.array[0],
	       p->ip.dest_hardware_addr.array[1],
	       p->ip.dest_hardware_addr.array[2],
	       p->ip.dest_hardware_addr.array[3],
	       p->ip.dest_hardware_addr.array[4],
	       p->ip.dest_hardware_addr.array[5]);
	printf("arp.dpa %d.%d.%d.%d, ",
	       p->ip.dest_protocol_addr.array[0],
	       p->ip.dest_protocol_addr.array[1],
	       p->ip.dest_protocol_addr.array[2],
	       p->ip.dest_protocol_addr.array[3]);

	if (!foocount--)
		while(1);
}

static void handle_arp_packet(int unit,
			      struct ttd_ether_header * ehp,
			      struct packet_header * pkt)
	
{
	arp_packet_t p;
	arpether_packet_t rpkt;

	/*
	 * Set to point at the separate arp part of the received message
	 */
	p = skip_pkt_header(pkt);
	
	/* ARP is simple; req is turned into reply and xmitted inline. */

	if (p->hardware_addr_type != ARPTYPE_ETHER) {	/* != 0x0100 */
		return;
	}
	if (p->protocol_addr_type != ETHERTYPE_IP) {	/* != 0x0008 */
		return;
	}
	if (p->hardware_addr_length != ETHER_ADDRESS_LENGTH) {	/* != 0x06 */
		return;
	}
	if (p->protocol_addr_length != IP_ADDRESS_LENGTH) {	/* != 0x04 */
		return;
	}
	if (p->arp_opcode != ARPOP_REQUEST) {	/* != 0x0100 */
		return;
	}
	if (!eq_ip(p->ip.dest_protocol_addr, my_ip_addr)) {
		return;
	}

	/*
	 * Fix up the reply message:
	 */

	rpkt = (arpether_packet_t)ttd_reply_msg;

	bzero(rpkt, MIN_PACKET);

	rpkt->eh.dest = ehp->source;
	rpkt->eh.source = ttd_host_ether_id;
	rpkt->eh.protocol = ETHERTYPE_ARP;

	rpkt->arp.hardware_addr_type = ARPTYPE_ETHER;
	rpkt->arp.protocol_addr_type = ETHERTYPE_IP;
	rpkt->arp.hardware_addr_length = ETHER_ADDRESS_LENGTH;
	rpkt->arp.protocol_addr_length = IP_ADDRESS_LENGTH;

	rpkt->arp.ip.dest_protocol_addr = p->ip.source_protocol_addr;
	rpkt->arp.ip.dest_hardware_addr = p->ip.source_hardware_addr;
	rpkt->arp.ip.source_protocol_addr = my_ip_addr;
	rpkt->arp.ip.source_hardware_addr = ttd_host_ether_id;
	rpkt->arp.arp_opcode = ARPOP_REPLY;

	/*
	 * Send a reply....
	 */

	ttd_send_packet(ttd_device_unit, rpkt, MIN_PACKET);
}

/*****************************************/
/*        Bootp Packet Handling          */
/*****************************************/

/*
 * Generate a ``random'' transaction id number.  It builds it out of the
 * hardware ethernet id.  It's not unique -- XXX
 */
static uint32 build_xid(void)
{
	natural_t ret;

	ret = ((ttd_host_ether_id.array[0] <<12) ||
	       (ttd_host_ether_id.array[5] <<8) ||
	       (ttd_host_ether_id.array[1] <<4) ||
	       ttd_host_ether_id.array[4]);

	ret ^= ((ttd_host_ether_id.array[2] <<12) ||
		(ttd_host_ether_id.array[3] <<4));

	return ret;
}

void dump_ipudpbootp(char * s, sndbootp_t ptr)
{
	recbootp_t	p = (recbootp_t)&ptr->ui;
	bootp_t		bp = (bootp_t)p->bpbuf;

	printf("%s",s);

	if (bp->bp_op == BOOTREQUEST) {
		printf(", bootrequest is ok\n");
		return;
	}else{
		printf("\n");
	}

	printf("UDP Header: source %d (%d)\n",p->ui.udp_h.source_port,
	       ((natural_t)&p->ui.udp_h.dest_port-(natural_t)&p->ui.udp_h.source_port));
	printf("dest %d, (%d)\n",p->ui.udp_h.dest_port, 
	       ((natural_t)&p->ui.udp_h.length - (natural_t)&p->ui.udp_h.dest_port));
	printf("length %d, (%d)\n",p->ui.udp_h.length, 
	       ((natural_t)&p->ui.udp_h.checksum - (natural_t)&p->ui.udp_h.length));
	printf("checksum %d, (%d)\n",p->ui.udp_h.checksum,
	       ((natural_t)&bp->bp_op - (natural_t)&p->ui.udp_h.checksum));
	printf("BOOTP Contents: bp_op %d, (%d)\n", bp->bp_op,
	       ((natural_t)&bp->bp_htype - (natural_t)&bp->bp_op));
	printf("bp_htype %d, (%d)\n", bp->bp_htype,
	       ((natural_t)&bp->bp_hlen - (natural_t)&bp->bp_htype));
	printf("bp_hlen %d, (%d)\n", bp->bp_hlen,
	       ((natural_t)&bp->bp_hops - (natural_t)&bp->bp_hlen));
	printf("bp_hops %d, (%d)\n", bp->bp_hops,
	       ((natural_t)&bp->bp_xid - (natural_t)&bp->bp_hops));
	printf("bp_xid %d, (%d)\n", bp->bp_xid,
	       ((natural_t)&bp->bp_secs - (natural_t)&bp->bp_xid));
	printf("bp_secs %d, (%d)\n", bp->bp_secs,
	       ((natural_t)&bp->bp_unused - (natural_t)&bp->bp_secs));
	printf("bp_unused %d, (%d)\n", bp->bp_unused,
	       ((natural_t)&bp->bp_ciaddr - (natural_t)&bp->bp_unused));
	printf("bp_ciaddr %d.%d.%d.%d, (%d)\n",
	       bp->bp_ciaddr.array[0],
	       bp->bp_ciaddr.array[1],
	       bp->bp_ciaddr.array[2],
	       bp->bp_ciaddr.array[3],
	       ((natural_t)&bp->bp_yiaddr - (natural_t)&bp->bp_ciaddr));
	printf("bp_yiaddr %d.%d.%d.%d, (%d)\n",
	       bp->bp_yiaddr.array[0],
	       bp->bp_yiaddr.array[1],
	       bp->bp_yiaddr.array[2],
	       bp->bp_yiaddr.array[3],
	       ((natural_t)&bp->bp_siaddr - (natural_t)&bp->bp_yiaddr));
	printf("bp_siaddr %d.%d.%d.%d, (%d)\n",
	       bp->bp_siaddr.array[0],
	       bp->bp_siaddr.array[1],
	       bp->bp_siaddr.array[2],
	       bp->bp_siaddr.array[3],
	       ((natural_t)&bp->bp_giaddr - (natural_t)&bp->bp_siaddr));
	printf("bp_giaddr %d.%d.%d.%d, (%d)\n",
	       bp->bp_giaddr.array[0],
	       bp->bp_giaddr.array[1],
	       bp->bp_giaddr.array[2],
	       bp->bp_giaddr.array[3],
	       ((natural_t)&bp->bp_chaddr - (natural_t)&bp->bp_giaddr));
	printf("bp_chaddr %x:%x:%x:%x:%x:%x, (%d)\n",
	       bp->bp_chaddr[0],
	       bp->bp_chaddr[1],
	       bp->bp_chaddr[2],
	       bp->bp_chaddr[3],
	       bp->bp_chaddr[4],
	       bp->bp_chaddr[5],
	       ((natural_t)&bp->bp_sname - (natural_t)&bp->bp_chaddr));

	while(1);
}

/*
 * Builds an ethernet packet that contains a valid bootp
 * request.  This consists of a valid ethernet header, valid
 * ip header, udp header, and bootp request record.
 */
static void build_bootp_packet(sndbootp_t buf, int size)
{
	bootp_t		bp;
	uint32		tmp;

	bp = (bootp_t)buf->bpbuf;

	/*
	 * Clean out the buffer.
	 */
	bzero(buf,size);

	/*
	 * Set up ethernet header:
	 */
	buf->eh.dest = broadcast;
	buf->eh.source = ttd_host_ether_id;
	buf->eh.protocol = ETHERTYPE_IP;

	/*
	 * Set up ip header:
	 */
	buf->ui.ip_h.header_checksum = 0;
	buf->ui.ip_h.id = 1;
	buf->ui.ip_h.fragment_stuff = 0;

	/* Don't know about these two!! XXX */
	buf->ui.ip_h.type_of_service = 0;
	buf->ui.ip_h.version = 4;

	buf->ui.ip_h.header_words = OUT_IP_HEADER_WORDS;
	buf->ui.ip_h.time_to_live = 0xff;

	buf->ui.ip_h.protocol =	PROTOCOL_UDP;
	buf->ui.ip_h.source =	ip_undefined;
	buf->ui.ip_h.dest =	ip_broadcast;
	buf->ui.ip_h.length =	sizeof(struct recbootp);

	/* Fix up for network byte order. */
	byteswap_ip_header(&buf->ui.ip_h);
	set_ip_checksum(&buf->ui.ip_h);

	/*
	 * Set up the udp header:
	 */
	buf->ui.udp_h.source_port =	netswap_2_bytes(UDP_BOOTPC);
	buf->ui.udp_h.dest_port =	netswap_2_bytes(UDP_BOOTPS);
	buf->ui.udp_h.length =		netswap_2_bytes(sizeof(struct udp_header)+
							sizeof(struct bootp));
	buf->ui.udp_h.checksum =	0;

	/*
	 * Set up bootp info:
	 */
	bp->bp_op =		BOOTREQUEST;
	bp->bp_htype =		ETHER_HTYPE;
	bp->bp_hlen =		ETHER_ADDRESS_LENGTH;
	tmp = netswap_4_bytes(build_xid());
	bp->bp_xid[0] =		tmp & 0xf;
	bp->bp_xid[1] =		(tmp & 0xf0) >> 8;
	bp->bp_xid[2] =		(tmp & 0xf00) >> 8;
	bp->bp_xid[3] =		(tmp & 0xf000) >> 8;
	bp->bp_chaddr[0] =	ttd_host_ether_id.array[0];
	bp->bp_chaddr[1] =	ttd_host_ether_id.array[1];
	bp->bp_chaddr[2] =	ttd_host_ether_id.array[2];
	bp->bp_chaddr[3] =	ttd_host_ether_id.array[3];
	bp->bp_chaddr[4] =	ttd_host_ether_id.array[4];
	bp->bp_chaddr[5] =	ttd_host_ether_id.array[5];
}

/*
 * Check an incoming message for a valid bootp reply.
 */
static boolean_t check_bootp_reply(void)
{
	recbootp_t			rbootp;
	struct ttd_ether_header *	ehp;
	bootp_t				bp;
	uint32				tmp;

	/*
	 * Set up the pointers to point at the separate ether and ip data
	 * in the netkmsg.
	 */
	ehp = (struct ttd_ether_header *)
		&((net_rcv_msg_t)&((ipc_kmsg_t)ttd_request_msg)->ikm_header)->header[0];
	rbootp = (recbootp_t)((natural_t)(&((net_rcv_msg_t)
				      &((ipc_kmsg_t)ttd_request_msg)->ikm_header)->packet[0])
			      +(natural_t)(sizeof(struct packet_header)));

	bp = (bootp_t)rbootp->bpbuf;

#if	DEBUG_ETHER_PACKETS
	printf("cBr.1 %x:%x:%x:%x:%x:%x %x:%x:%x:%x:%x:%x ",
	       ehp->dest.array[0],
	       ehp->dest.array[1],
	       ehp->dest.array[2],
	       ehp->dest.array[3],
	       ehp->dest.array[4],
	       ehp->dest.array[5],
	       ehp->source.array[0],
	       ehp->source.array[1],
	       ehp->source.array[2],
	       ehp->source.array[3],
	       ehp->source.array[4],
	       ehp->source.array[5]);
#endif	/* DEBUG_ETHER_PACKETS */

	/*
	 * Check the ethernet header for my destination address and
	 * the Ethernet IP protocol.
	 */
	if (!ETHER_ADDRESS_EQ(&ehp->dest, &ttd_host_ether_id))
		return FALSE;
	if (ehp->protocol != ETHERTYPE_IP)
		return FALSE;
	/*
	 * Got a packet for my machine.  Check the IP for datagram packet.
	 */

	if (!ip_checksum_ok(&rbootp->ui.ip_h))
		return FALSE;
	byteswap_ip_header(&rbootp->ui.ip_h);

	if (rbootp->ui.ip_h.protocol != PROTOCOL_UDP)
		return FALSE;

	/*
	 * It's a UDP datagram (maybe :-))
	 * Check for bootp udp ports, udp length, xid, etc...
	 */

	if (rbootp->ui.udp_h.source_port != netswap_2_bytes(UDP_BOOTPS))
		return FALSE;
	if (rbootp->ui.udp_h.dest_port != netswap_2_bytes(UDP_BOOTPC))
		return FALSE;
	if (netswap_2_bytes(rbootp->ui.udp_h.length) < 
	    (sizeof(struct udp_header) + sizeof(struct bootp)))
		return FALSE;

	if (bp->bp_op != BOOTREPLY)
		return FALSE;
	tmp = netswap_4_bytes(build_xid());
	if ((bp->bp_xid[0] != tmp & 0xf) &&
	    (bp->bp_xid[1] != (tmp & 0xf0) >> 8) &&
	    (bp->bp_xid[2] != (tmp & 0xf00) >> 8) &&
	    (bp->bp_xid[3] != (tmp & 0xf000) >> 8))
		return FALSE;

	if (bp->bp_htype != ETHER_HTYPE)
		return FALSE;
	if (bp->bp_hlen != ETHER_ADDRESS_LENGTH)
		return FALSE;

	my_ip_addr = bp->bp_yiaddr;
	return TRUE;
}

/*
 * Try to get my host ip via the bootp protocol.  This routine
 * sends a bootp packet and then waits for a valid bootp return
 * call.  The routine returns true upon success, false if no bootp
 * server responds within the timeout period.
 */
static boolean_t do_bootp(void)
{
	natural_t max_tries;
	natural_t backoff;
	natural_t recvd;

	/*
	 * Only send BOOTPMAX_TRIES bootp queries before giving up.
	 */
	for(max_tries = BOOTPMAX_TRIES, backoff = 4;
	    max_tries;
	    max_tries--, backoff = ((backoff < 60) ? (backoff*2) : 60)) {

		build_bootp_packet((sndbootp_t)ttd_request_msg, BOOTPMSG_SIZE);

		ttd_send_packet(ttd_device_unit, ttd_request_msg, BOOTPMSG_SIZE);

		for(recvd = 0;
		    recvd < backoff;
		    recvd++) {
			/*
			 * Clean up receive buffer and get a message
			 */
			bzero(ttd_request_msg, BOOTPMSG_SIZE);
			ttd_get_packet(ttd_device_unit);
			
			if (check_bootp_reply())
				return TRUE;
		}
	}
	return FALSE;
}

boolean_t ttd_ip_bootp(void)
{
	boolean_t ret;

	/*
	 * Turn on ttd so that ether drivers work with the polling routines.
	 */

	printf("Looking for bootp server...");
	kttd_active = MAX_KTTD_ACTIVE;
	ret = do_bootp();
	kttd_active = MIN_KTTD_ACTIVE;

	if(ret) {
		printf("found bootp server, ip address = %d.%d.%d.%d\n",
		    my_ip_addr.array[0],
		    my_ip_addr.array[1],
		    my_ip_addr.array[2],  
		    my_ip_addr.array[3]);
	}else{
		kttd_enabled = FALSE;
		printf("\ncouldn't find a bootp server.\n");
	}

	return ret;
}

/***************************************/
/*         IP Packet Handling          */
/***************************************/

static void pseudo_header(struct ip_header *ip,
			  struct udp_pseudo_header *pseudo_hdr)
{
	pseudo_hdr->source    = ip->source;
	pseudo_hdr->dest      = ip->dest;
	pseudo_hdr->zero      = 0;
	pseudo_hdr->protocol  = ip->protocol;
	/*  pseudo_hdr->udpLength = <set later>;  --  must be in network order */
}

void dump_ether_header(char * mess, struct ttd_ether_header * ehp)
{
	printf("%s dhost: %x:%x:%x:%x:%x:%x",
	       mess,
	       ehp->dest.array[0],
	       ehp->dest.array[1],
	       ehp->dest.array[2],
	       ehp->dest.array[3],
	       ehp->dest.array[4],
	       ehp->dest.array[5]);

	printf("  shost: %x:%x:%x:%x:%x:%x",
	       ehp->source.array[0],
	       ehp->source.array[1],
	       ehp->source.array[2],
	       ehp->source.array[3],
	       ehp->source.array[4],
	       ehp->source.array[5]);

	printf("  type = %x",ehp->protocol);
}

/*
 * New routines for kttd:
 */

/*
 * poll_request:
 *
 * Get a packet from the ethernet via polling.
 *
 */
static void kttd_poll_request(void)
{
	ttd_get_packet(ttd_device_unit);
}

static boolean_t valid_udp_packet(struct udp_packet	*udp,
				  struct udp_pseudo_header udp_phdr,
				  natural_t		udp_length,
				  vm_offset_t		*ttd_pkt)
{

	udp_phdr.udp_length = udp->hdr.length;	/* in network order */

	if (!udp_checksum_ok(&udp->hdr, udp_phdr))
		return FALSE;

	if (udp->hdr.dest_port != TTD_PORT)		/* in network order */
		return FALSE;

	if (netswap_2_bytes(udp->hdr.length) > udp_length) {
		if (kttd_debug)
			printf("INVALID UDP Header Length!!!\n");
		return FALSE;
	}

	*ttd_pkt = (vm_offset_t)&udp->data[0];

	return TRUE;
}

static boolean_t valid_ip_packet(ip_packet_t	*ip,
				 natural_t	ip_len,
				 struct udp_pseudo_header *udp_phdr)
{
	/*
	 * Make sure checksum adds up!
	 */
	if (!ip_checksum_ok (&ip->hdr))
		return FALSE;

	pseudo_header(&ip->hdr, udp_phdr); /* pseudo-header for UDP */

	byteswap_ip_header(&ip->hdr); /* Can't touch header fields before this! */

	/*
	 * IP's length is bad, throw packet away...
	 */
	if ((ip->hdr.length > ip_len) || (ip_len > MAX_ETHER_DATA)) {
		byteswap_ip_header(&ip->hdr);
		if (kttd_debug)
			printf("INVALID IP LENGTH!!! 0x%x:0x%x\n",
			       ip->hdr.length, ip_len);
		return FALSE;
	}
	
	/*
	 * TTD packets are type UDP, discard others...
	 */
	if (ip->hdr.protocol != PROTOCOL_UDP) {
		byteswap_ip_header(&ip->hdr);
		return FALSE;
	}

	return TRUE;

}

static boolean_t valid_ether_packet(struct ttd_ether_header	*ehp,
				    struct packet_header	*pkt,
				    boolean_t handle_arp)
{
	/*
	 * ignore broadcast replies
	 */
	if (ETHER_ADDRESS_EQ (&ehp->source, &broadcast)) {
		return FALSE;
	}

	/*
	 * If arp packet then check for broadcast request.
	 */
	if ((ehp->protocol == ETHERTYPE_ARP) && handle_arp) {
		handle_arp_packet(ttd_device_unit, ehp, pkt);
		return FALSE;
	}

	if (ehp->protocol != ETHERTYPE_IP) {
		return FALSE;
	}

	/*
	 * IP packet, means this might be a ttd packet.
	 *
	 * We aren't looking for broadcast ip dest though....
	 */

	if (ETHER_ADDRESS_EQ(&ehp->dest, &broadcast))
		return FALSE;

	/*
	 * Passes the ethernet header check...
	 */
	return TRUE;
}

/*
 * kttd_valid_request(request, handle_arp):
 *
 *  Given a pointer to an ipc_kmsg, determine whether it is a valid kttd
 * request packet.
 *
 *  Arp packets are handled here handle_arp is TRUE.
 *
 *  If the packet is a valid KTTD packet, the procedure sets the ttd_data
 * paramter to point to the start of the TTD message and returns TRUE.  If
 * it is not a valid ttd packet it returns FALSE.
 *
 */
boolean_t kttd_valid_request(ipc_kmsg_t	request,
			     boolean_t	handle_arp,
			     vm_offset_t *ttd_data,
			     natural_t *request_length)
{
	struct ttd_ether_header *ehp;
	struct packet_header	*pkt;

	ip_packet_t		*ip;
	natural_t		length;
	natural_t		ip_hsize;

	struct udp_packet	*udp_pkt;
	struct udp_pseudo_header udp_phdr;
#if	1
	int i;
	u_char *ptr;
#endif	1

	vm_offset_t	ttd_pkt;

	/*
	 * Check Ether packet:
	 */
	ehp = (struct ttd_ether_header *)
		&((net_rcv_msg_t)
		  &((ipc_kmsg_t)request)->ikm_header)->header[0];
	pkt = (struct packet_header *)
		&((net_rcv_msg_t)
		  &((ipc_kmsg_t)request)->ikm_header)->packet[0];

	if (!valid_ether_packet(ehp, pkt, handle_arp))
		return FALSE;

	/*
	 * Check IP Packet:
	 */
	length = pkt->length;
	ip = (ip_packet_t *)skip_pkt_header(pkt);
	
	if (!valid_ip_packet(ip, length, &udp_phdr))
		return FALSE;

	/*
	 * Check UDP Packet:
	 */

	length -= sizeof(struct ip_header);
	ip_hsize = ip->hdr.header_words * 4;
	udp_pkt = (udp_packet_t) &ip->data[ip_hsize / 2];

	if (!valid_udp_packet(udp_pkt, udp_phdr, length, ttd_data)) {
		byteswap_ip_header(&ip->hdr);	
		return FALSE;
	}

	/*
	 * Passed all of the tests, it's a TTD packet.
	 */

	/* ttd_data was set by valid_udp_packet */

	*request_length = (length - sizeof(struct udp_header) -
			   sizeof(struct packet_header));

#if	DEBUG_ETHER_PACKETS
	dump_ether_header("Ether header: ", ehp);
	printf("\nPacket contents: ");
	ptr = (u_char *)ip;
	for (i = 0; i < 100; i++) {
		printf("%x:",(u_char) *ptr);
		ptr++;
	}
	printf("\n");
#endif	/* DEBUG_ETHER_PACKETS */

	return TRUE;
}

/*
 * kttd_get_request:
 *
 *  This routines waits for a ttd request packet to arrive on the
 * ethernet.  It does a polling input for a packet, and then checks
 * the packet for a valid ttd request.  It loops until it receives
 * a valid ttd request.
 *
 * Note:  this routine only called from the synchronous ttd code,
 *        so arp requests are handled.
 *
 * We assume that the ttd driver support has been checked and is
 * present.
 */
boolean_t kttd_get_request(vm_offset_t	*ttd_request,
			   natural_t *request_length)
{
#if	VERBOSE
	if (kttd_debug)
		printf("kttd_get_request entered.\n");
#endif	/* VERBOSE */

	/*
	 * clean it out, just to be safe.
	 */
	bzero(ttd_request_msg, MAX_TTD_MSG_SIZE);

	for(;;) {
		kttd_poll_request();

#if	VERBOSE		
		if (kttd_debug)
			printf("-");
#endif	/* VERBOSE */

		if (kttd_valid_request((ipc_kmsg_t)ttd_request_msg,
				       TRUE,
				       (vm_offset_t *)ttd_request,
				       request_length))
			break;
	}

	return TRUE;
}

/*
 * Index into the reply.
 */
ttd_reply_t skip_net_headers(char * ptr)
{
	/*
	 * Skip ether header.
	 */
	ptr += sizeof (struct ttd_ether_header);
	
	/*
	 * Skip IP header.
	 *
	 * This is dangerous!!!  IP header length is assumed to
	 * be constant for this to work.  Make sure when we
	 * build the reply packet that we are setting the header
	 * words field correctly.
	 */
	ptr += OUT_IP_HEADER_BYTES;

	/*
	 * Skip UDP header.
	 */
	ptr += sizeof (struct udp_header);

	return (ttd_reply_t)ptr;
}

/*
 * Wrap the old build and finish routines into one build
 * routine.
 */
void complete_and_send_ttd_reply(natural_t kttd_reply_length)
{
	struct ttd_ether_header *req_ether_header;
	struct packet_header	*req_ether_pkt;
	struct ttd_ether_header	*rpy_ether_header;

	struct ip_header	*req_ip_header;

	struct udp_header	*req_udp_header;

	struct udp_header	*rpy_udp_header;
	struct ip_header	*rpy_ip_header;

	natural_t		tmp_length;
	struct udp_pseudo_header uph;

	ipc_kmsg_t		tmp_kmsg;

	tmp_kmsg = (ipc_kmsg_t)((kttd_current_kmsg == (ipc_kmsg_t)NULL) ?
				ttd_request_msg :
				kttd_current_kmsg);

	/*
	 * Build the ethernet part of the reply.
	 */

	req_ether_header = (struct ttd_ether_header *)
		&((net_rcv_msg_t)&(tmp_kmsg)->ikm_header)->header[0];
	req_ether_pkt = (struct packet_header *)
		&((net_rcv_msg_t)&(tmp_kmsg)->ikm_header)->packet[0];


	/*
	 * The ttd_reply_msg is just the ether/ip/udp/ttd, it doesn't
	 * have the kmsg header junk that the ttd_request_msg has.
	 */
	rpy_ether_header = (struct ttd_ether_header *)ttd_reply_msg;

	rpy_ether_header->source	= req_ether_header->dest;
	rpy_ether_header->dest		= req_ether_header->source;
	rpy_ether_header->protocol	= req_ether_header->protocol;

	/*
	 * Partially build the IP header.
	 *
	 * Note:  Everything at this point is NOT in network order.
	 *        The finish_ttd_build procedure must insert the
	 *        correct length, byteswap the reply header and 
	 *        generate the checksum and insert it.
	 */

	req_ip_header = (struct ip_header *)skip_pkt_header(req_ether_pkt);
	rpy_ip_header = (struct ip_header *)((natural_t)rpy_ether_header +
					     sizeof (struct ttd_ether_header));

	rpy_ip_header->header_words	= OUT_IP_HEADER_WORDS;
	rpy_ip_header->version		= req_ip_header->version;
	rpy_ip_header->type_of_service	= req_ip_header->type_of_service;
/*	rpy_ip_header->length 		= <set later>; */
	rpy_ip_header->fragment_stuff	= 0;
	rpy_ip_header->id		= 0;
/*	rpy_ip_header->header_checksum	= <set later>; */
	rpy_ip_header->protocol		= req_ip_header->protocol;
	rpy_ip_header->time_to_live	= 255;
	rpy_ip_header->source		= req_ip_header->dest;
	rpy_ip_header->dest		= req_ip_header->source;

	/*
	 * Partially build the UDP header.
	 *
	 * Note:  Same problem here as the IP layer.  We will still need
	 *        to set the length in the finish_ttd_build section along
	 *        with the checksum (if not using zero).
	 */

	req_udp_header = (struct udp_header *)((natural_t)req_ip_header +
					       ((natural_t)req_ip_header->header_words * 4));
	rpy_udp_header = (struct udp_header *)((natural_t)rpy_ip_header +
					       OUT_IP_HEADER_BYTES);

	rpy_udp_header->source_port	= req_udp_header->dest_port;
	rpy_udp_header->dest_port	= req_udp_header->source_port;
/*	rpy_udp_header->length		= <set later>; */
/*	rpy_udp_header->checksum	= <set later>; */

	/*
	 * Sanity check.  Make sure that the calculated ttd_reply_msg
	 * is the same one we just came up with!!!
	 */

	if (kttd_debug && ((natural_t) skip_net_headers(ttd_reply_msg) !=
			   ((natural_t)rpy_udp_header + (sizeof(struct udp_header))))) {
		printf("TTDComm.c: Woah!!! ttd_reply_msg is skewed!!\n");
	}
	    
#if	VERBOSE
	if (kttd_debug)
		printf("build_partial: ttd_reply should start at: 0x%x, ttd_kmsg 0x%x\n",
		       (natural_t)rpy_udp_header + sizeof(struct udp_header),
		       ttd_reply_msg);
#endif	/* VERBOSE */

	/*
	 * Fix up UDP length and checksum:
	 */

	tmp_length = kttd_reply_length + sizeof(struct udp_header);
	rpy_udp_header->length = netswap_2_bytes(tmp_length);

	uph.source = rpy_ip_header->source;
	uph.dest = rpy_ip_header->dest;
	uph.zero = 0;
	uph.protocol = rpy_ip_header->protocol;
	uph.udp_length = rpy_udp_header->length;

	set_udp_checksum(rpy_udp_header, uph);

	/*
	 * Fix up IP length and checksum:
	 */

	tmp_length += OUT_IP_HEADER_BYTES;
	rpy_ip_header->length = tmp_length;
	byteswap_ip_header(rpy_ip_header);
	set_ip_checksum(rpy_ip_header);

#if	DEBUG_ETHER_PACKETS
	if (kttd_debug) {
		int i;
		u_char * ptr;

		ptr = (u_char *)ttd_reply_msg;

		for (i = 0; i< 100; i++)
			printf("%x: ", *ptr++);

		printf("\nlength = %d",tmp_length);

	}
#endif	/* DEBUG_ETHER_PACKETS */

	/*
	 * We've built it, let's send it off!!!
	 */
	ttd_send_packet(ttd_device_unit, ttd_reply_msg,
			((MIN_PACKET > tmp_length + sizeof(struct ttd_ether_header)) ?
			  MIN_PACKET : tmp_length + sizeof(struct ttd_ether_header)));
}
