/* 
 * Mach Operating System
 * Copyright (c) 1992 Carnegie Mellon University
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
 * Revision 2.5  93/05/20  21:01:41  mrt
 * 	Changed use of zero to ra in call to NESTED.
 * 	[93/05/18            mrt]
 * 
 * Revision 2.4  93/05/17  18:17:03  mrt
 * 	Initialize stack pointer. Used if starting from console
 * 	with start command. New firmware starts with sp set to 0.
 * 	Fix from Michael Uhlenberg.
 * 	[93/05/17            mrt]
 * 
 * Revision 2.3  93/03/09  10:50:53  danner
 * 	GP setup was wrong under GCC.
 * 	[93/03/05            af]
 * 
 * Revision 2.2  93/02/05  07:59:50  danner
 * 	Change all mov calls to or calls due to strange chip bug
 * 	on Flamingo with pass 2 chip
 * 	[93/01/12            jeffreyh]
 * 	Changed ADU-specific way to setup k0seg mappings into
 * 	MI way to do it, using ldqp/stqp pal calls.
 * 	[93/01/15            af]
 * 	Got more memory on the ADU, bump NLEV3 up.
 * 	[92/12/25  01:41:26  af]
 * 
 * 	Added reference to doc for the HWRPB &co.
 * 	[92/12/22            af]
 * 	Added reference to documentation source(s).
 * 	[92/12/16  15:18:08  af]
 * 
 * 	Created.
 * 	[92/06/03            af]
 * 
 */

/*
 *	File: start.s
 * 	Author: Alessandro Forin, Carnegie Mellon University
 *	Date:	6/92
 *
 *	Kernel entry point
 *
 *	This code was derived exclusively from information available in
 *	"Alpha Architecture Reference Manual", Richard L. Sites ed.
 *	Digital Press, Burlington, MA 01803
 *	ISBN 1-55558-098-X, Order no. EY-L520E-DP
 *
 *	"VMS for Alpha Platforms Internals and Data Structures"
 *	Digital Press 1992, Burlington, MA 01803
 *	Order number EY-L466E-P1/2, ISBN 1-55558-095-5
 *	[Especially volume 1, chapter 33 "Bootstrap processing"]
 */
#include <cpus.h>
#include <mach_kdb.h>

#include <mach/alpha/asm.h>
#include <mach/alpha/alpha_instruction.h>
#include <alpha/trap.h>
#include <alpha/alpha_scb.h>
#include <alpha/prom_interface.h>
#include <mach/alpha/vm_param.h>

#include "assym.s"

/*
 *	Object:
 *		alpha_scb			EXPORTED SCB
 *
 *		Exception and interrupt dispatching table.
 *
 * Since the SCB must be 8kpage aligned.. here it goes as well.
 */

	.globl	TRAP_generic
	.globl	TRAP_interrupt
	.globl	stray_trap
	.globl	stray_interrupt

#define	t(code)		.quad TRAP_generic,code
#define	i(code)		.quad TRAP_interrupt,stray_interrupt
#define	u(code)		.quad TRAP_interrupt,stray_trap

	.globl	TRAP_dispatcher

#define	d(handler)	\
	.globl	handler;\
	.quad TRAP_dispatcher,handler

	.text
	.set	noreorder
	.set	noat

	.globl	alpha_scb

alpha_scb:
	/*
	 * On the ADU, this ends up being the kernel entry point.
	 * So we branch to the right place, and later on fix
	 * this first entry.  Which is unused anyways.
	 */
/*000	u(0) */
	nop
	br	zero,start
	.quad	0
/*010*/
	t(T_FPA_DISABLED)
	u(2)
	u(3)
	u(4)
	u(5)
	u(6)
	u(7)
	t(T_PROT_FAULT)
	t(T_TRANS_INVALID)
	t(T_READ_FAULT)
	t(T_WRITE_FAULT)
	t(T_EXECUTE_FAULT)
	u(13)
	u(14)
	u(15)
/*100*/	u(16)
	u(17)
	u(18)
	u(19)
	u(20)
	u(21)
	u(22)
	u(23)
	u(24)
	u(25)
	u(26)
	u(27)
	u(28)
	u(29)
	u(30)
	u(31)
