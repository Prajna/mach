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
 * $Log:	syscall_sw.h,v $
 * Revision 2.13  93/08/10  15:12:27  mrt
 * 	Moved device_write traps to avoid conflict with atm traps.
 * 	[93/07/03            cmaeda]
 * 
 * 	Included traps for network interface.
 * 	[93/06/09  15:45:40  jcb]
 * 	Added device traps.
 * 	[93/04/14            cmaeda]
 * 
 * 	Added evc_wait_clear.
 * 	[93/04/14            cmaeda]
 * 
 * Revision 2.12  93/01/14  17:47:42  danner
 * 	Cleanup.
 * 	[92/06/10            pds]
 * 
 * Revision 2.11  92/07/20  13:33:15  cmaeda
 * 	Added set_ras_address (a unixoid trap).
 * 	[92/05/11  14:38:24  cmaeda]
 * 
 * Revision 2.10  92/02/19  16:07:22  elf
 * 	Removed STANDALONE conditionals.
 * 	[92/02/19            elf]
 * 
 * 	Added syscall_thread_depress_abort.
 * 	[92/01/20            rwd]
 * 
 * Revision 2.9  92/01/15  13:44:35  rpd
 * 	Changed MACH_IPC_COMPAT conditionals to default to not present.
 * 
 * Revision 2.8  91/12/13  14:55:06  jsb
 * 	Added evc_wait.
 * 	[91/11/02  17:44:54  af]
 * 
 * Revision 2.7  91/05/14  17:00:32  mrt
 * 	Correcting copyright
 * 
 * Revision 2.6  91/02/05  17:36:20  mrt
 * 	Changed to new Mach copyright
 * 	[91/02/01  17:21:07  mrt]
 * 
 * Revision 2.5  90/09/09  14:33:12  rpd
 * 	Added mach_port_allocate_name trap.
 * 	[90/09/05            rwd]
 * 
 * Revision 2.4  90/06/19  23:00:14  rpd
 * 	Added pid_by_task.
 * 	[90/06/14            rpd]
 * 
 * 	Added mach_port_allocate, mach_port_deallocate, mach_port_insert_right.
 * 	[90/06/02            rpd]
 * 
 * Revision 2.3  90/06/02  14:59:58  rpd
 * 	Removed syscall_vm_allocate_with_pager.
 * 	[90/05/31            rpd]
 * 
 * 	Added map_fd, rfs_make_symlink.
 * 	[90/04/04            rpd]
 * 	Converted to new IPC.
 * 	[90/03/26  22:39:28  rpd]
 * 
 * Revision 2.2  90/05/29  18:36:57  rwd
 * 	New vm/task/threads calls from rfr.
 * 	[90/04/20            rwd]
 * 
 * Revision 2.1  89/08/03  16:03:28  rwd
 * Created.
 * 
 */

#ifndef	_MACH_SYSCALL_SW_H_
#define _MACH_SYSCALL_SW_H_

/*
 *	The machine-dependent "syscall_sw.h" file should
 *	define a macro for
 *		kernel_trap(trap_name, trap_number, arg_count)
 *	which will expand into assembly code for the
 *	trap.
 *
 *	N.B.: When adding calls, do not put spaces in the macros.
 */

#include <mach/machine/syscall_sw.h>

/*
 *	These trap numbers should be taken from the
 *	table in <kern/syscall_sw.c>.
 */

kernel_trap(evc_wait,-17,1)
kernel_trap(evc_wait_clear,-18,1)

kernel_trap(mach_msg_trap,-25,7)
kernel_trap(mach_reply_port,-26,0)
kernel_trap(mach_thread_self,-27,0)
kernel_trap(mach_task_self,-28,0)
kernel_trap(mach_host_self,-29,0)

kernel_trap(swtch_pri,-59,1)
kernel_trap(swtch,-60,0)
kernel_trap(thread_switch,-61,3)
kernel_trap(nw_update,-80,3)
kernel_trap(nw_lookup,-81,2)
kernel_trap(nw_endpoint_allocate,-82,4)
kernel_trap(nw_endpoint_deallocate,-83,1)
kernel_trap(nw_buffer_allocate,-84,2)
kernel_trap(nw_buffer_deallocate,-85,2)
kernel_trap(nw_connection_open,-86,4)
kernel_trap(nw_connection_accept,-87,3)
kernel_trap(nw_connection_close,-88,1)
kernel_trap(nw_multicast_add,-89,4)
kernel_trap(nw_multicast_drop,-90,4)
kernel_trap(nw_endpoint_status,-91,3)
kernel_trap(nw_send,-92,3)
kernel_trap(nw_receive,-93,2)
kernel_trap(nw_rpc,-94,4)
kernel_trap(nw_select,-95,3)


/*
 *	These are syscall versions of Mach kernel calls.
 *	They only work on local tasks.
 */

kernel_trap(syscall_vm_map,-64,11)
kernel_trap(syscall_vm_allocate,-65,4)
kernel_trap(syscall_vm_deallocate,-66,3)

kernel_trap(syscall_task_create,-68,3)
kernel_trap(syscall_task_terminate,-69,1)
kernel_trap(syscall_task_suspend,-70,1)
kernel_trap(syscall_task_set_special_port,-71,3)

kernel_trap(syscall_mach_port_allocate,-72,3)
kernel_trap(syscall_mach_port_deallocate,-73,2)
kernel_trap(syscall_mach_port_insert_right,-74,4)
kernel_trap(syscall_mach_port_allocate_name,-75,3)
kernel_trap(syscall_thread_depress_abort,-76,1)

kernel_trap(syscall_device_writev_request,-39,6)
kernel_trap(syscall_device_write_request,-40,6)

/*
 *	These "Mach" traps are not implemented by the kernel;
 *	the emulation library and Unix server implement them.
 *	But they are traditionally part of libmach, and use
 *	the Mach trap calling conventions and numbering.
 */

kernel_trap(task_by_pid,-33,1)
kernel_trap(pid_by_task,-34,4)
kernel_trap(init_process,-41,0)
kernel_trap(map_fd,-43,5)
kernel_trap(rfs_make_symlink,-44,3)
kernel_trap(htg_syscall,-52,3)
kernel_trap(set_ras_address,-53,2)

/* Traps for the old IPC interface. */

#if	MACH_IPC_COMPAT

kernel_trap(task_self,-10,0)
kernel_trap(thread_reply,-11,0)
kernel_trap(task_notify,-12,0)
kernel_trap(thread_self,-13,0)
kernel_trap(msg_send_trap,-20,4)
kernel_trap(msg_receive_trap,-21,5)
kernel_trap(msg_rpc_trap,-22,6)
kernel_trap(host_self,-55,0)

#endif	/* MACH_IPC_COMPAT */

#endif	/* _MACH_SYSCALL_SW_H_ */
