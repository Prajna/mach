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
 * $Log:	kd_abios.c,v $
 * Revision 2.2  93/02/04  08:00:50  danner
 * 	Integrate PS2 code from IBM.
 * 	[93/01/18            prithvi]
 * 
 */

/* ported --------------------------------------------------------------*/

#define KBD_DATA_PORT				0x60
#define KBD_STATUS_PORT				0x64
#define OUTPUT_BUFFER_FULL			BIT0
#define INPUT_BUFFER_FULL			BIT1
#define KBD_TX					BIT6
#define KBD_RESET_CMD				0xff
#define KBD_ACK					0xfa
#define KBD_BAT_RC				0xaa
#define CTLR_READ_CMD_BYTE			0x20
#define CTLR_WRITE_CMD_BYTE			0x60

#define K_pad4SC   0x4b

static void
kbd_wait( int i)
{
	while (i) i--;
}

static u_char
kbd_read_data_port()
{
	while ((inb(KBD_STATUS_PORT) & OUTPUT_BUFFER_FULL) == 0); /* wait */
	/*
	 * Hardware ref manual says we need to wait 7 usec before the
	 * data byte would show up at KBD_DATA_PORT.  Wait a little
	 * longer just to be sure.
	 */
	kbd_wait(7+3);			
	return(inb(KBD_DATA_PORT));
}

static void
kbd_write_port(int port, u_char val)
{
	while (inb(KBD_STATUS_PORT) & (INPUT_BUFFER_FULL | OUTPUT_BUFFER_FULL));
	outb(port, val);
}

static void
kbd_clr_hw_buffer()
{
	/*
	 * get rid of any characters may have left in the keyboard
	 * hardware buffer (just want to start out clean)
	 */
	while (inb(KBD_STATUS_PORT) & OUTPUT_BUFFER_FULL)
		kbd_read_data_port();
} /* kbd_clr_hw_buffer */

/*
 * kbd_tx_off
 *
 * Turn off keyboard controler scan code translation.  For PC compatability,
 * type 1 kbd controler will do scan code translation, which means you can
 * only use scan code set 1.  To use scan code set 3, we need to turn the
 * translation off.
 *
 * It goes like this:
 * outb(0x64, 0x20)			read controler command byte
 * ch = inb(0x60) & ~BIT6		turn off kbd translate
 * outb(0x64, 0x60)			write controler command byte
 * outb(0x60, ch)
 *
 * This routine is called very early in main().  After hardware init
 * but before debugger init.
 */
void
kbd_tx_off()
{
	u_char	ch;

	kbd_clr_hw_buffer();
	kbd_write_port(KBD_STATUS_PORT, CTLR_READ_CMD_BYTE);
	ch = kbd_read_data_port() & (~KBD_TX);	/* turn off kbd translate */
	kbd_write_port(KBD_STATUS_PORT, CTLR_WRITE_CMD_BYTE);
	kbd_write_port(KBD_DATA_PORT, ch);	/* write it back */

} /* kbd_tx_off */

void
kbd_tx_on()
{
	u_char	ch;

	kbd_clr_hw_buffer();
	kbd_write_port(KBD_STATUS_PORT, CTLR_READ_CMD_BYTE);
	ch = kbd_read_data_port() | KBD_TX;	/* turn on kbd translate */
	kbd_write_port(KBD_STATUS_PORT, CTLR_WRITE_CMD_BYTE);
	kbd_write_port(KBD_DATA_PORT, ch);	/* write it back */

} /* kbd_tx_on */

/*
 * kbd_debugger_init
 *
 * Init the keyboard for the kernel debugger.  At power on, kbd
 * default to scan code set 2.  We want to use scan code set 3.
 * So some juggling is needed here.  Can't use abios here since
 * this is done very early in the boot process.  In the normal
 * boot (without debugger), this is done in ktsopen().  It goes
 * something like this:
 *
 *					cmds		ack	rc good/bad
 *	reset kbd			0xff		0xfa	0xaa/0xfc
 *	select scan code set 3		0xf0,0x03	0xfa,0xfa
 *	set all key t/m/b		0xfa		0xfa
 *	set individual key m/b		0xfc		0xfa
 *	sc3 for left-alt		0x19		0xfa
 *	sc3 for right-alt		0x39		0xfa
 *	sc3 for left-ctrl		0x11		0xfa
 *	sc3 for action (right-ctrl)	0x58		0xfa
 *	sc3 for left-shift		0x12		0xfa
 *	sc3 for right-shift		0x59		0xfa
 *	sc3 for capslock		0x14		0xfa
 *	sc3 for numlock			0x76		0xfa
 *	enable kbd			0xf4		0xfa
 */
