/* 
 * Mach Operating System
 * Copyright (c) 1991 Carnegie Mellon University
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
 * Copyright 1988, 1989, 1990, 1991 by Intel Corporation,
 * Santa Clara, California.
 * 
 *                          All Rights Reserved
 * 
 * Permission to use, copy, modify, and distribute this software and its
 * documentation for any purpose and without fee is hereby granted,
 * provided that the above copyright notice appears in all copies and that
 * both the copyright notice and this permission notice appear in
 * supporting documentation, and that the name of Intel not be used in
 * advertising or publicity pertaining to distribution of the software
 * without specific, written prior permission.
 * 
 * INTEL DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING
 * ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT
 * SHALL INTEL BE LIABLE FOR ANY SPECIAL, INDIRECT, OR CONSEQUENTIAL
 * DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR
 * PROFITS, WHETHER IN ACTION OF CONTRACT, NEGLIGENCE, OR OTHER TORTIOUS
 * ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF
 * THIS SOFTWARE.
 */
/*
 * HISTORY
 * $Log:	sdintr.c,v $
 * Revision 2.4  91/07/01  08:24:24  jsb
 * 	Replace "Wrong direction DATAOUT" assertion with printf.
 * 	I seem to hit this a lot when running a STD+iPSC+NORMA+TEST kernel.
 * 	[91/06/29  16:13:34  jsb]
 * 
 * Revision 2.3  91/06/18  20:50:37  jsb
 * 	New copyright from Intel.
 * 	[91/06/18  20:08:01  jsb]
 * 
 * Revision 2.2  90/12/04  14:47:48  jsb
 * 	First checkin.
 * 	[90/12/04  10:58:42  jsb]
 * 
 */ 
/* 
 * sdintr.c Don Cameron September 1989
 *
 *    This is the bottom half of the SCSI disk driver for MACH on the iPSC/2.
 *    The code is a rewrite of scsiphase.c for NX on the iPSC/2 and portions
 *    of hd.c for MACH on the AT. 
 */
 
#include <sys/types.h>
#include <i386ipsc/scsi.h>
#include <i386at/disk.h>

#ifdef	MACH_KERNEL
#include <i386/mach_param.h>
#include <device/buf.h>
#include <i386ipsc/sd.h>
#include <device/errno.h>
#else	MACH_KERNEL
#include <i386ipsc/hz.h>
#include <sys/ioctl.h>
#include <sys/param.h>
#include <sys/buf.h>
#include <i386ipsc/sd.h>
#include <sys/errno.h>
#include <sys/user.h>
#include <sys/vmparam.h>
#include <sys/uio.h>
#endif	MACH_KERNEL

#define ONE_SECOND	(666000) 	/* Best guess of one second delay */
#ifdef MACH_KERNEL
unsigned long	elapsed_ticks;			/* Used to wait for a retry */
#else MACH_KERNEL
extern time_t lbolt;			/* Used to wait for a retry */
#endif MACH_KERNEL

int target_id = -1;		/* Current active drive on SCSI bus */
int command_waiting = 0;	/* Bit set of drives with pending commands */
unsigned short	scsi_st;	/* SCSI controller status */
unsigned char	scsi_ist;	/* SCSI controller interrupt status */
unsigned char	scsi_msg;	/* Recent message byte */
unsigned long	data_count;	/* Byte count for current transfer phase */
unsigned long	read_fifo;	/* Byte count transferred from read fifo */
unsigned long	write_fifo;	/* Byte count transferred to write fifo */

int	step;			/* Driver state */

#define ASSERT(t,m) if (!(t)) panic(m)

/* Variables defined in sd.c */
extern struct hh 	hh[NDRIVES];
extern int	 	scanning;	/* In drive scan, timeouts OK */
extern int		write_reserved;	/* Write fifo lock variable */
extern volatile caddr_t scsiphys0;
extern volatile caddr_t scsiphys1;
extern volatile caddr_t scsiphys2;
extern volatile caddr_t scsiphys3;

