/* 
 * Mach Operating System
 * Copyright (c) 1991 Carnegie Mellon University
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
 * $Log:	abioscall.s,v $
 * Revision 2.2  93/02/04  07:58:44  danner
 * 	Defined NO_ABIOS_INTERRUPTS here instead of as a config option.
 * 	[93/01/16            chs]
 * 
 * 	Set thread stack descriptor base address in abioscall.
 * 	[92/03/24            dbg@ibm]
 * 
 */

/
/ call_abios( (u_long) 16_bit far ptr, (u_long) 0, request, anchor ptr, 0, 0);
/  the last two are reserved areas to store stuff we need..
/

#define LOCORE
#include <i386/asm.h>
#include "./assym.s"

#define NO_ABIOS_INTERRUPTS 1

/
/ call ABIOS
/
/ check to see if we are running on the interrupt stack
/ change to the appropriate stack selector and fix esp to be an offset
/ into that stack selector.
/ Note: the changing of %esp MUST happen in the instruction following
/ setting of %ss! Touching the SS disables interrupts for one instruction.
.set	RET_SAVE,20
.set	ABIOS_ROUTE,0x14
Entry(abios_call)
	/ Save the return address
	popl	%eax
	mov	%eax,RET_SAVE(%esp)
#ifdef NO_ABIOS_INTERRUPTS
	cli				 / disable interrupts while in ABIOS
#endif NO_ABIOS_INTERRUPTS
	cmpl	_int_stack_high,%esp	/ on an interrupt stack?
        ja      1f

    / running on interrupt stack

	pushl	$(ABIOS_INT_RET<<16)	/ return to abios_int_return
	movl	$(ABIOS_INT_SS),%eax	/ switch to 16-bit stack segment
	movw	%ax,%ss
	subl	$_intstack,%esp		/ change to 16-bit stack pointer
	data16
	ljmp	ABIOS_ROUTE(%esp)       / call abios

	/ running on current thread`s kernel stack
1:
	pushl	$(ABIOS_TH_RET<<16)	/ return to abios_th_return
	movl	_active_stacks,%edx	/ get base of current stack
	movl	%edx,%eax		/ set in descriptor:
	movw	%ax,_gdt+ABIOS_TH_SS+2	/  15..0
	shrl	$16,%eax
	movb	%al,_gdt+ABIOS_TH_SS+4	/  23..16
	movb	%ah,_gdt+ABIOS_TH_SS+7	/  31..24
	movl	$(ABIOS_TH_SS),%eax	/ switch to
	movw	%ax,%ss			/   16-bit stack segment
	subl	%edx,%esp / change to 16-bit pointer
	data16
	ljmp	ABIOS_ROUTE(%esp)       / call abios

/
/ return from ABIOS
/
/ abios_int_return jumps here
int_return_here:
	/ now set up data to restore the stack
	movw	%ds,%ax
	movw	%ax,%ss			/ switch to 32-bit stack segment
	addl	$_intstack,%esp		/ restore 32-bit pointer
#ifdef NO_ABIOS_INTERRUPTS
	sti				/ re-enable interrupts
#endif	NO_ABIOS_INTERRUPTS
	jmp	*RET_SAVE(%esp)		/ return to caller

/ Cannot return to a gate... so we return to a 16-bit code
/ segment, which jumps to the 32-bit flat code segment.
Entry(abios_int_return)
	ljmp	$(KERNEL_CS),$ int_return_here

/ abios_th_return jumps here
th_return_here:
	/ now set up data to restore the stack
	movw	%ds,%ax
	movw	%ax,%ss			/ switch to 32-bit stack segment
	addl	_active_stacks,%esp	/ restore 32-bit pointer
#ifdef NO_ABIOS_INTERRUPTS
	sti				/ re-enable interrupts
#endif	NO_ABIOS_INTERRUPTS
	jmp	*RET_SAVE(%esp)		/ return to caller

/ Cannot return to a gate... so we return to a 16-bit code
/ segment, which jumps to the 32-bit flat code segment.
Entry(abios_th_return)
	ljmp	$(KERNEL_CS),$ th_return_here

#ifdef IDLE_WAIT_TEST
/
/
/
/ Temparary test code to find out who is trashing registers!!
/
/
/
Entry(idle_wait_test)
	push	%ebp	
	movl	%esp,%ebp
	pushal
	mov	8(%ebp),%ebx
	movl	12(%ebp),%edi
	movl	16(%ebp),%esi
	movl	$ 0xaaaaaaaa,%eax
	movl	$ 0xcccccccc,%ecx
	movl	$ 0xdddddddd,%edx
1:
	cmpl	$ 0,(%ebx)
	jnz	2f
	cmpl	$ 0,(%edi)
	jnz	2f
	cmpl	$ 0,(%esi)
	jnz	2f
	cmpl	8(%ebp),%ebx
	jnz	3f
	cmpl	12(%ebp),%edi
	jnz	3f
	cmpl	16(%ebp),%esi
	jnz	3f
	cmpl	$ 0xaaaaaaaa,%eax
	jnz	3f
	cmpl	$ 0xcccccccc,%ecx
	jnz	3f
	cmpl	$ 0xdddddddd,%edx
	jnz	3f
	jmp	1b
2:
	popal
	pop	%ebp
	ret
3:
	int	$ 3
	
#endif IDLE_WAIT_TEST

/*
 * Allocate a free GDT entry.
 * Done in assembler to use lock prefix instead of SPL.
 */
	.data
	.globl	_gdt_hint
_gdt_hint:
	.long	ABIOS_FIRST_AVAIL_SEL
	.text

Entry(allocate_gdt)
	movl	_gdt_hint,%eax		/ start after last used GDT entry
	movl	$(GDTSZ-(ABIOS_FIRST_AVAIL_SEL>>3)),%ecx
					/ set limit on searches
alloc_loop:
	lock
	btsl	$7,_gdt+5(%eax)		/ test-and-set ACC_P (segment present)
	jnc	alloc_found		/ found one if bit wasn`t set
	addl	$8,%eax			/ try next entry
	cmpl	$(GDTSZ<<3),%eax	/ if at end of table,
	jb	0f
	movl	$(ABIOS_FIRST_AVAIL_SEL),%eax
					/ reset to beginning
0:	loop	alloc_loop

	movl	$0xffff,%eax		/ no descriptors available
	ret

alloc_found:				/ found one
	movl	%eax,_gdt_hint		/ set hint
	ret				/ return selector

/*
 * Free a selector.
 */
Entry(free_gdt)
	movl	4(%esp),%eax		/ get selector
	movl	$0,_gdt(%eax)		/ clear first word
	lock				/ clear second word (and present bit)
	andl	$0,_gdt+4(%eax)		/ with locked instruction
	movl	%eax,_gdt_hint		/ set hint
	ret
