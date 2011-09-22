/* 
 * Mach Operating System
 * Copyright (c) 1994,1993 Carnegie Mellon University
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
 * $Log:	if_en.c,v $
 * Revision 2.2.1.1  94/02/17  14:10:00  mja
 * 	Added MACH_TTD support.
 * 	[94/01/02            zon]
 * 
 * Revision 2.2  93/11/17  16:53:33  dbg
 * 	Changed start() to return void.
 * 	[93/09/13            dbg]
 * 
 * 	Created, used if_pc586 as a reference.
 * 	[93/09/09            zon]
 * 
 */
#include <en.h>
#include <mach_ttd.h>

/* default: no debug messages */
#undef EN_DEBUG

/* default: thick ethernet */
#define EN_THIN_ETHERNET 0

/* default: no loopback */
#define EN_LOOPBACK 0

#include <kern/time_out.h>
#include <device/device_types.h>
#include <device/errno.h>
#include <device/io_req.h>
#include <device/if_hdr.h>
#include <device/if_ether.h>
#include <device/net_status.h>
#include <device/net_io.h>

#include <i386/ipl.h>
#include <mach/vm_param.h>
#include <vm/vm_kern.h>

#if MACH_TTD
#include <ttd/ttd_stub.h>
#endif

#include <i386ps2/bus.h>

#ifdef EN_DEBUG
/* if non-zero, print debugging information */
/* if > 1, print more information */
/* if > 2, print even more information */
int en_debug = EN_DEBUG;
#endif

/* if non-zero, configure for THIN ethernet, otherwise for THICK ethernet */
int en_thin_ethernet = EN_THIN_ETHERNET;

/* if non-zero, enable hardware loopback */
int en_loopback = EN_LOOPBACK;

/*****************************************************************************/

/*
 * LOCAL MEMORY MAP
 *
 * SCP  FFF6 -- FFFF, 10 BYTES
 * ISCP FFEE -- FFF5, 8 BYTES
 * SCB  FFDE -- FFED, 16 BYTES
 * CU   FFCC -- FFDD, 18 BYTES
 * TBD  FFC4 -- FFCB, 8 BYTES
 * TBUF F80E -- FFC3, 1974 BYTES
 * RBUF C320 -- F80D, 13550 BYTES (25 X 542)
 * RBD  C226 -- C31F, 250 BYTES (25 X 10)
 * FD   C000 -- C225, 550 BYTES (25 X 22)
 */

#define RBUF_SIZE 542 /* size of a receive buffer */

#define TBUF_SIZE 1974 /* size of transmit buffer */

#define MAX_FD  25 /* FD chain has 25 units */
#define MAX_RBD 25 /* RBD chain has 25 units */

#define ETHER_SIZE 6 /* size of ethernet address */

#define SCP   0xFFF6 /* local address of SCP  */
#define ISCP  0xFFEE /* local address of ISCP */
#define SCB   0xFFDE /* local address of SCB  */
#define CU    0xFFCC /* local address of CU   */
#define TBD   0xFFC4 /* local address of TBD  */
#define TBUF  0xF80E /* local address of TBUF */

/* local address of RBUF[i] */
#define RBUF(i) (0xC320 + ((i) * RBUF_SIZE))

/* local address of RBD[i] */
#define RBD(i) (0xC226 + ((i) * sizeof(struct rbd)))

/* local address of FD[i] */
#define FD(i) (0xC000 + ((i) * sizeof(struct fd)))
#define FD_INDEX(f) (((f) - FD(0)) / sizeof(struct fd))

typedef struct scp {
	unsigned short sysbus;
	unsigned short unused[2];
	unsigned short iscp;
	unsigned short iscp_base;
} *scp_t;

typedef struct iscp {
	unsigned short busy;
	unsigned short scb_offset;
	unsigned short scb;
	unsigned short scb_base;
} *iscp_t;

typedef struct scb {
	unsigned short status;
	unsigned short command;
	unsigned short cbl_offset;
	unsigned short rfa_offset;
	unsigned short crcerrs;
	unsigned short alnerrs;
	unsigned short rscerrs;
	unsigned short ovrnerrs;
} *scb_t;

typedef	struct transmit {
	unsigned short tbd_offset;
	unsigned char dest_addr[ETHER_SIZE];
	unsigned short length;
} *transmit_t;

typedef	struct configure {
	unsigned short fifolim_bytecnt;
	unsigned short addrlen_mode;
	unsigned short linprio_interframe;
	unsigned short slot_time;
	unsigned short hardware;
	unsigned short min_frame_len;
} *configure_t;

/* standard configuration parameters */
#define FIFOLIM_BYTECNT    0x080C
#define ADDRLEN_MODE       0x2600
#define LINPRIO_INTERFRAME 0x6000
#define SLOT_TIME          0xF200
#define HARDWARE           0x0000
#define MIN_FRAME_LEN      0x0040

typedef	struct ac {
	unsigned short status;
	unsigned short command;
	unsigned short link_offset;
	union {
		struct transmit transmit;
		struct configure configure;
		unsigned char iasetup[ETHER_SIZE];
	} cmd;
} *ac_t;

typedef struct tbd {
	unsigned short act_count;
	unsigned short next_tbd_offset;
	unsigned short buffer_addr;
	unsigned short buffer_base;
} *tbd_t;

typedef	struct fd {
	unsigned short status;
	unsigned short command;
	unsigned short link_offset;
	unsigned short rbd_offset;
	unsigned char destination[ETHER_SIZE];
	unsigned char source[ETHER_SIZE];
	unsigned short length;
} *fd_t;

typedef struct rbd {
	unsigned short status;
	unsigned short next_rbd_offset;
	unsigned short buffer_addr;
	unsigned short buffer_base;
	unsigned short size;
} *rbd_t;

#define SCB_SW_INT      0xF000
#define SCB_SW_CX       0x8000
#define SCB_SW_FR       0x4000
#define SCB_SW_CNA      0x2000
#define SCB_SW_RNR      0x1000

#define SCB_IDLE        0x0700
#define SCB_CUS_IDLE    0x0000
#define SCB_CUS_SUSPND  0x0100
#define SCB_CUS_ACTV    0x0200

#define SCB_RUS_IDLE    0x0000
#define SCB_RUS_SUSPND  0x0010
#define SCB_RUS_NORESRC 0x0020
#define SCB_RUS_READY   0x0040

#define SCB_ACK_CX      0x8000
#define SCB_ACK_FR      0x4000
#define SCB_ACK_CNA     0x2000
#define SCB_ACK_RNR     0x1000

#define SCB_CU_STRT     0x0100
#define SCB_CU_RSUM     0x0200
#define SCB_CU_SUSPND   0x0300
#define SCB_CU_ABRT     0x0400

#define SCB_RESET       0x0080

#define SCB_RU_STRT     0x0010
#define SCB_RU_RSUM     0x0020
#define SCB_RU_SUSPND   0x0030
#define SCB_RU_ABRT     0x0040

