/* 
 * Mach Operating System
 * Copyright (c) 1991,1990,1989,1988 Carnegie Mellon University
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
 * Revision 2.21  93/05/17  16:34:33  rvb
 * 	Use nomap where required vs nodev or "0"
 * 	[93/05/17            rvb]
 * 
 * Revision 2.20  93/05/10  21:20:59  rvb
 * 	Zero map function where unused.
 * 	Use block_io for disks.
 * 	[93/05/06  09:32:51  af]
 * 
 * Revision 2.19  93/03/26  17:57:48  mrt
 * 	Added audio driver, fixed types.
 * 	[93/03/17            af]
 * 
 * Revision 2.18  93/03/09  10:56:00  danner
 * 	Added CD-ROM audio.
 * 	[93/03/06            af]
 * 
 * Revision 2.17  93/02/04  14:27:09  mrt
 * 	added include of vm_types to make conf.h work.
 * 	[93/02/04            mrt]
 * 
 * Revision 2.16  93/01/21  12:25:15  danner
 * 	ecc --> atm
 * 	[93/01/19  16:36:03  bershad]
 * 
 * Revision 2.15  92/04/03  12:09:51  rpd
 * 	Add FORE ATM support.
 * 	[92/03/23            rvb]
 * 
 * Revision 2.14  92/04/01  15:15:14  rpd
 * 	Added mapped timer device
 * 	[92/03/11  02:34:30  af]
 * 
 * Revision 2.13  92/03/02  18:34:54  rpd
 * 	Added MAXine's floppy.
 * 	[92/03/02  02:12:41  af]
 * 
 * Revision 2.12  92/01/03  20:41:14  dbg
 * 	Added devinfo routine to scsi to accomodate MI change
 * 	that screwed up extra large writes.
 * 	[91/12/26  11:01:14  af]
 * 
 * Revision 2.11  91/08/28  11:15:46  jsb
 * 	Added entries for new dev_info field.
 * 	[91/08/27  17:59:55  jsb]
 * 
 * Revision 2.10  91/08/24  12:22:24  af
 * 	Generic console driver, SCSI processor devices.
 * 	[91/08/02  03:07:13  af]
 * 
 * Revision 2.9  91/05/14  17:32:51  mrt
 * 	Correcting copyright
 * 
 * Revision 2.8  91/02/05  17:47:29  mrt
 * 	Added author notices
 * 	[91/02/04  11:21:26  mrt]
 * 
 * 	Changed to use new Mach copyright
 * 	[91/02/02  12:24:58  mrt]
 * 
 * Revision 2.7  90/12/05  23:37:09  af
 * 	Mods for new, copyright free PMAX drivers.
 * 	Added mapped SCSI.  
 * 	SCSI tape "tz" just has a different name now
 * 	(for backward compat), but it really is the
 * 	same as the disk "rz" one.
 * 	[90/12/03  23:02:10  af]
 * 
 * Revision 2.5  90/09/10  15:00:34  rpd
 * 	Made se_name be "se" and SE_name be "SE".
 * 	[90/09/10            rpd]
 * 
 * Revision 2.4  90/09/09  23:20:57  rpd
 * 	Added mapped ether device, kept the old one for now.
 * 	[90/08/30            af]
 * 
 * Revision 2.3  89/12/08  19:47:38  rwd
 * 	Bitmap screen is mapped via the mouse, with the console's
 * 	major. Yeech.
 * 	[89/12/05  02:22:14  af]
 * 
 * Revision 2.2  89/11/29  14:12:48  af
 * 	Got ether to work, with some name changes.
 * 	[89/11/26  10:31:55  af]
 * 
 * 	Created.
 * 	[89/10/05            af]
 * 
 */
/*
 *	File: conf.c
 * 	Author: Alessandro Forin, Carnegie Mellon University
 *	Date:	10/89
 *
 *	Device switch for MIPS.
 *
 */

#include <mach/std_types.h>
#include <device/device_types.h>
#include <device/conf.h>

extern vm_offset_t block_io_mmap();

#include <ts.h>
#if NTS > 0
io_return_t	tsopen(),tsclose(),/*tsstrategy()*/,tsread(),tswrite();
io_return_t	tsioctl(),tssetstat(),/*tsdump()*/,tsreset();
#define tsname "ts"
#endif


#include <tthu.h>
#if NTTHU > 0
io_return_t	tthopen(),tthclose(),/*tthstrategy(),*/tthread(),tthwrite();
io_return_t	tthioctl(), tthsetstat();
#define tthname "tt"
#endif

#include <rd.h>
#if NRD > 0
io_return_t	rdopen(),/*rdstrategy(),*/rdread(),rdwrite(),
	/*rddump(),*/rdioctl(),/*rdsize(),*/rdsetstat();
#define rdname "rd"
#endif

#include <dkip.h>
#if NDKIP > 0
io_return_t	dkipopen()/*,dkipstrategy()*/,dkipread(),dkipwrite()/*,dkipdump()*/;
io_return_t	dkipioctl()/*,dkipsize()*/;
#define dkname "ip"
#endif

/* SCSI (always here since driver is dynamic) */
io_return_t     rz_open(), rz_close(), rz_read(), rz_write();
io_return_t     rz_get_status(), rz_set_status(), rz_devinfo();
#define rzname "rz"

/* Mapped SCSI (ditto) */
io_return_t     RZ_open(), RZ_close(), RZ_portdeath(),
		RZ_set_status(), RZ_get_status();
