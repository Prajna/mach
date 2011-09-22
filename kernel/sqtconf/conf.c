/* 
 * Mach Operating System
 * Copyright (c) 1993 Carnegie Mellon University
 * Copyright (c) 1991 Carnegie Mellon University
 * Copyright (c) 1991 Sequent Computer Systems
 * All Rights Reserved.
 * 
 * Permission to use, copy, modify and distribute this software and its
 * documentation is hereby granted, provided that both the copyright
 * notice and this permission notice appear in all copies of the
 * software, derivative works or modified versions, and any portions
 * thereof, and that both notices appear in supporting documentation.
 * 
 * CARNEGIE MELLON AND SEQUENT COMPUTER SYSTEMS ALLOW FREE USE OF
 * THIS SOFTWARE IN ITS "AS IS" CONDITION.  CARNEGIE MELLON AND
 * SEQUENT COMPUTER SYSTEMS DISCLAIM ANY LIABILITY OF ANY KIND FOR
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
 * Revision 2.8  93/05/18  11:12:13  rvb
 * 	Revision 2.8  93/05/17  16:33:27  rvb
 * 	Type casts, etc to quiet gcc 2.3.3 warnings
 * 	[93/05/18            rvb]
 * 
 * Revision 2.7  93/02/05  08:18:37  danner
 * 	Fixed include.
 * 	[93/02/04            danner]
 * 
 * Revision 2.6  92/08/03  18:21:19  jfriedl
 * 	Add "zd_devinfo" to "zd" disk driver description record.
 * 	[92/08/03  14:34:21  jms]
 * 
 * Revision 2.5  92/02/23  22:45:11  elf
 * 	Added getstat routine for sd and zd drivers.
 * 	[92/02/22  19:54:46  af]
 * 
 * Revision 2.4  91/08/28  11:16:51  jsb
 * 	Added entries for new dev_info field.
 * 	[91/08/27  17:56:16  jsb]
 * 
 * Revision 2.3  91/07/31  18:05:01  dbg
 * 	Changed copyright.
 * 	[91/07/31            dbg]
 * 
 * Revision 2.2  91/05/08  13:01:48  dbg
 * 	Created.
 * 	[90/10/04            dbg]
 * 
 */

/*
 * Device table for Sequent.
 */

#include <mach/machine/vm_types.h>
#include <device/conf.h>

extern vm_offset_t block_io_mmap();

extern int	timeopen(),timeclose();
extern vm_offset_t timemmap();
#define timename	"time"

extern int	coopen(),coclose(),coread(),cowrite();
extern int	cogetstat(),cosetstat(), coportdeath();
#define	coname		"co"

#include <sd.h>
#if	NSD > 0
extern int	sdopen(), sdread(), sdwrite(), sdgetstat();
#define	sdname		"sd"
#endif

#include <zd.h>
#if	NZD > 0
extern int	zdopen(), zdclose(), zdread(), zdwrite(), zdgetstat(), zd_devinfo();
#define	zdname		"zd"
#endif

#include <se.h>
#if	NSE > 0
extern int	se_open(), se_output(), se_getstat(), se_setstat();
extern int	se_setinput();
#define	sename		"se"
#endif

/*
 * List of devices.  Console must be at slot 0.
 */
struct dev_ops	dev_name_list[] =
{
	/*name,		open,		close,		read,
	  write,	getstat,	setstat,	mmap,
	  async_in,	reset,		port_death,	subdev,
	  dev_info */

	{ coname,	coopen,		coclose,	coread,
	  cowrite,	cogetstat,	cosetstat,	nomap,
	  nodev,	nulldev,	coportdeath,	0,
	  nodev },

	{ timename,	timeopen,	timeclose,	nulldev,
	  nulldev,	nulldev,	nulldev,	timemmap,
	  nodev,	nulldev,	nulldev,	0,
	  nodev },

#if	NSD > 0
	{ sdname,	sdopen,		nulldev,	sdread,
	  sdwrite,	sdgetstat,	nulldev,	block_io_mmap,
	  nulldev,	nulldev,	nulldev,	8,
	  nodev },
#endif

#if	NZD > 0
	{ zdname,	zdopen,		zdclose,	zdread,
	  zdwrite,	zdgetstat,	nulldev,	block_io_mmap,
	  nodev,	nulldev,	nulldev,	8,
	  zd_devinfo },
#endif

#if	NSE > 0
	{ sename,	se_open,	nulldev,	nulldev,
	  se_output,	se_getstat,	se_setstat,	nomap,
	  se_setinput,	nulldev,	nulldev,	0,
	  nodev },
#endif

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
