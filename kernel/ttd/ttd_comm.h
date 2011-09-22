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
 * TTD Communications parsing code's header file.
 *
 * HISTORY:
 * $Log:	ttd_comm.h,v $
 * Revision 2.2  93/05/10  23:24:37  rvb
 * 	Checkin for MK80 branch.
 * 	[93/05/10  15:07:45  grm]
 * 
 * Revision 2.1.2.2  93/04/20  10:51:53  grm
 * 	Added support for multiple endian archs.  Changed some of the
 * 	types for a more universal protocol.  Moved some of the sln.h
 * 	code into here.
 * 	[93/04/20            grm]
 * 
 * Revision 2.1.2.1  93/03/03  14:35:40  grm
 * 	Changed the interface.  Version works.
 * 	[93/03/03            grm]
 * 
 * Revision 2.1.1.2  93/01/28  15:21:56  grm
 * 	Added prototypes.
 * 
 * Revision 2.1.1.1  92/09/15  18:27:37  grm
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

#ifndef	_TTD_COMM_H_
#define	_TTD_COMM_H_

#include <mach/boolean.h>
#include <mach/mach_types.h>
#include <device/if_ether.h>
#include <ttd/ttd_msg.h>


/* Packet-type codes in Ethernet packets, in network order.  */
#if	BYTE_MSF
#define ETHERTYPE_IP	0x0800
#define ETHERTYPE_ARP	0x0806
#define ETHERTYPE_RARP	0x8035
#else
#define ETHERTYPE_IP	0x0008  
#define ETHERTYPE_ARP	0x0608
#define ETHERTYPE_RARP	0x3580
#endif	/* BYTE_MSF */

/* Type/operation codes in ARP/RARP packets, in network order.  */
#if	BYTE_MSF
#define ARPTYPE_ETHER	0x0001
#define ARPOP_REQUEST	0x0001
#define ARPOP_REPLY	0x0002
#define RARPOP_REQUEST	0x0003
#define RARPOP_REPLY	0x0004
#else
#define ARPTYPE_ETHER	0x0100  
#define ARPOP_REQUEST	0x0100
#define ARPOP_REPLY	0x0200
#define RARPOP_REQUEST	0x0300
#define RARPOP_REPLY	0x0400
#endif	/* BYTE_MSF */


#define ETHER_HTYPE	1

/* UDP Port for TTD protocol, in network order.  */
#if	BYTE_MSF	
#define TTD_PORT 0x8765
#else
#define TTD_PORT 0x6587
#endif	/* BYTE_MSF */

/* 1-byte packet-type code in IP packets.  */
#define PROTOCOL_UDP 17

typedef unsigned char byte_t;
typedef unsigned short half_word_t;


/**************************************/
/*   Ethernet-related declarations    */
/**************************************/

struct ether_hardware_address { /* Ethernet address, in network order */
    unsigned char array[6];
};

#define ETHER_ADDRESS_EQ(x,y) ( \
       ((x)->array[0] == (y)->array[0]) && ((x)->array[1] == (y)->array[1]) \
    && ((x)->array[2] == (y)->array[2]) && ((x)->array[3] == (y)->array[3]) \
    && ((x)->array[4] == (y)->array[4]) && ((x)->array[5] == (y)->array[5]))

struct ethernet_header { /* header of an ethernet packet */
    struct ether_hardware_address dest_uid;
    struct ether_hardware_address srce_uid;
    short type;
};

#define MAX_ETHER_DATA (1600 - sizeof (struct ethernet_header))

typedef half_word_t protocol_t;

struct ttd_ether_header {
	struct ether_hardware_address	dest;
	struct ether_hardware_address	source;
	protocol_t			protocol;
};

#define ETHER_ADDRESS_LENGTH	sizeof(struct ether_hardware_address)
#define ETHER_HEADER_LENGTH	sizeof(struct ttd_ether_header)
#define MIN_PACKET 		60


/************************************/
/*     IP-related declarations      */
/************************************/

struct ip_address {
	unsigned char array[4];
};

#define IP_ADDRESS_LENGTH sizeof(struct ip_address)

struct ip_header {		/* Network order */
#if	BYTE_MSF
	byte_t			version :4;
	byte_t			header_words :4;
#else
	byte_t			header_words :4;
	byte_t			version :4;
#endif	/* BYTE_MSF */
	byte_t			type_of_service;
	half_word_t		length;
	half_word_t		id;
	half_word_t		fragment_stuff;
	byte_t			time_to_live;
	byte_t			protocol;
	half_word_t		header_checksum;
	struct ip_address	source;
	struct ip_address	dest;
};

#define MAX_IP_DATA (MAX_ETHER_DATA - sizeof(struct ip_header))

