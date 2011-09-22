/* 
 * Mach Operating System
 * Copyright (c) 1993,1992,1991,1990,1989 Carnegie Mellon University
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
 * $Log:	autoconf.c,v $
 * Revision 2.21  93/11/17  16:41:08  dbg
 * 	Added 174x driver (dherity@cs.tcd.ie & ralf@chpc.org)
 * 	[93/10/06            af]
 * 	Lint.
 * 	[93/05/08  11:20:03  af]
 * 
 * 	Made it possible to have two Adaptec boards.
 * 	[93/05/07  18:37:34  af]
 * 
 * 	Added include of standard types.
 * 	[93/05/06  09:30:11  af]
 * 
 * Revision 2.19  93/08/10  15:57:38  mrt
 * 	Iff rcline != -1, then proberc will turn on the com line first
 * 	thing.  So the rest of the probe will be reported thru rconsole.
 * 	[93/06/08            rvb]
 * 
 * Revision 2.18  93/05/10  23:24:01  rvb
 * 	Added TTD teledebug code used to initialize the teledebugger and
 * 	begin setup debugging.
 * 	[93/03/01            grm]
 * 
 * Revision 2.17  93/05/10  21:18:41  rvb
 * 	Lint.
 * 	[93/05/08  11:20:03  af]
 * 
 * 	Made it possible to have two Adaptec boards.
 * 	[93/05/07  18:37:34  af]
 * 
 * 	Added include of standard types.
 * 	[93/05/06  09:30:11  af]
 * 
 * Revision 2.16  92/04/03  12:08:35  rpd
 * 	Use port 0x250, 0x350 for irq 5 bnc, thick respectively.
 * 	[92/04/02            rvb]
 * 	Tossing (0x3bc) below is the WRONG thing to do!!
 * 	[92/03/10            rvb]
 * 
 * Revision 2.15  92/03/06  13:38:29  rpd
 * 	Removed 0x3bc entry for lprdriver until someone can come up
 * 	with a better fix.
 * 	[92/03/06  13:16:43  jsb]
 * 
 * Revision 2.14  92/02/19  16:29:36  elf
 * 	Re-oder device probes -- disks first.
 * 	[92/02/18            rvb]
 * 
 * Revision 2.13  92/02/19  15:08:08  elf
 * 	Made take_dev_irq() print device name instead of XYZ in case of
 * 	failure.
 * 	[92/01/20            kivinen]
 * 
 * 	Added more possible configurations for ns8390driver to be able to
 * 	use two wd cards simultaneously.
 * 	[92/01/20            kivinen]
 * 
 * Revision 2.12  92/01/14  16:43:46  rpd
 * 	Flush com3 it is hardly every available and would use irq 6 -- a no-no.
 * 	[92/01/08            rvb]
 * 
 * Revision 2.11  91/10/07  17:25:07  af
 * 	Allow for second fd controller and fix hd entry.
 * 	[91/10/01  15:26:32  rvb]
 * 
 * 	Synchronize with 2.5 tables.
 * 	[91/09/04            rvb]
 * 
 * Revision 2.10  91/08/24  11:57:16  af
 * 	Use new MI autoconf procedures. Added SCSI.
 * 	[91/08/02  02:52:15  af]
 * 
 * Revision 2.9  91/06/19  11:55:44  rvb
 * 	cputypes.h->platforms.h
 * 	[91/06/12  13:45:33  rvb]
 * 
 * Revision 2.8  91/05/14  16:19:38  mrt
 * 	Correcting copyright
 * 
 * Revision 2.7  91/02/05  17:15:58  mrt
 * 	Changed to new Mach copyright
 * 	[91/02/01  17:41:36  mrt]
 * 
 * Revision 2.6  91/01/09  19:56:29  rpd
 * 	Converted from wd8003 to ns8390.
 * 	[91/01/08            rpd]
 * 
 * Revision 2.4  90/11/26  14:48:56  rvb
 * 	jsb beat me to XMK34, sigh ...
 * 	[90/11/26            rvb]
 * 	Synched 2.5 & 3.0 at I386q (r1.7.1.8) & XMK35 (r2.4)
 * 	[90/11/15            rvb]
 * 
 * Revision 2.3  90/10/01  14:22:53  jeffreyh
 * 	added wd ethernet driver
 * 	[90/09/27  18:23:07  jeffreyh]
 * 
 * Revision 1.7.1.7  90/08/25  15:42:34  rvb
 * 	Define take_<>_irq() vs direct manipulations of ivect and friends.
 * 	Replace single Ctlr[] with one per Ctlr type.
 * 	[90/08/20            rvb]
 * 
 * 	Added parallel port printer driver.
 * 	[90/08/14            mg32]
 * 
 * Revision 1.7.1.6  90/07/27  11:22:57  rvb
 * 	Move wd8003 IRQ to IRQ9.
 * 	[90/07/26            rvb]
 * 
 * Revision 1.7.1.5  90/07/10  11:41:45  rvb
 * 	iPSC2: subtype and call dcminit().
 * 	[90/06/16            rvb]
 * 
 * 	Rework probe and attach to be much more bsd flavored.
 * 	[90/06/07            rvb]
 * 
 * Revision 1.7.1.4  90/06/07  08:04:46  rvb
 * 	updated for new hd driver probe/attach	[eugene]
 * 	(Temporarily disabled.)
 * 
 * Revision 1.7.1.3  90/05/14  13:17:45  rvb
 * 	Copy in slave_config() from loose_ends.c
 * 	[90/04/23            rvb]
 * 
 * Revision 2.2  90/05/03  15:40:37  dbg
 * 	Converted for pure kernel.
 * 	[90/04/23            dbg]
 * 
 * Revision 1.7.1.2  90/03/16  18:14:51  rvb
 * 	Add 3com 3c501 ether [bernadat]
 * 	Also clean up things, at least for ether; there are NO
 * 	controllers -- just devices.
 * 
 * Revision 1.7.1.1  89/10/22  11:29:41  rvb
 * 	Call setconf for generic kernels.
 * 	[89/10/17            rvb]
 * 
 * Revision 1.7  89/09/20  17:26:26  rvb
 * 	Support ln for ORC
 * 	[89/09/20            rvb]
 * 
 * Revision 1.6  89/09/09  15:19:19  rvb
 * 	pc586, qd and com are now configured based on the appropriate .h
 * 	files and pccom -> com.
 * 	[89/09/09            rvb]
 * 
 * Revision 1.5  89/07/17  10:34:58  rvb
 * 	Olivetti Changes to X79 upto 5/9/89:
 * 		An almost legitimate probe routine().
 * 	[89/07/11            rvb]
 * 
 * Revision 1.4  89/02/26  12:25:02  gm0w
 * 	Changes for cleanup.
 * 
 */
 
