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

/* @(#)62       1.1  mk/src/latest/kernel/i386/PS2/fd_abios.h, MACH 4/4/91 10:22:05 */
/*
 * COMPONENT_NAME: SYSXFD floppy diskette driver
 *
 * FUNCTIONS:
 *
 * ORIGINS: 27
 */                                     

/*
 * HISTORY
 * $Log:	fd_abios.h,v $
 * Revision 2.3  93/03/11  14:09:04  danner
 * 	u_long -> u_int
 * 	[93/03/09            danner]
 * 
 * Revision 2.2  93/02/04  07:59:35  danner
 * 	Integrate PS2 code from IBM.
 * 	[93/01/18            prithvi]
 * 
 */                              

#ifndef _H_FDABIOS
#define _H_FDABIOS

/* ABIOS diskette specific function codes.
** 	- see <sys/abios.h> for generic ABIOS request defines
*/
#define	ABIOS_FD_VERIFY_SECTOR		0x0b
#define	ABIOS_FD_READ_MEDIA_TYPE	0x0c
#define	ABIOS_FD_SET_MEDIA_TYPE		0x0d
#define	ABIOS_FD_CHANGE_SIGNAL_STATUS	0x0e
#define ABIOS_FD_TURN_OFF_MOTOR		0x0f
#define ABIOS_FD_INTERRUPT_STATUS	0x10

/* ABIOS diskette specific return codes.
*/
#define ABIOS_FD_RC_TIMEOUT		0xfffe
#define ABIOS_FD_RC_DONE		0x0000


/*  Set up fd ABIOS request block, assuming request block length
** of FD_REQUEST_BLOCK_LEN+0x10 bytes.  This should be sufficient.
** The real value can be found by issuing ABIOS_LOGICAL_PARAMETER
** function.
*/
#define FD_REQUEST_BLOCK_LEN		128
struct	Fd_request {
	struct Request_header request_header;	/* 0x00-0x0f abios.h */
	union {
		struct Logical_id_params	logical_id_params;
		u_char				uc[FD_REQUEST_BLOCK_LEN];
	} un;
	int state;
	int sleep_on_intr;			/* for syncronous requests */
	int req_errno;				/* for syncronous errors */
};

/* Possible States an ABIOS request block
*/
#define FD_RB_IDLE		0		/* don't use BIT0 */
#define FD_RB_STARTED		BIT1
#define FD_RB_STAGING		BIT2
#define FD_RB_SLEEPING		BIT3
#define FD_RB_SYNC		BIT4
#define FD_RB_RESET		BIT5


/* ABIOS_LOGICAL_PARAMETER (0x01)
**	- Always returns 0.
**	- Does not interrupt.
** INPUT: see fd_cmd.c
** OUTPUT: see <sys/abios.h>
*/
#define FD_SET_RESERVED_ABIOS_LOGICAL_PARAMETER(rb) \
	*( (u_short *) &((rb)->un.uc[0x0a]) ) = 0; \
	*( (u_short *) &((rb)->un.uc[0x0c]) ) = 0; \
	*( (u_short *) &((rb)->un.uc[0x0e]) ) = 0;


/* ABIOS_READ_PARAMETER (0x03)
**	- function which returns info specific to the selected drive.
**	- only returns 0
**	- does not interrupt
** INPUT: drive number
**
*/
#define FD_SET_RESERVED_ABIOS_READ_PARAMETER(rb) \
	*( (u_short *) &((rb)->un.uc[0x08]) ) = 0;

