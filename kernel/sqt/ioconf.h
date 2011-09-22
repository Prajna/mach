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
 * $Log:	ioconf.h,v $
 * Revision 2.4  93/03/11  14:05:29  danner
 * 	u_long -> u_int
 * 	[93/03/09            danner]
 * 
 * Revision 2.3  91/07/31  18:02:11  dbg
 * 	Changed copyright.
 * 	[91/07/31            dbg]
 * 
 * Revision 2.2  91/05/08  12:57:07  dbg
 * 	Adapted for pure Mach kernel.
 * 	[91/04/26  14:53:25  dbg]
 * 
 */

/*
 * $Header: ioconf.h,v 2.4 93/03/11 14:05:29 danner Exp $
 *
 * ioconf.h
 *	IO Configuration Definitions.
 *
 */

/*
 * Revision 1.1  89/07/05  13:15:33  kak
 * Initial revision
 * 
 */

#ifndef	_SQT_IOCONF_H_
#define	_SQT_IOCONF_H_

#include <sys/types.h>

/*
 * MBAd device configuration structure(s).
 */

/*
 * An array of mbad_conf's constitutes the configuration of MBAd
 * drivers and devices.
 */

struct	mbad_conf {
	struct	mbad_driver *mc_driver;		/* -> per-driver data */
	int		mc_nent;		/* # entries in mbad_dev[] */
	struct	mbad_dev *mc_dev;		/* array describing related HW */
};

/*
 * Each driver includes one or more mbad_driver struct's in its source
 * to define its characteristics.
 */

struct	mbad_driver {
	char		*mbd_name;		/* name, eg "xb" (no digit) */
	int		mbd_cflags;		/* per-driver flags (below) */
	int		(*mbd_probe)();		/* probe procedure */
	int		(*mbd_boot)();		/* boot procedure */
	int		(*mbd_intr)();		/* intr procedure */
};

/*
 * mbad_conf entry points at array of mbad_dev's; each mbad_dev structure
 * describes a single MBAd board.
 *
 * After probing, driver boot procedure is called with a pointer to an
 * array of these.
 */

struct	mbad_dev {
	int		md_idx;			/* mbad index; -1 == wildcard */
	u_int		md_csr;			/* command/status register */
	u_int		md_flags;		/* flags (arbitrary) */
	u_short		md_mapwant;		/* # map registers needed */
	u_char		md_bin;			/* interrupt bin # */
	u_char		md_level;		/* multibus interrupt level */
/*
 * The remaining fields are filled in when the configure() runs.
 * Initial implementation copies md_mapwant to md_nmaps.
 * md_vector of 1st mbad_dev in a set defines "base" vector.
 */
	struct	mb_desc	*md_desc;		/* -> host mb_desc structure */
	u_short		md_basemap;		/* 1st map register allocated */
	u_short		md_nmaps;		/* # maps allocated */
	u_char		md_vector;		/* assigned interrupt vector */
	u_char		md_alive;		/* is alive? */
};

/*
 * MBAd per-driver configuration flags (mbd_cflags).
 *
 * If no device on a MBAd uses c-list, then c-list is not mapped in the
 * MBAd, allowing more map registers to be used by other drivers.
 *
 * MBD_MONOP assigns a directed interrupt to the particular level (directed
 * to the booting processor) and insures the handling processor can't be
 * taken off-line.
 *
 * For MBD_HAS{INTR,PROBE,BOOT} the corresponding proc pointer is valid only
 * if the bit is on.  If !MBD_HASPROBE, we assume the thing is alive.
 */

#define	MBD_CLIST	0x00000001		/* uses c-list */
#define	MBD_MONOP	0x00000002		/* runs mono-processor */
#define	MBD_HASINTR	0x00000004		/* has interrupt proc */
#define	MBD_HASPROBE	0x00000008		/* has probe procedure */
#define	MBD_HASBOOT	0x00000010		/* has boot procedure */

#define	MBD_TYPICAL	(MBD_HASINTR|MBD_HASPROBE|MBD_HASBOOT)

/*
 * Structure passed to MBAd device's probe routine.
 */

struct	mbad_probe {
	struct	mb_desc	*mp_desc;		/* pointer to MBAd descriptor */
	u_int		mp_csr;			/* command/status address */
	u_int		mp_flags;		/* copy of md_flags */
};

/*
 * Drivers boot procedure is called as follows:
 *
 *	XXboot(nXX, md)
 *		int	nXX;
 *		struct	mbad_dev *md;
 *
 * `nXX' is the # of "XX" entries (contiguously) in the mbad_dev[] table,
 * and includes all XX's even those not alive.  `md' is a pointer to the
 * first "XX" entry in the mbad_dev[] array.  The configuration utilities
 * insure that all entries for a particular driver are contiguous in the
 * mbad_dev[] table.  The 'md_vector' in the 1st mbad_dev[] entry is the
 * 1st of the (contiguous) set for XX's (if XX's were configured to all
 * interrupt in same bin).
 */

/*
 * Pseudo-device configuration structure(s).
 */

struct	pseudo_dev {
	char		*pd_name;		/* name (eg, "pty") */
	u_int		pd_flags;		/* flags (arbitrary) */
	int		(*pd_boot)();		/* boot procedure */
};

