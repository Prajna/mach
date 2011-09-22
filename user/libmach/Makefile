#
# Mach Operating System
# Copyright (c) 1993,1992,1991,1990,1989 Carnegie Mellon University
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
# 18-May-94  Johannes Helander (jvh) at Helsinki University of Technology
#	Added seqnos_device_reply_server.
#
# 17-Sep-93 Mary Thompson (mrt) at Carnegie Mellon
# 	Added -rbase and -B to the alpha compilation flags.
# 	This is needed so that crt0.o and libmach_sa can be
# 	used by the emulator which lives in a high address
# 	quadrant.
# 
# $Log:	Makefile,v $
# Revision 2.47  93/08/10  15:20:41  mrt
# 	Made device_stamp depend on files included by device/device.defs.
# 	[93/07/12            cmaeda]
# 
# 	Added ATM kernel traps from jcb.
# 	[93/07/03            cmaeda]
# 
# 	Added device traps.
# 	[93/04/14            cmaeda]
# 
# 	Added evc_wait_clear.
# 	[93/04/14            cmaeda]
# 
# Revision 2.46  93/08/03  12:35:24  mrt
# 	[93/08/02  16:56:26  bershad]
# 
# 	[93/07/30  10:27:47  bershad]
# 
# Revision 2.45  93/06/02  20:49:59  rvb
# 	Added NOSTRIP definition.
# 	[93/06/02            mrt]
# 
# Revision 2.44  93/05/13  13:46:45  rvb
# 	Corrected errors.
# 	[93/05/11            mrt]
# 
# Revision 2.43  93/05/11  15:05:41  rvb
# 	LIBRARIES_P only for mips and i386
# 
# Revision 2.42  93/05/10  21:23:22  rvb
# 	Fixed rule for i386 defs.
# 	[93/05/06  09:20:47  af]
# 
# Revision 2.41  93/05/10  19:43:05  rvb
# 	Removed -pg and -DGRPOF for mips and i386.  Added
# 	LIBRARIES_P instead. Now libmach.a libmach_p.a, libmach_sa.a
# 	and libmach_p_sa.a will always get built and exported.
# 	[93/05/09            mrt]
# 
# 	Removed bogus trailing blank from mach_i386.defs rule.
# 	[93/04/08            mrt]
# 
# Revision 2.40  93/05/10  17:50:30  rvb
# 	Fixed typo on mach/i386/mach_i386.defs rule.
# 	[93/05/05  14:40:23  rvb]
# 
# Revision 2.39  93/03/29  17:12:32  mrt
# 	Added -DMACH_IPC_COMPAT and all the little IPC_COMPAT objects
# 	for i386, mips, luna88k, sun3 and vax
# 	[93/03/29            mrt]
# 
# Revision 2.38  93/03/26  18:03:39  mrt
# 	Added moncrt0.o object and crt0.o_CFLAGS
# 	Added -pg and GPROF for i386 and mips
# 	[93/03/22            mrt]
# 
# Revision 2.37  93/02/04  12:58:39  mrt
# 	Added mem.o for the alpha
# 
# Revision 2.36  93/02/01  09:55:52  danner
# 	Sun4 support (from berman).
# 	[93/01/24            danner]
# 
# 	Fixed the mig rules for parallel make.
# 	[93/01/24            mrt]
# 
# 	Update ras calls.
# 	[93/01/20            danner]
# 
# 	Modified for ODE make
# 	[93/01/14            mrt]
# 
# Revision 2.35  93/01/14  18:02:25  danner
# 	Added mem.o to mach arches standalone library.
# 	[93/01/11            danner]
# 	Added new routines and files from default pager.
# 	[92/12/10  20:52:32  af]
# 
# Revision 2.34  92/08/03  18:15:02  jfriedl
# 	Added PURE_M88K_OBJS [jfriedl]
# 	Changed SA_LUNA88K_OBJS to SA_M88K_OBJS [danner]
# 
# Revision 2.33  92/07/20  13:33:30  cmaeda
# 	Added set_ras_address to list of unixoid traps.
# 	Added task_set_ras_pc to list of kernel traps.
# 	[92/05/11  14:40:31  cmaeda]
# 
# Revision 2.32  92/05/04  11:28:37  danner
# 	The unixoid trap cs files now have a define UNIXOID_TRAP 1 in
# 	 them. This is for systems that need to generate different
# 	 syscall stubs for unixoid and pure mach syscalls.
# 	[92/04/14            danner]
# 
# Revision 2.31  92/04/08  23:19:52  rpd
# 	Fixed to remove tmp directory before creating it.
# 	[92/03/18            jvh]
# 
# Revision 2.30  92/03/05  22:46:21  rpd
# 	Added bootstrap functions.
# 	[92/03/02            rpd]
# 
# 	Changed to use double-colon rules for top-level targets.
# 	[92/02/28            rpd]
# 
# Revision 2.29  92/02/20  13:42:51  elf
# 	Added default_pager_paging_file.
# 	[92/02/19  17:37:12  af]
# 
# Revision 2.28  92/02/19  16:07:29  elf
# 	Added thread_depress_abort.
# 	Added i386/memcpy.s.
# 	[92/02/19            elf]
# 
# Revision 2.27  92/02/19  15:10:38  elf
# 	Added mach_error_string.o, htg_vm_map.o, panic.o,
# 	doprnt.o, printf.o, sprintf.o, get_privileged_ports.o.
# 	[92/02/11            rpd]
# 
# Revision 2.26  92/02/19  10:51:30  elf
# 	Added SA_LUNA88K_OBJS. Updated Copyright.
# 	[92/02/19            danner]
# 
# Revision 2.25  92/01/24  00:50:04  rpd
# 	Fixed exc_server.o to have type-checking.
# 	[92/01/23            rpd]
# 
# Revision 2.24  92/01/23  15:21:49  rpd
# 	Revised for new Makefile organization.
# 	[92/01/16            rpd]
# 

