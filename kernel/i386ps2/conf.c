/* 
 * Mach Operating System
 * Copyright (c) 1993-1989 Carnegie Mellon University
 * Copyright (c) 1991 IBM Corporation 
 * All Rights Reserved.
 * 
 * Permission to use, copy, modify and distribute this software and its
 * documentation is hereby granted, provided that both the copyright
 * notice and this permission notice appear in all copies of the
 * software, derivative works or modified versions, and any portions
 * thereof, and that both notices appear in supporting documentation,
 * and that the name IBM not be used in advertising or publicity 
 * pertaining to distribution of the software without specific, written
 * prior permission.
 * 
 * CARNEGIE MELLON AND IBM ALLOW FREE USE OF THIS SOFTWARE IN ITS "AS IS"
 * CONDITION.  CARNEGIE MELLON AND IBM DISCLAIM ANY LIABILITY OF ANY KIND FOR
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
 * COMPONENT_NAME: (CONSOLE) Console driver
 *
 * FUNCTIONS: 
 *
 * ORIGINS: 6, 27
 */

/* 
 * HISTORY
 * $Log:	conf.c,v $
 * Revision 2.5.1.1  94/02/17  14:09:35  mja
 * 	Added ethernet device.
 * 	[94/01/02            zon]
 * 
 * Revision 2.4  93/05/18  11:20:08  rvb
 * 	Lint: <>map is now a vm_offset_t
 * 
 * Revision 2.3  93/02/05  08:20:24  danner
 * 	Fix includes.
 * 
 * 
 * Revision 2.2  93/02/04  07:59:07  danner
 * 	Integrate PS2 code from IBM.
 * 	[93/01/18            prithvi]
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
extern int	hdgetstat(), hdsetstat();
#define	hdname			"hd"
#endif	/* NHD > 0 */

#include <fd.h>
#if	NFD > 0
extern int	fdopen(), fdclose(), fdread(), fdwrite();
extern int	fdgetstat(), fdsetstat();
#define	fdname			"fd"
#endif	/* NFD > 0 */

#include <wt.h>
#if	NWT > 0
extern int	wtopen(), wtread(), wtwrite(), wtclose();
#define	wtname			"wt"
#endif	/* NWT > 0 */

#include <pc586.h>
#if	NPC586 > 0
extern int	pc586open(), pc586output(), pc586getstat(), pc586setstat(),
		pc586setinput();
#define	pc586name		"pc"
#endif	/* NPC586 > 0 */

#include <ns8390.h>
#if	NNS8390 > 0
extern int	ns8390open(), ns8390output(), ns8390getstat(), ns8390setstat(),
		ns8390setinput();
#define	ns8390wdname		"wd"
#define	ns8390elname		"el"
#endif	/* NNS8390 > 0 */

#include <at3c501.h>
#if	NAT3C501 > 0
extern int	at3c501open(), at3c501output(),
		at3c501getstat(), at3c501setstat(),
		at3c501setinput();
#define	at3c501name		"et"
#endif	/* NAT3C501 > 0 */

#include <un.h>
#if	NUN > 0
extern int	unopen(), unoutput(), ungetstat(), unsetstat(),
		unsetinput();
#define	unname		"un"
#endif	/* NUN > 0 */

#include <en.h>
#if	NEN > 0
extern int	enopen(), enoutput(), engetstat(), ensetstat(),
		ensetinput();
#define enname		"en"
#endif	/* NEN > 0 */

#include <tr.h>
#if	NTR > 0
extern int	tropen(), troutput(), trgetstat(), trsetstat(),
		trsetinput();
#define	trname		"tr"
#endif	/* NTR > 0 */

#include <ibm_console.h>
#if IBM_CONSOLE > 0
extern int	cnopen(), cnclose(), cnread(), cnwrite();
extern int	cngetstat(), cnsetstat(), cnportdeath();
extern vm_offset_t cnmmap();
#define cnname			"cn"
#else /* IBM_CONSOLE > 0 */
extern int      kdopen(), kdclose(), kdread(), kdwrite();
extern int	kdgetstat(), kdsetstat(), kdportdeath();
extern vm_offset_t kdmmap();
#define	kdname			"kd"
#endif /* IBM_CONSOLE > 0 */

#include <com.h>
#if	NCOM > 0
extern int	comopen(), comclose(), comread(), comwrite();
extern int	comgetstat(), comsetstat(), comportdeath();
#define	comname			"com"
#endif	/* NCOM > 0 */

#include <qd.h>
#if	NQD > 0
extern int	qdopen(), qdclose(), qdread(), qdwrite(), qdioctl();
#define	qdname			"qd"
#endif	/* NQD > 0 */