/*200*/	t(T_ARITHMETIC)
	u(33)
	u(34)
	u(35)
	t(T_AST_K)
	t(T_AST_E)
	t(T_AST_S)
	t(T_AST_U)
	t(T_UNALIGNED)
	u(41)
	u(42)
	u(43)
	u(44)
	u(45)
	u(46)
	u(47)
/*300*/	u(48)
	u(49)
	u(50)
	u(51)
	u(52)
	u(53)
	u(54)
	u(55)
	u(56)
	u(57)
	u(58)
	u(59)
	u(60)
	u(61)
	u(62)
	u(63)
#if	MACH_KDB
/*400*/	d(kdb_breakpoint)
#else
/*400*/	t(T_BP)
#endif
	t(T_BUG)
	t(T_ILL)
	t(T_PAL)
	u(68)		/* generate software trap ?? */
	u(69)
	u(70)
	u(71)
	t(T_CHMK)
	t(T_CHME)
	t(T_CHMS)
	t(T_CHMU)
	u(76)
	u(77)
	u(78)
	u(79)
/*500*/	u(80)
	i( /* Software interrupt level 1, entry # */ SCB_SOFTCLOCK )
	i( /* Software interrupt level 2, entry # */ 82 )
	i( /* Software interrupt level 3, entry # */ 83 )
	i( /* Software interrupt level 4, entry # */ 84 )
	i( /* Software interrupt level 5, entry # */ 85 )
	i( /* Software interrupt level 6, entry # */ 86 )
	i( /* Software interrupt level 7, entry # */ 87 )
	i( /* Software interrupt level 8, entry # */ 88 )
	i( /* Software interrupt level 9, entry # */ 89 )
	i( /* Software interrupt level 10, entry # */ 90 )
	i( /* Software interrupt level 11, entry # */ 91 )
	i( /* Software interrupt level 12, entry # */ 92 )
	i( /* Software interrupt level 13, entry # */ 93 )
	i( /* Software interrupt level 14, entry # */ 94 )
	i( /* Software interrupt level 15, entry # */ 95 )
/*600*/	i( /* Interval clock interrupt */ SCB_CLOCK )
	i( /* Interprocessor interrupt */ SCB_INTERPROC )
	t(T_SCE)
	t(T_PCE)
	t(T_PFAIL)
	u(101)			/* perfmon ?? */
	t(T_SCHECK)
	t(T_PCHECK)
	u(104)
	u(105)
	u(106)
	u(107)
	u(108)
	u(109)
	u(110)
	u(111)			/* passive release ?? */
/*700*/	u(112)
	u(113)
	u(114)
	u(115)
	u(116)
	u(117)
	u(118)
	u(119)
	u(120)
	u(121)
	u(122)
	u(123)
	u(124)
	u(125)
	u(126)
	u(127)
/*800*/	i(0)
	i(1)
	i(2)
	i(3)
	i(4)
	i(5)
	i(6)
	i(7)
	i(8)
	i(9)
	i(10)
	i(11)
	i(12)
	i(13)
	i(14)
	i(15)
/*900*/	i(16)
	i(17)
	i(18)
	i(19)
	i(20)
	i(21)
	i(22)
	i(23)
	i(24)
	i(25)
	i(26)
	i(27)
	i(28)
	i(29)
	i(30)
	i(31)
/*a00*/	i(32)
	i(33)
	i(34)
	i(35)
	i(36)
	i(37)
	i(38)
	i(39)
	i(40)
	i(41)
	i(42)
	i(43)
	i(44)
	i(45)
	i(46)
	i(47)
/*b00*/	i(48)
	i(49)
	i(50)
	i(51)
	i(52)
	i(53)
	i(54)
	i(55)
	i(56)
	i(57)
	i(58)
	i(59)
	i(60)
	i(61)
	i(62)
	i(63)
/*c00*/	i(64)
	i(65)
	i(66)
	i(67)
	i(68)
	i(69)
	i(70)
	i(71)
	i(72)
	i(73)
	i(74)
	i(75)
	i(76)
	i(77)
	i(78)
	i(79)
/*d00*/	i(80)
	i(81)
	i(82)
	i(83)
	i(84)
	i(85)
	i(86)
	i(87)
	i(88)
	i(89)
	i(90)
	i(91)
	i(92)
	i(93)
	i(94)
	i(95)
