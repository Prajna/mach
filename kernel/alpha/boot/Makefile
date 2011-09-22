#
# Mach Operating System
# Copyright (c) 1992 Carnegie Mellon University
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
# Revision 2.4  93/08/31  15:15:45  mrt
# 	Actually made it work with odemake.
# 	[93/08/23            af]
# 
# Revision 2.3  93/03/29  17:14:40  mrt
# 	Converted for odemake.
# 	[93/03/09            mrt]
# 
# Revision 2.2.1.1  93/03/06  14:28:57  af
# 	Reworked to build two separate things: "osf_boot" and
# 	"15sec_boot".
# 	The first one is for OSF/1 builds, you build it only with
# 	DEC compiler's [must be COFF, you can try "gld -Thdr"]
# 	and install it as the secondary boot.
# 	The second one is for GCC builds, you build it only with
# 	GCC tools [wont fit otherwise] and install it as the
# 	primary and only boot.
# 
# Revision 2.2  93/02/05  08:00:20  danner
# 	Created.
# 	[93/02/04  01:01:46  af]
# 
#

DEPENDENCIES	=

PROGRAMS	= 15sec_boot osf_boot test
ILIST		= 15sec_boot
IDIR		= /special/
 
EXPBIN_TARGETS	= export_15sec_boot

# build rules


# Override some definitions in Makefile.template:

#
# GCC specific things
#
# If you compile with DEC's compiler it is hopeless
# to try fit "15sec_boot" into 15 sectors.
# Your only way around is to install "osf_boot" as
# the secondary on top of your root's "osf_boot".
# Both solutions are known to work just fine.
#


.if ( ${HOST_MACHINE} == "PMAX" )
# mips cross cross compiler
    CMACHFLAGS = 
.endif

.if ( ${HOST_MACHINE} == "ALPHA")
.if defined(NOT_USING_GCC2)
#    BL10 native - OSF/1 native compiler
    CMACHFLAGS = 

.else
#   gcc 2.3  compiler
    CMACHFLAGS = -mno-fp-regs -O2 -D__GNU_AS__
    ALPHA_ASFLAGS = -Bk
.endif
.endif

CC_OPT_LEVEL = ${CMACHFLAGS} -DINTERACTIVE=1 -DUSE_LABEL=1


COMMONOBJS = start.o c_misc.o prom_routines.o ufs.o
BOOTOBJS = ${COMMONOBJS} disk_boot.o supage.o
TESTOBJS = ${COMMONOBJS} test.o supage.o

15sec_boot_OFILES = ${BOOTOBJS}
osf_boot_OFILES = ${BOOTOBJS}
test_OFILES	= ${TESTOBJS}

OBJS	= ${BOOTOBJS}

LIBS =

#
# There is a dangerous depend here on the type and size
# of the executable's header: 80 bytes is what we get with
# the current a.out.  The linker really should do this..
#
# This will (must!!) fail with DEC's compiler
#
# This boot is supposed to be installed in this way:
#	dd if=15sec_boot of=/dev/<rawdisk> seek=1
#
15sec_boot : ${OBJS}
	@echo  
	@-${RM} ${.TARGET} ${.TARGET}.out
	${_LD_} -Tstrip 20000000 -N -e start -o ${.TARGET}.out ${OBJS} ${LIBS} ${LDLIBS} && \
	strip ${.TARGET}.out && \
	dd if=${.TARGET}.out of=${.TARGET} bs=80 skip=1 && \
	size ${.TARGET}.out && \
	${RM} ${.TARGET}.out
	@echo "Size below should not be more than 7680 bytes (text+data)"
	@ls -l ${.TARGET}

#
# This has been seen to work, too.
#
# This boot is supposed to be installed in this way:
#	cp osf_boot <true_root_of_disk>/osf_boot
#
osf_boot : ${OBJS}
	@echo  
	@-${RM} ${.TARGET} ${.TARGET}.out
	${_LD_} -T 20004000 -N -e start -o ${.TARGET}.out ${OBJS} ${LIBS} ${LDLIBS} && \
	${XSTRIP} ${.TARGET}.out && \
	${MV} ${.TARGET}.out ${.TARGET}

#
# Test program, for new machines
#
# This boot is supposed to be installed in this way:
#	cp test <true_root_of_disk>/test
#
# It is used presumably in this way:
#	boot -fi "test" dka300
#
test : ${TESTOBJS}
	@echo  
	@-${RM} ${.TARGET} ${.TARGET}.out
	${_LD_} -T 20020000 -N -e start -o ${.TARGET}.out ${TESTOBJS} ${LIBS} ${LDLIBS} && \
	${MV} ${.TARGET}.out ${.TARGET}



.include <${RULES_MK}>
