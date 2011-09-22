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
 * $Log:	pm_hdw.c,v $
 * Revision 2.16  93/05/15  19:37:36  mrt
 * 	machparam.h -> machspl.h
 * 
 * Revision 2.15  93/05/10  20:08:30  rvb
 * 	Fixed types.
 * 	[93/05/06  09:57:15  af]
 * 
 * Revision 2.14  93/01/14  17:21:39  danner
 * 	Proper spl typing.
 * 	[92/11/30            af]
 * 
 * Revision 2.13  92/05/22  15:48:07  jfriedl
 * 	Now all screens clearup at boot.
 * 	[92/05/21            af]
 * 
 * Revision 2.12  91/08/24  11:52:40  af
 * 	Declare screen sizes, new calling sequences for a couple funcs.
 * 	[91/08/02  01:58:48  af]
 * 
 * Revision 2.11  91/06/25  20:54:31  rpd
 * 	Tweaks to make gcc happy.
 * 
 * Revision 2.10  91/06/19  11:54:07  rvb
 * 	mips->DECSTATION; vax->VAXSTATION
 * 	[91/06/12  14:02:01  rvb]
 * 
 * 	File moved here from mips/PMAX since it tries to be generic;
 * 	it is used on the PMAX and the Vax3100.
 * 	[91/06/04            rvb]
 * 
 * Revision 2.9  91/05/14  17:25:24  mrt
 * 	Correcting copyright
 * 
 * Revision 2.8  91/02/14  14:35:10  mrt
 * 	In interrupt routine, drop priority as now required.
 * 	[91/02/12  12:42:39  af]
 * 
 * Revision 2.7  91/02/05  17:43:11  mrt
 * 	Added author notices
 * 	[91/02/04  11:15:55  mrt]
 * 
 * 	Changed to use new Mach copyright
 * 	[91/02/02  12:14:43  mrt]
 * 
 * Revision 2.6  90/12/05  23:33:16  af
 * 	Cleanups.
 * 	[90/12/03  23:30:26  af]
 * 
 * Revision 2.4.1.1  90/11/01  03:45:08  af
 * 	Created, from the DEC specs:
 * 	"DECstation 3100 Desktop Workstation Functional Specification"
 * 	Workstation Systems Engineering, Palo Alto, CA. Aug 28, 1990.
 * 	And from code written by Robert V. Baron at CMU.
 * 	[90/09/03            af]
 */
/*
 *	File: pm_hdw.c
 * 	Author: Alessandro Forin, Carnegie Mellon University
 *	Date:	9/90
 *
 *	Driver for the VFB01/02 Mono/Color framebuffer (pmax)
 *	Hardware-level operations.
 */

#include <bm.h>
#if	NBM>0
#include <platforms.h>

#include <machine/machspl.h>		/* spl definitions */
#include <chips/screen_defs.h>
#include <chips/pm_defs.h>
#include <chips/busses.h>
#include <machine/machspl.h>

#ifdef	DECSTATION
#include <mips/mips_cpu.h>
#include <mips/PMAX/kn01.h>

#define	KN01_CSR_ADDR		PHYS_TO_K1SEG(KN01_SYS_CSR)
#define	KN01_FBUF_ADDR		PHYS_TO_K1SEG(KN01_PHYS_FBUF_START)
#define	KN01_PLM_ADDR		PHYS_TO_K1SEG(KN01_PHYS_COLMASK_START)
#define	KN01_BT478_ADDR		PHYS_TO_K1SEG(KN01_SYS_VDAC)
#define	KN01_DC503_ADDR		PHYS_TO_K1SEG(KN01_SYS_PCC)

#define VRETRACE		dc503_vretrace
#define	MONO_FRAME_WIDTH	2048
#define ISA_MONO		((*(volatile short*)KN01_CSR_ADDR)&KN01_CSR_MONO)

#endif	/*DECSTATION*/

#ifdef	VAXSTATION
#include <vax/ka3100.h>
#define VRETRACE		ka42_vretrace
#define ISA_MONO		1
#define	MONO_FRAME_WIDTH	1024
#endif	/*VAXSTATION*/

