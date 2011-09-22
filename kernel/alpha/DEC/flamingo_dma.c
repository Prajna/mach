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
 * CARNEGIE MELLON ALLOWS FREE USE OF THIS SOFTWARE IN ITS AS-IS
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
 * $Log:	flamingo_dma.c,v $
 * Revision 2.5  93/05/15  20:08:58  mrt
 * 	machparam.h machspl.h
 * 
 * Revision 2.4  93/05/10  22:16:30  rvb
 * 	Types.
 * 	[93/05/07  14:43:57  af]
 * 
 * Revision 2.3  93/05/10  20:07:01  rvb
 * 	Types.
 * 	[93/05/07  14:43:57  af]
 * 
 * Revision 2.2  93/03/09  10:48:22  danner
 * 	Created.
 * 	[92/11/18            jeffreyh]
 * 
 */
/*
 *	File: flamingo_dma.c
 * 	Author: Jeffrey Heller, Kubota Pacific Computers
 *	Date:	1/93
 *
 *	Flamingo DMA functions, to handle the TCDS chip.
 *
 */

#include <mach/std_types.h>
#include <chips/busses.h>
#include <machine/machspl.h>		/* spl definitions */
#include <device/io_req.h>
#include <device/conf.h>

#include <alpha/alpha_cpu.h>
#include <alpha/DEC/flamingo.h>

extern vm_offset_t	kvtophys( vm_offset_t );
extern int scsi_debug;

#define NASIC	1	/* XXX */
#define KN15AA_IOSLOT_SCSI 6   /* XXX */

#define	LANCE_DECODE		PHYS_TO_K0SEG(KN15AA_REG_LANCE_DECODE)
/*#define	SCSI_DECODE		PHYS_TO_K0SEG(KN15AA_REG_SCSI_DECODE)*/
#define	SCC0_DECODE		PHYS_TO_K0SEG(KN15AA_REG_SCC0_DECODE)
#define	SCC1_DECODE		PHYS_TO_K0SEG(KN15AA_REG_SCC1_DECODE)


#define	CSR			PHYS_TO_K0SEG(KN15AA_REG_CSR)
#define	INTR			PHYS_TO_K0SEG(KN15AA_REG_INTR)

#define	LANCE_DMAPTR		PHYS_TO_K0SEG(KN15AA_REG_LANCE_DMAPTR)

#define SCSI_CIR		PHYS_TO_K0SEG(KN15AA_REG_SCSI_CIR)
#define SCSI_IMER		PHYS_TO_K0SEG(KN15AA_REG_SCSI_IMER)

#define	DMAR_READ		SCSI_DIC_DMADIR	/* from SCSI to memory */
#define	DMA_ADDR(p)		(((unsigned)p) >> 2)

#ifdef mips
#define	SCSI_DMAPTR		PHYS_TO_K0SEG(KN15AA_REG_SCSI_DMAPTR)
#define	SCSI_NEXT_DMAPTR	PHYS_TO_K0SEG(KN15AA_REG_SCSI_DMANPTR)
#define	SCSI_SCR		PHYS_TO_K0SEG(KN15AA_REG_SCSI_SCR)


#endif
/*
 * Definition of the driver for the auto-configuration program.
 */

int	asic_probe(), asic_intr();
static void asic_attach();

vm_offset_t	asic_std[NASIC] = { 0 };
struct	bus_device *asic_info[NASIC];
struct	bus_driver asic_driver = 
        { asic_probe, 0, asic_attach, 0, asic_std, "asic", asic_info,};

/*
 * Adapt/Probe/Attach functions
 */
asic_probe( xxx, ui)
	struct bus_device *ui;
{
	/* we are only called if hand-filled in */
	return 1;
}

static void
asic_attach(ui)
	register struct bus_device *ui;
{
	/* we got nothing to say */
}

/* wrong */
asic_init(isa_3min)
	boolean_t	isa_3min;
{
	volatile unsigned int	*ssr, *decoder;

	decoder = (volatile unsigned int *)LANCE_DECODE;
	*decoder = KN15AA_LANCE_CONFIG;
	wbflush();
	decoder = (volatile unsigned int *)SCC0_DECODE;
	*decoder = KN15AA_SCC0_CONFIG;
	wbflush();
	ssr = (volatile unsigned int *)CSR;

	decoder = (volatile unsigned int *)SCC1_DECODE;
	*decoder = KN15AA_SCC1_CONFIG;
	wbflush();
	/* take all chips out of reset now */
	*ssr = 0x00000f00;
	wbflush();
}