#define	AC_NOP          0x00
#define AC_IASETUP      0x01
#define AC_CONFIGURE    0x02
#define AC_MCSETUP      0x03
#define AC_TRANSMIT     0x04
#define AC_TDR          0x05
#define AC_DUMP         0x06
#define AC_DIAGNOSE     0x07

#define AC_SW_C         0x8000
#define AC_SW_B         0x4000
#define AC_SW_OK        0x2000
#define AC_SW_A         0x1000
#define AC_SW_FAIL      0x0800

#define	AC_CW_EL        0x8000
#define AC_CW_S         0x4000
#define AC_CW_I         0x2000

#define TC_CARRIER      0x0400
#define TC_CLS          0x0200
#define TC_DMA          0x0100
#define TC_DEFER        0x0080
#define TC_SQE          0x0040
#define TC_COLLISION    0x0020

#define TBD_SW_EOF      0x8000
#define TBD_SW_COUNT    0x3FFF

#define RBD_SW_EOF      0x8000
#define RBD_SW_COUNT    0x3FFF
#define RBD_EL          0x8000
#define RFD_DONE        0x8000
#define RFD_BUSY        0x4000
#define RFD_OK          0x2000
#define RFD_CRC         0x0800
#define RFD_ALN         0x0400
#define RFD_RSC         0x0200
#define RFD_DMA         0x0100
#define RFD_SHORT       0x0080
#define RFD_EOF         0x0040
#define RFD_EL          0x8000
#define RFD_SUSP        0x4000

/*****************************************************************************/

static int probe();
static int attach();
static int interrupt();

#if MACH_TTD
int en_get_packet();
int en_send_packet();
#endif

static struct i386_dev *info[NEN];

struct i386_driver endriver = { probe, 0, attach, "en", info, 0, 0 };

int (*enintrs[])() = { interrupt, 0 };

/* spl for network interrupts */
#define SPLNET spl6

/* POS identification for ethernet board */
#define EN_POS_ID 0x6042

/* do automatic reset if no interrupts for 10 seconds */
#define AUTOMATIC_RESET 10000

/* maximum local address (local address is 16 bits) */
#define MAX_ADDRESS 0x00010000

/* window to local memory is 16K bytes */
#define WINDOW_SIZE round_page(16 * 1024)

/* information from ethernet POS registers */
typedef struct pos {
	int card_enabled;          /* if non-zero, the card is enabled      */
	int thin_ethernet_enabled; /* if non-zero, thin ethernet is enabled */
	unsigned short io_base;    /* ethernet I/O register base            */
	unsigned long ram_base;    /* local memory window base              */
	int interrupt_level;       /* ethernet interrupt level (IRQ)        */
	int revision_level;        /* ethernet adapter revision level       */
	unsigned short register6;  /* address of "register 6"               */
} *pos_t;

/* ethernet device per-unit state information */
typedef struct en {
	int unit;                         /* unit number, index to en[]    */
	struct pos pos;                   /* POS information               */
	unsigned char bank;               /* current local memory bank     */
	unsigned short minimum_address;   /* minimum local memory address  */
	unsigned char *window;            /* window to local memory        */
	char network_address[ETHER_SIZE]; /* ethernet address              */
	int attached;                     /* if non-zero, unit is attached */
	int opened;                       /* if non-zero, unit is opened   */
	int busy;                         /* if non-zero, output is busy   */
#if MACH_TTD
	int kttd_enabled;                 /* if non-zero, kttd is enabled  */
#endif
	int interrupt_count;              /* automatic_reset() counter     */
	unsigned short fd;                /* (local) address of current fd */
	unsigned short end_fd;            /* (local) address of end fd     */
	unsigned short end_rbd;           /* (local) address of end rbd    */
	volatile scp_t scp;               /* pointer to SCP in bank #3     */
	volatile iscp_t iscp;             /* pointer to ISCP in bank #3    */
	volatile scb_t scb;               /* pointer to SCB in bank #3     */
	volatile ac_t cb;                 /* pointer to CB in bank #3      */
	volatile tbd_t tbd;               /* pointer to TBD in bank #3     */
	struct ifnet ifnet;               /* IFNET structure               */
} *en_t;

/* ethernet device per-unit state information */
static struct en en[NEN];

/*****************************************************************************/

/* print an ethernet address */
static print_ethernet_address(unsigned char *address)
{
	printf("%02X:%02X:%02X:%02X:%02X:%02X",
	       address[0],
	       address[1],
	       address[2],
	       address[3],
	       address[4],
	       address[5]);
}

/*****************************************************************************/

#define MAX_POS_SLOT      0x0008
#define POS_SELECT_PORT   0x0096
#define POS_SELECT_BASE   0x0008
#define POS_REGISTER_BASE 0x0100

/* read a POS register */
static unsigned char POS_read(int slot, int n)
{
	unsigned char value;

	outb(POS_SELECT_PORT, POS_SELECT_BASE + slot);
	value = inb(POS_REGISTER_BASE + n);
	outb(POS_SELECT_PORT, 0);

	return value;

}

/* write a POS register */
static void POS_write(int slot, int n, unsigned char value)
{

	outb(POS_SELECT_PORT, POS_SELECT_BASE + slot);
	outb(POS_REGISTER_BASE + n, value);
	outb(POS_SELECT_PORT, 0);

}

/* read POS identification in the adapter at the specified slot */
static unsigned short POS_read_slot_id(int slot)
{
	union {
		unsigned short word;
		unsigned char byte[2];
	} x;

	x.byte[0] = POS_read(slot, 0);
	x.byte[1] = POS_read(slot, 1);

	return x.word;

}

/* read POS register 'n' in the adapter at the specified slot */
static unsigned char POS_read_slot_data(int slot, int n)
{

	return POS_read(slot, n + 2);

}

/* write POS register 'n' in the adaptor at the specified slot */
static void POS_write_slot_data(int slot, int n, unsigned char value)
{

	POS_write(slot, n + 2, value);

}

/*****************************************************************************/

/* reset the ethernet adaptor, disable interrupt generation */
static void
	channel_reset(register en_t x)
{
	register volatile unsigned char *w = x->window;
	register volatile unsigned char ww;

#ifdef EN_DEBUG
	if (en_debug > 2)
		printf("EN%d CHANNEL RESET\n", x->unit);
#endif

	outb(x->pos.register6, 0x03);
	ww = *w;
	ww = *w;
	ww = *w;
	ww = *w;
	outb(x->pos.register6, 0x83);

	x->bank = 3;

}

/* enable the ethernet adaptor to generate interrupts */
/* interrupt generation is disabled after a channel_reset() */
static void
	enable_interrupts(register en_t x)
{

	outb(x->pos.register6, inb(x->pos.register6) | 0x04);

}

/* strobe the ethernet adaptor's channel attention */
static void
	channel_attention(register en_t x)
{
	register unsigned char register6 = inb(x->pos.register6);
	register volatile unsigned char *w = x->window;
	register volatile unsigned char ww;

#ifdef EN_DEBUG
	if (en_debug > 2)
		printf("EN%d CHANNEL ATTENTION\n", x->unit);
#endif

	outb(x->pos.register6, register6 | 0x40);
	ww = *w;
	ww = *w;
	outb(x->pos.register6, register6);

}

