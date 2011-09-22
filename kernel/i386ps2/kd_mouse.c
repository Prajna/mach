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
 * $Log:	kd_mouse.c,v $
 * Revision 2.3  93/08/02  21:44:09  mrt
 * 	Rewrite mouse driver to work correctly.
 * 	[93/07/12            zon]
 * 
 * Revision 2.2  93/02/04  08:00:56  danner
 * 	Integrate PS2 code from IBM.
 * 	[93/01/18            prithvi]
 * 
 */

/* PS/2 Mouse Driver */

#include <mach/boolean.h>
#include <sys/types.h>
#include <device/errno.h>
#include <device/io_req.h>

#include <kern/queue.h>

#include <i386/ipl.h>

#include <i386at/kd.h>
#include <i386at/kd_queue.h>

#include <i386ps2/abios.h>
#include <i386ps2/kd_mouse_abios.h>
#include <i386ps2/kd_mouse_io.h>

boolean_t mouse_in_use = FALSE;

kd_event_queue mouse_queue;

queue_head_t mouse_read_queue = { &mouse_read_queue, &mouse_read_queue };

int kd_mouse = 0; /* used by kd.c */

struct Mouse_request mouserb;
struct Mouse_request mousecontrb;

struct Logical_id_params mouseparams = { 0 };
struct Logical_id_params mousecontparams = { 0 };

int last_buttons = 0;
#define MOUSE_UP        1
#define MOUSE_DOWN      0
#define MOUSE_ALL_UP    0x7

#define SPLMS SPLKD

static void
	mouseabioswait(struct Mouse_request *request,
		       int abios_function,
		       int mouse_flag);

static int mouseintr(void);


static int mouse_setup(void)
{
	static int already_done = 0;
        int lid_num;

	if (already_done)
		return 0;

	kdq_reset(&mouse_queue);

	last_buttons = MOUSE_ALL_UP;

	/* Init mouse hardware: get logical parameters */
	mouserb.r_current_req_blck_len = sizeof(struct Mouse_request);
	mouserb.request_header.Request_Block_Flags = 0;
	mouserb.request_header.ELA_Offset = 0;
	lid_num = MS_ID;
	lid_num = abios_next_LID(MS_ID, lid_num);
	MOUSE_SET_RESERVED_ABIOS_LOGICAL_PARAMETER(mouserb);
	mouserb.r_logical_id = lid_num;
	mouserb.r_unit = 0;
	mouserb.r_flags = MS_NOERR;
	mouseabioswait(&mouserb,
		       ABIOS_LOGICAL_PARAMETER,
		       mouseparams.Logical_id_flags);

	if (mouserb.r_return_code != ABIOS_MOUSE_RC_DONE)
	  if (mouserb.r_return_code == ABIOS_MOUSE_RC_INVALID_LOGICAL_ID)
	    return -1;

	if (mouserb.r_request_block_length > mouserb.r_current_req_blck_len)
	  panic("mouse: rb len");

	mouserb.r_current_req_blck_len = mouserb.r_request_block_length;
	mouseparams = mouserb.un.logical_id_params;

	take_irq(mouserb.r_hardware_intr, 0, mouseintr, SPLTTY, "ms");

	/*
	 * Continuous Read has its own Request Block, because it will
	 * always run in the background.
	 */
	mousecontrb.r_current_req_blck_len = mouserb.r_current_req_blck_len;
	mousecontrb.request_header.Request_Block_Flags =
	        mouserb.request_header.Request_Block_Flags;
	mousecontrb.request_header.ELA_Offset =
	        mouserb.request_header.ELA_Offset;
	mousecontrb.r_logical_id = mouserb.r_logical_id;
	mousecontrb.r_unit       = mouserb.r_unit;
	mousecontparams          = mouseparams;
	MOUSE_SET_RESERVED_ABIOS_POINTING_DEVICE_CONTINUOUS_READ(mousecontrb);
	mousecontrb.r_8_data_package_size = 3;
	mousecontrb.r_flags = MS_DEFLT;
	mouseabioswait(&mousecontrb,
		       ABIOS_MOUSE_POINTING_DEVICE_CONTINUOUS_READ,
		       mousecontparams.Logical_id_flags);

	already_done = 1;
	return 0;

}

int mouseopen(dev_t dev, int flags)
{

        if (mouse_in_use)
	    return EBUSY;
	mouse_in_use = TRUE;

	if (mouse_setup())
		return D_NO_SUCH_DEVICE;

	/* Reset/Initialize Pointing Device */
	MOUSE_SET_RESERVED_ABIOS_RESET_POINTING_DEVICE(mouserb);
	mouserb.r_flags = MS_DEFLT;
	mouseabioswait(&mouserb,
		       ABIOS_MOUSE_RESET_POINTING_DEVICE,
		       mouseparams.Logical_id_flags);

        /* Enable Pointing Device */
        MOUSE_SET_RESERVED_ABIOS_ENABLE_POINTING_DEVICE(mouserb);
	mouserb.r_flags = MS_DEFLT;
	mouseabioswait(&mouserb,
		       ABIOS_MOUSE_ENABLE_POINTING_DEVICE,
		       mouseparams.Logical_id_flags);

	return D_SUCCESS;
}

