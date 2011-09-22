#
# Mach Operating System
# Copyright (c) 1991,1990,1989 Carnegie Mellon University
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
# Revision 2.3  93/08/02  21:44:28  mrt
# 	Added strings.h.
# 	[93/08/02            mrt]
# 
# Revision 2.2  93/02/04  13:35:25  mrt
# 	Created for ODE make
# 	[92/12/17            mrt]
# 
#
#	Export the inlcude files needed to build and
#	run mig.
#

VPATH		= ../../../mach:../../../device:../../../../bootstrap/mach:../../../mach_debug:../../../../user/libmach

EXPINC_SUBDIRS	= ${KERN_MACHINE_DIR}

EXPORT_USING_TAR=

OTHERS		= machine

INCLUDES	= kern_return.h message.h port.h boolean.h strings.h \
		  std_types.defs mach_types.defs device_types.defs \
		  default_pager_types.defs \
		  mach_debug_types.defs \
		  ${OTHERS}

EXPDIR				= /include/mach/
device_types.defs_EXPDIR	= /include/device/
mach_debug_types.defs_EXPDIR	= /include/mach_debug/
strings_h_EXPDIR		= /include

.include <${RULES_MK}>

machine: ${ALWAYS}
	${RM} ${_RMFLAGS_} machine
	${LN} -s ${KERN_MACHINE_DIR} machine
