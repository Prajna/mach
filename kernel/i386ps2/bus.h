/* 
 * Mach Operating System
 * Copyright (c) 1991,1990,1989 Carnegie Mellon University
 * Copyright (c) 1991 IBM Corporation 
 * All Rights Reserved.
 * 
 * Permission to use, copy, modify and distribute this software and its
 * documentation is hereby granted, provided that both the copyright
 * notice and this permission notice appear in all copies of the
 * software, derivative works or modified versions, and any portions
 * thereof, and that both notices appear in supporting documentation,
 * and that the name IBM not be used in advertising or publicity 
 * pertaining to distribution of the software without specific, written
 * prior permission.
 * 
 * CARNEGIE MELLON AND IBM ALLOW FREE USE OF THIS SOFTWARE IN ITS "AS IS"
 * CONDITION.  CARNEGIE MELLON AND IBM DISCLAIM ANY LIABILITY OF ANY KIND FOR
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
 * $Log:	bus.h,v $
 * Revision 2.3  93/03/11  14:08:48  danner
 * 	u_long -> u_int
 * 	[93/03/09            danner]
 * 
 * 	[93/03/09            danner]
 * 
 * Revision 2.2  93/02/04  07:59:01  danner
 * 	Integrate PS2 code from IBM.
 * 	[93/01/18            prithvi]
 * 
 */
/*
 * Copyright (c) 1982, 1986 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 *
 *	@(#)ubavar.h	7.1 (Berkeley) 6/5/86
 */


/*
 * per-controller & driver definitions
 */

#ifndef ASSEMBLER
#include <sys/types.h>

#define i386_ctlr isa_ctlr	/* backwards compatibility */
#define i386_dev isa_dev	/* backwards compatibility */
#define i386_driver isa_driver	/* backwards compatibility */

/*
 * Per-controller structure.
 * (E.g. one for each disk and tape controller, and other things
 * which have slave-style devices).
 *
 */
struct isa_ctlr {
	struct	isa_driver *ctlr_driver;
	long	ctlr_ctlr;	/* controller index in driver */
	long	ctlr_alive;	/* controller exists */
	caddr_t	ctlr_addr;	/* csr address */
	long	ctlr_spl;	/* spl level set upon interrupt */
	long	ctlr_pic;	/* pic line for controller */
	int	(**ctlr_intr)();/* interrupt handler */
	caddr_t	ctlr_start;	/* start address in mem space */
	u_int	ctlr_len;	/* length of mem space used */
};

/*
 * Per ``device'' structure.
 * (Everything else is a ``device''.)
 *
 * If a controller has many drives attached, then there will
 * be several isa_dev structures associated
 *
 */
struct isa_dev {
	struct	isa_driver *dev_driver;
	long	dev_unit;	/* unit number on the system */
	long	dev_ctlr;	/* ctlr number; -1 if none */
	long	dev_slave;	/* slave on controller */
	long	dev_alive;	/* Was it found at config time? */
	caddr_t	dev_addr;	/* csr address */
	short	dev_spl;	/* spl level */
	long	dev_pic;	/* pic line for device */
	long	dev_dk;		/* if init 1 set to number for iostat */
	long	dev_flags;	/* parameter from system specification */
	int	(**dev_intr)();	/* interrupt handler(s) */
	caddr_t	dev_start;	/* start address in mem space */
	u_int	dev_len;	/* length of mem space used */
	long	dev_type;	/* driver specific type information */
/* this is the forward link in a list of devices on a controller */
	struct	isa_dev *dev_forw;
/* if the device is connected to a controller, this is the controller */
	struct	isa_ctlr *dev_mi;
};

/*
 * Per-driver structure.
 *
 * Each driver defines entries for a set of routines for use
 * at boot time by the autoconfig routines.
 */
struct isa_driver {
	int	(*driver_probe)();	/* see if a driver is really there */
	int	(*driver_slave)();	/* see if a slave is there */
	int	(*driver_attach)();	/* setup driver for a slave */
	char	*driver_dname;		/* name of a device */
	struct	isa_dev **driver_dinfo;/* backptrs to init structs */
	char	*driver_mname;		/* name of a controller */
	struct	isa_ctlr **driver_minfo;/* backpointers to init structs */
};

#define MAX_POS_SLOTS	8	/* max number of slots */
#define MAX_POS_BYTES	8	/* max number of bytes */
#define MAX_POS_INFO	6	/* max number of info bytes */

#define POS_DISABLE_VGA	0x94
#define POS_PORT	0x96
#define POS_ENABLE(slot) (0x08+(slot))
#define POS_DISABLE	0x00
#define POS_GET_DATA(slot) inb(0x100+(slot))

#define POS_ID_NONE	0xffff	/* no card present */

struct pos_info {
	union {
		u_short	un_pos_id;		/* the ID as a short */
		u_char	un_data[2];		/* first two bytes of data */
	} un;
	u_char	pos_info[MAX_POS_INFO];		/* the bytes from hardware */
} slots[MAX_POS_SLOTS];

#define pos_data un.un_data
#define pos_id un.un_pos_id

#endif
