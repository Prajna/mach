#
# Mach Operating System
# Copyright (c) 1993-1989 Carnegie Mellon University
# All Rights Reserved.
# 
# Permission to use, copy, modify and distribute this software and its
# documentation is hereby granted, provided that both the copyright
# notice and this permission notice appear in all copies of the
# software, derivative works or modified versions, and any portions
# thereof, and that both notices appear in supporting documentation.
# 
# CARNEGIE MELLON ALLOWS FREE USE OF THIS SOFTWARE IN ITS "AS IS"
# CONDITION.  CARNEGIE MELLON DISCLAIMS ANY LIABILITY OF ANY KIND FOR
# ANY DAMAGES WHATSOEVER RESULTING FROM THE USE OF THIS SOFTWARE.
# 
# Carnegie Mellon requests users of this software to return to
# 
#  Software Distribution Coordinator  or  Software.Distribution@CS.CMU.EDU
#  School of Computer Science
#  Carnegie Mellon University
#  Pittsburgh PA 15213-3890
# 
# any improvements or extensions that they make and grant Carnegie Mellon
# the rights to redistribute these changes.
#
#
# HISTORY
# $Log:	Makefile,v $
# Revision 2.3  93/08/10  15:16:10  mrt
# 	Added pc_sampling man pages.
# 	[93/08/09            mrt]
# 
# Revision 2.2  93/03/26  16:38:19  mrt
# 	Jan-1-93  Mary Thompson (mrt) at Carnegie-Mellon University
# 	Created for Reno make
# 

IDIR		= /man/man2/
MANSECTION	= 2
ILIST		= ${MANPAGES:=.2}

#  This is my best shot, but it doesn't work, since the variable is always
#  defined even if it is empty.
# IDIR = ${.TARGET:M*2:U/man/cat2/}
# so I guess we need another makefile for the cat versions.


# man pages that we install

MANPAGES = ddb host_info host_ipc_statistics host_kernel_version \
	host_processor_set_priv host_processor_sets host_processors \
	mach_host_self mach_msg mach_port_allocate \
	mach_port_allocate_name mach_port_deallocate mach_port_destroy \
	mach_port_extract_right mach_port_get_receive_status \
	mach_port_get_refs mach_port_get_set_status \
	mach_port_insert_right mach_port_mod_refs mach_port_move_member \
	mach_port_names mach_port_rename mach_port_request_notification \
	mach_port_set_mscount mach_port_set_qlimit mach_port_set_seqno \
	mach_port_type mach_ports mach_reply_port mach_task_self \
	mach_thread_self memory_object_copy memory_object_create \
	memory_object_data_error memory_object_data_initialize \
	memory_object_data_provided memory_object_data_request \
	memory_object_data_unavailable memory_object_data_unlock \
	memory_object_data_write memory_object_destroy \
	memory_object_get_attributes memory_object_init \
	memory_object_lock_completed memory_object_lock_request \
	memory_object_server memory_object_set_attributes \
	memory_object_terminate processor_assign processor_control \
	processor_exit processor_get_assignment \
	processor_info processor_set_create processor_set_default \
	processor_set_destroy processor_set_info \
	processor_set_max_priority processor_set_policy_disable \
	processor_set_policy_enable processor_set_tasks \
	processor_set_threads processor_start task_assign \
	task_assign_default task_create task_get_assignment \
	task_enable_pc_sampling  task_disable_pc_sampling \
	task_get_sampled_pcs \
	task_get_special_ports task_info task_priority  \
	task_ras_control task_resume \
	task_set_special_ports task_suspend task_terminate task_threads \
	thread_abort thread_assign thread_create \
	thread_get_special_port thread_get_state thread_info \
	thread_enable_pc_sampling thread_disable_pc_sampling \
	thread_get_sampled_pcs \
	thread_policy thread_priority thread_resume  \
	thread_set_special_port thread_set_state \
	thread_suspend thread_switch thread_terminate \
	thread_wire vm_allocate vm_copy vm_deallocate vm_inherit \
	vm_machine_attribute vm_map vm_protect vm_read vm_region \
	vm_set_default_memory_manager vm_statistics vm_wire vm_write

# Note all the link man pages (those that consist of .so XXX.2)
# must be built after the man page they include.

processor_exit.0: 		processor_control.2
processor_get_assignment.0:	processor_assign.2
processor_set_policy_disable.0: processor_set_policy_enable.2
processor_start.0:		processor_control.2
task_assign_default.0:		task_assign.2
task_set_special_ports.0:	task_get_special_ports.2
thread_set_state.0:		thread_get_state.2
task_disable_pc_sampling.0:	task_enable_pc_sampling.2
task_get_sampled_pcs.0:		task_enable_pc_sampling.2
thread_enable_pc_sampling.0:	task_enable_pc_sampling.2
thread_disable_pc_sampling.0:	task_enable_pc_sampling.2
thread_get_sampled_pcs.0:	task_enable_pc_sampling.2


.include <${RULES_MK}>