/* works */
asic_enable_lance(phys)
	vm_offset_t	phys;
{
	volatile unsigned int	*ssr, *ldp;

	ssr = (volatile unsigned int *)CSR;
	ldp = (volatile unsigned int *)LANCE_DMAPTR;

	*ldp = (phys << 3) & 0xfff00000;

	*ssr |= ASIC_CSR_DMAEN_LANCE;
}

/* no idea */
asic_intr(unit, intr)
{
	if (intr & ASIC_INTR_LANCE_READ_E) {
		/* Just clear it and let the lance driver handle */
		*(volatile unsigned int *)INTR = ~ASIC_INTR_LANCE_READ_E;
	} 

		gimmeabreak();

}

/*
 * 	Interrupt routine for the tcds chip
 */
int tcds_delay = 0;
int intr_debug = 0;
tcds_intr()
{
	volatile unsigned int	*cir;
	register unsigned int    cirv;
	unsigned int	junk;



	cir = (volatile unsigned int *)SCSI_CIR;
	wbflush();
	cirv = *cir;
	if (intr_debug) printf("tcds_intr: cir = %x\n",*cir);
	/* Clear now all intr that we will handle here */
	*cir = cirv & 0xffff;
	wbflush();
	/* From 7.3 if FMM 5.0 */
	junk = 	(*(volatile unsigned int *)PHYS_TO_K0SEG(0x1f0080220));
	wbflush();
	

	if (cirv & SCSI_CIR_53C94_INT0){
		asc_intr(0, 0);
	}

	if (cirv & SCSI_CIR_53C94_INT1){
		asc_intr(1, 0);
	}

	if (cirv & (SCSI_CIR_PREF0|SCSI_CIR_PREF1)){
		printf("tcds_intr: ?? Prefetch intr, might need to do something \n");
	}
	
	if (cirv & SCSI_CIR_ERROR) {
		printf("tcds_intr: Error not handled cir = %lx\n",cirv);
	}

	if (tcds_delay)
		delay(1); /* This is wrong, but machine keeps dying */
}

tcds_init()
{
	volatile unsigned int	*imer, *cir, *dic;

	/* SCSI init */
	imer = (volatile unsigned int *)SCSI_IMER;
	*imer = ((SCSI_CIR_53C94_INT0| SCSI_CIR_53C94_INT1) | 
		(((SCSI_CIR_53C94_INT0| SCSI_CIR_53C94_INT1) >> 16)));
	wbflush();
	cir = (volatile unsigned int *)SCSI_CIR;
	*cir = SCSI_CIR_RESET0|SCSI_CIR_RESET1;
	wbflush();
	kn15aa_set_ioslot(KN15AA_IOSLOT_SCSI, IOSLOT_SGDMA, TRUE);
}




/*
 *	Functions to support DMA for the on-board SCSI
 */

#include <scsi/compat_30.h>
#include <scsi/scsi.h>
#include <scsi/scsi_defs.h>
#include <scsi/adapters/scsi_dma.h>
#include <scsi/adapters/scsi_53C94.h>

/* utilities */

#define	u_min(a,b)	(((a) < (b)) ? (a) : (b))
#define NDMA 2 /* The tcds only can have 2 */

struct dma_softc {
	int		dev_unit;
	volatile unsigned int	*sda; /* The scsi DMA address register */
	volatile unsigned int	*dic; /* The scsi interrupt control register */
	volatile unsigned int	*dudb; /* The DMA unaligned data at begining */
	volatile unsigned int	*dude; /* The DMA unaligned data at end*/
	unsigned int		dma_ena; /* CIR bit to turn on DMA */
	vm_size_t	xfer_count;

} kn15aa_softc_data[NDMA];

typedef struct dma_softc *dma_softc_t;

dma_softc_t	kn15aa_softc[NDMA];


