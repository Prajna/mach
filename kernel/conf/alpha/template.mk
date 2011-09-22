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
# HISTORY
# $Log:	template.mk,v $
# Revision 2.4  93/05/20  21:10:37  mrt
# 		Removed use of LINENO. It is used in machine independent
# 		template.mk.
# 		Made all the compiler options set dynamically.
# 	[93/05/14            mrt]
# 
# Revision 2.3  93/03/09  10:52:40  danner
# 	GCC & BL10 changes.
# 	[93/03/05            af]
# 
# Revision 2.2  93/02/04  16:54:50  mrt
# 	Updated for ODE make.
# 	[93/01/16            mrt]
# 
# Revision 2.2  93/01/14  17:23:18  danner
# 	Created.
# 	[92/05/31            af]
# 


######################################################################
#BEGIN	Machine dependent Makefile fragment for the ALPHA
######################################################################

# Note: TEXTBASE is/canbe machdep, MASTER.alpha sets it

# Override some definitions in Makefile.template:


.if ( ${HOST_MACHINE} == "PMAX" )
# mips cross cross compiler
    CMACHFLAGS = -Wc,-nofloat -O2
    ALPHA_ASFLAGS = -nocpp 
    MK_LDFLAGS = -EL -N -T ${TEXTBASE} -e start
.endif

.if ( ${HOST_MACHINE} == "ALPHA")
.if defined(NOT_USING_GCC2)
#    BL10 native - OSF/1 native compiler
    CMACHFLAGS = -Wc,-nofloat  -O -O1
    ALPHA_ASFLAGS = -nocpp 
    MK_LDFLAGS =  -EL -N -w2 -static -T ${TEXTBASE} -e start  `Flags L LPATH`

.else
#   gcc 2.3  compiler
    CMACHFLAGS = -kbase -mno-fp-regs -O2 -D__GNU_AS__
    ALPHA_ASFLAGS = -Bk
    MK_LDFLAGS = -N -Tstrip ${TEXTBASE} -e start
.endif
.endif

CC_OPT_LEVEL = ${CMACHFLAGS}

# scb goes into an 8k aligned page!
LDOBJS_PREFIX = start.o


#  Explicit dependencies on generated files,
#  to ensure that genassym/mig has been run by the time
#  these files are compiled.

${SOBJS} : assym.s

exec.o : mach/mach_interface.h mach/mach_user_internal.h \
	mach/mach_port_internal.h

######################################################################
#END	Machine dependent Makefile fragment for the ALPHA
######################################################################
