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
 * 11-Feb-94  Johannes Helander (jvh) at Helsinki University of Technology
 *	Wrapped code with ifdef MACH_IPC_COMPAT.
 *
 * $Log:	msg.c,v $
 * Revision 2.6  93/03/29  14:42:36  mrt
 * 	Resurrected and updated to match new types.
 * 	[93/03/29            pds]
 * 
 * Revision 2.1  89/08/03  17:05:52  rwd
 * Created.
 * 
 * 21-Oct-88  Richard Draves (rpd) at Carnegie-Mellon University
 *	Added msg_send wrapper, which handles SEND_INTERRUPT.
 *	Fixed bug in msg_rpc wrapper; it gave the wrong size to	msg_receive_.
 *	Converted to first try the new *_trap calls and fall back on
 *	the (renamed) *_old calls if they don't work.
 *
 * 19-May-87  Mary Thompson (mrt) at Carnegie-Mellon University
 *	Fixed the test for interupts in msg_rpc_.
 *	(Copied from mwyoung's version.)
 */

#if MACH_IPC_COMPAT

#include <mach/kern_return.h>
#include <mach/message.h>

msg_return_t	msg_send(header, option, timeout)
	msg_header_t	*header;
	msg_option_t	option;
	msg_timeout_t	timeout;
{
	register
	msg_return_t	result;


	result = msg_send_trap(header, option, header->msg_size, timeout);
	if (result == SEND_SUCCESS)
		return result;

	if ((result == SEND_INTERRUPTED) && !(option & SEND_INTERRUPT))
		do
			result = msg_send_trap(header, option,
					       header->msg_size, timeout);
		while (result == SEND_INTERRUPTED);

	return result;
}

msg_return_t	msg_receive(header, option, timeout)
	msg_header_t	*header;
	msg_option_t	option;
	msg_timeout_t	timeout;
{
	register
	msg_return_t	result;

	result = msg_receive_trap(header, option, header->msg_size,
				  header->msg_local_port, timeout);
	if (result == RCV_SUCCESS)
		return result;

	if ((result == RCV_INTERRUPTED) && !(option & RCV_INTERRUPT))
		do
			result = msg_receive_trap(header, option,
						  header->msg_size,
						  header->msg_local_port,
						  timeout);
		while (result == RCV_INTERRUPTED);

	return result;
}

msg_return_t	msg_rpc(header, option, rcv_size, send_timeout, rcv_timeout)
	msg_header_t	*header;
	msg_option_t	option;
	msg_size_t	rcv_size;
	msg_timeout_t	send_timeout;
	msg_timeout_t	rcv_timeout;
{
	register
	msg_return_t	result;
	
	result = msg_rpc_trap(header, option, header->msg_size,
			      rcv_size, send_timeout, rcv_timeout);
	if (result == RPC_SUCCESS)
		return result;

	if ((result == SEND_INTERRUPTED) && !(option & SEND_INTERRUPT)) {
		do
			result = msg_rpc_trap(header, option,
					      header->msg_size, rcv_size,
					      send_timeout, rcv_timeout);
		while (result == SEND_INTERRUPT);
	}

	if ((result == RCV_INTERRUPTED) && !(option & RCV_INTERRUPT))
		do
			result = msg_receive_trap(header, option, rcv_size,
						  header->msg_local_port,
						  rcv_timeout);
		while (result == RCV_INTERRUPTED);

	return result;
}

#endif /* MACH_IPC_COMPAT */
