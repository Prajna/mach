/* 
 * Mach Operating System
 * Copyright (c) 1993-1990 Carnegie Mellon University
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
 * $Log:	start.s,v $
 * Revision 2.18  93/05/10  17:46:38  rvb
 * 	Use C comments
 * 	[93/05/04  17:17:00  rvb]
 * 
 * Revision 2.17  93/02/04  07:58:02  danner
 * 	Replace lost ps2 asm_startup include.
 * 
 * Revision 2.16  93/01/14  17:29:39  danner
 * 	Boot_info changed.
 * 	[92/12/10  17:43:44  af]
 * 
 * Revision 2.15  92/04/04  11:51:04  rpd
 * 	Changed #-style comments to /-style, for ANSI preprocessors.
 * 
 * Revision 2.14  92/01/03  20:08:53  dbg
 * 	Move symbol table and bootstrap image out of BSS.
 * 	[91/08/02            dbg]
 * 
 * Revision 2.13  91/07/31  17:40:55  dbg
 * 	Add pointers to interrupt stack (for uniprocessor).
 * 	[91/07/30  16:57:13  dbg]
 * 
 * Revision 2.12  91/06/19  11:55:39  rvb
 * 	cputypes.h->platforms.h
 * 	[91/06/12  13:45:27  rvb]
 * 
 * Revision 2.11  91/05/14  16:16:59  mrt
 * 	Correcting copyright
 * 
 * Revision 2.10  91/05/08  12:42:43  dbg
 * 	Put parentheses around substituted immediate expressions, so
 * 	that they will pass through the GNU preprocessor.
 * 
 * 	Moved model-specific code to machine-dependent directories.
 * 	Added startup code for multiple CPUs.
 * 	[91/04/26  14:38:55  dbg]
 * 
 * Revision 2.9  91/02/05  17:14:50  mrt
 * 	Changed to new Mach copyright
 * 	[91/02/01  17:38:15  mrt]
 * 
 * Revision 2.8  90/12/20  16:37:02  jeffreyh
 * 	Changes for __STDC__
 * 	[90/12/07  15:43:21  jeffreyh]
 * 
 *
 * Revision 2.7  90/12/04  14:46:38  jsb
 * 	iPSC2 -> iPSC386; ipsc2_foo -> ipsc_foo;
 * 	changes for merged intel/pmap.{c,h}.
 * 	[90/12/04  11:20:35  jsb]
 * 
 * Revision 2.6  90/11/24  15:14:56  jsb
 * 	Added AT386 conditional around "BIOS/DOS hack".
 * 	[90/11/24  11:44:47  jsb]
 * 
 * Revision 2.5  90/11/05  14:27:51  rpd
 * 	Since we steal pages after esym for page tables, use first_avail
 * 	to record the last page +1 that we stole.
 * 	Tell bios to warm boot on reboot.
 * 	[90/09/05            rvb]
 * 
 * Revision 2.4  90/09/23  17:45:20  jsb
 * 	Added support for iPSC386.
 * 	[90/09/21  16:42:34  jsb]
 * 
 * Revision 2.3  90/08/27  21:58:29  dbg
 * 	Change fix_desc to match new fake_descriptor format.
 * 	[90/07/25            dbg]
 * 
 * Revision 2.2  90/05/03  15:37:40  dbg
 * 	Created.
 * 	[90/02/14            dbg]
 * 
 */

#include <platforms.h>
#include <cpus.h>
#include <mach_kdb.h>

#include <i386/asm.h>
#include <i386/proc_reg.h>
#include <assym.s>

#if	NCPUS > 1

#ifdef	SYMMETRY
#include <sqt/asm_macros.h>
#endif

#endif	NCPUS > 1

/*
 * GAS won't handle an intersegment jump with a relocatable offset.
 */
#define	LJMP(segment,address)	\
	.byte	0xea		;\
	.long	address		;\
	.word	segment


#define	KVTOPHYS	(-KERNELBASE)
#define	KVTOLINEAR	(0)

