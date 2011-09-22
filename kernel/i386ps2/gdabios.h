/* 
 * Mach Operating System
 * Copyright (c) 1991 Carnegie Mellon University
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

#ifndef _H_GDABIOS
#define _H_GDABIOS
/*
 * COMPONENT:	SYSXGD
 *
 * ORIGINS:	27
 *
 */

/*
 * HISTORY
 * $Log:	gdabios.h,v $
 * Revision 2.3  93/03/11  14:09:15  danner
 * 	u_long -> u_int
 * 	[93/03/09            danner]
 * 
 * Revision 2.2  93/02/04  07:59:49  danner
 * 	Integrate PS2 code from IBM.
 * 	[93/01/18            prithvi]
 * 
 */

#ifdef notdef
#include "abios.h"
#else
#include <i386ps2/abios.h>
#endif

/*
 * ABIOS generic disk specific function codes.
 */
#define ABIOS_GD_WRITE_VERIFY	 		0x0a
#define ABIOS_GD_VERIFY_BLOCK			0x0b
#define ABIOS_GD_INTERRUPT_STATUS		0x0c
#define ABIOS_GD_ENABLE_INTEL_BUFFER		0x0d
#define ABIOS_GD_DISABLE_INTEL_BUFFER		0x0e
#define ABIOS_GD_RETURN_INTEL_BUF_STAT		0x0f
#define ABIOS_GD_SET_DMA_PACING_FACTOR		0x10
#define ABIOS_GD_RET_DMA_PACING_FACTOR		0x11
#define ABIOS_GD_TRANSFER_SCB			0x12
#define ABIOS_GD_DEALLOCATE			0x14

/*
 * Set up gd ABIOS request block, assuming request block length
 * of GD_REQUEST_BLOCK_LEN+0x10 bytes.  This should be sufficient.
 * The real value can be found by issuing ABIOS_LOGICAL_PARAMETER
 * function.
 */
#define GD_REQUEST_BLOCK_LEN	160	
struct	Gd_request {
	struct Request_header	request_header;	/* 0x00-0x0f abios.h */
	union {
		struct Logical_id_params        logical_id_params;
		u_char				uc[GD_REQUEST_BLOCK_LEN];
	} un;
};

/*
 * For ABIOS_LOGICAL_PARAMETER (0x01)
 * rc = 0 only.  NO interrupt.
 */
/* INPUT */
#define GD_SET_RESERVED_ABIOS_LOGICAL_PARAMETER(rb) \
        { \
        *((u_short *)&(rb).un.uc[0x0a]) = 0; \
        *((u_short *)&(rb).un.uc[0x0c]) = 0; \
        *((u_short *)&(rb).un.uc[0x0e]) = 0; \
        }
/* OUTPUT */
/* defined in abios.h */

/*
 * For ABIOS_READ_PARAMETER (0x03) function which returns info
 * specific to the current DRIVE.
 */
/* INPUT */
#define GD_SET_RESERVED_ABIOS_READ_PARAMETER(rb) \
	{ \
	*((u_short *)&(rb).un.uc[0x08]) = 0; \
	}
/* OUTPUT */
#define gd_sector_per_track(rb)		*((u_short *)&(rb).un.uc[0x00])
#define gd_byte_per_sector(rb)		*((u_short *)&(rb).un.uc[0x02])
#define GD_512_BYTE_PER_SECTOR		0x0002
#define gd_dev_ctrl_flag(rb)		*((u_short *)&(rb).un.uc[0x04])
#define GD_PARAMS_INVALID		0x0001  /* bit 0 */
#define GD_POWER_OFF			0x0002  /* bit 1 */
#define GD_CHANGE_SIGNAL_AVAIL		0x0004	/* bit 2 */
#define GD_WRITE_MANY			0x0008	/* bit 3 */
#define GD_SUPPORT_CACHING		0x0010	/* bit 4 */
#define GD_READABLE			0x0020	/* bit 5 */
#define GD_LOCKABLE			0x0040  /* bit 6 */
#define GD_SEQUENTIAL			0x0080  /* bit 7 */
#define GD_EJECTABLE			0x0100  /* bit 8 */
#define GD_CONCURRENT			0x0200  /* bit 9 */
#define GD_ST506			0x0400  /* bit 10 */
#define GD_NO_FORMAT			0x0000  /* bits 11&12 */
#define GD_TRACK_FORMAT			0x0800  /* bits 11&12 */
#define GD_UNIT_FORMAT			0x1000  /* bits 11&12 */
#define GD_BOTH_FORMAT			0x1800  /* bits 11&12 */
#define GD_FORMAT_SUPPORT		0x1800
#define GD_SCSI_DEVICE			0x4000  /* bit 14 */
#define GD_SCB_TRANSFER_SUPPORT		0x8000  /* bit 15 */
#define gd_number_cylinders(rb)		*((u_int *)&(rb).un.uc[0x08])
#define gd_number_heads(rb)		*((u_char *)&(rb).un.uc[0x0c])
#define gd_max_retries(rb)		*((u_char *)&(rb).un.uc[0x0d])
#define gd_max_block_number(rb)		*((u_int *)&(rb).un.uc[0x10])
#define gd_max_transfer(rb)		*((u_short *)&(rb).un.uc[0x1c])

