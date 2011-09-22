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
# HISTORY
# $Log:	template.mk,v $
# Revision 2.6  93/05/20  21:09:01  mrt
# 	Changed AS_FLAGS_NORMAL to PMAX_ASFLAGS so the value
# 	would get used.
# 	[93/05/20            mrt]
# 
# Revision 2.5  93/05/11  12:16:14  rvb
# 	Changed uses of CMACHFLAGS to CC_OPT_EXTRA.
# 	[93/05/11            mrt]
# 
# Revision 2.4  93/05/10  19:40:51  rvb
# 	Removed LINENO from here as it is now used in machine independent
# 	MASTER file. Set CC_OPT_EXTRA which is now an addition to
# 	CC_OPT_LEVEL.
# 	[93/05/10            mrt]
# 
# Revision 2.3  93/05/10  17:45:47  rvb
# 	Use CC_OPT_LEVEL and CC_OPT_EXTRA for max flexibility
# 	[93/05/07            rvb]
# 
# Revision 2.2  93/02/04  14:50:08  mrt
# 	Updated for ODE make and gcc2.3.2.
# 	[93/01/16            mrt]
# 
# Revision 2.9  92/02/19  15:07:01  elf
# 	Simplified, condensed history.
# 	[92/02/13            rpd]
# 
#
# Condensed history:
#	Added lineno (jsb).
#	Created (af).

######################################################################
#BEGIN	Machine dependent Makefile fragment for the MIPS
######################################################################

TEXTBASE = 80030000

# Override some definitions in Makefile.template:

# ENDIAN, GPSIZE are set from the config file
CC_OPT_EXTRA += ${ENDIAN} ${GPSIZE}

PMAX_ASFLAGS = -w -nocpp 

LDOBJS_PREFIX = start.o
MK_LDFLAGS = ${CC_OPT_EXTRA} -N -T ${TEXTBASE} -e start

#  Explicit dependencies on generated files,
#  to ensure that genassym/mig has been run by the time
#  these files are compiled.

${SOBJS} : assym.s

exec.o : mach/mach_interface.h mach/mach_user_internal.h \
	mach/mach_port_internal.h

pm_tty.o : mach/mach_interface.h

######################################################################
#END	Machine dependent Makefile fragment for the MIPS
######################################################################
