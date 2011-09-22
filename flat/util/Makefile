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
# Revision 2.1.1.1  94/06/01  10:23:55  rvb
# 	From BSDSS
# 
# Revision 2.3  93/12/14  14:24:12  rvb
# 	Upgrade for NetBSD 0.9 and "md".
# 	[93/12/13            rvb]
# 
# Revision 2.2  93/05/11  11:56:24  rvb
# 		Placed under BSDSS Source Control
# 	[93/05/05            rvb]
# 
#

PATH= ${.CURDIR} ${MACH_SRC:S/$/\/util/g}
.PATH: ${PATH}
ISRC=  -I${.CURDIR} ${MACH_SRC:S/^/-I/g:S/$/\/util/g} 

CFLAGS+= -MD ${MYFLAGS} ${ISRC}


all: md shadow_path

md: ${.TARGET}.o
	${CC} -o ${.TARGET} ${.ALLSRC}

shadow_path: ${.TARGET}.o
	${CC} -o ${.TARGET} ${.ALLSRC}

clean:
	rm -f md md.o shadow_path shadow_path.o core
	rm -f .depend *.d

.END .INTERRUPT:
	-@../util/md -u .depend ${ISRC} -d *.d 