/*
 * For ABIOS_WRITE_PARAMETER (0x04)
 */
/* Reserved for disk devices */
/* INPUT */
/* none */
/* OUTPUT */
/* none */

/*
 * For ABIOS_RESET (0x05)
 */
/* INPUT */
#define GD_SET_RESERVED_ABIOS_RESET(rb) \
	{ \
	*((u_short *)&(rb).un.uc[0x00]) = 0; \
	}
/* OUTPUT */
#define gd_wait_time(rb)        *((u_int *)&(rb).un.uc[0x18])

/*
 * For ABIOS_ENABLE_INTR (0x06)
 */
/* Reserved for disk devices */
/* INPUT */
/* none */
/* OUTPUT */
/* none */

/*
 * For ABIOS_DISABLE_INTR (0x07)
 */
/* INPUT */
/* none */
/* OUTPUT */
/* none */

/*
 * For ABIOS_READ (0x08)
 * For ABIOS_WRITE (0x09)
 * For ABIOS_GD_WRITE_VERIFY (0x0a)
 * For ABIOS_GD_VERIFY_BLOCK (0x0b)
 */
/* INPUT */
#define GD_SET_RESERVED_ABIOS_GD_VERIFY_BLOCK(rb) \
	{ \
	*((u_short *)&(rb).un.uc[0x00]) = 0; \
	*((u_short *)&(rb).un.uc[0x06]) = 0; \
	*((u_short *)&(rb).un.uc[0x08]) = 0; \
	*((u_short *)&(rb).un.uc[0x0e]) = 0; \
        *((u_int *)&(rb).un.uc[0x14]) = 0; \
	}
#define GD_SET_RESERVED_CACHING \
	{ \
	*((u_char *)&(rb).un.uc[0x1e]) = 0; \
	}
#define GD_SET_RESERVED_ABIOS_READ(rb) \
        { \
        *((u_short *)&(rb).un.uc[0x00]) = 0; \
        *((u_short *)&(rb).un.uc[0x06]) = 0; \
        *((u_short *)&(rb).un.uc[0x08]) = 0; \
        *((u_short *)&(rb).un.uc[0x0e]) = 0; \
        *((u_int *)&(rb).un.uc[0x14]) = 0; \
        *((u_char *)&(rb).un.uc[0x1e]) = 0; \
        }
#define GD_SET_RESERVED_ABIOS_WRITE(rb) \
        { \
        *((u_short *)&(rb).un.uc[0x00]) = 0; \
        *((u_short *)&(rb).un.uc[0x06]) = 0; \
        *((u_short *)&(rb).un.uc[0x08]) = 0; \
        *((u_short *)&(rb).un.uc[0x0e]) = 0; \
        *((u_int *)&(rb).un.uc[0x14]) = 0; \
        *((u_char *)&(rb).un.uc[0x1e]) = 0; \
        }
#define GD_SET_RESERVED_ABIOS_GD_WRITE_VERIFY(rb) \
        { \
        *((u_short *)&(rb).un.uc[0x00]) = 0; \
        *((u_short *)&(rb).un.uc[0x06]) = 0; \
        *((u_short *)&(rb).un.uc[0x08]) = 0; \
        *((u_short *)&(rb).un.uc[0x0e]) = 0; \
        *((u_int *)&(rb).un.uc[0x14]) = 0; \
        *((u_char *)&(rb).un.uc[0x1e]) = 0; \
        }

#define gd_logical_ptr(rb)		*((u_int *)&(rb).un.uc[0x02])
#define gd_physical_ptr(rb)		*((u_int *)&(rb).un.uc[0x0a])
#define gd_relative_block_address(rb)	*((u_int *)&(rb).un.uc[0x10])
#define gd_blocks_to_read(rb)		*((u_short *)&(rb).un.uc[0x1c])
#define gd_blocks_to_write(rb)		*((u_short *)&(rb).un.uc[0x1c])
#define gd_caching_ok(rb)		*((u_char *)&(rb).un.uc[0x1e])
#define GD_DONT_CACHE			0x01

