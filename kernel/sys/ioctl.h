/*
 * Mach Operating System
 * Copyright (c) 1991 Carnegie Mellon University
 * All Rights Reserved.
 * 
 * Permission to use, copy, modify and distribute this software and its
 * documentation is hereby granted, provided that both the copyright
 * notice and this permission notice appear in all copies of the
 * software, derivative works or modified versions, and any portions
 * thereof, and that both notices appear in supporting documentation.
 *
 * CARNEGIE MELLON ALLOWS FREE USE OF THIS SOFTWARE IN ITS "AS IS"
 * CONDITION.  CARNEGIE MELLON DISCLAIMS ANY LIABILITY OF ANY KIND FOR
 * ANY DAMAGES WHATSOEVER RESULTING FROM THE USE OF THIS SOFTWARE.
 *
 * Carnegie Mellon requests users of this software to return to
 *
 *  Software Distribution Coordinator  or  Software.Distribution@CS.CMU.EDU
 *  School of Computer Science
 *  Carnegie Mellon University
 *  Pittsburgh PA 15213-3890
 *
 * any improvements or extensions that they make and grant Carnegie Mellon rights
 * to redistribute these changes.
 */
/*
 * HISTORY
 * $Log:	ioctl.h,v $
 * Revision 2.6  92/05/21  17:25:06  jfriedl
 * 	Appended 'U' to constants that would otherwise be signed.
 * 	[92/05/16            jfriedl]
 * 
 * Revision 2.5  91/10/09  16:18:46  af
 * 	 Revision 2.4.1.1  91/09/01  15:53:00  af
 * 	 	Upgraded to BSD 4.4.
 * 	 	[91/09/01            af]
 * 
 * Revision 2.4.1.1  91/09/01  15:53:00  af
 * 	Upgraded to BSD 4.4.
 * 	[91/09/01            af]
 * 
 * Revision 2.4  91/05/14  17:40:04  mrt
 * 	Correcting copyright
 * 
 * Revision 2.3  91/03/16  15:01:35  rpd
 * 	Fixed the definitions for ANSI C.
 * 	[91/02/20            rpd]
 * 
 * Revision 2.2  91/02/14  15:04:02  mrt
 * 	Changed to new Mach copyright
 * 
 * 
 */
/*
 * Format definitions for 'ioctl' commands in device definitions.
 *
 * From BSD4.4.
 */

#ifndef _SYS_IOCTL_H_
#define _SYS_IOCTL_H_
/*
 * Ioctl's have the command encoded in the lower word, and the size of
 * any in or out parameters in the upper word.  The high 3 bits of the
 * upper word are used to encode the in/out status of the parameter.
 */
#define	IOCPARM_MASK	0x1fff		/* parameter length, at most 13 bits */
#define	IOC_VOID	0x20000000	/* no parameters */
#define	IOC_OUT		0x40000000	/* copy out parameters */
#define	IOC_IN		0x80000000U	/* copy in parameters */
#define	IOC_INOUT	(IOC_IN|IOC_OUT)

#define _IOC(inout,group,num,len) \
	(inout | ((len & IOCPARM_MASK) << 16) | ((group) << 8) | (num))
#define	_IO(g,n)	_IOC(IOC_VOID,	(g), (n), 0)
#define	_IOR(g,n,t)	_IOC(IOC_OUT,	(g), (n), sizeof(t))
#define	_IOW(g,n,t)	_IOC(IOC_IN,	(g), (n), sizeof(t))
#define	_IOWR(g,n,t)	_IOC(IOC_INOUT,	(g), (n), sizeof(t))

#endif	 _SYS_IOCTL_H_