/* return an access pointer to local memory */
/*** this may move the window to a different bank ***/
/* NOTE: all interesting structures are in bank #3 (0xC000 - 0xFFFF) */
static unsigned char *
	window(register en_t x, register unsigned short address)
{
	register unsigned char bank = ((address) >> 14) & 3;

	if (address < x->minimum_address)
		panic("EN%d WINDOW ADDRESS 0x%04X < 0x%04X\n",
		       x->unit,
		       address,
		       x->minimum_address);

	if (bank != x->bank) {
		outb(x->pos.register6,
		     (inb(x->pos.register6) & 0xFC) | bank);
		x->bank = bank;
	}

	return &x->window[address & 0x3FFF];

}

/* check a bank of local memory, return zero if OK */
/* return address+1 if local memory error */
static int bank_check(register en_t x, unsigned short base)
{
	int i;
	int bank = (base >> 14) & 3;
	unsigned char *w = window(x, base);

	for (i = 0; i < 0x4000; i++)
		*window(x, base + i) = (i + bank) & 0xFF;

	for (i = 0; i < 0x4000; i++)
		if (*window(x, base + i) != ((i + bank) & 0xFF))
			return base + i + 1;

	return 0;

}

/*****************************************************************************/

/* probe for an ethernet device, return non-zero if one is found */
/* this routine reads all POS register information for the adaptor */
static int probe(addr, iod)
register caddr_t addr;
register struct i386_dev *iod;
{
	register en_t x = &en[iod->dev_unit];
	unsigned char register0;
	unsigned char register1;
	int slot;
	int unit;

	/* search for an adaptor for the specified device unit */
	unit = 0;
	for (slot = 0; slot < MAX_POS_SLOT; slot++) {
		if ((POS_read_slot_id(slot) == EN_POS_ID) &&
		    (unit++ == iod->dev_unit))
			break;
	}
	if (slot == MAX_POS_SLOT)
		return 0;

	/* set device unit (index to en[]) */
	x->unit = iod->dev_unit;

	/* read POS register #0 */
	register0 = POS_read_slot_data(slot, 0);

	/* read CARD_ENABLED bit */
	x->pos.card_enabled = register0 & 0x01;

	/* read/set THIN_ETHERNET_ENABLED bit */
	if (x->pos.thin_ethernet_enabled = !(register0 & 0x20)) {
		if (!en_thin_ethernet) {
			register0 |= 0x20;
			POS_write_slot_data(slot, 0, register0);
		}
	}
	else {
		if (en_thin_ethernet) {
			register0 &= ~0x20;
			POS_write_slot_data(slot, 0, register0);
		}
	}

	/* read IO_BASE */
	switch (register0 & 0x06) {
	case 0x00:
		x->pos.io_base = 0x0300;
		break;
	case 0x02:
		x->pos.io_base = 0x1300;
		break;
	case 0x04:
		x->pos.io_base = 0x2300;
		break;
	case 0x06:
		x->pos.io_base = 0x3300;
		break;
	}

	/* read RAM_BASE */
	switch (register0 & 0x18) {
	case 0x00:
		x->pos.ram_base = 0x000C0000;
		break;
	case 0x08:
		x->pos.ram_base = 0x000C8000;
		break;
	case 0x10:
		x->pos.ram_base = 0x000D0000;
		break;
	case 0x18:
		x->pos.ram_base = 0x000D8000;
		break;
	}

	/* read INTERRUPT_LEVEL */
	switch (register0 & 0xC0) {
	case 0x00:
		x->pos.interrupt_level = 12;
		break;
	case 0x40:
		x->pos.interrupt_level = 7;
		break;
	case 0x80:
		x->pos.interrupt_level = 3;
		break;
	case 0xC0:
		x->pos.interrupt_level = 9;
		break;
	}

	/* write INTERRUPT_LEVEL to cover POST routine bug */
	register1 = POS_read_slot_data(slot, 1);
	register1 &= 0xF0;
	switch (x->pos.interrupt_level) {
	case 12:
		register1 |= 0x01;
		break;
	case 7:
		register1 |= 0x02;
		break;
	case 3:
		register1 |= 0x04;
		break;
	case 9:
		register1 |= 0x08;
		break;
	}
	POS_write_slot_data(slot, 1, register1);

#ifdef EN_DEBUG
	if (en_debug > 2) {
		printf("ENPROBE: EN%d\n", x->unit);
		printf("         BOARD IS %s\n",
		       x->pos.card_enabled ? "ENABLED" : "DISABLED");
		printf("         THIN ETHERNET IS %s\n",
		       x->pos.thin_ethernet_enabled ? "ENABLED" : "DISABLED");
		printf("         IO BASE IS 0x%04X\n",
		       x->pos.io_base);
		printf("         RAM BASE IS 0x%08X\n",
		       x->pos.ram_base);
		printf("         INTERRUPT LEVEL IS %d\n",
		       x->pos.interrupt_level);
	}
#endif

	/* complain if the adaptor is not enabled */
	if (!x->pos.card_enabled)
		printf("EN%d: ETHERNET ADAPTOR IS NOT ENABLED.\n", x->unit);

	/* return non-zero if enabled ethernet adaptor has been found */
	return x->pos.card_enabled;

}

