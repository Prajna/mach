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
 * $Log:	kd_var.h,v $
 * Revision 2.2  93/02/04  08:01:02  danner
 * 	Integrate PS2 code from IBM.
 * 	[93/01/18            prithvi]
 * 
 */

#include <i386ps2/debugf.h>
#include <i386ps2/abios.h>
#include <i386ps2/kdabios.h>
#include <i386/ipl.h>

#define INTR_SUCC		0
#define INTR_FAIL		-1
#define KTSDEBUG_TERSE		BIT22	/* debug scancode <==> char */
#define KTSDEBUG_INFO		BIT30	/* useful msgs */

#define DISP_START	vid_start
#define DISP_END	DISP_START+ONE_PAGE
#define DISP_LAST_LINE	DISP_START+BOTTOM_LINE

struct Kbd_abios_data	kbd_abios_data;
struct Kbd_request	kreq[2];
int			kbd_debugger_inited = FALSE;
int			kdabios_initialized = FALSE;
int			kddebug = (DEBUG_ERR_COND |
				KTSDEBUG_INFO);
int			outputing_debugger_text = FALSE;
void			kbd_init_keyboard();

extern int		debug_output;
#define DEBUG_OUT_SCR   1		/* from sadebug.c */
#define DEBUG_OUT_COM   2

/*
 * Color attribute byte.  0-7 are normal intensity colors.
 * 8-15 are high intensity colors.  For example, (INTENSE + RED)
 * or (INTENSE | RED) will give you the 'Light Red' color.
 */
#define BLACK                           0
#define BLUE                            1
#define GREEN                           2
#define CYAN                            3
#define RED                             4
#define MAGENTA                         5
#define YELLOW                          6
#define WHITE                           7
#define INTENSE                         8
