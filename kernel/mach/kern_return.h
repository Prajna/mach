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
 * $Log:	kern_return.h,v $
 * Revision 2.6  93/01/14  17:43:03  danner
 * 	Cleanup.
 * 	[92/06/10            pds]
 * 
 * Revision 2.5  91/05/14  16:54:08  mrt
 * 	Correcting copyright
 * 
 * Revision 2.4  91/02/05  17:33:06  mrt
 * 	Changed to new Mach copyright
 * 	[91/02/01  17:17:20  mrt]
 * 
 * Revision 2.3  90/08/07  18:00:17  rpd
 * 	Added KERN_MEMORY_PRESENT (not used yet).
 * 	[90/08/06            rpd]
 * 
 * Revision 2.2  90/06/02  14:58:03  rpd
 * 	Added codes for new IPC.
 * 	[90/03/26  22:30:08  rpd]
 * 
 * Revision 2.1  89/08/03  16:02:22  rwd
 * Created.
 * 
 * Revision 2.6  89/02/25  18:13:36  gm0w
 * 	Changes for cleanup.
 * 
 * Revision 2.5  89/02/07  00:52:16  mwyoung
 * Relocated from sys/kern_return.h
 * 
 * Revision 2.4  88/08/24  02:31:47  mwyoung
 * 	Adjusted include file references.
 * 	[88/08/17  02:15:07  mwyoung]
 * 
 * Revision 2.3  88/07/20  16:48:31  rpd
 * Added KERN_NAME_EXISTS.
 * Added KERN_ALREADY_IN_SET, KERN_NOT_IN_SET.
 * Made comments legible.
 * 
 *  3-Feb-88  Michael Young (mwyoung) at Carnegie-Mellon University
 *	Added memory management error conditions.
 *	Documented.
 *
 * 23-Feb-87  Avadis Tevanian (avie) at Carnegie-Mellon University
 *	Deleted kern_return_t casts on error codes so that they may be
 *	used in assembly code.
 *
 * 17-Sep-85  Avadis Tevanian (avie) at Carnegie-Mellon University
 *	Created.
 */
/*
 *	File:	h/kern_return.h
 *	Author:	Avadis Tevanian, Jr.
 *	Date:	1985
 *
 *	Kernel return codes.
 *
 */

#ifndef	_MACH_KERN_RETURN_H_
#define _MACH_KERN_RETURN_H_

#include <mach/machine/kern_return.h>

#define KERN_SUCCESS			0

#define KERN_INVALID_ADDRESS		1
		/* Specified address is not currently valid.
		 */

#define KERN_PROTECTION_FAILURE		2
		/* Specified memory is valid, but does not permit the
		 * required forms of access.
		 */

#define KERN_NO_SPACE			3
		/* The address range specified is already in use, or
		 * no address range of the size specified could be
		 * found.
		 */

#define KERN_INVALID_ARGUMENT		4
		/* The function requested was not applicable to this
		 * type of argument, or an argument
		 */

#define KERN_FAILURE			5
		/* The function could not be performed.  A catch-all.
		 */

#define KERN_RESOURCE_SHORTAGE		6
		/* A system resource could not be allocated to fulfill
		 * this request.  This failure may not be permanent.
		 */

#define KERN_NOT_RECEIVER		7
		/* The task in question does not hold receive rights
		 * for the port argument.
		 */

#define KERN_NO_ACCESS			8
		/* Bogus access restriction.
		 */

#define KERN_MEMORY_FAILURE		9
		/* During a page fault, the target address refers to a
		 * memory object that has been destroyed.  This
		 * failure is permanent.
		 */

#define KERN_MEMORY_ERROR		10
		/* During a page fault, the memory object indicated
		 * that the data could not be returned.  This failure
		 * may be temporary; future attempts to access this
		 * same data may succeed, as defined by the memory
		 * object.
		 */

/*	KERN_ALREADY_IN_SET		11	obsolete */

#define KERN_NOT_IN_SET			12
		/* The receive right is not a member of a port set.
		 */

#define KERN_NAME_EXISTS		13
		/* The name already denotes a right in the task.
		 */

#define KERN_ABORTED			14
		/* The operation was aborted.  Ipc code will
		 * catch this and reflect it as a message error.
		 */

#define KERN_INVALID_NAME		15
		/* The name doesn't denote a right in the task.
		 */

#define	KERN_INVALID_TASK		16
		/* Target task isn't an active task.
		 */

#define KERN_INVALID_RIGHT		17
		/* The name denotes a right, but not an appropriate right.
		 */

#define KERN_INVALID_VALUE		18
		/* A blatant range error.
		 */

#define	KERN_UREFS_OVERFLOW		19
		/* Operation would overflow limit on user-references.
		 */

#define	KERN_INVALID_CAPABILITY		20
		/* The supplied (port) capability is improper.
		 */

#define KERN_RIGHT_EXISTS		21
		/* The task already has send or receive rights
		 * for the port under another name.
		 */

#define	KERN_INVALID_HOST		22
		/* Target host isn't actually a host.
		 */

#define KERN_MEMORY_PRESENT		23
		/* An attempt was made to supply "precious" data
		 * for memory that is already present in a
		 * memory object.
		 */

#endif	/* _MACH_KERN_RETURN_H_ */