int mouseclose(dev_t dev, int flags)
{
        int  s;

        s = SPLMS();

	MOUSE_SET_RESERVED_ABIOS_DISABLE_POINTING_DEVICE(mouserb);
	mouserb.r_flags = MS_DEFLT;
	mouseabioswait(&mouserb,
		       ABIOS_MOUSE_DISABLE_POINTING_DEVICE,
		       mouseparams.Logical_id_flags);

	splx(s);

	kdq_reset(&mouse_queue);

	mouse_in_use = FALSE;

	return 0;
}

#ifdef PRINT_EVENT
static void print_event(register kd_event *ev)
{

	switch (ev->type) {

	case MOUSE_LEFT:
		printf("LEFT %s\n", ev->value.up ? "UP" : "DOWN");
		break;

	case MOUSE_MIDDLE:
		printf("MIDDLE %s\n", ev->value.up ? "UP" : "DOWN");
		break;

	case MOUSE_RIGHT:
		printf("RIGHT %s\n", ev->value.up ? "UP" : "DOWN");
		break;

	case MOUSE_MOTION:
		printf("MOVE DX=%d DY=%d\n",
		       ev->value.mmotion.mm_deltaX,
		       ev->value.mmotion.mm_deltaY);
		break;

	default:
		printf("ev->type==%d\n", ev->type);

	}

}
#endif

static boolean_t mouse_read_done(io_req_t ior);

int mouseread(dev_t dev, register io_req_t ior)
{
	int err;
	int s;
	int count;

	err = device_read_alloc(ior, (vm_size_t)ior->io_count);
	if (err != KERN_SUCCESS)
	    return err;

	s = SPLMS();

	if (kdq_empty(&mouse_queue)) {
	    if (ior->io_mode & D_NOWAIT) {
	        splx(s);
		return D_WOULD_BLOCK;
	    }
	    ior->io_done = mouse_read_done;
	    enqueue_tail(&mouse_read_queue, (queue_entry_t) ior);
	    splx(s);
	    return D_IO_QUEUED;
	}

	count = 0;
	while (!kdq_empty(&mouse_queue) && count < ior->io_count) {
	    register kd_event *ev = kdq_get(&mouse_queue);
#ifdef PRINT_EVENT
	    print_event(ev);
#endif
	    *(kd_event *)(&ior->io_data[count]) = *ev;
	    count += sizeof(kd_event);
	}

	splx(s);

	ior->io_residual = ior->io_count - count;

	return D_SUCCESS;

}

static boolean_t mouse_read_done(io_req_t ior)
{
	int s;
	int count;

	s = SPLMS();

	if (kdq_empty(&mouse_queue)) {
	    ior->io_done = mouse_read_done;
	    enqueue_tail(&mouse_read_queue, (queue_entry_t)ior);
	    splx(s);
	    return FALSE;
	}

	count = 0;
	while (!kdq_empty(&mouse_queue) && count < ior->io_count) {
	    kd_event *ev = kdq_get(&mouse_queue);
#ifdef PRINT_EVENT
	    print_event(ev);
#endif
	    *(kd_event *)(&ior->io_data[count]) = *ev;
	    count += sizeof(kd_event);
	}
	splx(s);

	ior->io_residual = ior->io_count - count;
	ds_read_done(ior);

	return TRUE;
}

/*
 * 3 byte ps2 format used
 *
 * 7  6  5  4  3  2  1  0
 * YO XO YS XS 1  0  R  L
 * X7 X6 X5 X4 X3 X3 X1 X0
 * Y7 Y6 Y5 Y4 Y3 Y2 Y1 Y0
 *
 */
#define BUTTON_MASK		0x3
#define LBUTTON_MASK		0x1
#define RBUTTON_MASK		0x2
#define MBUTTON_MASK		0x3
#define XNEG			0x00000010
#define YNEG			0x00000020
#define XOVER			0x00000040
#define YOVER			0x00000080
#define DELTA_MASK		0x000000ff
#define SIGNXTND		0xffffff00

static void mouse_enqueue(register kd_event *ev)
{
	register io_req_t ior;

	if (kdq_full(&mouse_queue))
		printf("mouse: queue full\n");
	else
		kdq_put(&mouse_queue, ev);

	while ((ior = (io_req_t)dequeue_head(&mouse_read_queue)) != 0)
	    iodone(ior);

}

