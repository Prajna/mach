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
 * CARNEGIE MELLON ALLOWS FREE USE OF THIS SOFTWARE IN ITS "AS-IS"
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
 * any improvements or extensions that they make and grant Carnegie Mellon the
 * the rights to redistribute these changes.
 */
/*
 * HISTORY
 * $Log:	scsi_ADU.h,v $
 * Revision 2.2  93/01/14  17:10:34  danner
 * 	Created, from the DEC specs:
 * 	"Alpha Demonstration Unit Specification"
 * 	V1.0, Aug 1990.
 * 	[92/06/02            af]
 * 
 * 	Created.
 * 	[92/12/10            af]
 * 
 */

/* SCSI */

#define	SZ_ICR_IE		0x1
#define	SZ_ICR_IRQ_NODE		0x1e
#define	SZ_ICR_IRQ_CHAN		0x3e0

typedef struct {
	volatile unsigned int	flag;
	char		pad0[28];
	natural_t	cmdtag;		/* +32 */
	unsigned int	target;
	unsigned int	bus;
	unsigned int	nmsg;
	unsigned int	ncmd;
	unsigned int	options;
	unsigned int	timeout;
	vm_offset_t	dataaddr;	/* +64 */
	unsigned int	ndata;
	char		pad1[20];
	natural_t	msg;		/* +96 */
	natural_t	cmd[3];
} sz_cmd_ring_t;

typedef struct {
	volatile unsigned int	flag;
	char		pad0[28];	/* +32 */
	natural_t	cmdtag;
	unsigned int	cstatus;
	unsigned int	ndata;
	unsigned int	nsstatus;
	char		pad1[12];
	natural_t	sstatus[8];	/* +64 */
} sz_msg_ring_t;

typedef struct {
	sz_cmd_ring_t	cmds[32];	/* 32*128=4096 */
	sz_msg_ring_t	msgs[32];	/* 32*128=4096 */
} sz_ringbuffer_t;

/* values for the flag field */
#define	SZ_F_EMPTY		0
#define	SZ_F_SEND_COMMAND	1
#define	SZ_F_COMMAND_COMPLETE	2

/* values for the options field */
#define	SZ_O_NORETRY		1
#define	SZ_O_NOSENSE		2

/* values for the cstatus field */
#define	SZ_S_OK			0	/* all is fine */
#define	SZ_S_SELECT		1	/* failure during select phase */
#define	SZ_S_REJECT		2	/* did not understand */
#define	SZ_S_TIMEOUT		3	/* command started, but. */
#define	SZ_S_BUS		4	/* arbitration/parity/?? */
#define	SZ_S_OVERRUN		5	/* too much data in */
#define	SZ_S_UNDERRUN		6	/* not enough data out */
#define	SZ_S_FIRMWARE		7	/* oops */

/* We'll use a separate, contiguous buffer for DMA.  Sigh. */
#define	SZ_MAX_DMA		(64*1024)