kn15aa_dma_read(dma_ptr, count, dma, tgt)
	vm_offset_t	dma_ptr;
	dma_softc_t	dma;
	target_info_t	*tgt;
{
	register volatile unsigned int	*cir;
	unsigned int	sgaddr;


	cir = (volatile unsigned int *)SCSI_CIR;

	/* stop DMA engine first */
	*cir &= ~(dma->dma_ena);
	wbflush();
	/* Grab a map if we do not already have one*/
	if (!tgt->transient_state.copy_count)
		tgt->transient_state.copy_count = sgmap_grab_entry(); 
	sgaddr = sgmap_load_map( dma_ptr, count, tgt->transient_state.copy_count);

	dma->xfer_count = count;

	*(dma->sda) = DMA_ADDR(sgaddr);
	wbflush();
	*(dma->dic) = ((sgaddr & SCSI_DIC_ADDR_MASK) | DMAR_READ);
	wbflush();
	*cir |= dma->dma_ena;
	wbflush();
	if (scsi_debug > 1)	{
		printf ("kn15aa_dma_read: cir = %x sda =%x, dic = %x \n",
			*cir, *(dma->sda), *(dma->dic));	
	}

}

kn15aa_dma_write(dma_ptr, count, dma, tgt)
	vm_offset_t	dma_ptr;
	dma_softc_t	dma;
	target_info_t	*tgt;
{

	register volatile unsigned int	*cir;
	unsigned int	sgaddr;

	cir = (volatile unsigned int *)SCSI_CIR;

	/* stop DMA engine first */
	*cir &= ~(dma->dma_ena);
	wbflush();
	/* Grab a map if we do not already have one*/
	if (!tgt->transient_state.copy_count)
		tgt->transient_state.copy_count = sgmap_grab_entry(); 
	sgaddr = sgmap_load_map( dma_ptr, count, tgt->transient_state.copy_count);

	dma->xfer_count = count;

	*(dma->sda) = DMA_ADDR(sgaddr);
	wbflush();
	*(dma->dic) = ((sgaddr & SCSI_DIC_ADDR_MASK) & ~DMAR_READ);
	wbflush();
	*cir |= dma->dma_ena;
	wbflush();
	if (scsi_debug > 1) {
		printf ("kn15aa_dma_write: cir = %x sda =%x, dic = %x \n",
			*cir, *(dma->sda), *(dma->dic));	
	}

}

int kn15aa_dma_errcnt, kn15aa_dma_ldcnt;


/*
 * Cold, probe time init
 */
opaque_t
kn15aa_dma_init(dev_unit, base, dma_bsizep)
	int		dev_unit;
	vm_offset_t	base;
	int		*dma_bsizep;
{
	static int 	unit = 0;
	dma_softc_t	dma;
	if (unit > 1) panic("kn15aa_dma_init: Unit 2???? \n");

	dma = kn15aa_softc[unit] = &kn15aa_softc_data[unit];
	dma->dev_unit = dev_unit;
	if (unit == 0) {
		dma->sda = (volatile unsigned int *)PHYS_TO_K0SEG(KN15AA_REG_SCSI_DMAPTR0);
		dma->dic = (volatile unsigned int *)PHYS_TO_K0SEG(KN15AA_REG_SCSI_DIC0);
		dma->dudb = (volatile unsigned int *)PHYS_TO_K0SEG(KN15AA_REG_SCSI_DUDB0);
		dma->dude = (volatile unsigned int *)PHYS_TO_K0SEG(KN15AA_REG_SCSI_DUDE0);
		dma->dma_ena = SCSI_CIR_DMAENA0;
	} else {
		dma->sda = (volatile unsigned int *)PHYS_TO_K0SEG(KN15AA_REG_SCSI_DMAPTR1);
		dma->dic = (volatile unsigned int *)PHYS_TO_K0SEG(KN15AA_REG_SCSI_DIC1);
		dma->dudb = (volatile unsigned int *)PHYS_TO_K0SEG(KN15AA_REG_SCSI_DUDB1);
		dma->dude = (volatile unsigned int *)PHYS_TO_K0SEG(KN15AA_REG_SCSI_DUDE1);
		dma->dma_ena = SCSI_CIR_DMAENA1;
	}
	unit++;
	return (opaque_t) dma;
}

