/* 
 * Mach Operating System
 * Copyright (c) 1992 Carnegie Mellon University
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
 * Revision 2.5  93/05/17  16:39:13  rvb
 * 	Use nomap to indicate no map entry -- it has the right type.
 * 	[93/05/17            rvb]
 * 
 * Revision 2.4  93/03/26  17:55:49  mrt
 * 	Added audio driver.
 * 	[93/03/17            af]
 * 
 * Revision 2.3  93/03/09  10:49:51  danner
 * 	zero d_mmap entries for non-mappable devices.
 * 	[93/03/07  15:22:06  af]
 * 
 * 	Added CD_ROM audio. Lint.
 * 	[93/03/05            af]
 * 
 * Revision 2.2  93/02/05  07:57:36  danner
 * 	Changes for Flamingo
 * 	[93/01/12            jeffreyh]
 * 	Created.
 * 	[92/12/10  14:54:50  af]
 * 
 */
/*
 *	File: conf.c
 * 	Author: Alessandro Forin, Carnegie Mellon University
 *	Date:	6/92
 *
 *	Device switch for ALPHA.
 *
 */

#include <mach/std_types.h>
#include <device/device_types.h>
#include <device/conf.h>

extern vm_offset_t block_io_mmap();

/* Names, must be unique */
#define	cons_name	"sl"
#define timename	"time"

#define rzname		"rz"	/* SCSI disk */
#define tzname		"tz"	/* tape */
#define	scname		"sc"	/* processors */
#define shname		"sh"	/* same, different protocol */
#define cdname		"cd_audio"	/* CD-ROM DA */
#define adu_se_name	"ase"	/* ether on ADU */
#define se_name		"se"	/* lance ether */
#define SE_name		"SE"	/* same, mapped */
#define auname		"audio"	/* ISDN audio channel */


/* SCSI (always here since driver is dynamic) */
io_return_t     rz_open(), rz_close(), rz_read(), rz_write();
io_return_t     rz_get_status(), rz_set_status(), rz_devinfo();

io_return_t	cd_open(), cd_close(), cd_read(), cd_write();

/* Ethernet */

#include <ase.h>
#if	NASE > 0

io_return_t	adu_se_open(), adu_se_output(), adu_se_get_status(), adu_se_set_status(),
	adu_se_setinput(), adu_se_restart();
#endif
#include <ln.h>
#if	NLN > 0
io_return_t	se_open(), se_output(), se_get_status(), se_set_status(),
	se_setinput(), se_restart();
#if notyet
io_return_t	SE_open(), SE_close(), SE_portdeath(),
	SE_get_status(), SE_set_status();
vm_offset_t SE_mmap();
#endif
#endif /*NLN*/

/* Time device (mappable device) */

extern io_return_t timeopen(),timeclose();
extern vm_offset_t timemmap();

/* Console */

#include <bm.h>
io_return_t	cons_open(),cons_close(),cons_read(),cons_write(),
	cons_get_status(), cons_set_status(), cons_portdeath();
#if	NBM > 0
extern vm_offset_t	screen_mmap();
#else
#define	screen_mmap	0
#endif

/*
 * Audio chips
 */
#include <audio.h>
#if	(NAUDIO > 0)
io_return_t	audio_open(), audio_close(), audio_read(), audio_write();
io_return_t	audio_set_status(), audio_get_status();
#endif


/*
 * List of devices - console must be at slot 0
 */
struct dev_ops	dev_name_list[] =
{
	/*name,		open,		close,		read,
	  write,	getstat,	setstat,	mmap,
	  async_in,	reset,		port_death,	subdev,
	  dev_info */

	{ cons_name,	cons_open,	cons_close,	cons_read,
	  cons_write,	cons_get_status,cons_set_status,screen_mmap,
	  nodev,	nulldev,	cons_portdeath, 0,
	  nodev },

	{ timename,	timeopen,	timeclose,	nulldev,
	  nulldev,	nulldev,	nulldev,	timemmap,
	  nodev,	nulldev,	nulldev,	0,
	  nodev },
#if	NASE > 0
	{ adu_se_name,	   adu_se_open,		nulldev,	nodev,
	  adu_se_output,   adu_se_get_status,	adu_se_set_status, nomap,
	  adu_se_setinput, adu_se_restart,	nulldev,	0,
	  nodev },
#endif
#if	NLN > 0
#if notyet
	{ SE_name,	SE_open,	SE_close,	nulldev,
	  nulldev,	SE_get_status,	SE_set_status,	SE_mmap,
	  nodev,	nulldev,	SE_portdeath,	0,
	  nodev },
#endif
	{ se_name,	se_open,	nulldev,	nodev,
	  se_output,	se_get_status,	se_set_status,	nomap,
	  se_setinput,	se_restart,	nulldev,	0,
	  nodev },
#endif /*NLN*/
	{ rzname,	rz_open,	rz_close,	rz_read,
	  rz_write,	rz_get_status,	rz_set_status,	nomap,
	  nodev,	nulldev,	nulldev,	8,
	  rz_devinfo },

	{ tzname,	rz_open,	rz_close,	rz_read,
	  rz_write,	rz_get_status,	rz_set_status,	nomap,
	  nodev,	nulldev,	nulldev,	8,
	  nodev },

	{ scname,	rz_open,	rz_close,	rz_read,
	  rz_write,	rz_get_status,	rz_set_status,	nomap,
	  nodev,	nulldev,	nulldev,	8,
	  nodev },

	{ shname,	rz_open,	rz_close,	rz_read,
	  rz_write,	rz_get_status,	rz_set_status,	nomap,
	  nodev,	nulldev,	nulldev,	8,
	  nodev },

	{ cdname,	cd_open,	cd_close,	cd_read,
	  cd_write,	nodev,		nodev,		nomap,
	  nodev,	nulldev,	nulldev,	8,
	  nodev },

#if (NAUDIO>0)

	{ auname,	audio_open,	audio_close,	audio_read,
	  audio_write,	audio_get_status, audio_set_status,	nomap,
	  nodev,	nulldev,	nulldev,	0,
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
				/sizeof(dev_indirect_list[0]);