#ifdef	MACH_KERNEL
#include <mach_ttd.h>

#include <mach/std_types.h>
#else	/* MACH_KERNEL */
#include <cpus.h>
#include <platforms.h>
#include <generic.h>
#include <sys/param.h>
#include <mach/machine.h>
#include <machine/cpu.h>
#endif	/* MACH_KERNEL */
#include <i386/ipl.h>
#include <chips/busses.h>

/* initialization typecasts */
#define	SPL_FIVE	(vm_offset_t)SPL5
#define	SPL_SIX		(vm_offset_t)SPL6
#define	SPL_TTY		(vm_offset_t)SPLTTY


#include <hd.h>
#if NHD > 0
extern	struct	bus_driver	hddriver;
extern int			hdintr();
#endif /* NHD */

#include <fd.h>
#if NFD > 0
extern	struct	bus_driver	fddriver;
extern int			fdintr();
#endif /* NFD */

#include <aha.h>
#if NAHA > 0
extern struct	bus_driver	aha_driver;
extern int			aha_intr();
#endif /* NAHA */

#include <eaha.h>
#if NEAHA > 0
extern struct	bus_driver	eaha_driver;
extern int	eaha_intr();
#endif /* NEAHA */

#include <pc586.h>
#if NPC586 > 0
extern	struct	bus_driver	pcdriver;
extern int			pc586intr();
#endif /* NPC586 */

#include <ns8390.h>
#if NNS8390 > 0
extern	struct	bus_driver	ns8390driver;
extern int			ns8390intr();
#endif /* NNS8390 */