#define	PA(addr)	(addr)+KVTOPHYS
#define	VA(addr)	(addr)-KVTOPHYS

	.data
/*
 * interrupt and bootup stack for initial processor.
 */
	.align	4
	.globl	_intstack
_intstack:
	.set	., .+4096
	.globl	_eintstack
_eintstack:

#if	NCPUS == 1
	.globl	_int_stack_high		/* all interrupt stacks */
_int_stack_high:			/* must lie below this */
	.long	_eintstack		/* address */

	.globl	_int_stack_top		/* top of interrupt stack */
_int_stack_top:
	.long	_eintstack

#endif

/*
 * Pointers to GDT and IDT.  These contain linear addresses.
 */
	.align	8
	.globl	EXT(gdtptr)
LEXT(gdtptr)
	.word	Times(8,GDTSZ)-1
	.long	EXT(gdt)+KVTOLINEAR

	.align	8
	.globl	EXT(idtptr)
LEXT(idtptr)
	.word	Times(8,IDTSZ)-1
	.long	EXT(idt)+KVTOLINEAR

#if	NCPUS > 1
	.data
	.globl	_start_lock
	.align	2
_start_lock:
	.long	0			/* lock for 'upyet' */
	.globl	_upyet
_upyet:
	.long	0			/* 1 when OK for other processors */
					/* to start */
	.globl	_mp_boot_pde
_mp_boot_pde:
	.long	0
#endif	NCPUS > 1

/*
 * All CPUs start here.
 *
 * Environment:
 *	protected mode, no paging, flat 32-bit address space.
 *	(Code/data/stack segments have base == 0, limit == 4G)
 */
	.text
	.align	2
	.globl	EXT(pstart)
LEXT(pstart)
	mov	$0,%ax			/* fs, gs must be zeroed; */
	mov	%ax,%fs			/* some bootstrappers don`t do this */
	mov	%ax,%gs			/* for us */

#if	NCPUS > 1
	jmp	1f
0:	cmpl	$0,PA(_start_lock)
	jne	0b
1:	movl	$1,%eax
	xchgl	%eax,PA(_start_lock)	/* locked */
	testl	%eax,%eax
	jnz	0b

	cmpl	$0,PA(_upyet)		/* are we first? */
	jne	_slave_start		/* no -- system already up. */
#endif	NCPUS > 1

