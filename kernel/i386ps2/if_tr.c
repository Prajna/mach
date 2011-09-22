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
 * and that the nema IBM not be used in advertising or publicity 
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
 * $Log:	if_tr.c,v $
 * Revision 2.4.1.1  94/02/17  14:19:41  mja
 * 	Discard non-Mach 3.0 code.
 * 	[93/12/29            zon]
 * 
 * 	Added MACH_TTD support.
 * 	[93/09/20            prithvi]
 * 
 * Revision 2.4  93/05/28  21:22:34  rvb
 * 	In tropen(), wait until the adapter is actually running before
 * 	returning.
 * 	[93/05/28  15:12:30  chs]
 * 
 * Revision 2.3  93/05/15  19:44:05  mrt
 * 	machparam.h -> machspl.h
 * 
 * Revision 2.2  93/02/04  08:00:19  danner
 * 	Integrate PS2 code from IBM.
 * 	[93/01/18            prithvi]
 * 
 */


/*
 * COMPONENT_NAME: (MKDEVNET) Mach Network Device Drivers
 *
 * FUNCTIONS: 
 *
 * ORIGINS: 27
 */

/*
 * IBM/4.3 6152 token-ring driver (Ungerman-Bass adaptor card)
 *
 * modified for OSF/1
 * routines:
 *	trprobe(addr)
 *	trattach(iod)
 *	trreset(unit)
 *	trsuspend(iod, idr, how)	needed only for DOS
 *	trinit(unit)
 *	trstart(trs,m0)
 *	trint(unit)
 *	trrint(unit)
 *	trxint(unit)
 *	trmbcopy(dhb, m0)
 *	trget(trs, totlen,ifp)		rewritten		
 *	troutput(ifp, m0, dst)		not moved to if_tokensubr.c
 *	trioctl(ifp, cmd, data)
 *	trbcopy(trs, dest, len)
 *	trprinttraddr(p)
 *	tropensap(unit,type) 
 *	trsleep(unit)
 *	trtimout(unit)
 *
 * Major problem areas:
 *	- getting the dependencies (header, byte order) correct, the if_un.c
 *	  code is the model
 *	- modification of trget() to match 4.4bsd mbuf changes, again, the
 *	  if_un.c code is the model
 *	- whether to create a net/if_tokensubr.c equivalent or change the
 *	  troutput()/trrint() routines in place
 * Minor nits:
 *	- the type fields are common (ip, arp, whatever), so use ETHERTYPE_IP
 *	  and ETHERTYPE_ARP for the moment (replaces TR_IPTYPE, TR_ARPTYPE).
 *	- there could be a net/if_802.5.h corresponding if if_ether.h, with
 *	  declarations moved from both if_trvar.h and if_trreg.h.
 * OSF/1 changes:
 *	- routing information field handling in arpresolve, etc., see
 *	  netinet/if_ether.c and netinet/if_ether.h
 * ACIS todo:
 *	- add IFF_BRIDGE check to output routine ca_atr/if_lan.c
 *	- make the two drivers consistent in symbol use, e.g., TR_IPTYPE
 */

#include "tr.h" 

#if NTR > 0

#define TOKEN_ADDR_SIZE	6

#if defined(TRDEBUG) && !defined(DEBUG)
#define DEBUG 1			/* driver specific debugging */
#endif

#include <kern/time_out.h>
#include <device/device_types.h>
#include <device/errno.h>
#include <device/io_req.h>
#include <device/if_hdr.h>
#include <device/if_ether.h>
#include <device/net_status.h>
#include <device/net_io.h>

/*
 * here is the first problem, ca/io.h and caio/ioccvar.h
 * are not used... we'll use the AT386 source as a model.
 * if_trreg.h and if_trvar.h appear to be used, so in they go.
 */

#include <i386ps2/if_common.h> 
#include <i386ps2/bus.h>
#include "if_trreg.h"
#include "if_trvar.h"
#include <i386/seg.h>
#include <i386/ipl.h>
#include <i386/machspl.h>

#ifdef MACH_TTD
#include <ttd/ttd_stub.h>
#endif

/*
 * here is the first problem with the token-ring driver:
 * ca/io.h and caio/ioccvar.h
 * are not used... we'll use the AT386 source as a model.
 *
 * ../machine/io.h
 *	large number of macros are defined, e.g., OUT(port,d)
 * #include "../machineio/ioccvar.h"
 */


int	trprobe(), trattach();
unsigned short tr_swap();

caddr_t	trstd[] = { (caddr_t) 0x00000a20, (caddr_t) 0x00000a24, 0};

/*
 * this was a pointer to an iocc_device in ca_atr/if_lan.c
 * in if_un.c we specify values, here we don't, refer to relyea, eric
 */
struct 	i386_dev *trinfo[NTR];

int trint(), trinit(), trioctl(), troutput(), trreset(), trtimout();
int trsuspend();
int trstart();

unsigned long tokdebug = 0;		/* flag for debugfs */

/*
 * see <i386/AT386/atbus.h> for declaration, eric
 */
struct	i386_driver trdriver =
	/* probe slave attach dname  dinfo mname minfo */
	{trprobe, 0, trattach, "tr", trinfo, 0, 0};


int (*trintrs[])() = {trint, 0};


/*
 *	Token-Ring software status per adapter
 */
struct	tr_softc {
	struct ifnet trs_if;		/* generic interface header */
	u_char  trs_addr[6];		/* token-ring hardware address */
	u_char	address[TOKEN_ADDR_SIZE];
	unsigned char trs_intlevel;	/* adapter interrupt level */
	short trs_ocount;		/* count of active output requests */
	char srbfree;			/* 0 = SRB free, 1 = SRB in use */
	unsigned char *mmio;		/* address of MMIO region */
	unsigned char *sram;		/* address of shared RAM */
	struct rbcb rbc;		/* receiver buffer control block */
	struct aca *aca;		/* pointer to adapter ACA */
	struct trs_cb cb;		/* struct of pointers to ctl blocks */
	struct init_resp init_rs;	/* adapter initialization response */
	unsigned short exsap_station;	/* station assigned by open sap cmd */
	struct	 trs_cb init_cb;     	/* place to save initial srb address */
	unsigned char wire_fault;    	/* flag a wire fault has occurred */
	caddr_t  trsleep_event;     	/* tr event signalled on successful */
					/* open of adapter  */
	int	alive;			/* card is alive */
	int	trs_flags;		/* saved flags over suspend */
#ifdef MACH_TTD
	int kttd_enabled;		/* if non-zero, kttd is enabled */
#endif
} tr_softc[NTR];

#ifdef MACH_TTD
int tr_get_packet();
int tr_send_packet();
static boolean ttd_poll_loop;
#endif

/*
 * trprobe 
 */
