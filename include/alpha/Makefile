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
# Revision 2.5  93/05/16  14:58:07  mrt
# 	Added KINCLUDES which are the include files the kernel
# 	gets out of machine/. These files are only exported and
# 	not released. The only UINCLUDES which are the files that 
# 	user level programs need are released.
# 	[93/05/14            mrt]
# 
# Revision 2.4  93/03/09  10:48:01  danner
# 	Added full set of varargs files. Sigh.
# 	[93/03/05            af]
# 
# Revision 2.3  93/01/24  13:54:58  danner
# 	Created for ODE make
# 	[92/12/17            mrt]
# 

VPATH		= ../../kernel/alpha:../../user/threads/alpha

UINCLUDES	= coff.h syms.h exec.h \
		  varargs.h varargs.h.gcc va-alpha.h varargs.h.dec \
		  cthreads.h 
KINCLUDES	= ast.h ast_types.h cpu_number.h \
		  db_machdep.h db_trace.h \
		  lock.h mach_param.h machspl.h \
		  pmap.h setjmp.h \
		  thread.h time_stamp.h  \
		  vm_tuning.h xpr.h


INCLUDES	= ${UINCLUDES} ${KINCLUDES}
ILIST		= ${UINCLUDES}

IDIR		= /include/alpha/

.include <${RULES_MK}>
