#!/bin/sh -
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
# $Log:	newvers.sh,v $
# Revision 2.8  93/05/16  15:25:40  mrt
# 	Removed use of version.minor and use version.major
# 	[93/05/16            mrt]
# 
# Revision 2.7  93/03/09  10:53:03  danner
# 	Fixed build of version string to accomodate both BSD and OSF/1
# 	/bin/echo commands. ("\n" can't be echoed with the same command)
# 	From: [93/01/12            bernadat]
# 
# Revision 2.6  91/05/08  12:25:38  dbg
# 	Added one more sed clause to deal with the "'s in the new 
# 	copyright.
# 	[91/05/07            mrt]
# 
# Revision 2.5  91/02/05  17:05:40  mrt
# 	Changed to new copyright
# 	[91/01/28  14:48:14  mrt]
# 
# Revision 2.4  90/09/14  12:56:01  rwd
# 	Dont printout major.minor in VERSION string.
# 	[90/09/14            rwd]
# 
# Revision 2.3  90/08/27  21:48:12  dbg
# 	Document that this file was created at CMU.
# 	[90/07/13            dbg]
# 
# Revision 2.2  89/11/29  14:08:47  af
# 	Use Mach version id.
# 
# Revision 2.1  89/08/03  17:38:33  rwd
# Created.
# 

#
# newvers.sh	copyright major minor variant edit patch
#

copyright="$1"; major="$2"; variant="$3"; edit="$4"; patch="$5"; 
v="VERSION(${variant}${edit}${patch})" d=`pwd` h=`hostname` t=`date`
if [ -z "$d" -o -z "$h" -o -z "$t" ]; then
    exit 1
fi
CONFIG=`expr "$d" : '.*/\([^/]*\)$'`
d=`expr "$d" : '.*/\([^/]*/[^/]*\)$'`
(
  /bin/echo "char  version_major[]   = \"${major}\";" ;
  /bin/echo "char version_variant[]  = \"${variant}\";" ;
  /bin/echo "char version_patch[]    = \"${patch}\";" ;
  /bin/echo "int  version_edit       = ${edit};" ;
  /bin/echo "char version[] = \"${major} ${v}: ${t}; $d ($h)\\n\";" ;
  /bin/echo "char cmu_copyright[] = \"\\" ;
  sed <$copyright -e '/^#/d' -e 's/"/\\"/g' -e 's;[ 	]*$;;' -e '/^$/d' -e 's;$;\\n\\;' ;
  /bin/echo "\";";
) > vers.c
if [ -s vers.suffix -o ! -f vers.suffix ]; then
    echo ".${variant}${edit}${patch}.${CONFIG}" >vers.suffix
fi
exit 0
