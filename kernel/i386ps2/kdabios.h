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
 * $Log:	kdabios.h,v $
 * Revision 2.3  93/03/11  14:09:33  danner
 * 	u_long -> u_int
 * 	[93/03/09            danner]
 * 
 * 	[93/03/09            danner]
 * 
 * Revision 2.2  93/02/04  08:01:15  danner
 * 	Integrate PS2 code from IBM.
 * 	[93/01/18            prithvi]
 * 
 */

#ifndef _H_KTSABIOS
#define _H_KTSABIOS
/*
 * COMPONENT_NAME: SYSX/HFTSS/KTSM Keyboard Device Driver ktsabios.h
 *
 * FUNCTIONS:
 *
 * ORIGINS: 27
 */                                                                   

/*
 * Few words on coding style: ---------------------------------
 *
 * set tabstop=8
 *
 */

/*
 * This structure is good for most of the kbd requests, most info
 * between kbd and abios are passed in bytes 0x14 and 0x15.
 *
 * The time_to_wait before resuming request is in microseconds.
 * It is valid only if rc = 0x0002 = ABIOS_STAGE_ON_TIME.
 */
struct Keyboard_params {
	u_int	time_to_wait;		/* 0x10, OUT */
	u_char	byte_1;			/* 0x14, IN/OUT */
	u_char	byte_2;			/* 0x15, IN/OUT */
};

/*
 * Set up kbd ABIOS request block, assuming request block length
 * of KBD_REQUEST_BLOCK_LEN+0x10 bytes.  This should be sufficient.
 * The real value can be found by issuing ABIOS_LOGICAL_PARAMETER
 * function.
 *
 * The last time I check, kbd rb len should be 49 bytes.
 */
#define KBD_REQUEST_BLOCK_LEN		64
struct Kbd_request {
	struct Request_header	request_header;	/* 0x00-0x0f abios.h */
	union {
		struct Logical_id_params	logical_id_params;
		struct Keyboard_params		keyboard_params;
		u_char				uc[KBD_REQUEST_BLOCK_LEN];
	} un;
	int	state;
	int	sleep_on_intr;	/* sleep waiting for interrupt */
};
#define KBD_RB_IDLE		0		/* don't use BIT0 */
#define KBD_RB_STARTED		BIT0
#define KBD_RB_STAGING		BIT1
#define KBD_RB_SLEEPING		BIT2

/*
 * We need at least 2 abios request blocks for keyboard:
 * kreq[0] for continuous read request (read the keystrokes)
 * kreq[1] for single-staged or discrete multi-staged request
 *	(reset, set caps lock led, set typematic rate, ... etc)
 */
extern struct Kbd_request kreq[2];

/*
 * ABIOS keyboard specific function codes.
 */
#define	ABIOS_KBD_READ_LEDS		0x0b
#define	ABIOS_KBD_SET_LEDS		0x0c
#define	ABIOS_KBD_SET_TYPEMATIC		0x0d
#define	ABIOS_KBD_READ_SC_MODE		0x0e
#define ABIOS_KBD_SET_SC_MODE		0x0f
#define ABIOS_KBD_CMD_CTRLR		0x10
#define ABIOS_KBD_CMD_KBD		0x11

/*
 * ABIOS keyboard specific return codes.
 */
#define ABIOS_KBD_RC_KEY_AVAIL		(ABIOS_ATTENTION | ABIOS_STAGE_ON_INT)
#define ABIOS_KBD_RC_BUSY		0x8000
#define ABIOS_KBD_RC_RESET_FAILED	0x9001
#define ABIOS_KBD_RC_WATCHDOG_TIMEOUT	0xfffe

/*
 * For ABIOS_LOGICAL_PARAMETER (0x01)
 * rc = 0 only.  NO interrupt.
 */
/* IN */
#define KBD_SET_RESERVED_ABIOS_LOGICAL_PARAMETER(rb) \
	{ \
	*((u_short *)&(rb).un.uc[0x0a]) = 0; \
	*((u_short *)&(rb).un.uc[0x0c]) = 0; \
	*((u_short *)&(rb).un.uc[0x0e]) = 0; \
	}
/* OUT */
/* define in abios.h */

/*
 * For ABIOS_READ_PARAMETER (0x03) function.
 * This function returns the keyboard id.
 */
/* IN */
#define KBD_SET_RESERVED_ABIOS_READ_PARAMETER(rb) \
					KBD_SET_RESERVED_ABIOS_RESET(rb)
/* OUT */
#define KBD_TIME_TO_WAIT(rb)		(rb).un.keyboard_params.time_to_wait
#define KBD_ID_BYTE_1(rb)		(rb).un.keyboard_params.byte_1
#define KBD_ID_BYTE_2(rb)		(rb).un.keyboard_params.byte_2
#define KBD_ID_SHORT(rb)		*((u_short *)&(rb).un.uc[0x04])

