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
# $Log:	template.mk,v $
# Revision 2.3  93/05/10  17:45:41  rvb
# 	Use CC_OPT_LEVEL from MI template.mk
# 	[93/05/07  12:39:42  rvb]
# 
# 	Make CC_OPT_LEVEL set in env override value that would be set
# 	here.
# 	[93/05/06            rvb]
# 
# Revision 2.2  93/02/04  14:47:24  mrt
# 	Updated for ODE make.
# 	[93/01/16            mrt]
# 
# Revision 2.14  93/01/14  17:23:23  danner
# 	"The hack below" has been fixed today.
# 	[92/11/30            af]
#
# Revision 2.13  92/04/05  18:28:13  rpd
# 	Removed definition for CC. It is instead defined in Makefile-compilers.
# 	[92/03/17            jvh]
# 
# Revision 2.12  92/03/01  00:39:42  rpd
# 	Removed extraneous MAKE_MACH.
# 	[92/02/29            rpd]
# 
# Revision 2.11  92/02/19  15:06:47  elf
# 	Simplified, condensed history.
# 	[92/02/13            rpd]
# 
#
# Condensed history:
#	Created (af).

###############################################################################
#BEGIN	Machine dependent Makefile fragment for the i386
###############################################################################

LDOBJS_PREFIX = ${ORDERED}

# TEXTORG is set from the config file
MK_LDFLAGS 	= -e _pstart -T ${TEXTORG}

#
# Rules to build mach_i386_server.c.
#

MKODIRS 			+= mach/i386/

MACH_I386_FILE 			= mach/i386/mach_i386_server.c
${MACH_I386_FILE}_MIGFLAGS	= ${MIGKSFLAGS}
${MACH_I386_FILE}_MIGSDEF	= mach/i386/mach_i386.defs

MIG_SRVS			+= ${MACH_I386_FILE}

#
# Things dependent on assym.s:
#
mutex.o:	assym.s
spl.o:		assym.s
misc.o:		assym.s
cswitch.o:	assym.s
idt.o:		assym.s
interrupt.o:	assym.s
locore.o:	assym.s
start.o:	assym.s
abioscall.o:	assym.s
kdasm.o:	assym.s

###############################################################################
#END	Machine dependent Makefile fragment for the i386
###############################################################################
