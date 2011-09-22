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
# Revision 2.3  93/03/26  17:53:53  mrt
# 	Added MACH_IPC_COMPAT for old architectures so that the
# 	netmsgserver can be built.
# 	[93/03/26            mrt]
# 
# Revision 2.2  93/02/04  18:41:16  mrt
# 	Add mach4.defs
# 	Created for ODE make -mrt
# 	[93/01/21            danner]
# 
# Revision 2.36  93/01/14  17:09:46  danner
# 	Added mach/mig_support.h.
# 	[92/12/14            pds]
# 	Alpha support.
# 	[92/11/30            af]
# 
#	Makefile to export and install then include files  from the
#	mk tree that go into /include/mach
#

DEPENDENCIES	=
VPATH		= ../../kernel/mach:../../bootstrap/mach

SUBDIRS		= ${KERN_MACHINE_DIR}

EXPINC_SUBDIRS	= ${SUBDIRS}
EXPORT_USING_TAR=


pmax_CFLAGS	= -DMACH_IPC_COMPAT
i386_CFLAGS	= -DMACH_IPC_COMPAT
sun3_CFLAGS	= -DMACH_IPC_COMPAT
vax_CFLAGS	= -DMACH_IPC_COMPAT
luna88k_CFLAGS	= -DMACH_IPC_COMPAT

CFLAGS		=${${target_machine}_CFLAGS:U}

crt0.o_CFLAGS	= -DCRT0

pmax_MIGFLAGS	= -DMACH_IPC_COMPAT
i386_MIGFLAGS	= -DMACH_IPC_COMPAT
sun3_MIGFLAGS	= -DMACH_IPC_COMPAT
vax_MIGFLAGS	= -DMACH_IPC_COMPAT
luna88k_MIGFLAGS= -DMACH_IPC_COMPAT

MIGFLAGS	=${${target_machine}_MIGFLAGS:U}

# files that live in ../../kernel/mach
KERNEL_DEFS	= exc.defs mach_host.defs  \
		  mach_port.defs memory_object_default.defs mach4.defs
KERNEL_H	= \
		  boolean.h boot_info.h error.h exception.h host_info.h \
		  kern_return.h mach.defs mach_norma.defs \
		  mach_param.h mach_traps.h mach_types.defs \
		  mach_types.h machine.h memory_object.defs \
		  memory_object.h message.h mig_errors.h mig_support.h \
		  msg_type.h norma_special_ports.h norma_task.defs \
		  notify.defs notify.h \
		  pc_sample.h policy.h port.h \
		  processor_info.h std_types.defs std_types.h \
		  syscall_sw.h task_info.h task_special_ports.h \
		  thread_info.h thread_special_ports.h thread_status.h \
		  thread_switch.h time_value.h vm_attributes.h  \
		  vm_inherit.h vm_param.h vm_prot.h vm_statistics.h 

# files that live in ../../bootstrap/mach
BOOTSTRAP_DEFS	= default_pager.defs bootstrap.defs \
		  default_pager_helper.defs
BOOTSTRAP_H	= default_pager_types.defs default_pager_types.h \
		  default_pager_helper.h

MIG_DEFS	= ${KERNEL_DEFS} ${BOOTSTRAP_DEFS}
MIG_HDRS	= ${MIG_DEFS:.defs=.h}
DATAFILES	= ${MIG_DEFS} ${KERNEL_H} ${BOOTSTRAP_H}
OTHERS		= machine ${MIG_HDRS} mach_interface.h memory_object_user.h \
		  notify_user.h

INCLUDES	= ${DATAFILES} ${OTHERS}
ILIST		= ${INCLUDES}

IDIR		= /include/mach/

.include <${RULES_MK}>

mach_interface.h: mach.defs
	${MIG} ${_MIGFLAGS_} ${mach.defs:P}\
		-header ${.TARGET} -server /dev/null -user /dev/null

memory_object_user.h: memory_object.defs
	${MIG} ${_MIGFLAGS_} ${memory_object.defs:P}\
		-header ${.TARGET} -server /dev/null -user /dev/null

notify_user.h: notify.defs
	${MIG} ${_MIGFLAGS_} ${notify.defs:P}\
		-header ${.TARGET} -server /dev/null -user /dev/null

machine: ${ALWAYS}
	${RM} ${_RMFLAGS_} machine
	${LN} -s ${KERN_MACHINE_DIR} machine