/*
 * Move symbol table out of the way of BSS.
 *
 * When kernel is loaded, at the start of BSS we have a struct boot_info:
 * _edata:
 *		.long	magic_number
 *		.long	sym_size
 *		.long	boot_size
 *		.long	load_info_size
 * sym_start:
 *		kernel symbols
 *		.align	2
 * boot_start:
 *		bootstrap image
 *		.align	2
 * load_info_start:
 *		bootstrap load information
 *
 * all of which must be moved somewhere else, since it
 * is sitting in the kernel BSS.  In addition, the bootstrap
 * image must be moved to a machine page boundary, so that we get:
 *
 * _edata:
 *		BSS
 * _end:			<-	kern_sym_start (VA)
 *		kernel symbols		.
 *		.align 2		. (kern_sym_size)
 *					.
 *		.align VAX_PAGE_SIZE
 *				<-	boot_start (VA)
 *		bootstrap image
 *				<-	load_info_start (VA)
 *		load information
 *				<-	%ebx (PA)
 *
 */

	lea	PA(_edata),%esi		/* point to symbol size word */
	movl	BI_SYM_SIZE(%esi),%edx	/* get symbol size */

	lea	PA(_end)+NBPG-1(%edx),%edi
					/* point after BSS, add symbol */
					/* size, and round up to */
	andl	$-NBPG,%edi		/* machine page boundary */

	lea	-KVTOPHYS(%edi),%eax	/* save virtual address */
	movl	%eax,PA(_boot_start)	/* of start of bootstrap */
	movl	BI_BOOT_SIZE(%esi),%ecx	/* get size of bootstrap */
	movl	%ecx,PA(_boot_size)	/* save size of bootstrap */
	lea	-KVTOPHYS(%edi,%ecx),%eax
	movl	%eax,PA(_load_info_start)
					/* save virtual address */
					/* of start of loader info */
	movl	BI_LOAD_INFO_SIZE(%esi),%eax	/* get size of loader info */
	movl	%eax,PA(_load_info_size)
					/* save size of loader info */
	addl	%eax,%ecx		/* get total size to move */

	leal	BI_SIZE(%esi,%edx),%esi	/* point to start of boot image - source */

	leal	(%edi,%ecx),%ebx	/* point to new location of */
					/* end of bootstrap - next */
					/* available physical address */

	lea	-4(%esi,%ecx),%esi	/* point to end of src - 4 */
	lea	-4(%edi,%ecx),%edi	/* point to end of dst - 4 */
	shrl	$2,%ecx			/* move by longs */
	std				/* move backwards */
	rep
	movsl				/* move bootstrap and loader_info */

	movl	$_end,PA(_kern_sym_start)
					/* save virtual address */
					/* of start of symbols */
	movl	%edx,PA(_kern_sym_size)	/* save symbol table size */
	testl	%edx,%edx		/* any symbols? */
	jz	0f			/* if so: */

					/* %esi already points to start of boot-4 */
					/* == end of symbol table (source) - 4 */
	leal	PA(_end)-4(%edx),%edi	/* point to end of dst - 4 */
	movl	%edx,%ecx		/* copy size */
	shrl	$2,%ecx			/* move by longs */
	std				/* move backwards */
	rep
	movsl				/* move symbols */
0:
	cld				/* reset direction flag */

/*
 * Get startup parameters.
 */

#ifdef	SYMMETRY
#include <sqt/asm_startup.h>
#endif
#ifdef	AT386
#include <i386at/asm_startup.h>
#endif
#ifdef	iPSC386
#include <i386ipsc/asm_startup.h>
#endif
#ifdef PS2
#include <i386ps2/asm_startup.h>
#endif

/*
 * Build initial page table directory and page tables.
 * %ebx holds first available physical address.
 */

	addl	$(NBPG-1),%ebx		/* round first avail physical addr */
	andl	$(-NBPG),%ebx		/* to machine page size */
	leal	-KVTOPHYS(%ebx),%eax	/* convert to virtual address */
	movl	%eax,PA(_kpde)		/* save as kernel page table directory */
	movl	%ebx,%cr3		/* set physical address in CR3 now */

	movl	%ebx,%edi		/* clear page table directory */
	movl	$(PTES_PER_PAGE),%ecx	/* one page of ptes */
	xorl	%eax,%eax
	cld
	rep
	stosl				/* edi now points to next page */

/*
 * Use next few pages for page tables.
 */
	addl	$(KERNELBASEPDE),%ebx	/* point to pde for kernel base */
	movl	%edi,%esi		/* point to end of current pte page */

/*
 * Enter 1-1 mappings for kernel and for kernel page tables.
 */
	movl	$(INTEL_PTE_KERNEL),%eax /* set up pte prototype */
0:
	cmpl	%esi,%edi		/* at end of pte page? */
	jb	1f			/* if so: */
	movl	%edi,%edx		/*    get pte address (physical) */
	andl	$(-NBPG),%edx		/*    mask out offset in page */
	orl	$(INTEL_PTE_KERNEL),%edx /*   add pte bits */
	movl	%edx,(%ebx)		/*    set pde */
	addl	$4,%ebx			/*    point to next pde */
	movl	%edi,%esi		/*    point to */
	addl	$(NBPG),%esi		/*    end of new pte page */
1:
	movl	%eax,(%edi)		/* set pte */
	addl	$4,%edi			/* advance to next pte */
	addl	$(NBPG),%eax		/* advance to next phys page */
	cmpl	%edi,%eax		/* have we mapped this pte page yet? */
	jb	0b			/* loop if not */

