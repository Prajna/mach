/* 
 * Mach Operating System
 * Copyright (c) 1991,1990 Carnegie Mellon University
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
 * $Log:	io_emulate.c,v $
 * Revision 2.6  92/01/03  20:06:54  dbg
 * 	Move AT-bus IO port emulation to i386at/iopl.c.
 * 	[92/01/02            dbg]
 * 
 * 	Rewrite to map IO ports into 'faulting' thread.
 * 	Move actual instruction decoding to trap.c.
 * 	Emulate certain IO ports for AT bus machines.
 * 	[91/11/09            dbg]
 * 
 * Revision 2.5  91/05/14  16:09:24  mrt
 * 	Correcting copyright
 * 
 * Revision 2.4  91/02/05  17:12:27  mrt
 * 	Changed to new Mach copyright
 * 	[91/02/01  17:35:06  mrt]
 * 
 * Revision 2.3  90/06/02  14:48:35  rpd
 * 	Converted to new IPC.
 * 	[90/06/01            rpd]
 * 
 * Revision 2.2  90/05/21  13:26:32  dbg
 * 	Created.
 * 	[90/05/09            dbg]
 * 
 */
#include <platforms.h>

#include <mach/boolean.h>
#include <mach/port.h>
#include <kern/thread.h>
#include <kern/task.h>

#include <ipc/ipc_port.h>
#include <ipc/ipc_space.h>
#include <ipc/ipc_right.h>
#include <ipc/ipc_object.h>
#include <ipc/ipc_entry.h>

#include <device/dev_hdr.h>

#include <i386/thread.h>
#include <i386/io_port.h>
#include <i386/io_emulate.h>

extern ipc_port_t	iopl_device_port;
extern device_t		iopl_device;

int
emulate_io(regs, opcode, io_port)
	struct i386_saved_state *regs;
	int	opcode;
	int	io_port;
{
	thread_t	thread = current_thread();

#if	AT386
	if (iopl_emulate(regs, opcode, io_port))
	    return EM_IO_DONE;
#endif	/* AT386 */

	if (iopb_check_mapping(thread, iopl_device))
	    return EM_IO_ERROR;

	/*
	 *	Check for send rights to the IOPL device port.
	 */
	if (iopl_device_port == IP_NULL)
	    return EM_IO_ERROR;
	{
	    ipc_space_t	space = current_space();
	    mach_port_t	name;
	    ipc_entry_t	entry;
	    boolean_t	has_rights = FALSE;

	    is_write_lock(space);
	    assert(space->is_active);

	    if (ipc_right_reverse(space, (ipc_object_t) iopl_device_port,
				  &name, &entry)) {
		/* iopl_device_port is locked and active */
		if (entry->ie_bits & MACH_PORT_TYPE_SEND)
		    has_rights = TRUE;
		ip_unlock(iopl_device_port);
	    }

	    is_write_unlock(space);
	    if (!has_rights) {
		return EM_IO_ERROR;
	    }
	}


	/*
	 * Map the IOPL port set into the thread.
	 */

	if (i386_io_port_add(thread, iopl_device)
		!= KERN_SUCCESS)
	    return EM_IO_ERROR;

	/*
	 * Make the thread use its IO_TSS to get the IO permissions;
	 * it may not have had one before this.
	 */
	switch_ktss(thread->pcb);

	return EM_IO_RETRY;
}