typedef union {
	struct ip_header hdr;
	unsigned short data[1];
     /*	Options opt;   -- CAUTION: variable amount of stuff in here!! */
     /* IPData data; */
} ip_packet_t;

#define OUT_IP_HEADER_BYTES sizeof(struct ip_header)     /* No options sent */
#define OUT_IP_HEADER_WORDS (OUT_IP_HEADER_BYTES / 4)


/******************************************/
/*     ARP/RARP-related declarations      */
/******************************************/

typedef byte_t length_byte_t;

struct arp_packet {	/* Network order, used for ARP */
	protocol_t     		hardware_addr_type;
	protocol_t     		protocol_addr_type;
	length_byte_t  		hardware_addr_length;
	length_byte_t  		protocol_addr_length;
	half_word_t    		arp_opcode;
	/* IPForward, IPReverse: */
	struct {
		struct ether_hardware_address	source_hardware_addr;
		struct ip_address		source_protocol_addr;
		struct ether_hardware_address	dest_hardware_addr;
		struct ip_address		dest_protocol_addr;
	} ip;
};

struct arpether_packet {
	struct ttd_ether_header	eh;
	struct arp_packet	arp;
};

typedef struct arpether_packet	*arpether_packet_t;
typedef struct arp_packet	*arp_packet_t;


/*************************************/
/*     UDP-related declarations      */
/*************************************/

struct udp_header {    /* Network order, direct from DMA */
	half_word_t	source_port;
	half_word_t	dest_port;
	half_word_t	length;
	half_word_t	checksum;
};

#define MAX_UDP_DATA (MAX_IP_DATA - sizeof(struct udp_header))

typedef byte_t udp_data_t[MAX_UDP_DATA];

struct udp_packet {
	struct udp_header hdr;
	udp_data_t data;
};

typedef struct udp_packet *udp_packet_t;

struct udp_pseudo_header {    /* All multi-byte fields in network order! */
	struct ip_address	source;
	struct ip_address	dest;
	byte_t     		zero;
	byte_t      		protocol;
	half_word_t  		udp_length;
};


/*************************************/
/*     BOOTP-related declarations    */
/*************************************/

struct bootp {
	u_char			bp_op;		/* packet opcode type */
#define	BOOTREQUEST	1
#define	BOOTREPLY	2
	u_char			bp_htype;	/* hardware addr type */
	u_char			bp_hlen;	/* hardware addr length */
	u_char			bp_hops;	/* gateway hops */
	u_char			bp_xid[4];	/* transaction ID */
	u_short			bp_secs;	/* seconds since boot began */	
	u_short			bp_unused;
	struct ip_address	bp_ciaddr;	/* client IP address */
	struct ip_address	bp_yiaddr;	/* 'your' IP address */
	struct ip_address	bp_siaddr;	/* server IP address */
	struct ip_address	bp_giaddr;	/* gateway IP address */
	u_char			bp_chaddr[16];	/* client hardware address */
	u_char			bp_sname[64];	/* server host name */
	u_char			bp_file[128];	/* boot file name */
	u_char			bp_vend[64];	/* vendor-specific area */
};

typedef struct bootp * bootp_t;

struct udpip_packet {
	struct ip_header	ip_h;
	struct udp_header	udp_h;
};

struct recbootp {
	struct udpip_packet	ui;
	char			bpbuf[300];	/* hack for alignment */
	
};

struct sndbootp {
	struct ttd_ether_header	eh;		/* ether header */
	struct udpip_packet	ui;		/* ip/udp header */
	char			bpbuf[300];	/* hack for alignment */
};

typedef struct recbootp * recbootp_t;
typedef struct sndbootp * sndbootp_t;

#define BOOTPMSG_SIZE	sizeof(struct sndbootp)
#define BOOTPMAX_TRIES	5

/*
 * UDP port numbers, server and client.
 */
#define	UDP_BOOTPS		67
#define	UDP_BOOTPC		68

/*
 * Make available our own byteswapping routines. Got bit by
 * this once, but not twice dammit!
 */
#if	BYTE_MSF
#define	netswap_2_bytes(n)	n
#define	netswap_4_bytes(n)	n
#else
extern u_short netswap_2_bytes(u_short n);
extern uint32 netswap_4_bytes(uint32 n);
#endif	/* BYTE_MSF */

/*
 * Prototype definitions:
 */
extern boolean_t ttd_ip_bootp(void);
extern boolean_t kttd_valid_request(ipc_kmsg_t	request,
				    boolean_t	handle_arp,
				    vm_address_t *ttd_data,
				    natural_t	*request_length);
extern boolean_t kttd_get_request(vm_address_t	*ttd_request,
				  natural_t	*request_length);
extern ttd_reply_t skip_net_headers(char * ptr);
extern void complete_and_send_ttd_reply(natural_t kttd_reply_length);

#endif	/* _TTD_COMM_H_ */