static int	fsi;			/* Controller fifo save count */
static unsigned char	fifo_save[16];		/* Controller fifo save area */

typedef struct { unsigned char byte[2048]; } B2048;
typedef struct { unsigned char byte[4096]; } B4096;

static char priority[] = {		/* Priority array for drive selection */
	0, 0, 1, 1, 2, 2, 2, 2, 3, 3, 3, 3, 3, 3, 3, 3,
	4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4,
	5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5,
	5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5,
	6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6,
	6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6,
	6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6,
	6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6,
	7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
	7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
	7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
	7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
	7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
	7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
	7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
	7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7
};

/* forward declarations */
static void sendcmd();
static void delay_one_second();

static dbgbuf[128]; /* DFC DBG */

/***************************************************************************
 *	
 *	Calling Sequence:
 *	      sdintr();
 *	
 *	Description:
 *	      Service SCSI interrupt.
 *	
 *	Parameters:
 *	      none
 *	
 *	Returns:
 *	      none
 *	
 */

sdintr()
{
	SCSI_SENSE sense;	/* Struct to read sense data into */
	struct hh *hh_p;

	if (target_id != -1)
		hh_p = &hh[target_id];

	/*
	 * Get status
	 */
	scsi_st = SCSI_STATUS;

	/*
	 * Check for major error
	 */

	if ((scsi_st & ESP_GROSS_STAT) != 0) {
		return;
	}


	/*
	 * Test for SCSI controller interrupt
	 */

	if ((scsi_st & SCSI_EINT_STAT) == 0) {

		/* Read (and clear) interrupt status */
		scsi_ist = SCSI_INT_STATUS;

		/*
		 * If a data transfer was in progress,
		 * clean up FIFO and complete the transfer.
		 */

		if (data_count != 0) {
			int cnt;

			/*
			 * Set cnt to the number of bytes transferred
			 * by the controller.  Read the transfer counter
			 * if necessary.
			 */

			if ((scsi_st & ESP_COUNT_STAT) == 0) {
				unsigned short t;

				t = SCSI_COUNT_LO;
				shortdelay();
				t |= (unsigned short)(SCSI_COUNT_HI) << 8;

				cnt = data_count - t;

			} else {
				cnt = data_count;
			}

			if (hh_p->direction == 1) {	/* Read */
				unsigned long *dp;

				/*
				 * Take all remaining bytes from
				 * the read FIFO.
				 */

				cnt -= read_fifo;
				dp = (unsigned long *)hh_p->rw_addr;
				hh_p->transfer_length -= cnt;
				if (hh_p->buf_io) {
					hh_p->buflst.b_actf->b_resid -= cnt;
				}
				hh_p->rw_addr += cnt;

				/*
				 * Transfer the data
				 */

				if (cnt >= 4096) {
					*(B4096 *)dp = *(B4096 *)(SCSI_FIFO);
					(char *) dp += 4096;
					cnt -= 4096;
				}
				if (cnt >= 2048) {
					*(B2048 *)dp = *(B2048 *)(SCSI_FIFO);
					(char *) dp += 2048;
					cnt -= 2048;
				}
				for (; cnt > 0; cnt -= 4) {
					*dp++ = *(unsigned long *)(SCSI_FIFO);
				}

				read_fifo = 0;

			} else {		/* Write */

				/*
				 * Save the SCSI controller FIFO in fifo_save
				 */

				fsi = SCSI_FIFO_FLAGS;
				if (fsi != 0) {
					int i;

					for (i = 0; i < fsi; i++)
						fifo_save[i] = SCSI_ESP_FIFO;
				}

				/*
				 * Adjust write_fifo to reflect the amount
				 * of data remaining in the write FIFO
				 */

				write_fifo -= cnt;
			}
			data_count = 0;
		}

		/*
		 * Check for special kinds of interrupts
		 */

		if (scsi_ist & ESP_ILLCMD_INT) {

			if (step == RSEL_STEP) {
				time_t t;

				/*
				 * Illegal command interrupt comes from
				 * select command executing after
				 * reselection occurs.  The reselection
				 * interrupt has already been processed
				 * and a ESP_MSGACCEPT command queued.
				 *			
				 * Give the ESP 5 ms to respond to the
				 * ESP_MSGACCEPT command.  If it does, go on;
				 * otherwise reissue the command.
				 *
				 * NB: Trying to wait for 5ms under UNIX is 
				 *     a joke of course, because the clock
				 *     ticks occur in increments like 10ms
				 *     and we need to wait for two clicks
				 *     because the first one may not have 
				 *     been a whole one.
				 */
#ifdef MACH_KERNEL
				t = elapsed_ticks + MIN(2, HZ/200);
#else MACH_KERNEL
				t = lbolt + MIN(2, HZ/200);
#endif MACH_KERNEL
      				do {
					scsi_st = SCSI_STATUS;
				} while ((scsi_st & SCSI_EINT_STAT) != 0 &&
#ifdef MACH_KERNEL
					 elapsed_ticks < t);
#else MACH_KERNEL
					 lbolt < t);
#endif MACH_KERNEL
				if ((scsi_st & SCSI_EINT_STAT) != 0) {
					SCSI_COMMAND = ESP_MSGACCEPT;
				}
				goto exitint;
			}


		} else if (scsi_ist & ESP_RESET_INT) {
			/*
			 * Ignore SCSI bus reset
			 */

			goto exitint;

		} else if (scsi_ist & ESP_RESEL_INT) {
			unsigned char	id;

			/*
			 * Controller has been reselected by a drive
			 */

			if (step == CMD_STEP) {

				/*
				 * Reselection overrides a command
				 * being sent to another drive.
				 * Set command_waiting flag for the other drive.
				 */

				command_waiting |= 1 << target_id;
				step = NULL_STEP;
				target_id = -1;
			}

			/*
			 * Get ID and message bytes from controller FIFO
			 */

			id = SCSI_ESP_FIFO;
			scsi_msg = SCSI_ESP_FIFO;
			shortdelay();
			if (SCSI_FIFO_FLAGS != 0) {
				/*
				 * The FIFO had some bytes in it from a
				 * command that was being sent to
				 * another drive
				 */

				SCSI_COMMAND = ESP_FLUSH_FIFO;
			}

			/*
			 * Decode drive number from id byte
			 * and accept reselection message
			 */

			for (target_id = 0;
			     (id & (1 << target_id)) == 0;
			     target_id++);

			SCSI_COMMAND = ESP_MSGACCEPT;
			step = RSEL_STEP;
			goto exitint;

		} else if (scsi_ist & ESP_DISC_INT) {
			int	drive, st;

			/*
			 * The current target drive has disconnected
			 */

			drive = target_id;
			target_id = -1;

			st = step;
			step = NULL_STEP;

			if (st == DONE_STEP) {

				/*
				 * Current operation is complete.
				 * Check for error status.
				 */

				if (hh_p->cmd_st == CMD_ST_CHECK) {
					/*
					 * Send request sense command
					 */

					bzero(hh_p->cmd, 6);
					hh_p->cmd_len = 6;
					hh_p->cmd[0] = SCSI_SENSE_CMD;
					hh_p->cmd[4] = sizeof(SCSI_SENSE);
					hh_p->transfer_length = sizeof(SCSI_SENSE);
					hh_p->rw_addr = (paddr_t)&sense;
					hh_p->buf_io = 0;
					hh_p->direction = 1;
					command_waiting |= 1 << drive;
				} else if (hh_p->buf_io) {
					struct buf *bp;

					bp = hh_p->buflst.b_actf;
					if (bp->b_resid != 0) {
						sdrestart();
					}
					else {
						hh_p->cmd_busy = 0;
						hh_p->buflst.b_actf =
							bp->av_forw;
						biodone(bp);
						sdstart();
						wakeup(&hh_p->cmd_busy);
					}
				} else {
					hh_p->cmd_busy = 0;
					wakeup(&hh_p->cmd_busy);
				}

			} else if (st != DISC_STEP) {

				/*
				 * If disconnect was unexpected, set
				 * timeout status
				 */

				hh_p->cmd_st = CMD_ST_TIMEOUT;
				hh_p->cmd_busy = 0;
				SCSI_COMMAND = ESP_FLUSH_FIFO;
				shortdelay();
				SCSI_COMMAND = ESP_ENABLE_SEL;
				ASSERT(scanning, "Unexpected disconnect");
				wakeup(&hh_p->cmd_busy);
				goto exitint;
			}

			/*
			 * Enable reselection by any drive
			 */

			SCSI_COMMAND = ESP_ENABLE_SEL;

			/*
			 * If there are commands waiting, attempt to
			 * send one now while the bus is available
			 */

			if (command_waiting != 0) {
				sendcmd(priority[command_waiting], 1);
			}
			goto exitint;
		}

		/*
		 * The distinction of CMD_STEP and RSEL_STEP is
		 * not important at this stage
		 */

		if (step == CMD_STEP || step == RSEL_STEP) {
			step = NULL_STEP;
		}

		/*
		 * Switch according to the bus phase asserted by the drive
		 */

		switch (scsi_st & ESP_PHASE_STAT) {

		case ESP_COMMAND_PH:
			ASSERT(0, "Unexpected SCSI phase");

		case ESP_MSGIN_PH:
			if (step == NULL_STEP) {
				ASSERT(SCSI_FIFO_FLAGS == 0,
					"ESP FIFO not clear in MSGIN");

				/* Initiate transfer of a message byte */
				SCSI_COMMAND = ESP_TRANSFER;
				step = MSG_STEP;

			} else if (step == MSG_STEP) {
				unsigned char msg;

				step = NULL_STEP;

				/*
				 * Obtain a message byte
				 */

				msg = SCSI_ESP_FIFO;
				shortdelay();

				/*
				 * Accept the message
				 */

				SCSI_COMMAND = ESP_MSGACCEPT;

				/*
				 * Switch based on the message contents
				 */

				switch(msg) {

				case MSG_SAVE_DP:
					break;	/* Ignore */

				case MSG_REST_DP:
					break;	/* Ignore */

				case MSG_DISCONNECT:
					/* Expect disconnect interrupt */
					step = DISC_STEP;
					break;

				case MSG_REJECT:
					ASSERT(0, "Message rejected");

				case MSG_NOP:
					break;	/* Ignore */

				default:
					ASSERT(0, "Invalid message in");
				}

			} else /* step == STATUS_STEP */ {
				unsigned long t;

				/*
				 * Obtain status and message bytes
				 */

				t = *(unsigned long *)(SCSI_FIFO);

				hh_p->cmd_st = t;
				hh_p->cmd_msg = t >> 8;

				/*
				 * Accept message and expect command
				 * completion disconnect
				 */

				SCSI_COMMAND = ESP_MSGACCEPT;
				step = DONE_STEP;

				/*
				 * If write command completed, release
				 * write FIFO lock
				 */

				if (hh_p->cmd[0] == SCSI_WRITE_CMD) {
					write_reserved = 0;
				}

			}
			break;

		case ESP_MSGOUT_PH:
			ASSERT(0, "Unexpected phase MSGOUT");
			break;

		case ESP_DATAOUT_PH:
			if (hh_p->direction != 0) {
				printf("sd%d: false interrupt\n", target_id);
				return;
			}

			/*
			 * Set up write transfer
			 */

			set_fifo_write();
			data_count = hh_p->transfer_length + write_fifo;

			SCSI_COUNT_LO = data_count;
			shortdelay();
			SCSI_COUNT_HI = data_count >> 8;

			/*
			 * Replace data in the controller FIFO from
			 * a previous transfer phase
			 */

			if (fsi != 0) {
				int	i;

				for (i = 0; i < fsi; i++)
					SCSI_ESP_FIFO = fifo_save[i];
				fsi = 0;
			}

			/*
			 * Start filling the write FIFO if there is room in it
			 */

			if ((scsi_st & SCSI_WHF_STAT) == 0) {
				unsigned long *dp;
				unsigned long cnt;

				cnt = hh_p->transfer_length;
				if (cnt > 4096)
					cnt = 4096;
				if (cnt > 2048 &&
				    (scsi_st & SCSI_WEF_STAT) != 0)
					cnt = 2048;

				dp = (unsigned long *)(hh_p->rw_addr);
				hh_p->transfer_length -= cnt;
				if (hh_p->buf_io) {
					hh_p->buflst.b_actf->b_resid -= cnt;
				}
				hh_p->rw_addr += cnt;
				write_fifo += cnt;

				if (cnt == 4096) {
					*(B4096 *)(SCSI_FIFO) = *(B4096 *)dp;

				} else {
					if (cnt >= 2048) {
						*(B2048 *)(SCSI_FIFO) = *(B2048 *)dp;
						(char *) dp += 2048;
						cnt -= 2048;
					}
					for (; cnt > 0; cnt -= 4) {
						*(unsigned long *)(SCSI_FIFO) = *dp++;
					}
				}
			}

			/*
			 * Issue transfer command.  Early versions
			 * of controller chip require NOP before
			 * transfer.
			 */

			shortdelay();
			SCSI_COMMAND = ESP_NOP;
			shortdelay();
			SCSI_COMMAND = ESP_TRANSFER | ESP_DMA;
			break;

		case ESP_DATAIN_PH:
			ASSERT(hh_p->direction == 1,
				"Wrong direction DATAIN");

			/*
			 * Set up read transfer
			 */

			set_fifo_read();
			data_count = hh_p->transfer_length;
			SCSI_COUNT_LO = data_count;
			shortdelay();
			SCSI_COUNT_HI = data_count >> 8;

			/*
			 * Issue transfer command.  Early versions
			 * of controller chip require NOP before
			 * transfer.
			 */

			shortdelay();
			SCSI_COMMAND = ESP_NOP;
			shortdelay();
			SCSI_COMMAND = ESP_TRANSFER | ESP_DMA;
			break;

		case ESP_STATUS_PH:

			/*
			 * Set up controller to DMA status and message
			 * bytes into the read FIFO.  This saves an
			 * interrupt compared with using the controller FIFO.
			 */

			set_fifo_read();
			SCSI_COUNT_LO = 2;
			shortdelay();
			SCSI_COUNT_HI = 0;
			shortdelay();
			SCSI_COMMAND = ESP_COMPLETE | ESP_DMA;
			step = STATUS_STEP;
			break;
		}

	}

