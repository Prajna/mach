/* 
 * Mach Operating System
 * Copyright (c) 1991,1990,1989 Carnegie Mellon University
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
 * $Log:	cfb_hdw.c,v $
 * Revision 2.14  93/05/15  19:35:54  mrt
 * 	machparam.h -> machspl.h
 * 
 * Revision 2.13  93/05/10  20:07:25  rvb
 * 	Fixed types.
 * 	[93/05/06  10:00:37  af]
 * 
 * Revision 2.12  93/02/05  08:04:55  danner
 * 	Changes for Flamingo.
 * 	[93/02/04  01:28:05  af]
 * 
 * 	Interrupt routine takes an spllevel on mips only.
 * 	[92/11/30            af]
 * 
 * Revision 2.11  92/05/22  15:46:47  jfriedl
 * 	Now all screens clearup at boot.
 * 	[92/05/21            af]
 * 
 * Revision 2.10  92/02/19  16:45:39  elf
 * 	Reflected changes in tc_enable_interrupt().
 * 	[92/02/10  17:13:38  af]
 * 
 * Revision 2.9  91/08/24  11:51:31  af
 * 	Works on 3mins too.  New calling seq, declare screen sizes.
 * 	[91/08/02  01:59:46  af]
 * 
 * Revision 2.8  91/06/25  20:53:24  rpd
 * 	Tweaks to make gcc happy.
 * 
 * Revision 2.7  91/06/19  16:57:45  rvb
 * 		File moved here from mips/PMAX since it tries to be generic.
 * 	[91/06/04            rvb]
 * 
 * Revision 2.6  91/05/14  17:20:02  mrt
 * 	Correcting copyright
 * 
 * Revision 2.5  91/02/14  14:33:38  mrt
 * 	In interrupt routine, drop priority as now required.
 * 	[91/02/12  12:44:36  af]
 * 
 * Revision 2.4  91/02/05  17:40:04  mrt
 * 	Added author notices
 * 	[91/02/04  11:12:22  mrt]
 * 
 * 	Changed to use new Mach copyright
 * 	[91/02/02  12:10:07  mrt]
 * 
 * Revision 2.3  90/12/05  23:30:35  af
 * 	Cleaned up.
 * 	[90/12/03  23:11:53  af]
 * 
 * Revision 2.1.1.1  90/11/01  03:42:09  af
 * 	Created, from the DEC specs:
 * 	"PMAG-BA TURBOchannel Color Frame Buffer Functional Specification"
 * 	Workstation Systems Engineering, Palo Alto, CA. Aug 27, 1990
 * 	[90/09/03            af]
 */
/*
 *	File: cfb_hdw.c
 * 	Author: Alessandro Forin, Carnegie Mellon University
 *	Date:	9/90
 *
 *	Driver for the 3max Color Frame Buffer Display,
 *	hardware-level operations.
 */

#include <cfb.h>
#if	(NCFB > 0)

#include <platforms.h>

#include <machine/machspl.h>
#include <mach/std_types.h>
#include <chips/busses.h>
#include <chips/screen_defs.h>
#include <chips/pm_defs.h>

typedef pm_softc_t	cfb_softc_t;

#ifdef	DECSTATION
#include <mips/PMAX/pmag_ba.h>
#include <mips/PMAX/tc.h>
#endif

#ifdef	FLAMINGO
#include <mips/PMAX/pmag_ba.h>		/* XXX fixme */
#include <alpha/DEC/tc.h>
#endif

/*
 * Definition of the driver for the auto-configuration program.
 */

int	cfb_probe(), cfb_intr();
static void	cfb_attach();

vm_offset_t	cfb_std[NCFB] = { 0 };
struct	bus_device *cfb_info[NCFB];
struct	bus_driver cfb_driver = 
        { cfb_probe, 0, cfb_attach, 0, cfb_std, "cfb", cfb_info,
	  0, 0, BUS_INTR_DISABLED};

/*
 * Probe/Attach functions
 */

cfb_probe( /* reg, ui */)
{
	static probed_once = 0;

	/*
	 * Probing was really done sweeping the TC long ago
	 */
	if (tc_probe("cfb") == 0)
		return 0;
	if (probed_once++ > 1)
		printf("[mappable] ");
	return 1;
}

static void
cfb_attach(ui)
	struct bus_device *ui;
{
	/* ... */
	printf(": color display");
}


/*
 * Interrupt routine
 */

cfb_intr(unit,spllevel)
	spl_t	spllevel;
{
	register volatile char *ack;

	/* acknowledge interrupt */
	ack = (volatile char *) cfb_info[unit]->address + CFB_OFFSET_IREQ;
	*ack = 0;

#ifdef	mips
	splx(spllevel);
#endif
	lk201_led(unit);
}

cfb_vretrace(cfb, on)
	cfb_softc_t	*cfb;
{
	int i;

	for (i = 0; i < NCFB; i++)
		if (cfb_info[i]->address == (vm_offset_t)cfb->framebuffer)
			break;
	if (i == NCFB) return;

	(*tc_enable_interrupt)(cfb_info[i]->adaptor, on, 0);
}

/*
 * Boot time initialization: must make device
 * usable as console asap.
 */
extern int
	cfb_soft_reset(), cfb_set_status(),
	bt459_pos_cursor(), bt459_video_on(),
	bt459_video_off(), cfb_vretrace(),
	pm_get_status(), pm_char_paint(),
	pm_insert_line(), pm_remove_line(),
	pm_clear_bitmap(), pm_map_page();

static struct screen_switch cfb_sw = {
	screen_noop,		/* graphic_open */
	cfb_soft_reset,		/* graphic_close */
	cfb_set_status,		/* set_status */
	pm_get_status,		/* get_status */
	pm_char_paint,		/* char_paint */
	bt459_pos_cursor,	/* pos_cursor */
	pm_insert_line,		/* insert_line */
	pm_remove_line,		/* remove_line */
	pm_clear_bitmap,	/* clear_bitmap */
	bt459_video_on,		/* video_on */
	bt459_video_off,	/* video_off */
	cfb_vretrace,		/* intr_enable */
	pm_map_page		/* map_page */
};

cfb_cold_init(unit, up)
	user_info_t	*up;
{
	cfb_softc_t	*cfb;
	screen_softc_t	sc = screen(unit);
	int		base = tc_probe("cfb");

	bcopy(&cfb_sw, &sc->sw, sizeof(sc->sw));
	sc->flags |= COLOR_SCREEN;
	sc->frame_scanline_width = 1024;
	sc->frame_height = 1024;
	sc->frame_visible_width = 1024;
	sc->frame_visible_height = 864;

	pm_init_screen_params(sc,up);
	(void) screen_up(unit, up);

	cfb = pm_alloc(unit, base + CFB_OFFSET_BT459, base + CFB_OFFSET_VRAM, -1);

	screen_default_colors(up);

	cfb_soft_reset(sc);

	/*
	 * Clearing the screen at boot saves from scrolling
	 * much, and speeds up booting quite a bit.
	 */
	screen_blitc( unit, 'C'-'@');/* clear screen */
}

#endif	(NCFB > 0)
