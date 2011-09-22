#
# Mach Operating System
# Copyright (c) 1993 Carnegie Mellon University
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
# Revision 2.1.1.1  94/06/01  10:26:38  rvb
# 	From BSDSS
# 
# Revision 2.3  93/05/13  13:50:36  rvb
# 	Support xstrip
# 
# Revision 2.2  93/05/11  11:41:16  rvb
# 	uxmon->bsdmon to alleviate confusion.  Bsdss uses bsdmon;ux uses uxmon
# 	[93/05/07  23:12:57  rvb]
# 
# 	Clean bsdmon also.
# 	[93/05/06            rvb]
# 	Created for bsdss
# 	[93/05/05  15:07:54  rvb]
# 

MACH_OBJ	?= ../../../obj

all: tree_bld pre_pgms kernel_pgm libs bootable_kernel pgms

tree_bld:
#	cd tree; make "MACH_OBJ=${MACH_OBJ}"

pre_pgms: util_pgm mig_pgm ld_pgm config_pgm xstrip_pgm

util_pgm:
	cd util; make

mig_pgm:
	cd mig; make

ld_pgm:
	cd binutils; make

xstrip_pgm:
	cd xstrip; make

config_pgm:
	cd config; make

kernel_pgm:
	cd kernel/conf; make
	cd kernel; make

libs: mach_lib thread_lib

mach_lib:
	cd libmach; make

thread_lib:
	cd libthreads; make

bootable_kernel: bootstrap_pgm makeboot_pgm

bootstrap_pgm:
	cd bootstrap; make

makeboot_pgm:
	cd makeboot; make

pgms:

clean:
	cd util; make clean
	cd mig; make clean
	cd binutils; make clean
	cd xstrip; make clean
	cd config; make clean
	cd kernel/conf; make clean
	cd libmach; make clean
	cd libthreads; make clean
	cd bootstrap; make clean
	cd makeboot; make clean

	cd tree; make "MACH_OBJ=${MACH_OBJ}" clean