#include <at3c501.h>
#if NAT3C501 > 0
extern	struct	bus_driver	at3c501driver;
extern int			at3c501intr();
#endif /* NAT3C501 */

#include <com.h>
#if NCOM > 0
extern	struct	bus_driver	comdriver;
extern int			comintr();
#endif /* NCOM */

#include <lpr.h>
#if NLPR > 0
extern	struct	bus_driver	lprdriver;
extern int			lprintr();
#endif /* NLPR */

#include <wt.h>
#if NWT > 0
extern	struct	bus_driver	wtdriver;
extern int			wtintr();
#endif /* NWT */

struct	bus_ctlr	bus_master_init[] = {

/* driver    name unit intr    address        len phys_address   
     adaptor alive flags spl    pic				 */

#if NHD > 0
  {&hddriver, "hdc",  0, hdintr, 0x1f0, 8, 0x1f0,
     '?',	0,  0,	 SPL_FIVE, 14},

  {&hddriver, "hdc",  1, hdintr, 0x170, 8, 0x170,
     '?',	0,  0,	 SPL_FIVE, 15},
#endif	/* NHD > 0 */

#if NAHA > 0
  {&aha_driver, "ahac",  0, aha_intr, 0x330, 4, 0x330,
     '?',	0,  0,	 SPL_FIVE, 11},

#if NAHA > 1

  {&aha_driver, "ahac",  1, aha_intr, 0x234, 4, 0x234,
     '?',	0,  0,	 SPL_FIVE, 12},
  {&aha_driver, "ahac",  1, aha_intr, 0x230, 4, 0x230,
     '?',	0,  0,	 SPL_FIVE, 12},
  {&aha_driver, "ahac",  1, aha_intr, 0x134, 4, 0x134,
     '?',	0,  0,	 SPL_FIVE, 12},
  {&aha_driver, "ahac",  1, aha_intr, 0x130, 4, 0x130,
     '?',	0,  0,	 SPL_FIVE, 12},

#else

  {&aha_driver, "ahac",  0, aha_intr, 0x334, 4, 0x334,
     '?',	0,  0,	 SPL_FIVE, 11},
  {&aha_driver, "ahac",  0, aha_intr, 0x234, 4, 0x234,
     '?',	0,  0,	 SPL_FIVE, 11},
  {&aha_driver, "ahac",  0, aha_intr, 0x230, 4, 0x230,
     '?',	0,  0,	 SPL_FIVE, 11},
  {&aha_driver, "ahac",  0, aha_intr, 0x134, 4, 0x134,
     '?',	0,  0,	 SPL_FIVE, 11},
  {&aha_driver, "ahac",  0, aha_intr, 0x130, 4, 0x130,
     '?',	0,  0,	 SPL_FIVE, 11},

#endif	/* NAHA > 1 */
#endif	/* NAHA > 0*/

#if NEAHA > 0
{&eaha_driver, "eahac", 0, eaha_intr, 0x0000, 4, 0x0000,
     '?',	0,  0,	 SPL_FIVE, 12},
{&eaha_driver, "eahac", 0, eaha_intr, 0x1000, 4, 0x1000,
     '?',	0,  0,	 SPL_FIVE, 12},
{&eaha_driver, "eahac", 0, eaha_intr, 0x2000, 4, 0x2000,
     '?',	0,  0,	 SPL_FIVE, 12},
{&eaha_driver, "eahac", 0, eaha_intr, 0x3000, 4, 0x3000,
     '?',	0,  0,	 SPL_FIVE, 12},
{&eaha_driver, "eahac", 0, eaha_intr, 0x4000, 4, 0x4000,
     '?',	0,  0,	 SPL_FIVE, 12},
{&eaha_driver, "eahac", 0, eaha_intr, 0x5000, 4, 0x5000,
     '?',	0,  0,	 SPL_FIVE, 12},
{&eaha_driver, "eahac", 0, eaha_intr, 0x6000, 4, 0x6000,
     '?',	0,  0,	 SPL_FIVE, 12},
{&eaha_driver, "eahac", 0, eaha_intr, 0x7000, 4, 0x7000,
     '?',	0,  0,	 SPL_FIVE, 12},
{&eaha_driver, "eahac", 0, eaha_intr, 0x8000, 4, 0x8000,
     '?',	0,  0,	 SPL_FIVE, 12},
{&eaha_driver, "eahac", 0, eaha_intr, 0x9000, 4, 0x9000,
     '?',	0,  0,	 SPL_FIVE, 12},
{&eaha_driver, "eahac", 0, eaha_intr, 0xa000, 4, 0xa000,
     '?',	0,  0,	 SPL_FIVE, 12},
{&eaha_driver, "eahac", 0, eaha_intr, 0xb000, 4, 0xb000,
     '?',	0,  0,	 SPL_FIVE, 12},
{&eaha_driver, "eahac", 0, eaha_intr, 0xc000, 4, 0xc000,
     '?',	0,  0,	 SPL_FIVE, 12},
{&eaha_driver, "eahac", 0, eaha_intr, 0xd000, 4, 0xd000,
     '?',	0,  0,	 SPL_FIVE, 12},
{&eaha_driver, "eahac", 0, eaha_intr, 0xe000, 4, 0xe000,
     '?',	0,  0,	 SPL_FIVE, 12},
{&eaha_driver, "eahac", 0, eaha_intr, 0xf000, 4, 0xf000,
     '?',	0,  0,	 SPL_FIVE, 12},
#endif /* NEAHA > 0 */

#if NFD > 0
  {&fddriver, "fdc",  0, fdintr, 0x3f2, 6, 0x3f2,
     '?',	0,  0,	 SPL_FIVE, 6},

