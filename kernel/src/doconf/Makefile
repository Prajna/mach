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
# Revision 2.6  93/05/31  17:29:36  mrt
# 	Added conditonal rule so the bnr environment would get
# 	its own version of the script.
# 	[93/05/31            mrt]
# 
# Revision 2.5  93/02/04  13:46:37  mrt
# 	Modifed for ODE make.
# 	[92/12/23            mrt]
# 

CCTYPE			= host
SCRIPTS			= doconf
IDIR			= /bin/
EXPLIB_TARGETS		= export_doconf

.if ${host_context} == "i386_bnr"
doconf.sh: doconf.sh.bnr
	${RM} -f doconf.sh
	cp -p ${doconf.sh.bnr:P} doconf.sh
.endif

ILIST			= ${SCRIPTS}

.include <${RULES_MK}>