static void mouse_moved(struct mouse_motion where)
{
	kd_event ev;

	ev.type = MOUSE_MOTION;
	ev.time = time;
	ev.value.mmotion = where;
	mouse_enqueue(&ev);

}

static void mouse_button(kev_type which, int up)
{
	kd_event ev;

	ev.type = which;
	ev.time = time;
	ev.value.up = up;
	mouse_enqueue(&ev);

}

static void mouse_packet(register int buttons, int x, int y)
{
	unsigned char buttonchanges;
	struct mouse_motion moved;

	moved.mm_deltaX = ((buttons & XNEG) ? SIGNXTND : 0 ) | x;
	moved.mm_deltaY = ((buttons & YNEG) ? SIGNXTND : 0 ) | y;
	if (moved.mm_deltaX || moved.mm_deltaY)
	    mouse_moved(moved);

	buttons &= BUTTON_MASK;
	if (buttonchanges = buttons ^ last_buttons) {
	    last_buttons = buttons;
	    switch (buttonchanges) {
		case LBUTTON_MASK:
		    mouse_button(MOUSE_LEFT,  !(buttons & LBUTTON_MASK));
		    break;
		case RBUTTON_MASK:
		    mouse_button(MOUSE_RIGHT, !(buttons & RBUTTON_MASK));
		    break;
/* disable the middle button until it can be simulated properly */
/*
		case MBUTTON_MASK:
		    mouse_button(MOUSE_MIDDLE, buttons != MBUTTON_MASK);
		    break;
*/
		}
	}

}

static int mouseintr(void)
{

        abios_common_interrupt(&mousecontrb,
			       mousecontparams.Logical_id_flags);

	switch (mousecontrb.r_return_code) {

	case ABIOS_MOUSE_RC_DONE:
	        panic("mouseintr: continuous read stopped!");
		break;

	case ABIOS_MOUSE_RC_STAGE_ON_INT:
		break;

	case ABIOS_MOUSE_RC_STAGE_ON_TIME:
		timeout(mouseintr,
			0,
			(MOUSE_TIME_TO_WAIT(mousecontrb)/1000000) * hz);
		break;

        case ABIOS_MOUSE_RC_NOT_MY_INT:
	case ABIOS_UNDEFINED:
		/*
		 * Must belong to a different mouse function.
		 * Check the other request block.
		 */
		abios_common_interrupt(&mouserb,
				       mouseparams.Logical_id_flags);
		switch (mouserb.r_return_code) {
		case ABIOS_MOUSE_RC_DONE:
		        wakeup((char *)&mouserb.r_return_code);
			break;
		case ABIOS_MOUSE_RC_STAGE_ON_INT:
			break;
		case ABIOS_MOUSE_RC_STAGE_ON_TIME:
			timeout(mouseintr,
				0,
				(MOUSE_TIME_TO_WAIT(mouserb)/1000000)*hz);
			break;
		case ABIOS_MOUSE_RC_NOT_MY_INT:
		case ABIOS_UNDEFINED:
			break;  /* ??? */
		case ABIOS_MOUSE_RC_ATTENTION:
		default:
			panic("mouseintr: non CR attention");
			break;
		}
		break;

	case ABIOS_MOUSE_RC_ATTENTION:
		mouse_packet(mousecontrb.r_pointing_device_data_status,
			     mousecontrb.r_pointing_device_data_deltax,
			     mousecontrb.r_pointing_device_data_deltay);
		break;

	default:
		break;

	}

	return 0;

}

static void
	mouseabioswait(register struct Mouse_request *request,
		       int abios_function,
		       int mouse_flag)
{
	int done;

	request->r_function = abios_function;
	request->r_return_code = ABIOS_UNDEFINED;

	abios_common_start(request, mouse_flag);

	done = 0;
	while ((request->r_return_code != ABIOS_MOUSE_RC_DONE) && !done)
		switch (request->r_return_code) {

		case ABIOS_UNDEFINED:
	            break;

		case ABIOS_MOUSE_RC_STAGE_ON_INT:
		    if (request->r_function ==
			ABIOS_MOUSE_POINTING_DEVICE_CONTINUOUS_READ)
		        done = 1;
		    else
		        sleep((char *) &request->r_return_code, 0);
		    break;

		case ABIOS_MOUSE_RC_STAGE_ON_TIME:
		    timeout(mouseintr,
			    0,
			    (MOUSE_TIME_TO_WAIT(*request)/1000000)*hz);
		    if (request->r_function ==
			ABIOS_MOUSE_POINTING_DEVICE_CONTINUOUS_READ)
		        done = 1;
		    else
		        sleep((char *) &request->r_return_code, 0);
		    break;

		case ABIOS_MOUSE_RC_NOT_MY_INT:
		    /* Wait until we get an informative reply */
		    if (request->r_function ==
			ABIOS_MOUSE_POINTING_DEVICE_CONTINUOUS_READ)
		        printf("mouseabioswait: NMI waiting...\n");
		    else
		        sleep((char *) &request->r_return_code, 0);
		    break;

		case ABIOS_MOUSE_RC_ATTENTION:
		    /* We shouldn't get this so soon */
		    panic("mouseabioswait: early attention");
		    break;

		default:
		    if (!(request->r_flags & MS_NOERR))
		        panic("mouseabioswait %x", request->r_return_code);
		    else
		        done = 1;
		    break;

		}

}

