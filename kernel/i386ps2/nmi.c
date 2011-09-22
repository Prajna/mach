/* 
 * Mach Operating System
 * Copyright (c) 1991,1990,1989 Carnegie-Mellon University
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
 * $Log:	nmi.c,v $
 * Revision 2.2  93/02/04  08:01:27  danner
 * 	Integrate PS2 code from IBM.
 * 	[93/01/18            prithvi]
 * 
 */

#include <platforms.h>
#include <cpus.h>

#include <sys/types.h>

#ifdef PS2
#include <i386ps2/abios.h>

struct NMI_params {
	u_short	Type;
	u_char	_Dummy1[12];
	u_char	Arb;
	u_char	Slot;
	u_char	_Dummy2;
	u_char	Reserved[2];
};

#define nmi_type	un.nmi_params.Type
#define nmi_arb		un.nmi_params.Arb
#define nmi_slot	un.nmi_params.Slot
#define nmi_reserved	un.nmi_params.Reserved

struct nmi_request {
	struct	Request_header	request_header;
	union {
		struct	Logical_id_params	logical_id_params;
		struct	NMI_params		nmi_params;
	} un;
};
#define NMI_PARITY	0x01
#define NMI_CHANNEL	0x02
#define NMI_DMA 	0x03
#define NMI_WATCHDOG	0x04

static struct nmi_request *nmi_request_block;
static struct nmi_request *nmi_enable_block;
static int     nmi_flags;
char nqbuf[200];        /*XXX temparary.. should use kmem_alloc or whatever..*/

nmi_enable()
{
        struct generic_request  temp_request_block;
        int rc;

        temp_request_block.r_current_req_blck_len = ABIOS_MIN_REQ_SIZE;
        temp_request_block.r_logical_id = abios_next_LID(NMI_ID,
                                                        ABIOS_FIRST_LID);
        temp_request_block.r_unit = 0;
        temp_request_block.r_function = ABIOS_LOGICAL_PARAMETER;
        temp_request_block.r_return_code = ABIOS_UNDEFINED;

        abios_common_start(&temp_request_block,0);
        if (temp_request_block.r_return_code != ABIOS_DONE) {
                printf("couldn't init abios nmi code!\n");
		return;
        }

        /*
         * now build the clock request for the hardware system clock
         */
        nmi_request_block = (struct nmi_request *)nqbuf;
        nmi_request_block->r_current_req_blck_len =
                                temp_request_block.r_request_block_length;
        nmi_request_block->r_logical_id = temp_request_block.r_logical_id;
        nmi_request_block->r_unit = 0;
        nmi_flags = temp_request_block.r_logical_id_flags;
        nmi_request_block->r_return_code = ABIOS_UNDEFINED;
        nmi_request_block->r_function = ABIOS_READ;
        nmi_request_block->nmi_reserved[0] = 0;
        nmi_request_block->nmi_reserved[1] = 0;

	/* start the continuous read request */
	abios_common_start(nmi_request_block,nmi_flags);

	/* build the enable request structure */
	nmi_enable_block = (struct nmi_request *)
			(&nqbuf[temp_request_block.r_request_block_length]);
        nmi_enable_block->r_current_req_blck_len =
                                temp_request_block.r_request_block_length;
        nmi_enable_block->r_logical_id = temp_request_block.r_logical_id;
        nmi_enable_block->r_unit = 0;
        nmi_enable_block->r_return_code = ABIOS_UNDEFINED;
        nmi_enable_block->r_function = ABIOS_ENABLE_INTR;
        nmi_enable_block->nmi_reserved[0] = 0;
        nmi_enable_block->nmi_reserved[1] = 0;
	abios_common_start(nmi_enable_block,nmi_flags);
}



int
nmi_trap(type,code,pc,locr0)
	int	type,code,pc;
	int	*locr0;
{
	if (nmi_request_block) {
        	nmi_request_block->r_return_code = ABIOS_UNDEFINED;
		abios_common_interrupt(nmi_request_block,nmi_flags);
		if (nmi_request_block->r_return_code != ABIOS_STAGE_ON_INT) {
			printf("Unknown NMI: does not belong to system\n");
		} else {
			printf("NMI: type=");
			switch(nmi_request_block->nmi_type) {
			case NMI_PARITY:
				printf("memory parity error");
				break;
			case NMI_CHANNEL:
				printf(
				"I/O channel check from card in slot %d\n",
						nmi_request_block->nmi_slot);
				break;
			case NMI_DMA:
				printf(
				"DMA Bus timeout error on arb level %d\n",
						nmi_request_block->nmi_arb);
				break;
			
			case NMI_WATCHDOG:
				printf
				  ("system clock hardware watchdog timeout\n");
				break;
			default:
				printf("Unknown");
				break;
			}
			/*
			 * some day we may want to keep going. AT that point
			 * we should call abios_common_start with 
			 * nmi_enable_block
			 */
			printf(" not restarting\n");
		}
	}
	return(0);
}
#endif
