# 
# Mach Operating System
# Copyright (c) 1993,1992 Carnegie Mellon University
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
# Revision 2.2  93/02/04  18:40:08  mrt
# 	Created for ODE make.
# 	[92/12/17            mrt]
# 

DEPENDENCIES	=

IDIR		= /include/mach_debug/

VPATH		= ../../kernel/mach_debug/

MIG_HDRS	=  mach_debug.h 

DATAFILES 	= hash_info.h ipc_info.h \
		  mach_debug.defs mach_debug_types.defs \
		  mach_debug_types.h \
		  vm_info.h zone_info.h

INCLUDES	= ${DATAFILES} ${MIG_HDRS}

ILIST		= ${INCLUDES}

.include <${RULES_MK}>
