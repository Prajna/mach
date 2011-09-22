/* 
 * Mach Operating System
 * Copyright (c) 1991 Carnegie Mellon University
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
 * $Log:	if_se.h,v $
 * Revision 2.4  93/03/10  11:30:42  danner
 * 	u_long -> u_int
 * 	[93/03/10            danner]
 * 
 * Revision 2.3  91/07/31  18:06:48  dbg
 * 	Changed copyright.
 * 	[91/07/31            dbg]
 * 
 * Revision 2.2  91/05/08  13:05:59  dbg
 * 	MACH_KERNEL conversion.
 * 	[90/10/05            dbg]
 * 
 */

/*
 * $Header: if_se.h,v 2.4 93/03/10 11:30:42 danner Exp $
 */

/*
 * Revision 1.1  89/07/05  13:18:32  kak
 * Initial revision
 * 
 */
#ifndef	_SQTSEC_IF_SE_H_
#define	_SQTSEC_IF_SE_H_

#include <sys/types.h>

#include <device/io_req.h>
#include <device/net_io.h>
#include <device/if_hdr.h>
#include <device/if_ether.h>

#include <sqt/mutex.h>
#include <sqtsec/sec.h>

/*
 * se_counts - statistics taken from the interface.
 */
struct se_counts {
	u_int	ec_rx_ovfl;
	u_int	ec_rx_crc;
	u_int	ec_rx_dribbles;
	u_int	ec_rx_short;
	u_int	ec_rx_good;
	u_int	ec_tx_unfl;
	u_int	ec_tx_coll;
	u_int	ec_tx_16xcoll;
	u_int	ec_tx_good;
};
/*
 * SEC-related types.
 *
 * Due to the SCSI-related interface to the Ether firmware,
 * it is helps to hide the interface from the driver.
 * Some day some of this should be made available for the other SEC
 * drivers but time does not permit that now.
 */

/*
 * sec_pq describes the complete state of a device program
 * queue for the SCSI/Ether controller.
 * For unknown reasons, the size of the queue is not included
 * in the standard SCSI/Ether interface.
 */

struct sec_pq {
	struct sec_progq	*sq_progq;
	u_short			sq_size;
};


/*
 * This type records the state of a queue of sec_iat structures.
 * These are used for both input and output of Ether packets.
 */

struct sec_iatq {
	struct sec_iat	*iq_iats;	/* ring of iats itself */
	u_short		iq_size;	/* number of entries in the array */
	u_short		iq_head;	/* index of next available iat */
};


#ifdef	MACH_KERNEL
/*
 * sec_msgq - type
 *
 * This structure records the state of a queue of message pointers,
 * just like the queue of IATs described above.
 * This is used in parallel with an IAT queue to handle
 * Ether input packets.
 */
struct sec_msgq {
	ipc_kmsg_t	*mq_msgs;
	unsigned short	mq_size;
	unsigned short	mq_head;
};
#else	MACH_KERNEL
/*
 * sec_mbufq - type.
 *
 * This type records the state of a queue of mbuf pointers,
 * much like the queue of iats described above.
 * This is used in parallel with an an iat queue to handle
 * Ether input packets.
 */

struct sec_mbufq {
	struct mbuf	**mq_mbufs;
	u_short		mq_size;
	u_short		mq_head;
};
#endif	MACH_KERNEL



/*
 * Ethernet software state per interface(one for each controller).
 * It contains 3 segments: details about the controller;
 * details about the input side; and details about the output side.
 * Locks exist on each of the separate segments, and should
 * be used to lock as little about the state as is necessary.
 *
 * It is assumed throughout that both input and output will be
 * locked whenever any of the controller details are changed.
 *
 * Locking rules to avoid deadlock are as follows:
 *	- Lock the output data before the controller data.
 *	- Lock the interrupt lock (output only) after the data.
 */

struct	se_state {
						/* Describing the controller: */
#ifdef	MACH_KERNEL
	struct ifnet		ss_if;		/* generic interface header */
	u_char			ss_addr[6];	/* Ethernet hardware address */
	u_char			ss_pad0[2];
#else	/* MACH_KERNEL */
	struct arpcom		ss_arp;		/* Ethernet common */
#endif	/* MACH_KERNEL */
#ifdef	MACH_KERNEL
	simple_lock_data_t	ss_lock;	/* mutex lock */
#else	/* MACH_KERNEL */
	lock_t			ss_lock;	/* mutex lock */
#endif	/* MACH_KERNEL */
	struct se_counts	ss_sum;		/* statistics summary */
	int			ss_scan_int;	/* stat scan interval */
	u_short			ss_ether_flags;	/* SETMODE flags */
	u_char			ss_slic;	/* slic address of SEC */
	u_char			ss_bin;		/* bin to intr SEC with */
	u_char			ss_alive:1,	/* controller alive? */
				ss_initted:1,	/* filled in? */
				ss_init_called:1;
						/* Describing the input half: */
#ifdef	MACH_KERNEL
	simple_lock_data_t	is_lock;	/* interrupt lock */
#else	/* MACH_KERNEL */
	lock_t			is_lock;	/* interrupt lock */
#endif	/* MACH_KERNEL */
	struct sec_cib		*is_cib;	/* input cib */
	int			is_status;	/* cib's status var */
	struct sec_pq		is_reqq;	/* request queue */
	struct sec_pq		is_doneq;	/* done queue */
	struct sec_iatq		is_iatq;	/* queue of iats */
#ifdef	MACH_KERNEL
	struct sec_msgq		is_msgq;	/* parallel net_msg queue */
#else	MACH_KERNEL
	struct sec_mbufq	is_mbufq;	/* parallel mbuf queue */
#endif	MACH_KERNEL
	u_char			is_initted:1;	/* filled in? */
						/* Describing the output half: */
#ifdef	MACH_KERNEL
	simple_lock_data_t	os_lock;	/* mutex lock */
#else	/* MACH_KERNEL */
	lock_t			os_lock;	/* mutex lock */
#endif	/* MACH_KERNEL */
	struct sec_cib		*os_cib;	/* output cib */
	int			os_status;	/* cib's status var */
	struct sec_pq		os_reqq;	/* request queue */
	struct sec_pq		os_doneq;	/* done queue */
	struct sec_iatq		os_iatq;	/* iats for output */
#ifdef	MACH_KERNEL
	io_req_t		os_pending;	/* io_req being written */
#else	MACH_KERNEL
	struct mbuf		*os_pending;	/* mbuf being written */
#endif	MACH_KERNEL
	struct sec_gmode	os_gmode;	/* temp for SINST_GETMODE */
	u_char			*os_buf;	/* buffer for Ether output */
	u_char			os_initted:1,	/* filled in */
				os_active:1;	/* output active */
	long			ss_pad;		/* 4-byte pad for bitfields */
};


#ifdef KERNEL

#define OS_BUF_SIZE		(se_mtu + 36)	/* sizeof *os_buf */

#ifdef	ns32000			/* no SLIC gates in SGS */
extern gate_t se_gate;		/* gate for locks */
#endif	ns32000

extern int se_watch_interval;	/* seconds between stats collection */
extern int se_write_iats;	/* number of IATs for Ether writes */
extern int se_bin;		/* bin number to interrupt SEC on mIntr */
extern short se_mtu;		/* max transfer unit on se interface */
#ifdef DEBUG
extern int se_ibug, se_obug;	/* debug flags */
#endif DEBUG
#endif KERNEL

#endif	/* _SQTSEC_IF_SE_H_ */