/* attach an ethernet device, return non-zero if success */
/* this routine sets up the device per-unit state information */
static int attach(iod)
register struct i386_dev *iod;
{
	register en_t x = &en[iod->dev_unit];
	register int i;
	int memory_size;

	/* cache the I/O address of adaptor register #6 */
	x->pos.register6 = x->pos.io_base + 6;

	/* get a pointer to the local memory window */
	x->window = (void *)phystokv(x->pos.ram_base);

	/* reset the ethernet adaptor */
	channel_reset(x);

	/* read the ethernet address */
	for (i = 0; i < ETHER_SIZE; i++)
		x->network_address[i] = inb(x->pos.io_base + i);

	/* display the ethernet address */
#ifdef EN_DEBUG
	if (en_debug > 2)
		printf("ENATTACH: EN%d\n"
		       "          ETHERNET ADDRESS ",
		       x->unit);
	else
#endif
	printf("EN%d: ", x->unit);
	print_ethernet_address(x->network_address);
	printf("\n");

	/* read the revision level from adaptor register #7 */
	x->pos.revision_level = inb(x->pos.io_base + 7) & 0x0F;

#ifdef EN_DEBUG
	if (en_debug > 2)
		printf("          REVISION LEVEL IS '%X'\n",
		       x->pos.revision_level);
#endif

	/* error if incorrect revision level */
	switch (x->pos.revision_level) {
	case 0x0F:
	case 0x0E:
		break;
	default:
		printf("EN%d: INCORRECT REVISION LEVEL '%X'\n",
		       x->unit,
		       x->pos.revision_level);
		return 0;
	}

	/* check local memory, determine size of local memory */
	/* error if can not access at least 16K of local memory */
	x->minimum_address = 0x0000;
	if (i = bank_check(x, 0xC000)) {
		printf("EN%d: CAN NOT ACCESS LOCAL MEMORY AT 0x%04x\n",
		       x->unit,
		       i - 1);
		return 0;
	}
	else if (bank_check(x, 0x8000)) {
		memory_size = 16;
		x->minimum_address = 0xC000;
	}
	else if (bank_check(x, 0x4000)) {
		memory_size = 32;
		x->minimum_address = 0x8000;
	}
	else if (bank_check(x, 0x0000)) {
		memory_size = 48;
		x->minimum_address = 0x4000;
	}
	else {
		memory_size = 64;
		x->minimum_address = 0x0000;
	}

#ifdef EN_DEBUG
	if (en_debug > 2)
		printf("          THERE ARE %dK BYTES OF LOCAL MEMORY\n",
		       memory_size);
#endif

	/* set up IFNET structure for this device unit */
	x->ifnet.if_unit = x->unit;
	x->ifnet.if_mtu = ETHERMTU;
	x->ifnet.if_flags = IFF_BROADCAST;
	x->ifnet.if_header_size = sizeof(struct ether_header);
	x->ifnet.if_header_format = HDR_ETHERNET;
	x->ifnet.if_address_size = 6;
	x->ifnet.if_address = x->network_address;
	if_init_queues(&x->ifnet);

	/* connect interrupt handler to the adaptor's IRQ */
	iod->dev_pic = x->pos.interrupt_level;
	iod->dev_spl = SPL4;
	take_dev_irq(iod);

	/* this ethernet device unit has been attached OK */
	x->attached = 1;

#if MACH_TTD
	if (x->kttd_enabled = !ttd_get_packet) {
#ifdef EN_DEBUG
		kttd_debug = en_debug;
#endif
		kttd_debug_init = TRUE;
		ttd_device_unit = x->unit;
		ttd_get_packet = en_get_packet;
		ttd_send_packet = en_send_packet;
		memcpy(ttd_host_ether_id.array,
		       x->network_address,
		       ETHER_SIZE);
		printf("*** USING ETHERNET FOR TTD ***\n");
	}
#endif

#ifdef EN_DEBUG
	if (en_debug > 2)
		printf("EN%d ATTACH: OK\n", x->unit);
#endif

	/* return non-zero if attached OK */
	return !x->attached;

}

/*****************************************************************************/

#ifdef EN_DEBUG

static void print_scp(en_t x, unsigned short scp_offset)
{
	scp_t scp = (scp_t)window(x, scp_offset);

	printf("SCP 0x%04X: SYSBUS=0x%04X ISCP=0x%04X ISCP_BASE=0x%04X\n",
	       scp_offset,
	       scp->sysbus,
	       scp->iscp,
	       scp->iscp_base);

}

static void print_iscp(en_t x, unsigned short iscp_offset)
{
	iscp_t iscp = (iscp_t)window(x, iscp_offset);

	printf("ISCP 0x%04X: BUSY=0x%04X SCB_OFFSET=0x%04X "
	       "SCB=0x%04X SCB_BASE=0x%04X\n",
	       iscp_offset,
	       iscp->busy,
	       iscp->scb_offset,
	       iscp->scb,
	       iscp->scb_base);

}

static void print_scb(en_t x, unsigned short scb_offset)
{
	scb_t scb = (scb_t)window(x, scb_offset);

	printf("SCB 0x%04X: STATUS=0x%04X COMMAND=0x%04X\n",
	       scb_offset,
	       scb->status,
	       scb->command);
	printf("            CBL_OFFSET=0x%04X RFA_OFFSET=0x%04X\n",
	       scb->cbl_offset,
	       scb->rfa_offset);
	printf("            ERRS 0x%04X 0x%04X 0x%04X 0x%04X\n",
	       scb->crcerrs,
	       scb->alnerrs,
	       scb->rscerrs,
	       scb->ovrnerrs);

}

static void print_ac(en_t x, unsigned short cb_offset)
{
	ac_t cb = (ac_t)window(x, cb_offset);

	printf("CB 0x%04X: STATUS=0x%04X COMMAND=0x%04X LINK_OFFSET=0x%04X\n",
	       cb_offset,
	       cb->status,
	       cb->command,
	       cb->link_offset);
	switch (cb->command & 0x0007) {
	case AC_NOP:
		printf("NOP\n");
		break;
	case AC_IASETUP:
		printf("IASETUP: ");
		print_ethernet_address(cb->cmd.iasetup);
		printf("\n");
		break;
	case AC_CONFIGURE:
		printf("CONFIGURE: %d %d %d %d %d %d\n",
		       cb->cmd.configure.fifolim_bytecnt,
		       cb->cmd.configure.addrlen_mode,
		       cb->cmd.configure.linprio_interframe,
		       cb->cmd.configure.slot_time,
		       cb->cmd.configure.hardware,
		       cb->cmd.configure.min_frame_len);
		break;
	case AC_MCSETUP:
		printf("MCSETUP\n");
		break;
	case AC_TRANSMIT:
		printf("TRANSMIT: 0x%04X ",
		       cb->cmd.transmit.tbd_offset);
		print_ethernet_address(cb->cmd.transmit.dest_addr);
		printf(" %d\n", cb->cmd.transmit.length);
		break;
	case AC_TDR:
		printf("TDR\n");
		break;
	case AC_DUMP:
		printf("DUMP\n");
		break;
	case AC_DIAGNOSE:
		printf("DIAGNOSE\n");
		break;
	}
}

static void print_fd(en_t x, unsigned short fd_offset)
{
	fd_t fd = (fd_t)window(x, fd_offset);

	printf("FD 0x%04X: STATUS=0x%04X COMMAND=0x%04X\n",
	       fd_offset,
	       fd->status,
	       fd->command);
	printf("           LINK=0x%04X RBD=0x%04X\n",
	       fd->link_offset,
	       fd->rbd_offset);
	printf("           DEST=");
	print_ethernet_address(fd->destination);
	printf("\n");
	printf("           SOURCE=");
	print_ethernet_address(fd->source);
	printf("\n");
	printf("           LENGTH=%d\n", fd->length);

}

#endif

/*****************************************************************************/

/* acknowledge any completed commands, return non-zero if error */
static int acknowledge(register en_t x, char *where)
{
	register int i;

	if (x->scb->command = (x->scb->status & SCB_SW_INT)) {

#ifdef EN_DEBUG
		if (en_debug > 2)
			printf("EN%d %s: ACKNOWLEDGE 0x%04X\n",
			       x->unit,
			       where,
			       x->scb->status & SCB_SW_INT);
#endif

		channel_attention(x);
		for (i = 0; i < 1000000; i++)
			if (!x->scb->command)
				break;
		if (x->scb->command) {
			printf("EN%d %s: 0x%04X NOT ACKNOWLEDGED\n",
			       x->unit,
			       where,
			       x->scb->command);
			return -1;
		}

	}

	return 0;

}

