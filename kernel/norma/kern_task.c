/* 
 * Mach Operating System
 * Copyright (c) 1991 Carnegie Mellon University
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
 * $Log:	kern_task.c,v $
 * Revision 2.9  92/03/10  16:28:44  jsb
 * 	Check for null parent task in task_create.
 * 	[92/03/10  13:46:21  jsb]
 * 
 * 	Merged in norma branch changes as of NORMA_MK7.
 * 	[92/03/09  12:50:50  jsb]
 * 
 * Revision 2.7.2.4  92/03/04  16:07:16  jeffreyh
 * 	Converted to use out-of-line forms of task_{get,set}_emulation_vector.
 * 	[92/03/04  14:55:51  jsb]
 * 
 * Revision 2.7.2.3  92/02/21  11:25:15  jsb
 * 	Release send right to memory_object after mapping it in remote task
 * 	in task_copy_vm.
 * 	[92/02/20  10:29:45  jsb]
 * 
 * 	Set copy flag TRUE in r_vm_map call in task_copy_vm.
 * 	This is now practical due to smarter copy strategy management
 * 	in xmm_svm (compared to when it always used MEMORY_OBJECT_COPY_NONE).
 * 	It also keeps xmm_copy from having to deal with data writes without
 * 	having to use an xmm_shadow layer.
 * 	[92/02/11  11:39:23  jsb]
 * 
 * 	Release map reference in task_copy_vm.
 * 	[92/01/22  10:29:57  jsb]
 * 
 * Revision 2.7.2.2  92/01/09  18:46:08  jsb
 * 	Added logic in task_create to alternate task creation between local
 * 	node and a patchable remote node. For testing purposes only.
 * 	[92/01/08  16:41:27  jsb]
 * 
 * 	Use varargs for debugging printfs.
 * 	[92/01/08  10:22:12  jsb]
 * 
 * 	Use remote_host() instead of norma_get_special_port().
 * 	[92/01/04  18:17:46  jsb]
 * 
 * Revision 2.7.2.1  92/01/03  16:38:29  jsb
 * 	Removed unused routine ipc_task_reinit.
 * 	[91/12/28  17:59:18  jsb]
 * 
 * Revision 2.7  91/12/13  13:53:22  jsb
 * 	Changed name of task_create_remote to norma_task_create.
 * 	Added check for local case in norma_task_create.
 * 
 * Revision 2.6  91/12/10  13:26:19  jsb
 * 	Changed printfs to frets.
 * 	[91/12/10  11:34:35  jsb]
 * 
 * Revision 2.5  91/11/14  16:51:51  rpd
 * 	Use new child_node task field in place of task_server_node to decide
 * 	upon what node to create child task. Also add task_set_child_node().
 * 	[91/09/23  09:22:18  jsb]
 * 
 * Revision 2.4  91/08/28  11:16:18  jsb
 * 	Turned off remaining printf.
 * 	[91/08/26  11:16:11  jsb]
 * 
 * 	Added support for remote task creation with inherited memory.
 * 	Remove task creation is accessible either via task_create_remote,
 * 	or task_create with task_server_node patched to some reasonable value.
 * 	[91/08/15  13:55:54  jsb]
 * 
 * Revision 2.3  91/06/17  15:48:07  jsb
 * 	Moved routines here from kern/ipc_tt.c and kern/task.c.
 * 	[91/06/17  11:01:51  jsb]
 * 
 * Revision 2.2  91/06/06  17:08:15  jsb
 * 	First checkin.
 * 	[91/05/25  11:47:39  jsb]
 * 
 */
/*
 *	File:	norma/kern_task.c
 *	Author:	Joseph S. Barrera III
 *
 *	NORMA task support.
 */

#include <norma_task.h>

#include <mach/machine/vm_types.h>
#include <mach/vm_param.h>
#include <mach/task_info.h>
#include <mach/task_special_ports.h>
#include <ipc/ipc_space.h>
#include <kern/mach_param.h>
#include <kern/task.h>
#include <kern/host.h>
#include <kern/thread.h>
#include <kern/zalloc.h>
#include <kern/kalloc.h>
#include <kern/processor.h>
#include <kern/ipc_tt.h>
#include <sys/varargs.h>

ipc_port_t norma_task_server;
decl_simple_lock_data(,norma_task_server_lock)

extern zone_t task_zone;

/*
 * XXX This definition should be elsewhere
 */
norma_node_self(host, node)
	host_t host;
	int *node;
{
	*node = node_self();
	return KERN_SUCCESS;
}

#if	NORMA_TASK

int fff = 1;
int mmm = 0;
int bbb = 0;
extern cnputc();

