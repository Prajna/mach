/* 
 * Mach Operating System
 * Copyright (c) 1993-1989 Carnegie Mellon University
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
 * any improvements or extensions that they make and grant Carnegie Mellon
 * the rights to redistribute these changes.
 */
/* 
 * HISTORY
 * $Log:	conf.c,v $
 * Revision 2.16  93/05/17  15:04:47  rvb
 * 	Type casts, etc to quiet gcc 2.3.3 warnings
 * 	[93/05/17            rvb]
 * 
 * Revision 2.15  93/03/09  10:54:33  danner
 * 	Added SCSI CD-ROM Audio.  Added back SCSI processor links.
 * 	[93/03/06            af]
 * 
 * Revision 2.14  93/02/05  08:18:31  danner
 * 	Added include.
 * 	[93/02/04            danner]
 * 
 * Revision 2.13  93/01/24  13:15:14  danner
 * 	Add d-link "600" ethernet device "de"
 * 	[92/08/13            rvb]
 * 
 * Revision 2.12  92/02/19  16:29:40  elf
 * 	Add lpr and par devices.  (taken from 2.5)
 * 	[92/02/13            rvb]
 * 
 * Revision 2.11  92/01/03  20:39:53  dbg
 * 	Added devinfo routine to scsi to accomodate MI change
 * 	that screwed up extra large writes.
 * 	[91/12/26  11:06:54  af]
 * 
 * Revision 2.10  91/08/28  11:11:37  jsb
 * 	Fixed field-describing comment in dev_name_list definition.
 * 	[91/08/27  17:52:06  jsb]
 * 
 * 	Convert bsize entries to devinfo entries.  Add nodev entries for
 *	devices that don't support devinfo.
 * 	[91/08/15  18:43:13  jsb]
 * 
 * 	Add block size entries for hd and fd.
 * 	[91/08/12  17:32:55  dlb]
 * 
 * Revision 2.9  91/08/24  11:57:26  af
 * 	Added SCSI disks, tapes, and cpus.
 * 	[91/08/02  02:56:08  af]
 * 
 * Revision 2.8  91/05/14  16:22:01  mrt
 * 	Correcting copyright
 * 
 * Revision 2.7  91/02/14  14:42:13  mrt
 * 	Allow com driver and distinguish EtherLinkII from wd8003
 * 	[91/01/28  15:27:02  rvb]
 * 
 * Revision 2.6  91/02/05  17:16:44  mrt
 * 	Changed to new Mach copyright
 * 	[91/02/01  17:42:38  mrt]
 * 
 * Revision 2.5  91/01/08  17:32:42  rpd
 * 	Support for get/set status on hd and fd.
 * 	Also fd has 64 minor devices per unit.
 * 	Switch wd8003 -> ns8390
 * 	[91/01/04  12:17:15  rvb]
 * 
 * Revision 2.4  90/10/01  14:23:02  jeffreyh
 * 	added wd8003 ethernet driver
 * 	[90/09/27  18:23:53  jeffreyh]
 * 
 * Revision 2.3  90/05/21  13:26:53  dbg
 * 	Add mouse, keyboard, IOPL devices.
 * 	[90/05/17            dbg]
 * 
 * Revision 2.2  90/05/03  15:41:34  dbg
 * 	Add 3c501 under name 'et'.
 * 	[90/04/27            dbg]
 * 
 * 	Created.
 * 	[90/02/20            dbg]
 * 
 */

/*
 * Device switch for i386 AT bus.
 */

#include <mach/machine/vm_types.h>
#include <device/conf.h>

extern vm_offset_t block_io_mmap();

extern int	timeopen(), timeclose();
extern vm_offset_t timemmap();
#define	timename		"time"

#include <hd.h>
#if	NHD > 0
extern int	hdopen(), hdclose(), hdread(), hdwrite();
extern int	hdgetstat(), hdsetstat(), hddevinfo();
#define	hdname			"hd"
#endif	NHD > 0

#include <aha.h>
#if	NAHA > 0
int     rz_open(), rz_close(), rz_read(), rz_write();
int     rz_get_status(), rz_set_status(), rz_devinfo();
int	cd_open(), cd_close(), cd_read(), cd_write();
#define rzname "sd"
#define	tzname "st"
#define	scname "sc"	/* processors */
#define cdname	"cd_audio"	/* CD-ROM DA */

#endif	/*NAHA > 0*/