DEPENDENCIES	=

VPATH		= ${target_cpu}

LIBRARIES	= libmach.a libmach_sa.a
.if ${target_cpu} == "mips" || ${target_cpu} == "i386"
LIBRARIES_P	= libmach_p.a libmach_sa_p.a
.endif

i386_LNOBJS	= gcrt0.o
mips_LNOBJS	= moncrt0.o 
OBJECTS		= crt0.o ${${target_cpu}_LNOBJS}
NOSTRIP	=


EXPLIB_TARGETS	= ${LIBRARIES:S/^/export_&/g} ${OBJECTS:S/^/export_&/g} 
ILIST		= ${LIBRARIES} ${OBJECTS}
.if defined(LIBRARIES_P)
EXPLIB_TARGETS	+= ${LIBRARIES_P:S/^/export_&/g}
ILIST		+= ${LIBRARIES_P}
.endif
IDIR		= /lib/

mips_CFLAGS	= -DMACH_IPC_COMPAT
i386_CFLAGS	= -DMACH_IPC_COMPAT
sun3_CFLAGS	= -DMACH_IPC_COMPAT
vax_CFLAGS	= -DMACH_IPC_COMPAT
m88k_CFLAGS	= -DMACH_IPC_COMPAT

CFLAGS		=${${target_cpu}_CFLAGS:U}

crt0.o_CFLAGS	= -DCRT0

# Note that ALPHA_?FLAGS is used by osf.mach3.mk and gets added for
# all files independently of CFLAGS. These switches allow code in the
# libraries to be loaded in any of the alpha address quadrants.
# note that code used by the emulator must be able to run in a
# high quadrant.
.if (${HOST_MACHINE} == "ALPHA")  && (!defined(NOT_USING_GCC2) )
ALPHA_CFLAGS    = -rbase
ALPHA_ASFLAGS   = -B
.endif

mips_MIGFLAGS	= -DMACH_IPC_COMPAT
i386_MIGFLAGS	= -DMACH_IPC_COMPAT
sun3_MIGFLAGS	= -DMACH_IPC_COMPAT
vax_MIGFLAGS	= -DMACH_IPC_COMPAT
m88k_MIGFLAGS	= -DMACH_IPC_COMPAT

MIGFLAGS	=${${target_cpu}_MIGFLAGS:U}

# Cause osf.mig.mk to be included 
MIG_USRS	=

# Use library rule for long file names
LIB_LONG_NAMES	=

# device traps from mach/syscall_sw.h

DEVICE_TRAPS = syscall_device_write_request syscall_device_writev_request

ATM_TRAPS = nw_update nw_lookup nw_endpoint_allocate \
        nw_endpoint_deallocate nw_buffer_allocate nw_buffer_deallocate \
	nw_connection_open nw_connection_accept nw_connection_close \
        nw_multicast_add nw_multicast_drop nw_endpoint_status \
        nw_send nw_receive nw_rpc nw_select

