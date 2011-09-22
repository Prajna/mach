/* 
 * Mach Operating System
 * Copyright (c) 1991,1990,1989,1988,1987 Carnegie Mellon University
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
 * $Log:	notify.h,v $
 * Revision 2.6  93/01/14  17:46:19  danner
 * 	Cleanup.
 * 	[92/06/10            pds]
 * 
 * Revision 2.5  92/01/15  13:44:41  rpd
 * 	Changed MACH_IPC_COMPAT conditionals to default to not present.
 * 
 * Revision 2.4  91/05/14  16:58:21  mrt
 * 	Correcting copyright
 * 
 * Revision 2.3  91/02/05  17:35:18  mrt
 * 	Changed to new Mach copyright
 * 	[91/02/01  17:20:02  mrt]
 * 
 * Revision 2.2  90/06/02  14:59:32  rpd
 * 	Converted to new IPC.
 * 	[90/03/26  22:38:14  rpd]
 * 
 * Revision 2.7.7.1  90/02/20  22:24:32  rpd
 * 	Revised for new IPC.
 * 	[90/02/19  23:38:57  rpd]
 * 
 *
 * Condensed history:
 *	Moved ownership rights under MACH_IPC_XXXHACK (rpd).
 * 	Added NOTIFY_PORT_DESTROYED (rpd).
 *	Added notification message structure definition (mwyoung).
 *	Created, based on Accent values (mwyoung).
 */
/*
 *	File:	mach/notify.h
 *
 *	Kernel notification message definitions.
 */

#ifndef	_MACH_NOTIFY_H_
#define _MACH_NOTIFY_H_

#ifdef	KERNEL
#include <mach_ipc_compat.h>
#endif	/* KERNEL */

#include <mach/port.h>
#include <mach/message.h>

/*
 *  An alternative specification of the notification interface
 *  may be found in mach/notify.defs.
 */

#define MACH_NOTIFY_FIRST		0100
#define MACH_NOTIFY_PORT_DELETED	(MACH_NOTIFY_FIRST + 001 )
			/* A send or send-once right was deleted. */
#define MACH_NOTIFY_MSG_ACCEPTED	(MACH_NOTIFY_FIRST + 002)
			/* A MACH_SEND_NOTIFY msg was accepted */
#define MACH_NOTIFY_PORT_DESTROYED	(MACH_NOTIFY_FIRST + 005)
			/* A receive right was (would have been) deallocated */
#define MACH_NOTIFY_NO_SENDERS		(MACH_NOTIFY_FIRST + 006)
			/* Receive right has no extant send rights */
#define MACH_NOTIFY_SEND_ONCE		(MACH_NOTIFY_FIRST + 007)
			/* An extant send-once right died */
#define MACH_NOTIFY_DEAD_NAME		(MACH_NOTIFY_FIRST + 010)
			/* Send or send-once right died, leaving a dead-name */
#define MACH_NOTIFY_LAST		(MACH_NOTIFY_FIRST + 015)

typedef struct {
    mach_msg_header_t	not_header;
    mach_msg_type_t	not_type;	/* MACH_MSG_TYPE_PORT_NAME */
    mach_port_t		not_port;
} mach_port_deleted_notification_t;

typedef struct {
    mach_msg_header_t	not_header;
    mach_msg_type_t	not_type;	/* MACH_MSG_TYPE_PORT_NAME */
    mach_port_t		not_port;
} mach_msg_accepted_notification_t;

typedef struct {
    mach_msg_header_t	not_header;
    mach_msg_type_t	not_type;	/* MACH_MSG_TYPE_PORT_RECEIVE */
    mach_port_t		not_port;
} mach_port_destroyed_notification_t;

typedef struct {
    mach_msg_header_t	not_header;
    mach_msg_type_t	not_type;	/* MACH_MSG_TYPE_INTEGER_32 */
    unsigned int	not_count;
} mach_no_senders_notification_t;

typedef struct {
    mach_msg_header_t	not_header;
} mach_send_once_notification_t;

typedef struct {
    mach_msg_header_t	not_header;
    mach_msg_type_t	not_type;	/* MACH_MSG_TYPE_PORT_NAME */
    mach_port_t		not_port;
} mach_dead_name_notification_t;


/* Definitions for the old IPC interface. */

#if	MACH_IPC_COMPAT

/*
 *	Notifications sent upon interesting system events.
 */

#define NOTIFY_FIRST			0100
#define NOTIFY_PORT_DELETED		( NOTIFY_FIRST + 001 )
#define NOTIFY_MSG_ACCEPTED		( NOTIFY_FIRST + 002 )
#define NOTIFY_OWNERSHIP_RIGHTS		( NOTIFY_FIRST + 003 )
#define NOTIFY_RECEIVE_RIGHTS		( NOTIFY_FIRST + 004 )
#define NOTIFY_PORT_DESTROYED		( NOTIFY_FIRST + 005 )
#define NOTIFY_NO_MORE_SENDERS		( NOTIFY_FIRST + 006 )
#define NOTIFY_LAST			( NOTIFY_FIRST + 015 )

typedef struct {
	msg_header_t	notify_header;
	msg_type_t	notify_type;
	port_t		notify_port;
} notification_t;

#endif	/* MACH_IPC_COMPAT */

#endif	/* _MACH_NOTIFY_H_ */
