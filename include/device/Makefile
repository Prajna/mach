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
# Revision 2.4  93/08/10  15:10:14  mrt
# 	Export bpf.h for BPF clients.
# 	[93/06/11            cmaeda]
# 	Need disk_status.h for label processing code.
# 	[93/07/09            rvb]
# 
# Revision 2.3  93/05/14  15:09:08  rvb
# 	Export audio_status.h for af's toys.
# 	[93/05/14            rvb]
# 
# Revision 2.2  93/02/04  18:40:19  mrt
# 	Created for ODE make
# 	[92/12/17            mrt]
# 

DEPENDENCIES	=
IDIR		= /include/device/

VPATH 		= ../../kernel/device/

MIG_HDRS	= device.h device_reply.h device_request.h 
DATAFILES 	= audio_status.h device.defs device_types.defs device_request.defs \
	    	  device_reply.defs net_status.h tape_status.h tty_status.h \
	    	  disk_status.h device_types.h bpf.h

INCLUDES	=  ${DATAFILES} ${MIG_HDRS}

ILIST		= ${INCLUDES}

.include <${RULES_MK}>
