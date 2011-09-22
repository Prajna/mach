#
# Mach Operating System
# Copyright (c) 1993-1986 Carnegie Mellon University
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
# Revision 2.10  93/08/02  21:43:37  mrt
# 	Added rule for mach_norma_user.c and mach_norma.h
# 	[93/06/28            mrt]
# 
# Revision 2.9  93/06/02  20:49:55  rvb
# 	Added definition of NOSTRIP.
# 	[93/06/02            mrt]
# 
# Revision 2.8  93/05/28  17:40:45  mrt
# 	Change relink rule to build mach.boot as well as linking the
# 	kernel.
# 	[93/05/28            mrt]
# 
# Revision 2.7  93/05/17  17:17:17  mrt
# 	Removed EXPORTBASE/kernel from list of INDIRS. We now
# 	put all those files in the regular export dir.
# 	Removed use of version.minor
# 	[93/05/14            mrt]
# 
# Revision 2.6  93/05/11  12:55:27  rvb
# 	We don't profile the kernel yet.
# 
# Revision 2.5  93/05/10  19:40:54  rvb
# 	Added trailing ` on cat ${VERSION_FILES_SOURCE line.
# 	Fix courtesy of Ian Dall.
# 	[93/04/29            mrt]
# 
# Revision 2.4  93/05/10  17:45:50  rvb
# 	Typo missing `
# 	Ian Dall <DALL@hfrd.dsto.gov.au>	4/28/93
# 	[93/05/10  13:17:11  rvb]
# 
# 	Use CC_OPT_LEVEL and CC_OPT_EXTRA for max flexibility
# 	[93/05/07            rvb]
# 
# Revision 2.3  93/02/15  14:18:09  mrt
# 	Fixed it to only install the bootable kernel.
# 	Added the CFLAGS macro
# 	[93/02/05            mrt]
# 
# Revision 2.2  93/02/05  12:36:11  danner
# 	Fixed support for profiling.
# 	Added mach4.  - danner
# 	Renamed Makefile.template to template.mk and modifed for ODE
# 	make.
# 	[93/01/21            mrt]
# 
# Revision 2.34  92/04/05  18:28:30  rpd
# 	Added ${VOLATILE} to genassym.c processing.
# 	[92/03/27            rpd]
# 	Changed to use Makefile-common (for some definitions).
# 	New cross-compilation support.
# 	[92/03/18            jvh]
# 
# Revision 2.33  92/03/05  18:53:45  rpd
# 	Corrected the dependencies for device/dev_forward.c
# 	and mach/mach_norma_user.c.
# 	Added a rule for mach/mach_user.c.
# 	[92/03/04            rpd]
# 
# Revision 2.32  92/02/29  15:33:24  rpd
# 	Restored explicit vers.c/vers.o rules.
# 	[92/02/27            rpd]
# 
# Revision 2.31  92/02/25  14:13:22  elf
# 	Defined ASCPP and initialized it to ${CPP}
# 	[92/02/23            elf]
# 
# 	Moved definition of MKDIRS to where it can be overriden by the
# 	machine dependent makefile fragment.
# 	[92/02/21            elf]
# 
# Revision 2.30  92/02/19  15:07:08  elf
# 	Simplified - no more *_RULE_[1234] stuff.
# 	Condensed history.
# 	[92/02/12            rpd]
# 
#
# Condensed history:
#	Changes for out-of-kernel bootstrap (dbg).
#	Changes for norma (jsb).
#	Changes for MACH_KERNEL (dbg).
#	Added mach_host.defs and related files (dlb).
#	Mucho hacking at Mig rules (rpd).
#	Lint hacking (mwyoung).
#	New memory_object files (mwyoung).
#	CPATH and -I hacking (mja, rpd).
#	Running Mig in the build directory (mwyoung).
#	VOLATILE hacking (rvb).
#	Created from old Makefile.vax (mja).
#	Reorganized for simplified config program (mja).
#	Introduced new make depend stuff (rvb).

VPATH		=..

# We want the MK version & configuration to be part of this name.
# Makeconf defined KERNEL_CONFIG and VERSION for us.

CONFIG		?=${KERNEL_${TARGET_MACHINE}_CONFIG:U${KERNEL_CONFIG:UDEFAULT}}

.if exists( ${MAKETOP}Makefile-version)
.include "${MAKETOP}Makefile-version"
.endif

# set BINARIES to get the osf.obj.mk rules included
BINARIES	=