#include <fd.h>
#if	NFD > 0
extern int	fdopen(), fdclose(), fdread(), fdwrite();
extern int	fdgetstat(), fdsetstat(), fddevinfo();
#define	fdname			"fd"
#endif	NFD > 0

#include <wt.h>
#if	NWT > 0
extern int	wtopen(), wtread(), wtwrite(), wtclose();
#define	wtname			"wt"
#endif	NWT > 0

#include <pc586.h>
#if	NPC586 > 0
extern int	pc586open(), pc586output(), pc586getstat(), pc586setstat(),
		pc586setinput();
#define	pc586name		"pc"
#endif NPC586 > 0

#include <ns8390.h>
#if	NNS8390 > 0
extern int	wd8003open(), eliiopen();
extern int	ns8390output(), ns8390getstat(), ns8390setstat(), 
		ns8390setinput();
#define	ns8390wdname		"wd"
#define	ns8390elname		"el"
#endif NNS8390 > 0

#include <at3c501.h>
#if	NAT3C501 > 0
extern int	at3c501open(), at3c501output(),
		at3c501getstat(), at3c501setstat(),
		at3c501setinput();
#define	at3c501name		"et"
#endif NAT3C501 > 0

#include <par.h>
#if	NPAR > 0
extern int	paropen(), paroutput(), pargetstat(), parsetstat(),
		parsetinput();
#define	parname		"par"
#endif NPAR > 0

#include <de6c.h>
#if	NDE6C > 0
extern int	de6copen(), de6coutput(), de6cgetstat(), de6csetstat(),
		de6csetinput();
#define	de6cname		"de"
#endif NDE6C > 0

extern int	kdopen(), kdclose(), kdread(), kdwrite();
extern int	kdgetstat(), kdsetstat(), kdportdeath();
extern vm_offset_t kdmmap();
#define	kdname			"kd"

#include <com.h>
#if	NCOM > 0
extern int	comopen(), comclose(), comread(), comwrite();
extern int	comgetstat(), comsetstat(), comportdeath();
#define	comname			"com"
#endif	NCOM > 0

#include <lpr.h>
#if	NLPR > 0
extern int	lpropen(), lprclose(), lprread(), lprwrite();
extern int	lprgetstat(), lprsetstat(), lprportdeath();
#define	lprname			"lpr"
#endif	NLPR > 0

#include <blit.h>
#if NBLIT > 0
extern int	blitopen(), blitclose(), blit_get_stat();
extern vm_offset_t blitmmap();
#define	blitname		"blit"

extern int	mouseinit(), mouseopen(), mouseclose();
extern int	mouseioctl(), mouseselect(), mouseread();
#endif

extern int	kbdopen(), kbdclose(), kbdread();
extern int	kbdgetstat(), kbdsetstat();
#define	kbdname			"kbd"

extern int	mouseopen(), mouseclose(), mouseread();
#define	mousename		"mouse"

extern int	ioplopen(), ioplclose();
extern vm_offset_t ioplmmap();
#define	ioplname		"iopl"


/*
 * List of devices - console must be at slot 0
 */
struct dev_ops	dev_name_list[] =
{
	/*name,		open,		close,		read,
	  write,	getstat,	setstat,	mmap,
	  async_in,	reset,		port_death,	subdev,
	  dev_info */

	{ kdname,	kdopen,		kdclose,	kdread,
	  kdwrite,	kdgetstat,	kdsetstat,	kdmmap,
	  nodev,	nulldev,	kdportdeath,	0,
	  nodev },

	{ timename,	timeopen,	timeclose,	nulldev,
	  nulldev,	nulldev,	nulldev,	timemmap,
	  nodev,	nulldev,	nulldev,	0,
	  nodev },

#if	NHD > 0
	{ hdname,	hdopen,		hdclose,	hdread,
	  hdwrite,	hdgetstat,	hdsetstat,	nomap,
	  nodev,	nulldev,	nulldev,	16,
	  hddevinfo },
#endif	NHD > 0

#if	NAHA > 0
	{ rzname,	rz_open,	rz_close,	rz_read,
	  rz_write,	rz_get_status,	rz_set_status,	nomap,
	  nodev,	nulldev,	nulldev,	8,
	  rz_devinfo },

	{ tzname,	rz_open,	rz_close,	rz_read,
	  rz_write,	rz_get_status,	rz_set_status,	nomap,
	  nodev,	nulldev,	nulldev,	8,
	  nodev },

