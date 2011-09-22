/* 
 * Mach Operating System
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
 * $Log:	intctl.h,v $
 * Revision 2.3  91/07/31  18:01:00  dbg
 * 	Changed copyright.
 * 	[91/07/31            dbg]
 * 
 * Revision 2.2  91/05/08  12:56:21  dbg
 * 	Converted for pure kernel and GCC.
 * 	[91/04/26  14:52:18  dbg]
 * 
 */

/*
 * $Header: intctl.h,v 2.3 91/07/31 18:01:00 dbg Exp $
 */

/*
 * Revision 1.1  89/07/05  13:15:33  kak
 * Initial revision
 * 
 * Revision 2.14  89/02/27  10:40:22  djg
 * increased slic synchronisation timmings to 22Mhz (=12 cylces)
 * 
 * Revision 2.13  88/11/10  08:25:31  djg
 * bak242 
 * 
 */
#ifndef	_SQT_INTCTL_H_
#define	_SQT_INTCTL_H_

#define	TMPOS_GROUP	1		/* SLIC group used by processors */
#define	SLICBINS	8		/* 8 bins in current SLIC */
#define	MSGSPERBIN	256		/* Interrupt vectors per bin */

/*
 * SPL masks for SLIC.
 */

#define SPL0	0xFF			/* all interrupts enabled */
#define	SPL1	0xFE			/* bins 1-7 enabled, 0 disabled */
#define	SPL2	0xFC			/* bins 2-7 enabled, 0-1 disabled */
#define	SPL3	0xF8			/* bins 3-7 enabled, 0-2 disabled */
#define	SPL_HOLE 0xF0			/* bins 4-7 enabled, 0-3 disabled */
#define	SPL4	0xE0			/* bins 5-7 enabled, 0-4 disabled */
#define	SPL5	0xC0			/* bins 6-7 enabled, 0-5 disabled */
#define	SPL6	0x80			/* bin    7 enabled, 0-6 disabled */
#define	SPL7	0x00			/* all interrupts disabled */

/*
 * Mnemonics for various implementation SPL's.
 * SPLFD chosen to insure proper nesting of spl's in ioctl's.
 */

#define	SPLNET	SPL1			/* block network SW interrupts */
#define SPLFD	SPL1			/* file-descriptor manipulation */
#define SPLTTY	SPL4			/* block tty interrupts */
#define	SPLBUF	SPL6			/* buffer-cache: all but clock(s) off */
#define	SPLSWP	SPL6			/* swap-buf headers: similarly */
#define	SPLFS	SPL6			/* inodes-list/file-sys: similarly */
#define	SPLIMP	SPL6			/* network devices, etc */
#define	SPLMEM	SPL6			/* memory list manipulation */
#define	SPLHI	SPL7			/* block all interrupts */

/*
 * Bit positions of Software Interrupts
 */

#define	NETINTR		0x01
#define	SOFTCLOCK	0x10
#define	RESCHED		0x40
#define	PMAPUPDATE	0x80

/*
 * NMI Interrupt messages
 */

#define PAUSESELF	0x01

#ifndef	ASSEMBLER
/*
 * Bin_header structure used for bin 1-7 interrupts.
 * We allocate one for bin0 for convenience, although it isn't used.
 *
 * locore.s assumes this data-structure is 8-bytes big.  If this
 * changes, *MUST* change locore.s (dev_common handler).
 */

struct	bin_header {
	int	bh_size;		/* # entries */
	int	(**bh_hdlrtab)();	/* real interrupt handlers */
};

extern	struct	bin_header int_bin_table[];
extern	int	bin_alloc[];
extern	int	bin_intr[];
extern	unsigned char	ivecall();

/*
 * The following interfaces (ivecres, ivecpeek, and ivecinit) may
 * be used by custom hardware configuration software to ease setting
 * up interrupt handling.
 * 
 * ivecres:	reserve interrupt vector slots.
 * ivecpeek:	peek at the next interrupt vector to be allocated
 * iveninit:	assign an interrupt handler to a vector
 */

#define ivecres(bin, count)	bin_intr[(bin)] += (count)
#define ivecpeek(bin)		bin_alloc[(bin)]
#define ivecinit(bin, vector, handler) \
	int_bin_table[(bin)].bh_hdlrtab[(vector)] = (handler)

/*
 * Typedef for spl mask.
 */
typedef	int	spl_t;

/*
 * In-line spl interfaces -- mask some set of SLIC interrupts, return
 * previous mask.
 *
 * Only low order byte of return value is meaningful; upper 3 bytes are
 * not set and not used by splx().
 *
 * All insure SLIC can't accept an interrupt once the "spl" is complete.
 * Must do a read to synch with the write of new SLIC mask, then pad by
 * at lease 8 cycles.  This insures that a pending SLIC interrupt (or one
 * accepted while the mask is being written) is taken before the spl()
 * completes.  Reading the slic local mask provides the synch and takes
 * enough extra time so 2 nops are enough pad. (at 16Mhz)
 */

#define	splnet	spl1
#define	splimp	spl6
#define	splhi	spl7			/* block all interrupts */

#endif	ASSEMBLER


#ifndef	__GNUC__
#if	!defined(GENASSYM) && !defined(lint)

