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
# Revision 2.17  93/05/17  17:16:08  mrt
# 	Removed kernel from list of EXPINC_SUBDIRS
# 	[93/05/14            mrt]
# 
# Revision 2.16  93/02/04  17:21:57  mrt
# 	Created for ODE make
# 	[92/12/17            mrt]
# 

EXPBIN_SUBDIRS	= kernel
EXPINC_SUBDIRS	= include
EXPLIB_SUBDIRS	= user bootstrap kernel
SUBDIRS		= include user bootstrap kernel 

.include <${RULES_MK}>