u_char debugger_kbdcmds[] = {
			0xf0,		/* select alternate scan code */
			0x03,		/* scan code set 3 */
			0xfa,		/* set all key t/m/b */
			0xfc,		/* set individual key m/b */
			0x19,		/* sc3 for left-alt */
			0x39,		/* sc3 for right-alt*/
			0x11,		/* sc3 for left-ctrl */
			0x58,		/* sc3 for action (right-ctrl) */
			0x12,		/* sc3 for left-shift */
			0x59,		/* sc3 for right-shift */
			0x14,		/* sc3 for capslock */
			0x76,		/* sc3 for numlock */
			0xf4};		/* enable kbd */
/*
 * kbd_debugger_init
 *
 * Init the keyboard for the kernel debugger.  At power on, kbd
 * default to scan code set 2.  OSF want to use scan code set 1.
 * So some juggling is needed here.  Can't use abios here since
 * this is done very early in the boot process.  In the normal
 * boot (without debugger), this is done in ktsopen().  It goes
 * something like this:
 *
 *					cmds		ack	rc good/bad
 *	reset kbd			0xff		0xfa	0xaa/0xfc
 *	select scan code set 1		0xf0,0x03	0xfa,0xfa
 */
u_char osf_debugger_kbdcmds[] = {
			0xf0,		/* select alternate scan code */
			0x01};		/* scan code set 1 */

static void
kbd_debugger_init()
{
	u_char	ch;
	int		i;

	kbd_clr_hw_buffer();

	/*
	 * reset the keyboard
	 */
	kbd_write_port(KBD_DATA_PORT, KBD_RESET_CMD);
	ch = kbd_read_data_port();	/* read ack to reset cmd */
	if (ch != KBD_ACK) {
		DEBUGF(kddebug & DEBUG_ERR_COND,
		printf("kbd_debugger_init: ERROR, expect 0x%x got 0x%x\n",
				KBD_ACK, ch));
	}
	kbd_wait(500000);		/* reset needs 1/2 second to complete */
	ch = kbd_read_data_port();	/* read BAT completion code */
	if (ch != KBD_BAT_RC) {
		DEBUGF(kddebug & DEBUG_ERR_COND,
		printf("kbd_debugger_init: ERROR, expect 0x%x got 0x%x\n",
				KBD_BAT_RC, ch));
	}

	kbd_tx_off();			/* turn off keyboard translation */

	/*
	 * now send the cmds
	 */
	for (i = 0; i < sizeof(osf_debugger_kbdcmds); i++) {
		kbd_write_port(KBD_DATA_PORT, osf_debugger_kbdcmds[i]);
		ch = kbd_read_data_port();	/* read ack to the cmd */
		if (ch != KBD_ACK) {
			DEBUGF(kddebug & DEBUG_ERR_COND,
			printf("kbd_debugger_init: ERROR, cmd 0x%x got 0x%x\n",
					osf_debugger_kbdcmds[i], ch));
		}
	}

} /* kbd_debugger_init */

void
kbd_init_keyboard()
{
	if (kbd_debugger_inited)
		return;
	kbd_debugger_init();
	kbd_debugger_inited = TRUE;
	DEBUGF(kddebug & KTSDEBUG_INFO,
		printf("kbd_init_keyboard: keyboard inited.\n"));
}

int
kbdcall_abios(int abios_function, register struct Kbd_request * rb)
{
	int	rc;

	DEBUGF(kddebug & DEBUG_ABIOS,
		printf("kbdcall_abios: entering...\n"));

	rb->r_function = abios_function;
	rb->r_return_code = ABIOS_UNDEFINED;
	rb->state |= KBD_RB_STARTED;
	abios_common_start(rb, kbd_abios_data.lid_flags);
	rc = rb->r_return_code;
	DEBUGF(kddebug & DEBUG_ABIOS,
		printf("kbdcall_abios: abios_common_start(), rc=0x%x\n", rc));

#ifdef LATER
	/* need to use timeout() instead of busy wait loop ... */
	if (rb->r_return_code == ABIOS_STAGE_ON_TIME) {
		kbd_wait(KBD_TIME_TO_WAIT(*rb));	/* in usec */
		abios_common_interrupt(rb, kbd_abios_data.lid_flags);
		rb->state = KBD_RB_IDLE;
	}
	/*
	 * work...
	 * start a watchdog timer, w_start(), here in case the interrupt
	 * didn't happen.  (rb->r_time_out >> 3) is the number of seconds
	 * to wait.  If it is zero, this operation didn't have a timeout.
	 *
	 * kbd_watchdog_handler() may need to see the stuffs between
	 * ifdef LATER.
	 */
	while (rb->r_return_code == ABIOS_STAGE_ON_INT) {
		int pri_level = i_disable(INTMAX);
		rb->state |= KBD_RB_SLEEPING;
		i_enable(pri_level);
		ktsstub_e_sleep(&rb->sleep_on_intr, EVENT_SIGRET);
	}
	/*
	 * work... may need w_stop() here 
	 * also, need to setup a watchdog timer handler somewhere in
	 * ktsopen().  When the timer pop, e.g: the expected intr didn't
	 * happen, the handler arranges to call abios_common_timeout().
	 */

	DEBUGF(kddebug & DEBUG_ABIOS,
		printf("kbdcall_abios: resolving final return code:\n"));
	DEBUGF(kddebug & DEBUG_ABIOS,
		dump_abios_rb((caddr_t) rb, DUMP_ENTIRE_BLOCK));
#endif /* LATER */

	/* work... need to handle other returned codes here */
	switch (rc) {
	case ABIOS_DONE:
		break;
	default:
		DEBUGF(kddebug & DEBUG_ERR_COND,
			printf("kbdcall_abios: ERROR, unknown rc=0x%x\n", rc));
	} /* switch */

	DEBUGF(kddebug & DEBUG_ABIOS, printf("kbdcall_abios: leaving...\n"));
	return (rc);
} /* kbdcall_abios*/