exitint:
	;
}

/***************************************************************************
 *	Calling Sequence:
 *	      sendcmd(drive);
 *	
 *	Description:
 *	      Send command to selected drive.
 *	
 *	Parameters:
 *	      drive		Drive index.
 *	
 *	Returns:
 *	      none
 *	
 */

static void sendcmd(drive)
	int	drive;
{
	int	i;
	struct hh *hh_p;

	hh_p = &hh[drive];
	if (hh_p->cmd[0] == SCSI_WRITE_CMD) {
		write_reserved = 1;
	}
	hh_p->cmd_busy = 1;
	if (target_id == -1) {
		data_count = 0;
		command_waiting &= ~(1 << drive);
		step = CMD_STEP;
		target_id = drive;


		SCSI_ESP_FIFO = MSG_IDENTIFY;
		shortdelay();
		SCSI_ID = target_id;
		for (i = 0; i < hh_p->cmd_len; i++) {
			SCSI_ESP_FIFO = hh_p->cmd[i];
		}
		SCSI_COMMAND = ESP_SELATN;
	} else {
		command_waiting |= 1 << drive;

	}
}

/***************************************************************************
 *	
 *	Calling Sequence:
 *	      reset_controller();
 *	
 *	Description:
 *	      Reset ESP.
 *	
 *	Parameters:
 *	      none
 */

