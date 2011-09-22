/* 
 * Mach Operating System
 * Copyright (c) 1993-1989 Carnegie Mellon University
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
 * $Log:	scsi_defs.h,v $
 * Revision 2.16  93/08/03  12:34:39  mrt
 * 	Added hooks for multi-LUN devices.
 * 	Addeddefs for COMM devices.
 * 	[93/07/29  23:39:53  af]
 * 
 * 	Added rewriting of label where we found it.
 * 	[93/05/06  10:05:31  af]
 * 
 * Revision 2.15  93/05/10  21:23:07  rvb
 * 	Added rewriting of label where we found it.
 * 	[93/05/06  10:05:31  af]
 * 
 * Revision 2.14  93/03/26  18:01:32  mrt
 * 	Use a pointer (to a list of alternate operations) to
 * 	mark CDROM drives that are non-standard.
 * 	[93/03/22            af]
 * 
 * 	Command is unsigned.
 * 	[93/03/17            af]
 * 
 * Revision 2.13  93/03/09  10:58:18  danner
 * 	Merge botch, oops.  So I did enough to get a clean build
 * 	under GCC on alpha.
 * 	[93/03/07  13:24:00  af]
 * 
 * 	Added cdrom status keeping. Lint.
 * 	[93/03/06            af]
 * 
 * 	Change type of optimize to void in scsi_devsw_t to match usage.
 * 	[93/02/17            jeffreyh]
 * 
 * Revision 2.12  93/01/14  17:55:51  danner
 * 	Define SCSI_{OPTIMIZE,OPEN,CLOSE}_NULL. 
 * 	Expanded scsi_devsw_t function prototypes.
 * 	[93/01/14            danner]
 * 
 * Revision 2.11  92/05/21  17:24:24  jfriedl
 * 	Appended 'U' to constants that would otherwise be signed.
 * 	wakeup() is void.
 * 		Other changes to quiet gcc warnings.
 * 	[92/05/16            jfriedl]
 * 
 * Revision 2.10  92/02/23  22:44:55  elf
 * 	Changed unused field into masterno in target descriptor.
 * 	[92/02/22  19:31:54  af]
 * 
 * Revision 2.9  91/08/24  12:28:38  af
 * 	Added processor_type infos, definition of an opaque type,
 * 	multiP locking.
 * 	[91/08/02  03:55:05  af]
 * 
 * Revision 2.8  91/07/09  23:22:53  danner
 * 	Added include of <scsi/rz_labels.h>
 * 	[91/07/09  11:16:30  danner]
 * 
 * Revision 2.7  91/06/19  11:57:43  rvb
 * 	File moved here from mips/PMAX since it is now "MI" code, also
 * 	used by Vax3100 and soon -- the omron luna88k.
 * 	[91/06/04            rvb]
 * 
 * Revision 2.6  91/05/14  17:30:18  mrt
 * 	Correcting copyright
 * 
 * Revision 2.5  91/05/13  06:05:34  af
 * 	Made counters unsigned, added copy_count for use by HBAs
 * 	that do unlimited DMA via double buffering.  Made explicit
 * 	two padding bytes, and let them be HBA-specific (e.g. used
 * 	for odd-byte-boundary conditions on some).
 * 	Made max_dma_data unsigned, a value of -1 means unlimited.
 * 	Removed unsed residue field.
 * 
 * 	Defined tape-specific information fields to target structure.
 * 	Added tape-specific flags and flag for targets that require
 * 	the long form of various scsi commands.
 * 	Added disconnected-state information to target structure.
 * 	Added watchdog field to adapter structure.
 * 	[91/05/12  16:24:10  af]
 * 
 * Revision 2.4.1.2  91/04/05  13:13:29  af
 * 	Made counters unsigned, added copy_count for use by HBAs
 * 	that do unlimited DMA via double buffering.  Made explicit
 * 	two padding bytes, and let them be HBA-specific (e.g. used
 * 	for odd-byte-boundary conditions on some).
 * 	Made max_dma_data unsigned, a value of -1 means unlimited.
 * 	Removed unsed residue field.
 * 
 * Revision 2.4.1.1  91/03/29  17:06:09  af
 * 	Defined tape-specific information fields to target structure.
 * 	Added tape-specific flags and flag for targets that require
 * 	the long form of various scsi commands.
 * 	Added disconnected-state information to target structure.
 * 	Added watchdog field to adapter structure.
 * 
 * Revision 2.4  91/02/05  17:45:43  mrt
 * 	Added author notices
 * 	[91/02/04  11:19:29  mrt]
 * 
 * 	Changed to use new Mach copyright
 * 	[91/02/02  12:18:11  mrt]
 * 
 * Revision 2.3  90/12/05  23:35:12  af
 * 	Cleanups, use BSD labels internally.
 * 	[90/12/03  23:47:29  af]
 * 
 * Revision 2.1.1.1  90/11/01  03:39:55  af
 * 	Created.
 * 	[90/09/03            af]
 */