/*
 * Definition of the driver for the auto-configuration program.
 */

int	pm_probe(), pm_intr();
static void pm_attach();

vm_offset_t	pm_std[] = { 0 };
struct	bus_device *pm_info[1];
struct	bus_driver pm_driver = 
        { pm_probe, 0, pm_attach, 0, pm_std, "pm", pm_info, };

/*
 * Probe/Attach functions
 */

pm_probe( /* reg, ui */)
{
	static probed_once = 0;
#ifdef	DECSTATION
	if (!isa_pmax())
		return 0;
	if (check_memory(KN01_FBUF_ADDR, 0))
		return 0;
#endif	/*DECSTATION*/
	if (probed_once++ > 1)
		printf("[mappable] ");
	return 1;
}

static void
pm_attach(ui)
	struct bus_device *ui;
{
	int isa_mono = ISA_MONO;

	printf(": %s%s",
		isa_mono ? "monochrome" : "color",
		" display");
}


/*
 * Interrupt routine
 */
#ifdef	DECSTATION
pm_intr(unit,spllevel)
	spl_t	spllevel;
{
	/* this is the vertical retrace one */
	splx(spllevel);
	lk201_led(unit);
}
#endif	/*DECSTATION*/

#ifdef	VAXSTATION
pm_intr(unit)
{
	lk201_led(unit);
}
#endif	/*VAXSTATION*/

/*
 * Boot time initialization: must make device
 * usable as console asap.
 */
extern int
	pm_cons_init(), pm_soft_reset(),
	dc503_video_on(), dc503_video_off(),
	pm_char_paint(), dc503_pos_cursor(),
	pm_insert_line(), pm_remove_line(), pm_clear_bitmap(),
	pm_set_status(), pm_get_status(), 
	VRETRACE(), pm_map_page();

static	struct screen_switch pm_sw = {
	screen_noop,		/* graphic_open */
	pm_soft_reset,		/* graphic_close */
	pm_set_status,		/* set_status */
	pm_get_status,		/* set_status */
	pm_char_paint,		/* char_paint */
	dc503_pos_cursor,	/* pos_cursor */
	pm_insert_line,		/* insert_line */
	pm_remove_line,		/* remove_line */
	pm_clear_bitmap,	/* clear_bitmap */
	dc503_video_on,		/* video_on */
	dc503_video_off,	/* video_off */
	VRETRACE,		/* enable vert retrace intr */
	pm_map_page		/* map_page */
};

pm_cold_init(unit, up)
	user_info_t	*up;
{
	pm_softc_t	*pm;
	screen_softc_t	sc = screen(unit);
	int		isa_mono = ISA_MONO;

	bcopy(&pm_sw, &sc->sw, sizeof(sc->sw));
	if (isa_mono) {
		sc->flags |= MONO_SCREEN;
		sc->frame_scanline_width = MONO_FRAME_WIDTH;
	} else {
		sc->flags |= COLOR_SCREEN;
		sc->frame_scanline_width = 1024;
	}
	sc->frame_height = 864;
	sc->frame_visible_width = 1024;
	sc->frame_visible_height = 864;

	pm_init_screen_params(sc, up);
	(void) screen_up(unit, up);

#ifdef	DECSTATION
	pm = pm_alloc(unit, KN01_DC503_ADDR, KN01_FBUF_ADDR, KN01_PLM_ADDR);
	pm->vdac_registers = (char*)KN01_BT478_ADDR;
#endif	/*DECSTATION*/
#ifdef	VAXSTATION
	pm = pm_alloc(unit, cur_xxx, bm_mem, 0);
#endif	/*VAXSTATION*/

	screen_default_colors(up);

	dc503_init(pm);

	pm_soft_reset(sc);

	/*
	 * Clearing the screen at boot saves from scrolling
	 * much, and speeds up booting quite a bit.
	 */
	screen_blitc( unit, 'C'-'@');/* clear screen */
}

#endif	NBM>0
