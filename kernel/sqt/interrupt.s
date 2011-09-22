/* 
 * Mach Operating System
 * Copyright (c) 1992,1991 Carnegie Mellon University
 * Copyright (c) 1992,1991 Sequent Computer Systems
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
 * $Log:	interrupt.s,v $
 * Revision 2.4  93/01/14  17:55:57  danner
 * 	Made NMI interrupts call nmi_intr instead of t_res.  Added
 * 	allow_nmi.
 * 	[92/10/25            dbg]
 * 	Made NMI interrupts call nmi_intr instead of t_res.  Added
 * 	allow_nmi.
 * 	[92/10/25            dbg]
 * 
 * Revision 2.3  91/07/31  18:02:03  dbg
 * 	Changed copyright.
 * 
 * 	Call interrupt routine, since we may be on a different stack.
 * 	[91/05/22            dbg]
 * 
 * Revision 2.2  91/05/08  12:56:40  dbg
 * 	Put parentheses around substituted immediate expressions, so
 * 	that they will pass through the GNU preprocessor.
 * 	[91/01/31            dbg]
 * 
 * 	Converted (interrupt handlers only) for pure kernel.
 * 	Use Mach Kernel interrupt sequence, not Dynix.
 * 	[90/05/02            dbg]
 * 
 */

.text

/*
 * interrupt.s
 *	Machine dependent low-level kernel stuff.
 *
 * Mostly interrupt and trap handling.
 *
 * Very machine dependent.  Intel 80386 version.
 */

#include <mach_kdb.h>

#include <assym.s>
#include <i386/asm.h>
#include <i386/eflags.h>
#include <sqt/asm_macros.h>
#include <sqt/intctl.h>

/*
 * Hardware interrupt handlers.
 *
 * These are pointed at thru a combination of the per-processor
 * Interrupt Descriptor Table (IDT) and Global Descriptor Table (GDT).
 *
 * There is one handler per SLIC bin.  Bins 1-7 are handled in a common
 * manner (the HW interrupts).  Bin 0 is special cased for SW interrupts.
 *
 * Bins 1-7 handle as follows:
 *	Save scratch registers.
 *	eax = bin#.
 *	Goto "dev_common".
 *
 * Dev_common:
 *	count device interrupt (except bin 7, used for clocks)
 *	Save entry IPL.
 *	Set up new IPL.
 *	Read vector from SLIC.
 *	Tell SLIC ok to accept another interrupt.
 *	Verify vector as valid.
 *	Call interrupt handler thru int_bin_table[].
 *	If returning to user mode, check for and handle redispatches
 *		(via falling into trap handler (T_SWTCH)).
 *	Else, disable interrupts, restore previous IPL, and return.
 *
 * Handlers are called with vector number as argument.  The bin #
 * information is *NOT* passed to the handler.
 *
 * All interrupts enter via interrupt-gates, thus SW must re-enable
 * interrupts at processor.  The main reason for interrupt-gates instead of
 * trap-gates is that the SLIC still yanks on the interrupt line until SW
 * tells SLIC it has the interrupt; thus if enter with trap-gate, it will
 * re-enter constantly and overflow the stack.  Also, other interrupts can
 * occur (eg, FPA).
 * Bin 0 (SW) interrupts are handled via reading the bin0 message register
 * and looping until we clear it out, calling an appropriate SW interrupt
 * handler for each bit:
 *
 *	Save scratch registers.
 *	spl1().
 *	ldata = SLIC Bin0 message data.
 *	ON processor ints.
 *	loop {
 *		BIT = FFS(ldata).
 *		If no bits set {
 *			OFF processor ints.
 *			spl0().
 *			restore registers.
 *			rett.
 *		}
 *		clear BIT in ldata.
 *		call SW handler(BIT).
 *		ldata |= SLIC Bin0 message data.
 *	}
 *
 * All entries clear "direction" flag, since C environment assumes this.
 *
 * Bin 0 interrupt handler uses an interrupt gate, to turn OFF processor
 * interrupts until ready to accept another bin0 (or higher) interrupt.
 */

/*
 * Kernel segments set up.
 * Interrupt number is in %eax.  Only %ecx, %edx usable.
 */
	.globl	_interrupt		/ entry point from locore
