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
# Revision 2.1.1.1  94/06/01  10:19:27  rvb
# 	From BSDSS
# 
# Revision 2.5  93/12/14  14:12:38  rvb
# 	Upgrade for NetBSD 0.9 and "md".
# 	[93/12/13            rvb]
# 
# Revision 2.3  93/06/17  12:49:14  rvb
# 	Switch over to "x" suffix on all pgms.
# 	But temporarily we need the non suffixed copies till
# 	we get around to editing MK82.
# 	[93/06/17            rvb]
# 
# Revision 2.2  93/05/11  11:41:36  rvb
# 		Gnu code originally used by Mach I386 release -> BSDSS
# 	[93/05/05  15:21:03  rvb]
# 
#
#

PATH= ${.CURDIR} ${MACH_SRC:S/$/\/binutils/g}
.PATH: ${PATH}
ISRC=  -I${.CURDIR} ${MACH_SRC:S/^/-I/g:S/$/\/binutils/g} 

CFLAGS+= -MD ${MYFLAGS} ${ISRC}

PGMS=arx ranlibx ldx nmx

#all: ${PGMS}
all: ar ranlib ldx nm

${PGMS}: ${.TARGET}.o
	${CC} -o ${.TARGET} ${.ALLSRC}

ranlib.o: ranlib.c
	$(CC) -c ${CFLAGS} "-DAR_PROG=\"`pwd`/ar\"" ${.ALLSRC:M*.c}
	

ar ranlib nm: ${.TARGET}.o
	${CC} -o ${.TARGET} ${.ALLSRC}
	rm -f ${.TARGET}x
	ln ${.TARGET} ${.TARGET}x

clean:
	rm -f ldx ldx.o arx arx.o ranlibx ranlibx.o nmx nmx.o core
	rm -f ar nm ranlib
	rm -f .depend *.d

.END .INTERRUPT:
	-@../util/md -u .depend ${ISRC} -d *.d 