fret(fmt, va_alist)
	char* fmt;
	va_dcl
{
	va_list	listp;

	if (fff) {
		va_start(listp);
		_doprnt(fmt, &listp, cnputc, 0);
		va_end(listp);
	}
}

mumble(fmt, va_alist)
	char* fmt;
	va_dcl
{
	va_list	listp;

	if (mmm) {
		va_start(listp);
		_doprnt(fmt, &listp, cnputc, 0);
		va_end(listp);
	}
}

babble(fmt, va_alist)
	char* fmt;
	va_dcl
{
	va_list	listp;

	if (bbb) {
		va_start(listp);
		_doprnt(fmt, &listp, cnputc, 0);
		va_end(listp);
	}
}

int task_create_remote_node = -1;
boolean_t task_create_use_remote = FALSE;

kern_return_t task_create(parent_task, inherit_memory, child_task)
	task_t		parent_task;
	boolean_t	inherit_memory;
	task_t		*child_task;		/* OUT */
{
	if (task_create_remote_node != -1) {
		task_create_use_remote = ! task_create_use_remote;
		if (task_create_use_remote) {
			return norma_task_create(parent_task, inherit_memory,
						 task_create_remote_node,
						 child_task);
		} else {
			return task_create_local(parent_task, inherit_memory,
						 child_task);
		}
	}
	if (! parent_task || parent_task->child_node == -1) {
		return task_create_local(parent_task, inherit_memory,
					 child_task);
	} else {
		return norma_task_create(parent_task, inherit_memory,
					 parent_task->child_node,
					 child_task);
	}
}

kern_return_t task_set_child_node(task, child_node)
	task_t	task;
	int	child_node;
{
	if (task == TASK_NULL) {
		return KERN_INVALID_ARGUMENT;
	} else {
		task->child_node = child_node;
		return KERN_SUCCESS;
	}
}

/*
 * This allows us to create a task without providing a parent.
 */
kern_return_t norma_task_allocate(host, task)
	host_t	host;
	task_t	*task;		/* OUT */
{
	babble("norma_task_allocate: called...\n");
	return task_create_local(TASK_NULL, FALSE, task);
}

kern_return_t
task_get_inherited_ports(task, r0, r1, r2, r3, exception, bootstrap)
	task_t task;
	ipc_port_t *r0;
	ipc_port_t *r1;
	ipc_port_t *r2;
	ipc_port_t *r3;
	ipc_port_t *exception;
	ipc_port_t *bootstrap;
{
	if (task == TASK_NULL) {
		return KERN_INVALID_ARGUMENT;
	}
	itk_lock(task);
	*r0 = ipc_port_copy_send(task->itk_registered[0]);
	*r1 = ipc_port_copy_send(task->itk_registered[1]);
	*r2 = ipc_port_copy_send(task->itk_registered[2]);
	*r3 = ipc_port_copy_send(task->itk_registered[3]);
	*exception = ipc_port_copy_send(task->itk_exception);
	*bootstrap = ipc_port_copy_send(task->itk_bootstrap);
	itk_unlock(task);
	return KERN_SUCCESS;
}

kern_return_t
task_set_inherited_ports(task, r0, r1, r2, r3, exception, bootstrap)
	task_t task;
	ipc_port_t r0;
	ipc_port_t r1;
	ipc_port_t r2;
	ipc_port_t r3;
	ipc_port_t exception;
	ipc_port_t bootstrap;
{
	int i;

	if (task == TASK_NULL) {
		return KERN_INVALID_ARGUMENT;
	}
	itk_lock(task);
	for (i = 0; i < 4; i++) {
		if (IP_VALID(task->itk_registered[i])) {
			ipc_port_release_send(task->itk_registered[i]);
		}
	}
	if (IP_VALID(task->itk_exception)) {
		ipc_port_release_send(task->itk_exception);
	}
	if (IP_VALID(task->itk_bootstrap)) {
		ipc_port_release_send(task->itk_bootstrap);
	}
	task->itk_registered[0] = r0;
	task->itk_registered[1] = r1;
	task->itk_registered[2] = r2;
	task->itk_registered[3] = r3;
	task->itk_exception = exception;
	task->itk_bootstrap = bootstrap;
	itk_unlock(task);
	return KERN_SUCCESS;
}

