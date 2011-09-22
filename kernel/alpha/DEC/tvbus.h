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
 * $Log:	tvbus.h,v $
 * Revision 2.2  93/01/14  17:10:49  danner
 * 	Created, from the DEC specs:
 * 	"Alpha Demonstration Unit Specification"
 * 	V1.0, Aug 1990.
 * 	[92/06/02            af]
 * 
 */
/*
 *	File: tvbus.h
 * 	Author: Alessandro Forin, Carnegie Mellon University
 *	Date:	6/92
 *
 *	Definitions for the ADU TVbus.
 */

#ifndef	_TVBUS_H_
#define	_TVBUS_H_	1

/*
 * Max conceivable number of slots on the bus
 */
#define	TV_MAX_SLOTS		14

#define	TV_PHYS_START		0x300000000
#define	TV_SLOT_SIZE		0x010000000

/*
 * Slot information, from console
 */

struct tv_consmap_common {
	unsigned int	type;
	unsigned int	flags;
};

#define	TV_TYPE_EMPTY		0
#define	TV_TYPE_IO_MODULE	1
#define	TV_TYPE_MEMORY		2
#define	TV_TYPE_DC227		3
#define	TV_TYPE_DC228		4

#define	TV_FLAG_BROKEN		1
#define	TV_FLAG_CPU		2
#define	TV_FLAG_CONSOLE		4

typedef struct {
	struct tv_consmap_common	h;
	char		unused[512-8];
} tv_any_module_t;

typedef struct {
	struct tv_consmap_common	h;
	char		ether_address[8];
	int		scsi_id;
	char		unused[512-20];
} tv_io_module_t;

typedef struct {
	struct tv_consmap_common	h;
	unsigned long	base_address;
	unsigned long	interleave_position;
	char		unused[512-24];
} tv_memory_module_t;

typedef tv_any_module_t	tv_cpu_module_t;	/* nothing special */


#ifdef	KERNEL

extern	tv_any_module_t	*tv_slot_info;	/* dynarray of TV_MAX_SLOTS elems */

extern	void		tv_find_all_options ();

#endif	KERNEL


/*
 * Interrupt dispatching
 */

#define	TV_MAX_IPL20_CHAN	0x17	/* ((0x970-0x800)/0x10) */

/*
 * Defines for IO module. Should be separate files.
 */

#define	TV_IO_NI_BASE		0x2100
#define	TV_IO_SCSI_BASE		0x2120
#define	TV_IO_SL1_BASE		0x2140
#define	TV_IO_SL2_BASE		0x21a0
#define	TV_IO_NI_ICR		0x2200
#define	TV_IO_SCSI_ICR		0x2220
#define	TV_IO_SL1_ICR		0x2240
#define	TV_IO_SL2_ICR		0x22a0
#define	TV_IO_NI_S_BELL		0x6000
#define	TV_IO_SCSI_S_BELL	0x6020
#define	TV_IO_SL1_S_BELL	0x6040
#define	TV_IO_SL2_S_BELL	0x60a0

/* there are two banks of registers defined and used */

#define	TV_IO_0_BASE		0x0
#define	TV_IO_1_BASE		0x400400

/* Serial line */

#define	SL_NLINES	2

#define	SL_ICR_RE		0x1
#define	SL_ICR_TE		0x2
#define	SL_ICR_IRQ_NODE		0x3c
#define	SL_ICR_RIRQ_CHAN	0x7c0
#define	SL_ICR_TIRQ_CHAN	0xf800

typedef struct {
	unsigned int	rxsize;
	unsigned int	txsize;
	unsigned int	rxui;
	unsigned int	txli;
	unsigned int	txmode;
#define	SL_M_NORMAL	0
#define	SL_M_PAUSE	1
#define	SL_M_FLUSH	2
	char		pad0[12];
	volatile unsigned int	rxli;
	volatile unsigned int	txui;
	char		pad1[24];
#define	SL_BUF_SIZE	(4096-32)
	volatile char	rxbuf[SL_BUF_SIZE];
	char		txbuf[SL_BUF_SIZE];
} sl_ringbuffer_t;

/* see scsi_ADU.h for the SCSI defs */

/* ethernet */

#define	SE_ICR_RE		0x1
#define	SE_ICR_TE		0x2
#define	SE_ICR_IRQ_NODE		0x3c
#define	SE_ICR_RIRQ_CHAN	0x7c0
#define	SE_ICR_TIRQ_CHAN	0xf800

#define	SE_NRINGS		64

typedef struct {
	struct xmt_ring_t {
		volatile unsigned int	flag;
		volatile unsigned int	status;
		int			ncol;
		char			pad0[20];
		vm_offset_t		bufp;	/* +32 */
		int			ntbuf;
		char			pad1[20];
	} xmt_ring[SE_NRINGS];

	struct rcv_ring_t {
		volatile unsigned int	flag;
		volatile unsigned int	status;
		int			nrbuf;
		char			pad0[20];
		vm_offset_t		bufp;
		char			pad1[24];
	} rcv_ring[SE_NRINGS];

} se_ringbuffer_t;

typedef struct {
		volatile unsigned int	flag;
		volatile unsigned int	status;
		char			pad0[24];
		natural_t		rxmode;
		unsigned char		rxaddr[8];
		unsigned char		lamask[8];
		char			pad1[8];
} se_init_packet_t;

/* values for the flag fields */
#define	SE_F_EMPTY		0
#define	SE_F_XMT_PKT		1
#define	SE_F_INIT_PKT		2
#define	SE_F_RCV_PKT		3

/* values for the status field */
#define	SE_S_OK			0
#define	SE_S_REJECT		1
#define	SE_S_COLLISION		2
#define	SE_S_FRAMING		3
#define	SE_S_CRC		4
#define	SE_S_FIRMWARE		5

/* values for the rxmode field */
#define	SE_R_NORMAL		0
#define	SE_R_PROMISC		1

#endif	_TVBUS_H_