MACH_KERNEL 	= mach_kernel.${VERSION}.${CONFIG}
MACH_BOOT 	= mach.boot.${VERSION}.${CONFIG}
BOOTSTRAP	= bootstrap.${VERSION}
MKODIRS		= mach/ mach_debug/ device/

OTHERS		= ${MACH_BOOT}
ILIST		= ${MACH_BOOT}
IDIR		= /special/
NOSTRIP		=

DEPENDENCIES	=

INCFLAGS	= -I..


#  Pre-processor environment.
#  config outputs a definition of IDENT.
#  LOCAL_DEFINES can be set by makeoptions in MASTER.local.
#  LINENO is (optionally) set from the config file
#  (eg. -DTIMEZONE=-120)
#  (VOLATILE should be defined as "-Dvolatile=" if your
#  compiler doesn't support volatile declarations.)

VOLATILE	?=
DEFINES		= ${LOCAL_DEFINES} ${IDENT} -DKERNEL ${VOLATILE}
CC_OPT_EXTRA	?= ${LINENO}

# CFLAGS are for all normal files
# DRIVER_CFLAGS are for files marked as device-driver
# NPROFILING_CFLAGS are for files which support profile and thus
# 	should not be compiled with the profiling flags
#CFLAGS		= ${DEFINES} ${PROFILING:D-pg -DGPROF} 
CFLAGS		= ${DEFINES}
DRIVER_CFLAGS	=${CFLAGS}
NPROFILING_CFLAGS=${DEFINES}


#
# Mig variables
#

MIGKSFLAGS	= -DKERNEL -DKERNEL_SERVER
MIGKUFLAGS	= -DKERNEL -DKERNEL_USER
_MIGKSFLAGS_	= -DKERNEL -DKERNEL_SERVER ${_MIGFLAGS_}
_MIGKUFLAGS_	= -DKERNEL -DKERNEL_USER ${_MIGFLAGS_}

# Files where mig only needs to produce one target and the
# standard mig rules can be used


MIG_HDRS	= ${EXC_FILE} 
MIG_SRVS	= ${MACH_FILE} ${MACH_PORT_FILE} \
		  ${MACH_HOST_FILE} ${MACH_DEBUG_FILE} \
		  ${DEVICE_FILE} ${DEVICE_PAGER_FILE} \
		  ${DEFAULT_PAGER_OBJECT_FILE} \
		  ${MACH_NORMA_FILE} \
		  ${NORMA_INTERNAL_FILE} \
		  ${MACH4_FILE}

EXC_FILE				= mach/exc.h
${EXC_FILE}_MIGFLAGS			= ${MIGKUFLAGS}

MACH_FILE				= mach/mach_server.c
${MACH_FILE}_MIGFLAGS			= ${MIGKSFLAGS}
${MACH_FILE}_MIGSDEF			= mach/mach.defs

MACH_PORT_FILE				= mach/mach_port_server.c
${MACH_PORT_FILE}_MIGFLAGS		= ${MIGKSFLAGS}
${MACH_PORT_FILE}_MIGSDEF		= mach/mach_port.defs

MACH_HOST_FILE				= mach/mach_host_server.c
${MACH_HOST_FILE}_MIGFLAGS		= ${MIGKSFLAGS}
${MACH_HOST_FILE}_MIGSDEF		= mach/mach_host.defs

MACH_DEBUG_FILE				= mach_debug/mach_debug_server.c
${MACH_DEBUG_FILE}_MIGFLAGS		= ${MIGKSFLAGS}
${MACH_DEBUG_FILE}_MIGSDEF		= mach_debug/mach_debug.defs

DEVICE_FILE				= device/device_server.c
${DEVICE_FILE}_MIGFLAGS			= ${MIGKSFLAGS}
${DEVICE_FILE}_MIGSDEF			= device/device.defs

DEVICE_PAGER_FILE 			= device/device_pager_server.c
${DEVICE_PAGER_FILE}_MIGFLAGS		= ${MIGKSFLAGS}
${DEVICE_PAGER_FILE}_MIGSDEF		= mach/memory_object.defs

DEFAULT_PAGER_OBJECT_FILE		= mach/default_pager_object_server.c
${DEFAULT_PAGER_OBJECT_FILE}_MIGFLAGS	= ${MIGKSFLAGS}
${DEFAULT_PAGER_OBJECT_FILE}_MIGSDEF	= mach/default_pager_object.defs

