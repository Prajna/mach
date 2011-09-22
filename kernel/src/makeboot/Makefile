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
# then rights to redistribute these changes.
#
#
# HISTORY
# $Log:	Makefile,v $
# Revision 2.8  93/02/04  13:46:28  mrt
# 	Created for ODE makefile
# 	[93/01/29            mrt]
# 
#

DEPENDENCIES	=

PROGRAMS	= makeboot
IDIR		= /bin/
EXPLIB_TARGETS	= export_makeboot
ILIST		= ${PROGRAMS}

CCTYPE		= host
VPATH		= ${target_cpu}

OFILES		= build_boot.o exec.o cross_32_to_64.o

# even though we are using the host cc we want to look in the
# INCDIRS for the mach header files.

HOST_INCDIRS	= ${INCDIRS}

.include <${RULES_MK}>