int kdintr();

kdabios_init()
{
	int	rc;
	int	i, s, rb_len, num_rb;
		
	if (kdabios_initialized)
		return;

	DEBUGF(kddebug & DEBUG_ABIOS,
		printf("kdabios_init: entering...\n"));


	kbd_abios_data.lid = abios_next_LID(KBD_ID,2);
	rb_len = sizeof(struct Kbd_request);
	num_rb = sizeof(kreq) / rb_len;
	kbd_abios_data.num_rb = num_rb;
	DEBUGF(kddebug & DEBUG_ABIOS,
		printf("kdabios_init: num_rb=%d rb_len=%d\n", num_rb, rb_len));

	/*
	 * initialize kbd request blocks
	 *
	 * request block kreq[0] is for continuous read function (0x08),
	 * (e.g.: get the keystrokes)
	 *
	 * request block kreq[1] is for single stage or discrete multi
	 * stage functions, (e.g.: reset, read leds, set leds, ...)
	 */
	for (i = 0; i < num_rb; i++) {
		kreq[i].r_current_req_blck_len = rb_len;
		kreq[i].r_logical_id = kbd_abios_data.lid;
		kreq[i].r_unit = 0;
		kreq[i].request_header.Request_Block_Flags = 0;
		kreq[i].request_header.ELA_Offset = 0;
		kreq[i].r_return_code = ABIOS_UNDEFINED;
		kreq[i].state = KBD_RB_IDLE;
		kreq[i].sleep_on_intr = FALSE;
	}

	/*
	 * read logical id parameters for keyboard
	 *
	 * work...
	 * If we ever decided that the config manager will do abios
	 * read logical paramater for all devices, then this is
	 * not needed.
	 */
	DEBUGF(kddebug & DEBUG_ABIOS,
		printf("kdabios_init: ABIOS_LOGICAL_PARAMETER\n"));
	KBD_SET_RESERVED_ABIOS_LOGICAL_PARAMETER(kreq[1]);
	kbd_abios_data.lid_flags = 0;
	rc = kbdcall_abios(ABIOS_LOGICAL_PARAMETER, &kreq[1]);
	if (rc) {
		DEBUGF(kddebug & DEBUG_ERR_COND,
			printf("kdabios_init: ERROR, rc=0x%x\n", rc));
		return;
	}

	/*
	 * update certain fields based on what abios just told us
	 */
	kbd_abios_data.installed = (kreq[1].r_number_units > 0);
	if (kbd_abios_data.installed == FALSE) {
		DEBUGF(kddebug & DEBUG_ERR_COND,
			printf("kdabios_init: ERROR, no keyboard!\n"));
		return;
	}
	kbd_abios_data.lid_flags = kreq[1].r_logical_id_flags;
	rb_len = kreq[1].r_request_block_length;
	for (i = 0; i < num_rb; i++) {
		kreq[i].r_current_req_blck_len = rb_len;
	}


	/*
	 * start reading keystrokes
	 */
	DEBUGF(kddebug & DEBUG_ABIOS,
		printf("kdabios_init: ABIOS_READ\n"));
	KBD_SET_RESERVED_ABIOS_READ(kreq[0]);
	kreq[0].r_function = ABIOS_READ;
	kreq[0].r_return_code = ABIOS_UNDEFINED;
	kreq[0].state |= KBD_RB_STARTED;
	s = SPLKD();
#ifdef OSF
	kd_handler.ih_level = kreq[1].r_hardware_intr;
	kd_handler.ih_handler = kdintr;
	kd_handler.ih_resolver = (int (*)()) NULL;
	kd_handler.ih_stats.intr_type = INTR_DEVICE;
	kd_handler.ih_stats.intr_cnt = 0;
	kd_handler.ih_priority = SPL6;
	kd_handler.ih_flags = IH_FLAG_NOSHARE;
	if ((kd_handler_id = handler_add( &kd_handler )) != NULL)
	    handler_enable( kd_handler_id );
	else
	    return;
#else
	take_irq(kreq[1].r_hardware_intr, 0, kdintr, SPL6, "kd");
#endif OSF
	abios_common_start(&kreq[0], kbd_abios_data.lid_flags);
	rc = kreq[0].r_return_code;
	splx(s);
	DEBUGF(kddebug & DEBUG_ABIOS,
		printf("kdabios_init: abios_common_start(), rc=%x\n", rc));
	if (rc != ABIOS_STAGE_ON_INT) {			/* read key fail */
		kreq[0].state = KBD_RB_IDLE;
	       	DEBUGF(kddebug & DEBUG_ERR_COND,
			printf("kdabios_init: ERROR, READ, rc=%x\n", rc));
		return;
	}

	kdabios_initialized = TRUE;

	DEBUGF(kddebug & KTSDEBUG_INFO,
	printf("When in loop, Ctrl-Alt-pad4=debugger, Ctrl-Alt-Del=reboot.\n"));
	DEBUGF(kddebug & DEBUG_ABIOS, printf("kdabios_init: leaving...\n"));
}

