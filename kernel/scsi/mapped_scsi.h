/* 
 * Mach Operating System
 * Copyright (c) 1991,1990,1989 Carnegie Mellon University
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
 * $Log:	mapped_scsi.h,v $
 * Revision 2.7  91/10/09  16:16:45  af
 * 	Define maximum size of mapped region.  The fifo flags
 * 	for the ASC chip do not change with interrupt ack, the
 * 	command might.
 * 	[91/10/05  10:25:03  af]
 * 
 * Revision 2.6  91/06/19  11:56:46  rvb
 * 	File moved here from mips/PMAX since it is now "MI" code, also
 * 	used by Vax3100 and soon -- the omron luna88k.
 * 	[91/06/04            rvb]
 * 
 * Revision 2.5  91/05/14  17:24:10  mrt
 * 	Correcting copyright
 * 
 * Revision 2.4  91/02/05  17:42:31  mrt
 * 	Added author notices
 * 	[91/02/04  11:14:56  mrt]
 * 
 * Revision 2.3  90/12/05  23:32:32  af
 * 	Changed to use new Mach copyright
 * 
 * Revision 2.1.1.1  90/11/13  15:09:49  af
 * 	Factored out, for user's benefit.
 * 	[90/11/08            af]
 */
/*
 *	File: mapped_scsi.h
 * 	Author: Alessandro Forin, Carnegie Mellon University
 *	Date:	11/90
 *
 *	Definitions for the User-level SCSI Driver
 */

/*
 * HBA chips of various sorts
 */

/* DEC 7061 used on pmaxen */

typedef struct sii_volatile_regs {
	unsigned short		sii_conn_csr;
	unsigned short		sii_data_csr;
} *sii_reg_t;

#define	HBA_DEC_7061	0x00000001

						/* layout of mapped stuff */
#define	SII_REGS_BASE	(SCSI_INFO_BASE+SCSI_INFO_SIZE)
#define	SII_REGS_SIZE	PAGE_SIZE
#define	SII_RAM_BASE	(SII_REGS_BASE+SII_REGS_SIZE)


/* National 53C94, used on 3maxen' PMAZ-AA boards */

typedef struct asc_volatile_regs {
	unsigned char		csr;
	unsigned char		isr;
	unsigned char		seq;
	unsigned char		cmd;
} *asc_reg_t;

#define	HBA_NCR_53c94	0x00000002

						/* layout of mapped stuff */
#define	ASC_REGS_BASE	(SCSI_INFO_BASE+SCSI_INFO_SIZE)
#define	ASC_REGS_SIZE	PAGE_SIZE
#define	ASC_DMAR_BASE	(ASC_REGS_BASE+ASC_REGS_SIZE)
#define	ASC_DMAR_SIZE	PAGE_SIZE
#define	ASC_RAM_BASE	(ASC_DMAR_BASE+ASC_DMAR_SIZE)

/*
 * User-mapped information block, common to all
 */
#define	SCSI_INFO_BASE	0
#define	SCSI_INFO_SIZE	PAGE_SIZE

#define	SCSI_MAX_MAPPED_SIZE	(ASC_RAM_BASE+128*1024)

typedef struct {
	int			interrupt_count;/* Counter kept by kernel */
	unsigned int		wait_event;	/* To wait for interrupts */
	unsigned		ram_size;
	int			hba_type;	/* Tag for regs union */
	union {					/* Space for regs saved on
						 * intr.  Only few used */
	    struct asc_volatile_regs	asc;
	    struct sii_volatile_regs	sii;
	} saved_regs;
} *mapped_scsi_info_t;

