/* 
 * Mach Operating System
 * Copyright (c) 1991 Carnegie Mellon University
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
 * $Log:	ipc_ether.h,v $
 * Revision 2.4  91/12/14  14:33:00  jsb
 * 	Picked up bootconfig code from sjs@osf.org.
 * 
 * Revision 2.3  91/08/28  11:15:57  jsb
 * 	Added fields to support nrp (node table resolution protocol).
 * 	[91/08/15  08:50:29  jsb]
 * 
 * Revision 2.2  91/08/03  18:19:16  jsb
 * 	Made netipc_udpip_header field names more regular.
 * 	[91/07/27  18:59:59  jsb]
 * 
 * 	First checkin.
 * 	[91/07/24  23:37:33  jsb]
 * 
 */
/*
 *	File:	norma/ipc_ether.h
 *	Author:	Joseph S. Barrera III
 *	Date:	1990
 *
 *	Definitions for NORMA_IPC over ethernet.
 */

#include <device/if_ether.h>

#define	EHLEN		14
#define	ETHERTYPE_IP	0x0800
#define	IPVERSION	4
#define IP_OVERHEAD	20
#define UDP_PROTOCOL	17
#define UDP_OVERHEAD	8
#define	NETIPC_UDPPORT	0xffff

extern struct node_addr *node_self_addr;

typedef struct node_addr		*node_addr_t;
typedef struct netipc_ether_header	*netipc_ether_header_t;
typedef struct netipc_udpip_header	*netipc_udpip_header_t;

struct node_addr {
	unsigned short	node_ether_addr[3];	/* typedef this? */
	unsigned long	node_ip_addr;
};

struct netipc_ether_header {
	u_short	e_dest[3];
	u_short	e_src[3];
  	short	e_ptype;
};

struct netipc_udpip_header {
#if	BYTE_MSF
	u_char	ip_version:4,		/* version */
		ip_header_length:4;	/* header length */
#else	BYTE_MSF
	u_char	ip_header_length:4,	/* header length */
		ip_version:4;		/* version */
#endif	BYTE_MSF
	u_char	ip_type_of_service;	/* type of service */
	short	ip_total_length;	/* total length */
	u_short	ip_id;			/* identification */
	short	ip_fragment_offset;	/* fragment offset field */
#define	IP_DF 0x4000			/* dont fragment flag */
#define	IP_MF 0x2000			/* more fragments flag */
	u_char	ip_time_to_live;	/* time to live */
	u_char	ip_protocol;		/* protocol */
	u_short	ip_checksum;		/* checksum */
	u_long	ip_src;			/* source ip address */
	u_long	ip_dst; 		/* dest ip address */

	u_short	udp_source_port;	/* source port */
	u_short	udp_dest_port;		/* destination port */
	short	udp_length;		/* udp length */
	u_short	udp_checksum;		/* udp checksum */
};

#define	IPPORT_NDREQUEST	10067
#define	IPPORT_NDREPLY		10068

#define	NDRQ_MAGIC		0xfc0a8319
#define	NDRP_MAGIC		0xfc0a831a
#define	NDRC_MAGIC		0xfc0a831b
#define	BOOT_CONFIG_SIZE	2048		/* max size of boot params */
#define	BOOT_CONFIG_PCKT_SIZE	1400		/* max size of boot params */

struct nd {
	unsigned long	nd_iaddr;
	short		nd_valid;
	char		nd_eaddr[6];
};

struct nd_request {
	unsigned long	ndrq_magic;
	unsigned short	ndrq_start;
	char		ndrq_eaddr[6];
	char		ndrq_pad[2];
};

struct nd_reply {
	unsigned long	ndrp_magic;
	unsigned short	ndrp_node_self;
	unsigned short	ndrp_table_size;
	unsigned short	ndrp_start;
	unsigned short	ndrp_count;
	struct nd	ndrp_table[120];
};

struct nd_config {
	unsigned long	ndrc_magic;
	unsigned short	ndrc_config_size;
	unsigned short	ndrc_offset;
	unsigned short	ndrc_msg_size;
	char		ndrc_boot_data[BOOT_CONFIG_PCKT_SIZE];
};