/*e00*/	i(96)
	i(97)
	i(98)
	i(99)
	i(100)
	i(101)
	i(102)
	i(103)
	i(104)
	i(105)
	i(106)
	i(107)
	i(108)
	i(109)
	i(110)
	i(111)
/*f00*/	i(112)
	i(113)
	i(114)
	i(115)
	i(116)
	i(117)
	i(118)
	i(119)
	i(120)
	i(121)
	i(122)
	i(123)
	i(124)
	i(125)
	i(126)
	i(127)
/*1000*/ i(128)
	i(129)
	i(130)
	i(131)
	i(132)
	i(133)
	i(134)
	i(135)
	i(136)
	i(137)
	i(138)
	i(139)
	i(140)
	i(141)
	i(142)
	i(143)
/*1100*/ i(144)
	i(145)
	i(146)
	i(147)
	i(148)
	i(149)
	i(150)
	i(151)
	i(152)
	i(153)
	i(154)
	i(155)
	i(156)
	i(157)
	i(158)
	i(159)
/*1200*/ i(160)
	i(161)
	i(162)
	i(163)
	i(164)
	i(165)
	i(166)
	i(167)
	i(168)
	i(169)
	i(170)
	i(171)
	i(172)
	i(173)
	i(174)
	i(175)
/*1300*/ i(176)
	i(177)
	i(178)
	i(179)
	i(180)
	i(181)
	i(182)
	i(183)
	i(184)
	i(185)
	i(186)
	i(187)
	i(188)
	i(189)
	i(190)
	i(191)
/*1400*/ i(192)
	i(193)
	i(194)
	i(195)
	i(196)
	i(197)
	i(198)
	i(199)
	i(200)
	i(201)
	i(202)
	i(203)
	i(204)
	i(205)
	i(206)
	i(207)
/*1500*/ i(208)
	i(209)
	i(210)
	i(211)
	i(212)
	i(213)
	i(214)
	i(215)
	i(216)
	i(217)
	i(218)
	i(219)
	i(220)
	i(221)
	i(222)
	i(223)
/*1600*/ i(224)
	i(225)
	i(226)
	i(227)
	i(228)
	i(229)
	i(230)
	i(231)
	i(232)
	i(233)
	i(234)
	i(235)
	i(236)
	i(237)
	i(238)
	i(239)
/*1700*/ i(240)
	i(241)
	i(242)
	i(243)
	i(244)
	i(245)
	i(246)
	i(247)
	i(248)
	i(249)
	i(250)
	i(251)
	i(252)
	i(253)
	i(254)
	i(255)
/*1800*/ i(256)
	i(257)
	i(258)
	i(259)
	i(260)
	i(261)
	i(262)
	i(263)
	i(264)
	i(265)
	i(266)
	i(267)
	i(268)
	i(269)
	i(270)
	i(271)
/*1900*/ i(272)
	i(273)
	i(274)
	i(275)
	i(276)
	i(277)
	i(278)
	i(279)
	i(280)
	i(281)
	i(282)
	i(283)
	i(284)
	i(285)
	i(286)
	i(287)
/*1a00*/ i(288)
	i(289)
	i(290)
	i(291)
	i(292)
	i(293)
	i(294)
	i(295)
	i(296)
	i(297)
	i(298)
	i(299)
	i(300)
	i(301)
	i(302)
	i(303)
/*1b00*/ i(304)
	i(305)
	i(306)
	i(307)
	i(308)
	i(309)
	i(310)
	i(311)
	i(312)
	i(313)
	i(314)
	i(315)
	i(316)
	i(317)
	i(318)
	i(319)
/*1c00*/ i(320)
	i(321)
	i(322)
	i(323)
	i(324)
	i(325)
	i(326)
	i(327)
	i(328)
	i(329)
	i(330)
	i(331)
	i(332)
	i(333)
	i(334)
	i(335)
/*1d00*/ i(336)
	i(337)
	i(338)
	i(339)
	i(340)
	i(341)
	i(342)
	i(343)
	i(344)
	i(345)
	i(346)
	i(347)
	i(348)
	i(349)
	i(350)
	i(351)
/*1e00*/ i(352)
	i(353)
	i(354)
	i(355)
	i(356)
	i(357)
	i(358)
	i(359)
	i(360)
	i(361)
	i(362)
	i(363)
	i(364)
	i(365)
	i(366)
	i(367)
