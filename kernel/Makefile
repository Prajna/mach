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
# Revision 2.25  93/05/16  14:32:46  mrt
# 	Eliminate the include SUBDIR.
# 	[93/05/14            mrt]
# 
# Revision 2.23.1.1  93/05/10  14:53:10  mrt
# 	Added ${MAKE_ARGS} to kernel_only make rule, at Daryl's
# 	suggestion. Makes possible commands like
# 
# 	odemake MAKE_ARGS=`locore.o relink` kernel_only
# 	[93/04/08            mrt]
# 
# Revision 2.23  93/03/26  17:54:35  mrt
# 	Added man to SUBDIRS.
# 
# 
# Revision 2.22  93/03/10  17:36:44  danner
# 	Expanded the kernel rule, so it would work in an empty shadow
# 	directory. Addded kernel_only target for building the kernel
# 	in an already populated area.
# 	[93/03/10            mrt]
# 
# Revision 2.21  93/01/24  13:55:02  danner
# 	Created for Reno make
# 	[91/05/17            mrt]
# 

EXPBIN_SUBDIRS	= src
EXPLIB_SUBDIRS	= src

CONFIG	?= ${KERNEL_${TARGET_MACHINE}_CONFIG:U${KERNEL_CONFIG:UDEFAULT}}

SUBDIRS	=  src conf ${CONFIG} man

.include <${RULES_MK}>

#  default target is build_all and will traverse all of SUBDIRS

#  kernel target does the minimum necessary to build a kernel (mach.boot)
#  in an empty shadow directory

kernel:
	@-if [ ! -d ../include ]; then mkdir ../include; fi
	cd ../include && ${MAKE}
	@-if [ ! -d ../bootstrap ]; then mkdir ../bootstrap; fi
	cd ../bootstrap && ${MAKE}
	@-if [ ! -d conf ]; then mkdir conf; fi
	cd conf && ${MAKE}
	cd ${CONFIG} && ${MAKE}

#  kernel_only builds the kernel in a populated directory

kernel_only:
	cd conf && ${MAKE}
	cd ${CONFIG} && ${MAKE} ${MAKE_ARGS}
