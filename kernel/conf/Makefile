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
# Revision 2.2  93/02/04  17:10:57  mrt
# 	Created for ODE make.
# 	[92/12/26            mrt]
# 


MASTER		= MASTER
MASTER_TM	= ${target_cpu}/MASTER
MASTER_LOCAL	= MASTER.local
MASTER_TM_LOCAL	= ${target_cpu}/MASTER.local

# We want the MK version & configuration to be part of this name.
# Makeconf defined KERNEL_CONFIG, KERNEL_VERSION and VERSION

CONFIG		?= ${KERNEL_${TARGET_MACHINE}_CONFIG:U${KERNEL_CONFIG:UDEFAULT}}
VERSION		?= ${KERNEL_VERSION}

OTHERS		= ../${CONFIG}/Makefile

MACH_KERNEL = mach_kernel.${VERSION}.${CONFIG}
MACH_BOOT = mach.boot.${VERSION}.${CONFIG}
BOOTSTRAP = bootstrap.${VERSION}


.include <${RULES_MK}>


# The doconf process produces a configuration file in the object subdirectory.
# The configuration file has the same name as the subdirectory, ie ${CONFIG}.
# As input it uses the various MASTER files from the conf subdirectory.


${CONFIG}:	${MASTER} ${MASTER_TM} \
		${MASTER_LOCAL} ${MASTER_TM_LOCAL} 
	@echo "[ generating ${.TARGET} from {,${target_cpu}/}MASTER{,.local} ]"
	@doconf="`wh -q doconf`"; \
	echo "${.TARGET}: $${doconf}" >> ${.TARGET}.d; \
	$${doconf} ${CONFIG}\
		${${MASTER}:P}\
		${${MASTER_TM}:P}\
		${${MASTER_LOCAL}:P}\
		${${MASTER_TM_LOCAL}:P} >${CONFIG}.tmp
	@-if [ -s ${CONFIG}.tmp ]; then \
	    if [ -f ${CONFIG} ]; then \
		diff ${CONFIG} ${CONFIG}.tmp; \
		mv ${CONFIG} ${CONFIG}.old; \
	    fi; \
	    mv ${CONFIG}.tmp ${CONFIG}; \
	else \
	    ${RM} -f ${CONFIG}.tmp; \
	fi


# The config process produces Makefile in the object subdirectory.
# As input it uses the configuration file from doconf and some files
# from the conf subdirectory.
#
# We generate a dependency on config.
#
# We save the current configuration in vers.config, so that if the
# object directory is renamed we will know to reconfigure.  We remove
# the ${MACH_KERNEL} binary, to guarantee that it is at least relinked.
# (For example, the current configuration might contain a subset of
# the .o files in the previous configuration.)

../${CONFIG}/Makefile:	${CONFIG} \
			files \
			${target_cpu}/files \
			template.mk \
			${target_cpu}/template.mk 
	@echo "[ configuring ${CONFIG} ]"
	@-if [ -d ../${CONFIG} ]; then true; else mkdir ../${CONFIG}; fi
	@config="`wh -q config`"; \
	echo "${.TARGET}: $${config}" >> ${.TARGET}.d; \
	$${config} `genpath -I.` -c . ${CONFIG}
	@${RM} -f ../${CONFIG}/Makefile
	@ln -s Makefile.internal ../${CONFIG}/Makefile
	@echo ${CONFIG} >../${CONFIG}/vers.config
	@${RM} -f ../${CONFIG}/${MACH_KERNEL}