reset_controller()
{
	int	tmp;	/* dummy var to force read of SCSI chip */

	/*
	 * Initialize operating globals
	 */

	target_id = -1;
	write_reserved = 0;
	command_waiting = 0;

	tmp = SCSI_RESET_ESP;
	shortdelay();
	tmp = SCSI_RESET_FIFO;
	shortdelay();
	write_fifo = SCSI_CLEAR_CNT;
	shortdelay();
	set_fifo_read();
	shortdelay();

	data_count = 0;
	SCSI_COMMAND = ESP_RESET_CHIP;
	shortdelay();
	SCSI_COMMAND = ESP_NOP;
	shortdelay();

	SCSI_CLOCK = 5;
	shortdelay();
	SCSI_CONFIG = 0x97;
	shortdelay();
	SCSI_TIMEOUT = 0x93;
	shortdelay();
	SCSI_SYNC_PER = 0x05;
	shortdelay();
	SCSI_SYNC_OFF = 0x00;
	shortdelay();

	data_count = 0;
	read_fifo = 0;
	write_fifo = 0;
	fsi = 0;
}

/***************************************************************************
 *	
 *	Calling Sequence:
 *	      reset_bus();
 *	
 *	Description:
 *	      Reset SCSI bus.
 *	
 *	Parameters:
 *	      none
 *	
 *	Returns:
 *	      none
 *	
 */