/*
 * Configuration data declarations.
 *
 * The mbad_conf and pseudo_dev arrays are terminated by an entry with a
 * zero "name" pointer.
 *
 * These are generated in ioconf.c by configuration utilities.
 */

extern	struct	mbad_conf	mbad_conf[];	/* mbad configuration */
extern	struct	pseudo_dev	pseudo_dev[];	/* pseudo-devices */
extern	u_int			MBAd_IOwindow;	/* 1/4Meg for IO window */
extern	int			bin_intr[];	/* # intrs/bin needed for IO */

/*
 * Description of an scsi device.
 */

/*
 * An array of sec_conf's constitutes the configuration of scsi/ether
 * drivers and devices.
 */

struct	sec_conf {
	struct	sec_driver	*sec_driver;	/* -> per-driver data */
	int			sec_nent;	/* # entries in sec_dev[] */
	struct	sec_dev		*sec_dev;	/* array describing related HW */
};

/*
 * Each driver includes one or more sec_driver struct's in its source
 * to define its characteristics.
 * The probe procedure is called on a per device basis.
 * The boot procedure is called on a per device driver basis.
 * The interrupt procedure is used to initialize interupt vectors.
 */

struct	sec_driver {
	char		*sed_name;		/* name, eg "sd" (no digit) */
	u_char		sed_base_chan;		/* SEC base channel number */
	int		sed_cflags;		/* per-driver flags (below) */
	int		(*sed_probe)();		/* probe procedure */
	int		(*sed_boot)();		/* boot procedure */
	int		(*sed_intr)();		/* intr procedure */
};

#define	SED_HASINTR	0x00000001		/* has interrupt procedure */
#define	SED_HASPROBE	0x00000002		/* has probe procedure */
#define	SED_HASBOOT	0x00000004		/* has boot procedure */
#define	SED_IS_SCSI	0x00000008		/* control devs on the scsi bus */
#define	SED_MONOP	0x00000010		/* runs monoprocessor */

#define	SED_TYPICAL	(SED_HASINTR|SED_HASPROBE|SED_HASBOOT)

/*
 * sec_conf entry points at array of sec_dev's; each sec_dev structure
 * describes a single device on the scsi/ether board(s).
 *
 * After probing, driver boot procedure is called with a pointer to an
 * array of these.
 */
struct	sec_dev {
	u_int			sd_flags;	/* flags (arbitrary) */
	u_char			sd_bin;		/* interrupt bin number */
	short			sd_req_size;	/* size of the request queue */
	short			sd_doneq_size;	/* size of the done queue */
/*
 * The remaining fields are filled in when the configure() routine runs.
 * sd_vector of first sec_dev in a set defines "base" vector.
 */
	short			sd_sec_idx;	/* sec index; -1 == wildcard */
	short			sd_target;	/* target number */
	short			sd_unit;	/* unit number */
	struct	sec_desc	*sd_desc;	/* host controller descriptor */
	u_char			sd_chan;	/* channel number to interrupt */
	u_char			sd_vector;	/* assigned interrupt vector */
	u_char			sd_destslic;	/* id or group id to interrupt */
	u_char			sd_alive;	/* is the device alive? */
	struct sec_cib		*sd_cib;	/* device info cib */
	struct sec_progq	*sd_requestq;	/* device input q location */
	struct sec_progq	*sd_doneq;	/* device output q location */
};

/*
 * Structure passed to Sec device's probe routine.
 */

struct	sec_probe {
	struct	sec_desc	*secp_desc;	/* SEC controller descriptor */
	struct	sec_powerup	*secp_puq;	/* command/status queues, etc. */
	short			secp_target;	/* target number */
	short			secp_unit;	/* unit number */
	u_char			secp_chan;	/* channel number to interrupt */
	u_int			secp_flags;	/* copy of sec_flags */
};

/*
 * One of these exists per active SCSI/Ether Controller.
 * Filled in by conf_scsi().
 */
struct sec_desc {
	u_int			sec_diag_flags;		/* flags from powerup */
	short			sec_target_no;		/* host adapter target # */
	u_char			sec_slicaddr;		/* slic address of SEC */
	u_char			sec_ether_addr[6];	/* ether address */
	u_char			sec_is_cons;		/* is this the console? */
	struct sec_powerup	*sec_powerup;		/* initial powerup queues */
	u_short			sec_version;		/* firmware version */
};

/*
 * table of entry points for b8k controller software
 */

struct cntlrs {
	int	(*conf_b8k)();		/* to probe controllers */
	int	(*probe_b8k_devs)();	/* probe devices on controlles */
	int	(*b8k_map)();		/* initialize controller mappings */
};

#ifdef KERNEL
extern	struct	sec_desc	*SEC_desc;	/* array alloc'd by conf_scsi */
extern	int	NSEC;				/* # SEC's to map at boot */
extern	u_int	SECvec;				/* bit-vector of existing SEC's */
extern	struct	sec_conf sec_conf[];		/* descriptions set up by config(1) */

extern struct cntlrs b8k_cntlrs[];		/* table of controller functions */
#endif KERNEL

#endif	_SQT_IOCONF_H_
