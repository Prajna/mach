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
 * $Log:	scsi.h,v $
 * Revision 2.3  91/06/18  20:50:30  jsb
 * 	New copyright from Intel.
 * 	[91/06/18  20:07:32  jsb]
 * 
 * Revision 2.2  90/12/04  14:47:38  jsb
 * 	First checkin.
 * 	[90/12/04  10:57:57  jsb]
 * 
 */ 
/*
 * This module contains constants for the SCSI interface it is derived
 * from NX's scsi.h.
 */

#define SCSIPHYS0	0xD0000000
#define SCSIPHYS1	0xD4000000
#define SCSIPHYS2	0xD8000000
#define SCSIPHYS3	0xDC000000

#define SCSIPHYS0_LEN	0x10000
#define SCSIPHYS1_LEN	0x100
#define SCSIPHYS2_LEN	0x100
#define SCSIPHYS3_LEN	0x100

#define SCSI_FIFO       scsiphys0
#define SCSI_ESP_BASE   scsiphys1
#define SCSI_AUX_BASE   scsiphys2
#define SCSI_LOOP_BASE  scsiphys3

#define SCSI_CLEAR_CNT  *(volatile long *)(SCSI_AUX_BASE + 0x00)
#define SCSI_RESET_ESP  *(volatile long *)(SCSI_AUX_BASE + 0x40)
#define SCSI_RESET_FIFO *(volatile long *)(SCSI_AUX_BASE + 0x80)

#define SCSI_READ_MODE  *(volatile long *)(SCSI_AUX_BASE + 0x00)
#define SCSI_WRITE_MODE *(volatile long *)(SCSI_AUX_BASE + 0x40)
#define SCSI_ENABLE_HF  *(volatile long *)(SCSI_AUX_BASE + 0x80)
#define SCSI_DISABLE_HF *(volatile long *)(SCSI_AUX_BASE + 0xC0)

#define SCSI_COUNT_LO   (*(volatile unsigned char *)(SCSI_ESP_BASE + 0x0000))
#define SCSI_COUNT_HI   (*(volatile unsigned char *)(SCSI_ESP_BASE + 0x0004))
#define SCSI_ESP_FIFO   (*(volatile unsigned char *)(SCSI_ESP_BASE + 0x0008))
#define SCSI_COMMAND    (*(volatile unsigned char *)(SCSI_ESP_BASE + 0x000C))
#define SCSI_STATUS     (*(volatile unsigned short *)(SCSI_ESP_BASE + 0x0010))
#define SCSI_ID         (*(volatile unsigned char *)(SCSI_ESP_BASE + 0x0010))
#define SCSI_INT_STATUS (*(volatile unsigned char *)(SCSI_ESP_BASE + 0x0014))
#define SCSI_TIMEOUT    (*(volatile unsigned char *)(SCSI_ESP_BASE + 0x0014))
#define SCSI_SEQUENCE   (*(volatile unsigned char *)(SCSI_ESP_BASE + 0x0018))
#define SCSI_SYNC_PER   (*(volatile unsigned char *)(SCSI_ESP_BASE + 0x0018))
#define SCSI_FIFO_FLAGS (*(volatile unsigned char *)(SCSI_ESP_BASE + 0x001C))
#define SCSI_SYNC_OFF   (*(volatile unsigned char *)(SCSI_ESP_BASE + 0x001C))
#define SCSI_CONFIG     (*(volatile unsigned char *)(SCSI_ESP_BASE + 0x0020))
#define SCSI_CLOCK      (*(volatile unsigned char *)(SCSI_ESP_BASE + 0x0024))

#define ESP_NOP         0x00
#define ESP_FLUSH_FIFO  0x01
#define ESP_RESET_CHIP  0x02
#define ESP_RESET_BUS   0x03
#define ESP_SELECT      0x41
#define ESP_SELATN      0x42
#define ESP_SELATNSTOP  0x43
#define ESP_ENABLE_SEL  0x44
#define ESP_DISABLE_SEL 0x45
#define ESP_TRANSFER    0x10
#define ESP_COMPLETE    0x11
#define ESP_MSGACCEPT   0x12
#define ESP_PAD         0x18
#define ESP_ATN         0x1A
#define ESP_DMA         0x80

#define ESP_SELECT_INT  0x01
#define ESP_SELATN_INT  0x02
#define ESP_RESEL_INT   0x04
#define ESP_COMP_INT    0x08
#define ESP_BUS_INT     0x10
#define ESP_DISC_INT    0x20
#define ESP_ILLCMD_INT  0x40
#define ESP_RESET_INT   0x80

#define ESP_PHASE_STAT  0x07
#define ESP_XFER_STAT   0x08
#define ESP_COUNT_STAT  0x10
#define ESP_PARITY_STAT 0x20
#define ESP_GROSS_STAT  0x40

#define SCSI_WEF_STAT   0x0100
#define SCSI_WHF_STAT   0x0200
#define SCSI_WFF_STAT   0x0400
#define SCSI_EINT_STAT  0x0800
#define SCSI_MODE_STAT  0x1000
#define SCSI_REF_STAT   0x2000
#define SCSI_RHF_STAT   0x4000
#define SCSI_RFF_STAT   0x8000

#define ESP_DATAOUT_PH  0x00
#define ESP_DATAIN_PH   0x01
#define ESP_COMMAND_PH  0x02
#define ESP_STATUS_PH   0x03
#define ESP_MSGOUT_PH   0x06
#define ESP_MSGIN_PH    0x07

#define ESP_CLK_NS      (1000/24)

#define CMD_ST_GOOD     0x00
#define CMD_ST_CHECK    0x02
#define CMD_ST_BUSY     0x08
#define CMD_ST_INTER    0x10
#define CMD_ST_RUNNING  0x60
#define CMD_ST_TIMEOUT  0x62

#define MSG_CMD_DONE    0x00
#define MSG_SAVE_DP     0x02
#define MSG_REST_DP     0x03
#define MSG_DISCONNECT  0x04
#define MSG_INITIAT_ERR 0x05
#define MSG_ABORT       0x06
#define MSG_REJECT      0x07
#define MSG_NOP         0x08
#define MSG_PARITY      0x09
#define MSG_LINK_DONE   0x0A
#define MSG_LINK_FLAG   0x0B
#define MSG_RESET       0x0C
#define MSG_IDENTIFY    0xC0

#define SCSI_INQUIRY_CMD        0x12
typedef
struct scsi_inquiry {
        unsigned long  dev_type        :8;
        unsigned long  qualifier       :7;
        unsigned long  rmb             :1;
        unsigned long  version         :8;
        unsigned long  format          :4;
        unsigned long  not_used        :4;
        unsigned char  added;
        unsigned char  reserved[3];
        unsigned char  extra[28];
} SCSI_INQUIRY;

#define SCSI_SENSE_CMD          0x03
typedef
struct scsi_sense {
        unsigned char  class;
        unsigned char  segment;
        unsigned char  key;
        unsigned char  info[4];
        unsigned char  added;
        unsigned char  reserved[4];
        unsigned char  code;
        unsigned char  fill1;
        unsigned char  fru;
        unsigned char  bit;
        unsigned short  field;
} SCSI_SENSE;

#define SCSI_CAPACITY_CMD       0x25
typedef
struct scsi_capacity {
        unsigned long   lba;
        unsigned long   blen;
} SCSI_CAPACITY;

#define SCSI_READ_CMD           0x28
#define SCSI_WRITE_CMD          0x2A