/*
 * Zero rest of last pte page.
 */
	xor	%eax,%eax		/* don`t map yet */
2:	cmpl	%esi,%edi		/* at end of pte page? */
	jae	3f
	movl	%eax,(%edi)		/* zero mapping */
	addl	$4,%edi
	jmp	2b
3:

#if	NCPUS > 1
/*
 * Grab (waste?) another page for a bootstrap page directory
 * for the other CPUs.  We don't want the running CPUs to see
 * addresses 0..3fffff mapped 1-1.
 */
	movl	%edi,PA(_mp_boot_pde)	/* save its physical address */
	movl	$(PTES_PER_PAGE),%ecx	/* and clear it */
	rep
	stosl
#endif	NCPUS > 1
	movl	%edi,PA(_first_avail)	/* save first available phys addr */

/*
 * pmap_bootstrap will enter rest of mappings.
 */

/*
 * Fix initial descriptor tables.
 */
	lea	PA(_idt),%esi		/* fix IDT */
	movl	$(IDTSZ),%ecx
	movl	$(PA(fix_idt_ret)),%ebx
	jmp	fix_desc		/* (cannot use stack) */
fix_idt_ret:

	lea	PA(_gdt),%esi		/* fix GDT */
	movl	$(GDTSZ),%ecx
	movl	$(PA(fix_gdt_ret)),%ebx
	jmp	fix_desc		/* (cannot use stack) */
fix_gdt_ret:

	lea	PA(_ldt),%esi		/* fix LDT */
	movl	$(LDTSZ),%ecx
	movl	$(PA(fix_ldt_ret)),%ebx
	jmp	fix_desc		/* (cannot use stack) */
fix_ldt_ret:

/*
 * Turn on paging.
 */
	movl	%cr3,%eax		/* retrieve kernel PDE phys address */
	movl	KERNELBASEPDE(%eax),%ecx
					/* point to pte for KERNELBASE */
	movl	%ecx,(%eax)		/* set it also as pte for location */
					/* 0..3fffff, so that the code */
					/* that enters paged mode is mapped */
					/* to identical addresses after */
					/* paged mode is enabled */

	movl	$_pag_start,%ebx	/* first paged code address */

	movl	%cr0,%eax
	orl	$(CR0_PG),%eax		/* set PG bit in CR0 */
	movl	%eax,%cr0		/* to enable paging */

	jmp	*%ebx			/* flush prefetch queue */

/*
 * We are now paging, and can run with correct addresses.
 */
_pag_start:
	lgdt	EXT(gdtptr)		/* load GDT */
	lidt	EXT(idtptr)		/* load IDT */
	LJMP(KERNEL_CS,_vstart)		/* switch to kernel code segment */

/*
 * Master is now running with correct addresses.
 */
_vstart:
	mov	$(KERNEL_DS),%ax	/* set kernel data segment */
	mov	%ax,%ds
	mov	%ax,%es
	mov	%ax,%ss
	mov	%ax,EXT(ktss)+TSS_SS0	/* set kernel stack segment */
					/* for traps to kernel */

	movw	$(KERNEL_LDT),%ax	/* get LDT segment */
	lldt	%ax			/* load LDT */
	movw	$(KERNEL_TSS),%ax
	ltr	%ax			/* set up KTSS */

	lea	EXT(eintstack),%esp	/* switch to the bootup stack */
	call	EXT(machine_startup)	/* run C code */
	/*NOTREACHED*/
	hlt

#if	NCPUS > 1
/*
 * We aren't the first.  Call slave_main to initialize the processor
 * and get Mach going on it.
 */
	.align	2
	.globl	EXT(slave_start)