# normal Mach system calls from mach/syscall_sw.h

MACH_TRAPS = evc_wait evc_wait_clear \
	mach_msg_trap mach_reply_port mach_thread_self \
	mach_task_self mach_host_self swtch_pri swtch thread_switch \
	$(DEVICE_TRAPS) $(ATM_TRAPS)

# system trap from mach/syscall_sw.h which implement the old ipc interface

MACH_IPC_COMPAT_TRAPS = task_self thread_reply task_notify thread_self \
	msg_send_trap msg_receive_trap msg_rpc_trap host_self 

I386_IPC_COMPAT_TRAPS = ${MACH_IPC_COMPAT_TRAPS}
VAX_IPC_COMPAT_TRAPS = ${MACH_IPC_COMPAT_TRAPS}
SUN3_IPC_COMPAT_TRAPS = ${MACH_IPC_COMPAT_TRAPS}
PMAX_IPC_COMPAT_TRAPS = ${MACH_IPC_COMPAT_TRAPS}
LUNA88K_IPC_COMPAT_TRAPS = ${MACH_IPC_COMPAT_TRAPS}

IPC_COMPAT_TRAPS = ${${TARGET_MACHINE}_IPC_COMPAT_TRAPS}

# system calls from mach/syscall_sw.h which the kernel doesn't implement

UNIXOID_TRAPS = task_by_pid pid_by_task init_process map_fd \
	rfs_make_symlink htg_syscall 

# routines from mach/mach.defs which have fast syscall versions

MACH_SYSCALLS = vm_allocate vm_map vm_deallocate task_create \
	task_terminate task_suspend task_set_special_port \
	thread_depress_abort

MS_MACH_SYSCALLS = ${MACH_SYSCALLS:S/^/ms_/g}
MIG_MACH_SYSCALLS = ${MACH_SYSCALLS:S/^/mig_/g}

# compat routines from mach/mach.defs, on for some machine types

IPC_COMPAT_ROUTINES = port_allocate port_deallocate \
	port_extract_receive port_extract_send \
	port_insert_receive port_insert_send \
	port_names port_rename port_set_add \
	port_set_allocate port_set_backlog \
	port_set_backup port_set_deallocate \
	port_set_remove port_set_status \
	port_status port_type


COMPAT_I386_ROUTINES = ${IPC_COMPAT_ROUTINES}
COMPAT_M88K_ROUTINES = ${IPC_COMPAT_ROUTINES}
COMPAT_MIPS_ROUTINES = ${IPC_COMPAT_ROUTINES}
COMPAT_SUN3_ROUTINES = ${IPC_COMPAT_ROUTINES}
COMPAT_VAX_ROUTINES  = ${IPC_COMPAT_ROUTINES}

# other routines from mach/mach.defs

MACH_ROUTINES = task_threads thread_terminate vm_protect vm_inherit \
	vm_read vm_write vm_copy vm_region vm_statistics \
	mach_ports_register mach_ports_lookup \
	memory_object_data_provided memory_object_data_unavailable \
	memory_object_get_attributes vm_set_default_memory_manager \
	memory_object_lock_request task_get_emulation_vector \
	task_set_emulation_vector task_resume task_get_special_port \
	task_info thread_create thread_suspend thread_resume \
	thread_abort thread_get_state thread_set_state \
	thread_get_special_port thread_set_special_port thread_info \
	task_set_emulation memory_object_data_error \
	memory_object_set_attributes memory_object_destroy \
	memory_object_data_supply memory_object_ready \
	memory_object_change_attributes vm_machine_attribute \
	task_ras_control ${COMPAT_${TARGET_CPU}_ROUTINES}

# mach/mach.defs is full so we spill over to mach4.defs

MACH4_ROUTINES = task_enable_pc_sampling task_disable_pc_sampling \
	task_get_sampled_pcs thread_enable_pc_sampling \
	thread_disable_pc_sampling thread_get_sampled_pcs

# routines from mach/mach_port.defs which have fast syscall versions

MACH_PORT_SYSCALLS = mach_port_allocate mach_port_allocate_name \
	mach_port_deallocate mach_port_insert_right

MS_MACH_PORT_SYSCALLS = ${MACH_PORT_SYSCALLS:S/^/ms_/g}
MIG_MACH_PORT_SYSCALLS = ${MACH_PORT_SYSCALLS:S/^/mig_/g}

# other routines from mach/mach_port.defs