/* run a command, return non-zero if error */
static int
	run_command(register en_t x,
	            char *where,
	            char *what,
	            register int interrupt)
{
	int i;

	x->cb->status = 0;
	x->cb->command |= AC_CW_EL;
	x->cb->link_offset = CU;

	if (interrupt)
		x->cb->command |= AC_CW_I;

#ifdef EN_DEBUG
	if (en_debug > 2)
		print_ac(x, CU);
#endif

	if (interrupt)
		while (x->scb->command);

	x->scb->command = SCB_CU_STRT;

	channel_attention(x);

	if (interrupt)
		return 0;

	for(i = 0; i < 10000000; i++)
		if (x->cb->status & AC_SW_C)
			break;

	x->cb->command = AC_CW_EL;

	if (x->cb->status & AC_SW_B) {
		printf("EN%d %s: %s TIMEOUT\n", x->unit, where, what);
		return -1;
	}

	if (!(x->cb->status & AC_SW_C) ||
	    !(x->cb->status & AC_SW_OK)) {
		printf("EN%d %s: %s FAILED (0x%04X)\n",
		       x->unit,
		       where,
		       what,
		       x->cb->status);
		return -1;
	}

	if (acknowledge(x, where))
		return -1;

	return 0;

}

/* do a NOP command, return non-zero if error */
static int ac_nop(register en_t x, char *where)
{

	x->cb->command = AC_NOP;

	return run_command(x, where, "NOP", 0);

}

/* do a DIAGNOSE command, return non-zero if error */
static int ac_diagnose(register en_t x, char *where)
{

	x->cb->command = AC_DIAGNOSE;

	return run_command(x, where, "DIAGNOSE", 0);

}

/* do a CONFIGURE command, return non-zero if error */
static int
	ac_configure(register en_t x,
	             char *where,
	             unsigned short fifolim_bytecnt,
	             unsigned short addrlen_mode,
	             unsigned short linprio_interframe,
	             unsigned short slot_time,
	             unsigned short hardware,
	             unsigned short min_frame_len)
{

	x->cb->command = AC_CONFIGURE;
	x->cb->cmd.configure.fifolim_bytecnt = fifolim_bytecnt;
	x->cb->cmd.configure.addrlen_mode = addrlen_mode;
	x->cb->cmd.configure.linprio_interframe = linprio_interframe;
	x->cb->cmd.configure.slot_time = slot_time;
	x->cb->cmd.configure.hardware = hardware;
	x->cb->cmd.configure.min_frame_len = min_frame_len;

	return run_command(x, where, "CONFIGURE", 0);

}

/* do an IASETUP command, return non-zero if error */
static int
	ac_iasetup(register en_t x,
	           char *where,
	           unsigned char *network_address)
{

	x->cb->command = AC_IASETUP;

	memcpy(x->cb->cmd.iasetup, x->network_address, ETHER_SIZE);

	return run_command(x, where, "IASETUP", 0);

}

/*****************************************************************************/

/* build the FD chain */
static void build_fd_chain(register en_t x)
{
	register int i;

	for (i = 0; i < MAX_FD; i++) {
		register fd_t fd = (fd_t)window(x, FD(i));
		fd->status = 0;
		fd->command = 0;
		fd->rbd_offset = i ? 0xFFFF : RBD(0);
		if ((i + 1) < MAX_FD) {
			fd->link_offset = FD(i + 1);
		}
		else {
			fd->command |= AC_CW_EL;
			fd->link_offset = 0xFFFF;
		}
	}

	/* the first FD in the chain is the "current" one */
	x->fd = FD(0);

	/* the last FD in the chain is for recycle() */
	x->end_fd = FD(MAX_FD - 1);

}

/* build the RBD chain */
static void build_rbd_chain(register en_t x)
{
	register int i;

	for (i = 0; i < MAX_RBD; i++) {
		register rbd_t rbd = (rbd_t)window(x, RBD(i));
		rbd->status = 0;
		rbd->buffer_addr = RBUF(i);
		rbd->buffer_base = 0;
		rbd->size = RBUF_SIZE;
		if ((i + 1) < MAX_RBD)
			rbd->next_rbd_offset = RBD(i + 1);
		else {
			rbd->size |= AC_CW_EL;
			rbd->next_rbd_offset = 0xFFFF;
		}
	}

	/* the last RBD in the chain is for recycle() */
	x->end_rbd = RBD(MAX_RBD - 1);

}

/* start up the RECEIVE UNIT */
static void start_receive_unit(register en_t x)
{

	if ((x->scb->status & SCB_RUS_READY) == SCB_RUS_READY) {
#ifdef EN_DEBUG
		if (en_debug)
			printf("EN%d START_RECEIVE_UNIT: READY FD 0x%04X\n",
			       x->unit,
			       x->fd);
#endif
		return;
	}

	build_fd_chain(x);

	build_rbd_chain(x);

#ifdef EN_DEBUG
	if (en_debug)
		printf("EN%d START_RECEIVE_UNIT: FD 0x%04X\n",
		       x->unit,
		       x->fd);
#endif

	x->scb->rfa_offset = x->fd;
	x->scb->command = SCB_RU_STRT;

	channel_attention(x);

}

/* transmit a packet frame */
static void transmit(register en_t x, char *data, int count)
{
	unsigned char *tbuf;
	struct ether_header *eh = (struct ether_header *)data;

	if (!x->opened)
		panic("EN%d TRANSMIT: NOT OPEN", x->unit);

#ifdef EN_DEBUG
	if (en_debug) {
		printf("EN%d TRANSMIT: DHOST ", x->unit);
		print_ethernet_address(eh->ether_dhost);
		printf("\n");
	}
#endif

	count -= sizeof(struct ether_header);

	tbuf = window(x, TBUF);
	memcpy(tbuf, data + sizeof(struct ether_header), count);
	if (count < ETHERMIN)
		bzero(&tbuf[count], ETHERMIN - count);

	/* move window back to bank #3 */
	(void)window(x, 0xFFFF);

	/* set up TBD */
	x->tbd->act_count = (count < ETHERMIN) ? ETHERMIN : count;
	x->tbd->next_tbd_offset = 0xFFFF;
	x->tbd->buffer_addr = TBUF;
	x->tbd->buffer_base = 0;
	x->tbd->act_count |= TBD_SW_EOF;

	/* set up CB with TRANSMIT command */
	x->cb->command = AC_TRANSMIT;
	x->cb->cmd.transmit.tbd_offset = TBD;
	x->cb->cmd.transmit.length = (unsigned short)eh->ether_type;
	memcpy(x->cb->cmd.transmit.dest_addr,
	       eh->ether_dhost,
	       ETHER_SIZE);

	/* run TRANSMIT command, interrupt when complete */
	run_command(x, "TRANSMIT", "TRANSMIT", 1);

}