/*
 * A target exists
 */
void
kn15aa_dma_new_target(dma, tgt)
	opaque_t	dma;
	target_info_t	*tgt;
{
	extern vm_offset_t	vm_page_grab_phys_addr();
	vm_offset_t		buf;

	buf = vm_page_grab_phys_addr();
	if (buf == -1) panic("kn15aa_dma_new_target");

	tgt->cmd_ptr = (char*)PHYS_TO_K0SEG(buf);
	tgt->dma_ptr = 0;

#ifdef	MACH_KERNEL
#else	/*MACH_KERNEL*/
	fdma_init(&tgt->fdma, scsi_per_target_virtual);
#endif	/*MACH_KERNEL*/

}

/*
 * Map virtual addresses onto physical ones
 * (and/or user virtuals on 2.5)
 */
void
kn15aa_dma_map(dma, tgt)
	opaque_t	dma;
	target_info_t	*tgt;
{

	/*
	 * We do real DMA.
	 */
#ifdef	MACH_KERNEL
#else	/*MACH_KERNEL*/
	if (tgt->ior)
		fdma_map(&tgt->fdma, tgt->ior);
#endif	/*MACH_KERNEL*/

	if ((tgt->cur_cmd == SCSI_CMD_WRITE) ||
	    (tgt->cur_cmd == SCSI_CMD_LONG_WRITE)){
		io_req_t	ior = tgt->ior;
		register int	len = ior->io_count;

		tgt->transient_state.out_count = len;

		/* avoid leaks ?? */
		if (len < tgt->block_size) {
/*			bzero(to + len, tgt->block_size - len);*/
			len = tgt->block_size;
			tgt->transient_state.out_count = len;
		}
		/* see start_cmd & start_dataout */
		tgt->transient_state.dma_offset = 1;
	} else {
		tgt->transient_state.out_count = 0;
		tgt->transient_state.dma_offset = 0;
	}
}

int
kn15aa_dma_start_cmd(dma, tgt)
	dma_softc_t	dma;
	register target_info_t	*tgt;
{
	register int	out_count;
	vm_offset_t	phys;
	/*
	 * Note that the out_count does not include the
	 * (optional) identify message but does include
	 * both the data and cmd out count.
	 */
	out_count  = tgt->transient_state.cmd_count;
	out_count += tgt->transient_state.out_count;

	/* loadup DMA engine with DMAOUT from phys of tgt->cmd_ptr; */
	kn15aa_dma_write(tgt->cmd_ptr, out_count, dma, tgt);

	return out_count;
}

void
kn15aa_dma_end_cmd(dma, tgt, ior)
	dma_softc_t	dma;
	target_info_t	*tgt;
	io_req_t	ior;
{
#ifdef	MACH_KERNEL
#else	/*MACH_KERNEL*/
	fdma_unmap(&tgt->fdma, ior);
#endif	/*MACH_KERNEL*/
	tgt->dma_ptr = 0;
	tgt->transient_state.dma_offset = 0;
}

void
kn15aa_dma_end_xfer(dma, tgt, bytes_read)
	dma_softc_t	dma;
	target_info_t	*tgt;
	int		bytes_read;
{

	/* Was this a write op ? if so, just stop dma engine */
	if (bytes_read == 0) goto stop_dma;

	if (*dma->dudb & (SCSI_DUDB_MASK01|SCSI_DUDB_MASK10|SCSI_DUDB_MASK11)){
		printf("kn15aa_dma_end_xfer:Need to write code for before unaligned data %x \n",*dma->dudb);
			gimmeabreak();
	}

	{
	    register unsigned int w = *dma->dude;

	    if (w & (SCSI_DUDE_MASK00|SCSI_DUDE_MASK01|SCSI_DUDE_MASK10)){

		register unsigned int *to, mask = 0;

		to = (unsigned int *)(PHYS_TO_K0SEG((*dma->sda) <<2)) ;

		if (w &SCSI_DUDE_MASK00)
			mask |= 0xff;
		if (w &SCSI_DUDE_MASK01)
			mask |= 0xff00;
		if (w &SCSI_DUDE_MASK10)
			mask |= 0xff0000;
		*to = (*to & ~mask) | (w & mask);

	    }
	}
stop_dma:
	{
		register volatile unsigned int	*cir;
		cir = (volatile unsigned int *)SCSI_CIR;
		*cir &= ~(dma->dma_ena);
		wbflush();
		sgmap_free_entry(tgt->transient_state.copy_count);
		tgt->transient_state.copy_count = 0;

	}

}

