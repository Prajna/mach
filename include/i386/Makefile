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
# Revision 2.5  93/08/10  15:10:10  mrt
# 	Pass pio.h to release.
# 	[93/07/09            rvb]
# 
# Revision 2.4  93/05/16  14:58:04  mrt
# 	Added KINCLUDES which are the include files the kernel
# 	gets out of machine/. These files are only exported and
# 	not released. The only UINCLUDES which are the files that 
# 	user level programs need are released.
# 	[93/05/14            mrt]
# 
# Revision 2.3  93/02/04  18:21:13  mrt
# 	Created for ODE make
# 	[92/12/17            mrt]
# 


VPATH		= ../../kernel/i386:../../kernel/i386at/:../../bootstrap/i386:../../user/threads/i386

UINCLUDES	= asm.h cthreads.h disk.h eflags.h exec.h pio.h
KINCLUDES	= ast.h ast_types.h  cpu_number.h \
		  db_machdep.h kttd_machdep.h \
		  lock.h mach_param.h machine_routines.h machspl.h \
		  pmap.h  sched_param.h setjmp.h \
		  thread.h time_stamp.h timer.h \
		  vm_tuning.h xpr.h

INCLUDES	= ${UINCLUDES} ${KINCLUDES}
ILIST		= ${UINCLUDES}

IDIR		= /include/i386/
disk.h_IDIR	= /include/i386at/

.include <${RULES_MK}>