MACH_NORMA_FILE				= mach/mach_norma_server.c
${MACH_NORMA_FILE}_MIGFLAGS		= ${MIGKSFLAGS}
${MACH_NORMA_FILE}_MIGSDEF		= mach/mach_norma.defs

NORMA_INTERNAL_FILE 			= mach/norma_internal_server.c
${NORMA_INTERNAL_FILE}_MIGFLAGS		= ${MIGKSFLAGS}
${NORMA_INTERNAL_FILE}_MIGSDEF		= norma/norma_internal.defs

MACH4_FILE				= mach/mach4_server.c
${MACH4_FILE}_MIGFLAGS			= ${MIGKSFLAGS}
${MACH4_FILE}_MIGSDEF			= mach/mach4.defs

#
#  LDOBJS is the set of object files which comprise the kernel.
#  LDOBJS_PREFIX and LDOBJS_SUFFIX are defined in the machine
#  dependent Makefile (if necessary).
#
LDOBJS=${LDOBJS_PREFIX} ${OBJS} ${LDOBJS_SUFFIX}

#
#  LDDEPS is the set of extra dependencies associated with
#  loading the kernel.
#
#  LDDEPS_PREFIX is defined in the machine dependent Makefile
#  (if necessary).
#
LDDEPS=${LDDEPS_PREFIX}


LDDEPS		=${LDDEPS_PREFIX} 

#
#  These macros are filled in by the config program depending on the
#  current configuration.  The MACHDEP macro is replaced by the
#  contents of the machine dependent makefile template and the others
#  are replaced by the corresponding symbol definitions for the
#  configuration.
#

%OBJS

%CFILES

%CFLAGS

%SFILES

%BFILES

%ORDERED


#  All macro definitions should be before this point,
#  so that the machine dependent fragment can redefine the macros.
#  All rules (that use macros) should be after this point,
#  so that they pick up any redefined macro values.

%MACHDEP


.include <${RULES_MK}>

# drop the standard .y .l and other suffixes which are not used
# in the kernel to save time

.SUFFIXES:
.SUFFIXES:  .o .s .c .b .X .out .h

#
#  OBJSDEPS is the set of files (defined in the machine dependent
#  template if necessary) which all objects depend on (such as an
#  in-line assembler expansion filter
#

${OBJS}: ${OBJSDEPS}

.PRECIOUS: Makefile

#
# The machine dependent template.mk may define the load flags
# so this is only a default value.

MK_LDFLAGS 	?= ${${TARGET_MACHINE}_LDFLAGS:U${LDFLAGS}}

NEWVERS_DEPS = \
	conf/version.major \
	conf/version.variant \
	conf/version.edit \
	conf/version.patch \
	conf/newvers.sh	\
	conf/copyright

VERSION_FILES_SOURCE = \
	${conf/version.major:P} \
	${conf/version.variant:P} \
	${conf/version.edit:P} \
	${conf/version.patch:P}


${MACH_KERNEL}: ${PRELDDEPS} ${LDOBJS} ${LDDEPS} ${SYSDEPS} \
		${CC_DEPS_NORMAL} ${NEWVERS_DEPS} LINKKERNEL

# The relink rule allows you to relink the kernel without checking
#  all the dependencies. 

relink: ${MACH_KERNEL}.relink

${MACH_KERNEL}.relink: ${LDDEPS} ${NEWVERS_DEPS} LINKKERNEL MAKEBOOT


#  We create vers.c/vers.o right here so that the timestamp in vers.o
#  always reflects the time that the kernel binary is actually created.
#  We link the kernel binary to "mach_kernel" so that there is a short name
#  for the most recently created binary in the object directory.

LINKKERNEL:	.USE
	@echo "creating vers.o"
	@${RM} -f vers.c vers.o
	@sh ${conf/newvers.sh:P} ${conf/copyright:P} `cat ${VERSION_FILES_SOURCE}`
	@${_CC_} -c ${_CCFLAGS_} vers.c
	@${RM} -f ${MACH_KERNEL} ${MACH_KERNEL}.out ${MACH_KERNEL}.unstripped
	@echo "loading ${MACH_KERNEL}"
	${_LD_} -o ${MACH_KERNEL}.out ${MK_LDFLAGS} \
		${LDOBJS} vers.o ${LIBS} ${LDLIBS} && \
		${MV} ${MACH_KERNEL}.out ${MACH_KERNEL}.unstripped
	-${SIZE} ${MACH_KERNEL}.unstripped
	${CP} ${MACH_KERNEL}.unstripped ${MACH_KERNEL}.out
	${XSTRIP} ${MACH_KERNEL}.out && ${MV} ${MACH_KERNEL}.out ${MACH_KERNEL}
	@${RM} -f mach_kernel
	ln ${MACH_KERNEL} mach_kernel
	${CP} ${MACH_KERNEL} ${EXPORTBASE}/special/${MACH_KERNEL}