asm spl0()
{
	movb	SLIC_MASK, %al			/* old interrupt mask */
	movb	$SPL0, SLIC_MASK		/* write new interrupt mask */
}

asm spl1()
{
	movb	SLIC_MASK, %al			/* old interrupt mask */
/PEEPOFF					/* turn off peephole optimizer*/
	movb	$SPL1, SLIC_MASK		/* write new interrupt mask */
	movb	SLIC_MASK, %dl			/* dummy read to synch write */
/***************SLICSYNC 2***************************************/
	nop					/* pad out +3 cycles */
	nop					/* pad out +3 cycles */
#if MHz == 20
	movl	%eax,%eax			/* pad out +2 cycles */
	movl	%eax,%eax			/* pad out +2 cycles */
#endif

/PEEPON						/* turn peephole opt back on */
}

asm spl2()
{
	movb	SLIC_MASK, %al			/* old interrupt mask */
/PEEPOFF					/* turn off peephole optimizer*/
	movb	$SPL2, SLIC_MASK		/* write new interrupt mask */
	movb	SLIC_MASK, %dl			/* dummy read to synch write */
/***************SLICSYNC 2***************************************/
	nop					/* pad out +3 cycles */
	nop					/* pad out +3 cycles */
#if MHz == 20
	movl	%eax,%eax			/* pad out +2 cycles */
	movl	%eax,%eax			/* pad out +2 cycles */
#endif
/PEEPON						/* turn peephole opt back on */
}

asm spl3()
{
	movb	SLIC_MASK, %al			/* old interrupt mask */
/PEEPOFF					/* turn off peephole optimizer*/
	movb	$SPL3, SLIC_MASK		/* write new interrupt mask */
	movb	SLIC_MASK, %dl			/* dummy read to synch write */
/***************SLICSYNC 2***************************************/
	nop					/* pad out +3 cycles */
	nop					/* pad out +3 cycles */
#if MHz == 20
	movl	%eax,%eax			/* pad out +2 cycles */
	movl	%eax,%eax			/* pad out +2 cycles */
#endif
/PEEPON						/* turn peephole opt back on */
}

asm spl4()
{
	movb	SLIC_MASK, %al			/* old interrupt mask */
/PEEPOFF					/* turn off peephole optimizer*/
	movb	$SPL4, SLIC_MASK		/* write new interrupt mask */
	movb	SLIC_MASK, %dl			/* dummy read to synch write */
/***************SLICSYNC 2***************************************/
	nop					/* pad out +3 cycles */
	nop					/* pad out +3 cycles */
#if MHz == 20
	movl	%eax,%eax			/* pad out +2 cycles */
	movl	%eax,%eax			/* pad out +2 cycles */
#endif
/PEEPON						/* turn peephole opt back on */
}

asm spl5()
{
	movb	SLIC_MASK, %al			/* old interrupt mask */
/PEEPOFF					/* turn off peephole optimizer*/
	movb	$SPL5, SLIC_MASK		/* write new interrupt mask */
	movb	SLIC_MASK, %dl			/* dummy read to synch write */
/***************SLICSYNC 2***************************************/
	nop					/* pad out +3 cycles */
	nop					/* pad out +3 cycles */
#if MHz == 20
	movl	%eax,%eax			/* pad out +2 cycles */
	movl	%eax,%eax			/* pad out +2 cycles */
#endif
/PEEPON						/* turn peephole opt back on */
}

asm spl6()
{
	movb	SLIC_MASK, %al			/* old interrupt mask */
/PEEPOFF					/* turn off peephole optimizer*/
	movb	$SPL6, SLIC_MASK		/* write new interrupt mask */
	movb	SLIC_MASK, %dl			/* dummy read to synch write */
/***************SLICSYNC 2***************************************/
	nop					/* pad out +3 cycles */
	nop					/* pad out +3 cycles */
#if MHz == 20
	movl	%eax,%eax			/* pad out +2 cycles */
	movl	%eax,%eax			/* pad out +2 cycles */
#endif
/PEEPON						/* turn peephole opt back on */
}

asm spl7()
{
	movb	SLIC_MASK, %al			/* old interrupt mask */
/PEEPOFF					/* turn off peephole optimizer*/
	movb	$SPL7, SLIC_MASK		/* write new interrupt mask */
	movb	SLIC_MASK, %dl			/* dummy read to synch write */
/***************SLICSYNC 2***************************************/
	nop					/* pad out +3 cycles */
	nop					/* pad out +3 cycles */
#if MHz == 20
	movl	%eax,%eax			/* pad out +2 cycles */
	movl	%eax,%eax			/* pad out +2 cycles */
#endif
/PEEPON						/* turn peephole opt back on */
}

/*
 * splx() lowers interrupt mask to a previous value.  No write-synch or padding
 * necessary since mask is allowing more interrupts, not masking more.
 * DEBUG uses out-of-line version that tests mask is lowering.
 */

#ifndef	DEBUG
asm splx(oldmask)
{
%reg oldmask;
	movl	oldmask, %eax
	movb	%al, SLIC_MASK
%con oldmask;
	movb	oldmask, SLIC_MASK
%mem oldmask;
	movb	oldmask, %al
	movb	%al, SLIC_MASK
}
#endif	DEBUG

#endif	!GENASSYM && !lint
#endif	__GNUC__

#endif	_SQT_INTCTL_H_