int
kdabios_intr()
{
	struct Kbd_abios_data *kabios;	/* ptr to kbd abios global data */
	int	rc, abios_rc;
	int	i;

	DEBUGF(kddebug & DEBUG_INTR,
		printf("\nkdabios_intr: ENTERING ...\n"));

	kabios = &kbd_abios_data;
	kabios->sc = 0;
	rc = INTR_FAIL;

	for (i = 0; i < kabios->num_rb; i++) {
		abios_rc = kreq[i].r_return_code;
		if ((kreq[i].state == KBD_RB_IDLE) ||
		    (abios_rc == ABIOS_UNDEFINED))
				continue;	/* not me! */

		if ((abios_rc & ABIOS_STAGE_ON_INT) == 0)
				continue;	/* not me either! */

		abios_common_interrupt(&kreq[i], kabios->lid_flags);
		abios_rc = kreq[i].r_return_code;
		DEBUGF(kddebug & DEBUG_INTR,
			printf("kdabios_intr: kreq[%d], abios_common_interrupt(), rc=0x%x\n", i, abios_rc));

		if (abios_rc & ABIOS_NOT_MY_INT)
				continue;	/* not me either! */

		rc = INTR_SUCC;
		switch (abios_rc) {		/* it is me */
		case ABIOS_STAGE_ON_INT:
			/*
			 * discrete multi stages, just continue.  Eventualy
			 * we will get a done or some sort of errors.
			 */
			kreq[i].state |= KBD_RB_STAGING;
			break;

		case ABIOS_KBD_RC_KEY_AVAIL:
			/*
			 * key stroke available, grasp it, process it
			 * and continue to wait for the next one
			 */
			kreq[i].state |= KBD_RB_STAGING;
			kabios->sc = KBD_SCAN_CODE(kreq[i]);
			DEBUGF(kddebug & KTSDEBUG_TERSE,
				printf("kdabios_intr: got a scancode, 0x%02x\n",
					kabios->sc));
			break;

		default:
			/*
			 * All other abios return codes, good or bad, will be
			 * handled by caller of abios.  As far as interrupt
			 * is concerned, we just have a successful one.
			 */
			kreq[i].state = KBD_RB_IDLE;
			break;

		} /* switch */
	} /* for */

	DEBUGF(kddebug & DEBUG_INTR,
		printf("kdabios_intr: rc = 0x%x %d\n", rc, rc));
	if (rc != INTR_SUCC) {
		DEBUGF(kddebug & DEBUG_ERR_COND,
		printf("kdabios_intr: spurious intr from kbd, count=%d\n",
				++kabios->bad_intr_count));
	}

	DEBUGF(kddebug & DEBUG_INTR, printf("kdabios_intr: LEAVING ...\n"));
	return (rc);
} /* kdabios_intr */

int
kbd_clr_ctrl_alt_pad4()
{
	int	ctrl_up, alt_up, pad4_up;
	u_char	ch;

	ctrl_up = alt_up = pad4_up = FALSE;
	while ((ctrl_up==FALSE) || (alt_up==FALSE) || (pad4_up==FALSE)) {
		ch = kbd_read_data_port();
		if (ch == (K_CTLSC | K_UP))
			ctrl_up = TRUE;
		else if (ch == (K_ALTSC | K_UP))
			alt_up = TRUE;
		else if (ch == (K_pad4SC | K_UP))
			pad4_up = TRUE;
	}
} /* kbd_clr_ctrl_alt_pad4 */

/* un-ported -----------------------------------------------------------*/