int
kn15aa_dma_start_datain(dma, tgt)
	dma_softc_t	dma;
	register target_info_t	*tgt;
{
	register char	*dma_ptr;
	register int	count;

	/* setup tgt->dma_ptr to tgt->cmd_ptr or to ior->io_data, */

	if (tgt->cur_cmd == SCSI_CMD_READ ||
	    tgt->cur_cmd == SCSI_CMD_LONG_READ)
		dma_ptr = tgt->ior->io_data;
	else
		/* swear it won't disconnect midway a xfer or else */
		dma_ptr = tgt->cmd_ptr;
	tgt->dma_ptr = dma_ptr;

	count = tgt->transient_state.in_count;

	kn15aa_dma_read(dma_ptr, count, dma, tgt);

	count = u_min(count, ASC_TC_MAX);

	return count;
}

int
kn15aa_dma_restart_datain_1(dma, tgt)
	dma_softc_t	dma;
	target_info_t	*tgt;
{
	register char	*dma_ptr;
	register int	count;

	/* This is the unlikely case we disconnected before
	   actually starting the data xfer */
	if (tgt->dma_ptr == 0)
		return kn15aa_dma_start_datain(dma, tgt);

	/* keep dma_offset anyways */
	dma_ptr = tgt->dma_ptr + tgt->transient_state.dma_offset;

	count = tgt->transient_state.in_count;

	kn15aa_dma_read(dma_ptr, count, dma, tgt);

	count = u_min(count, ASC_TC_MAX);
	return count;
}

int
kn15aa_dma_restart_datain_2(dma, tgt, xferred)
	dma_softc_t	dma;
	target_info_t	*tgt;
	int		xferred;
{
	tgt->transient_state.dma_offset += xferred;

	return kn15aa_dma_restart_datain_1(dma, tgt);
}

void
kn15aa_dma_restart_datain_3(dma, tgt)
	dma_softc_t	dma;
	target_info_t	*tgt;
{
	/* No need to do anything */
	return;
}


boolean_t
kn15aa_dma_start_dataout(dma, tgt, regp, cmd)
	dma_softc_t	dma;
	register target_info_t	*tgt;
	volatile char	*regp, cmd;
{
	register char	*dma_ptr;

	if (tgt->transient_state.dma_offset) {

		*regp = 1;/*flush fifo*/
		wbflush();
		delay(1);

		dma_ptr = tgt->ior->io_data;
		kn15aa_dma_write(dma_ptr, tgt->transient_state.out_count, dma, tgt);

	} else {
		/* do not change the current pointer, which is ok */
		dma_ptr = tgt->cmd_ptr /* + tgt->transient_state.cmd_count*/;
	}

	tgt->transient_state.dma_offset = 0;
	tgt->dma_ptr = dma_ptr;

	return TRUE;
}

int
kn15aa_dma_restart_dataout_1(dma, tgt)
	dma_softc_t	dma;
	target_info_t	*tgt;
{
	int	count;

	/* this is if we disconnected before starting any xfer */
	if (tgt->dma_ptr == 0)
		kn15aa_dma_start_dataout(dma, tgt, &count, 0);

	count = tgt->transient_state.out_count;
	count = u_min(count, ASC_TC_MAX);

	return count;
}

int
kn15aa_dma_restart_dataout_2(dma, tgt, xferred)
	dma_softc_t	dma;
	register target_info_t	*tgt;
	int		xferred;
{
	tgt->transient_state.dma_offset += xferred;

	return kn15aa_dma_restart_dataout_1(dma, tgt);
}

