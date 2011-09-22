/* 
 * Mach Operating System
 * Copyright (c) 1991,1990,1989 Carnegie Mellon University
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
 * HISTORY
 * $Log:	autoconf.c,v $
 * Revision 2.2  93/02/04  07:58:55  danner
 * 	Integrate PS2 code from IBM.
 * 	[93/01/18            prithvi]
 * 
 * Revision 1.7.1.8  90/11/27  13:40:50  rvb
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
 
#include <platforms.h>
#ifdef	MACH_KERNEL
#include <sys/types.h>
#else	MACH_KERNEL
#include <cpus.h>
#include <cputypes.h>
#include <generic.h>
#include <sys/param.h>
#include <mach/machine.h>
#include <machine/cpu.h>
#include <sys/dk.h>
#endif	MACH_KERNEL
#include <i386/ipl.h>
#include <i386ps2/bus.h>

extern struct	isa_dev	Devs[];
extern struct	isa_ctlr	Ctlrs[];

/*
 * probeio:
 *
 *	Probe and subsequently attach devices out on the i386 bus.
 *
 *	We do this by looking at the Devs array, and doing controller
 *	probe's for all the controllers that are pointed to by the 
 *	devices. Strictly speaking we probably should use the Ctrl's
 *	array but this will suffice for now.
 *
 */
probeio()

{
	struct	isa_dev		*dev_p;
	struct	isa_driver	*drv_p;
	struct isa_ctlr		*ctl_p;
	int result;
	int			dkn = 0;

	/*
	 * loop the the controller structures and if probe returns
	 * that the device exists then it is available to 
	 * have slaves attached to it.
	 */
	for (ctl_p = Ctlrs; drv_p = ctl_p->ctlr_driver; ctl_p++) {
		if ((*drv_p->driver_probe)(ctl_p->ctlr_addr, ctl_p))
			ctl_p->ctlr_alive = 1;
		else
			continue;
		if (drv_p->driver_minfo)
			drv_p->driver_minfo[ctl_p->ctlr_ctlr] = ctl_p;
		printf("%s%d controller at %x irq %d spl %d\n",
			drv_p->driver_mname ? drv_p->driver_mname : "??",
			ctl_p->ctlr_ctlr,
			ctl_p->ctlr_addr,
			ctl_p->ctlr_pic,
			ctl_p->ctlr_spl);
	}
	for (dev_p = Devs; drv_p = dev_p->dev_driver; dev_p++) {
		ctl_p = dev_p->dev_mi;
#ifdef	DEBUG
		printf("%s%d\n", drv_p->driver_dname, dev_p->dev_unit);
#endif	DEBUG
	/*
	 * if the device has a controller then skip it if the controller
	 * isn't alive.
	 */
		if ((int)ctl_p && !ctl_p->ctlr_alive) 
			continue;
	/*
	 * if it has a controller then we call the slave routine 
	 * otherwise we call the probe routine. In either case if 
	 * it is there we call the attach routine.
	 */
		if ((int)ctl_p)
			result = (*drv_p->driver_slave)(dev_p, dev_p->dev_addr);
		else
			result = (*drv_p->driver_probe)(dev_p->dev_addr, dev_p);
		if (result) {
			dev_p->dev_alive = 1;
			if (drv_p->driver_dinfo)
				drv_p->driver_dinfo[dev_p->dev_unit] = dev_p;
			if (ctl_p) {
#ifndef MACH_KERNEL
				if (dev_p->dev_dk && dkn < DK_NDRIVE)
                                        dev_p->dev_dk = dkn++;
                                else
                                        dev_p->dev_dk = -1;
#endif MACH_KERNEL

				printf("%s%d at %s%d slave %d\n", 
					drv_p->driver_dname? drv_p->driver_dname : "??",
					dev_p->dev_unit,
					drv_p->driver_mname ? drv_p->driver_mname : "??",
					dev_p->dev_ctlr,
					dev_p->dev_slave);
			} else {
				printf("%s%d adapter at %x irq %d spl %d\n",
					drv_p->driver_dname? drv_p->driver_dname : "??",
					dev_p->dev_unit,
					dev_p->dev_addr, dev_p->dev_pic,
					dev_p->dev_spl);
			}
			(*drv_p->driver_attach)(dev_p);
		}
	}
}

take_dev_irq(dev)
struct isa_dev	*dev;
{
	take_irq(dev->dev_pic, dev->dev_unit, dev->dev_intr[0], 
		dev->dev_spl, dev->dev_driver->driver_dname);
}