  {&fddriver, "fdc",  1, fdintr, 0x372, 6, 0x372,
     '?',	0,  0,	 SPL_FIVE, 10},
#endif	/* NFD > 0 */

  0
};


struct	bus_device	bus_device_init[] = {

/* driver     name unit intr    address       am   phys_address 
     adaptor alive ctlr slave flags *mi       *next  sysdep sysdep */

#if NHD > 0
  {&hddriver, "hd", 0, hdintr, 0x104, 8, 0x1f0,
     '?',    0,   0,    0,    0,   0,  0,  SPL_FIVE, 14},
  {&hddriver, "hd", 1, hdintr, 0x118, 8, 0x1f0,
     '?',    0,   0,    1,    0,   0,  0,  SPL_FIVE, 14},
  {&hddriver, "hd", 2, hdintr, 0x104, 8, 0x170,  /*??*/
     '?',    0,   1,    0,    0,   0,  0,  SPL_FIVE, 15},
  {&hddriver, "hd", 3, hdintr, 0x118, 8, 0x170,
     '?',    0,   1,    1,    0,   0,  0,  SPL_FIVE, 15},
#endif /* NHD > 0 */

#if NAHA > 0
{ &aha_driver,	"rz",   0,  0,  0x0,0,	0,    '?',     0,   0,   0,    0, },
{ &aha_driver,	"rz",   1,  0,  0x0,0,	0,    '?',     0,   0,   1,    0, },
{ &aha_driver,	"rz",   2,  0,  0x0,0,	0,    '?',     0,   0,   2,    0, },
{ &aha_driver,	"rz",   3,  0,  0x0,0,	0,    '?',     0,   0,   3,    0, },
{ &aha_driver,	"rz",   4,  0,  0x0,0,	0,    '?',     0,   0,   4,    0, },
{ &aha_driver,	"rz",   5,  0,  0x0,0,	0,    '?',     0,   0,   5,    0, },
{ &aha_driver,	"rz",   6,  0,  0x0,0,	0,    '?',     0,   0,   6,    0, },
{ &aha_driver,	"rz",   7,  0,  0x0,0,	0,    '?',     0,   0,   7,    0, },

{ &aha_driver,	"tz",   0,  0,  0x0,0,	0,    '?',     0,   0,   0,    0, },
{ &aha_driver,	"tz",   1,  0,  0x0,0,	0,    '?',     0,   0,   1,    0, },
{ &aha_driver,	"tz",   2,  0,  0x0,0,	0,    '?',     0,   0,   2,    0, },
{ &aha_driver,	"tz",   3,  0,  0x0,0,	0,    '?',     0,   0,   3,    0, },
{ &aha_driver,	"tz",   4,  0,  0x0,0,	0,    '?',     0,   0,   4,    0, },
{ &aha_driver,	"tz",   5,  0,  0x0,0,	0,    '?',     0,   0,   5,    0, },
{ &aha_driver,	"tz",   6,  0,  0x0,0,	0,    '?',     0,   0,   6,    0, },
{ &aha_driver,	"tz",   7,  0,  0x0,0,	0,    '?',     0,   0,   7,    0, },

#if NAHA > 1

{ &aha_driver,	"rz",   8,  0,  0x0,0,	0,    '?',     0,   1,   0,    0, },
{ &aha_driver,	"rz",   9,  0,  0x0,0,	0,    '?',     0,   1,   1,    0, },
{ &aha_driver,	"rz",  10,  0,  0x0,0,	0,    '?',     0,   1,   2,    0, },
{ &aha_driver,	"rz",  11,  0,  0x0,0,	0,    '?',     0,   1,   3,    0, },
{ &aha_driver,	"rz",  12,  0,  0x0,0,	0,    '?',     0,   1,   4,    0, },
{ &aha_driver,	"rz",  13,  0,  0x0,0,	0,    '?',     0,   1,   5,    0, },
{ &aha_driver,	"rz",  14,  0,  0x0,0,	0,    '?',     0,   1,   6,    0, },
{ &aha_driver,	"rz",  15,  0,  0x0,0,	0,    '?',     0,   1,   7,    0, },

{ &aha_driver,	"tz",   8,  0,  0x0,0,	0,    '?',     0,   1,   0,    0, },
{ &aha_driver,	"tz",   9,  0,  0x0,0,	0,    '?',     0,   1,   1,    0, },
{ &aha_driver,	"tz",  10,  0,  0x0,0,	0,    '?',     0,   1,   2,    0, },
{ &aha_driver,	"tz",  11,  0,  0x0,0,	0,    '?',     0,   1,   3,    0, },
{ &aha_driver,	"tz",  12,  0,  0x0,0,	0,    '?',     0,   1,   4,    0, },
{ &aha_driver,	"tz",  13,  0,  0x0,0,	0,    '?',     0,   1,   5,    0, },
{ &aha_driver,	"tz",  14,  0,  0x0,0,	0,    '?',     0,   1,   6,    0, },
{ &aha_driver,	"tz",  15,  0,  0x0,0,	0,    '?',     0,   1,   7,    0, },
#endif	/* NAHA > 1 */
#endif	/* NAHA > 0 */

#if NEAHA > 0
{ &eaha_driver,	"rz",   0,  0,  0x0,0,	0,    '?',     0,   0,   0,    0, },
{ &eaha_driver,	"rz",   1,  0,  0x0,0,	0,    '?',     0,   0,   1,    0, },
{ &eaha_driver,	"rz",   2,  0,  0x0,0,	0,    '?',     0,   0,   2,    0, },
{ &eaha_driver,	"rz",   3,  0,  0x0,0,	0,    '?',     0,   0,   3,    0, },
{ &eaha_driver,	"rz",   4,  0,  0x0,0,	0,    '?',     0,   0,   4,    0, },
{ &eaha_driver,	"rz",   5,  0,  0x0,0,	0,    '?',     0,   0,   5,    0, },
{ &eaha_driver,	"rz",   6,  0,  0x0,0,	0,    '?',     0,   0,   6,    0, },
{ &eaha_driver,	"rz",   7,  0,  0x0,0,	0,    '?',     0,   0,   7,    0, },

{ &eaha_driver,	"tz",   0,  0,  0x0,0,	0,    '?',     0,   0,   0,    0, },
{ &eaha_driver,	"tz",   1,  0,  0x0,0,	0,    '?',     0,   0,   1,    0, },
{ &eaha_driver,	"tz",   2,  0,  0x0,0,	0,    '?',     0,   0,   2,    0, },
{ &eaha_driver,	"tz",   3,  0,  0x0,0,	0,    '?',     0,   0,   3,    0, },
{ &eaha_driver,	"tz",   4,  0,  0x0,0,	0,    '?',     0,   0,   4,    0, },
{ &eaha_driver,	"tz",   5,  0,  0x0,0,	0,    '?',     0,   0,   5,    0, },
{ &eaha_driver,	"tz",   6,  0,  0x0,0,	0,    '?',     0,   0,   6,    0, },
{ &eaha_driver,	"tz",   7,  0,  0x0,0,	0,    '?',     0,   0,   7,    0, },
#endif	/* NEAHA > 0*/

#if NFD > 0
  {&fddriver, "fd", 0, fdintr, 0x3f2, 6, 0x3f2,
     '?',    0,   0,    0,    0,   0,  0,   SPL_FIVE, 6},
  {&fddriver, "fd", 1, fdintr, 0x3f2, 6, 0x3f2,
     '?',    0,   0,    1,    0,   0,  0,   SPL_FIVE, 6},