int
kn15aa_dma_restart_dataout_3(dma, tgt, regp)
	dma_softc_t	dma;
	target_info_t	*tgt;
	volatile char	*regp;
{
	register char	*dma_ptr;
	int		ret;

	/* keep dma_offset anyways */
	dma_ptr = tgt->dma_ptr + tgt->transient_state.dma_offset;

/*unaligned issues here, align to 8 bytes ? (maybe not..)*/
	/*
	 * See NCR spec, Pag. 12 about 'preloading the FIFO'
	 */
	if ((vm_offset_t)dma_ptr & 1) {
		*regp = *dma_ptr++;
		ret = 1;
	} else
		ret = 0;

	kn15aa_dma_write(dma_ptr, tgt->transient_state.out_count, dma, tgt );

	return ret;
}

void
kn15aa_dma_restart_dataout_4(dma, tgt)
	dma_softc_t	dma;
	target_info_t	*tgt;
{
	/* Nothing needed here */
}

int
kn15aa_dma_start_msgin(dma, tgt)
	dma_softc_t	dma;
	target_info_t	*tgt;
{
	kn15aa_dma_read(tgt->cmd_ptr, 6, dma, tgt);
}

void
kn15aa_dma_end_msgin(dma, tgt)
	dma_softc_t	dma;
	target_info_t	*tgt;
{
	register unsigned int		*msgp, w;

	/*
	 * Must look at the DUDE register to see if anything left
	 * Obviously yes there is stuff, else it would not be fun
	 * Note however that we expect two lonely bytes at most,
	 * so we just pick the whole register.
	 */
	w = *dma->dude;
	if ((w & (SCSI_DUDE_MASK00|SCSI_DUDE_MASK01|SCSI_DUDE_MASK10)) == 0)
		return;
	msgp = (unsigned int *)tgt->cmd_ptr;
	*msgp = w;
}


boolean_t
kn15aa_dma_disconn_1(dma, tgt, xferred)
	dma_softc_t	dma;
	target_info_t	*tgt;
	int		xferred;
{
	/*
	 * XXX Known bug: if we disconnect on an odd bb ??
	 */

	{
		register volatile unsigned int	*cir;
		cir = (volatile unsigned int *)SCSI_CIR;
		*cir &= ~(dma->dma_ena);
	}

	tgt->transient_state.dma_offset += xferred;
	return FALSE;
}

boolean_t
kn15aa_dma_disconn_2(dma, tgt)
	dma_softc_t	dma;
	target_info_t	*tgt;
{
	/* Nothing needed */
	return FALSE;
}

#if notyet
boolean_t
kn15aa_dma_disconn_3(dma, tgt, xferred)
	dma_softc_t	dma;
	target_info_t	*tgt;
	int		xferred;
{

	register volatile char	*ptr;

	{
		register volatile unsigned int	*cir;
		cir = (volatile unsigned int *)SCSI_CIR;
		*cir &= ~(dma->dma_ena);
	}

	tgt->transient_state.dma_offset += xferred;
	return FALSE;
}
#define	kn15aa_dma_disconn_4		kn15aa_dma_disconn_3

#endif

#define	kn15aa_dma_disconn_3		kn15aa_dma_disconn_1
#define	kn15aa_dma_disconn_4		kn15aa_dma_disconn_1
#define	kn15aa_dma_disconn_5		kn15aa_dma_disconn_1
#define	kn15aa_dma_disconn_callback	0	/* never called */

scsi_dma_ops_t kn15aa_dma_ops = {
	kn15aa_dma_init,
	kn15aa_dma_new_target,
	kn15aa_dma_map,
	kn15aa_dma_start_cmd,
	kn15aa_dma_end_xfer,
	kn15aa_dma_end_cmd,
	kn15aa_dma_start_datain,
	kn15aa_dma_start_msgin,
	kn15aa_dma_end_msgin,
	kn15aa_dma_start_dataout,
	kn15aa_dma_restart_datain_1,
	kn15aa_dma_restart_datain_2,
	kn15aa_dma_restart_datain_3,
	kn15aa_dma_restart_dataout_1,
	kn15aa_dma_restart_dataout_2,
	kn15aa_dma_restart_dataout_3,
	kn15aa_dma_restart_dataout_4,
	kn15aa_dma_disconn_1,
	kn15aa_dma_disconn_2,
	kn15aa_dma_disconn_3,
	kn15aa_dma_disconn_4,
	kn15aa_dma_disconn_5,
	kn15aa_dma_disconn_callback,
};