take_ctlr_irq(ctlr)
struct isa_ctlr *ctlr;
{
	take_irq(ctlr->ctlr_pic, ctlr->ctlr_ctlr, ctlr->ctlr_intr[0],
		ctlr->ctlr_spl, ctlr->ctlr_driver->driver_mname);
}

take_irq(pic, unit, intr, spl, name)
	int pic, unit, spl;
	int (*intr)();
	char *name;
{
	if (name == 0)
		name = "??";
	if (pic < 0 || pic > 15) {
		printf("The device %s%d has invalid IRQ %d; ignored\n", name, unit, pic);
		return;
	}
	if (spl == SPL0)
		printf("Warning: device %s%d is at SPL0! This doesn't inhibit interrupts.\n", name, unit);
	if (spl == SPLHI)
		printf("Warning: device %s%d is at SPLHI! Only the clock can be at SPLHI.\n", name, unit);
	if (intpri[pic] == 0) {
		iunit[pic] = unit;
		ivect[pic] = intr;
		intpri[pic] = spl;
		form_pic_mask();
	} else if (ivect[pic] == intr) {
		printf("The device %s%d already has IRQ %d; ignored\n", name, unit, pic);
	} else {
		printf("The device %s%d will clobber IRQ %d.\n", name, unit, pic);
		printf("You have two devices at the same IRQ.  This won't work.\n");
		printf("Reconfigure your hardware and try again.\n");
	}
}

#ifdef	MACH_KERNEL
#else	MACH_KERNEL
/*
 * Determine mass storage and memory configuration for a machine.
 * Get cpu type, and then switch out to machine specific procedures
 * which will probe adaptors to see what is out there.
 */
configure()
{

	master_cpu = 0;
	set_cpu_number();
#if	NCPUS > 1
	printf("Master cpu at slot %d.\n", master_cpu);
#endif	NCPUS > 1
	machine_slot[master_cpu].is_cpu = TRUE;
	machine_slot[master_cpu].running = TRUE;
	machine_slot[master_cpu].cpu_type = CPU_TYPE_I386;
	cpuspeed = 6;
#ifdef	AT386
	machine_slot[master_cpu].cpu_subtype = CPU_SUBTYPE_AT386;
	probeio();
#endif	AT386
#ifdef	PS2
	findspeed();		/* calculate the delays first */
	microfind();		/* as we need them in the drivers */
	machine_slot[master_cpu].cpu_subtype = CPU_SUBTYPE_PS2;
	probeio();
#endif	AT386

#if     iPSC2
        machine_slot[master_cpu].cpu_subtype = CPU_SUBTYPE_iPSC2;

        /* Call DCM driver to initialize itself, interrupts are already
           enabled at this point. We may want to change this later so that
           we disable DCM interrupts during bootld and turn them back on
           in dcminit().
         */
        dcminit();
#endif  iPSC2

#if	GENERIC > 0
	setconf();
#endif	GENERIC > 0
	return;
}

	/*
	 * slave_config is a temporary artifact which will go away as soon
	 * as kern/slave.c is made conditional on NCPUS > 1
	 */
slave_config()
{
}
#endif	MACH_KERNEL

#ifdef PS2
/*
 * get the POS registers information so that drivers can determine 
 * what adapters are out there.
 */
pos_init()
{
	int i, j;

	outb(POS_DISABLE_VGA, 0xff);	/* disable vga etc. */
	for (i=0; i<MAX_POS_SLOTS; ++i) {
		outb(POS_PORT, POS_ENABLE(i));
		for (j=0; j<MAX_POS_BYTES; ++j)
			slots[i].pos_data[j] = POS_GET_DATA(j);
#ifndef i386
/*
 * on i386 we don't need to do anything because it is already handled
 * by the union 
 */
		slots[i].pos_id = (slots[i].pos_data[1] << 8) |
			slots[i].pos_data[0];
#endif
	}
	outb(POS_PORT, POS_DISABLE);

#ifdef POS_DEBUG
	for (i=0; i<MAX_POS_SLOTS; ++i) {
		if (slots[i].pos_id == POS_ID_NONE)
			continue;
		printf("slot %d id %x", i, slots[i].pos_id);
		for (j=0; j<MAX_POS_INFO; ++j)
			printf(" %x", slots[i].pos_info[j]);
		printf("\n");
	}
#endif POS_DEBUG
}

/*
 * function to lookup given adapter card id and return
 * it's slot number. We are given the initial slot number
 * to start at so we can find more than one card.
 * we return -1 when we fail.
 */
int pos_slot(card_id, slot)
u_short card_id;
int	slot;
{
	int i, j;

	for (; slot >= 0 && slot<MAX_POS_SLOTS; ++slot)
		if (card_id == slots[slot].pos_id)
			return(slot);
	return(-1);
}
#endif PS2