/* OUTPUT */
/* gd_wait_time is the same as Reset/Initialize */
#define gd_blocks_read(rb)		*((u_short *)&(rb).un.uc[0x1c])
#define gd_blocks_written(rb)		*((u_short *)&(rb).un.uc[0x1c])
#define gd_soft_error(rb)		*((u_short *)&(rb).un.uc[0x1f])
#define GD_NO_SOFT_ERROR		0x0000
#define gd_interrupt_status(rb)		*((u_char *)&(rb).un.uc[0x00])
#define GD_NO_INTERRUPT_PENDING		0x00
#define GD_INTERRUPT_PENDING		0x01




/*
 * For ABIOS_GD_ENABLE_INTEL_BUFFER    (0x0d)
 * For ABIOS_GD_DISABLE_INTEL_BUFFER   (0x0e)
 * For ABIOS_GD_RETURN_INTEL_BUF_STAT  (0x0f)

 */
/* INPUT */
#define GD_SET_RESERVED_ABIOS_GD_ENABLE_INTEL_BUFFER(rb) \
	{ \
	*((u_short *)&(rb).un.uc[0x06]) = 0; \
	}
#define GD_SET_RESERVED_ABIOS_GD_DISABLE_INTEL_BUFFER(rb) \
		 	GD_SET_RESERVED_ABIOS_GD_ENABLE_INTEL_BUFFER(rb)
#define GD_SET_RESERVED_ABIOS_GD_RETURN_INTEL_BUF_STAT(rb) \
			GD_SET_RESERVED_ABIOS_GD_ENABLE_INTEL_BUFFER(rb)
/* OUTPUT */
/* gd_wait_time is the same as Reset/Initialize */
#define gd_intel_buffer_status(rb)	*((u_char *)&(rb).un.uc[0x00])
#define GD_INTEL_BUFFER_DISABLED	0x01
/*
 * For ABIOS_GD_SET_DMA_PACING_FACTOR  (0x10)
 * For ABIOS_GD_RET_DMA_PACING_FACTOR  (0x11)
 */
/* INPUT */
#define GD_SET_RESERVED_ABIOS_GD_SET_DMA_PACING_FACTOR \
			D_SET_RESERVED_ABIOS_GD_ENABLE_INTEL_BUFFER(rb)
#define GD_SET_RESERVED_ABIOS_GD_RET_DMA_PACING_FACTOR \
                        D_SET_RESERVED_ABIOS_GD_ENABLE_INTEL_BUFFER(rb)
#define gd_pacing_value(rb)		*((u_char *)&(rb).un.uc[0x00])

/* OUTPUT */
/* gd_wait_time is the same as Reset/Initialize */
#define gd_return_pacing_value(rb)             *((u_char *)&(rb).un.uc[0x00])

/*
 * For ABIOS_GD_TRANSFER_SCB (0x12)
 */
/* INPUT */
#define GD_SET_RESERVED_ABIOS_GD_TRANSFER_SCB(rb) \
        { \
        *((u_short *)&(rb).un.uc[0x04]) = 0; \
        *((u_short *)&(rb).un.uc[0x0c]) = 0; \
        *((u_short *)&(rb).un.uc[0x16]) = 0; \
        *((u_short *)&(rb).un.uc[0x1c]) = 0; \
        *((u_char *)&(rb).un.uc[0x1e]) = 0; \
        }
#define gd_set_flags(rb)                        *((u_char *)&(rb).un.uc[0x1e])
#define GD_LONG_SCB                             0x01

/* OUTPUT */
#define gd_last_scb(rb)                         *((u_int *)&(rb).un.uc[0x0e])
/* gd_wait_time is the same as Reset/Initialize */
/* gd_soft_errors is the same as Abios Read */

/*
 * For ABIOS_GD_DEALLOCATE             (0x14)
 *
/* INPUT */
#define GD_SET_RESERVED_ABIOS_GD_DEALLOCATE(rb) \
        { \
        *((u_short *)&(rb).un.uc[0x06]) = 0; \
        }
/* OUTPUT */
#define gd_scsi_disk_number(rb)			*((u_short *)&(rb).un.uc[0x02])

struct		hdsoft {
	u_short		status;
#define IDLE			0
#define	IN_TRANSFER		1
#define	BAD_BLOCK_RECOVER	2
#define	PIO_TRANSFER		3
#define	GET_PARAMETERS		4
#define	GET_BAD_BLOCK		5
	u_short	retries;
	u_short	max_retry;
	u_short	max_transfer;
	u_short	blocks_left;
	u_short	hdtimeout;
	u_short	byte_per_block;
	u_int	last_block;
	u_char	irq;
};
#endif /* _H_GDABIOS */