trprobe(addr, iod)
register struct trdevice *addr;		  /* pointer to device registers */
register struct i386_dev *iod;		/* pointer to device structure */
{
	register unsigned char *mmio = 0;	/* pointer to MMIO Region */
	register struct aca *aca;	/* pointer to Attachment Control Area */
	int s;
	int irq;
	int i, count;


	if (inb(addr) == inb(0xffff)) {
		outb(addr,0);
		if (inb(addr) == inb(0xffff)) {
			return(0);
		}
	}

	if (irq = tr_get_irq(addr))
		iod->dev_pic = irq;	/* set the IRQ level */

	iod->dev_spl = SPL4;		/* set to known spl level */


	/*
	 * Generate interrupt
	 */
	OUT(&addr->reset, 0);	/* latch on an unconditional adapter reset */
	trdelay(50); /* delay 50ms */

	/*
	 * turn off adapter reset, generate interrupt
	 */
	OUT(&addr->release_reset, 0);

	/*
	 * Calculate address of Attachment Control Area
	 */
	mmio = (unsigned char *) (((IN(&addr->switch_read) & 0xfc) << 11)
		+ TR_MMIO_OFFSET);
	aca = (struct aca *)(mmio + TR_ACA_OFFSET);	/* get ACA address */

	/*
	 * allow adapter to interrupt PC
	 */
	s = splhigh();
	MM_OUT(&aca->set.isrp_h, INT_ENABLE);

	trdelay(1000);
	
#ifdef notdef
	PROBE_DELAY(1 000 000); /* delay for 1 seconds, delay is interruptable */
#endif
	/*
	 * since ps-2 uses level trigerring for interrupts, we must
	 * not allow interupts to go to the pc until we clear the
	 * SRB RESPONSE INTERRUPT in TRATTACH
	 */
	MM_OUT(&aca->reset.isrp_h, ~(INT_ENABLE));
	splx(s);


	return(PROBE_OK);	/* indicate adapter should have interrupted */
}

#define TOKEN_ID 0xE000

/*
 * determine what IRQ we are at
 */
tr_get_irq(port)
	int	port;
{
	int slot;
	int addr;
	int irq = 0;

	for (slot=0; (slot = pos_slot(TOKEN_ID, slot)) >= 0; ++slot) {
		addr = (slots[slot].pos_info[1] & 1) == 0 ? 0xa20 : 0xa24;
		if (addr != port)
			continue;	/* look for another */
		irq = 2 + (slots[slot].pos_info[1]>>7)  +
			((slots[slot].pos_info[2]&1) << 3);
		break;
	}

	if (irq == 2)
		irq = 9;	/* IRQ 2 is remaped to IRQ 9 */
	return(irq);		/* don't know - assume normal default */
}


/*
 * trattach - Make this interface available to the system-network-software.
 * When the system-network-software is ready to accept packets it will
 * initialize the interface by calling trinit().
 */
