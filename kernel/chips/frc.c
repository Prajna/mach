/* 
 * Mach Operating System
 * Copyright (c) 1993,1992 Carnegie Mellon University
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
 * $Log:	frc.c,v $
 * Revision 2.9  93/08/10  15:18:26  mrt
 * 	Made frc's accessible to kernel code via frc_address[].
 * 	[93/07/20            cmaeda]
 * 
 * Revision 2.8  93/05/17  10:27:00  rvb
 * 	Type casts, etc to quiet gcc 2.3.3 warnings
 * 
 * Revision 2.7  93/05/10  20:07:53  rvb
 * 	Fixed types.
 * 	[93/05/06  09:58:36  af]
 * 
 * Revision 2.6  93/03/26  17:58:49  mrt
 * 	Removed all uses of minor().
 * 	[93/03/17            af]
 * 
 * Revision 2.5  93/01/24  13:55:13  danner
 * 	Sandro's fixes to danner's fixes to mattz's fixes.
 * 	[93/01/17            mrt]
 * 
 * Revision 2.4  93/01/14  17:16:25  danner
 * 	Fixed mattz's open of nonexistenent frc problem.
 * 	[93/01/14            danner]
 * 
 * 	Corrected type of frc_std.
 * 	[93/01/14            danner]
 * 
 * Revision 2.3  92/05/05  10:53:00  danner
 * 	Added frc_set_address(), to customize it.
 * 	Makes NSC's boards happy (any TC box).
 * 	[92/04/13            jcb]
 * 
 * Revision 2.2  92/04/01  15:14:23  rpd
 * 	Created, based on maxine's counter.
 * 	[92/03/10            af]
 * 
 */
/*
 *	File: frc.c
 * 	Author: Alessandro Forin, Carnegie Mellon University
 *	Date:	3/92
 *
 *	Generic, mappable free running counter driver.
 */

#include <frc.h>
#if	NFRC > 0

#include <mach/std_types.h>
#include <chips/busses.h>
#include <device/device_types.h>

/*
 * Machine defines
 * All you need to do to get this working on a
 * random box is to define one macro and provide
 * the correct virtual address.
 */
#include	<platforms.h>
#ifdef	DECSTATION
#define	btop(x)		mips_btop(x)
#endif	/* DECSTATION */

/*
 * Autoconf info
 */

static vm_offset_t frc_std[NFRC] = { 0 };
static vm_size_t frc_offset[NFRC] = { 0 };
static struct bus_device *frc_info[NFRC];
static int frc_probe(vm_offset_t,struct bus_ctlr *);
static void frc_attach(struct bus_device *);

struct bus_driver frc_driver =
       { frc_probe, 0, frc_attach, 0, frc_std, "frc", frc_info, };

/*
 * Externally visible functions
 */
io_return_t	frc_openclose(int,int);			/* user */
vm_offset_t	frc_mmap(int,vm_offset_t,vm_prot_t);
void		frc_set_address(int,vm_size_t);

/*
 * FRC's in kernel virtual memory.  For in-kernel timestamps.
 */
vm_offset_t frc_address[NFRC];

/* machine-specific setups */
void
frc_set_address(
	int		unit,
	vm_size_t	offset)
{
	if (unit < NFRC) {
		frc_offset[unit] =  offset;
	}
}


/*
 * Probe chip to see if it is there
 */
static frc_probe (
	vm_offset_t	reg,
	struct bus_ctlr *ui)
{
	/* see if something present at the given address */
	if (check_memory(reg, 0)) {
		frc_address[ui->unit] = 0;
		return 0;
	}
	frc_std[ui->unit] = (vm_offset_t) reg;
	printf("[mappable] ");
	return 1;
}

static void
frc_attach (
		   struct bus_device *ui)
{
	if (ui->unit < NFRC) {
		frc_address[ui->unit] =
			(vm_offset_t) frc_std[ui->unit] + frc_offset[ui->unit];
		printf(": free running counter %d at kernel vaddr 0x%x",
		       ui->unit, frc_address[ui->unit]);
	}
	else
		panic("frc: unknown unit number"); /* shouldn't happen */
}

int frc_intr()
{
	/* we do not expect interrupts */
	panic("frc_intr");
}

io_return_t
frc_openclose(
	      int dev, 
	      int flag)
{
  if (frc_std[dev])
    return D_SUCCESS;
  else
    return D_NO_SUCH_DEVICE;
}

vm_offset_t
frc_mmap(
	int		dev,
	vm_offset_t	off,
	vm_prot_t	prot)
{
  	vm_offset_t addr;
	if ((prot & VM_PROT_WRITE) || (off >= PAGE_SIZE) )
		return (-1);
	addr = (vm_offset_t) frc_std[dev] + frc_offset[dev];
	return btop(pmap_extract(pmap_kernel(), addr));
}

#endif