/* start output */
static void start(int unit)
{
	register en_t x = &en[unit];
	io_req_t m;

	/* error if ethernet device is not UP and RUNNING */
	if (!x->opened) {
		printf("EN%d START: NOT OPEN\n", unit);
		IF_DEQUEUE(&x->ifnet.if_snd, m);
		if (m)
			iodone(m);
		return;
	}

	/* if currently busy with output, do nothing */
	if (x->busy && (x->scb->status & SCB_IDLE)) {
#ifdef EN_DEBUG
		if (en_debug)
			printf("EN%d START: BUSY\n", x->unit);
#endif
		return;
	}
	x->busy = 0;

	/* take the I/O request out of the queue */
	IF_DEQUEUE(&x->ifnet.if_snd, m);

	/* if there is an I/O request, transmit the packet */
	if (m) {
#ifdef EN_DEBUG
		if (en_debug)
			printf("EN%d START: OK\n", unit);
#endif
		x->busy++;
		transmit(x, m->io_data, m->io_count);
		iodone(m);
	}
#ifdef EN_DEBUG
	else if (en_debug)
		printf("EN%d START: EMPTY\n", x->unit);
#endif

}

/* forward declaration of reset() */
static void reset(register en_t x, char *where);

/* automatic reset if there is no interrupt activity */
static automatic_reset(x)
en_t x;
{

	/* if ethernet device is not open, do nothing */
	if (!x->opened)
		return;

	/* if there has been no interrupt activity, reset() */
	if (!x->interrupt_count)
		reset(x, "AUTOMATIC_RESET");
	/* otherwise, clear the counter and set up the timeout() */
	else {
		x->interrupt_count = 0;
		(void)untimeout(automatic_reset, x);
		timeout(automatic_reset, x, AUTOMATIC_RESET);
	}

}

/* reset the ethernet device, set UP and RUNNING if success */
static void reset(register en_t x, char *where)
{
	int i;
	spl_t s;

#ifdef EN_DEBUG
	if (en_debug)
		printf("EN%d RESET (%s)\n", x->unit, where);
#endif

#if MACH_TTD
	/* interrupts not allowed if ttd is enabled */
	if (!x->kttd_enabled)
#endif
	s = SPLNET();

	/* do hardware reset */
	channel_reset(x);

	/* enable hardware loopback if required */
	if (en_loopback)
		outb(x->pos.register6, inb(x->pos.register6) | 0x20);

	/* clear all local memory */
	for (i = x->minimum_address; i < MAX_ADDRESS; i += 0x4000)
		bzero(window(x, i), 0x4000);

	/* set up SCP */
	x->scp = (scp_t)window(x, SCP);
	x->scp->sysbus = 0;
	x->scp->iscp = ISCP;
	x->scp->iscp_base = 0;
#ifdef EN_DEBUG
	if (en_debug > 2)
		print_scp(x, SCP);
#endif

	/* set up ISCP */
	x->iscp = (iscp_t)window(x, ISCP);
	x->iscp->busy = 1;
	x->iscp->scb_offset = SCB;
	x->iscp->scb = 0;
	x->iscp->scb_base = 0;
#ifdef EN_DEBUG
	if (en_debug > 2)
		print_iscp(x, ISCP);
#endif

	/* set up SCB */
	x->scb = (scb_t)window(x, SCB);
	x->scb->status = 0;
	x->scb->command = 0;
	x->scb->cbl_offset = CU;
	x->scb->rfa_offset = FD(0);
	x->scb->crcerrs = 0;
	x->scb->alnerrs = 0;
	x->scb->rscerrs = 0;
	x->scb->ovrnerrs = 0;
#ifdef EN_DEBUG
	if (en_debug > 2)
		print_scb(x, SCB);
#endif

	/* set up CB pointer */
	x->cb = (ac_t)window(x, CU);

	/* set up TBD pointer */
	x->tbd = (tbd_t)window(x, TBD);

	/* issue SCB_RESET */
	x->scb->command = SCB_RESET;
	channel_attention(x);

	/* error if SCB_RESET timeout */
	for (i = 0; i < 1000000; i++)
		if (!x->iscp->busy)
			break;
	if (x->iscp->busy) {
		printf("EN%d RESET: SCB_RESET TIMEOUT\n", x->unit);
		goto error;
	}

	/* error if not ready after SCB_RESET */
	for (i = 0; i < 1000000; i++)
		if (x->scb->status == (SCB_SW_CX | SCB_SW_CNA))
			break;
	if (x->scb->status != (SCB_SW_CX | SCB_SW_CNA)) {
		printf("EN%d RESET: SCB_RESET NOT READY\n", x->unit);
		goto error;
	}

	/* acknowledge the SCB_RESET */
	(void)acknowledge(x, "RESET");

	/* do a NOP, error if error */
	if (ac_nop(x, "RESET"))
		goto error;

	/* do a DIAGNOSE, error if error */
	if (ac_diagnose(x, "RESET"))
		goto error;

	/* do a CONFIGURE, error if error */
	if (ac_configure(x,
	                 "RESET",
	                 FIFOLIM_BYTECNT,
	                 ADDRLEN_MODE,
	                 LINPRIO_INTERFRAME,
	                 SLOT_TIME,
	                 HARDWARE,
	                 MIN_FRAME_LEN))
		goto error;

	/* do an IASETUP, error if error */
	if (ac_iasetup(x, "RESET", x->network_address))
		goto error;

	/* ethernet device is open for business */
	x->ifnet.if_flags |= IFF_UP | IFF_RUNNING;
	x->opened = 1;

	/* clear start() busy flag */
	x->busy = 0;

	/* set automatic reset timer */
	x->interrupt_count = 1;
	automatic_reset(x);

	/* allow the adaptor to generate interrupts */
#if MACH_TTD
	/* interrupts not allowed if ttd is enabled */
	if (!x->kttd_enabled)
#endif
	enable_interrupts(x);

	/* start up the RECEIVE UNIT */
	start_receive_unit(x);

	/* start up the transmitter */
	start(x->unit);

	/* all done */
#if MACH_TTD
	/* interrupts not allowed if ttd is enabled */
	if (!x->kttd_enabled)
#endif
	splx(s);
	return;

	/* reset has failed */
error:

	/* do a channel_reset() to turn off the adaptor */
	channel_reset(x);

	/* ethernet device is not available */
	x->ifnet.if_flags &= ~(IFF_RUNNING | IFF_UP);
	x->opened = 0;

	/* complain that the reset failed */
	printf("EN%d %s: RESET FAILED\n", x->unit, where);

	/* all done */
#if MACH_TTD
	/* interrupts not allowed if ttd is enabled */
	if (!x->kttd_enabled)
#endif
	splx(s);

}

