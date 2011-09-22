# 
# Mach Operating System
# Copyright (c) 1991,1990,1989 Carnegie Mellon University
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
# $Log:	cthread_inline.awk,v $
# Revision 2.5  91/05/14  17:57:59  mrt
# 	Correcting copyright
# 
# Revision 2.4  91/02/14  14:20:40  mrt
# 	Added new Mach copyright
# 	[91/02/13  12:38:45  mrt]
# 
# Revision 2.3  89/12/08  19:49:36  rwd
# 	Remove mutex_unlock
# 	[89/12/06            rwd]
# 
# Revision 2.2  89/11/29  14:19:05  af
# 	Oooops, a typo. I mean, this file _really_ is unnecessary..
# 	[89/10/28            af]
# 
# 	Created.
# 	[89/07/06            af]
# 

# pmax/cthread_inline.awk
#
# Awk script to inline critical C Threads primitives on MIPS.
# This is not really needed, but there it goes anyways.
#

NF == 2 && $1 == "jal" && $2 == "spin_unlock" {
	print	"	#	BEGIN INLINE " $2
	print	"	sw	$0,0($4)"
	print	"	#	END INLINE " $2
	continue
}
NF == 2 && $1 == "jal" && $2 == "cthread_sp" {
	print	"	#	BEGIN INLINE cthread_sp"
	print	"	move	$2,$29"
	print	"	#	END INLINE cthread_sp"
	continue
}
# default:
{
	print
}
