/* 
 * Mach Operating System
 * Copyright (c) 1994-1989 Carnegie Mellon University
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
 * 11-Aug-94  David Golub (dbg) at Carnegie-Mellon University
 *	MK83-A
 *
 * Revision 2.10  93/11/17  16:10:43  dbg
 * 	Import mach/machine/vm_types.h for vm_offset_t.
 * 	[93/06/11            dbg]
 * 
 * $Log:	busses.h,v $
 * Revision 2.9  93/05/10  20:07:21  rvb
 * 	No more caddr_t.
 * 	[93/04/09            af]
 * 
 * Revision 2.8  93/01/14  17:15:13  danner
 * 	Added BUS_CTLR flag.  Use on machines that truly have bus
 * 	adaptors, and write a probe function for it.
 * 	This was the grand plan from the beginning, alpha now does it.
 * 	[92/11/30            af]
 * 
 * Revision 2.7  91/08/24  11:51:27  af
 * 	Mods to cope with the 386 PC/AT ISA bus.
 * 	[91/06/20            af]
 * 
 * Revision 2.7  91/06/19  11:46:25  rvb
 * 	File moved here from mips/PMAX since it tries to be generic;
 * 	it is used on the PMAX and the Vax3100.
 * 	[91/06/04            rvb]
 * 
 * Revision 2.6  91/05/14  17:32:30  mrt
 * 	Correcting copyright
 * 
 * Revision 2.5  91/02/05  17:47:09  mrt
 * 	Added author notices
 * 	[91/02/04  11:21:07  mrt]
 * 
 * 	Changed to use new Mach copyright
 * 	[91/02/02  12:24:40  mrt]
 * 
 * Revision 2.4  90/12/05  23:50:20  af
 * 
 * 
 * Revision 2.3  90/12/05  20:49:31  af
 * 	Made reentrant.
 * 	New flag defs for new TC autoconf code.  Defs for exported funcs.
 * 	[90/12/03  23:01:22  af]
 * 
 * Revision 2.2  90/08/07  22:21:08  rpd
 * 
 * 	Created.
 * 	[90/04/18            af]
 * 
 */
/*
 *	File: busses.h
 * 	Author: Alessandro Forin, Carnegie Mellon University
 *	Date:	4/90
 *
 *	Structures used by configuration routines to
 *	explore a given bus structure.
 */

#ifndef	_CHIPS_BUSSES_H_
#define	_CHIPS_BUSSES_H_

#include <mach/boolean.h>
#include <mach/machine/vm_types.h>

/*
 *
 * This is mildly modeled after the Unibus on Vaxen,
 * one of the most complicated bus structures.
 * Therefore, let's hope this can be done once and for all.
 *
 * At the bottom level there is a "bus_device", which
 * might exist in isolation (e.g. a clock on the CPU
 * board) or be a standard component of an architecture
 * (e.g. the bitmap display on some workstations).
 *
 * Disk devices and communication lines support multiple
 * units, hence the "bus_driver" structure which is more
 * flexible and allows probing and dynamic configuration
 * of the number and type of attached devices.
 *
 * At the top level there is a "bus_ctlr" structure, used
 * in systems where the I/O bus(ses) are separate from
 * the memory bus(ses), and/or when memory boards can be
 * added to the main bus (and they must be config-ed
 * and/or can interrupt the processor for ECC errors).
 *
 * The autoconfiguration process typically starts at
 * the top level and walks down tables that are
 * defined either in a generic file or are specially
 * created by config.
 */

/*
 * Per-controller structure.
 */
struct bus_ctlr {
	struct bus_driver  *driver;	/* myself, as a device */
	char		   *name;	/* readability */
	int		    unit;	/* index in driver */
	int		  (*intr)();	/* interrupt handler(s) */
	vm_offset_t	    address;	/* device virtual address */
	int		    am;		/* address modifier */
	vm_offset_t	    phys_address;/* device phys address */
	char		    adaptor;	/* slot where found */
	char		    alive;	/* probed successfully */
	char		    flags;	/* any special conditions */
	vm_offset_t	    sysdep;	/* On some systems, queue of
					 * operations in-progress */
	natural_t	    sysdep1;	/* System dependent */
};


/*
 * Per-``device'' structure
 */
struct bus_device {
	struct bus_driver  *driver;	/* autoconf info */
	char		   *name;	/* my name */
	int		    unit;
	int		  (*intr)();
	vm_offset_t	    address;	/* device address */
	int		    am;		/* address modifier */
	vm_offset_t	    phys_address;/* device phys address */
	char		    adaptor;
	char		    alive;
	char		    ctlr;
	char		    slave;
	int		    flags;
	struct bus_ctlr    *mi;		/* backpointer to controller */
	struct bus_device  *next;	/* optional chaining */
	vm_offset_t	    sysdep;	/* System dependent */
	natural_t	    sysdep1;	/* System dependent */
};

/*
 * General flag definitions
 */
#define BUS_INTR_B4_PROBE  0x01		/* enable interrupts before probe */
#define BUS_INTR_DISABLED  0x02		/* ignore all interrupts */
#define	BUS_CTLR	   0x04		/* descriptor for a bus adaptor */
#define BUS_XCLU	   0x80		/* want exclusive use of bdp's */

/*
 * Per-driver structure.
 *
 * Each bus driver defines entries for a set of routines
 * that are used at boot time by the configuration program.
 */
struct bus_driver {
	int	(*probe)(		/* see if the driver is there */
		    /*	vm_offset_t	address,
			struct bus_ctlr * */ );
	int	(*slave)(          	/* see if any slave is there */	
		    /*	struct bus_device *,
			vm_offset_t	  */ );
	void	(*attach)(		/* setup driver after probe */
		    /*	struct bus_device * */);
	int	(*dgo)();		/* start transfer */
	vm_offset_t *addr;		/* device csr addresses */
	char	*dname;			/* name of a device */
	struct	bus_device **dinfo;	/* backpointers to init structs */
	char	*mname;			/* name of a controller */
	struct	bus_ctlr **minfo;	/* backpointers to init structs */
	int	flags;
};

#ifdef	KERNEL
extern struct bus_ctlr		bus_master_init[];
extern struct bus_device	bus_device_init[];

extern boolean_t configure_bus_master(char *, vm_offset_t, vm_offset_t,
				      int, char * );
extern boolean_t configure_bus_device(char *, vm_offset_t, vm_offset_t,
				      int, char * );
#endif	/* KERNEL */


#endif	/* _CHIPS_BUSSES_H_ */