# mach.boot is made with makeboot, which combines a kernel image
# and a bootstrap image to produce a single bootable image.
# The sh mess does generates a dependency on makeboot 
#
# Just as ${MACH_KERNEL} is linked to mach_kernel,
# ${MACH_BOOT} is linked to mach.boot.

# NB: ${MACH_KERNEL} is xstripped; That is the one we want

bootstrap	= ${EXPORTBASE}/special/${BOOTSTRAP}
${MACH_BOOT} : ${MACH_KERNEL} ${bootstrap} MAKEBOOT

MAKEBOOT:	.USE
	@echo "${MACH_BOOT}: $${makeboot}"  >> ${MACH_BOOT}.d; \
	echo "[ generating ${MACH_BOOT} from ${MACH_KERNEL} and ${bootstrap} ]"; 
	makeboot="`${WH} -q makeboot`"; \
	${RM} -f ${MACH_BOOT} mach.boot; \
	$${makeboot} -o ${MACH_BOOT} ${MACH_KERNEL} ${bootstrap} && \
	ln ${MACH_BOOT} mach.boot
	cp ${MACH_BOOT} ${EXPORTBASE}/special/${MACH_BOOT}


${OBJS}: ${OBJSDEPS}

# Use the standard rules with slightly non-standard .IMPSRC

${COBJS}: $${$${.TARGET}_SOURCE}

${SOBJS}: $${$${.TARGET}_SOURCE}

${BOBJS}: $${$${.TARGET}_SOURCE}


#
#  Rules for components which are not part of the kernel proper or that
#  need to be built in a special manner.
#

# genassym needs to be preprocessed as for the target machine but run
# on the host machine.  In order to get the correct predefines it is
# necessary to use the target preprocessor.

HOST_INCDIRS	= ${INCDIRS}
HOST_CFLAGS	= ${DEFINES}
HOST_LDFLAGS	= ${LDFLAGS}
genassym_CCTYPE	= host

xxx_genassym.c: ${KERN_MACHINE_DIR}/genassym.c
	${ansi_CPP} -P ${_CCFLAGS_}  ${${KERN_MACHINE_DIR}/genassym.c:P} > xxx_genassym.c
	cat /dev/null >> genassym.d
	sed 's/^genassym\.o/xxx_genassym.c/' genassym.d > xxx_genassym.c.d;
	${RM} -f genassym.d

genassym: xxx_genassym.c
	( LPATH="${_host_LPATH_}"; export LPATH; \
	 ${HOST_CC}  ${_CC_CFLAGS_} -o  genassym.X xxx_genassym.c )
	${MV} genassym.X genassym

#
#  Special dependencies for locore.
#

assym.s: genassym
	./genassym > ${.TARGET}

locore.o: assym.s ${LOCORE_HEADS}

# Make all the directories in the object directory

.BEGIN:
	-makepath ${MKODIRS}

vm_pageout.o: mach/memory_object_user.h mach/memory_object_default.h

vm_object.o: mach/memory_object_user.h mach/memory_object_default.h

vm_fault.o: mach/memory_object_user.h

memory_object.o: mach/memory_object_user.h

exception.o: mach/exc.h

dev_pager.o : device/device_pager_server.c mach/mach_user_kernel.h

ds_routines.o : device/device_reply.h


#  The Mig-generated files go into subdirectories.
#  and have non-standard names. Also mig rules that build
#  multiple targets have to generate an intermediate "timestamp"
#  target that the final targets can depend on in order 
#  to avoid being run redundently


MACH_KU_FUNCS = \
	memory_object_data_provided memory_object_data_unavailable \
	memory_object_data_error memory_object_set_attributes \
	memory_object_data_supply memory_object_ready \
	memory_object_change_attributes

MACH_KU_FFILES = ${MACH_KU_FUNCS:@.F.@mach/${.F.}.c@}
MACH_KU_FILES = mach/mach_user_kernel.h $(MACH_KU_FFILES)