kern_return_t norma_task_create(parent_task, inherit_memory, child_node,
				child_task)
	task_t		parent_task;
	boolean_t	inherit_memory;
	int		child_node;
	task_t		*child_task;		/* OUT */
{
	ipc_port_t remote_task;
	task_t new_task;
	kern_return_t kr;
	int vector_start;
	unsigned int entry_vector_count;
	mach_port_t r0, r1, r2, r3, exception, bootstrap;
	emulation_vector_t *entry_vector;

	if (child_node == node_self()) {
		return task_create_local(parent_task, inherit_memory,
					 child_task);
	}

	babble("task_create: doing %d -> %d\n", node_self(), child_node);

	kr = r_norma_task_allocate(remote_host(child_node), &remote_task);
	if (kr != KERN_SUCCESS) {
		return kr;
	}

	kr = task_get_emulation_vector(parent_task, &vector_start,
				       &entry_vector, &entry_vector_count);
	if (kr != KERN_SUCCESS) {
		fret("task_get_emulation_vector failed: kr %d %x\n", kr, kr);
		return kr;
	}

	kr = r_task_set_emulation_vector(remote_task, vector_start,
					 entry_vector, entry_vector_count);
	if (kr != KERN_SUCCESS) {
		fret("task_set_emulation_vector failed: kr %d %x\n", kr, kr);
		return kr;
	}

	kr = task_get_inherited_ports(parent_task, &r0, &r1, &r2, &r3,
				      &exception, &bootstrap);
	if (kr != KERN_SUCCESS) {
		fret("task_get_inherited_ports failed: kr %d %x\n", kr, kr);
		return kr;
	}

	mumble("%x %x %x %x %x %x\n", r0, r1, r2, r3, exception, bootstrap);
	kr = r_task_set_inherited_ports(remote_task, r0, r1, r2, r3,
					exception, bootstrap);
	if (kr != KERN_SUCCESS) {
		fret("task_set_inherited_ports failed: kr %d %x\n", kr, kr);
		return kr;
	}

	if (inherit_memory) {
		kr = task_copy_vm(parent_task->map, remote_task);
		mumble("task_create: task_copy_vm: (%x)\n", kr);
		if (kr != KERN_SUCCESS) {
			return kr;
		}
	}

	/*
	 * Create a placeholder task for the benefit of convert_task_to_port.
	 * Set new_task->map to VM_MAP_NULL so that task_deallocate will
	 * know that this is only a placeholder task.
	 * XXX decr send-right count?
	 */
	new_task = (task_t) zalloc(task_zone);
	if (new_task == TASK_NULL) {
		panic("task_create: no memory for task structure");
	}

	/* only one ref, for our caller */
	new_task->ref_count = 1;

	new_task->map = VM_MAP_NULL;
	new_task->itk_self = remote_task;
	simple_lock_init(&new_task->lock);

	babble("task_create: did   %d -> %d\n", node_self(), child_node);

	*child_task = new_task;
	return(KERN_SUCCESS);
}

task_copy_vm(from0, to)
	vm_map_t from0;
	ipc_port_t to;
{
	vm_offset_t address;
	vm_size_t size;
	vm_prot_t protection, max_protection;
	vm_inherit_t inheritance;
	boolean_t shared;
	port_t object_name;
	vm_offset_t offset;
	kern_return_t kr;
	vm_map_t from;
	ipc_port_t memory_object;
	
	from = vm_map_fork(from0);
	if (from == VM_MAP_NULL) {
		panic("task_copy_vm: vm_map_fork\n");
	}
	for (address = 0;; address += size) {
		kr = vm_region(from, &address, &size, &protection,
			       &max_protection, &inheritance, &shared,
			       &object_name, &offset);
		if (kr == KERN_NO_SPACE) {
			break;
		}
		switch (inheritance) {
			case VM_INHERIT_NONE:
			break;

			case VM_INHERIT_SHARE:
			fret("task_copy_vm: VM_INHERIT_SHARE!\n");
			return KERN_FAILURE;

			case VM_INHERIT_COPY:
			kr = norma_copy_create(from, address, size,
					       &memory_object);
			if (kr != KERN_SUCCESS) {
				fret("task_cv: copy_create: %d 0x%x\n",
				     kr, kr);
				return kr;
			}
			kr = r_vm_map(to, &address, size, 0, FALSE,
				      memory_object, 0, TRUE,
				      protection, max_protection,
				      inheritance);
			if (kr != KERN_SUCCESS) {
				fret("task_cv: vm_map: %d 0x%x\n",
				     kr, kr);
				return kr;
			}
			ipc_port_release_send(memory_object);
			break;

			default:
			panic("task_copy_vm: inheritance=%d!\n",
			      inheritance);
		}
	}
	vm_map_deallocate(from);
	return KERN_SUCCESS;
}
#else	NORMA_TASK
task_create_local()
{
}

norma_task_allocate()
{
}

task_get_inherited_ports()
{
}

task_set_inherited_ports()
{
}

norma_task_create()
{
}

norma_copy_create()
{
}

#endif	NORMA_TASK