vm_offset_t	RZ_mmap();
#define RZname "RZ"

/* SCSI CD-ROM-DA */
io_return_t	cd_open(), cd_close(), cd_read(), cd_write();

#define tzname "tz"	/* tape */
#define	scname "sc"	/* processors */
#define shname "sh"	/* same, different protocol */
#define cdname		"cd_audio"	/* CD-ROM DA */

/* Floppy */
#include <fd.h>
#if NFD > 0
io_return_t	fd_open(),fd_close(),fd_read(),fd_write(),
	fd_get_status(),fd_set_status();
#define	fdname "fd"
#endif

/* Audio */
#include <audio.h>
#if NAUDIO > 0
#define auname	"audio"
io_return_t	audio_open(), audio_close(), audio_read(), audio_write();
io_return_t	audio_set_status(), audio_get_status();
#endif

/* Time device (mappable device) */

extern io_return_t timeopen(),timeclose();
vm_offset_t	timemmap();
#define timename	"time"

/* High resolution counter */
#include <frc.h>
#if NFRC > 0
extern io_return_t frc_openclose();
vm_offset_t frc_mmap();
#define frcname "frc"
#endif

/* Char devices */

io_return_t	cons_open(),cons_close(),cons_read(),cons_write(),
		cons_get_status(), cons_set_status(), cons_portdeath();
vm_offset_t	screen_mmap();
#define	cons_name		"sl"

#include <cp.h>
#if NCP > 0
io_return_t	cpopen(),cpclose(),cpread(),cpwrite(),cpioctl(),
	/*cpstop(),*/ cpsetstat(), cpreset(), cpportdeath();
/*extern struct	tty cp_tty[];*/
#define cpname "cp"
#endif

io_return_t	se_open(), se_output(), se_get_status(), se_set_status(),
	se_setinput(), se_restart();
#define se_name "SE"

io_return_t	SE_open(), SE_close(), SE_portdeath(),
		SE_get_status(), SE_set_status();
vm_offset_t	SE_mmap();
#define SE_name "se"

#include <atm.h>
#if NATM > 0
io_return_t	atm_open(), atm_close(), atm_read(), atm_write(),
		atm_get_status(), atm_set_status(),
		atm_setinput(), atm_restart(), atm_portdeath();
vm_offset_t	 atm_mmap();
#define	atm_name	"atm"
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

	{ frcname,	frc_openclose,	frc_openclose,	nulldev,
	  nulldev,	nulldev,	nulldev,	frc_mmap,
	  nodev,	nulldev,	nulldev,	0,
	  nodev },

#if	NTS > 0
	{ tsname,	tsopen,		tsclose,	tsread,
	  tswrite,	tsioctl,	tssetstat,	nomap,
	  nodev,	tsreset,	nulldev,	1,
	  nodev },
#endif

#if	NTTHU > 0
	{ tthname,	tthopen,	tthclose,	tthread,
	  tthwrite,	tthioctl,	tthsetstat,	nomap,
	  nodev,	nulldev,	nulldev,	8,
	  nodev },
#endif

#if	NRD > 0
	{ rdname,	rdopen,		nulldev,	rdread,
	  rdwrite,	rdioctl,	rdsetstat,	nomap,
	  nodev,	nulldev,	nulldev,	0,
	  nodev },
#endif

#if	NDKIP > 0
	{ dkipname,	dkipopen,	nulldev,	dkipread,
	  dkipwrite,	dkipioctl,	nulldev,	block_io_mmap,
	  nodev,	nulldev,	nulldev,	8,
	  nodev },
#endif

	{ rzname,	rz_open,	rz_close,	rz_read,
	  rz_write,	rz_get_status,	rz_set_status,	block_io_mmap,
	  nodev,	nulldev,	nulldev,	8,
	  rz_devinfo },

	{ RZname,	RZ_open,	RZ_close,	nulldev,
	  nulldev,	RZ_get_status,	RZ_set_status,	RZ_mmap,
	  nodev,	nulldev,	RZ_portdeath,	0,
	  nodev },

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

#if	NAUDIO > 0

	{ auname,	audio_open,	audio_close,	audio_read,
	  audio_write,	audio_get_status, audio_set_status,	nomap,
	  nodev,	nulldev,	nulldev,	0,
	  nodev },

#endif

#if	NCP > 0
	{ cpname,	cpopen,		cpclose,	cpread,
	  cpwrite,	cpioctl,	cpsetstat,	nomap,
	  nodev,	cpreset,	cpportdeath,	16,
	  nodev },
#endif

	{ SE_name,	SE_open,	SE_close,	nulldev,
	  nulldev,	SE_get_status,	SE_set_status,	SE_mmap,
	  nodev,	nulldev,	SE_portdeath,	0,
	  nodev },

	{ se_name,	se_open,	nulldev,	nodev,
	  se_output,	se_get_status,	se_set_status,	nomap,
	  se_setinput,	se_restart,	nulldev,	0,
	  nodev },

#if	NFD > 0

	{ fdname,	fd_open,	fd_close,	fd_read,
	  fd_write,	fd_get_status,	fd_set_status,	block_io_mmap,
	  nodev,	nulldev,	nulldev,	8,
	  rz_devinfo },

#endif

#if	NATM > 0
	{ atm_name,	atm_open,	atm_close,	atm_read,
	  atm_write,	atm_get_status,	atm_set_status,	atm_mmap,
	  atm_setinput,	atm_restart,	atm_portdeath,	0,
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