_interrupt:
	subl	$0x20,%eax		/ subtract interrupt base
	jl	_bad_interrupt		/ bad if < 0
	je	_bin0int		/ separate handling for bin 0
	cmpl	$7,%eax			/ bad if
	jg	_bad_interrupt		/ > 7

/*
 * Common handling for bin 1-7 interrupts.  EAX == bin# on entry.
 * Indexing on int_bin_table assumes the bin_header structure is
 * 8-bytes (quad-word).
 */
dev_common:
	movb	VA_SLIC+SL_LMASK, %cl	/ ECX = old SPL.  Save...
	movb	spltab(%eax), %ch	/ new SPL from table...
	movb	%ch, VA_SLIC+SL_LMASK	/	...masks bin and lower.
	pushl	%ecx			/	...below scratch regs on stack.
					/ the low byte is the old spl
					/ the high our new spl.
	movzbl	VA_SLIC+SL_BININT, %ecx	/ ECX = vector # from message data.

/*
 * The cpu will block until the write completes.
 * This will insure that the spl mask has been set.
 * The following write may now take place before or after the "sti".
 * The current SL_LMASK is the stronger of the intended spl and the
 * spl at the time of the interupt. This simplfies the concerns when
 * returning from interrupt later on.
 */

	movb	$0, VA_SLIC+SL_BININT	/ tell SLIC ok for more interrupts.
	sti				/ ok for more interrupts now.
	cld				/ in case intr`d code had it set.
	leal	_int_bin_table(,%eax,8), %edx / EDX -> intr table for this bin.
	pushl	%ecx			/ argument = vector #.
	cmpl	%ecx, BH_SIZE(%edx)	/ valid vector?
	jle	bogusint		/ nope.
	movl	BH_HDLRTAB(%edx), %eax	/ EAX == base of vectors for this bin.
	call	*(%eax,%ecx,4)		/ call handler.
	addl	$4, %esp		/ clear stack.
intdone:
	popl	%ecx			/ Get entry SPL.
	testl	$(EFL_VM), R_EFLAGS(%esp) / returning to V86 mode?
	jne	inturet
	testb	$3, R_CS(%esp)		/ returning to user mode?
	je	intkret			/ no
inturet:
	cli				/ OFF processor interrupts.
	movb	%cl, VA_SLIC+SL_LMASK	/ restore entry SPL.
intret0:
	ret				/ return to interrupt exit

/*
 * Interrupt return to kernel.
 * Since the SLIC mask is now guaranteed to be stronger than the saved
 * spl we no longer need to worry when it takes effect with respect to the
 * the iret.
 */
intkret:
	cli				/ OFF processor interrupts.
/*
 * %cl is the old mask, %ch is the current mask.
 * it is possible that the saved mask was set to a higher value than
 * than we are currently running at. This would happen for example if
 * we had just written to the slic mask and within 500 ns the interrupt
 * that we are returning from occured.
 * In this case we must be careful that we don't hit the iret
 * without insuring that the mask has been set.
 */

	cmpb	%cl,%ch			/ going back to higher SPL?
	je	intret0			/ same level do nothing
	jb	intret1			/ going to higher spl
	movb	%cl, VA_SLIC+SL_LMASK	/ restore entry SPL.
	jmp	intret0			/ return from interrupt
intret1:
	movb	%cl, VA_SLIC+SL_LMASK	/ restore entry SPL.
intret2:
	movb	VA_SLIC+SL_LMASK, %ch
	cmpb	%ch,%cl			/ loop until set
	jne	intret2
	jmp	intret0			/ return from interrupt

/*
 * spltab[]
 *	Maps bin # to IPL value to put in SLIC local-mask register.
 *
 * spltab[i] masks interrupts `i' and lower priority.
 */
	.align	2
spltab:
	.byte	SPL1			/ [0]
	.byte	SPL2			/ [1]
	.byte	SPL3			/ [2]
	.byte	SPL_HOLE		/ [3]
	.byte	SPL4			/ [4]
	.byte	SPL5			/ [5]
	.byte	SPL6			/ [6]
	.byte	SPL7			/ [7]

/*
 * Got bogus interrupt...  Vector # larger than allocated handler table
 * for the bin.  Dev_common already pushed vector #.
 */

	.text