  {&fddriver, "fd", 2, fdintr, 0x372, 6, 0x372,
     '?',    0,   1,    0,    0,   0,  0,   SPL_FIVE, 10},
  {&fddriver, "fd", 3, fdintr, 0x372, 6, 0x372,
     '?',    0,   1,    1,    0,   0,  0,   SPL_FIVE, 10},
#endif	/* NFD > 0 */

#if NPC586 > 0
  /* For MACH Default */
  {&pcdriver, "pc", 0, pc586intr, 0xd0000, 0, 0xd0000,
     '?',    0,   -1,    -1,    0,   0,        0,   SPL_FIVE, 9},
  /* For Factory Default */
  {&pcdriver, "pc", 0, pc586intr, 0xc0000, 0, 0xc0000,
     '?',    0,   -1,    -1,    0,   0,        0,   SPL_FIVE, 5},
  /* For what Intel Ships */
  {&pcdriver, "pc", 0, pc586intr, 0xf00000, 0, 0xf00000,
     '?',    0,   -1,    -1,    0,   0,        0,   SPL_FIVE, 12},
#endif /* NPC586 > 0 */

#if NNS8390 > 0
	/* "wd" and "el" */
  {&ns8390driver, "wd", 0, ns8390intr, 0x280,0x2000,0xd0000,
     '?',    0,   -1,    -1,    0,   0,        0,   SPL_SIX, 9},
  {&ns8390driver, "wd", 0, ns8390intr, 0x2a0,0x2000,0xd0000,
     '?',    0,   -1,    -1,    0,   0,        0,   SPL_SIX, 9},
  {&ns8390driver, "wd", 0, ns8390intr, 0x2e0,0x2000,0xd0000,
     '?',    0,   -1,    -1,    0,   0,        0,   SPL_SIX, 5},
  {&ns8390driver, "wd", 0, ns8390intr, 0x300,0x2000,0xd0000,
     '?',    0,   -1,    -1,    0,   0,        0,   SPL_SIX, 5},
  {&ns8390driver, "wd", 0, ns8390intr, 0x250,0x2000,0xd0000,
     '?',    0,   -1,    -1,    0,   0,        0,   SPL_SIX, 5},
  {&ns8390driver, "wd", 0, ns8390intr, 0x350,0x2000,0xd0000,
     '?',    0,   -1,    -1,    0,   0,        0,   SPL_SIX, 5},
  {&ns8390driver, "wd", 0, ns8390intr, 0x240,0x2000,0xd0000,
     '?',    0,   -1,    -1,    0,   0,        0,   SPL_SIX, 11},
  {&ns8390driver, "wd", 1, ns8390intr, 0x340,0x2000,0xe8000,
     '?',    0,   -1,    -1,    0,   0,        0,   SPL_SIX, 15},
#endif /* NNS8390 > 0 */

#if NAT3C501 > 0
  {&at3c501driver, "et", 0, at3c501intr, 0x300, 0,0x300,
     '?',    0,   -1,    -1,    0,   0,        0,   SPL_SIX, 9},
#endif /* NAT3C501 > 0 */

#if NCOM > 0
  {&comdriver, "com", 0, comintr, 0x3f8, 8, 0x3f8,
     '?',    0,   -1,    -1,    0,   0,        0,   SPL_TTY, 4},
  {&comdriver, "com", 1, comintr, 0x2f8, 8, 0x2f8,
     '?',    0,   -1,    -1,    0,   0,        0,   SPL_TTY, 3},
  {&comdriver, "com", 2, comintr, 0x3e8, 8, 0x3e8,
     '?',    0,   -1,    -1,    0,   0,        0,   SPL_TTY, 5},
#endif /* NCOM > 0 */

#if NLPR > 0
  {&lprdriver, "lpr", 0, lprintr, 0x378, 3, 0x378,
     '?',    0,   -1,    -1,    0,   0,        0,   SPL_TTY, 7},
  {&lprdriver, "lpr", 0, lprintr, 0x278, 3, 0x278,
     '?',    0,   -1,    -1,    0,   0,        0,   SPL_TTY, 7},
  {&lprdriver, "lpr", 0, lprintr, 0x3bc, 3, 0x3bc,
     '?',    0,   -1,    -1,    0,   0,        0,   SPL_TTY, 7},
#endif /* NLPR > 0 */

#if NWT > 0
  {&wtdriver, "wt", 0, wtintr, 0x300, 2, 0x300,
     '?',    0,   -1,    -1,    0,   0,        0,   SPL_FIVE, 5},
  {&wtdriver, "wt", 0, wtintr, 0x288, 2, 0x288,
     '?',    0,   -1,    -1,    0,   0,        0,   SPL_FIVE, 5},
  {&wtdriver, "wt", 0, wtintr, 0x388, 2, 0x388,
     '?',    0,   -1,    -1,    0,   0,        0,   SPL_FIVE, 5},
#endif /* NWT > 0 */