LEXT(slave_start)
	cli				/* disable interrupts, so we don`t */
					/* need IDT for a while */
	movl	PA(_kpde),%ebx		/* get PDE virtual address */
	addl	$(KVTOPHYS),%ebx	/* convert to physical address */

	movl	PA(_mp_boot_pde),%edx	/* point to the bootstrap PDE */
	movl	KERNELBASEPDE(%ebx),%eax
					/* point to pte for KERNELBASE */
	movl	%eax,KERNELBASEPDE(%edx)
					/* set in bootstrap PDE */
	movl	%eax,(%edx)		/* set it also as pte for location */
					/* 0..3fffff, so that the code */
					/* that enters paged mode is mapped */
					/* to identical addresses after */
					/* paged mode is enabled */
	movl	%edx,%cr3		/* use bootstrap PDE to enable paging */

	movl	$_spag_start,%edx	/* first paged code address */

	movl	%cr0,%eax
	orl	$(CR0_PG),%eax		/* set PG bit in CR0 */
	movl	%eax,%cr0		/* to enable paging */

	jmp	*%edx			/* flush prefetch queue. */

/*
 * We are now paging, and can run with correct addresses.
 */
_spag_start:
	movl	%ebx,%cr3		/* switch to the real kernel PDE */

	lgdt	EXT(gdtptr)		/* load GDT */
	lidt	EXT(idtptr)		/* load IDT */
	LJMP(KERNEL_CS,_svstart)	/* switch to kernel code segment */

/*
 * Slave is now running with correct addresses.
 */
_svstart:
	mov	$(KERNEL_DS),%ax	/* set kernel data segment */
	mov	%ax,%ds
	mov	%ax,%es
	mov	%ax,%ss

	CPU_NUMBER(%eax)		/* get CPU number */
	movl	EXT(interrupt_stack)(,%eax,4),%esp
					/* get stack */
	addl	$(INTSTACK_SIZE),%esp	/* point to top */
	xorl	%ebp,%ebp		/* for completeness */

	movl	$0,%ecx			/* unlock start_lock */
	xchgl	%ecx,_start_lock	/* since we are no longer using */
					/* bootstrap stack */

/*
 * switch to the per-cpu descriptor tables
 */
	pushl	%eax			/* pass CPU number */
	call	_mp_desc_init		/* set up local table */
					/* pointer returned in %eax */
	subl	$4,%esp			/* get space to build pseudo-descriptors */
	
	movw	$(GDTSZ*8-1),0(%esp)	/* set GDT size in GDT descriptor */
	lea	MP_GDT+KVTOLINEAR(%eax),%edx
	movl	%edx,2(%esp)		/* point to local GDT (linear address) */
	lgdt	0(%esp)			/* load new GDT */
	
	movw	$(IDTSZ*8-1),0(%esp)	/* set IDT size in IDT descriptor */
	lea	MP_IDT+KVTOLINEAR(%eax),%edx
	movl	%edx,2(%esp)		/* point to local IDT (linear address) */
	lidt	0(%esp)			/* load new IDT */
	
	movw	$(KERNEL_LDT),%ax
	lldt	%ax			/* load new LDT */
	
	movw	$(KERNEL_TSS),%ax
	ltr	%ax			/* load new KTSS */

	call	_slave_main		/* start MACH */
	/*NOTREACHED*/
	hlt
#endif	NCPUS > 1

/*
 * Convert a descriptor from fake to real format.
 *
 * Calls from assembly code:
 * %ebx = return address (physical) CANNOT USE STACK
 * %esi	= descriptor table address (physical)
 * %ecx = number of descriptors
 *
 * Calls from C:
 * 0(%esp) = return address
 * 4(%esp) = descriptor table address (physical)
 * 8(%esp) = number of descriptors
 *
 * Fake descriptor format:
 *	bytes 0..3		base 31..0
 *	bytes 4..5		limit 15..0
 *	byte  6			access byte 2 | limit 19..16
 *	byte  7			access byte 1
 *
 * Real descriptor format:
 *	bytes 0..1		limit 15..0
 *	bytes 2..3		base 15..0
 *	byte  4			base 23..16
 *	byte  5			access byte 1
 *	byte  6			access byte 2 | limit 19..16
 *	byte  7			base 31..24
 *
 * Fake gate format:
 *	bytes 0..3		offset
 *	bytes 4..5		selector
 *	byte  6			word count << 4 (to match fake descriptor)
 *	byte  7			access byte 1
 *
 * Real gate format:
 *	bytes 0..1		offset 15..0
 *	bytes 2..3		selector
 *	byte  4			word count
 *	byte  5			access byte 1
 *	bytes 6..7		offset 31..16
 */
	.globl	EXT(fix_desc)