bogusint:
	pushl	%eax			/ bin #
	call	_bogusint		/ complain about this!
	addl	$8, %esp		/ clear junk off stack.
	jmp	intdone			/ return from interrupt.

/*
 * Undefined SW trap handler.
 */
ENTRY(swt_undef)
	pushl	$swtundef		/ panic message
	call	_panic			/ no deposit, no return
	#addl	$4, %esp		/ not really
	#ret				/ not really

	.data
swtundef:
	.asciz	"Undefined software trap"
	.text

/*
 * Bin0 (SW) interrupt handler.  Entered thru interrupt gate, thus
 * interrupts masked at processor.
 *
 * Called routines must *not* redispatch; they must behave as interrupts.
 */
ENTRY(bin0int)
/*
 * was SPL_ASM($SPL1,%al) but to set SPL to mask bin 0.
 * But slic mask may now be greater than spl0 due to synchronisation
 * slippage, so add spl1 to what it currently is.
 */

	movb	VA_SLIC+SL_LMASK, %al
	movb	$(SPL1),%ah		/ store the new mask for int_ret
	movb	%ah, VA_SLIC+SL_LMASK
	pushl	%eax			/ save entry SPL (should be SPL0).
	movzbl	VA_SLIC+SL_B0INT, %ecx	/ ECX = Bin0 message data (mask).
	sti				/ ON processor interrupts.
	cld				/ in case intr`d code had it set.
0:	bsfl	%ecx, %eax		/ Find software trap bit
	je	intdone			/ no bit ==> done.
	btrl	%eax, %ecx		/ clear soft interrupt bit.
	pushl	%ecx			/ save remaining interrupt data.
	call	*_softvec(,%eax,4)	/ call soft interrupt routine.
	popl	%ecx			/ restore interrupt data
	orb	VA_SLIC+SL_B0INT, %cl	/ CL |= Bin0 message data (mask)
	jmp	0b			/ repeat until no bits set.

/*
 * Unconditionally configured SW interrupt handlers.
 */

/*
 * undef()
 *	No such.  Somebody goofed.
 */
ENTRY(undef)
	pushl	$undefmsg		/ panic message
	call	_panic			/ no deposit, no return
	#addl	$4, %esp		/ not really
	#ret				/ not really

	.data
undefmsg:
	.asciz	"Undefined software interrupt"
	.text

/*
 * Interrupt other than for bins 0..7
 */
_bad_interrupt:
	addl	$0x20,%eax		/ add back interrupt base
	cmpl	$2,%eax			/ is it an NMI?
	jne	EXT(t_res)		/ if not, handle as reserved trap
	jmp	EXT(t_nmi)		/ if so, test NMI causes

/*
 * T_NMI -- Non-Maskable Interrupt.  No error code.
 * Entered thru interrupt gate (interrupts disabled).
 *
 * If probe_nmi == NULL, handle as trap (which will panic the system).
 * Else, jump to probe_nmi.  The "jump" is via an iret, to allow NMI's
 * again in the processor (80386 disables NMI's until an iret is executed).
 * The "iret" also removes the NMI stack frame.
 */
#ifdef	KERNEL_PROFILING
	.globl	_kp_nmi
#endif	KERNEL_PROFILING

ENTRY(t_nmi)
	movl	_probe_nmi, %eax	/ probe_nmi procedure, or NULL.
	cmpl	$0, %eax		/ probing?
	jz	t_nmi_real		/ no -- a real NMI
	movl	$_return_to_iret,%edx
	cmpl	(%esp),%edx		/ did we switch stacks?
	je	0f			/ if not:
	movl	%eax, 4*4(%esp)		/ alter return IP to probe_nmi function.
	ret				/ restore regs and jump to
					/ probe-NMI handler.
0:
	movl	4(%esp),%edx		/ point to interrupt reg save area
	movl	%eax,I_EIP(%esp)	/ alter return IP to probe_nmi function.
	ret				/ restore regs and jump to
					/ probe-NMI handler.

	.data
	.globl	_probe_nmi
_probe_nmi:
	.long	0
	.text

/*
 * NMI and no probe routine set.  If kernel profiling configured,
 * do it.
 */
t_nmi_real:
#ifdef	KERNEL_PROFILING
	cld				/ in case trapped code had it set.
	call	_kp_nmi			/ assume profiler NMI
	testl	%eax,%eax		/ was it really?
	jne	0f			/ no -- a real NMI.
	ret				/ yes -- return from trap
0:
#endif	KERNEL_PROFILING

/*
 * Real NMI.  Call C.
 */
	movzbl	VA_SLIC+SL_LMASK,%ecx	/ get previous SPL
	pushl	%ecx			/ push to make stack same as
	pushl	$2			/ normal interrupt stack
	call	_nmi_intr		/ call C
	addl	$8,%esp			/ clean up stack
	ret				/ done

/*
 * Allow further NMI interrupts by executing an IRET.
 */
ENTRY(allow_nmi)
	popl	%eax			/ get return address
	pushfl				/ push flags
	push	%cs			/ push kernel code segment
	pushl	%eax			/ push return address
	iret				/ IRET to caller to enable NMIs

/*
 * T_RES -- Reserved trap entry.  Serious Problem.
 *
 * This is entered via interrupt-gate (interrupts masked at processor).
 * Use "splhi()" to insure interrupts can't be turned on; panic printf's
 * will re-enable processor interrupts due to "v_gate()".
 *
 * This is used in all otherwise unused slots in the IDT.  Thus it catches
 * bogus interrupt vectors from the hardware.
 */
ENTRY(t_res)
	SPL_ASM($(SPLHI),%bl)		/ %bl = splhi()
	sti				/ processor now allows interrupts.
	pushl	%eax			/ push error code
	pushl	$t_res_msg		/ message
	call	_panic			/ panic(msg, int number)
	addl	$8,%esp			/ allow return
	ret
t_res_msg:
	.asciz	"Undefined interrupt %d"

#ifdef	notdef
/*
 * t_fpa -- FPA exception.  This is actually an interrupt.
 *
 * Enter via interrupt gate so interrupts are disabled.
 * Must read FPA process context register (PCR), then mask all exceptions
 * and insure this is sync'd, then finally enable interrupts and call
 * fpa_trap() to do the dirty work.
 */
ENTRY(t_fpa)
	TRAP_ENTER_NOERR(99)
	jmp	trap_common
	/*
	 * Enter much like bin0int.
	 */
	pushl	%eax			# save...
	pushl	%ecx			#	...scratch
	pushl	%edx			#		...registers.
	movw	$(KERNEL_DS), %ax	# establish...
	movw	%ax, %ds		#	...kernel DS
	movw	%ax, %es		#		... kernel ES.
	movb	VA_SLIC+SL_LMASK, %al	# %al = entry SPL.
	movb	$(SPL0), %ah		# for intdone
	pushl	%eax			# save entry SPL.
	/*
	 * Read FPA PCR, then mask all exceptions.
	 */
	movl	VA_FPA+FPA_STCTX, %ecx			# %ecx = FPA PCR
	movl	$(FPA_PCR_EM_ALL), VA_FPA+FPA_LDCTX	# mask all exceptions.
	movl	VA_FPA+FPA_STCTX, %edx			# synch the above write.
	/*
	 * Now can re-enable interrupts and call real FPA trap handler.
	 * Once re-enable processor interrupts, can take SLIC interrupt.
	 * Note that SLIC interrupt goes first if FPA and SLIC arrive
	 * at processor simultaneously.
	 */
	sti				# interrupts ON again.
	pushl	%ecx			# call it with nasty PCR.
	call	_fpa_trap		# poke at process.
	popl	%ecx			# clear stack.
	jmp	intdone			# all done.

#ifdef	KERNEL_PROFILING
kp_trapret:
#endif	KERNEL_PROFILING
	addl	$8, %esp		# clear off traptype and error code
	testb	$(RPL_MASK), SP_CS(%esp) # going back to user mode?
	je	9f			# Nope -- avoid seg-reg fuss.
	cli				# restoring ds, es can`t reenter!
	movw	$(USER_DS), %ax		# restore...
	movw	%ax, %ds		#	...user-mode DS
	movw	%ax, %es		#		...user-mode ES.
9:	popal				# restore interrupted registers.
	iret				# back from whence we came.

#endif	notdef