/* OUTPUT */
#define fd_sectors_per_track(rb)	*( (u_short *) &((rb)->un.uc[0x00]) )
#define fd_bytes_per_sector(rb)		*( (u_short *) &((rb)->un.uc[0x02]) )
#define FD_256_BYTE_PER_SECTOR		0x0001
#define FD_512_BYTE_PER_SECTOR		0x0002
#define fd_dev_ctrl_flag(rb)		*( (u_short *) &((rb)->un.uc[0x04]) )
#define FD_CHANGE_SIGNAL_AVAIL		0x0001	/* bit 0 */
#define FD_SUPPORT_FORMAT_CMD		0x0002	/* bit 1 */
#define FD_SUPPORT_CONCURRENT		0x0004	/* bit 2 */
#define FD_RECALIBRATE_REQUIRED		0x0008	/* bit 3 */
#define FD_ABIOS_PROVIDES_GAP_LEN	0x0020	/* bit 6 */
#define fd_drive_type(rb)		*( (u_short *) &((rb)->un.uc[0x06]) )
#define FD_NO_DRIVE			0
#define FD_360_KB_DRIVE			1
#define FD_1440_KB_DRIVE		4
#define fd_motor_off_delay_time(rb)	*( (u_int *) &((rb)->un.uc[0x0c]) )
#define fd_motor_start_delay_time(rb)	*( (u_int *) &((rb)->un.uc[0x10]) )
#define fd_num_of_cylinders(rb)		*( (u_short *) &((rb)->un.uc[0x16]) )
#define fd_num_of_heads(rb)		*( (u_char *) &((rb)->un.uc[0x1a]) )
#define fd_retry_count(rb)		*( (u_char *) &((rb)->un.uc[0x1b]) )
#define fd_format_fill_byte(rb)		*( (u_char *) &((rb)->un.uc[0x1c]) )
#define fd_head_settle_time(rb)		*( (u_int *) &((rb)->un.uc[0x1d]) )
#define fd_rwv_gap_len(rb)		*( (u_char *) &((rb)->un.uc[0x21]) )
#define fd_format_gap_len(rb)		*( (u_char *) &((rb)->un.uc[0x22]) )
#define fd_data_len(rb)			*( (u_char *) &((rb)->un.uc[0x23]) )


/* For ABIOS_WRITE_PARAMETER (0x04)
**	- Set device parameters
** INPUT: this ABIOS service is currently unused
** OUTPUT: none
*/
#define FD_SET_RESERVED_ABIOS_WRITE_PARAMETER(rb) \
	*((u_short *)&(rb)->un.uc[0x00]) = 0;


/* For ABIOS_RESET (0x05)
**	- resets the diskette system to an initial state
** INPUT: none
** OUTPUT: none
*/
#define FD_SET_RESERVED_ABIOS_RESET(rb) \
	*( (u_short *) &((rb)->un.uc[0x00]) ) = 0;


/* For ABIOS_DISABLE_INTR (0x07)
**	- this ABIOS service is currently unused
** INPUT: none
** OUTPUT: none
*/
#define FD_SET_RESERVED_ABIOS_DISABLE_INTR(rb) \
	*((u_short *)&(rb)->un.uc[0x08]) = 0;


/* ABIOS_READ (0x08)
**	- Read data from the diskette
** INPUT (of special note):
**	data_ptr_1	- Virtual address (not supported as of 1/90)
**	data_ptr_2	- Physical address
*/
#define FD_SET_RESERVED_ABIOS_READ(rb) \
	*( (u_short *) &((rb)->un.uc[0x00]) ) = 0; \
	*( (u_short *) &((rb)->un.uc[0x06]) ) = 0; \
	*( (u_short *) &((rb)->un.uc[0x08]) ) = 0; \
	*( (u_short *) &((rb)->un.uc[0x0e]) ) = 0;

#define FD_TIME_TO_WAIT(rb)		*( (u_int  *) &((rb)->un.uc[0x10]) )
#define fd_data_ptr_1(rb)		*( (u_int  *) &((rb)->un.uc[0x02]) )
#define fd_data_ptr_2(rb)		*( (u_int  *) &((rb)->un.uc[0x0a]) )
#define fd_num_sector_rw(rb)		*( (u_short *) &((rb)->un.uc[0x14]) )
#define fd_cylinder_num(rb)		*( (u_short *) &((rb)->un.uc[0x16]) )
#define fd_head_num(rb)			*( (u_char  *) &((rb)->un.uc[0x1a]) )
#define fd_sector_num(rb)		*( (u_short *) &((rb)->un.uc[0x21]) )
#define fd_xfer_sub_function(rb)	*( (u_short *) &((rb)->un.uc[0x14]) )
#define FD_XFER_SUB_FORMAT		0

/* OUTPUT: */
#define fd_wait_time(rb)		*( (u_int *)  &((rb)->un.uc[0x10]) )
#define fd_sectors_moved(rb)		*( (u_short *) &((rb)->un.uc[0x14]) )