/*
 * For ABIOS_RESET (0x05)
 * For ABIOS_ENABLE_INTR (0x06)
 * For ABIOS_DISABLE_INTR (0x07)
 */
/* IN */
#define KBD_SET_RESERVED_ABIOS_RESET(rb) \
	{ \
	*((u_short *)&(rb).un.uc[0x06]) = 0; \
	}
#define KBD_SET_RESERVED_ABIOS_ENABLE_INTR(rb) \
					KBD_SET_RESERVED_ABIOS_RESET(rb)
#define KBD_SET_RESERVED_ABIOS_DISABLE_INTR(rb) \
					KBD_SET_RESERVED_ABIOS_RESET(rb)
/* OUT */
/* none */

/*
 * For ABIOS_READ (0x08)
 */
/* IN */
#define KBD_SET_RESERVED_ABIOS_READ(rb) \
					KBD_SET_RESERVED_ABIOS_RESET(rb)
/* OUT */
#define KBD_SCAN_CODE(rb)		(rb).un.keyboard_params.byte_1

/*
 * For ABIOS_KBD_READ_LEDS (0x0b)
 * For ABIOS_KBD_SET_LEDS (0x0c)
 */
/* IN */
#define KBD_SET_RESERVED_ABIOS_KBD_READ_LEDS(rb) \
					KBD_SET_RESERVED_ABIOS_RESET(rb)
#define KBD_SET_RESERVED_ABIOS_KBD_SET_LEDS(rb) \
					KBD_SET_RESERVED_ABIOS_RESET(rb)
/* IN/OUT */
#define KBD_LEDS(rb)			(rb).un.keyboard_params.byte_1
#define KBD_LED_SCROLL			BIT0
#define KBD_LED_NUM			BIT1
#define KBD_LED_CAPS			BIT2

/*
 * For ABIOS_KBD_SET_TYPEMATIC (0x0d)
 */
/* IN */
#define KBD_SET_RESERVED_ABIOS_KBD_SET_TYPEMATIC(rb) \
					KBD_SET_RESERVED_ABIOS_RESET(rb)
#define KBD_TYPEMATIC_RATE(rb)		(rb).un.keyboard_params.byte_1
#define KBD_T_RATE_30_CPS		0	/* 30.0 char per second */
#define KBD_T_RATE_27_CPS		1	/* 26.7 CPS */
#define KBD_T_RATE_24_CPS		2	/* 24.0 CPS */
#define KBD_T_RATE_22_CPS		3	/* 21.8 CPS */
#define KBD_T_RATE_20_CPS		4	/* 20.0 CPS */
#define KBD_T_RATE_19_CPS		5	/* 18.5 CPS */
#define KBD_T_RATE_17_CPS		6	/* 17.1 CPS */
#define KBD_T_RATE_16_CPS		7	/* 16.0 CPS */
#define KBD_T_RATE_15_CPS		8	/* 15.0 CPS */
#define KBD_T_RATE_13_CPS		9	/* 13.3 CPS */
#define KBD_T_RATE_12_CPS		10	/* 12.0 CPS */
#define KBD_T_RATE_11_CPS		11	/* 10.9 CPS */
#define KBD_T_RATE_10_CPS		12	/* 10.0 CPS */
#define KBD_T_RATE_9_CPS		13	/* 9.2 CPS, 14 = 8.6 CPS */
#define KBD_T_RATE_8_CPS		15	/* 8.0 CPS, 16 = 7.5 CPS */
#define KBD_T_RATE_7_CPS		17	/* 6.7 CPS */
#define KBD_T_RATE_6_CPS		18	/* 6.0 CPS, 19 = 5.5 CPS */
#define KBD_T_RATE_5_CPS		20	/* 5.0 CPS, 21=4.6, 22=4.3 */
#define KBD_T_RATE_4_CPS		23	/* 4.0 CPS, 24=3.7, 25=3.3 */
#define KBD_T_RATE_3_CPS		26	/* 3.0 CPS, 27=2.7, 28=2.5 */
#define KBD_T_RATE_2_CPS		31	/* 2.0 CPS, 29=2.3, 30=2.1 */
#define KBD_T_RATE_DEFAULT		KBD_T_RATE_11_CPS
#define KBD_T_RATE_FAST			KBD_T_RATE_20_CPS
#define KBD_T_RATE_MEDIUM		KBD_T_RATE_10_CPS
#define KBD_T_RATE_SLOW			KBD_T_RATE_5_CPS
#define KBD_TYPEMATIC_DELAY(rb)		(rb).un.keyboard_params.byte_2
#define KBD_T_DELAY_250_MSEC		0
#define KBD_T_DELAY_500_MSEC		1
#define KBD_T_DELAY_750_MSEC		2
#define KBD_T_DELAY_1000_MSEC		3
/* OUT */
/* none */