/* recycle the current receive frame descriptor and its receive buffers */
/* return a pointer to the current FD, return zero if error */
static fd_t recycle(register en_t x, unsigned short end_rbd)
{
	fd_t fd;
	unsigned short next_fd;
	volatile rbd_t rbd;

#ifdef EN_DEBUG
	if (en_debug > 1)
		printf("EN%d RECYCLE: FD[%d]\n", x->unit, FD_INDEX(x->fd));
#endif

	/* get a pointer to the current frame descriptor */
	fd = (fd_t)window(x, x->fd);

	/* if there are receive buffers to recycle... */
	if (fd->rbd_offset != 0xFFFF) {

		/* get a pointer to the new end of the free list */
		if (end_rbd == 0xFFFF)
			end_rbd = fd->rbd_offset;
		for (;;) {
			rbd = (rbd_t)window(x, end_rbd);
			if (rbd->status & RBD_SW_EOF)
				break;
			if ((end_rbd = rbd->next_rbd_offset) == 0xFFFF) {
				printf("EN%d RECYCLE: FD[%d] BAD RBD CHAIN\n",
				       x->unit,
				       FD_INDEX(x->fd));
				return 0;
			}
		}

		/* terminate the new end of the free list */
		rbd->status = 0;
		rbd->next_rbd_offset = 0xFFFF;
		rbd->size |= RBD_EL;

		/* connect to the end of the free list */
		rbd = (rbd_t)window(x, x->end_rbd);
		rbd->next_rbd_offset = fd->rbd_offset;
		rbd->size &= ~RBD_EL;

		/* note the new end of the free list */
		x->end_rbd = end_rbd;

	}

	/* get a pointer to the current frame descriptor */
	fd = (fd_t)window(x, x->fd);

	/* error if FD chain is broken */
	if ((next_fd = fd->link_offset) == 0xFFFF) {
		printf("EN%d RECYCLE: FD[%d] BAD FD CHAIN\n",
		       x->unit,
		       FD_INDEX(x->fd));
		return 0;
	}

	/* clear the fields for this FD structure */
	fd->status = 0;
	fd->command = AC_CW_EL;
	fd->link_offset = 0xFFFF;
	fd->rbd_offset = 0xFFFF;

	/* attach to the end of the free list */
	fd = (fd_t)window(x, x->end_fd);
	fd->link_offset = x->fd;
	fd->command = 0;
	x->end_fd = x->fd;

	/* make the next FD the current FD */
	return (fd_t)window(x, x->fd = next_fd);

}

/* receive a packet, return zero if no packet received */
static ipc_kmsg_t receive(register en_t x, ipc_kmsg_t m)
{
	volatile fd_t fd;
	volatile rbd_t rbd;
	ipc_kmsg_t allocated;
	struct ether_header *e;
	struct packet_header *p;
	char *d;
	unsigned short n;
	unsigned short rbd_address;

	if (!x->opened)
		panic("EN%d RECEIVE: NOT OPEN", x->unit);

	/* get a pointer to the current frame descriptor */
	fd = (fd_t)window(x, x->fd);

	/* while there is a completed frame to process... */
	while (fd->status & RFD_DONE) {

		/* if the frame is not OK, complain and drop the packet */
		if (!(fd->status & RFD_OK)) {
			printf("EN%d RECEIVE: FD[%d] STATUS 0x%04X\n",
			       x->unit,
			       FD_INDEX(x->fd),
			       fd->status & 0x0FFF);
			x->ifnet.if_ierrors++;
			if (!(fd = recycle(x, 0xFFFF)))
				goto error;
			continue;
		}

		/* if can't get a message buffer, complain and drop packet */
		allocated = 0;
		if (!m && ((allocated = m = net_kmsg_get()) == IKM_NULL)) {
#ifdef EN_DEBUG
			if (en_debug)
				printf("EN%d RECEIVE: "
				       "FD[%d] NET_KMSG_GET FAILED\n",
				       x->unit,
				       FD_INDEX(x->fd));
#endif
			x->ifnet.if_rcvdrops++;
			if (!(fd = recycle(x, 0xFFFF)))
				goto error;
			continue;
		}

#ifdef EN_DEBUG
		if (en_debug) {
			printf("EN%d RECEIVE: FD[%d] ",
			       x->unit,
			       FD_INDEX(x->fd));
			print_ethernet_address(fd->source);
			printf(" => ", x->unit);
			print_ethernet_address(fd->destination);
			printf("\n");
		}
#endif

		/* get pointers to the ethernet header and packet data */
		e = (struct ether_header *)(&net_kmsg(m)->header[0]);
		p = (struct packet_header *)(&net_kmsg(m)->packet[0]);
		d = (char *)(p + 1);
		p->type = e->ether_type = fd->length;
		p->length = sizeof(struct packet_header);

		/* set ether_type, ether_shost and ether_dhost */
		memcpy(e->ether_shost, fd->source, ETHER_SIZE);
		memcpy(e->ether_dhost, fd->destination, ETHER_SIZE);

		/* error if there is no receive buffer */
		if ((rbd_address = fd->rbd_offset) == 0xFFFF) {
			printf("EN%d RECEIVE: FD[%d] MISSING RBD\n",
			       x->unit,
			       FD_INDEX(x->fd));
			x->ifnet.if_rcvdrops++;
			goto error;
		}

		/* copy each receive buffer into the message buffer... */
		for (;;) {

			/* get pointer to receive buffer descriptor */
			rbd = (rbd_t)window(x, rbd_address);

			/* copy receive buffer to message buffer */
			n = rbd->status & RBD_SW_COUNT;
			memcpy(d, window(x, rbd->buffer_addr), n);
			d += n;

			/* accumulate packet length */
			p->length += n;

			/* get pointer to receive buffer descriptor */
			rbd = (rbd_t)window(x, rbd_address);

			/* all done if end of receive buffer chain */
			if (rbd->status & RBD_SW_EOF)
				break;

			/* go to the next buffer, error if chain is broken */
			if ((rbd_address = rbd->next_rbd_offset) == 0xFFFF) {
				printf("EN%d RECEIVE: FD[%d] BAD RBD CHAIN\n",
				       x->unit,
				       FD_INDEX(x->fd));
				x->ifnet.if_rcvdrops++;
				goto error;
			}

		}

		/* all done, recycle this frame descriptor and its buffers */
		if (!(fd = recycle(x, rbd_address)))
			goto error;

		/* all done, no error, return message */
		return m;

	}

	/* all done, there is no message to return */
	return 0;

error:

	/* if a message buffer was allocated, put it back */
	if (allocated)
		net_kmsg_put(allocated);

	/* reset */
	reset(x, "RECEIVE");

	/* failed, return zero */
	return 0;

}

/*****************************************************************************/