$(MACH_KU_FILES): mach_stamp
mach_stamp: mach/mach.defs
	@echo ""
	@touch mach_stamp
	${MIG} ${_MIGKUFLAGS_} \
	    -header mach/mach_user_kernel.h \
	    -i mach/ -user '$$(MACH_KU_FFILES)' \
	    -server /dev/null \
		${mach/mach.defs:P}

MEMORY_OBJECT_FILES = mach/memory_object_user.h mach/memory_object_user.c

${MEMORY_OBJECT_FILES}: memory_object_stamp
memory_object_stamp: mach/memory_object.defs
	@echo ""
	@touch memory_object_stamp
	${MIG}  ${_MIGKUFLAGS_} -DSEQNOS \
		-header mach/memory_object_user.h\
		-user mach/memory_object_user.c\
		-server /dev/null \
		${mach/memory_object.defs:P} 


MEMORY_OBJECT_DEFAULT_FILES = mach/memory_object_default.h \
	mach/memory_object_default_user.c

${MEMORY_OBJECT_DEFAULT_FILES}: memory_object_default_stamp
memory_object_default_stamp: mach/memory_object_default.defs
	@echo ""
	@touch memory_object_default_stamp
	${MIG}  ${_MIGKUFLAGS_} -DSEQNOS \
		-header mach/memory_object_default.h\
		-user mach/memory_object_default_user.c\
		-server /dev/null \
		${mach/memory_object_default.defs:P}

PROXY_FILES = mach/proxy.h mach/proxy_user.c mach/proxy_server.c

$(PROXY_FILES): xmm_proxy_stamp
xmm_proxy_stamp: norma/xmm_proxy.defs
	@touch xmm_proxy_stamp
	@echo ""
	$(MIG)  $(_MIGKUFLAGS_) \
		-header mach/proxy.h \
		-user mach/proxy_user.c \
		-server /dev/null \
		${norma/xmm_proxy.defs:P}
	$(MIG)  $(_MIGKSFLAGS_) \
		-header /dev/null \
		-user /dev/null \
		-server mach/proxy_server.c \
		${norma/xmm_proxy.defs:P}

DEVKUSER_FILES = mach/dev_forward.c mach/dev_forward.h

${DEVKUSER_FILES}: dev_forward_stamp
dev_forward_stamp: device/dev_forward.defs
	@touch dev_forward_stamp
	@echo ""
	${MIG}  ${_MIGKUFLAGS_}\
		-header mach/dev_forward.h\
		-user mach/dev_forward.c\
		-server /dev/null\
		${device/dev_forward.defs:P}


DEVICE_REPLY_FILES = device/device_reply.h device/device_reply_user.c

${DEVICE_REPLY_FILES}: device_reply_stamp
device_reply_stamp: device/device_reply.defs
	@touch device_reply_stamp
	@echo ""
	${MIG}  ${_MIGKUFLAGS_}\
		-header device/device_reply.h\
		-user device/device_reply_user.c\
		-server /dev/null\
		${device/device_reply.defs:P}

MACH_NORMA_USER_FILES	= mach/mach_norma.h mach/mach_norma_user.c

${MACH_NORMA_USER_FILES}: mach_norma_user_stamp
mach_norma_user_stamp: mach/mach_norma.defs
	@touch mach_norma_user_stamp
	@echo "" 
	$(MIG)  $(_MIGKUFLAGS_) \
	    -header mach/mach_norma.h \
	    -user mach/mach_norma_user.c \
	    -server /dev/null \
		${mach/mach_norma.defs:P}

NORMA_INTERNAL_USER_FILES = mach/norma_internal.h mach/norma_internal_user.c

$(NORMA_INTERNAL_USER_FILES): norma_internal_stamp
norma_internal_stamp: norma/norma_internal.defs
	@touch norma_internal_stamp
	@echo ""
	$(MIG)  $(_MIGKUFLAGS_) \
	    -header mach/norma_internal.h \
	    -user mach/norma_internal_user.c \
	    -server /dev/null \
		${norma/norma_internal.defs:P}

MACH_USER_FILES = mach/mach_user.h mach/mach_user.c

${MACH_USER_FILES}: mach_user_stamp
mach_user_stamp: mach/mach.defs
	@touch mach_user_stamp
	@echo ""
	${MIG} ${_MIGKUFLAGS_} \
                -header mach/mach_user.h \
                -user mach/mach_user.c \
                -server /dev/null \
		${mach/mach.defs:P}