/*
 * For ABIOS_KBD_READ_SC_MODE (0x0e)
 * For ABIOS_KBD_SET_SC_MODE (0x0f)
 */
/* IN */
#define KBD_SET_RESERVED_ABIOS_KBD_READ_SC_MODE(rb) \
					KBD_SET_RESERVED_ABIOS_RESET(rb)
#define KBD_SET_RESERVED_ABIOS_KBD_SET_SC_MODE(rb) \
					KBD_SET_RESERVED_ABIOS_RESET(rb)
/* IN/OUT */
#define KBD_SC_MODE(rb)			(rb).un.keyboard_params.byte_1
#define KBD_SC_MODE_1			1
#define KBD_SC_MODE_2			2
#define KBD_SC_MODE_3			3

/*
 * For ABIOS_KBD_CMD_CTRLR (0x10)
 * For ABIOS_KBD_CMD_KBD (0x11)
 */
/* IN */
#define KBD_SET_RESERVED_ABIOS_KBD_CMD_CTRLR(rb) \
	{ \
	*((u_short *)&(rb).un.uc[0x04]) = 0; \
	*((u_short *)&(rb).un.uc[0x18]) = 0; \
	}
#define KBD_SET_RESERVED_ABIOS_KBD_CMD_KBD(rb) \
				KBD_SET_RESERVED_ABIOS_KBD_CMD_CTRLR(rb)
#define KBD_CMD_PTR(rb)			*((u_int *)&(rb).un.uc[0x06])
#define KBD_CMD_COUNT(rb)		((rb).un.uc[0x0c])
#define KBD_ALL_KEY_T			0xf7
#define KBD_ALL_KEY_M_B			0xf8
#define KBD_ALL_KEY_M			0xf9
#define KBD_ALL_KEY_T_M_B		0xfa
#define KBD_ONE_KEY_T			0xfb
#define KBD_ONE_KEY_M_B			0xfc
#define KBD_ONE_KEY_M			0xfd

struct Kbd_abios_data {
	int		rb_inited;	/* kreq[0], kreq[1] inited? */
	int		kbd_inited;	/* sc3?, t/m/b? ...etc */
	int		installed;	/* keyboard is present */
	int		num_rb;		/* number of request blocks */
	u_int		bad_intr_count;	/* count spurious intr fr kbd */
	u_short		lid;		/* abios logical id for kbd */
	u_short		lid_flags;	/* until abios32 */
	u_char		sc;
};
extern struct Kbd_abios_data kbd_abios_data;

/*
 * Work ...
 * temporary define's, need to find out what
 * the real keyboard ids for 102 & 106 keyboards.
 */
#define KBD_TYPE_NONE	0		/* no key board */
#define KBD_TYPE_101	0x83ab		/* U.S. keyboard */
#define KBD_TYPE_102	2		/* World Trade keyboard, 2 for now */
#define KBD_TYPE_106	3		/* Japanese keyboard, 3 for now */

/* Keys needed for the low level keyboard control.
** This is scan code set 3.
*/
#define SC_KEYUP	0xf0
#define SC_LCTL		0x11
#define SC_LSFT		0x12
#define SC_CAPS		0x14
#define SC_LALT		0x19
#define SC_RALT		0x39
#define SC_RCTL		0x58
#define SC_RSFT		0x59
#define SC_PAD4		0x6b
#define SC_PDEL		0x71

/*
 * For SC3 to SC1 conversion 
 */
#define K_UP_SC3	0xf0
#define K_CTLSC3L	0x11
#define K_CTLSC3R	0x58
#define K_CTLSC3	K_CTLSC3L
#define K_ALTSC3L	0x19
#define K_ALTSC3R	0x39
#define K_ALTSC3	K_ALTSC3L
#define K_pad4SC3	0x6b
#define K_DELSC3	0x71

extern unsigned char tbl_sc3to1[];
#define SC3_TO_SC1(sc3) (tbl_sc3to1[sc3])

extern int kddebug;
#define KDDEBUG_VERBOSE	BIT16	/* I'm here, I'm there, blah, blah, blah ... */
#define KDDEBUG_TERSE	BIT29		/* debug scancode <==> char */
#define KDDEBUG_INFO	BIT30		/* useful msgs */

#define RC_GOOD		0
#define RC_BAD		-1

#endif /* _H_KTSABIOS */