/*
 *	File: scsi_defs.h
 * 	Author: Alessandro Forin, Carnegie Mellon University
 *	Date:	9/90
 *
 *	Controller-independent definitions for the SCSI driver
 */

#ifndef	_SCSI_SCSI_DEFS_H_
#define	_SCSI_SCSI_DEFS_H_

#include <kern/queue.h>
#include <kern/lock.h>

#define	await(event)	sleep(event,0)
extern	void wakeup();

typedef	vm_offset_t	opaque_t;	/* should be elsewhere */

/*
 * Internal error codes, and return values
 * XXX use the mach/error.h scheme XXX
 */
typedef unsigned int		scsi_ret_t;

#define	SCSI_ERR_GRAVITY(x)	((unsigned)(x)&0xf0000000U)
#define	SCSI_ERR_GRAVE		0x80000000U
#define SCSI_ERR_BAD		0x40000000

#define	SCSI_ERR_CLASS(x)	((unsigned)(x)&0x0fffffffU)
#define	SCSI_ERR_STATUS		0x00000001
#define	SCSI_ERR_SENSE		0x00000002
#define SCSI_ERR_MSEL		0x00000004

extern	void	scsi_error(/* target_info_t *, unsigned, unsigned */);

#define	SCSI_RET_IN_PROGRESS	0x00
#define	SCSI_RET_SUCCESS	0x01
#define	SCSI_RET_RETRY		0x02
#define SCSI_RET_NEED_SENSE	0x04
#define SCSI_RET_ABORTED	0x08
#define	SCSI_RET_DEVICE_DOWN	0x10

/*
 * Device-specific information kept by driver
 */
typedef struct {
	struct disklabel	l;
	struct {
	    unsigned int	badblockno;
	    unsigned int	save_rec;
	    char		*save_addr;
	    int			save_count;
	    int			save_resid;
	    int			retry_count;
	} b;
	int			labelsector;
	int			labeloffset;
} scsi_disk_info_t;

typedef struct {
	boolean_t	read_only;
	unsigned int	speed;
	unsigned int	density;
	unsigned int	maxreclen;
	boolean_t	fixed_size;
} scsi_tape_info_t;

typedef struct {
	char		req_pending;
	char		req_id;
	char		req_lun;
	char		req_cmd;
	unsigned int	req_len;
	/* more later */
} scsi_processor_info_t;

typedef struct {
	void		*result;
	boolean_t	result_available;
	int		result_size;
	struct red_list	*violates_standards;
} scsi_cdrom_info_t;

typedef struct {
#	define SCSI_MAX_COMM_TTYS	16
	struct tty	*tty[SCSI_MAX_COMM_TTYS];
	io_req_t	ior;
} scsi_comm_info_t;

/*
 * Device descriptor
 */

#define	SCSI_TARGET_NAME_LEN	8+16+4+8	/* our way to keep it */

typedef struct target_info {
	queue_chain_t	links;			/* to queue for bus */
	io_req_t	ior;			/* what we are doing */

	unsigned int	flags;
#define	TGT_DID_SYNCH		0x00000001	/* finished the synch neg */
#define	TGT_TRY_SYNCH		0x00000002	/* do the synch negotiation */
#define	TGT_FULLY_PROBED	0x00000004	/* can sleep to wait */
#define	TGT_ONLINE		0x00000008	/* did the once-only stuff */
#define	TGT_ALIVE		0x00000010
#define	TGT_BBR_ACTIVE		0x00000020	/* bad block replc in progress */
#define	TGT_DISCONNECTED	0x00000040	/* waiting for reconnect */
#define	TGT_WRITTEN_TO		0x00000080	/* tapes: needs a filemark on close */
#define	TGT_REWIND_ON_CLOSE	0x00000100	/* tapes: rewind */
#define	TGT_BIG			0x00000200	/* disks: > 1Gb, use long R/W */
#define	TGT_REMOVABLE_MEDIA	0x00000400	/* e.g. floppy, cd-rom,.. */
#define	TGT_READONLY		0x00000800	/* cd-rom, scanner, .. */
#define	TGT_OPTIONAL_CMD	0x00001000	/* optional cmd, ignore errors */
#define TGT_WRITE_LABEL		0x00002000	/* disks: enable overwriting of label */
#define	TGT_US			0x00004000	/* our desc, when target role */

#define	TGT_HW_SPECIFIC_BITS	0xffff0000U	/* see specific HBA */
	char		*hw_state;		/* opaque */
	char		*dma_ptr;
	char		*cmd_ptr;
	struct scsi_devsw_struct	*dev_ops;	/* circularity */
	struct target_info	*next_lun;	/* if multi-LUN */
	char		target_id;
	char		unit_no;
	unsigned char	sync_period;
	unsigned char	sync_offset;
	decl_simple_lock_data(,target_lock)
#ifdef	MACH_KERNEL
#else	/*MACH_KERNEL*/
	struct fdma	fdma;
#endif	/*MACH_KERNEL*/
	/*
	 * State info kept while waiting to seize bus, either for first
	 * selection or while in disconnected state
	 */
	struct {
	    struct script	*script;
	    int			(*handler)();
	    unsigned int	out_count;
	    unsigned int	in_count;
	    unsigned int	copy_count;	/* optional */
	    unsigned int	dma_offset;
	    unsigned char	identify;
	    unsigned char	cmd_count;
	    unsigned char	hba_dep[2];
	} transient_state;
	unsigned int	block_size;
	volatile char	done;
	unsigned char	cur_cmd;
	unsigned char	lun;
	char		masterno;
	char		tgt_name[SCSI_TARGET_NAME_LEN];
	union {
	    scsi_disk_info_t	disk;
	    scsi_tape_info_t	tape;
	    scsi_cdrom_info_t	cdrom;
	    scsi_processor_info_t	cpu;
	    scsi_comm_info_t	comm;
	} dev_info;
} target_info_t;


