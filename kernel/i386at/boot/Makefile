#
# Mach Operating System
# Copyright (c) 1992, 1991 Carnegie Mellon University
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
# Revision 2.4  93/11/17  16:41:34  dbg
# 	Update for odemake. 
# 	[93/11/08            mrt]
# 
# Revision 2.3  93/08/10  15:56:42  mrt
# 	DEBUG -> LABEL_DEBUG
# 	[93/08/02            rvb]
# 	Enable Debugging printouts.
# 	[93/07/09  15:22:09  rvb]
# 
# Revision 2.2  92/04/04  11:33:46  rpd
# 	Fabricated for 3.0
# 	[92/03/30            mg32]
# 
#

DEPENDENCIES	=
PROGRAMS	= boot

CC_OPT_LEVEL	= -O
# LABEL_DEBUG causes FILE, LABEL and partition info to be printed on boot.
CFLAGS		= -DLABEL_DEBUG -DSD
# Add -DSD to CFLAGS if buiding a boot program for SCSI disks
# Add -DIBM_L40 if building a boot program for an IBM L40
LIBS		= -lc

NOSTRIP		=
IDIR		= /special/
ILIST		= boot.blocks

# start.o should be first, table.o should be second
OFILES = start.o table.o boot2.o boot.o asm.o bios.o io.o disk.o sys.o

boot: boot.sed ${OFILES}
	$(_LD_) -N -T 0 -o ${.TARGET}.sym $(OFILES) $(LIBS)
	cp ${.TARGET}.sym ${.TARGET}
	@strip ${.TARGET}
	dd if=${.TARGET} of=${.TARGET}.tmp ibs=32 skip=1 obs=1024b
	mv -f ${.TARGET}.tmp ${.TARGET}
	ln ${.TARGET} boot.blocks
	@ls -l boot.blocks

clean:
	/bin/rm -f *.o *.d boot boot.sym


.include <${RULES_MK}>

.c.o:
	$(_CC_) -S $(_CCFLAGS_)  ${.IMPSRC}
	sed -f ${boot.sed:P} ${.TARGET:S/.o$/.s/} >  ${.TARGET:S/.o$/.i/}
	$(_AS_) ${.TARGET:S/.o$/.i/} -o ${.TARGET}
	rm -f ${.TARGET:S/.o$/.s/}  ${.TARGET:S/.o$/.i/}
