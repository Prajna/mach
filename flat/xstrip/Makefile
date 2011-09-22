# 
# Mach Operating System
# Copyright (c) 1993 Carnegie Mellon University
# All Rights Reserved.
# 
# Permission to use, copy, modify and distribute this software and its
# documentation is hereby granted, provided that both the copyright
# notice and this permission notice appear in all copies of the
# software, derivative works or modified versions, and any portions
# thereof, and that both notices appear in supporting documentation.
# 
# CARNEGIE MELLON ALLOWS FREE USE OF THIS SOFTWARE IN ITS 
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
# any improvements or extensions that they make and grant Carnegie the
# rights to redistribute these changes.
# 
#
# HISTORY
# $Log:	Makefile,v $
# Revision 2.1.1.1  94/06/01  10:24:29  rvb
# 	From BSDSS
# 
# Revision 2.3  93/12/14  14:24:56  rvb
# 	Upgrade for NetBSD 0.9 and "md".
# 	[93/12/13            rvb]
# 
# Revision 2.2  93/05/12  13:27:28  rvb
# 	We have to use the same a.out.h as binutils
# 
# 

PATH= ${.CURDIR} ${MACH_SRC:S/$/\/xstrip/g}
.PATH: ${PATH}
ISRC=  -I${.CURDIR} ${MACH_SRC:S/^/-I/g:S/$/\/xstrip/g} 

CFLAGS+= -MD ${MYFLAGS} -I. ${ISRC}

all: a.out.h xstrip

xstrip: xstrip.o
	cc -o xstrip xstrip.o

a.out.h: ../binutils/a.out.h
	/bin/rm -f ${.TARGET}
	/bin/cp ${.OODATE} ${.TARGET}

clean:
	rm -f xstrip xstrip.o a.out.h core
	rm -f .depend *.d

.END .INTERRUPT:
	-@../util/md -u .depend ${ISRC} -d *.d 