MACH_PORT_ROUTINES = mach_port_names mach_port_type mach_port_rename \
	mach_port_destroy mach_port_get_refs mach_port_mod_refs \
	old_mach_port_get_receive_status mach_port_set_qlimit \
	mach_port_set_mscount mach_port_get_set_status \
	mach_port_move_member mach_port_request_notification \
	mach_port_extract_right mach_port_get_receive_status \
	mach_port_set_seqno

# routines from device/device.defs

DEVICE_ROUTINES = device_open device_close device_write \
	device_write_inband device_read device_read_inband \
	device_set_status device_get_status device_set_filter device_map

# routines from mach/mach_host.defs

MACH_HOST_ROUTINES = host_processors host_info processor_info \
	processor_start processor_exit processor_control \
	processor_set_default xxx_processor_set_default_priv \
	processor_set_create processor_set_destroy processor_set_info \
	processor_assign processor_get_assignment thread_assign \
	thread_assign_default thread_get_assignment task_assign \
	task_assign_default task_get_assignment host_kernel_version \
	thread_priority thread_max_priority task_priority \
	processor_set_max_priority thread_policy \
	processor_set_policy_enable processor_set_policy_disable \
	processor_set_tasks processor_set_threads host_processor_sets \
	host_processor_set_priv host_set_time host_adjust_time \
	host_get_time host_reboot vm_wire thread_wire \
	host_get_boot_info

# routines from mach_debug/mach_debug.defs

MACH_DEBUG_ROUTINES = host_zone_info mach_port_get_srights \
	host_ipc_hash_info host_ipc_marequest_info mach_port_space_info \
	mach_port_dnrequest_info mach_vm_region_info mach_vm_object_info \
	mach_vm_object_pages host_stack_usage processor_set_stack_usage \
	host_virtual_physical_table_info host_load_symbol_table \
	mach_port_kernel_object

# routines from mach/default_pager.defs

DPAGER_ROUTINES = default_pager_object_create default_pager_info \
	default_pager_objects default_pager_object_pages \
	default_pager_paging_file  default_pager_register_fileserver


# routines from mach/default_pager_helper.defs

DP_HELPER_ROUTINES = dp_helper_paging_space

# routines from mach/bootstrap.defs

BOOTSTRAP_ROUTINES = bootstrap_privileged_ports

# all system calls from mach/syscall_sw.h

PURE_TRAPS = ${MACH_TRAPS} \
	${MACH_SYSCALLS:S/^/syscall_&/g} \
	${MACH_PORT_SYSCALLS:S/^/syscall_&/g}

ALL_TRAPS = ${PURE_TRAPS} ${UNIXOID_TRAPS} ${IPC_COMPAT_TRAPS}


MACH_OBJS = mach_init.o mach_msg.o mach_msg_send.o mach_msg_receive.o \
	mach_msg_destroy.o  mach_msg_server.o

# objects that implement the old ipc interface

MACH_IPC_COMPAT_OBJS = msg.o

I386_IPC_COMPAT_OBJS = ${MACH_IPC_COMPAT_OBJS}
VAX_IPC_COMPAT_OBJS = ${MACH_IPC_COMPAT_OBJS}
SUN3_IPC_COMPAT_OBJS = ${MACH_IPC_COMPAT_OBJS}
PMAX_IPC_COMPAT_OBJS = ${MACH_IPC_COMPAT_OBJS}
LUNA88K_IPC_COMPAT_OBJS = ${MACH_IPC_COMPAT_OBJS}

IPC_COMPAT_OBJS = ${${TARGET_MACHINE}_IPC_COMPAT_OBJS}

MIG_OBJS = mig_support.o mig_strncpy.o \
	mig_allocate.o mig_deallocate.o mig_reply_setup.o

USER_OBJS = slot_name.o error_codes.o mach_error_string.o

LIBC_OBJS = sbrk.o fork.o Ovfork.o


PURE_I386_OBJS = mach_i386_user.o
PURE_M88K_OBJS = _setjmp.o

