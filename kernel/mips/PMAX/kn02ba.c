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
 * $Log:	kn02ba.c,v $
 * Revision 2.6  93/05/30  21:08:25  rvb
 * 	Took pity of a friend and fixed copyout not to overcopy.
 * 	[93/05/28            af]
 * 
 *
 * Revision 2.5  93/05/10  21:20:38  rvb
 * 	Debounce halt button press.
 * 	[93/05/06  09:45:53  af]
 * 
 * 	No more sys/types.h
 * 	[93/05/06  09:41:26  af]
 * 
 * Revision 2.4  93/01/14  17:50:19  danner
 * 	Added storage declarator for kn02ba_haltintr_count.
 * 	[93/01/14            danner]
 * 
 * Revision 2.3  92/05/05  10:46:30  danner
 * 	Fix from Charles_Silvers@CS.CMU.EDU: copyin_gap32 was not
 * 	copying properly the last two bytes at the end of the loop.
 * 	[92/05/01            af]
 * 
 * Revision 2.2  92/03/02  18:34:20  rpd
 * 	Created, from kmin code.
 * 	[92/03/02            af]
 * 
 */
/*
 *	File: kn02ba.c
 * 	Author: Alessandro Forin, Carnegie Mellon University
 *	Date:	2/92
 *
 *	Routines common to all uses of the KN02BA/DA daughtercard
 *	processors, currently on 3min and MAXine.
 *	We use the 3min's defines where the two systems
 *	have identical address mappings.
 */

#include <mach/std_types.h>
#include <mach/vm_param.h>
#include <chips/serial_defs.h>

#include <mips/thread.h>
#include <mips/mips_cpu.h>
#include <mips/mips_box.h>
#include <mips/prom_interface.h>
#include <mips/PMAX/maxine.h>
#include <mips/PMAX/kmin.h>
#include <mips/PMAX/tc.h>

#define	KMIN_INTR		PHYS_TO_K1SEG(KMIN_REG_INTR)
#define	KMIN_IMSK		PHYS_TO_K1SEG(KMIN_REG_IMSK)

#define	KMIN_ERR		PHYS_TO_K1SEG(KMIN_REG_MER)
#define	KMIN_MSR		PHYS_TO_K1SEG(KMIN_REG_MSR)

#define	KMIN_ADERR		PHYS_TO_K1SEG(KMIN_REG_AER)
#define	KMIN_TIMEO		PHYS_TO_K1SEG(KMIN_REG_TIMEOUT)

/*
 *	Object:
 *		kn02ba_errintr			EXPORTED function
 *
 *	Handle "memory errors":
 *		ECC errors, DMA errors and overrun, CPU write timeouts
 *
 */
static unsigned
kn02ba_recover_erradr(phys, mer)
	register unsigned	phys, mer;
{
	/* phys holds bits 28:2, mer knows which byte */
	switch (mer & KMIN_MER_LASTBYTE) {
	case KMIN_LASTB31:
		mer = 3; break;
	case KMIN_LASTB23:
		mer = 2; break;
	case KMIN_LASTB15:
		mer = 1; break;
	case KMIN_LASTB07:
		mer = 0; break;
	}
	return ((phys & KMIN_AER_ADDR_MASK) | mer);
}

int errintr_cnt;

kn02ba_errintr(st,spllevel)
{
	register int mer, adr, siz;

	siz = *(volatile int *)KMIN_MSR;
	mer = *(volatile int *)KMIN_ERR;
	adr = *(volatile int *)KMIN_ADERR;

	/* clear interrupt bit */
	*(unsigned int *)KMIN_TIMEO = 0;

	errintr_cnt++;
	printf("(%d)%s%x [%x %x %x]\n", errintr_cnt,
	       "Bad memory chip at phys ",
	       kn02ba_recover_erradr(adr, mer),
	       mer, siz, adr);
}

/*
 *	Object:
 *		kn02ba_haltintr		EXPORTED function
 *
 *	Handle interrupt because user pushed the Halt button
 *
 */
unsigned kn02ba_haltintr_count = 0;	/* patch for production to 2 */
kn02ba_haltintr(st,spllevel)
{
	/* It is not debounced, so wait half a sec */
	delay(500000);
	if (++kn02ba_haltintr_count == 3)
		halt_all_cpus(FALSE);
	gimmeabreak();
}

/*
 *	Object:
 *		kn02ba_steal_memory		EXPORTED function
 *
 *	Take some contiguous physical memory away from the system
 *	for our use: for the screen mapped control info and for
 *	the Lance buffer.
 *	The waste is big because: (a) the Lance buffer must be aligned
 *	on a 128k boundary and (b) we must take 128k but will
 *	only use 64k.  Double sigh.
 *	[it is actually possible to use the other 64k, ugly but possible]
 *	XXXXXXXXXX   fix me XXXXXXXXXX fix me XXXXXXXXXXXX fix me XXXXXXX
 *
 */
vm_size_t
kn02ba_steal_memory(start)
	vm_offset_t	start;
{
	vm_size_t	needed;
	vm_offset_t	sebuf;

	needed = screen_memory_alloc(start);
	needed = mips_round_page(needed);

	/* make phys, round up to 128k */
	start = K0SEG_TO_PHYS(start);
	sebuf = start + needed;
	sebuf = (sebuf + 0x1ffff) & ~0x1ffff;

	needed  = sebuf - start;	/* due to rounding */
	needed += 128 * 1024;		/* 64k needed, but.. */

	setse_switch(2, PHYS_TO_K1SEG(KMIN_SYS_LANCE), 
			PHYS_TO_K1SEG(sebuf), 0,
			PHYS_TO_K1SEG(KMIN_SYS_ETHER_ADDRESS));
	asic_enable_lance(sebuf);

	return needed;
}