/*1f00*/ i(368)
	i(369)
	i(370)
	i(371)
	i(372)
	i(373)
	i(374)
	i(375)
	i(376)
	i(377)
	i(378)
	i(379)
	i(380)
	i(381)
	i(382)
	i(383)
EXPORT(end_alpha_scb)

/*
 *	Object:
 *		root_kpdes			EXPORTED PTEs
 *
 *		Kernel's root pagetable (seg1)
 *
 * This also needs aligned.
 */

		.globl	root_kpdes
root_kpdes:
		.space	ALPHA_PGBYTES

/*
 *	Object:
 *		spage_ptes			LOCAL PTEs
 *
 *		Superpage mappings
 *
 * This also needs aligned.
 */

spage_ptes_2:
		.space	ALPHA_PGBYTES		/* lev 2 */
spage_ptes_3:
		/* enough for 256 meg */
#define	NLEV3 32
		.space  32*ALPHA_PGBYTES
/* 
 *	Should use NLEV3 but the gcc preprocessor puts in a space
 *	after NLEV that the gcc as can't handle 
 *
 *	.space	NLEV3*ALPHA_PGBYTES		lev 3 
 */

/*
 *	Object:
 *		boot_pcb			EXPORTED PCB
 *
 *		Initial HW pcb structure.
 *
 * This also needs aligned.
 */

		.globl	boot_pcb
boot_pcb:
		.space	512



	.text
/*
 *	Object:
 *		start				EXPORTED function
 *
 *		Kernel start
 *
 *	Arguments:
 *		first_free_pfn			unsigned long
 *
 */
NESTED(start,1,0,ra,0,0)

	/*
	 * Setup gp pointer
	 */
	br	pv,1f
1:
	ldgp	gp,0(pv)
#if	__GNU_AS__
	setgp	0(pv)
#endif
	lda     sp,0x20010000   /* for the moment ? */


#if	(NCPUS > 1)
	br	s5,1f
	.globl	processors_running
processors_running:		/* s5 points here */
	.space	(NCPUS*4)
1:

	/* Decide here who the primary is, cuz that is
	   the only one that can talk to the console */

	call_pal op_mfpr_whami	
	s4addq	v0,s5,s5
	addq	zero,1,a0			/* proc #0 --> 1 */
	stl	a0,0(s5)

	lda	a0,RESTART_ADDR		/* HWRPB */
	ldq	s4,HWRPB_PRIMARY_ID(a0)
	subq	v0,s4,s4
	beq	s4,1f

	/* secondary spins here a bit, to let primary set pagetables */
	ldah	t0,0x7ff(zero)
aa:	lda	t0,-1(t0)
	bne	t0,aa
	br	zero,all_cpus
1:
#endif	/* NCPUS > 1 */

	/*
	 *  Make sure the superpage is working for us
	 */

	/* This is needed on the ADU with EV4-pass1 chips,
	   no superpage and VMS pal does not emulate it.
	   So we setup the 1:1 mappings of the superpage */

	/* get PT base */
	call_pal op_mfpr_ptbr 
	sll	v0,13,s1		/* PFN -> phys */

	/* find PFN of spage_ptes_2 */
	lda	t0,spage_ptes_2		/* assumes GP ok */
	zap	t0,0xe0,s2		/* k0seg -> phys */
	srl	s2,13,t0		/* phys -> PFN */

	/* build a pte */
	lda	t1,0x1111(zero)		/* protections */
	sll	t0,32,t0		/* shift PFN into place */
	addq	t0,t1,a1		/* pte complete */
	lda	a0,(8*0x200)(s1)	/* pdenum(fffffc00..00) */
	call_pal op_stqp	/* 0x4 */
	/* mapped lev2 page, now on to lev3 */

	/* find PFN of spage_ptes_3 */
	lda	t0,spage_ptes_3
	zap	t0,0xe0,s3		/* k0seg -> phys */
	srl	s3,13,t0		/* phys -> PFN */

	/* now fill in lev2 page */
	lda	t3,NLEV3(zero)
	or	s2,zero,a0		/* lev2`s phys */
