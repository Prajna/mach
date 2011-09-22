#
# Mach Operating System
# Copyright (c) 1993-1989 Carnegie Mellon University
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
# Revision 2.38  93/02/04  18:21:17  mrt
# 	M88K_EXTRA_DIRS -> LUNA88K_EXTRA_DIRS
# 	[93/02/04            jfriedl]
# 
# 	Add sys to subdirs.
# 	[93/01/25            danner]
# 
# Revision 2.37  93/01/24  13:54:50  danner
# 	Created for ODE make
# 	[92/12/17            mrt]
# 
#
#	Makefile to export and install the include files from the 
#	mk tree that go into /include
#

DEPENCENCIES	=
VPATH		= ../user/libmach:../user/threads

LUNA88K_EXTRA_DIRS = luna88k

SUBDIRS		= mach mach_debug device chips scsi sys\
		  ${KERN_MACHINE_DIR} ${${TARGET_MACHINE}_EXTRA_DIRS}

EXPINC_SUBDIRS	= ${SUBDIRS}
EXPORT_USING_TAR=

IDIR		= /include/

# files which live in ../user/libmach
LIBMACH_H	= errorlib.h mach.h mach_error.h mach_init.h \
		  setjmp.h strings.h

# files which live in ../user/threads
THREADS_H	= cthreads.h

# files which we create
STRANGE_H	= mig_errors.h msg_type.h varargs.h

DATAFILES	= ${LIBMACH_H} ${THREADS_H} ${STRANGE_H}
OTHERS		= machine
INCLUDES	= ${DATAFILES} ${OTHERS}
ILIST 		= ${INCLUDES}

.include <${RULES_MK}>

machine:
	${RM} ${_RMFLAGS_} machine
	${LN} -s ${KERN_MACHINE_DIR} machine

mig_errors.h:	
	${RM} ${_RMFLAGS_} ${.TARGET}
	echo '#include <mach/mig_errors.h>' > ${.TARGET}
	${CHMOD}  644 ${.TARGET}

msg_type.h :
	${RM} ${_RMFLAGS_}  ${.TARGET}
	echo '#include <mach/msg_type.h>' > ${.TARGET}
	${CHMOD} 644 ${.TARGET}

varargs.h :
	${RM}  ${_RMFLAGS_} ${.TARGET}
	echo '#include <sys/varargs.h>' > ${.TARGET}
	${CHMOD} 644 ${.TARGET}