PURE_OBJS = ${MACH_OBJS} ${MIG_OBJS} ${USER_OBJS} \
	${PURE_${TARGET_CPU}_OBJS} htg_vm_map.o device_request_user.o\
	device_reply_server.o seqnos_device_reply_server.o \
	exc_user.o exc_server.o mach_norma_user.o \
	memory_object_server.o seqnos_memory_object_server.o \
	memory_object_default_server.o seqnos_memory_object_default_server.o \
	notify_user.o notify_server.o seqnos_notify_server.o \
	mach_server.o mach_port_server.o mach_host_server.o \
	mach_debug_server.o mach_norma_server.o device_server.o \
	default_pager_server.o default_pager_helper_server.o \
	memory_object_user.o memory_object_default_user.o \
	${PURE_TRAPS:S/$/&.o/g} \
	${MS_MACH_SYSCALLS:S/$/&.o/g} ${MIG_MACH_SYSCALLS:S/$/&.o/g} \
	${MS_MACH_PORT_SYSCALLS:S/$/.o/g} ${MIG_MACH_PORT_SYSCALLS:S/$/&.o/g} \
	${MACH_ROUTINES:S/$/&.o/g} ${MACH_PORT_ROUTINES:S/$/&.o/g} \
	${MACH4_ROUTINES:S/$/&.o/g} \
	${DEVICE_ROUTINES:S/$/&.o/g} ${MACH_HOST_ROUTINES:S/$/&.o/g} \
	${MACH_DEBUG_ROUTINES:S/$/&.o/g} ${DPAGER_ROUTINES:S/$/&.o/g} \
	bootstrap_server.o ${BOOTSTRAP_ROUTINES:S/$/&.o/g} \
	${DP_HELPER_ROUTINES/.*/&.o}

UNIXOID_OBJS = mach_init_ports.o mach_error.o \
	${LIBC_OBJS} ${UNIXOID_TRAPS:S/$/&.o/g} \
	${IPC_COMPAT_OBJS} ${IPC_COMPAT_TRAPS:S/$/&.o/g}

SA_ALPHA_OBJS   = mem.o
SA_VAX_OBJS     = urem.o udiv.o mem.o
SA_SUN3_OBJS    = start_float.o mem.o
SA_I386_OBJS    = gcc.o memcpy.o bcopy.o
SA_I860_OBJS    = pre_crt0.o
SA_M88K_OBJS	= gcc.o mem.o
SA_MIPS_OBJS	= mem.o
SA_SUN4_OBJS    = div.o main.o memcpy.o multiply.o ptr_call.o rem.o \
			stret1.o stret2.o stret4.o umultiply.o

SA_OBJS = mach_init_sa.o ${SA_${TARGET_CPU}_OBJS} \
	exit.o _setjmp.o bzero.o bcopy.o \
	strcmp.o strlen.o strncpy.o strcat.o strcpy.o \
	doprnt.o printf.o sprintf.o panic.o \
	get_privileged_ports.o

libmach.a_OFILES 	=  ${PURE_OBJS} ${UNIXOID_OBJS}
libmach_sa.a_OFILES	=  ${PURE_OBJS}  ${SA_OBJS}

.include <${RULES_MK}> 

.if ( ${TARGET_MACHINE} == "I386" )
gcrt0.o: crt0.o
	${RM} -f ${.TARGET}
	${LN} -s ${crt0.o:P} ${.TARGET}
.endif

.if ( ${TARGET_MACHINE} == "PMAX" )
moncrt0.o: crt0.o
	${RM} -f ${.TARGET}
	${LN} -s ${crt0.o:P} ${.TARGET}
.endif

# mig rules to generate intermediate source files in the object directory
# Make all the mig targets depend on a single "timestamp" source. Othewise
# the mig rule gets run once for each target.

${UNIXOID_TRAPS:S/$/&.s/g} : mach/syscall_sw.h
	{ echo '#define UNIXOID_TRAP 1'; \
 	  echo '#include <mach/machine/syscall_sw.h>'; \
	  egrep '^kernel_trap\($*,' ${mach/syscall_sw.h:P}; } > ${.TARGET}

${PURE_TRAPS:S/$/&.s/g} ${IPC_COMPAT_TRAPS:S/$/&.s/g} : mach/syscall_sw.h
	{ echo '#include <mach/machine/syscall_sw.h>'; \
	  egrep '^kernel_trap\($*,' ${mach/syscall_sw.h:P}; } > ${.TARGET}

MIG_SYSCALL_SRCS = ${MIG_MACH_SYSCALLS:S/$/&.c/g} \
		   ${MIG_MACH_PORT_SYSCALLS:S/$/&.c/g}