/*
 *	Object:
 *		copyout_gap32			EXPORTED function
 *
 *	Copy data to lance (data) buffer: dma is 4 words every
 *	other 4 words.  We know: 'to' is aligned to a 4 words
 *	boundary, 'from' is short-aligned.  Too bad if we
 *	copy a bit too much.
 */
copyout_gap32(from, to, nbytes)
	register unsigned int	*from;
	register unsigned int	*to;
	register int		nbytes;
{
	register unsigned int	t0,t1,t2,t3;
	register unsigned short s0;


	/* first copy until 'to' is quadword aligned */
	if (s0 = ((vm_offset_t)to) & 0xf) {
		register unsigned short *sfrom, *sto;
		
		sfrom = (unsigned short*)from;
		sto = (unsigned short *)to;

		s0 = 16 - s0;

		switch (s0) {
		case 14: *sto++ = *sfrom++;
		case 12: *sto++ = *sfrom++;
		case 10: *sto++ = *sfrom++;
		case  8: *sto++ = *sfrom++;
		case  6: *sto++ = *sfrom++;
		case  4: *sto++ = *sfrom++;
		case  2: *sto++ = *sfrom++;
		}

		nbytes -= s0;
		to = (unsigned int *)sto;
		to += 4;/*dma twist!*/
		from = (unsigned int *)sfrom;
	}

	/* now it is 'from' that we don't know much about */
	if (((vm_offset_t)from) & 0x2) {

#if 0
		/* bad day, we get many */
		s0 = *(unsigned short *)from;
		from = (unsigned int *)(((short*)from) + 1); /* aligned now */

		while (1) {
			t0 = from[0]; t1 = from[1]; t2 = from[2]; t3 =from[3];
			to[0] = 	s0 | (t0 << 16);
			to[1] = (t0 >> 16) | (t1 << 16);
			to[2] = (t1 >> 16) | (t2 << 16);
			to[3] = (t2 >> 16) | (t3 << 16);
			nbytes -= 4 * sizeof(int);
			if (nbytes <= 0)
				break;
			s0 = t3 >> 16;
			to += 8;/* dma twist */
			from += 4;
		}
#else
		register unsigned short *sfrom, *sto;

		sfrom = (unsigned short*)from;
		sto = (unsigned short *)to;

		while (nbytes > 0) {
			*sto++ = *sfrom++;
			nbytes -= sizeof(short);
			if (((int)sto & 0xf) == 0)
				sto += 8;/*dma twist*/
		}

#endif

	} else
		/* a good day */
		while (nbytes >= (int)(4*sizeof(int))) {
			t0 = from[0]; t1 = from[1]; t2 = from[2]; t3=from[3];
			from += 4;
			to[0] = t0; to[1] = t1; to[2] = t2; to[3] = t3;
			to += 8;/*dma twist!*/
			nbytes -= 4 * sizeof(int);
		}
		switch (nbytes) {
		case (4*sizeof(int) - 1):
		case (4*sizeof(int) - 2):
		case (4*sizeof(int) - 3):
			to[3] = from[3];	/* fall through */
		case (3*sizeof(int) - 0):
		case (3*sizeof(int) - 1):
		case (3*sizeof(int) - 2):
		case (3*sizeof(int) - 3):
			to[2] = from[2];	/* fall through */
		case (2*sizeof(int) - 0):
		case (2*sizeof(int) - 1):
		case (2*sizeof(int) - 2):
		case (2*sizeof(int) - 3):
			to[1] = from[1];	/* fall through */
		case (1*sizeof(int) - 0):
		case (1*sizeof(int) - 1):
		case (1*sizeof(int) - 2):
		case (1*sizeof(int) - 3):
			to[0] = from[0];	/* fall through */
		default:
			break;
		}
}


/*
 *	Object:
 *		copyin_gap32			EXPORTED function
 *
 *	Copy data from lance (data) buffer: dma is 4 words every
 *	other 4 words.  Called in two modes: (a) for the ether header
 *	which is 14 bytes, word aligned (b) for the data, which
 *	is any size but the source is short aligned (triple sigh).
 *	Destinations are word aligned in both cases
 */
copyin_gap32(from, to, len)
	register unsigned int	*from;
	register unsigned int	*to;
	register int		len;
{
	/* no need for generalities, just do it fast */
	if (len <= 16) {
		/* ether header */
		register int	t0,t1,t2,t3;

		t0 = from[0]; t1 = from[1]; t2 = from[2]; t3 = from[3];
		to[0] = t0; to[1] = t1; to[2] = t2; to[3] = t3;

	} else {
		/* data */
		register unsigned int	t0,t1,t2,t3;
		register unsigned short s0;

		s0 = *(unsigned short *)from;
		from = (unsigned int *)(((short*)from) + 1);	/* aligned now */
		from += 4;					/* skipto */
		len -= sizeof(short);

		while (len > 0) {
			t0 = from[0]; t1 = from[1]; t2 = from[2]; t3 = from[3];
			from += 8;/*dma twist!*/
			/* byteorderdep */
			to[0] = s0 | (t0 << 16);
			to[1] = (t0 >> 16) | (t1 << 16);
			to[2] = (t1 >> 16) | (t2 << 16);
			to[3] = (t2 >> 16) | (t3 << 16);
			s0 = t3 >> 16;
			to += 4;
			len -= 4 * sizeof(int);
		}
		*(unsigned short *)to = s0;
	}
}