/*
 * Device-specific operations
 */
typedef struct scsi_devsw_struct {
	char		*(*driver_name)(boolean_t);	  /* my driver's name */
	void		(*optimize)(target_info_t *);	  /* tune up internal params */
	scsi_ret_t	(*open)(target_info_t *,io_req_t);/* open time ops */
	scsi_ret_t	(*close)(target_info_t *);	  /* close time ops */
	int		(*strategy)(io_req_t);	          /* sort/start routine */
	void		(*restart)(target_info_t *,
				   boolean_t);		  /* completion routine */
	io_return_t	(*get_status)(int, 
				      target_info_t *,
				      dev_flavor_t,
				      dev_status_t,
				      natural_t *);	  /* specialization */
	io_return_t	(*set_status)(int, 
				      target_info_t *,
				      dev_flavor_t,
				      dev_status_t,
				      natural_t);	  /* specialization */
} scsi_devsw_t;

#define SCSI_OPTIMIZE_NULL ((void (*)(target_info_t *)) 0)
#define SCSI_OPEN_NULL ((scsi_ret_t (*)(target_info_t *,io_req_t)) 0)
#define SCSI_CLOSE_NULL ((scsi_ret_t (*)(target_info_t *)) 0)

extern scsi_devsw_t	scsi_devsw[];

/*
 * HBA descriptor
 */

typedef struct {
	/* initiator (us) state */
	unsigned char	initiator_id;
	unsigned char	masterno;
	unsigned int	max_dma_data;
	char		*hw_state;		/* opaque */
	int		(*go)();
	void		(*watchdog)();
	boolean_t	(*probe)();
	/* per-target state */
	target_info_t		*target[8];
} scsi_softc_t;

extern scsi_softc_t	*scsi_softc[];
extern scsi_softc_t	*scsi_master_alloc(/* int unit */);
extern target_info_t	*scsi_slave_alloc(/* int unit, int slave, char *hw */);

#define	BGET(d,mid,id)	(d[mid] & (1 << id))		/* bitmap ops */
#define BSET(d,mid,id)	d[mid] |= (1 << id)
#define BCLR(d,mid,id)	d[mid] &= ~(1 << id)

extern unsigned char	scsi_no_synchronous_xfer[];	/* one bitmap per ctlr */
extern unsigned char	scsi_use_long_form[];		/* one bitmap per ctlr */
extern unsigned char	scsi_might_disconnect[];	/* one bitmap per ctlr */
extern unsigned char	scsi_should_disconnect[];	/* one bitmap per ctlr */
extern unsigned char	scsi_initiator_id[];		/* one id per ctlr */

extern boolean_t	scsi_exabyte_filemarks;
extern boolean_t	scsi_no_automatic_bbr;
extern int		scsi_bbr_retries;
extern int		scsi_watchdog_period;
extern int		scsi_delay_after_reset;
extern unsigned int	scsi_per_target_virtual;	/* 2.5 only */

extern int		scsi_debug;

/*
 * HBA-independent Watchdog
 */
typedef struct {

	unsigned short	reset_count;
	char		nactive;

	char		watchdog_state;

#define SCSI_WD_INACTIVE	0
#define	SCSI_WD_ACTIVE		1
#define SCSI_WD_EXPIRED		2

	int		(*reset)();

} watchdog_t;

extern void scsi_watchdog( watchdog_t* );

#endif	_SCSI_SCSI_DEFS_H_
