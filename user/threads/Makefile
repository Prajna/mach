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
# Revision 2.30  93/11/17  19:00:21  dbg
# 	Added -rbase and -B to the alpha compilation flags.
# 	This is needed so that crt0.o and libmach_sa can be
# 	used by the emulator which lives in a high address
# 	quadrant.
# 	[93/09/17            mrt]
# 
# Revision 2.29  93/08/02  21:44:56  mrt
# 	Added LIB_LONG_NAMES for the mips and i386 since one of
# 	the .o names in the libthreads_p.a is too long.
# 	[93/06/13            mrt]
# 
# Revision 2.28  93/06/02  20:50:07  rvb
# 	Added definition of NOSTRIP.
# 	[93/06/02            mrt]
# 
# Revision 2.27  93/05/13  13:47:35  rvb
# 	".if <var> == string" requires string to be in double quotes.
# 
# Revision 2.26  93/05/11  15:06:02  rvb
# 	LIBRARIES_P only for mips and i386
# 
# Revision 2.25  93/05/11  09:48:35  rvb
# 	Removed -pg and -DGRPOF for mips and i386.  Added
# 	LIBRARIES_P instead. Now libthreads.a and libthreads_p.a
# 	will always get built and exported.
# 	[93/05/11            mrt]
# 
# Revision 2.24  93/05/10  17:51:13  rvb
# 	Handle -pg and -DGPROF just like libthreads does.
# 	[93/05/06            rvb]
# 	-DCTHREAD_DATA is now unnecessary.  It is effectively
# 	always true.
# 	[93/05/06            rvb]
# 
# Revision 2.23  93/01/24  13:27:10  danner
# 	Modified for ODE make
# 	[92/12/17            mrt]
# 
# Revision 2.21  92/05/22  18:38:31  jfriedl
# 	From Mike Kupfer <kupfer@sprite.Berkeley.EDU>:
# 	Enable CTHREAD_DATA for the kernel bootstrap program.
# 
# Revision 2.20  92/03/05  22:46:34  rpd
# 	Changed to use double-colon rules for top-level targets.
# 	[92/02/28            rpd]
# 
# Revision 2.19  92/01/23  15:22:53  rpd
# 	Revised for new Makefile organization.
# 	[92/01/16            rpd]
# 


DEPENDENCIES	=

VPATH		= ${target_cpu}

LIBRARIES	= libthreads.a
.if ${target_cpu} == "mips" || ${target_cpu} == "i386"
LIBRARIES_P	= libthreads_p.a
# Use library rule for long file names
LIB_LONG_NAMES	=
.endif

EXPLIB_TARGETS	= ${LIBRARIES:S/^/export_&/g} ${OBJECTS:S/^/export_&/g}
ILIST		= ${LIBRARIES}
.if defined(LIBRARIES_P)
EXPLIB_TARGETS	+= ${LIBRARIES_P:S/^/export_&/g}
ILIST		+= ${LIBRARIES_P}
.endif
NOSTRIP		=
IDIR		= /lib/

CFLAGS		=${${target_cpu}_CFLAGS:U}

# Note that ALPHA_?FLAGS is used by osf.mach3.mk and gets added for
# all files independently of CFLAGS. These switches allow code in the
# libraries to be loaded in any of the alpha address quadrants.
# note that code used by the emulator must be able to run in a
# high quadrant.
.if (${HOST_MACHINE} == "ALPHA")  && (!defined(NOT_USING_GCC2) )
ALPHA_CFLAGS    = -rbase
ALPHA_ASFLAGS   = -B
.endif

OFILES	= cprocs.o cthreads.o malloc.o \
          mig_support.o stack.o sync.o \
       	  thread.o lock.o csw.o cthread_data.o

.include <${RULES_MK}>
