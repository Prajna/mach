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
# Revision 2.12  93/02/04  13:48:15  mrt
# 	Modifed for ODE make.
# 	[92/12/23            mrt]
# 
# Revision 2.10  92/04/08  23:19:33  rpd
# 	Added cross-compilation support.
# 	[92/03/18            jvh]
# 

DEPENDENCIES =

PROGRAMS		= config
IDIR			= /bin/
EXPLIB_TARGETS		= export_config

ILIST			= ${PROGRAMS}

CCTYPE			= host

# Using ${PROGRAM}_OFILES makes them all depend on HFILES
config_OFILES		=  main.o gram.o parse.o mkioconf.o mkmakefile.o \
			  mkheaders.o mkglue.o
HFILES			= gram.h config.h


LIBS			= ${LIBL} 

.if ${TARGET_MACHINE} == "SUN3" || ${TARGET_MACHINE} == "SUN4"
mkioconf.o:	sun/sun_mkioconf.c
.endif

.include <${RULES_MK}>

# Write our own yacc rule to avoid having yacc run twice
# when Make is run in parallel mode.

gram.c: gram.h
gram.h:	gram.y
		${YACC} -d ${_YFLAGS_} ${gram.y:P}
		${MV} -f y.tab.c gram.c
		${MV} -f y.tab.h gram.h