flv2:
	sll	t0,32,a1
	addq	a1,t1,a1		/* pte built */
	call_pal op_stqp		/* stick it in */

	subq	t3,1,t3			/* one less to go */
	addq	a0,8,a0			/* next ppte */
	addq	t0,1,t0			/* next PFN */
	bne	t3,flv2			/* mapped all lev2 pages ? */

	/* now fill in lev3 pages */
	lda	t3,NLEV3(zero)
	sll	t3,10,t3		/* nptes to fill */
	lda	t0,0(zero)		/* PFN==0 */
	or	s3,zero,a0		/* lev3`s phys */
flv3:
	sll	t0,32,a1
	addq	a1,t1,a1		/* pte built */
	call_pal op_stqp

	subq	t3,1,t3			/* done this one */
	addq	a0,8,a0			/* ppte++ */
	addq	t0,1,t0			/* PFN++ */
	bne	t3,flv3			/* mapped all lev3 pages ? */

	/* Done mapping 1:1
	   Now, since we'll be using root_kpdes as
	   our lev1, copy entries over in anticipation
	   of context-switching soon afterwards */

	lda	t0,root_kpdes
	lda	t2,ALPHA_PGBYTES(zero)
	or	s1,zero,a0
cp:
	call_pal op_ldqp	/* 0x3 */
	stq	v0,0(t0)		/* use kseg0 */
	subq	t2,8,t2			/* one entry copied */
	addq	t0,8,t0			/* to_ptep++ */
	addq	a0,8,a0			/* from_ptep++ */
	bne	t2,cp

	/*
	 * Get HWRPB address in k0seg, to be indep of the
	 * mappings the console has setup for us. which
	 * we will get rid of pretty soon (cuz useg)
	 */
	IMPORT(alpha_hwrpb,8)

	lda	a0,RESTART_ADDR
	ldq	a0,0(a0)
	lda	t0,alpha_hwrpb
	stq	a0,0(t0)		/* physical! */

all_cpus:

#if 0 /* debug */
	ldl	a0,0(s5)
	addq	a0,1,a0
	stl	a0,0(s5)
#endif

	/*
	 * Disable FPA (just in case)
	 */
	or	zero,zero,a0
#if 0
	/* COMPILER USES FLOATS FOR VARARGS by default. CAVEAT */
	addq	zero,1,a0
#endif
	call_pal op_mtpr_fen

	/*
	 *  Let processor know where the SCB is
	 */
	lda	a0,alpha_scb
				/* map to physical .. */
	zapnot	a0,0xf,a0
	srl	a0,13,a0	/* .. pfn */
	call_pal op_mtpr_scbb

#if	(NCPUS > 1)
	beq	s4,1f

	CALL(alpha_slave_start)
	/* NOTREACHED */
	call_pal op_halt

1:
#endif

	/*
	 * 	Hop onto boot stack
	 */
	lda	sp,bootstack

	/* Let debugger know if we (might) have a non-coff symtab */
#if	__GNU_AS__
	addq	zero,1,a0
#else
	or	zero,zero,a0
#endif

	CALL(alpha_init)

	/* NOTREACHED */
	call_pal op_halt

	END(start)

#if	(NCPUS > 1)
	.data
	.globl slave_init_lock
slave_init_lock:	.long 1
	.text
	.align 4

/* In assembly because no stack! */
LEAF(alpha_slave_start,0)
	ldgp	gp,0(pv)
	or	pv,zero,sp		/* sanity */
	lda	a0,slave_init_lock
1:	ldq	a1,0(a0)
	bne	a1,1b
	ldq_l	a1,0(a0)
	addq	zero,1,a2
	stq_c	a2,0(a0)
	beq	a2,1b
	/*
	 *	Hop onto boot stack, and go
	 */
	lda	sp,bootstack
#if 1 /* DEBUG */
	call_pal op_mfpr_whami	
	stq v0,-8(sp)
	lda sp,-8(sp)
#endif

	CALL(alpha_slave_init)
	/* NOTREACHED */
	call_pal op_halt

	END(alpha_slave_start)

#endif	/* NCPUS > 1 */

/*
 *	Object:
 *		bootstack			LOCAL stack
 *
 *		Initial boot stack
 *
 */

bootstack_end:
		.align	4
		.space	2*8192
bootstack:	.quad	0


/*
 * Compilerfix: BL6 does not get edata right if no .sdata
 */

	.sdata
	.long 0