/* handle an ethernet adaptor interrupt */
static int interrupt(unit)
int unit;
{
	register volatile en_t x = &en[unit];
	unsigned short type;
	ipc_kmsg_t m;
	struct packet_header *p;

#ifdef EN_DEBUG
	if (en_debug)
		printf("EN%d INTERRUPT 0x%04X\n",
		       x->unit,
		       x->scb->status & SCB_SW_INT);
#endif

#if MACH_TTD
	/* interrupts not allowed if ttd is enabled */
	if (x->kttd_enabled)
		panic("EN%d INTERRUPT: TTD ENABLED\n", unit);
#endif

	/* if not open, complain then do nothing */
	if (!x->opened) {
		printf("EN%d INTERRUPT: NOT OPEN\n", unit);
		return -1;
	}

	/* count this interrupt for automatic_reset() */
	x->interrupt_count++;

	/* while there are interrupts to process... */
	while (type = (x->scb->status & SCB_SW_INT)) {

		/* acknowledge the interrupting command(s) */
		acknowledge(x, "INTERRUPT");

		/* if a frame has been received, process it (them) */
		if (type & SCB_SW_FR)
			while (m = receive(x, 0)) {
				p = (struct packet_header *)
					(&net_kmsg(m)->packet[0]);
				net_packet(&x->ifnet,
					   m,
					   p->length,
					   ethernet_priority(m));
			}

		/* if the RECEIVE UNIT went off-line, restart it */
		if (type & SCB_SW_RNR)
			start_receive_unit(x);

		/* if a packet has been transmitted, try for another */
		if (type & SCB_SW_CX) {
			x->busy = 0;
			start(unit);
		}

	}

	/* return zero, nothing looks at this return value anyway... */
	return 0;

}

/*****************************************************************************/

#if MACH_TTD

int en_get_packet(unit)
int unit;
{
	en_t x = &en[unit];

	/* crash if TTD not enabled */
	if (!x->kttd_enabled)
		panic("EN%d EN_GET_PACKET: KTTD NOT ENABLED\n");

	/* if not open, do a reset() */
	if (!x->opened) {
		reset(x, "EN_GET_PACKET");
		if (!x->opened)
			panic("EN%d EN_GET_PACKET: RESET FAILED\n", unit);
	}

	/* loop until a packet is received */
	while (!receive(x, (ipc_kmsg_t)ttd_request_msg));

	/* acknowledge any completed commands */
	(void)acknowledge(x, "EN_GET_PACKET");

	return 0;

}

int en_send_packet(unit, packet, len)
int unit;
char *packet;
int len;
{
	en_t x = &en[unit];

	/* crash if TTD not enabled */
	if (!x->kttd_enabled)
		panic("EN%d EN_SEND_PACKET: KTTD NOT ENABLED\n", unit);

	/* if not open, do a reset() */
	if (!x->opened) {
		reset(x, "EN_GET_PACKET");
		if (!x->opened)
			panic("EN%d EN_SEND_PACKET: RESET FAILED\n", unit);
	}

	/* transmit the packet */
	transmit(x, packet, len);

	/* acknowledge any completed commands */
	(void)acknowledge(x, "EN_SEND_PACKET");

	return 0;

}

#endif

/*****************************************************************************/

/* return en structure pointer for specified ethernet device */
/* return zero if bad unit number or device not UP */
static en_t dev_to_en(register dev_t dev, char *name)
{
	register unsigned int unit = minor(dev);
	register en_t x;

#ifdef EN_DEBUG
	if (en_debug)
		printf("EN%d %s\n", unit, name);
#endif

	if ((unit < NEN) && (x = &en[unit])->opened)
		return x;

	return 0;

}

/* open an ethernet device */
io_return_t
	enopen(dev_t dev, dev_mode_t mode, io_req_t ior)
{
	unsigned int unit = minor(dev);
	register en_t x;

#ifdef EN_DEBUG
	if (en_debug)
		printf("EN%d OPEN\n", unit);
#endif

	/* error if bad unit number or device not attached */
	if ((unit >NEN) || !(x = &en[unit])->attached)
		return D_NO_SUCH_DEVICE;

#ifdef EN_DEBUG
	if (en_debug > 2) {
		int i, base;
		unsigned char c;
		printf("    BOARD IS %s\n",
		       x->pos.card_enabled ? "ENABLED" : "DISABLED");
		printf("    THIN ETHERNET IS %s\n",
		       x->pos.thin_ethernet_enabled ? "ENABLED" : "DISABLED");
		printf("    IO BASE IS 0x%04X\n",
		       x->pos.io_base);
		printf("    RAM BASE IS 0x%08X\n",
		       x->pos.ram_base);
		printf("    INTERRUPT LEVEL IS %d\n",
		       x->pos.interrupt_level);
		printf("    ETHERNET ADDRESS ");
		print_ethernet_address(x->network_address);
		printf("\n");
		printf("    ETHERNET BOARD REVISION LEVEL IS '%X'\n",
		       x->pos.revision_level);
		printf("    THE MINIMUM ADDRESS IS 0x%04X\n",
		       x->minimum_address);
		for (base = x->minimum_address;
		     base < MAX_ADDRESS;
		     base += 0x4000)
			if (i = bank_check(x, base))
				break;
		if (i)
			printf("    CAN NOT ACCESS BYTE AT 0x%04X\n", i - 1);
		else
			printf("    RAM ACCESS OK\n");
	}
#endif

	/* reset the ethernet device */
	reset(x, "ENOPEN");

	/* success if the device is open */
	if (x->opened)
		return D_SUCCESS;

	/* error if the reset() failed */
	return D_IO_ERROR;

}

/* output to an ethernet device */
io_return_t
	enoutput(dev_t dev, io_req_t ior)
{
	register en_t x = dev_to_en(dev, "ENOUTPUT");

	/* error if no such device */
	if (!x)
		return D_NO_SUCH_DEVICE;

	/* use the generic net_write() */
	return net_write(&x->ifnet, start, ior);

}

/* set the input filter for an ethernet device */
io_return_t
	ensetinput(dev_t dev,
	           mach_port_t receive_port,
	           int priority,
	           filter_t *filter,
	           unsigned int filter_count)
{
	register en_t x = dev_to_en(dev, "ENSETINPUT");

	/* error if no such device */
	if (!x)
		return D_NO_SUCH_DEVICE;

	/* use the generic net_set_filter() */
	return net_set_filter(&x->ifnet,
	                      receive_port,
	                      priority,
	                      filter,
	                      filter_count);

}

/* get status for an ethernet device */
io_return_t
	engetstat(dev_t dev,
	          int flavor,
	          dev_status_t status,
	          unsigned int *count)
{
	register en_t x = dev_to_en(dev, "ENGETSTAT");

	/* error if no such device */
	if (!x)
		return D_NO_SUCH_DEVICE;

	/* use the generic net_getstat() */
	return net_getstat(&x->ifnet, flavor, status, count);

}

/* set status for an ethernet device */
io_return_t
	ensetstat(dev_t dev,
	          int flavor,
	          dev_status_t status,
	          unsigned int count)
{
	register en_t x = dev_to_en(dev, "ENSETSTAT");
	register struct net_status *ns = (struct net_status *)status;

	/* error if no such device */
	if (!x)
		return D_NO_SUCH_DEVICE;

	/* do the right thing... */
	switch (flavor) {
	case NET_STATUS:
		if (count != NET_STATUS_COUNT)
			return D_INVALID_OPERATION;
#ifdef EN_DEBUG
		if (en_debug)
			printf("EN%d SETSTAT: NET_STATUS 0x%08X\n",
			       x->unit,
			       ns->flags);
#endif
		reset(x, "ENSETSTAT");
		break;
	default:
		return D_INVALID_OPERATION;
	}

	return D_SUCCESS;

}