trattach(iod)
register struct i386_dev *iod;	/* pointer to device structure */
{
	register struct tr_softc *trs = &tr_softc[iod->dev_unit];
	register struct ifnet *ifp = &trs->trs_if;
	register struct trdevice *addr =
		(struct trdevice *)((unsigned int)iod->dev_addr & 0xffffff);
	unsigned long i;
	int temp, unit;

	trs->wire_fault = 0x00;

	/*
	 * init network-visible interface 
	 */
	ifp->if_unit = iod->dev_unit;
	ifp->if_mtu = ETHERMTU;
	ifp->if_header_size = sizeof(struct tr_head);
	ifp->if_header_format = HDR_802;
	ifp->if_address_size = 6;
	ifp->if_address = (char *)&trs->trs_addr[0];
	if_init_queues(ifp);

	/*
	 * Save interrupt level in softc struct
	 * not used! eric
	 * trs->trs_intlevel = iod->dev_irq;
	 */

	/* Read adapter switches and calculate addresses of MMIO and ACA */
	i = IN(&addr->switch_read);	/* Read switches */

	/* set MMIO address */
	trs->mmio = 0;
	trs->mmio = (unsigned char *) (((i & 0xfc) << 11) + TR_MMIO_OFFSET);
	/* set ACA address */
	trs->aca = (struct aca *)(trs->mmio + TR_ACA_OFFSET);
	DEBUGF(tokdebug,printf("trattach: ram = %x mmio=%x aca=%x\n",
					((i&0xfc) <<11),trs->mmio, trs->aca););

	/*
	 * Get address of adapter shared RAM from PIO read of loc. 0xa22
	 */
	i = IN(&addr->release_reset) & 0xfe;	/* Read switches */

	/*
	 * Get address of sram
	 */
	trs->sram = (unsigned char *) (caddr_t)(i << 12);
	DEBUGF(tokdebug,printf("trattach: sram = %x\n",trs->sram););

	/*
	 * Save address of SRB in softc struct
	 */
#if     1
	trdelay(2000);
	temp = MM_IN(&trs->aca->rw.wrb_h);
	temp <<= 8;
/*	trdelay(500); */
	temp |= MM_IN(&trs->aca->rw.wrb_l);
#else
	for (temp=i=0; i<10; ++i)
		temp |= MM_INW(&trs->aca->rw.wrb);	/* hardware bug? */
#endif
	trs->cb.srb = (union sr_srb *)(trs->sram + temp);
	trs->init_cb.srb = trs->cb.srb;
	DEBUGF(tokdebug,printf("trattach: srb = %x\n",trs->init_cb.srb));
	DEBUGF(tokdebug,printf("trattach: srb at sram + %#x\n", temp));
	/*
	 * Save SRB response to adapter card initialization
	 */
	for (i = 0; i < sizeof(struct init_resp); i++)
		((unsigned char *)&trs->init_rs)[i] = MM_IN(((char *)trs->cb.srb + i));

	trs->init_rs.encoded_addr = tr_swap(trs->init_rs.encoded_addr);
	DEBUGF(tokdebug,
	{
	   trs->init_rs.bring_up_res = tr_swap(trs->init_rs.bring_up_res);
	   trs->init_rs.level_addr = tr_swap(trs->init_rs.level_addr);
	   trs->init_rs.adap_addr = tr_swap(trs->init_rs.adap_addr);
	   trs->init_rs.params_addr = tr_swap(trs->init_rs.params_addr);
	   trs->init_rs.mac_addr = tr_swap(trs->init_rs.mac_addr);
	   printf("\t\tbring up result=%x\n",trs->init_rs.bring_up_res);
	   printf("\t\tencoded address=%x\n",trs->init_rs.encoded_addr);
	   printf("\t\tlevel address=%x\n",trs->init_rs.level_addr);
	   printf("\t\tadapter address=%x\n",trs->init_rs.adap_addr);
	   printf("\t\tparameter address=%x\n",trs->init_rs.params_addr);
	   printf("\t\tMAC buffer address=%x\n",trs->init_rs.mac_addr);
	})

	/*
	 * Save adapter hardware address
	 */
	for (i = 0; i < TR_ADDR_LEN; i++)
		trs->trs_addr[i] =
			MM_IN((trs->init_rs.encoded_addr + trs->sram + i));

	printf("tr%d: token-ring address ", ifp->if_unit);
	trprinttraddr(trs->trs_addr);
	printf("\n");

	/* Indicate that SRB is used */
	trs->srbfree = 1;
	ifp->if_flags = IFF_BROADCAST;
	/*
	 * clear the SRB-response interrupt bit
	 */
	MM_OUT(&trs->aca->reset.isrp_l, ~(SRB_RESP_INT));
	MM_OUT(&trs->aca->reset.isra_l, ~(CMD_IN_SRB));
	MM_OUT(&trs->aca->set.isrp_h, INT_ENABLE); /*allow pc ints. */

	trs->alive = 1;
	DEBUGF(tokdebug, printf("tr%d: attached\n", iod->dev_unit);)
#ifdef	MACH_TTD
	unit = ifp->if_unit;
	if (trs->kttd_enabled = !ttd_get_packet) {
	        kttd_debug_init = TRUE;
		ttd_device_unit = unit;
		ttd_get_packet = tr_get_packet;
		ttd_send_packet = tr_send_packet;
		ttd_host_ether_id.array[0] = *(trs->address);
		ttd_host_ether_id.array[1] = *(trs->address + 1);
		ttd_host_ether_id.array[2] = *(trs->address + 2);
		ttd_host_ether_id.array[3] = *(trs->address + 3);
		ttd_host_ether_id.array[4] = *(trs->address + 4);
		ttd_host_ether_id.array[5] = *(trs->address + 5);
#endif
}

/*
 * trreset - reset interface (issue a DIR CLOSE ADAPTER command)
 */
trreset(unit)
register int unit;
{
	register struct i386_dev *iod;   /* pointer to device structure */
	register struct tr_softc *trs = &tr_softc[unit];
	register union sr_srb *srb = trs->cb.srb;
	register struct ifnet *ifp = &trs->trs_if;



	if (unit < NTR && (iod = trinfo[unit]) != 0 && iod->dev_alive != 0){
                if (ifp->if_flags & IFF_RUNNING) {
			trs->srbfree = 1;	/* indicate SRB in use */
			/* close command */
			MM_OUT(&srb->close.command, DIR_CLOSE);
			/* tell adapter: command in SRB */
			MM_OUT(&trs->aca->set.isra_l, CMD_IN_SRB);
			/* wait for it to complete */
			trsleep(ifp->if_unit);
		}
		MM_OUT(&trs->aca->reset.isrp_h, ~(INT_ENABLE)); /* paranoia */
		DEBUGF(tokdebug, printf("tr%d: reset\n", unit);)
	}
}

/*
 *  trinit - initialize network interface, open adapter for packet
 *	     reception and start any pending output
 */
trinit(unit)
register int unit;
{
	register struct tr_softc *trs = &tr_softc[unit];
	register struct ifnet *ifp = &trs->trs_if;
	register union sr_srb *srb = trs->init_cb.srb;
	register int i;
	register struct i386_dev *iod = trinfo[unit];	/* pointer to device structure */
	int s;

        if ((ifp->if_flags & IFF_RUNNING) == 0) { /* if not running */
		s = splimp();
		trs->trs_ocount = 0;	/* reset active-output-request count */
		/*
		 * open adapter.  the open-complete interrupt handler will
		 * start any pending output.
		 */
		trs->srbfree = 1;	/* indicate SRB in use */

		/* zero SRB */
                trs->cb.srb = trs->init_cb.srb;
		/* restore initial srb in trs structure */

		for (i=0; i < sizeof(*trs->cb.srb); i++)
			MM_OUT(((char *)(trs->cb.srb) + i), 0);

		/* set open parameters in SRB */
		MM_OUT(&srb->open_cmd.command, DIR_OPEN_ADAPTER); /* open cmd */
		MM_OUTW(&srb->open_cmd.open_options, OPEN_PASS_BCON_MAC);
		MM_OUTW(&srb->open_cmd.num_rcv_buf, NUM_RCV_BUF);
		MM_OUTW(&srb->open_cmd.rcv_buf_len, RCV_BUF_LEN);
		MM_OUTW(&srb->open_cmd.dhb_length, DHB_LENGTH);
		MM_OUT(&srb->open_cmd.num_dhb, NUM_DHB);
		MM_OUT(&srb->open_cmd.dlc_max_sap, 2);
		/* allow MAX of two SAPS */
		MM_OUT(&srb->open_cmd.dlc_max_sta, 8);
		/* allow MAX of 8  stations */

		take_dev_irq(iod);		/* take over interrupt */
		
		/* allow interrupts */
		MM_OUT(&trs->aca->set.isrp_h, INT_ENABLE);

		/* tell adapter: command in SRB */
		MM_OUT(&trs->aca->set.isra_l, CMD_IN_SRB);


		splx(s);
	}
	DEBUGF(tokdebug, printf("tr%d: init'ed\n", unit);)
}



/*
 *  trstart - Present transmit request to adapter
 */
trstart(trs)
register struct tr_softc *trs;	/* pointer to softc struct */
{
	register union sr_srb *srb = trs->cb.srb;	/* pointer to SRB */

	DEBUGF(tokdebug, printf("tr%d: trstart\n",trs->trs_if.if_unit);)
	trs->srbfree = 1;	/* indicate SRB in use */

	/* load SRB to request transmit */

	MM_OUT(&srb->xmit.command, XMIT_UI_FRM); /* xmit ui frame */

	MM_OUTW(&srb->xmit.station_id, trs->exsap_station);
	/* tell adapter that SRB loaded and ask for SRB free interrupt */
	MM_OUT(&trs->aca->set.isra_l, (CMD_IN_SRB + SRB_FREE));
}


#define	IF_EMPTYQUEUE(queue) ((queue).ifq_len == 0)

/*
 *  trint - interrupt handler.  Find the cause of the interrupt and
 *  service it.
 */
trint()
{
	register int unit = 0; /* TEMP */
	/* get address of softc struct for this adapter */
	register struct tr_softc *trs = &tr_softc[unit];
	register struct ifnet *ifp = &trs->trs_if;


	/* holds status read from adapter status register */
	register unsigned char status;

	/* holds command read from status or request block */
	register unsigned char command;

	/* 1 = unclaimed interrupt, 0 = interrupt claimed */
	int rc = 1;

	int close_signal = 0;		/* remember if now closed */

	if (trs->aca == 0) {
		return(rc);
	}

	/* do not allow interrupts until we finish and reset the */
	/* corresponding system block.                           */
	MM_OUT(&trs->aca->reset.isrp_h, ~(INT_ENABLE));

	/* Is this interrupt caused by an adapter check? */
	if ((status = MM_IN(&trs->aca->rw.isrp_l)) & ADAP_CHK_INT) {
		printf("tr%d: adapter check\n", unit);
	
		/* Clear this interrupt bit */
		MM_OUT(&trs->aca->reset.isrp_l, ~(ADAP_CHK_INT));
		MM_OUT(&trs->aca->set.isrp_h, INT_ENABLE);
	
		rc = 0;		/* Claim responsibility for interrupt */
	}

	/* Is this interrupt caused by an adapter error or access violation? */
	else if (MM_IN(&trs->aca->rw.isrp_h) & TCR_INT + ERR_INT + ACCESS_INT) {
		printf("tr%d: adapter error, ISRPH = %x\n",
			unit, MM_IN(&trs->aca->rw.isrp_h));

		/* Clear these interrupt bits */
		MM_OUT(&trs->aca->reset.isrp_h, ~(TCR_INT + ERR_INT + ACCESS_INT));
		MM_OUT(&trs->aca->set.isrp_h, INT_ENABLE);

		rc = 0;		/* Claim responsibility for interrupt */

	}

	/* Is this interrupt caused by normal operation of adapter? */
	else if (status & SRB_RESP_INT + ASB_FREE_INT
			+ ARB_CMD_INT + SSB_RESP_INT) {

		DEBUGF(tokdebug, printf("tr%d: ISRPL = %x\n", unit, status);)

		/* Process SRB_RESP_INT, ASB_FREE_INT, ARB_CMD_INT
		   & SSB_RESP_INT in that order, ISRP-L Hi to Lo */

		if (status & SRB_RESP_INT) {	/* Adapter response in SRB? */
		register union sr_srb *srb = trs->cb.srb;  /* pointer to SRB */

		command = MM_IN(&srb->xmit.command);
		switch(command) {

			case XMIT_DIR_FRAME:	/* Response to xmit request */
			/* Response not valid? */
			if (MM_IN(&srb->xmit.retcode) != 0xff)
				printf("tr%d: SRB error on xmit request =%x\n",
				unit, MM_IN(&srb->xmit.retcode));
			else {
			/* inc count of active xmit cmds */
				trs->trs_ocount++;
				DEBUGF(tokdebug,
				printf("tr%d: trs_ocount = %d\n",
					       unit, trs->trs_ocount);)
			}
			break;

			case XMIT_UI_FRM:	/* Response to xmit request */
			if (MM_IN(&srb->xmit.retcode) != 0xff)
				printf("tr%d: SRB error on xmit request =%x\n",
				unit, MM_IN(&srb->xmit.retcode));
			else {
			/* inc count of active xmit cmds */
				trs->trs_ocount++;
				DEBUGF(tokdebug,
					printf("tr%d: trs_ocount = %d\n",
						unit, trs->trs_ocount);)
			}
			break;
			case DIR_OPEN_ADAPTER:	/* open-adapter-cmd response */
			/* Open successful? */
			if (MM_IN(&srb->open_resp.retcode) == 0) {
#ifdef MACH_TTD
			  if (!trs->kttd_active)
#endif
			  trs->trs_if.if_flags |= IFF_UP | IFF_RUNNING;
			/* Save new ACA ctrl block addresses */
trs->cb.ssb = (struct sr_ssb *) (MM_INW(&srb->open_resp.ssb_addr) + trs->sram);
trs->cb.arb = (union sr_arb *) (MM_INW(&srb->open_resp.arb_addr) + trs->sram);
trs->cb.srb = (union sr_srb *) (MM_INW(&srb->open_resp.srb_addr) + trs->sram);
trs->cb.asb = (union sr_asb *) (MM_INW(&srb->open_resp.asb_addr) + trs->sram);

			/* re enable interrupts to the PC so we do not */
			/* miss the open sap interrupt....             */
				MM_OUT(&trs->aca->set.isrp_h, INT_ENABLE);

			/* reset the wire fault signal */
				trs->wire_fault = 0x00;
				tropensap(unit,EXTENDED_SAP); 

			} else {
				printf("tr%d: Open error = %x\n",
					unit, MM_IN(&srb->open_resp.retcode));
				ifp->if_flags &= ~IFF_RUNNING;
				ifp->if_flags &= ~IFF_UP;
				timeout(trinit,ifp->if_unit,hz*30);
			}
			break;

			case DIR_CLOSE:	/* Response to close adapter command */
				/* close not successful? */
				if (MM_IN(&srb->close.retcode) != 0)
				    printf("tr%d: close error = %x\n",
					   unit, MM_IN(&srb->close.retcode));
			else if
				(MM_IN(&srb->close.retcode) == 0) {
					close_signal = 1;
					ifp->if_flags &= ~IFF_RUNNING;
					ifp->if_flags &= ~IFF_UP;
					MM_OUT(&trs->aca->reset.isrp_h, ~(INT_ENABLE));
					untimeout(trtimout, unit);
					wakeup(&trs->trsleep_event);
				}
				break;

			case DIR_INTERRUPT:  /* Response to generate-intr cmd */
				/* generate-interrupt cmd not successful? */
				if (MM_IN(&srb->intr.retcode) != 0)
					printf("tr%d: int cmd error =%x\n",
					unit, MM_IN(&srb->intr.retcode));
				break;

			case DIR_MOD_OPEN_PARAMS:   /* Response to mod params */
				/* cmd not successful? */
				if (MM_IN(&srb->mod_params.retcode) != 0)
					printf("tr%d: mod params cmd err =%x\n",
					unit, MM_IN(&srb->mod_params.retcode));
				break;

			case DIR_SET_GRP_ADDR:	/* Response to set-grp-addr */
			case DIR_SET_FUNC_ADDR:	/* Response to set-func-addr */
				/* cmd not successful? */
				if (MM_IN(&srb->set_addr.retcode) != 0)
				printf("tr%d: set grp or func addr err =%x\n",
				unit, MM_IN(&srb->set_addr.retcode));
				break;

			case OPEN_SAP:         	/* Response to open sap cmd */

				if (MM_IN(&srb->open_sap.sap_value) == EXTENDED_SAP) {
					trs->exsap_station = MM_INW(&srb->open_sap.station_id);
				}
				printf("tr%d: Token Ring opened\n", unit);
				untimeout(trtimout, unit);
				wakeup(&trs->trsleep_event);
				break;
			case CLOSE_SAP: /* Response to close sap cmd */
				break;
			case DIR_READ_LOG:   /* Response to read log */
				/* cmd not successful? */
				if (MM_IN(&srb->log.retcode) != 0)
					printf("tr%d: read error log cmd err =%x\n",
					unit, MM_IN(&srb->log.retcode));
				DEBUGF(tokdebug, {
				printf("tr%d: ERROR LOG:\n",unit);
				printf("tr%d: Line=%d, Internal=%d, Burst=%d\n",unit,(MM_IN(&srb->log.data[0])),(MM_IN(&srb->log.data[1])),(MM_IN(&srb->log.data[2])));
				printf("tr%d: A/C=%d, Abort=%d, Lost frames=%d\n",unit,(MM_IN(&srb->log.data[3])),(MM_IN(&srb->log.data[4])),(MM_IN(&srb->log.data[6])));
				printf("tr%d: Receive congestion=%d, Frame copied=%d, Frequency=%d\n",unit,(MM_IN(&srb->log.data[7])),(MM_IN(&srb->log.data[8])),(MM_IN(&srb->log.data[9])));
				printf("tr%d: Token=%d\n",unit,(MM_IN(&srb->log.data[10])));
				};)
				trs->srbfree = 0;	/* done with SRB */
				break;
			default:
				printf("tr%d: bad SRB command encountered\n",
				unit);
				break;
			}
			/* clear the SRB-response interrupt bit */
			MM_OUT(&trs->aca->reset.isrp_l, ~(SRB_RESP_INT));

			/* Try to start output? */
			if (command == OPEN_SAP &&
			MM_IN(&srb->open_resp.retcode) == 0) {
				
				trs->srbfree = 0;
				/* if output not active */
				if (trs->trs_ocount == 0 && 
				/* and data on send queue */
				! IF_EMPTYQUEUE(trs->trs_if.if_snd))
					/* Continue output from queue */
					trstart(trs);
			}
		}

		if (status & ASB_FREE_INT){  /* Is Adapter Status Block Free? */
		register union sr_asb *asb = trs->cb.asb;  /* pointer to ASB */

			switch (command = MM_IN(&asb->rec_resp.command)) {
			case REC_DATA:		/* Receive */
				/* Response not valid? */
				if (MM_IN(&asb->rec_resp.retcode) != 0xff)
				printf("tr%d: ASB bad receive response =%x\n",
				unit, MM_IN(&asb->rec_resp.retcode));
				break;

			case XMIT_DIR_FRAME:	/* Transmit */
				/* Response not valid? */
				if (MM_IN(&asb->xmit_resp.retcode) != 0xff)
				printf("tr%d: ASB response err on xmit =%x\n",
				unit, MM_IN(&asb->xmit_resp.retcode));
				break;

			case XMIT_UI_FRM:   	/* Transmit */
				/* Response not valid? */
				if (MM_IN(&asb->xmit_resp.retcode) != 0xff)
				printf("tr%d: ASB response err on xmit =%x\n",
				unit, MM_IN(&asb->xmit_resp.retcode));
				break;

			default:
				printf("tr%d: Invalid command in ASB =%x\n",
				unit, command);
				break;
			}
		
			/* Clear this interrupt bit */
			MM_OUT(&trs->aca->reset.isrp_l, ~(ASB_FREE_INT));
		}

		if (status & ARB_CMD_INT) {	/* Command for PC to process? */
			switch (command = MM_IN(&trs->cb.arb->stat.command)) {
			case DLC_STATUS:    /* DLC status change */	
				DEBUGF(tokdebug, printf("tr%d: ARB new DLC  status = 0x%x\n",
					unit, MM_INW(&trs->cb.arb->dlc.status));)
				break;

			case REC_DATA:		/* Adapter has data for PC */
				/* Call receive interrupt handler */
				trrint(unit);
				break;

			case RING_STAT_CHANGE:	/* Ring status change */
				if (MM_INW(&trs->cb.arb->stat.ring_status) & (SIGNAL_LOSS + LOBE_FAULT)){
				printf("tr%d: SIGNAL LOSS/LOBE FAULT\n",
					unit);

					ifp->if_flags &= ~IFF_RUNNING;
					ifp->if_flags &= ~IFF_UP;
					trs->wire_fault = 0x01;
					timeout(trinit,ifp->if_unit,hz*30);
				} else {
				DEBUGF(tokdebug,
				if (MM_INW(&trs->cb.arb->stat.ring_status) & ~(SOFT_ERR))
				printf("tr%d: ARB new ring status = 0x%x\n",
					unit, MM_INW(&trs->cb.arb->stat.ring_status));)
				}
				if (MM_INW(&trs->cb.arb->stat.ring_status) & LOG_OFLOW){
					trs->srbfree = 1;	/* using SRB */
					MM_OUT(&trs->cb.srb->log.command, DIR_READ_LOG);

					/* read & reset err log cmnd in SRB */
					/* enable PC ints so we get response */

					MM_OUT(&trs->aca->set.isrp_h, INT_ENABLE);
					MM_OUT(&trs->aca->set.isra_l, CMD_IN_SRB);
					DEBUGF(tokdebug, printf("tr%d: read adapter error log\n", unit);)
				}
				break;

			case XMIT_DATA_REQ: /* Adapter wants data to transmit */
				/* Call transmit interrupt handler */
				trxint(unit);


				break;

			default:
				printf("tr%d: Invalid command in ARB =%x\n",
				unit, command);
				break;
			}

			/* Clear this interrupt bit */
			MM_OUT(&trs->aca->reset.isrp_l, ~(ARB_CMD_INT)); 

			/* Tell adapter that ARB is now free */
			MM_OUT(&trs->aca->set.isra_l, ARB_FREE);
		}


		if (status & SSB_RESP_INT){   /* SSB response to SRB command? */

			/* pointer to SSB */
			register struct sr_ssb *ssb = trs->cb.ssb;
			switch(MM_IN(&ssb->command)) {

			case XMIT_DIR_FRAME:  /* SSB response to SRB xmit cmd */
				if (trs->trs_ocount <= 0)
					printf("tr%d: bad active-xmit count\n",
					unit);

				/* collect status on last packet */
				if (MM_IN(&ssb->retcode) != 0) {    /* error on xmit? */
					/* increment output error count */
					trs->trs_if.if_oerrors++;
					DEBUGF(tokdebug,
					{printf("trint: xmit err retcode = %x ",
					MM_IN(&ssb->retcode));
					printf(" xmit_error = 0x%x\n",
					MM_IN(&ssb->xmit_err));})
				} else
					/* increment output packet count */
					trs->trs_if.if_opackets++;

				/* decrement active output count */
				trs->trs_ocount--;

				/* if data on send queue */
				if (! IF_EMPTYQUEUE(trs->trs_if.if_snd) &&
				/* and SRB is free */
				trs->srbfree == 0)
					trstart(trs);
				break;

			case XMIT_UI_FRM:     /* SSB response to SRB xmit cmd */
				trs->srbfree = 0;
				if (trs->trs_ocount <= 0)
					printf("tr%d: bad active-xmit count\n",
					unit);

				/* collect status on last packet */
				if (MM_IN(&ssb->retcode) != 0) {    /* error on xmit? */
					/* increment output error count */
					trs->trs_if.if_oerrors++;
					DEBUGF(tokdebug,
					{printf("trint: xmit err retcode = %x ",
					MM_IN(&ssb->retcode));
					printf(" xmit_error = 0x%x\n",
					MM_IN(&ssb->xmit_err));})
				} else
					/* increment output packet count */
					trs->trs_if.if_opackets++;

				/* decrement active output count */
				trs->trs_ocount--;

				/* if data on send queue */
				if (! IF_EMPTYQUEUE(trs->trs_if.if_snd) &&
				/* and SRB is free */
				trs->srbfree == 0) {
					trstart(trs);
				}
				break;

			case XMIT_XID_CMD:
				printf("trint: xmit XID return code = 0x%x\n",
				MM_IN(&ssb->retcode));
				break;

			default:
				printf("tr%d: SSB error, invalid command =%x\n",
				unit, MM_IN(&ssb->command));
			}
			/* clear this interrupt bit */
			MM_OUT(&trs->aca->reset.isrp_l, ~(SSB_RESP_INT));

			/* tell adapter that SSB is available */
			MM_OUT(&trs->aca->set.isra_l, SSB_FREE);
		}
		rc = 0;		/* Claim responsibility for interrupt */
	}
	DEBUGF(tokdebug, printf("tr%d: rc=%d\n", unit, rc);)
	if (!close_signal) {
		MM_OUT(&trs->aca->set.isrp_h, INT_ENABLE);
	}
	return(rc);
}
 
#ifdef	MACH_TTD

int tr_get_packet(unit)
int unit;
{
	register struct tr_softc *trs = &tr_softc[unit];
	int s;

	s = splhigh();

	trs->ttd_poll_loop = TRUE;	
	while (trs->ttd_poll_loop)
		trint(unit);

	splx(s);

	return 0;

}

int tr_send_packet(unit, packet, len)
int unit;
char *packet;
int len;
{
}

#endif

/*
 *  U-B receive interrupt.
 *
 * in the original version, this routine had three tasks:
 *
 *	1. move the data into the receive buffer and set up various pointers
 *	   in the tr_softc struct
 *	2. switch on the type field for ip and arp, dropping all else
 *	3. resetting the adaptor status block info (asb) and updating the
 *	   tr_softc struct
 *		determine lan message type, pull packet off interface and
 *		pass to an appropriate higher-level routine
 *
 */
trrint(unit)
int unit;
{
	register struct tr_softc *trs = &tr_softc[unit];
	register union sr_arb *arb = trs->cb.arb;
	register union sr_asb *asb = trs->cb.asb;
	register struct rbcb *rbc = &trs->rbc;
	register int totlen;
	int i;
	register int hsize;
	unsigned char type;
	unsigned short etype;
	struct ifqueue *inq;
	unsigned char *llcptr;
	register struct tr_head *trhead;
	unsigned char trheadbuf[64];
	ipc_kmsg_t	new_kmsg;
	struct tr_head *thp;
	struct packet_header *pkt;

	DEBUGF(tokdebug,
	       printf("trrint: arb.command = %x, arb.station_id= %x\n",
		MM_IN(&arb->rec.command), MM_INW(&arb->rec.station_id));)
	DEBUGF(tokdebug,
		printf("arb.buf_addr = %x, arb.lan_hdr_len = %x\n",
		MM_INW(&arb->rec.buf_addr), MM_IN(&arb->rec.lan_hdr_len));)
	DEBUGF(tokdebug,
		printf("arb.dlc_hdr_len = %d, arb.frame_len = %d\n",
		MM_IN(&arb->rec.dlc_hdr_len), MM_INW(&arb->rec.frame_len));)
	DEBUGF(tokdebug,
		printf("arb.msg_type = %x\n", MM_IN(&arb->rec.msg_type));)
	/*
	 * copy the offset in RAM of the first receive buffer from the
	 * receive-data block of the adapter request block associated
	 * with the unit's softc struct into the receive control block.
	 */
	rbc->rbufp = (struct rec_buf *)(MM_INW(&arb->rec.buf_addr) + trs->sram);

	/*
	 * copy the pointer to data in first receive buffer
	 */
	rbc->rbuf_datap = rbc->rbufp->data;
	/*
	 * the token-ring header is viewed as two header structs: the physical
	 * header (aka TR header) with access, frame, dest, src, and routing
	 * information, and the logical link control header (aka LLC header)
	 * with dsap, ssap, llc, proto and type fields.
	 *
	 * rfc1042 requires support for unnumbered information (UI) commands,
	 * but does not specify a required semantic, so we'll discard them.
	 *
	 * set llcptr to point past the TR physical header (including
	 * routing information), and also skip the dsap and ssap fields
	 * of the LLC frame, if UI, chuckit.
	 */
	llcptr = (unsigned char *)(
		   (int) rbc->rbuf_datap + MM_IN(&arb->rec.lan_hdr_len))
			+ SKIP_DSAP_SSAP;
	if ((type = MM_IN(llcptr)) !=UI_CMD) {
		DEBUGF(tokdebug,
		printf("trrint: non-UI frame received, type = %x\n",type);)
		goto chuckit;
	}
	/*
	 * backup to the beginning of the LLC header and get the type field
	 */
	llcptr = (unsigned char *)(
		(int)rbc->rbuf_datap + MM_IN(&arb->rec.lan_hdr_len));
	etype = MM_INW(&((struct tr_llc *)llcptr)->ethertype);
	/*
	 * if there is a second receive buffer, set up the next pointer
	 */
	if (MM_INW(&rbc->rbufp->buf_pointer))
		rbc->rbufp_next =
			(struct rec_buf *)(MM_INW(&rbc->rbufp->buf_pointer)
			+ trs->sram - 2);
	else
		rbc->rbufp_next = 0;	/* we're finished */
	/*
	 * get size of lan and dlc headers, add 5 for the SNAP LENGTH
	 */
	hsize = MM_IN(&arb->rec.lan_hdr_len)
		+ MM_IN(&arb->rec.dlc_hdr_len)
		+ SNAP_LENGTH;
	/*
	 * get total length of data in frame
	 */
	totlen = MM_INW(&arb->rec.frame_len) - hsize;

	DEBUGF(tokdebug & 2,{
		int i;
		printf("tr%d: trrint - physical header\n", unit);
		for (i=0; i < hsize; i++) 
			printf("0x%x ",MM_IN((rbc->rbuf_datap + i)));
		printf("\n");
	})
	/*
	 * read in the tr header and store it in trheadbuf
	 */
	for (i=0; i < hsize; i++) 
		trheadbuf[i] = MM_IN(rbc->rbuf_datap + i);
	/*
	 * set trhead to point to lan header for tr_arpinput
	 */
	trhead = (struct tr_head *)trheadbuf;
	/*
	 * adjust data pointer to skip headers not needed
	 */
	rbc->rbuf_datap += hsize;
	/*
	 * calculate length of data in 1st receive buffer
	 */
	rbc->data_len = MM_INW(&rbc->rbufp->buf_len) - hsize;
	/*
	 * At this point we move the packet from the adapter to a chain
	 * of mbufs
	 */

#ifdef MACH_TTD
	if (trs->kttd_active)
		new_kmsg = (ipc_kmsg_t)ttd_request_msg;
	else
#endif

	new_kmsg = net_kmsg_get();

	if (new_kmsg == IKM_NULL) {
		DEBUGF(tokdebug, printf("tr%d: trrint: net_kmsg_get failed\n",
			unit));
		goto chuckit;
	}
	
	thp =	(struct tr_head *)	(&net_kmsg(new_kmsg)->header[0]);
	pkt =	(struct packet_header *)(&net_kmsg(new_kmsg)->packet[0]);

	/*
	 * Get header.
	 */
	*thp = *trhead;

	/*
	 * Get packet body.
	 */
	trbcopy(trs, (char *)(pkt + 1), totlen);

	pkt->type = htons(etype);
	pkt->length = totlen + sizeof(struct packet_header);
	/*
	 * Send the packet through the input filters.
	 */
	DEBUGF(tokdebug&2, printf("tr%d: trrint: calling net_packet; len=%d\n",
		unit, pkt->length);)
#ifdef MACH_TTD
	/* Don't want to call net_packet if we are polling for a packet */
	if (trs->kttd_active) 
	  trs->ttd_poll_loop = FALSE;
	else
#endif
	net_packet(&trs->trs_if, new_kmsg, pkt->length, TRUE);
	                                /* !tr_broadcast(new_kmsg) XXX */

	trs->trs_if.if_ipackets++;	/* increment input packet count */

	if (MM_IN(&asb->rec_resp.retcode) != 0xff)
		printf("trrint: ASB IS NOT FREE!!!\n");
	/*
	 * load receive response into ASB
	 */
	MM_OUT(&asb->rec_resp.command, REC_DATA);
	MM_OUTW(&asb->rec_resp.station_id, MM_INW(&arb->rec.station_id));
	MM_OUTW(&asb->rec_resp.rec_buf_addr, MM_INW(&arb->rec.buf_addr));
	/*
	 * indicate successful receive
	 */
	MM_OUT(&asb->rec_resp.retcode, 0);
	/*
	 * tell adapter response in ASB
	 */
	MM_OUT(&trs->aca->set.isra_l, RESP_IN_ASB);

	return;
chuckit:

	trs->trs_if.if_ierrors++;	/* increment input error count */

	if (MM_IN(&asb->rec_resp.retcode) != 0xff)
		printf("trrint: ASB IS NOT FREE!!!\n");

	DEBUGF(tokdebug, printf("trrint: packet dropped\n");)
	/*
	 * load receive response into ASB
	 */
	MM_OUT(&asb->rec_resp.command, REC_DATA);
	MM_OUTW(&asb->rec_resp.station_id, MM_INW(&arb->rec.station_id));
	MM_OUTW(&asb->rec_resp.rec_buf_addr, MM_INW(&arb->rec.buf_addr));
	/*
	 * tell adapter data lost, no mbufs
	 */
	MM_OUT(&asb->rec_resp.retcode, 0x20);
	/*
	 * tell adapter response in ASB
	 */
	MM_OUT(&trs->aca->set.isra_l, RESP_IN_ASB);
}

/*
 *  Interrupt handler for "adapter requires data to transmit" interrupt
 */
trxint(unit)
int unit;
{
	register struct tr_softc *trs = &tr_softc[unit];
	register union sr_arb *arb = trs->cb.arb;	/* pointer to ARB */
	register union sr_asb *asb = trs->cb.asb;	/* pointer to ASB */
	register unsigned char *dhb;			/* pointer to DHB */
	io_req_t	m0;
	register unsigned short size = 0;
	int i;
	unsigned char ri_present_byt;
	u_short rcf = 0;
	int ri_len;

	DEBUGF(tokdebug, printf("tr%d: trxint\n", unit);)
if (MM_IN(&asb->xmit_resp.retcode) != 0xff)
	printf("trxint: ASB IS NOT FREE!!!\n");

	/* load parameters into ASB */
	MM_OUT(&asb->xmit_resp.command, XMIT_UI_FRM);  
	MM_OUT(&asb->xmit_resp.cmd_corr, MM_IN(&arb->xmit.cmd_corr));
	MM_OUTW(&asb->xmit_resp.station_id, MM_INW(&arb->xmit.station_id));
	MM_OUT(&asb->xmit_resp.retcode, 0);
	MM_OUT(&asb->xmit_resp.rsap_value, EXTENDED_SAP);

if (MM_IN(&trs->cb.srb->xmit.command) == 0x0e) {
	MM_OUT(&asb->xmit_resp.command, 0x0e);	/* command is TRANSMIT.XID.CMD */
	MM_OUTW(&asb->xmit_resp.frame_len, 0x11);	/* length of frame */
	MM_OUT(&asb->xmit_resp.header_len, 0x0e);	/* length of TR header */
	MM_OUT(&asb->xmit_resp.rsap_value, EXTENDED_SAP);	/* destination SAP */
	/* get address of data holding buffer */
	dhb = (unsigned char *)(MM_INW(&arb->xmit.dhb_addr) + trs->sram);

	/* load dhb */
	MM_OUT(&dhb[0], AC);
	MM_OUT(&dhb[1], FC);
	/* load destination and source addresses */
	for (i=0; i < TR_ADDR_LEN; i++) {
		MM_OUT(&dhb[2 + i], 0xff);
		MM_OUT(&dhb[8 + i], 0x00);
	}
} else if (MM_IN(&trs->cb.srb->xmit.command) == 0x11) {
	MM_OUT(&asb->xmit_resp.command, 0x11);	/* command is TRANSMIT.TEST.CMD */
	MM_OUTW(&asb->xmit_resp.frame_len, 0x11);	/* length of frame */
	MM_OUT(&asb->xmit_resp.header_len, 0x0e);	/* length of TR header */
	MM_OUT(&asb->xmit_resp.rsap_value, EXTENDED_SAP);	/* destination SAP */
	/* get address of data holding buffer */
	dhb = (unsigned char *)(MM_INW(&arb->xmit.dhb_addr) + trs->sram);

	/* load dhb */
	MM_OUT(&dhb[0], AC);
	MM_OUT(&dhb[1], FC);
	/* load destination and source addresses */
	for (i=0; i < TR_ADDR_LEN; i++) {
		MM_OUT(&dhb[2 + i], 0xff);
		MM_OUT(&dhb[8 + i], 0x00);
	}
} else {

	/* if data in queue, copy mbuf chain to DHB */
	if (! IF_EMPTYQUEUE(trs->trs_if.if_snd)) {
		/* compute address of DHB */
		dhb = (unsigned char *)(MM_INW(&arb->xmit.dhb_addr) + trs->sram);
		
		/* pull packet off interface send queue, fill DHB */
		IF_DEQUEUE(&trs->trs_if.if_snd, m0);

		size = m0->io_count;
		DEBUGF(tokdebug, printf("tr%d: trxint: filling DHB\n", unit);)
		bcopy(m0->io_data, dhb+KVBASE, size);
		iodone(m0);

		DEBUGF(tokdebug & 2,{  		
		int i;
		unsigned char *dhbp = (unsigned char *)(MM_INW(&arb->xmit.dhb_addr) + trs->sram);
		printf("tr%d: trxint - contents of xmit buffer\n", unit);
		for (i=0; i < size; i++)
		printf("0x%x ",MM_IN(&dhbp[i]));
		} 
		) 

		/* check to see if there is routing info... */
		/* if yes, figure out length of the routing */
		/* info and update the ASB. Otherwise set   */
		/* the ASB length to 14                     */

		/* compute address of DHB */
		dhb = (unsigned char *)(MM_INW(&arb->xmit.dhb_addr) + trs->sram);

		ri_present_byt = MM_IN(&dhb[SOURCE_ADDR_BYT0]);


		if (ri_present_byt & TR_RI_PRESENT) {
			rcf = MM_IN(&dhb[RCF_BYT0]) << 8;
			ri_len = (rcf & TR_RCF_LEN_MASK) >> 8;
			MM_OUT(&asb->xmit_resp.header_len,  
				(HDR_LNGTH_NOROUTE + ri_len));
		} else
			MM_OUT(&asb->xmit_resp.header_len, HDR_LNGTH_NOROUTE);

		/* set size of transmission frame in ASB */
		MM_OUTW(&asb->xmit_resp.frame_len, size);
		
	} else {
		printf("tr%d: unexpected empty mbuf send queue\n", unit);
		
		/* set size of transmission frame in ASB to zero */
		MM_OUTW(&asb->xmit_resp.frame_len, 0);
	}
}
	/* tell adapter that there is a response in the ASB */
	MM_OUT(&trs->aca->set.isra_l, RESP_IN_ASB);
}

/*ARGSUSED*/
tropen(dev, flag)
dev_t	dev;
int	flag;
{
	register int	unit = minor(dev);

	if (unit < 0 || unit >= NTR || !tr_softc[unit].alive)
		return (ENXIO);
	
	DEBUGF(tokdebug, printf("tr%d: tropen\n", unit);)
	tr_softc[unit].trs_if.if_flags |= IFF_UP;
	trinit(unit);

	/* don't return until it's really open. */
	trsleep(unit);

	DEBUGF(tokdebug, printf("tr%d: tropen done.\n", unit);)
	return (0);
}

trnetstart(unit)
{
	register struct tr_softc *trs = &tr_softc[unit];

	if (trs->srbfree == 0 && trs->trs_ocount == 0)
		return(trstart(trs));

	DEBUGF(tokdebug, printf("tr%d: trnetstart: output already active\n",
		trs->trs_if.if_unit);)
}

troutput(dev, ior)
dev_t	dev;
io_req_t ior;
{
	register int	unit = minor(dev);

	if (unit < 0 || unit >= NTR || !tr_softc[unit].alive)
		return (ENXIO);
	
	DEBUGF(tokdebug, printf("tr%d: troutput request\n", unit);)
	return (net_write(&tr_softc[unit].trs_if, trnetstart, ior));
}

trsetinput(dev, receive_port, priority, filter, filter_count)
dev_t           dev;
mach_port_t     receive_port;
int             priority;
filter_t        filter[];
unsigned int    filter_count;
{
	register int unit = minor(dev);

	if (unit < 0 || unit >= NTR || !tr_softc[unit].alive)
		return (ENXIO);

	DEBUGF(tokdebug, printf("tr%d: trsetinput\n", unit);)
	return (net_set_filter(&tr_softc[unit].trs_if,
			receive_port, priority,
			filter, filter_count));
}

trgetstat(dev, flavor, status, count)
dev_t	dev;
int	flavor;
dev_status_t	status;		/* pointer to OUT array */
unsigned int	*count;		/* out */
{
	register int	unit = minor(dev);

	if (unit < 0 || unit >= NTR || !tr_softc[unit].alive)
		return (ENXIO);

	DEBUGF(tokdebug, printf("tr%d: trgetstat\n", unit);)
	return (net_getstat(&tr_softc[unit].trs_if, flavor, status, count));
}

trsetstat(dev, flavor, status, count)
dev_t	dev;
int	flavor;
dev_status_t	status;
unsigned int	count;
{
	register int	unit = minor(dev);
	register struct tr_softc	*sp;

	if (unit < 0 || unit >= NTR || !tr_softc[unit].alive)
		return (ENXIO);

	DEBUGF(tokdebug, printf("tr%d: trsetstat\n", unit);)
	sp = &tr_softc[unit];

	switch (flavor) {
	    case NET_STATUS:
	    {
#ifdef notdef
		/*
		 * All we can change are flags, and not many of those.
		 */
		register struct net_status *ns = (struct net_status *)status;
		int	mode = 0;

		if (count < NET_STATUS_COUNT)
		    return (D_INVALID_OPERATION);

		if (ns->flags & IFF_ALLMULTI)
		    mode |= MOD_ENAL;
		if (ns->flags & IFF_PROMISC)
		    mode |= MOD_PROM;

		/*
		 * Force a complete reset if the receive mode changes
		 * so that these take effect immediately.
		 */
		if (sp->mode != mode) {
		    sp->mode = mode;
		    if (sp->flags & DSF_RUNNING) {
			sp->flags &= ~(DSF_LOCK|DSF_RUNNING);
			trinit(unit);
		    }
		}
		break;
#endif
	    }

	    default:
		return (D_INVALID_OPERATION);
	}
	return (D_SUCCESS);

}

/*
 *  trbcopy - like bcopy except that it knows about the structure of
 *	      adapter receive buffers.
 */

trbcopy(trs, dest, len)
register struct tr_softc *trs;	/* pointer to softc struct for this adapter */
register unsigned char *dest;		/* destination address */
register int len;		/* number of bytes to copy */
{
	register int i;
	register struct rbcb *rbc = &trs->rbc;	/* pointer to rec buf ctl blk */

	/* while amount of data needed >= amount in current receive buffer */
	while (len >= rbc->data_len) {
		/* copy all data from receive buffer to destination */

		for (i = 0; i < rbc->data_len; i++)
			dest[i] = MM_IN(rbc->rbuf_datap + i);

		DEBUGF(tokdebug & 2,{
			int i;
			printf("trrint - packet data\n");
			for (i=0; i < rbc->data_len; i++)
				printf("0x%x ",*(dest + i));
			printf("\n");
		})
		len -= rbc->data_len;	/* update length left to transfer */
		dest += rbc->data_len;	/* update destination address */

		/* make next receive buffer current receive buffer */
		if (rbc->rbufp = rbc->rbufp_next) { /* more receive buffers? */

			/* calculate pointer to next receive buffer */
			if (MM_INW(&rbc->rbufp->buf_pointer))
				rbc->rbufp_next =
				(struct rec_buf *)(trs->sram +
				MM_INW(&rbc->rbufp->buf_pointer) - 2);
			else
				rbc->rbufp_next = 0;	/* at end of chain */
	
			/* get pointer to data in current receive buffer */
			rbc->rbuf_datap = rbc->rbufp->data;

			/* get length of data in current receive buffer */
			rbc->data_len = MM_INW(&rbc->rbufp->buf_len);
		} else {
			if (len != 0)	/* len should equal zero */
				printf("trbcopy: residual data not copied\n");
			return;
		}
	}

	/* amount of data needed is < amount in current receive buffer */

		for (i = 0; i < len; i++)
			dest[i] = MM_IN(rbc->rbuf_datap + i);

		DEBUGF(tokdebug & 2,{
			int i;
			printf("trrint - packet data\n");
			for (i=0; i < len; i++)
				printf("0x%x ",*(dest + i));
			printf("\n");
		})
	rbc->data_len -= len;	/* update count of data in receive buffer */
	rbc->rbuf_datap += len;	/* update pointer to receive buffer data */
	pc_copy_in += len;
}

/*
 *  trprinttraddr - print a token-ring address
 */
trprinttraddr(p)
register unsigned char *p;
{
	register int i;

	for (i = 0; i < TR_ADDR_LEN; i++) {
		if (i != 0) printf(":");
		printf("%x", *p++);
	}
}

/*
 *  tropensap - open the token ring SAP interface
 */
tropensap(unit,type) 
register int unit;
unsigned char type;
{
	register struct tr_softc *trs = &tr_softc[unit];
	register union sr_srb *srb = trs->cb.srb;

/************************************************************************
 ** To use the SAP level interface, we will have to execute a          ** 
 ** DLC.OPEN.SAP (pg.6-61 of the Token RIng Tech. Ref.) after we have  **
 ** received a good return code from the DIR.OPEN.ADAPTER command.     **
 ** We will open the IP SAP x'aa'.                                     **
 **                                                                    **
 ** STEPS:                                                             **
 **      1) Reset SRB response interrupt bit                           **
 **      2) Use the open_sap srb.                                      **
 **      3) Fill the following fields:                                 **
 **            command    - x'15'                                      **
 **            sap_value  - x'aa'                                      **
 **            sap_options- x'24'                                      **
 **                                                                    **
 ***********************************************************************/

	MM_OUT(&trs->aca->reset.isrp_l, ~(SRB_RESP_INT));

	MM_OUT(&srb->open_sap.command, OPEN_SAP);  
	MM_OUT(&srb->open_sap.retcode, 0x00);  
	MM_OUTW(&srb->open_sap.station_id, 0x0000);
        MM_OUT(&srb->open_sap.timer_t1, 0x00);
        MM_OUT(&srb->open_sap.timer_t2, 0x00);
        MM_OUT(&srb->open_sap.timer_ti, 0x00);
        MM_OUT(&srb->open_sap.maxout, 0x00);
        MM_OUT(&srb->open_sap.maxin, 0x00);
        MM_OUT(&srb->open_sap.maxout_incr, 0x00);
        MM_OUT(&srb->open_sap.maxretry, 0x00);
        MM_OUT(&srb->open_sap.gsapmaxmem, 0x00);
	MM_OUT(&srb->open_sap.sap_value, type);     
        MM_OUTW(&srb->open_sap.max_i_field, 0x0088);  
	MM_OUT(&srb->open_sap.sap_options, 0x24);
        MM_OUT(&srb->open_sap.station_cnt, 0x01);
        MM_OUT(&srb->open_sap.sap_gsap_mems, 0x00);

	MM_OUT(&trs->aca->set.isra_l, CMD_IN_SRB);
}

/*
 *  trsleep - sleep to wait for adapter to open
 */
trsleep(unit)
register int unit;
{
  	register struct tr_softc *trs = &tr_softc[unit];

  	timeout(trtimout,unit,hz*30);
  	sleep(&trs->trsleep_event, 1);
}

/*
 *  trtimout - timeout routine if adapter does not open in 30 seconds
 */
trtimout(unit)
register int unit;
{
  	register int error = 0;
  	register struct tr_softc *trs = &tr_softc[unit];

  	printf("Token Ring timeout\n");
  	wakeup(&trs->trsleep_event);
  	error = EIO;
  	return(error);
}

unsigned short
tr_swap(x)
	unsigned short x;
{
	return (((x & 0xff) << 8) | ((x & 0xff00) >> 8));
}

/*
 *  trdelay - delay for given number of milliseconds.
 *
 *  spinwait() is normally the routine that should be invoked to delay for a
 *  specified number of milliseconds.  However, this routine must only be
 *  called after the clock has been initialized, which is not done until after
 *  the device autoconfig stage.  trdelay() is provided for delays needed
 *  before spinwait() is useable.
 */
trdelay(millis)
int	millis;
{
	int	i, j;

	for (i = 0; i < millis; i++)
		for (j = 0; j < 100; j++)	/* 100 * 10microsecs = 1 ms */
			tenmicrosec();
}

#endif
