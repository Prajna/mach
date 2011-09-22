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
# Revision 2.1.1.1  94/06/01  10:23:52  rvb
# 	From BSDSS
# 
# Revision 2.3  93/05/13  13:50:40  rvb
# 	Support xstrip
# 
# Revision 2.2  93/05/11  11:56:15  rvb
# 	uxmon->bsdmon to alleviate confusion.  Bsdss uses bsdmon;ux uses uxmon
# 	[93/05/07  23:13:02  rvb]
# 
# 	I should be a shell!
# 	[93/05/05  15:57:53  rvb]
# 

.if	defined(SYS)
NAME=/${SYS}
LINK=/@sys
.else
NAME=
LINK=
.endif

MACH_OBJ?=../../../obj
KERNEL	?= STD+WS
SERVER	?= STD+WS

all: objs util_dir mig_dir bin_dir xstrip_dir config_dir kernel_dir \
	mach_dir threads_dir bootstrap_dir makeboot_dir \
	includes

objs:
.if	!exists(${MACH_OBJ}${NAME})
	mkdir ${MACH_OBJ}${NAME}
.endif
.if	!exists(${MACH_OBJ}${NAME}/mach)
	mkdir ${MACH_OBJ}${NAME}/mach
.endif
.if	!exists(${MACH_OBJ}${NAME}/mach/lib)
	mkdir ${MACH_OBJ}${NAME}/mach/lib
.endif
.if	!exists(${MACH_OBJ}${NAME}/mach/mach_servers)
	mkdir ${MACH_OBJ}${NAME}/mach/mach_servers
.endif
.if	!exists(${MACH_OBJ}${NAME}/mach/include)
	mkdir ${MACH_OBJ}${NAME}/mach/include
.endif

util_dir:
.if	!exists(${MACH_OBJ}${NAME}/mach/util)
	mkdir ${MACH_OBJ}${NAME}/mach/util
.endif
.if	!exists(../util/obj)
	ln -s ${MACH_OBJ}${LINK}/mach/util ../util/obj
.endif

mig_dir:
.if	!exists(${MACH_OBJ}${NAME}/mach/mig)
	mkdir ${MACH_OBJ}${NAME}/mach/mig
.endif
.if	!exists(../mig/obj)
	ln -s ${MACH_OBJ}${LINK}/mach/mig ../mig/obj
.endif

bin_dir:
.if	!exists(${MACH_OBJ}${NAME}/mach/binutils)
	mkdir ${MACH_OBJ}${NAME}/mach/binutils
.endif
.if	!exists(../binutils/obj)
	ln -s ${MACH_OBJ}${LINK}/mach/binutils ../binutils/obj
.endif

xstrip_dir:
.if	!exists(${MACH_OBJ}${NAME}/mach/xstrip)
	mkdir ${MACH_OBJ}${NAME}/mach/xstrip
.endif
.if	!exists(../xstrip/obj)
	ln -s ${MACH_OBJ}${LINK}/mach/xstrip ../xstrip/obj
.endif

config_dir:
.if	!exists(${MACH_OBJ}${NAME}/mach/config)
	mkdir ${MACH_OBJ}${NAME}/mach/config
.endif
.if	!exists(../config/obj)
	ln -s ${MACH_OBJ}${LINK}/mach/config ../config/obj
.endif

kernel_dir:
.if	!exists(${MACH_OBJ}${NAME}/mach/kernel)
	mkdir ${MACH_OBJ}${NAME}/mach/kernel
.endif
.if	!exists(../kernel/conf/obj)
	ln -s ../obj/.. ../kernel/conf/obj
.endif
.if	!exists(${MACH_OBJ}${NAME}/mach/kernel/${KERNEL})
	mkdir ${MACH_OBJ}${NAME}/mach/kernel/${KERNEL}
.endif
.if	!exists(../kernel/obj)
	ln -s ${MACH_OBJ}${LINK}/mach/kernel/${KERNEL} ../kernel/obj
.endif

mach_dir:
.if	!exists(${MACH_OBJ}${NAME}/mach/libmach)
	mkdir ${MACH_OBJ}${NAME}/mach/libmach
.endif
.if	!exists(../libmach/obj)
	ln -s ${MACH_OBJ}${LINK}/mach/libmach ../libmach/obj
.endif