	{ cdname,	cd_open,	cd_close,	cd_read,
	  cd_write,	nodev,		nodev,		nomap,
	  nodev,	nulldev,	nulldev,	8,
	  nodev },

	{ scname,	rz_open,	rz_close,	rz_read,
	  rz_write,	rz_get_status,	rz_set_status,	nomap,
	  nodev,	nulldev,	nulldev,	8,
	  nodev },

#endif	/*NAHA > 0*/

#if	NFD > 0
	{ fdname,	fdopen,		fdclose,	fdread,
	  fdwrite,	fdgetstat,	fdsetstat,	nomap,
	  nodev,	nulldev,	nulldev,	64,
	  fddevinfo },
#endif	NFD > 0

#if	NWT > 0
	{ wtname,	wtopen,		wtclose,	wtread,
	  wtwrite,	nulldev,	nulldev,	nomap,
	  nodev,	nulldev,	nulldev,	0,
	  nodev },
#endif	NWT > 0

#if	NPC586 > 0
	{ pc586name,	pc586open,	nulldev,	nulldev,
	  pc586output,	pc586getstat,	pc586setstat,	nomap,
	  pc586setinput,nulldev,	nulldev, 	0,
	  nodev },
#endif

#if	NAT3C501 > 0
	{ at3c501name,	at3c501open,	nulldev,	nulldev,
	  at3c501output,at3c501getstat,	at3c501setstat,	nomap,
	  at3c501setinput, nulldev,	nulldev, 	0,
	  nodev },
#endif

#if	NNS8390 > 0
	{ ns8390wdname,	wd8003open,	nulldev,	nulldev,
	  ns8390output, ns8390getstat,	ns8390setstat,	nomap,
	  ns8390setinput, nulldev,	nulldev,	0,
	  nodev },

	{ ns8390elname,	eliiopen,	nulldev,	nulldev,
	  ns8390output, ns8390getstat,	ns8390setstat,	nomap,
	  ns8390setinput, nulldev,	nulldev,	0,
	  nodev },
#endif

#if	NPAR > 0
	{ parname,	paropen,	nulldev,	nulldev,
	  paroutput,	pargetstat,	parsetstat,	nomap,
	  parsetinput,	nulldev,	nulldev, 	0,
	  nodev },
#endif

#if	NDE6C > 0
	{ de6cname,	de6copen,	nulldev,	nulldev,
	  de6coutput,	de6cgetstat,	de6csetstat,	nomap,
	  de6csetinput,	nulldev,	nulldev, 	0,
	  nodev },
#endif

#if	NCOM > 0
	{ comname,	comopen,	comclose,	comread,
	  comwrite,	comgetstat,	comsetstat,	nomap,
	  nodev,	nulldev,	comportdeath,	0,
	  nodev },
#endif

#if	NLPR > 0
	{ lprname,	lpropen,	lprclose,	lprread,
	  lprwrite,	lprgetstat,	lprsetstat,	nomap,
	  nodev,	nulldev,	lprportdeath,	0,
	  nodev },
#endif

#if	NBLIT > 0
	{ blitname,	blitopen,	blitclose,	nodev,
	  nodev,	blit_get_stat,	nodev,		blitmmap,
	  nodev,	nodev,		nodev,		0,
	  nodev },
#endif

	{ mousename,	mouseopen,	mouseclose,	mouseread,
	  nodev,	nulldev,	nulldev,	nomap,
	  nodev,	nulldev,	nulldev,	0,
	  nodev },

	{ kbdname,	kbdopen,	kbdclose,	kbdread,
	  nodev,	kbdgetstat,	kbdsetstat,	nomap,
	  nodev,	nulldev,	nulldev,	0,
	  nodev },

	{ ioplname,	ioplopen,	ioplclose,	nodev,
	  nodev,	nodev,		nodev,		ioplmmap,
	  nodev,	nulldev,	nulldev,	0,
	  nodev },
};
int	dev_name_count = sizeof(dev_name_list)/sizeof(dev_name_list[0]);

/*
 * Indirect list.
 */
struct dev_indirect dev_indirect_list[] = {

	/* console */
	{ "console",	&dev_name_list[0],	0 }
};
int	dev_indirect_count = sizeof(dev_indirect_list)
				/ sizeof(dev_indirect_list[0]);