  0
};

/*
 * probeio:
 *
 *	Probe and subsequently attach devices out on the AT bus.
 *
 *
 */
void probeio(void)
{
	register struct	bus_device	*device;
	register struct	bus_ctlr	*master;
	int				i = 0;

	for (master = bus_master_init; master->driver; master++)
		if (configure_bus_master(master->name, master->address,
				master->phys_address, i, "atbus"))
			i++;

	for (device = bus_device_init; device->driver; device++) {
		/* ignore what we (should) have found already */
		if (device->alive || device->ctlr != -1)
			continue;
		if (configure_bus_device(device->name, device->address,
				device->phys_address, i, "atbus"))
			i++;
	}

#if	MACH_TTD
	/*
	 * Initialize Remote kernel debugger.
	 */
	ttd_init();
#endif	/* MACH_TTD */
}

void proberc(void)
{
	register struct	bus_device	*device;
	int				i = 0;
	extern int			rcline;

	if (rcline == -1) return;

	for (device = bus_device_init; device->driver; device++) {
		if (device->driver == &comdriver && device->unit == rcline)
			configure_bus_device(device->name, device->address,
					device->phys_address, i, "atbus");
	}
}

void take_dev_irq(
	struct bus_device *dev)
{
	int pic = (int)dev->sysdep1;

	if (intpri[pic] == 0) {
		iunit[pic] = dev->unit;
		ivect[pic] = dev->intr;
		intpri[pic] = (int)dev->sysdep;
		form_pic_mask();
	} else {
		printf("The device below will clobber IRQ %d.\n", pic);
		printf("You have two devices at the same IRQ.\n");
		printf("This won't work.  Reconfigure your hardware and try again.\n");
		printf("%s%d: port = %x, spl = %d, pic = %d.\n",
		        dev->name, dev->unit, dev->address,
			dev->sysdep, dev->sysdep1);
		while (1);
	}
		
}

void take_ctlr_irq(
	struct bus_ctlr *ctlr)
{
	int pic = ctlr->sysdep1;
	if (intpri[pic] == 0) {
		iunit[pic] = ctlr->unit;
		ivect[pic] = ctlr->intr;
		intpri[pic] = (int)ctlr->sysdep;
		form_pic_mask();
	} else {
		printf("The device below will clobber IRQ %d.\n", pic);
		printf("You have two devices at the same IRQ.  This won't work.\n");
		printf("Reconfigure your hardware and try again.\n");
		while (1);
	}
}