threads_dir:
.if	!exists(${MACH_OBJ}${NAME}/mach/libthreads)
	mkdir ${MACH_OBJ}${NAME}/mach/libthreads
.endif
.if	!exists(../libthreads/obj)
	ln -s ${MACH_OBJ}${LINK}/mach/libthreads ../libthreads/obj
.endif

bootstrap_dir:
.if	!exists(${MACH_OBJ}${NAME}/mach/bootstrap)
	mkdir ${MACH_OBJ}${NAME}/mach/bootstrap
.endif
.if	!exists(../bootstrap/obj)
	ln -s ${MACH_OBJ}${LINK}/mach/bootstrap ../bootstrap/obj
.endif

makeboot_dir:
.if	!exists(${MACH_OBJ}${NAME}/mach/makeboot)
	mkdir ${MACH_OBJ}${NAME}/mach/makeboot
.endif
.if	!exists(../makeboot/obj)
	ln -s ${MACH_OBJ}${LINK}/mach/makeboot ../makeboot/obj
.endif

includes:
.if	!exists(${MACH_OBJ}${NAME}/mach/include/device)
	mkdir ${MACH_OBJ}${NAME}/mach/include/device
.endif
.if	!exists(${MACH_OBJ}${NAME}/mach/include/mach)
	mkdir ${MACH_OBJ}${NAME}/mach/include/mach
.endif
.if	!exists(${MACH_OBJ}${NAME}/mach/include/mach_debug)
	mkdir ${MACH_OBJ}${NAME}/mach/include/mach_debug
.endif
.if	!exists(${MACH_OBJ}${NAME}/mach/include/sys)
	mkdir ${MACH_OBJ}${NAME}/mach/include/sys
.endif
.if	!exists(${MACH_OBJ}${NAME}/mach/include/uxkern)
	mkdir ${MACH_OBJ}${NAME}/mach/include/uxkern
.endif

.if	!exists(${MACH_OBJ}${NAME}/mach/include/${MACHINE})
	mkdir ${MACH_OBJ}${NAME}/mach/include/${MACHINE}
.endif
.if	!exists(${MACH_OBJ}${NAME}/mach/include/machine)
	ln -s ${MACHINE} ${MACH_OBJ}${NAME}/mach/include/machine
.endif
.if	!exists(${MACH_OBJ}${NAME}/mach/include/mach/${MACHINE})
	mkdir ${MACH_OBJ}${NAME}/mach/include/mach/${MACHINE}
.endif
.if	!exists(${MACH_OBJ}${NAME}/mach/include/mach/machine)
	ln -s ${MACHINE} ${MACH_OBJ}${NAME}/mach/include/mach/machine
.endif
.if	${MACHINE} == i386
.if	!exists(${MACH_OBJ}${NAME}/mach/include/i386at)
	mkdir ${MACH_OBJ}${NAME}/mach/include/i386at
.endif
.endif	

clean:
	-rmdir ${MACH_OBJ}${NAME}/mach/util
	-rm -f ../util/obj

	-rmdir ${MACH_OBJ}${NAME}/mach/mig
	-rm -f ../mig/obj

	-rmdir ${MACH_OBJ}${NAME}/mach/binutils
	-rm -f ../binutils/obj

	-rmdir ${MACH_OBJ}${NAME}/mach/xstrip
	-rm -f ../xstrip/obj

	-rmdir ${MACH_OBJ}${NAME}/mach/config
	rm -f ../config/obj

	-rm -rf ${MACH_OBJ}${NAME}/mach/kernel
	-rm -f ../kernel/conf/obj
	-rm -f ../kernel/obj

	-rmdir ${MACH_OBJ}${NAME}/mach/libmach
	-rm -f ../libmach/obj

	-rmdir ${MACH_OBJ}${NAME}/mach/libthreads
	-rm -f ../libthreads/obj

	-rmdir ${MACH_OBJ}${NAME}/mach/bootstrap
	-rm -f ../bootstrap/obj

	-rmdir ${MACH_OBJ}${NAME}/mach/makeboot
	-rm -f ../makeboot/obj

	-rmdir ${MACH_OBJ}${NAME}/mach/mach_servers
	-rmdir ${MACH_OBJ}${NAME}/mach/lib
	-rm -rf ${MACH_OBJ}${NAME}/mach/include

	-rmdir ${MACH_OBJ}${NAME}/mach