int mousegetstat(dev_t dev, int flavor, int *data, unsigned int *count)
{
	int s;
        struct mouse_status *status;

	s = SPLMS();
	switch (flavor) {

        case MSIC_STATUS:
		if (*count < sizeof(struct mouse_status)/sizeof(int)) {
		    splx(s);
		    return KERN_INVALID_ARGUMENT;
		}
                /* 0x03 Read Device Parameters */
                status = (struct mouse_status *)data;
		MOUSE_SET_RESERVED_ABIOS_READ_PARAMETER(mouserb);
		mouserb.r_flags = MS_DEFLT;
		mouseabioswait(&mouserb,
			       ABIOS_READ_PARAMETER,
			       mouseparams.Logical_id_flags);
                /* Fill out data with mouse status */
		status->interface_status = mouserb.r_interface_status;
		status->data_package_size = mouserb.r_3_data_package_size;
		status->flag_word = mouserb.r_flag_word;
		status->current_resolution = mouserb.r_current_resolution;
                status->current_sample_rate = mouserb.r_current_sample_rate;
		break;

        case MSIC_PDIC:
		/* 0x0E Read Pointing Device Identication Code */
		MOUSE_SET_RESERVED_ABIOS_READ_POINTING_DEVICE_IDENTIFICATION_CODE(mouserb);
		mouserb.r_flags = MS_DEFLT;
		mouseabioswait(&mouserb,
			       ABIOS_MOUSE_READ_POINTING_DEVICE_IDENTIFICATION_CODE,
			       mouseparams.Logical_id_flags);
		*(long *)data =
		  (long)mouserb.r_auxiliary_device_identification_code;
		/* IBM's value is 0x0 */
		break;

        default:
	        splx(s);
		return EINVAL;

        }

	splx(s);
	return D_SUCCESS;

}

int mousesetstat(dev_t dev, int flavor, int *data, unsigned int count)
{
	int s;

	s = SPLMS();
	switch (flavor) {

	case MSIC_DISABLE:
	        /* 0x07 Disable Pointing Device */
	        MOUSE_SET_RESERVED_ABIOS_DISABLE_POINTING_DEVICE(mouserb);
		mouserb.r_flags = MS_DEFLT;
	        mouseabioswait(&mouserb,
			       ABIOS_MOUSE_DISABLE_POINTING_DEVICE,
			       mouseparams.Logical_id_flags);
                break;

	case MSIC_ENABLE:
		/* 0x06 Enable Pointing Device */
	        MOUSE_SET_RESERVED_ABIOS_ENABLE_POINTING_DEVICE(mouserb);
		mouserb.r_flags = MS_DEFLT;
	        mouseabioswait(&mouserb,
			       ABIOS_MOUSE_ENABLE_POINTING_DEVICE,
			       mouseparams.Logical_id_flags);
                break;

	case MSIC_SCALE:
		/* 0x0D Set Scaling Factor */
		MOUSE_SET_RESERVED_ABIOS_SET_SCALING_FACTOR(mouserb);
		mouserb.r_scaling_factor = (u_char) *data;
		mouserb.r_flags = MS_DEFLT;
		mouseabioswait(&mouserb,
			       ABIOS_MOUSE_SET_SCALING_FACTOR,
		               mouseparams.Logical_id_flags);
                break;

	case MSIC_SAMPLE:
		/* 0x0B Set Sample Rate */
           /*   MOUSE_SET_RESERVED_ABIOS_SET_SAMPLE_RATE(mouserb); */
                mouserb.r_sample_rate = (u_short) *data;
		mouserb.r_flags = MS_DEFLT;
	        mouseabioswait(&mouserb,
			       ABIOS_MOUSE_SET_SAMPLE_RATE,
		               mouseparams.Logical_id_flags);
                break;

	case MSIC_RESL:
                /* 0x0C Set Resolution */
		MOUSE_SET_RESERVED_ABIOS_SET_RESOLUTION(mouserb);
                mouserb.r_resolution = (u_short) *data;
		mouserb.r_flags = MS_DEFLT;
	        mouseabioswait(&mouserb,
			       ABIOS_MOUSE_SET_RESOLUTION,
		               mouseparams.Logical_id_flags);
                break;

	default:
		splx(s);
		return EINVAL;

        }

	splx(s);
	return D_SUCCESS;

}