LEXT(fix_desc)
	pushl	%ebp			/* set up */
	movl	%esp,%ebp		/* stack frame */
	pushl	%esi			/* save registers */
	pushl	%ebx
	movl	B_ARG0,%esi		/* point to first descriptor */
	movl	B_ARG1,%ecx		/* get number of descriptors */
	lea	0f,%ebx			/* get return address */
	jmp	fix_desc		/* call internal routine */
0:	popl	%ebx			/* restore registers */
	popl	%esi
	leave				/* pop stack frame */
	ret				/* return */

fix_desc:
0:
	movw	6(%esi),%dx		/* get access byte */
	movb	%dh,%al
	andb	$0x14,%al
	cmpb	$0x04,%al		/* gate or descriptor? */
	je	1f

/* descriptor */
	movl	0(%esi),%eax		/* get base in eax */
	rol	$16,%eax		/* swap 15..0 with 31..16 */
					/* (15..0 in correct place) */
	movb	%al,%dl			/* combine bits 23..16 with ACC1 */
					/* in dh/dl */
	movb	%ah,7(%esi)		/* store bits 31..24 in correct place */
	movw	4(%esi),%ax		/* move limit bits 0..15 to word 0 */
	movl	%eax,0(%esi)		/* store (bytes 0..3 correct) */
	movw	%dx,4(%esi)		/* store bytes 4..5 */
	jmp	2f

/* gate */
1:
	movw	4(%esi),%ax		/* get selector */
	shrb	$4,%dl			/* shift word count to proper place */
	movw	%dx,4(%esi)		/* store word count / ACC1 */
	movw	2(%esi),%dx		/* get offset 16..31 */
	movw	%dx,6(%esi)		/* store in correct place */
	movw	%ax,2(%esi)		/* store selector in correct place */
2:
	addl	$8,%esi			/* bump to next descriptor */
	loop	0b			/* repeat */
	jmp	*%ebx			/* all done */

/*
 * put arg in kbd leds and spin a while
 * eats eax, ecx, edx
 */
#define	K_RDWR		0x60
#define	K_CMD_LEDS	0xed
#define	K_STATUS	0x64
#define	K_IBUF_FULL	0x02		/* input (to kbd) buffer full */
#define	K_OBUF_FULL	0x01		/* output (from kbd) buffer full */

ENTRY(set_kbd_leds)
	mov	S_ARG0,%cl		/* save led value */
	
0:	inb	$(K_STATUS),%al		/* get kbd status */
	testb	$(K_IBUF_FULL),%al	/* input busy? */
	jne	0b			/* loop until not */
	
	mov	$(K_CMD_LEDS),%al	/* K_CMD_LEDS */
	outb	%al,$(K_RDWR)		/* to kbd */

0:	inb	$(K_STATUS),%al		/* get kbd status */
	testb	$(K_OBUF_FULL),%al	/* output present? */
	je	0b			/* loop if not */

	inb	$(K_RDWR),%al		/* read status (and discard) */

0:	inb	$(K_STATUS),%al		/* get kbd status */
	testb	$(K_IBUF_FULL),%al	/* input busy? */
	jne	0b			/* loop until not */
	
	mov	%cl,%al			/* move led value */
	outb	%al,$(K_RDWR)		/* to kbd */

	movl	$10000000,%ecx		/* spin */
0:	nop
	nop
	loop	0b			/* a while */

	ret