#include <blit.h>
#if NBLIT > 0
extern int	blitopen(), blitclose(), blit_get_stat();
extern vm_offset_t blitmmap();
#define	blitname		"blit"
#endif

extern int	kbdopen(), kbdclose(), kbdread();
extern int	kbdgetstat(), kbdsetstat();
#define	kbdname			"kbd"

#include <mouse.h>
#if NMOUSE > 0
extern int	mouseinit(), mouseopen(), mouseclose();
extern int	mousegetstat(), mousesetstat(), mouseread();
#define	mousename		"mouse"
#endif

extern int	ioplopen(), ioplclose();
extern vm_offset_t ioplmmap();
#define	ioplname		"iopl"


/*
 * List of devices - console must be at slot 0
 */
struct dev_ops	dev_name_list[] =
{
        /*name,         open,           close,          read,
          write,        getstat,        setstat,        mmap,
          async_in,     reset,          port_death,     subdev,
          dev_info */

#if IBM_CONSOLE > 0
	{ cnname,	cnopen,		cnclose,	cnread,
	  cnwrite,	cngetstat,	cnsetstat,	cnmmap,
	  nodev,	nulldev,	cnportdeath,	0,
	  nodev },
#else /* IBM_CONSOLE > 0 */
	{ kdname,	kdopen,		kdclose,	kdread,
	  kdwrite,	kdgetstat,	kdsetstat,	kdmmap,
	  nodev,	nulldev,	kdportdeath,	0,
	  nodev },
#endif /* IBM_CONSOLE > 0 */

	{ timename,	timeopen,	timeclose,	nulldev,
	  nulldev,	nulldev,	nulldev,	timemmap,
	  nodev,	nulldev,	nulldev,	0,
	  nodev },

#if	NHD > 0
	{ hdname,	hdopen,		hdclose,	hdread,
	  hdwrite,	hdgetstat,	hdsetstat,	nomap,
	  nodev,	nulldev,	nulldev,	16,
	  nodev },
#endif

#if	NFD > 0
	{ fdname,	fdopen,		fdclose,	fdread,
	  fdwrite,	fdgetstat,	fdsetstat,	nomap,
	  nodev,	nulldev,	nulldev,	64,
	  nodev },
#endif

#if	NWT > 0
	{ wtname,	wtopen,		wtclose,	wtread,
	  wtwrite,	nulldev,	nulldev,	nomap,
	  nodev,	nulldev,	nulldev,	0,
	  nodev },
#endif

#if	NPC586 > 0
	{ pc586name,	pc586open,	nulldev,	nulldev,
	  pc586output,	pc586getstat,	pc586setstat,	nomap,
	  pc586setinput,nulldev,	nulldev,	0,
	  nodev },
#endif

#if	NAT3C501 > 0
	{ at3c501name,	at3c501open,	nulldev,	nulldev,
	  at3c501output,at3c501getstat,	at3c501setstat,	nomap,
	  at3c501setinput, nulldev,	nulldev,	0,
	  nodev },
#endif

#if	NNS8390 > 0
	{ ns8390wdname,	ns8390open,	nulldev,	nulldev,
	  ns8390output, ns8390getstat,	ns8390setstat,	nomap,
	  ns8390setinput, nulldev,	nulldev,	0,
	  nodev },

	{ ns8390elname,	ns8390open,	nulldev,	nulldev,
	  ns8390output, ns8390getstat,	ns8390setstat,	nomap,
	  ns8390setinput, nulldev,	nulldev,	0,
	  nodev },
#endif

#if	NUN > 0
	{ unname,	unopen,	nulldev,	nulldev,
	  unoutput,	ungetstat,	unsetstat,	nomap,
	  unsetinput,	nulldev,	nulldev,	0,
	  nodev },
#endif

#if	NTR > 0
	{ trname,	tropen,	nulldev,	nulldev,
	  troutput,	trgetstat,	trsetstat,	nomap,
	  trsetinput,	nulldev,	nulldev,	0,
	  nodev },
#endif

#if	NEN > 0
	{ enname,	enopen,	nulldev,	nulldev,
	  enoutput,	engetstat,	ensetstat,	nomap,
	  ensetinput,	nulldev,	nulldev,	0,
	  nodev },
#endif

#if	NBLIT > 0
	{ blitname,	blitopen,	blitclose,	nodev,
	  nodev,	blit_get_stat,	nodev,		blitmmap,
	  nodev,	nodev,		nodev,	0,
	  nodev },
#endif

#if	NMOUSE > 0
	{ mousename,	mouseopen,	mouseclose,	mouseread,
	  nodev,	mousegetstat,	mousesetstat,	nomap,
	  nodev,	nulldev,	nulldev,	0,
	  nodev },
#endif

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