/* ABIOS_WRITE (0x09)
**	- Write data to the diskette
** INPUT/OUTPUT: see ABIOS_READ
*/
#define FD_SET_RESERVED_ABIOS_WRITE(rb) FD_SET_RESERVED_ABIOS_READ(rb)


/* ABIOS_ADDITIONAL_XFER (0x0a)
**	- When the FD_XFER_SUB_FORMAT is set this formats a track
** INPUT/OUTPUT: see ABIOS_READ
*/
#define FD_SET_RESERVED_ABIOS_ADDITIONAL_XFER(rb) \
		FD_SET_RESERVED_ABIOS_READ(rb)


/* ABIOS_FD_VERIFY_SECTOR (0x0b)
**	- Verify data on the diskette
**	- This ABIOS service is currently not used
**
** INPUT: almost the same as ABIOS_READ
** OUTPUT: see ABIOS_READ
*/
#define FD_SET_RESERVED_ABIOS_FD_VERIFY_SECTOR(rb) \
	*( (u_short *) &((rb)->un.uc[0x06]) ) = 0; \
	*( (u_short *) &((rb)->un.uc[0x0e]) ) = 0;


/* ABIOS_FD_READ_MEDIA_TYPE (0x0c) 
**	- reports the media type used on the last read/write/format
** INPUT: none.
** OUTPUT: see ABIOS_READ_PARAMETER
*/
#define FD_SET_RESERVED_ABIOS_FD_READ_MEDIA_TYPE(rb) \
	*( (u_short *) &((rb)->un.uc[0x06]) ) = 0;


/* ABIOS_FD_SET_MEDIA_TYPE (0x0d)
**	- sets the media type for a format
** INPUT: see output of ABIOS_FD_READ_MEDIA_TYPE
** OUTPUT: see READ_DATA
*/
#define FD_SET_RESERVED_ABIOS_FD_SET_MEDIA_TYPE(rb) \
	FD_SET_RESERVED_ABIOS_FD_READ_MEDIA_TYPE(rb) 


/* ABIOS_FD_CHANGE_SIGNAL_STATUS (0x0e)
**	- reports if the diskette change signal is on
** INPUT: none
** OUTPUT: change signal status
*/
#define FD_SET_RESERVED_ABIOS_FD_CHANGE_SIGNAL_STATUS(rb) \
			FD_SET_RESERVED_ABIOS_FD_READ_MEDIA_TYPE(rb) 

#define fd_change_signal_status(rb)	*((u_char *)&((rb)->un.uc[0x00]))
#define FD_CHANGE_SIGNAL_INACTIVE	0
#define FD_CHANGE_SIGNAL_ACTIVE		6


/* ABIOS_FD_TURN_OFF_MOTOR (0x0f)
**	- turn the motor (and the light off)
** INPUT/OUTPUT: none.
*/
#define FD_SET_RESERVED_ABIOS_FD_TURN_OFF_MOTOR(rb) \
	FD_SET_RESERVED_ABIOS_FD_READ_MEDIA_TYPE(rb) 


/* For ABIOS_FD_INTERRUPT_STATUS (0x10)
**	- returns the diskette interrupt pending status
** INPUT: none
** OUTPUT: status of interrrupt
*/
#define FD_SET_RESERVED_ABIOS_FD_INTERRUPT_STATUS(rb) \
	FD_SET_RESERVED_ABIOS_FD_READ_MEDIA_TYPE(rb) 

#define fd_interrupt_status(rb)		*((u_char *)&(rb).un.uc[0x00])
#define FD_INTERRUPT_PENNDING(rb)	1


/* Diskette specific ABIOS error codes:
*/
#define FDABIOS_WRITEPROTECTED	0x8003
#define FDABIOS_MEDIACHANGED	0x8006
#define FDABIOS_MEDIANOTPRESENT	0x800d
#define FDABIOS_DMAOVERRUN	0x9108
#define FDABIOS_BADCRC		0x9110
#define FDABIOS_CONTFAIL	0x9120
#define FDABIOS_SEEKFAIL	0x9140
#define FDABIOS_GENERALERROR	0x9180

#endif /* _H_FDABIOS */
