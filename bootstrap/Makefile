# 
# Mach Operating System
# Copyright (c) 1993-1991 Carnegie Mellon University
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
# Revision 2.14  93/02/04  13:03:02  mrt
# 	Removed the installation of bootstrap. This file is only
# 	used during a kernel build when it is found in export.
# 	[93/02/04            mrt]
# 
# Revision 2.13  93/02/04  10:55:30  mrt
# 	Made xstripping rule explicit again as it has been
# 	removed from osf.mach3.mk.
# 	[93/02/02            mrt]
# 
# Revision 2.12  93/02/01  08:37:49  danner
# 	Added -Dsun4c to Sun 4 CFLAGS (after the include of rules.mk).
# 	It is needed for now to get the right parameters for allocating
# 	the stack.
# 	[93/01/29            dlc]
# 
# Revision 2.11  93/01/24  13:54:45  danner
# 	Xstrip rule now implicit.
# 	[93/01/24            danner]
# 
# 	Updated for ODE make.
# 	[93/01/01            mrt]
# 
# 

DEPENDENCIES	=

# find machine-dependent files in machine subdirectory

VPATH		= ${target_cpu}

# We want the MK version to be part of the bootstrap name.
# Makefile-version defines VERSION for us.

.include "${MAKETOP}Makefile-version"
BOOTSTRAP 	= bootstrap.${VERSION}

OTHERS		= ${BOOTSTRAP}
BINARIES	=

EXPLIB_TARGETS	= export_${BOOTSTRAP}
EXPDIR		= /special/

OFILES		 = bootstrap.o def_pager_setup.o default_pager.o \
		   exec.o file_io.o kalloc.o load.o strfcns.o wiring.o 


LIBS 		= ${LIBTHREADS} ${LIBMACH_SA} ${LDLIBS}

# We are going to use xstrip, so don't want release to try to strip it
# again.
NO_STRIP	= 


.include <${RULES_MK}>

#
# Temporary measure for now: need sun4c in machine specific CFLAGS - dlc
#

SUN4_CFLAGS += -Dsun4c

# We don't want to load with libc.a

${BOOTSTRAP}: ${OFILES}
	${RM} -f ${.TARGET} ${.TARGET}.out
	${_LD_} -z -o ${.TARGET}.out ${_LDFLAGS_} ${CRT0} ${OFILES} ${LIBS} && \
	${MV} ${.TARGET}.out ${.TARGET}.unstripped
	${XSTRIP} ${.TARGET}.unstripped && ${MV} ${.TARGET}.unstripped ${.TARGET}