${MIG_SYSCALL_SRCS} : ${.TARGET:S/^mig_//g}
	name=${.PREFIX:S/mig_//}; \
	sed -e "s/$${name}/mig_$${name}/g" < $${name}.c > mig_$${name}.c

MACH_USER_SRCS = ${MACH_ROUTINES:S/$/&.c/g} ${MACH_SYSCALLS:S/$/&.c/g}
MACH_SRCS = mach_interface.h ${MACH_USER_SRCS} mach_server.c

${MACH_SRCS} : mach_stamp
mach_stamp: mach/mach.defs
	${MIG} ${_MIGFLAGS_} -i ./ -user '${MACH_USER_SRCS}' \
		-header mach_interface.h \
		-server mach_server.c  ${mach/mach.defs:P} && \
	touch mach_stamp

MACH4_USER_SRCS = ${MACH4_ROUTINES:S/$/&.c/g}
MACH4_SRCS = mach4_interface.h ${MACH4_USER_SRCS} mach4_server.c

${MACH4_SRCS} : mach4_stamp
mach4_stamp: mach/mach4.defs
	${MIG} ${_MIGFLAGS_} -i ./ -user '${MACH4_USER_SRCS}' \
		-header mach4_interface.h \
		-server mach4_server.c  ${mach/mach4.defs:P} && \
	touch mach4_stamp


MACH_PORT_USER_SRCS = ${MACH_PORT_ROUTINES:S/$/&.c/g} \
			${MACH_PORT_SYSCALLS:S/$/&.c/g}
MACH_PORT_SRCS = mach_port.h ${MACH_PORT_USER_SRCS} mach_port_server.c

${MACH_PORT_SRCS} : mach_port_stamp
mach_port_stamp:  mach/mach_port.defs
	${MIG} ${_MIGFLAGS_} -i ./ -user '${MACH_PORT_USER_SRCS}' \
		-server mach_port_server.c  ${mach/mach_port.defs:P} && \
	touch mach_port_stamp

DEVICE_USER_SRCS = ${DEVICE_ROUTINES:S/$/&.c/g}
DEVICE_SRCS = device.h ${DEVICE_USER_SRCS} device_server.c

${DEVICE_SRCS} :  device_stamp
device_stamp: device/device.defs device/device_types.defs device/net_status.h
	${MIG} ${_MIGFLAGS_} -i ./ -user '${DEVICE_USER_SRCS}' \
		-server device_server.c  ${device/device.defs:P} && \
	touch device_stamp

MACH_HOST_USER_SRCS = ${MACH_HOST_ROUTINES:S/$/&.c/g}
MACH_HOST_SRCS = mach_host.h ${MACH_HOST_USER_SRCS} mach_host_server.c

${MACH_HOST_SRCS} : mach_host_stamp
mach_host_stamp: mach/mach_host.defs
	${MIG} ${_MIGFLAGS_} -i ./ -user '${MACH_HOST_USER_SRCS}' \
		-server mach_host_server.c  ${mach/mach_host.defs:P} && \
	touch mach_host_stamp

MACH_DEBUG_USER_SRCS = ${MACH_DEBUG_ROUTINES:S/$/&.c/g}
MACH_DEBUG_SRCS = mach_debug.h ${MACH_DEBUG_USER_SRCS} mach_debug_server.c

${MACH_DEBUG_SRCS} : mach_debug_stamp
mach_debug_stamp: mach_debug/mach_debug.defs
	${MIG} ${_MIGFLAGS_} -i ./ -user '${MACH_DEBUG_USER_SRCS}' \
		-server mach_debug_server.c  ${mach_debug/mach_debug.defs:P} && \
	touch mach_debug_stamp

DPAGER_USER_SRCS = ${DPAGER_ROUTINES:S/$/&.c/g}
DPAGER_SRCS = default_pager.h ${DPAGER_USER_SRCS} default_pager_server.c

${DPAGER_SRCS} : default_pager_stamp
default_pager_stamp: mach/default_pager.defs
	${MIG} ${_MIGFLAGS_} -i ./ -user '${DPAGER_USER_SRCS}' \
		-server default_pager_server.c  ${mach/default_pager.defs:P} && \
	touch default_pager_stamp

BOOTSTRAP_USER_SRCS = ${BOOTSTRAP_ROUTINES:S/$/&.c/g}
BOOTSTRAP_SRCS = bootstrap.h ${BOOTSTRAP_USER_SRCS} bootstrap_server.c

${BOOTSTRAP_SRCS} : bootstrap_stamp
bootstrap_stamp: mach/bootstrap.defs
	${MIG} ${_MIGFLAGS_} -i ./ -user '${BOOTSTRAP_USER_SRCS}' \
		-server bootstrap_server.c  ${mach/bootstrap.defs:P} && \
	touch bootstrap_stamp


DP_HELPER_USER_SRCS = ${DP_HELPER_ROUTINES:S/$/&.c/g}
DP_HELPER_SRCS = default_pager_helper.h ${DP_HELPER_USER_SRCS} \
		 default_pager_helper_server.c

${DP_HELPER_SRCS} : default_pager_helper_stamp 
default_pager_helper_stamp: mach/default_pager_helper.defs
	${MIG} ${_MIGFLAGS_} -i ./ -user '${DP_HELPER_USER_SRCS}' \
		-server default_pager_helper_server.c \
		 ${mach/default_pager_helper.defs:P} && \
	touch default_pager_helper_stamp

EXC_SRCS = exc.h exc_user.c exc_server.c

${EXC_SRCS} : exc_stamp
exc_stamp: mach/exc.defs
	${MIG} ${_MIGFLAGS_}  -user exc_user.c \
		-server exc_server.c  ${mach/exc.defs:P} && \
	touch exc_stamp

NOTIFY_SRCS = notify_user.h notify_user.c notify_server.c

${NOTIFY_SRCS} : notify_stamp
notify_stamp: mach/notify.defs
	${MIG} ${_MIGFLAGS_}  -user notify_user.c \
		-header notify_user.h \
		-server notify_server.c  ${mach/notify.defs:P} && \
	touch notify_stamp

seqnos_notify_server.c : mach/notify.defs
	${MIG} ${_MIGFLAGS_}  -DSEQNOS  -user /dev/null \
		-header /dev/null \
		-server seqnos_notify_server.c  ${mach/notify.defs:P} 

MEMORY_OBJECT_SRCS = memory_object_user.h \
	memory_object_user.c memory_object_server.c

${MEMORY_OBJECT_SRCS} : memory_object_stamp
memory_object_stamp: mach/memory_object.defs
	${MIG} ${_MIGFLAGS_}  -user memory_object_user.c \
		-header memory_object_user.h \
		-server memory_object_server.c  ${mach/memory_object.defs:P} && \
	touch memory_object_stamp

seqnos_memory_object_server.c : mach/memory_object.defs
	${MIG} ${_MIGFLAGS_} -DSEQNOS  -user /dev/null \
		-header /dev/null \
		-server seqnos_memory_object_server.c  ${mach/memory_object.defs:P}

MEMORY_OBJECT_DEFAULT_SRCS = memory_object_default.h \
	memory_object_default_user.c memory_object_default_server.c

${MEMORY_OBJECT_DEFAULT_SRCS} : memory_object_default_stamp 
memory_object_default_stamp: mach/memory_object_default.defs
	${MIG} ${_MIGFLAGS_}  -user memory_object_default_user.c \
		-header memory_object_default.h \
		-server memory_object_default_server.c  ${mach/memory_object_default.defs:P} && \
	touch memory_object_default_stamp 


seqnos_memory_object_default_server.c : mach/memory_object_default.defs
	${MIG} ${_MIGFLAGS_}  -DSEQNOS -user /dev/null \
		-header /dev/null \
		-server seqnos_memory_object_default_server.c  ${mach/memory_object_default.defs:P}


DEVICE_REQUEST_SRCS = device_request.h device_request_user.c

${DEVICE_REQUEST_SRCS} : device_request_stamp
device_request_stamp: device/device_request.defs
	${MIG} ${_MIGFLAGS_}  -user device_request_user.c \
		-server /dev/null ${device/device_request.defs:P} && \
	touch device_request_stamp

DEVICE_REPLY_SRCS = device_reply_server.c

${DEVICE_REPLY_SRCS} : device/device_reply.defs
	${MIG} ${_MIGFLAGS_} -user /dev/null \
		-header /dev/null \
		-server device_reply_server.c ${device/device_reply.defs:P} ; \

seqnos_device_reply_server.c : device/device_reply.defs
	${MIG} ${_MIGFLAGS_}  -DSEQNOS -user /dev/null \
		-header /dev/null \
		-server seqnos_device_reply_server.c ${device/device_reply.defs:P}

NORMA_SRCS = mach_norma.h mach_norma_user.c mach_norma_server.c

${NORMA_SRCS} : mach_norma_stamp
mach_norma_stamp: mach/mach_norma.defs
	${MIG} ${_MIGFLAGS_}  -user mach_norma_user.c \
		-server mach_norma_server.c  ${mach/mach_norma.defs:P} && \
	touch mach_norma_stamp

MACH_I386_SRCS = mach_i386.h mach_i386_user.c mach_i386_server.c

${MACH_I386_SRCS} : mach_i386_stamp
mach_i386_stamp: mach/i386/mach_i386.defs
	${MIG} ${_MIGFLAGS_}  -user mach_i386_user.c \
		-server mach_i386_server.c  ${mach/i386/mach_i386.defs:P} && \
	touch mach_i386_stamp

# This is a funny one.  We do the mig into a temporary directory and
# then pick out the one file that we actually want.  We use a
# temporary directory to avoid interfering with the files normally
# generated from mach/mach.defs.  We use htg_vm_map.defs instead of
# using mach/mach.defs directly so that the dependencies show up in
# htg_vm_map.defs.d instead of overwriting mach.defs.d.

htg_vm_map.c : htg_vm_map.defs
	-${RM} -rf tmp
	mkdir tmp
	${MIG} ${_MIGFLAGS_} -DEMULATOR -i tmp/ \
		-user htg_vm_map.c -header tmp/mach_interface.h \
		 -server /dev/null htg_vm_map.defs
	mv tmp/htg_vm_map.c htg_vm_map.c

htg_vm_map.defs : mach/mach.defs
	echo "#include <mach/mach.defs>" > ${.TARGET}

htg_vm_map.o : mach_interface.h

# mig generated code normally does stringent type-checking of received
# messages.  We disable this type-checking (for performance reasons)
# on messages that are received from the kernel, because the kernel
# is trusted to send correct messages.  Exceptions:
#	mach_debug user stubs - the mach_debug interface changes often,
#		so type-checking is useful to catch version skew
#	exc_server.c - users also have access to exception ports and
#		can make exception RPCs directly

NO_TYPE_CHECK_SRCS = ${MACH_USER_SRCS} ${MACH_PORT_USER_SRCS} \
	${MIG_SYSCALL_SRCS} ${DEVICE_USER_SRCS} ${MACH_HOST_USER_SRCS} \
	${MACH4_USER_SRCS} \
	notify_server.c seqnos_notify_server.c \
	memory_object_server.c seqnos_memory_object_server.c \
	memory_object_default_server.c \
	seqnos_memory_object_default_server.c device_request_user.c \
	device_reply_server.c seqnos_device_reply_server.c  mach_norma_user.c \
	mach_i386_user.c htg_vm_map.c bootstrap_server.c ${BOOTSTRAP_USER_SRCS}


NO_TYPE_CHECK_OBJS = ${NO_TYPE_CHECK_SRCS:S/.c$/.o/g}

${NO_TYPE_CHECK_OBJS} : ${.PREFIX}.c
	@echo ""
	${_CC_} -c -DTypeCheck=0 ${_CCFLAGS_} ${.PREFIX}.c

# Files to be deleted in addition to .o files by the clean rule

GARBAGE =  depend.mk depend.mk.old \
	${UNIXOID_TRAPS:S/$/&.s/g} ${PURE_TRAPS:S/$/&.s/g} \
	${MIG_SYSCALL_SRCS} \
	${MACH_SRCS} mach_stamp \
	${MACH4_SRCS} mach4_stamp \
	${MACH_PORT_SRCS} mach_port_stamp \
	${DEVICE_SRCS} device_stamp \
	${MACH_HOST_SRCS}  mach_host_stamp \
	${MACH_DEBUG_SRCS} mach_debug_stamp \
	${DPAGER_SRCS} default_pager_stamp \
	${BOOTSTRAP_SRCS} bootstrap_stamp \
	${DP_HELPER_SRCS} default_pager_helper_stamp \
	${EXC_SRCS} exc_stamp \
	${NOTIFY_SRCS} notify_stamp \
	seqnos_notify_server.c \
	${MEMORY_OBJECT_SRCS} memory_object_stamp \
	seqnos_memory_object_server.c \
	${MEMORY_OBJECT_DEFAULT_SRCS} memory_object_default_stamp \
	seqnos_memory_object_default_server.c \
	${DEVICE_REQUEST_SRCS} device_request_stamp \
	${NORMA_SRCS} mach_norma_stamp \
	${DEVICE_REPLY_SRCS} seqnos_device_reply_server.c \
	htg_vm_map.c htg_vm_map.defs ${MACH_I386_SRCS} mach_i386_stamp 