reset_bus()
{
	data_count = 0;
	SCSI_COMMAND = ESP_RESET_BUS;

	delay_one_second();

	scsi_st = SCSI_STATUS;
	shortdelay();
	scsi_ist = SCSI_INT_STATUS;
}

/***************************************************************************
 *	
 *	Calling Sequence:
 *	      shortdelay();
 *	
 *	Description:
 *	      Short delay for ESP.
 *	
 *	Parameters:
 *	      none
 *	
 *	Returns:
 *	      none
 */


#define SHORTDELAY 10

shortdelay()
{
	volatile unsigned long	t;

	for (t = SHORTDELAY; t; t--);
}

/***************************************************************************
 *	
 *	Calling Sequence:
 *	      set_fifo_read();
 *	
 *	Description:
 *	      Put SCSI controller FIFO's in read mode.
 *	
 *	Parameters:
 *	      none
 *	
 *	Returns:
 *	      none
 *	
 */


set_fifo_read()
{

	SCSI_READ_MODE = SCSI_CLEAR_CNT;
}

/***************************************************************************
 *	
 *	Calling Sequence:
 *	      set_fifo_write();
 *	
 *	Description:
 *	      Put SCSI controller FIFO's in write mode.
 *	
 *	Parameters:
 *	      none
 *	
 *	Returns:
 *	      none
 *	
 */

set_fifo_write()
{

	SCSI_WRITE_MODE = SCSI_CLEAR_CNT;
}

/***************************************************************************
 *	
 *	Calling Sequence:
 *	      reverse_long(p);
 *	
 *	Description:
 *	      Reverse the bytes of a long.
 *	
 *	Parameters:
 *	      p:	Pointer to a long.
 *	
 *	Returns:
 *	      Side effect in *p.
 *	
 */

reverse_long(p)
	unsigned char *p;
{
	unsigned char	t;

	t = p[0];
	p[0] = p[3];
	p[3] = t;

	t = p[1];
	p[1] = p[2];
	p[2] = t;
}

/*
 * delay_one_second 
 *	Delay for one second.
 */
static void delay_one_second()
{
	long time;
	/* Have to brute force this */
	time = ONE_SECOND;
	while (time--);
}
