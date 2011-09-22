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
 * $Log:	fd.c,v $
 * Revision 2.3  93/03/11  14:08:53  danner
 * 	u_long -> u_int
 * 	[93/03/09            danner]
 * 
 * Revision 2.2  93/02/04  07:59:23  danner
 * 	Disable media-change detection for the floppy to allow booting
 * 	from them in the microkernel world.
 * 	[93/01/18            zon]
 * 
 * 	Integrate PS2 code from IBM.
 * 	[93/01/18            prithvi]
 * 
 */

/*
 * COMPONENT_NAME: (FLOPPY) Mach 3.0 PS/2 Floppy Driver  
 *
 * FUNCTIONS: 
 *
 * ORIGINS: 27
 */                                                                   

/*  OSF/1 PS/2 ABIOS diskette driver.  Driver lineage: AIX v3 R2 -> AIX v3 PS/2
 * -> OSF/1 PS/2.
 */


/* Changed for the 3.0 port */

#include <fd.h>

#ifdef 	MACH_KERNEL 
#include <sys/types.h>
#include <device/io_req.h>
#include <device/buf.h>
#include <device/errno.h>
#include <i386/pmap.h>
#include <sys/ioctl.h>
#include <sys/syslog.h>
#include <i386ps2/abios.h>
#include <i386/ipl.h>
#include <i386ps2/bus.h>
#include <i386ps2/debugf.h>
#include <i386ps2/fd_abios.h>
#include <i386ps2/fdreg.h>
#define PRIBIO          20  /* taken from i386at/m765.h */
#else	MACH_KERNEL
#include <sys/types.h>
#include <sys/buf.h>
#include <sys/errno.h>
#ifdef OSF
#include <sys/fcntl.h>
#else
#include <sys/file.h>
#endif OSF
#include <i386/pmap.h>
#include <sys/ioctl.h>
#include <sys/param.h>
#include <sys/proc.h>
#include <sys/syslog.h>
#include <sys/uio.h>
#include <sys/user.h>
#include <i386ps2/abios.h>
#include <i386/ipl.h>
#include <kern/assert.h>
#include <kern/task.h>
#ifdef OSF
#include <i386/handler.h>
#endif OSF
#include <i386ps2/bus.h>
#include "debugf.h"
#include "fd_abios.h"
#include "fdreg.h"
#include "fd.h"
#endif	MACH_KERNEL

extern int hz;

extern hd_delay(int);

#define HZ	hz

#if defined(FDDEBUG) && !defined(DEBUG)
#define DEBUG	1
#endif

#ifdef DEBUG
int fddebug = 0;
#define FDDEBUG(cond,args) if (cond) (args)
#define FDINIT		(fddebug & DEBUG_CONFIG)
#define FDOPEN		(fddebug & DEBUG_OPEN)
#define FDCLOSE		(fddebug & DEBUG_CLOSE)
#define FDREAD		(fddebug & DEBUG_READ)
#define FDWRITE		(fddebug & DEBUG_WRITE)
#define FDSTRAT		(fddebug & DEBUG_STRATEGY)
#define FDINTR		(fddebug & DEBUG_INTR)
#define FDIOCTL		(fddebug & DEBUG_IOCTL)
#define FDCMD		(fddebug & BIT16)
#define FDSTAT		(fddebug & BIT9)
#define FDABIOS		(fddebug & DEBUG_ABIOS)
#define FDSTUB		(fddebug & BIT17)
#define FDDETAIL	(fddebug & DEBUG_ERR_COND)
#else
#define FDDEBUG(cond,args)
#endif

struct buf *geteblk();		/* Added for Mach 3.0  */

/* Changed for the 3.0 port */

#ifdef	MACH_KERNEL
int fdopen(), fdclose(), fdread(), fdwrite(), fdstrategy(), fdintr();
#else	MACH_KERNEL
int fdopen(), fdclose(), fdread(), fdwrite(), fdstrategy(), fdioctl();
int fdintr();
#endif	MACH_KERNEL

/* Additional internal device driver routines. 
*/
int  fd_iocformat(dev_t, long, struct floppy *);
int  fd_iocsinfo(struct floppy *, long);
static void fd_sched(struct floppy *);
void fd_stot();
static int fdcall_abios(int, struct Fd_request *);
static int fdcall_abios_poll(int, struct Fd_request *, int);
int  fdcheck_sequence(struct floppy *);
static void fdcleanup(struct floppy *);
static int fdcommon_intr(struct Fd_request *);
int  fddisk_changed(struct floppy *);
int  fddoor_check(struct floppy *,int);
int  fderror(struct floppy *);
static int  fdexecute_command(struct floppy *, int);
static void fdget_adapter();
static void fdio(struct buf *);
static void fdiodone(struct buf *, struct Fd_request *, struct floppy *, int);
static int  fdissue_command(struct floppy *, int);
static int  fdload_floppy(struct floppy *, int);
int  fdmincnt(struct buf *);
int  fdsettle_test(struct floppy *, int);
static int  fdsetup_transfer(struct floppy *);
int  fdspeed_test(struct floppy *);
int  fdstage1_config(dev_t);
void fdtimer(struct fdwatchdog *);
static void fdlog_error();
static int d_max_page();

/* Macros:
*/
#define fdtype(fdp)		fdload_floppy(fdp,FDRMEDIA_PARMS)
#define fdstartio(fdp)		fdexecute_command(fdp,\
		(fdp->headptr->b_flags&B_READ) ? FDREAD_DATA : FDWRITE_DATA);

#define fd_no_retry(fdp)	(fdp)->retry_count = FDMAXRETRIES;
#define fd_clear_retry(fdp)	(fdp)->retry_count = \
				((fdp)->retry_flag == TRUE) ? 0 : FDMAXRETRIES;

/* Changed for the 3.0 Port */

#ifdef	MACH_KERNEL             
#define get_pmap(bp)		(kernel_pmap)
#else	MACH_KERNEL
#define get_pmap(bp)		( ((bp)->b_proc) ?\
					(bp)->b_proc->task->map->pmap :\
					kernel_pmap )
#endif	MACH_KERNEL


#define NS_PER_TICK	(1000000000/HZ)
#define NS_TO_TICKS(x)	((x + (NS_PER_TICK - 1)) / NS_PER_TICK)
#define us2tics(us)	NS_TO_TICKS( ((us) * 1000) )	/* us -> ticks */

/* Global vars:
*/
static struct adapter_structure fdadapter;
static struct floppy floppies[FDMAXDRIVES];
static struct buf fdbp[FDMAXDRIVES];

#ifdef OSF
static	ihandler_t	fd_handler;
static	ihandler_id_t	*fd_handler_id;
#endif OSF

static struct fd_data fd_data[] = {
	{ 18, 80, 2, 27, 108, 255 },		/* FDLOAD_144 */
	{ 9, 80, 2, 42, 80, 255 },		/* FDLOAD_720 */
	{ 9, 40, 2, 42, 80, 255 }		/* FDLOAD_360 */
};

static int fddrive_val[FDMAXDRIVES] = {
	FDDRIVE0,
	FDDRIVE1
};

static char *fd_tmp_errors[] = {
	"unsupported drive type",	/* FD_UNSUP_DRIVE	*/
	"unknown bytes/sector",		/* FD_UNSUP_SECT	*/
	"unsupported minor",		/* FD_UNSUP_MINOR	*/
	"lost interrupt",		/* FD_ERR_LOSTINT	*/
	"media not present",		/* FD_ERR_NOMEDIA	*/
	"media bad",			/* FD_ERR_RETRY		*/
	"media changed"			/* FD_ERR_CHMEDIA	*/
};

int fdprobe(), fdslave(), fdattach();

int (*fdcintrs[])() = { fdintr, 0};

struct i386_dev *fdinfo[NFD];
struct i386_ctlr *fdcinfo[NFDC];

struct  i386_driver      fdcdriver = {
	/* probe slave    attach   dname  dinfo   mname  minfo */
        fdprobe, fdslave, fdattach, "fd", fdinfo, "fdc", fdcinfo };

#ifdef DEBUG
#define MAX_ABIOS_FUNCTION_NAMES	(sizeof abios_function_names/ sizeof abios_function_names[0])
static char *abios_function_names[] =  {
	"def_int", "ret_log_id_param",				/* 0 - 1 */
	"reserved", "read_dev_param",				/* 2 - 3 */
	"set_dev_param", "reset",				/* 4 - 5 */
	"enable", "disable",					/* 6 - 7 */
	"read", "write",					/* 8 - 9 */
	"write-verify", "verify",				/* a - b */
	"read-media-type", "set-media-type",			/* c - d */
	"change-signal-status", "motor-off"			/* e - f */
	"int-status"						/* 10    */
};

#define ABIOS_FUNCTION_NAME(fn) ((unsigned) (fn) < MAX_ABIOS_FUNCTION_NAMES ? abios_function_names[fn] : "unknown")
#endif DEBUG

/*
 * TODO: 
 * call fdinit here so that we can set up interrupts properly before
 * open is called. Can't do it now because fdexecute_command currently
 * will sleep which appears to be a no-no at this stage.
 */

fdprobe(addr,ctlr)
struct i386_ctlr *ctlr;
{
	return(fdinit(ctlr) == FDSUCCESS);
}

/*
 * determine if the given slave (unit) is present
 */
fdslave(iod)
struct i386_dev *iod;
{
    if (iod->dev_slave >= fdadapter.fd_numd)
	return(0);		/* no such drive */
    return(1);
}

fdattach(iod)
struct i386_dev *iod;
{
}


int 
fdinit(ctlr)
struct i386_ctlr *ctlr;
{
  struct floppy *fdp = &floppies[0];
  int i, rc;

  FDDEBUG(FDINIT,printf("fdinit: start\n"));

  printf("IBM ABIOS Floppy Disk Driver Ver : 1.1\n");


  /* Initialize the adapter struture
  */
  fdadapter.fdintr_status = FDFREE;
  fdadapter.need_controller = FALSE;
  fdadapter.int_wd.func = NULL;
  fdadapter.mot_wd.func = NULL;
  fdadapter.need_sleep = 0;		/* NEEDS WORK -- needed ? */


  /* Initialize each floppy structure
   */
  for (i = 0; i < FDMAXDRIVES; i++) {
    floppies[i].drive_state = FDSTAT_CLOS;
    floppies[i].headptr = NULL;
    floppies[i].tailptr = NULL;
    floppies[i].close_sleep = 0;	/* NEEDS WORK -- sleep on? */
    floppies[i].timeout_flag = FALSE;
    floppies[i].retry_flag = TRUE;
    floppies[i].read_count_bytes = 0;
    floppies[i].read_count_megabytes = 0;
    floppies[i].write_count_bytes = 0;
    floppies[i].write_count_megabytes = 0;
    floppies[i].open_check = FALSE;
    floppies[i].motor_off_time = 10;

    /* NEEDS WORK: THE FOLLOWING CAN PROBABLY GO AWAY! */
    floppies[i].drive_type = D_135H;
    floppies[i].head_settle_time = 0;
    floppies[i].motor_start = 0;
  }

  fdadapter.int_wd.time = 5 * HZ;	/* 5 second interrupt watchdog */
  fdadapter.int_wd.func = fdtimer;
  fdadapter.int_wd.type = FDINT_WD;

  /* 10 second motor off watchdog
   */
  fdadapter.mot_wd.time = fdp->motor_off_time * HZ;
  fdadapter.mot_wd.func = fdtimer;
  fdadapter.mot_wd.type = FDMOT_WD;

  /*  Do initialization of ABIOS structures.  This must
   * be done before int_enable() and ABIOS_RESET.
   *
   * Note: we can set FDBUSY because we are resetting
   *	 the adapter and there can't be any other 
   *	 activity on the floppy.
   */
  fdadapter.fdintr_status = FDBUSY;


#ifdef	MACH_KERNEL
  rc = fdabios_init(fdp);
#else	MACH_KERNEL
  rc = fdexecute_command(fdp,FDABIOS_INIT);
#endif	MACH_KERNEL


  if (rc != FDSUCCESS) {
    FDDEBUG(FDOPEN,printf("FDOPEN: FDABIOS_INIT failed\n"));
    fdcleanup(NULL);
    return(EIO);
  }

  if (fdadapter.fd_numd == 0)		/* ABIOS sanity check */
  {
#ifdef  MACH_KERNEL
    return D_NO_SUCH_DEVICE;
#else   /* MACH_KERNEL */
    return(ENODEV);
#endif  /* MACH_KERNEL */
  }

  if (fdadapter.fd_numd > FDMAXDRIVES) {
    log(LOG_WARNING,"fd: %d drives present.  Only %d are supported\n",
	fdadapter.fd_numd, FDMAXDRIVES);
    fdadapter.fd_numd = FDMAXDRIVES;
  }

#ifdef OSF
  /* init interrupts */
  fd_handler.ih_level = fdadapter.fd_intl;
  fd_handler.ih_handler =  fdintr;
  fd_handler.ih_resolver = (int (*)()) NULL;
  fd_handler.ih_stats.intr_type = INTR_DEVICE;
  fd_handler.ih_stats.intr_cnt = 0;
  fd_handler.ih_priority = SPLFD;
  fd_handler.ih_flags = IH_FLAG_NOSHARE;
  if ((fd_handler_id = handler_add( &fd_handler )) != NULL)
    handler_enable( fd_handler_id );
  else
    return(ENXIO);
#else
  ctlr->ctlr_pic = fdadapter.fd_intl;
  ctlr->ctlr_spl = SPLFD;
  take_ctlr_irq(ctlr);
#endif /* OSF */
  return(FDSUCCESS);
}

/*
 * reset the adapter, since this causes an interrupt and hence
 * a sleep we are called from open not from probe/slave/attach
 */
fdreset()
{
  struct floppy *fdp = &floppies[0];
  int rc;

  /*  Reset the ABIOS "diskette system".  This does
   * stuff for the adapter, not a particular diskette
   * drive.
   */
  fdadapter.fdintr_status = FDBUSY;
  rc = fdexecute_command(fdp,FDABIOS_RESET);
  if (rc != FDSUCCESS) {
    FDDEBUG(FDOPEN, printf( "FDOPEN: FDABIOS_RESET failed\n"));
    fdcleanup(fdp);
    return(EIO);
  }
  
  return(FDSUCCESS);
}

/* NOTES: The 'devno' parameter always indicates which drive to open, and may
 * indicate the diskette characteristics to use.  The 'minor' macro is used
 * to extract the minor number from 'devno'.  If the minor number portion of
 * 'devno' is a 0 or a 1, then the fdopen() routine will attempt to
 * determine the drive type and diskette type by reading from the drive with
 * different drive characteristics.  This is done by calling the fdtype()
 * routine.  If the minor number is 4 or greater, then the diskette
 * characteristics implied by that format specific special file are used. 
 * The determination of the drive type can be suppressed by ORing the O_NDELAY
 * flag into the 'devflag' parameter.  This is mainly used by the format
 * command since the open will otherwise fail for an unformatted diskette. 
 */
int
fdopen(dev, flags)
     dev_t dev;
     int flags;
{
  register struct floppy *fdp;
  int drive;
  int rc;
  int firstopen = 1;		/* For MULTIPLE_OPENS */
  static int fdinited = 0;

  FDDEBUG(FDOPEN,printf("fdopen(%x,%x)\n",dev,flags));

  /*  This needs to be done early so we can use the values it
   * initializes.
   */
  if (!fdinited) {
    fdinited = 1;
    if ( (rc = fdreset()) != FDSUCCESS )
      return(rc);
  }

  drive = fd_drive(dev);		/* get the drive # */
  if (drive < 0 || drive >= fdadapter.fd_numd) {
    FDDEBUG(FDOPEN, printf("FDOPEN: bad drive number %d\n",drive));
    return (EINVAL);
  }

  fdp = &floppies[drive];

/* This was commented out for MACH 3.0 because flags is not passed down */

#ifdef	MACH_KERNEL
#else	MACH_KERNEL

  if (fdp->drive_state != FDSTAT_CLOS) {
    /*  Check to see if 2nd open has the same characteristics as the
     * previous opens, as well as exclusive opens.
     */
    FDDEBUG(FDOPEN, printf("fdopen: drive already open\n"));

    if ( (minor(fdp->device_number) != minor(dev)) ||
	 (flags & O_EXCL) || (fdp->drive_state & FDSTAT_EXOP) ) {
      return(EBUSY);
    }
    
    goto fd_multopen;	/* skip some 1st time stuff */
  }
#endif	MACH_KERNEL


  /* Load the structure 'floppy' with the default values and the minor
   * device number. 
   */
  fdp->device_number = dev;
  rc = fdload_floppy(fdp, FDREAD_PARMS);	/* drive info */
  if (rc != FDSUCCESS) {
    fdcleanup(fdp);
    FDDEBUG(FDOPEN,printf("FDOPEN: default load failed\n"));
    return (rc);
  }

  /* This has to happen before fddoor_check() so that we get past
   * the media changed signal successfully.
   */
  fdp->fd_change = FDCHANGE_NEW;

fd_multopen:

#ifdef	MACH_KERNEL
if (!(flags & D_NODELAY)) {

/*
   Commented out to allow working of floppy for Mach 3.0 

   rc = fddoor_check(fdp, (flags&D_WRITE));
*/
   if (rc != FDSUCCESS) {
	fdcleanup(fdp);
	return(rc);
   }
}
#else	MACH_KERNEL

  if (flags & O_EXCL)
    fdp->drive_state |= FDSTAT_EXOP;

  /* Don't read the diskette if open with O_NDELAY
  */
  if (!(flags & O_NDELAY)) {
    /* Check for diskette presence and write protection (if opening for
     * write).
     */
    rc = fddoor_check(fdp, (flags&FWRITE));
    if (rc != FDSUCCESS) {
      fdcleanup(fdp);
      return (rc);
    }
  }

#endif 	MACH_KERNEL

  else {
    /* If the O_NDELAY flag is set, the open is done.  Set
     * drive state to FDOPEN and return. 
     */
    fdp->drive_state = FDSTAT_OPEN;
    FDDEBUG(FDOPEN,printf("fdopen: opened O_NDELAY\n"));
    return(0);
  }

  /*  Some more stuff that needs to only be done on the first open.
  */
  if (firstopen) {
#ifdef NEEDS_WORK		/* NEEDS WORK */
/* NEEDS WORK --  Need to do media sence and r/w the diskette before we
** NEEDS WORK -- can determine it's soft type...someday.
*/
    /*  Call fdtype() to load the floppy structure with the
     * correct characteristics.
     */
    rc = fdtype(fdp);
    if (rc != FDSUCCESS) {
      FDDEBUG(FDOPEN, ("FDOPEN: fdtype failed\n"));
      fdcleanup(fdp);
      fdp->drive_state = FDSTAT_CLOS;
      return (rc);
    }
#endif

    /*  We're finally ready to set the open flag.  Note that
     * we don't want to set the open flag until the very end
     * so that we can tell if the drive was on a multiple
     * open or not.
     */
    fdp->drive_state = FDSTAT_OPEN;
    fdp->close_waiting = FALSE;
  }

  FDDEBUG(FDOPEN,printf("fdopen: successful\n"));
  return(0);
}

/*  Close the diskette drive.  Turn off the light if there is no other
 * activity on the adapter.  Make sure timers and stuff are off.
 */
int 
fdclose(devno)
     dev_t devno;
{
  register struct floppy *fdp;
  int pri;				/* for spl* calls */
  int drive;

  FDDEBUG(FDCLOSE, printf("fdclose: entering...\n"));

  drive = fd_drive(devno);

  FDDEBUG(FDCLOSE,printf("fdclose: drive %d\n", drive));

  /* Is the drive given valid?
   */
  if (drive < 0 || drive >= fdadapter.fd_numd)
    return (EINVAL);

  fdp = &floppies[drive];

  /* Check if see if finished with all i/o for this drive.  If
   * not, sleep until i/o is finished. 
   */
  pri = splfd();
  while (fdp->headptr != NULL) {
    FDDEBUG(FDCLOSE, printf("fdclose: wait for i/o to finish\n"));

    fdp->close_waiting = TRUE;
    sleep(&fdp->close_sleep, PRIBIO);	/* NEEDS WORK */
  }
  splx(pri);

  /* Mark this drive as FDCLOSED, call fdcleanup() to do any
   * cleanup that is possible (including turning off the
   * light if need be).
   */
  fdp->drive_state = FDSTAT_CLOS;
  fdcleanup(fdp);

  FDDEBUG(FDCLOSE, printf("fdclose: done\n"));
  return(0);
}

/*  The block i/o entry point.  Check parameters for validity and enqueue
 * the request, starting i/o if the adapter is free.
 */
int 
fdstrategy(bp)
     register struct buf *bp;
{
  register struct floppy *fdp;
  int pri;

  FDDEBUG(FDSTRAT, printf("fdstrategy: entering...\n"));

  fdp = &floppies[fd_drive(bp->b_dev)];
  bp->b_resid = 0;
  bp->b_error = 0;
  bp->av_forw = NULL;

  /* If the byte transfer length is not a multiple of 512
   * bytes, return EINVAL and return. 
   */
  if (bp->b_bcount % fdp->bytes_per_sector) {
    FDDEBUG(FDSTRAT, printf("fdstrategy: n != x*512\n"));
    bp->b_flags |= B_ERROR;
    bp->b_error = EINVAL;

    /* no data transferred */
    bp->b_resid += bp->b_bcount;

    iodone(bp);
    goto fd_done;
  }

  /*  Verify that i/o does not start past the end of the
   * diskette. Also, verify that the starting block number is
   * non-negative.
   *
   *  Error codes:
   *	EINVAL - the blkno is negative, or is a read past
   *	the last block on the media
   *	ENOSPC - A write past the end of the media.
   *
   *  Note that a read on the last block (actually 1st non-
   * valid block) is not an error, but not data is transferred.
   */
  if ( (bp->b_blkno < 0) || (bp->b_blkno >= fdp->number_of_blocks) ) {
    FDDEBUG(FDSTRAT, printf("fdstrategy: invalid blkno\n"));

    if (!(bp->b_flags & B_READ)) {	/* B_WRITE */
      bp->b_flags |= B_ERROR;
      if (bp->b_blkno < 0)
	bp->b_error = EINVAL;
      else
#ifdef	MACH_KERNEL
	bp->b_error = EIO;   		/* EIO is mapped D_IO_ERROR */
#else	MACH_KERNEL
	bp->b_error = ENOSPC;
#endif	MACH_KERNEL
    }
    else if (bp->b_blkno == fdp->number_of_blocks) {
      bp->b_error = 0;
    }
    else {
      bp->b_flags |= B_ERROR;
      bp->b_error = EINVAL;
    }

    /* all cases use the same b_resid */
    bp->b_resid = bp->b_bcount;

    iodone(bp);
    goto fd_done;
  }

  /*  See if there are any buffer headers in the queue.
   * If there are, put the buffer header at the end of
   * the queue.  If the queue is empty, put the buffer
   * header in the queue and call fdio(). 
   */
  pri = splfd();

  if (fdp->headptr != NULL) {	/* queue not empty */
    FDDEBUG(FDSTRAT, printf("fdstrategy: buf appended to q\n"));

    /* link it in the list:
     *	- old tail points at new element
     *	- new element becomes the new tail
     */
    fdp->tailptr->av_forw = bp;
    fdp->tailptr = bp;

    splx(pri);
  }
  else {			/* q empty -> start i/o */
    /* set up the queue of 1 buf struct
     *	- head and tail are the same.
     */
    fdp->headptr = fdp->tailptr = bp;

    /*  If the adapter is free (other diskettes also
     * not busy) then start the io.  If the adapter
     * is busy, then the interrupt handler will find
     * the request and start it up.
     *
     *  To start the i/o, re-enable interrupts, make
     * sure we can get the adapter (it will not sleep
     * if we are at interrupt level), and then start
     * the i/o.
     */
    if (fdadapter.fdintr_status == FDFREE) {
      FDDEBUG(FDSTRAT, printf("fdstrategy: startio\n"));

      splx(pri);
      fdget_adapter();
      fdio(bp);
    }
    else
      splx(pri);
  }

fd_done:
  FDDEBUG(FDSTRAT, printf("fdstrategy: done\n"));
  return (FDSUCCESS);
}

/*  Raw read.  Use physio() to do the transfer.
 *
 * NEEDS WORK - We should handle unaligned transfers for both read and
 * NEEDS WORK - in a common routine around here somewhere.
 */
int 
fdread(devno, uiop)
     dev_t devno;
     struct uio *uiop;
{
  register struct buf *bp = &fdbp[fd_drive(devno)];

#ifdef	MACH_KERNEL
#else	MACH_KERNEL
  register struct iovec *iov;
  register int cnt;
#endif	MACH_KERNEL

  FDDEBUG(FDREAD, printf("fdread: start\n"));


#ifdef	MACH_KERNEL
#else	MACH_KERNEL

  /*  Check for atleast 512 byte alignment on all io_vec's.
   */
  iov = uiop->uio_iov;
  for(cnt = 0 ; cnt < uiop->uio_iovcnt; cnt++) {
    if ( ((u_int)(iov++)->iov_base) & 0x1ff)
      return(EIO);
  }

#endif	MACH_KERNEL

  return(physio(fdstrategy,bp,devno,B_READ,fdmincnt,uiop));
}

/* Raw write.  Use physio() to do the transfer.
 */
int 
fdwrite(devno, uiop)
     dev_t devno;
     struct uio *uiop;
{
  register struct buf *bp = &fdbp[fd_drive(devno)];

#ifdef	MACH_KERNEL
#else	MACH_KERNEL
  register struct iovec *iov;
  register int cnt;
#endif	MACH_KERNEL

  FDDEBUG(FDWRITE, printf("fdwrite: start\n"));


#ifdef	MACH_KERNEL
#else	MACH_KERNEL
  /*  Check for atleast 512 byte alignment on all io_vec's.
   */
  iov = uiop->uio_iov;
  for(cnt = 0 ; cnt < uiop->uio_iovcnt; cnt++) {
    if ( ((u_int)(iov++)->iov_base) & 0x1ff)
      return(EIO);
  }
#endif	MACH_KERNEL

  return(physio(fdstrategy,bp,devno,B_WRITE,fdmincnt,uiop));
}

/* Changed for the 3.0 port */

#ifdef	MACH_KERNEL
#else	MACH_KERNEL

/* ioctl entry point.  Lots of status, plus things like format.
 *
 * NEEDS WORK -- no ioctl's are currently implemented.
 */
int 
fdioctl(devno, op, arg, devflag)
     dev_t devno;
     int op;
     register long arg;
     u_int devflag;
{
#ifdef NEEDS_WORK			/* NEEDS WORK: LOOK AT IOCTLs LATER */
  register struct floppy *fdp;
  register struct devinfo *devinfop;
  register struct fdinfo *fdinfop;
  register struct fdparms *fdparmsp;
  int tmp;
  int rc = FDSUCCESS;

  FDDEBUG(FDIOCTL, printf("fdioctl: start\n"));

  fdp = &floppies[fd_drive(devno)];

  switch (op) {

  case IOCINFO:
    /* The following ioctl operation is defined for every device
     * that uses the ioctl interface. 
     *
     * IOCINFO - returns some information about the diskette.
     * This is a standard ioctl option that can be issued to find
     * out information about any device that uses ioctls.  A
     * poiner to a structure of type devinfo should be passed in
     * the 'arg' parameter.  The information about the diskette
     * will be loaded into the devinfo structure. 
     */
    FDDEBUG(FDIOCTL, printf("fdioctl: IOCINFO\n"));

    /* Allocate a devinfo structure. */
    devinfop = (struct devinfo *) malloc(sizeof(struct devinfo));

    if (devinfop == NULL)
      rc = ENOMEM;
    else {
      devinfop->devtype = DD_DISK;
      devinfop->flags = DF_RAND;
      devinfop->un.dk.bytpsec =	(short) fdp->bytes_per_sector;
      devinfop->un.dk.secptrk = (short) fdp->sectors_per_track;
      devinfop->un.dk.trkpcyl = (short) fdp->tracks_per_cylinder;
      devinfop->un.dk.numblks = (long) fdp->number_of_blocks;

      /* Copy the structure to the user's address space.
       */
      tmp = copyout( (char *)devinfop, (char *)arg,
		    sizeof(struct devinfo) );
      if (tmp == -1)
	rc = EINVAL;

      free((caddr_t) devinfop);
    }
    break;

  case FDIOCFORMAT:
    /* See the header comment for fdiocformat()
     */
    FDDEBUG(FDIOCTL, printf("fdioctl: FDIOCFORMAT\n"));

    rc = fd_iocformat(devno,arg,fdp);

    FDDEBUG(FDIOCTL, printf("fdioctl: done\n"));
    break;

  case FDIOCGINFO:
    /* FDIOCGINFO - gets the current diskette characteristics.  A
     * pointer to a structure of type fdinfo must be passed in
     * the 'arg' parameter.  This operation will load the fdinfo
     * structure with the current diskette parameters. 
     */
    FDDEBUG(FDIOCTL, printf("fdioctl: FDIOCGINFO\n"));

    /* Allocate an fdinfo structure. */
    fdinfop = (struct fdinfo *) malloc(sizeof(struct fdinfo));

    if (fdinfop == NULL)
      rc = ENOMEM;
    else {
      fdinfop->type = (short) (fdp->drive_type);
      fdinfop->nsects = (int) (fdp->sectors_per_track);
      fdinfop->sides = (int) (fdp->tracks_per_cylinder);
      fdinfop->ncyls = (int) (fdp->cylinders_per_disk);

      /* Call copyout to copy the fdinfo structure to the
       * user. 
       */
      tmp = copyout((char *) (fdinfop), (char *) (arg),
		    sizeof(struct fdinfo));
      if (tmp == -1)
	rc = EINVAL;

      free((caddr_t) fdinfop);
    }
    break;

  case FDIOCSINFO:
    /* See the header comment for fd_iocsinfo()
     */
    FDDEBUG(FDIOCTL, printf("fdioctl: FDIOCSINFO\n"));
    rc = fd_iocsinfo(fdp,arg);
    break;

  case FDIOCRETRY:
  case FDIOCNORETRY:
    /* FDIOCRETRY - enables retries on errors.
     * FDIOCNORETRY - disables retries on errors.
     *
     * These operations require RAS_CONFIG authority.
     */
    FDDEBUG(FDIOCTL, printf("fdioctl: FDIOCNORETRY\n"));

    if (priv_chk(RAS_CONFIG))
      fdp->retry_flag = (op == FDIOCRETRY) ? TRUE : FALSE;
    else
      rc = EACCES;
    break;

  case FDIOCSTATUS:
    /* See header comment for fd_iocstatus()
     */
    rc = fd_iocstatus(fdp,arg);
    break;

  case FDIOCGETPARMS:
    /* FDIOCGETPARMS - gets various drive and diskette parameters
     * and returns them to the caller. 
     *
     * The following two ioctls are designed for use by the PC
     * simulator. 
     */
    FDDEBUG(FDIOCTL, printf("fdioctl: FDIOCGETPARMS\n"));

    /* Allocate an fdparms structure.
     */
    fdparmsp = (struct fdparms *) malloc(sizeof(struct fdparms));
    if (fdparmsp == NULL) {
      rc = ENOMEM;
      break;
    }

    fdparmsp->diskette_type = fdp->diskette_type;
    fdparmsp->sector_size = fdp->e_bytesps;
    fdparmsp->sectors_per_track = fdp->sectors_per_track;
    fdparmsp->sectors_per_cylinder = fdp->sectors_per_cylinder;
    fdparmsp->tracks_per_cylinder = fdp->tracks_per_cylinder;
    fdparmsp->cylinders_per_disk = fdp->cylinders_per_disk;
    fdparmsp->data_rate = fdp->data_rate;
    fdparmsp->head_settle_time = fdp->head_settle_time;
    fdparmsp->head_load = 0;
    fdparmsp->fill_byte = fdp->fill_byte;
    fdparmsp->step_rate = 0;
    fdparmsp->step_rate_time = 0;
    fdparmsp->gap = fdp->gap;
    fdparmsp->format_gap = fdp->format_gap;
    fdparmsp->data_length = fdp->data_length;
    fdparmsp->motor_off_time = fdp->motor_off_time;
    fdparmsp->bytes_per_sector = fdp->bytes_per_sector;
    fdparmsp->number_of_blocks = fdp->number_of_blocks;

    /* Call copyout to copy the fdinfo structure to the user. */

    tmp = copyout((char *) (fdparmsp), (char *) (arg), sizeof(struct fdparms));
    if (tmp == -1)
      rc = EINVAL;
    free((caddr_t) fdparmsp);
    break;

  case FDIOCSETPARMS:
#define	check(cnd,errno)	if (!(cnd)) {			
					free((caddr_t) fdparmsp);
					rc = (errno);		
					break;	
				}
#define cond(member)		(fdp->member == fdparmsp->member)

    /* FDIOCGETPARMS - sets various drive and diskette parameters
     * from values passed in from the caller. 
     */
    FDDEBUG(FDIOCTL, printf("fdioctl: FDIOCSETPARMS\n"));

    /* Allocate an fdparms structure. */

    fdparmsp = (struct fdparms *) malloc(sizeof(struct fdparms));
    if (fdparmsp == NULL) {
      rc = ENOMEM;
      break;
    }

    /* get the fdparms structure from the user
     */
    tmp = copyin((char *) (arg), (char *) (fdparmsp),
		 sizeof(struct fdparms));
    check(tmp == 0,EINVAL);

    /*  There are certin fields we just can't let the user
     * change, because it will reek havoc on the driver, and
     * potentially the kernel.  Here we check them, and return
     * EINVAL if they are bad.  Note they are also not set, but
     * we want to return EINVAL, just to make sure someone
     * doesn't get the delusion that something has changed.
     */
    check(fdp->diskette_type == fdparmsp->diskette_type,EINVAL);
    check(fdparmsp->head_load == 0,EINVAL);
    check(fdparmsp->step_rate == 0,EINVAL);
    check(fdparmsp->step_rate_time == 0,EINVAL);
    check(cond(sectors_per_track),EINVAL);
    check(cond(sectors_per_cylinder),EINVAL);
    check(cond(tracks_per_cylinder),EINVAL);
    check(cond(cylinders_per_disk),EINVAL);
    check(cond(data_rate),EINVAL);
    check(cond(head_settle_time),EINVAL);
    check(cond(number_of_blocks),EINVAL);
    check(cond(bytes_per_sector),EINVAL);

    /*  Now the fields we can really change with a "Set Device
     * Parameters" ABIOS call.  If the attribute has changed
     * then we save the original value, and call FD_SETDP_POLL
     * or FD_SETMT_POLL to attempt to change the value.  If the
     * call fails, we back out the change, and fail the ioctl
     * with EINVAL.
     */
    if (fdp->e_bytesps != fdparmsp->sector_size) {
      check((fdparmsp->sector_size==FD_256_BYTE_PER_SECTOR)||
	    (fdparmsp->sector_size==FD_512_BYTE_PER_SECTOR),
	    EINVAL);
      tmp = fdp->e_bytesps;
      fdp->e_bytesps = fdparmsp->sector_size;
      fdget_adapter();
      if (fdexecute_command(fdp,FD_SETDP_POLL)!=FDSUCCESS) {
	fdp->e_bytesps = tmp;
	free(fdparmsp);
	rc = EINVAL;
	break;
      }

      /*  When changing the sector_size, bytes_per_sector
       * is implicitly changed, and we need to reflect
       * this back to the user so he can continue to make
       * fdiocsetparms ioctl's with the current fdparm
       * structure.
       */
      fdparmsp->bytes_per_sector = fdp->bytes_per_sector;
      tmp = copyout((char *) (fdparmsp), (char *) (arg),
		    sizeof(struct fdparms));
      check(tmp == 0,EINVAL);
    }

    if (fdp->gap != fdparmsp->gap) {
      tmp = fdp->gap;
      fdp->gap = fdparmsp->gap;
      fdget_adapter();
      if (fdexecute_command(fdp,FD_SETDP_POLL)!=FDSUCCESS) {
	fdp->gap = tmp;
	free(fdparmsp);
	rc = EINVAL;
	break;
      }
    }

    if (fdp->data_length != fdparmsp->data_length) {
      tmp = fdp->data_length;
      fdp->data_length = fdparmsp->data_length;
      fdget_adapter();
      if (fdexecute_command(fdp,FD_SETDP_POLL)!=FDSUCCESS) {
	fdp->data_length = tmp;
	free(fdparmsp);
	rc = EINVAL;
	break;
      }
    }

    if (fdp->fill_byte != fdparmsp->fill_byte) {
      tmp = fdp->fill_byte;
      fdp->fill_byte = fdparmsp->fill_byte;
      fdget_adapter();
      if (fdexecute_command(fdp,FD_SETMT_POLL)!=FDSUCCESS) {
	fdp->fill_byte = tmp;
	free(fdparmsp);
	rc = EINVAL;
	break;
      }
    }

    if (fdp->format_gap != fdparmsp->format_gap) {
      tmp = fdp->format_gap;
      fdp->format_gap = fdparmsp->format_gap;
      fdget_adapter();
      if (fdexecute_command(fdp,FD_SETMT_POLL)!=FDSUCCESS) {
	fdp->format_gap = tmp;
	free(fdparmsp);
	rc = EINVAL;
	break;
      }
    }

    /*  Update the motor off time:  If it is really changed, then
     * calculate the delta between the 2 times, making sure the
     * value is atleast 1 second so it won't possibly get missed.
     */
    /* NEEDS WORK -- this code changed a touch from version 3
     * NEEDS WORK -- since we use 1/HZ timers instead of watch-
     * NEEDS WORK -- dogish timers, and we have to untimeout()
     * NEEDS WORK -- them, and can't update on the fly...Don't
     * NEEDS WORK -- if we can get it to change the time, until
     * NEEDS WORK -- with the next time out...
     */
    if (fdp->motor_off_time != fdparmsp->motor_off_time) {
#ifdef NEEDS_WORK	/* NEEDS WORK */
      tmp =	fdadapter.mot_wd.time - 
	(fdp->motor_off_time -
	 fdparmsp->motor_off_time);
      tmp = (tmp < 1) ? 1 : tmp;
      fdadapter.mot_wd.timer.count = tmp;
#endif
      fdadapter.mot_wd.time = fdparmsp->motor_off_time;
      fdp->motor_off_time = fdparmsp->motor_off_time;
    }

    free((caddr_t) fdparmsp);
    break;

  default:
    FDDEBUG(FDIOCTL, printf("fdioctl: no such ioctl\n"));
    rc = EINVAL;
    break;
  }

  FDDEBUG(FDIOCTL, printf("fdioctl: done\n"));
  return (rc);
#else
  return(EINVAL);
#endif
}
#endif	MACH_KERNEL


/*  This routine calls fdcall_abios() and uses sleep() to wait until
 * the call is completed.  The FD_RB_SYNC flag is set in the request
 * block so the interrupt handler knows to handle the request specially.
 *
 * The interrupt handler uses wakeup() to wake this process up, and does
 * not do an iodone() since a polled request will never come from the 
 * buffer cache.
 *
 * The 3rd parameter to this function (sched) is a boolean that indicates
 * if we should call fd_sched() to look for more ABIOS requests to process.
 * This is useful when a particular function requires multiple ABIOS
 * requests or the caller needs to use values out of the request block,
 * and does not want to have these values trashed by subsequent ABIOS calls.
 *
 * NOTES: fdcall_abios_poll() should not be used for ABIOS calls that do
 * not stage on time, or stage on interrupt.  Use fdcall_abios() for this
 * directly.  If the poll functionality is used in this case, then the
 * error code cannot be returned properly.
 *
 *  open_check gets set to FALSE when the polled request is done.  See
 * fdtimer() for more info.
 */
static int
fdcall_abios_poll(abios_function, rb, sched)
     int abios_function;
     register struct Fd_request *rb;
     int sched;
{
  register struct floppy *fdp;
  int rc;
  int pri;			/* for i_ calls */

  FDDEBUG(FDABIOS, printf("fdcall_abios_poll: function 0x%x (%s) rb=0x%x sched=%d\n",
	abios_function, ABIOS_FUNCTION_NAME(abios_function),rb,sched));

  fdp = &floppies[rb->r_unit];	/* get floppy structure */
  fdp->timeout_flag = FALSE;	/* clear timeout flag */
  rb->state |= FD_RB_SYNC;	/* this is a synchronous request */
  rb->req_errno = 0;		/* clear for non-intr calls */


  fdcall_abios(abios_function,rb);



  /*  Sleep until our request is finished.
   */
  
  pri = splfd();

  while (rb->r_return_code != ABIOS_DONE) {
    FDDEBUG(FDABIOS,printf("fdcall_abios_poll: calling sleep\n"));
    sleep(&rb->sleep_on_intr, PRIBIO);  /* NEEDS WORK */	
    FDDEBUG(FDABIOS, printf("fdcall_abios_poll: sleep done\n"));
  }


  splx(pri);


  /*  Any errors will be found on interrupt level and passed 
   * back to use through req_errno.
   */

  rc = rb->req_errno;

  pri = splfd();
  rb->state &= ~FD_RB_SYNC;	/* done with sync reqest */
  fdp->open_check = FALSE;	/* make sure this gets set off */

  /*  If the caller wants is to schedule more jobs, and the request
   * passed then try schedule a job.  If the request failed then
   * fdexecute_command() will end up calling fd_sched() for us.
   */

  if ( (sched == TRUE) && (rc == FDSUCCESS) )
    fd_sched(fdp);		/* look for additional requests */

  splx(pri);

  FDDEBUG(FDABIOS, printf("fdcall_abios_poll: done\n"));

  return (rc);
}

/*  Calls ABIOS (abios_common_start() is used).  Sets up a watchdog
 * timer to watch for losing interrupts.  Also sets up for the first
 * first stage on time by scheduling a stage on time "interrupt".
 */
static int
fdcall_abios(abios_function, rb)
     int abios_function;
     register struct Fd_request *rb;
{
  FDDEBUG(FDABIOS, printf("fdcall_abios: function 0x%x (%s)\n", abios_function,       ABIOS_FUNCTION_NAME(abios_function)));

  rb->r_function = abios_function;
  rb->r_return_code = ABIOS_UNDEFINED;
  rb->state |= FD_RB_STARTED;

  abios_common_start(rb, fdadapter.fd_flag);

  FDDEBUG(FDABIOS, {
      printf("[start_rw] RET abios_common_start... status=%x [", rb->r_return_code);
        fd_error_decode(rb->r_return_code, "]\n");});

  /*  Schedule a pseudeo interrupt if ABIOS tells us to wait.
   */


  if (rb->r_return_code == ABIOS_STAGE_ON_TIME) {
    FDDEBUG(FDABIOS,printf("fdcall_abios: ABIOS_STAGE_ON_TIME(%d)\n",
			   fd_wait_time(rb)));

    timeout(fd_stot,(caddr_t)NULL,us2tics(fd_wait_time(rb)));
  }

  /*  If this operation does not return immediatly, it will set
   * r_time_out.  Set a watchdog for the proper # of seconds
   * incase this operation does not complete.
   */

  if (rb->r_time_out != 0) {
    fdadapter.int_wd.time = (rb->r_time_out >> 3) * HZ;
    fdadapter.int_wd.fdp = &floppies[fdadapter.fd_req.r_unit];
    TIMEOUT(fdadapter.int_wd);
  }

  FDDEBUG(FDABIOS, printf("fdcall_abios: done\n"));
  return(FDSUCCESS);
}

/*  Does some front/back end processing for fdissue_command().
 *
 *  Calls fdissue_command() and arranges for proper scheduling of further
 * jobs after that request is done.
 */
static int 
fdexecute_command(fdp, arg)
     register struct floppy *fdp;
     int arg;
{
  int sched_needed;		/* schedule flag */
  int pri;			/* for i_* */
  int rc;

  /*  This is a safty catch.  It is assumed that the job calling
   * fdexecute_command() has gotten access to the controller.
   */
  assert(fdadapter.fdintr_status == FDBUSY);

  rc = fdissue_command(fdp, arg);

  if (rc == FDSUCCESS) {
    switch (arg) {
    case FDABIOS_INIT:
    case FDREAD_PARMS:
    case FDRMEDIA_PARMS:
    case FDLOAD_144:
    case FDLOAD_720:
    case FDLOAD_360:
    case FD_SETDP_POLL:
      /*  For commands that poll and need to read data
       * out of the request block, we must schedule
       * the pending requests.
       */
      sched_needed = TRUE;
      break;

    default:
      sched_needed = FALSE;
      break;
    }
  }
  else {
    /*  A schedule is not on polled requests that fail.  All
     * non-polled requests return FDSUCCESS always.
     */
    sched_needed = TRUE;
  }

  /*  If we have a status of free, check to see if there are any
   * requests to do, and if so, do start them up.
   */
  pri = splfd();
  if (sched_needed == TRUE)
    fd_sched(fdp);
  splx(pri);

  return (rc);
}

/*  Sets up the floppy structure for i/o and calls fdstartio. This routine
 * translates the starting block of a read or write into the cylinder,
 * head, sector address required by the controller.  It then computes
 * the total number of reads or writes needed and initializes the fields
 * in the floppy structure that are used to execute a read or write. 
 *
 * NOTES: This routine is called once for each buf struct processed.  It is
 * called by both fdstrategy() and fdintr() to initiate a new i/o sequence
 * for a given buf struct. 
 */
static void 
fdio(bp)
     register struct buf *bp;
{
  register struct floppy *fdp;
  u_char drive;
  daddr_t last_block;

  drive = fd_drive(bp->b_dev);
  fdp = &floppies[drive];
  FDDEBUG(FDCMD, printf("fdio: start io drive %d\n", drive));

  /* See if a request goes past the end of the diskette.
   */
  last_block = bp->b_blkno + (bp->b_bcount / fdp->bytes_per_sector);

  fdp->modified_bcount = bp->b_bcount;	/* cnt for this transfer */

  if (last_block > fdp->number_of_blocks) {
    /* Request starts before the end of the diskette but
     * runs over the end. Set up to do a partial data transfer
     * and return the number of bytes not transferred in the
     * b_resid field. 
     */
    FDDEBUG(FDCMD, printf("fdio: truncated request\n"));

    bp->b_resid =	(last_block - fdp->number_of_blocks) *
      fdp->bytes_per_sector;
    fdp->modified_bcount -= bp->b_resid;
  }

  /* initialize io counters */
  fdp->buf_offset = 0;
  fdp->sectors_done = 0;
  fd_clear_retry(fdp);

  /*  Set up the transfer with fdsetup_transfer, and if this
   * succeeds, then actually start the transfer.
   */
  if (fdsetup_transfer(fdp))
    fdstartio(fdp);

  return;
}

/*  Takes an fdp, and sets the transfer in terms of what AIBOS understands.
 * Mainly this is converting an offset into a physical address on the
 * diskette.
 *
 *  We also have to check for splitting transfers that would encompass
 * multiple heads.  Another thing we do is set up the request block which
 * includes using pmap_extract() to get the physical address.
 */
static int
fdsetup_transfer(fdp)
     register struct floppy *fdp;
{
  daddr_t blkno;
  u_char cyl;
  u_char head;
  u_char sect;
  u_short sectors_left;
  u_short temp;
  int msize;			/* maximum size of transfer */
  u_int paddr;
  u_int vaddr;

  /*  Calculate blkno of transfer as where we were suppost to 
   * start plus where we currently are.
   */
  blkno = (daddr_t) fdp->headptr->b_blkno + fdp->sectors_done;

  /* Translate block number into physical address. 
   */
  FDDEBUG(FDCMD, printf("fdio: start block is #%d\n", blkno));
  cyl = fdp->start.cylinder = blkno / fdp->sectors_per_cylinder;
  temp = blkno % fdp->sectors_per_cylinder;
  head = fdp->start.head = temp / fdp->sectors_per_track;
  sect = fdp->start.sector = (temp % fdp->sectors_per_track) + 1;

  /* Set up the other fields in 'floppy' that are used for keeping
   * track of reads and writes. 
   */
  fdp->start_block = blkno;
  fdp->sector_count = fdp->modified_bcount / fdp->bytes_per_sector;

  /* compute number of sectors to r/w from starting cylinder
   */
  sectors_left = fdp->sectors_per_cylinder - head *
    fdp->sectors_per_track - sect + 1;

  /*  If a read requires of multiple sectors goes onto another cylinder
   * then we have to split it into multiple requests.
   */
  if (sectors_left < fdp->sector_count) {
    FDDEBUG(FDCMD, printf("fdsetup_transfer: multiple requests!!\n"));

    fdp->transfer_split = TRUE;
    fdp->start.transfer_length = sectors_left * fdp->bytes_per_sector;
  }
  else {
    fdp->start.transfer_length = fdp->modified_bcount;
    fdp->transfer_split = FALSE;
  }

  /* Check to see if transfer crosses a page boundry.  If so we
   * need to split it into multiple transfers.  Also set up both
   * the virtual and physical pointers for the ABIOS call.
   */
  vaddr = (u_int) fdp->headptr->b_un.b_addr + fdp->buf_offset;
  msize = d_max_page(fdp->headptr, vaddr, &paddr, fdp->start.transfer_length);
  fd_data_ptr_1(&fdadapter.fd_req) = vaddr;
  fd_data_ptr_2(&fdadapter.fd_req) = paddr;

  if (msize < fdp->start.transfer_length) {
    FDDEBUG(FDCMD,printf("fdsetup_transfer: split for page!\n"));

    if (!msize) {
      fdiodone(fdp->headptr,&fdadapter.fd_req,fdp,EIO);
      return(0);
    }

    fdp->transfer_split = TRUE;
    fdp->start.transfer_length = msize;
  }

  return(1);
}

/*  Builds a command for ABIOS, and then calls the ABIOS interface routines.
 *
 *  This should only be called when you have the FDBUSY flag set for you
 * job.  Currently only fdexecute_command() and fdcommon_intr() call this.
 */
static int 
fdissue_command(fdp, arg)
     register struct floppy *fdp;
     int arg;
{
  int rc = FDSUCCESS;
  register struct Fd_request *fdreq;
  u_char drive_number;
  int poll = arg & FDABIOS_POLL;	/* save poll status */

  arg &= ~FDABIOS_POLL;		/* make it transparent to switch */

  /* Set up:
   *	- reset timeout flag.
   *	- stop previous motor watchdog timer.
   *	- get local copies of the adapter pointer and the driveno
   */
  fdp->timeout_flag = FALSE;
  UNTIMEOUT(fdadapter.mot_wd);
  fdreq = &fdadapter.fd_req;
  drive_number = fd_drive(fdp->device_number);

  switch (arg) {
  case FDABIOS_INIT:
    /*  Does the first few ABIOS calls to initialize the 
     * request block, and find a little bit of info about
     * the adapter.  This is the first thing that should
     * be called for ABIOS, and before i_init() is done.
     *
     *  ie fdintr() must not be called till after this
     * done.  Nothing here will cause an interrupt.
     */
    FDDEBUG(FDCMD, printf("FDABIOS_INIT: entering...\n"));

    fdadapter.fd_flag = 0;
    fdreq->state = FD_RB_IDLE;
    fdreq->sleep_on_intr = 0;	/* NEEDS WORK -- sleep on? */

    /* First we have to set up the initial request block,
     * and call "Return Logical ID parameters" in order
     * to get info for future ABIOS calls.
     */

    FD_SET_RESERVED_ABIOS_LOGICAL_PARAMETER(fdreq);

    fdreq->r_current_req_blck_len = sizeof(struct Fd_request);
    fdreq->r_logical_id = abios_next_LID(FD_ID,2);
    fdreq->r_unit = 0;
    fdreq->request_header.Request_Block_Flags = 0; /* ABIOS32 */
    fdreq->request_header.ELA_Offset = 0;
    fd_no_retry(fdp);

    rc = fdcall_abios_poll(ABIOS_LOGICAL_PARAMETER, fdreq, FALSE);

    if (rc != FDSUCCESS)
      break;

    /*  Note:  We don't have to do an fdget_adapter() here
     * because the above call will not reschedule the floppy
     * adapter, and the FDBUSY flag will not get turned off.
     */

    /* update certain fields based on what abios just told us
     *  1) use the real block length
     *  2) logical .vs. physical ptr 
     *  3) hardware interrupt level
     *  4) # of drives on the adapter
     */
    fdreq->r_current_req_blck_len = fdreq->r_request_block_length;
    fdadapter.fd_flag = fdreq->r_logical_id_flags;
    fdadapter.fd_intl = fdreq->r_hardware_intr;
    fdadapter.fd_numd = fdreq->r_number_units;

    /* We need to make this call before ABIOS_RESET inorder
     * to get the motor_off_delay
     */
    
    FD_SET_RESERVED_ABIOS_READ_PARAMETER(fdreq);
    fdreq->r_unit = 0;
    fd_no_retry(fdp);

    rc = fdcall_abios_poll(ABIOS_READ_PARAMETER, fdreq, FALSE);

    fdadapter.motor_off_delay = fd_motor_off_delay_time(fdreq);
    /* rc falls through */
    break;

  case FDABIOS_RESET:
    /*  This does an ABIOS_RESET on the diskette system which
     * basically gets the adapter set up.  This should
     * be the 2nd thing called when initializing the adapter.
     * First FDABIOS_INIT should be called.
     */
    FDDEBUG(FDCMD, printf("FDABIOS_RESET: entering...\n"));

    FD_SET_RESERVED_ABIOS_RESET(fdreq);
    fdreq->r_unit = fd_drive(fdp->device_number);
    fd_no_retry(fdp);

    rc = fdcall_abios_poll(ABIOS_RESET, fdreq, TRUE);
    /* rc falls through */
    break;

  case FDREAD_PARMS:
    /*  This does a "Read Device Parameters" ABIOS call
     * inorder to populate the floppy structure with what
     * ABIOS thinks the values should be.
     */
    FDDEBUG(FDCMD, printf("FDREAD_PARMS: entering...\n"));

    FD_SET_RESERVED_ABIOS_READ_PARAMETER(fdreq);
    fdreq->r_unit = fd_drive(fdp->device_number);
    fd_no_retry(fdp);

    rc = fdcall_abios_poll(ABIOS_READ_PARAMETER, fdreq, FALSE);
    if (rc != FDSUCCESS)
      break;

    switch(fd_drive_type(fdreq)) {
    case FD_NO_DRIVE:
      rc = EIO;
      break;
    case FD_360_KB_DRIVE:
      fdp->drive_type = D_48;
      break;
    case FD_1440_KB_DRIVE:
      fdp->drive_type = D_135H;
      break;
    default:
      fdlog_error(fdp,FD_UNSUP_DRIVE);
      rc = EIO;
      break;
    }

    switch (fdp->e_bytesps = fd_bytes_per_sector(fdreq)) {
    case FD_256_BYTE_PER_SECTOR:
      fdp->bytes_per_sector = 256;
      break;
    case FD_512_BYTE_PER_SECTOR:
      fdp->bytes_per_sector = 512;
      break;
    default:
      fdlog_error(fdp,FD_UNSUP_SECT);
      rc = EIO;
    }

    /* Move info from request block to the fdp
     */
    fdp->sectors_per_track = fd_sectors_per_track(fdreq);
    fdp->abios_flags = fd_dev_ctrl_flag(fdreq);
    fdp->motor_start = fd_motor_start_delay_time(fdreq);
    fdp->cylinders_per_disk = fd_num_of_cylinders(fdreq);
    fdp->tracks_per_cylinder = fd_num_of_heads(fdreq);
    fdp->fill_byte = fd_format_fill_byte(fdreq);
    fdp->head_settle_time = fd_head_settle_time(fdreq);
    fdp->gap = fd_rwv_gap_len(fdreq);
    fdp->format_gap = fd_format_gap_len(fdreq);
    fdp->data_length = fd_data_len(fdreq);


    /* Calculate the rest of the fdp values:
     */
    fdp->number_of_blocks = fdp->sectors_per_track *
      fdp->cylinders_per_disk *	fdp->tracks_per_cylinder;
    fdp->sectors_per_cylinder = fdp->sectors_per_track *
      fdp->tracks_per_cylinder;
    break;

  case FDRMEDIA_PARMS:
    /*  This will report the media parameters used for the
     * the last r/w/format. 
     */
    FDDEBUG(FDCMD, printf("FDRMEDIA_PARMS: entering...\n"));

    FD_SET_RESERVED_ABIOS_FD_READ_MEDIA_TYPE(fdreq);
    fdreq->r_unit = fd_drive(fdp->device_number);
    fd_no_retry(fdp);

    rc = fdcall_abios_poll(ABIOS_FD_READ_MEDIA_TYPE, fdreq, FALSE);
    if (rc != FDSUCCESS)
      break;

    /* Convert ABIOS return to a real value
     */
    switch (fdp->e_bytesps = fd_bytes_per_sector(fdreq)) {
    case FD_256_BYTE_PER_SECTOR:
      fdp->bytes_per_sector = 256;
      break;
    case FD_512_BYTE_PER_SECTOR:
      fdp->bytes_per_sector = 512;
      break;
    default:
      fdlog_error(fdp,FD_UNSUP_SECT);
      rc = EIO;
    }

    /* On the PS/2 the 5 1/4" adapter is 250 KBPS.  all the
     * other adapters are 500 KBPS.
     */
    fdp->data_rate = (minor(fdp->device_number) & FD_5)
      ? FD250KBPS : FD500KBPS;

    /* Move data from request block into fdp
     */
    fdp->sectors_per_track = fd_sectors_per_track(fdreq);
    fdp->cylinders_per_disk = fd_num_of_cylinders(fdreq);
    fdp->tracks_per_cylinder = fd_num_of_heads(fdreq);
    fdp->gap = fd_rwv_gap_len(fdreq);
    fdp->format_gap = fd_format_gap_len(fdreq);
    fdp->data_length = fd_data_len(fdreq);

    fdp->number_of_blocks = fdp->sectors_per_track *
      fdp->cylinders_per_disk *	fdp->tracks_per_cylinder;
    fdp->sectors_per_cylinder = fdp->sectors_per_track *
      fdp->tracks_per_cylinder;

    /*  Now see if the values returned by the read media type
     * agree with the minor we're opened under.  If they do,
     * then everything is fine, otherwise return an appropriate
     * errno.
     */
    switch (minor(fdp->device_number) & ~FDDRIVEMASK) {
    case FD_GENERIC | FD_3:
    case FD_GENERIC | FD_5:
      /*  We don't have to check because by giving the
       * generic minor, we agree to use what the drive
       * tells us.
       */
      break;
    case FD_3H:
      if (fdp->number_of_blocks != 2880)
	rc = EIO;
      break;
    case FD_3L:
      if (fdp->number_of_blocks != 1440)
	rc = EIO;
      break;
    case FD_5L:
      if (fdp->number_of_blocks != 720)
	rc = EIO;
      break;
    case FD_5H:
    default:
      fdlog_error(fdp,FD_UNSUP_MINOR);
      rc = EINVAL;
      break;
    }

    /*  Lastly we want to set the diskette_type field.  We
     * already know the disk_type.  This will tell us how
     * the media in the diskette is formatted.
     */
    switch (fdp->number_of_blocks) {
    case 720:		/* 360K 5 1/4" */
      fdp->diskette_type = FD360_5;
      break;
    case 1440:		/* 720K 3 1/2" */
      fdp->diskette_type = FD720_3;
      break;
    case 2880:		/* 144M 3 1/2" */
      fdp->diskette_type = FD1440_3;
      break;
    default:
      fdlog_error(fdp,FD_UNSUP_DRIVE);
      rc = EINVAL;
    }

    break;

  case FDLOAD_144:
  case FDLOAD_720:
  case FDLOAD_360:
    /* These are dummy commands that do not use ABIOS, but
     * make the setting of drive parameters easier because
     * we can use a consistant interface.
     */
    {
      int off = arg & ~0x100;

      /* Copy the data from the array to the fdp
       */
      fdp->sectors_per_track = fd_data[off].sectors;
      fdp->cylinders_per_disk = fd_data[off].cylinders;
      fdp->tracks_per_cylinder = fd_data[off].heads;
      fdp->gap = fd_data[off].gap;
      fdp->format_gap = fd_data[off].format_gap;
      fdp->data_length = fd_data[off].data_len;

      fdp->number_of_blocks = fdp->sectors_per_track *
	fdp->cylinders_per_disk * fdp->tracks_per_cylinder;
      fdp->sectors_per_cylinder = fdp->sectors_per_track *
	fdp->tracks_per_cylinder;
    }
    break;

  case FDREAD_DATA:
    /*  This reads data off the diskette.  It finishes setting
     * up the the request block and then calls the ABIOS 
     * proper ABIOS interface (polled, or non-polled).
     *
     *  The data pointers should be set up before this is
     * called.
     *
     * NOTE: the retry count is controlled by fdio().
     */
    FDDEBUG(FDCMD, printf("fdissue_command: READ_DATA\n"));

    FD_SET_RESERVED_ABIOS_READ(fdreq);
    fdreq->r_unit = fd_drive(fdp->device_number);

    fd_num_sector_rw(fdreq) = fdp->start.transfer_length /
      fdp->bytes_per_sector;
    fd_cylinder_num(fdreq) = fdp->start.cylinder;
    fd_head_num(fdreq) = fdp->start.head;
    fd_sector_num(fdreq) = fdp->start.sector;

    FDDEBUG(FDCMD, printf("vaddr: %x\n",fd_data_ptr_1(fdreq)));
    FDDEBUG(FDCMD, printf("paddr: %x\n",fd_data_ptr_2(fdreq)));
    FDDEBUG(FDCMD, printf("# sectors: %d\n",
			  fdp->start.transfer_length / fdp->bytes_per_sector));
    FDDEBUG(FDCMD, printf("cylinder: %d\n",fdp->start.cylinder));
    FDDEBUG(FDCMD, printf("head: %d\n",fdp->start.head));
    FDDEBUG(FDCMD, printf("sector: %d\n",fdp->start.sector));

    rc = (poll) 	? fdcall_abios_poll(ABIOS_READ,fdreq, TRUE)
			: fdcall_abios(ABIOS_READ, fdreq);
    break;

  case FDWRITE_DATA:
    /*  This writes data to the diskette.  It finishes setting
     * up the the request block and then calls the ABIOS 
     * proper ABIOS interface (polled, or non-polled).
     *
     *  The data pointers should be set up before this is
     * called.
     *
     * NOTE: the retry count is controlled by fdio().
     */
    FDDEBUG(FDCMD, printf("fdissue_command: WRITE_DATA\n"));

    FD_SET_RESERVED_ABIOS_READ(fdreq);
    fdreq->r_unit = fd_drive(fdp->device_number);
    fd_num_sector_rw(fdreq) = fdp->start.transfer_length /
      fdp->bytes_per_sector;
    fd_cylinder_num(fdreq) = fdp->start.cylinder;
    fd_head_num(fdreq) = fdp->start.head;
    fd_sector_num(fdreq) = fdp->start.sector;

    FDDEBUG(FDCMD, printf("vaddr: %x\n",fd_data_ptr_1(fdreq)));
    FDDEBUG(FDCMD, printf("paddr: %x\n",fd_data_ptr_2(fdreq)));
    FDDEBUG(FDCMD, printf("# sectors: %d\n",
			  fdp->start.transfer_length / fdp->bytes_per_sector));
    FDDEBUG(FDCMD, printf("cylinder: %d\n",fdp->start.cylinder));
    FDDEBUG(FDCMD, printf("head: %d\n",fdp->start.head));
    FDDEBUG(FDCMD, printf("sector: %d\n",fdp->start.sector));
    
    rc = (poll)	? fdcall_abios_poll(ABIOS_WRITE, fdreq, TRUE)
		: fdcall_abios(ABIOS_WRITE, fdreq);
    break;

  case FDMOTOR_OFF:
    /*  This does an ABIOS request to turn the motor (and
     * the light) for the drive off.
     */
    FD_SET_RESERVED_ABIOS_FD_TURN_OFF_MOTOR(fdreq);
    fdreq->r_unit = fd_drive(fdp->device_number);
    fd_no_retry(fdp);

    rc = fdcall_abios_poll(ABIOS_FD_TURN_OFF_MOTOR, fdreq, TRUE);
    /* RC Falls through to break */

    /*  fdcall_abios_poll() will call fdsched() {like it shoud}
     * and restart the motor watchdog.
     */
    UNTIMEOUT(fdadapter.mot_wd);
    break;

  case FDFORMAT_TRACK:
    /*  This does and ABIOS request to format a track.  It
     * has to create an xmem descriptor and get the physical
     * address of the format information.
     *
     *  It fills in the request block and does the format
     * to how the drive was set up (high or low, etc).
     */
    FDDEBUG(FDCMD,"fdissue_command: FDFORMAT_TRACK\n");

    FD_SET_RESERVED_ABIOS_FD_SET_MEDIA_TYPE(fdreq);
    fdreq->r_unit = fd_drive(fdp->device_number);

    fd_sectors_per_track(fdreq) = fdp->sectors_per_track;
    fd_bytes_per_sector(fdreq) = fdp->e_bytesps;
    fd_num_of_cylinders(fdreq) = fdp->cylinders_per_disk;
    fd_format_fill_byte(fdreq) = fdp->fill_byte;
    fd_format_gap_len(fdreq) = fdp->format_gap;

    fd_no_retry(fdp);		/* don't try again */

    rc = fdcall_abios_poll(ABIOS_FD_SET_MEDIA_TYPE, fdreq, FALSE);
    if (rc != FDSUCCESS) {
      break;
    }

    fd_xfer_sub_function(fdreq) = FD_XFER_SUB_FORMAT;
    fd_cylinder_num(fdreq) = fdp->cylinder_id;
    fd_head_num(fdreq) = fdp->head_id;

    fd_clear_retry(fdp);		/* retry this request */

    rc = (poll) ? fdcall_abios_poll(ABIOS_ADDITIONAL_XFER, fdreq, TRUE)
      		: fdcall_abios(ABIOS_ADDITIONAL_XFER, fdreq);

    break;

  case FDDISK_SAME:
    /*  Check to see if the Change signal is high.  In order
     * to return a value w/o using the fdp structure, we use
     * two return codes that are > 0, and will look like
     * errors.  fdexecute_command() will see the error and
     * try to schedule more work for the diskette adapter
     * to do.
     *
     *  Note that we first check to see if the change signal
     * is supported.  If it is not supported with the current
     * drive, then we just say the diskette is the same.
     */
    if (!(fdp->abios_flags & FD_CHANGE_SIGNAL_AVAIL)) {
      rc = DISK_SAME;
      break;
    }

    FD_SET_RESERVED_ABIOS_FD_CHANGE_SIGNAL_STATUS(fdreq);
    fdreq->r_unit = fd_drive(fdp->device_number);
		
    fdcall_abios_poll(ABIOS_FD_CHANGE_SIGNAL_STATUS,fdreq,FALSE);

    /*  If the diskette changed signal is high report it.  If
     * no say the diskette is the same, even though this drive
     * may not support it.
     */
    if (fd_change_signal_status(fdreq) == FD_CHANGE_SIGNAL_ACTIVE)
      rc = DISK_CHANGED;
    else
      rc = DISK_SAME;
    break;

  case FD_SETDP:
    /*  This call does a "Set Device Parameters" ABIOS call
     * and is used by the one of the ioctl()'s.
     */
    FD_SET_RESERVED_ABIOS_WRITE_PARAMETER(fdreq);
    fd_bytes_per_sector(fdreq) = fdp->e_bytesps;
    fd_rwv_gap_len(fdreq) = fdp->gap;
    fd_data_len(fdreq) = fdp->data_length;

    fdreq->r_unit = fd_drive(fdp->device_number);

    rc = fdcall_abios_poll(ABIOS_WRITE_PARAMETER, fdreq, FALSE);

    if (rc == FDSUCCESS) {
      switch(fdp->e_bytesps) {
      case FD_256_BYTE_PER_SECTOR:
	fdp->bytes_per_sector = 256;
	break;
      case FD_512_BYTE_PER_SECTOR:
	fdp->bytes_per_sector = 512;
	break;
      default:
	fdlog_error(fdp,FD_UNSUP_SECT);
	rc = EIO;
      }
    }
    break;

  case FD_SETMT:
    /*  This call does a "Set Media Type for Format" ABIOS call
     * and is used by one of the ioctl's.  It is done at this
     * time instead of waiting till a format occurs to make
     * sure the data given is valid.
     */
    FD_SET_RESERVED_ABIOS_FD_SET_MEDIA_TYPE(fdreq);
    fd_sectors_per_track(fdreq) = fdp->sectors_per_track;
    fd_bytes_per_sector(fdreq) = fdp->e_bytesps;
    fd_num_of_cylinders(fdreq) = fdp->cylinders_per_disk;
    fd_format_fill_byte(fdreq) = fdp->fill_byte;
    fd_format_gap_len(fdreq) = fdp->format_gap;
    
    fdreq->r_unit = fd_drive(fdp->device_number);

    rc = (poll) ? fdcall_abios_poll(ABIOS_FD_SET_MEDIA_TYPE,fdreq,TRUE)
		: fdcall_abios(ABIOS_FD_SET_MEDIA_TYPE, fdreq);
    break;

  default:
    FDDEBUG(FDCMD, printf("fdissue_command: unknown command\n"));
    rc = EINVAL;
    break;
  }
  return (rc);
}

/*  Handles the diskette timer exceptions. This routine is called when
 * when a watchdog timer pops.  It then determines the proper actions to
 * take and does them. 
 *
 *  There are 2 types of watchdogs.  There is one to turn the motor off,
 * and one for lost interrupts.  The motor timer is easy, but the interrupt
 * timer is harder because it can mean different things depending on the
 * state of the drive, and the type of drive.
 *
 * NEEDS WORK: IS THE FOLLOWING STILL NEEDED, AND/OR TRUE WITH OSF?
 * NOTES: It is assumed that the interrupt level when the timer pops will
 * splfd().
 */
void 
fdtimer(timer)
     register struct fdwatchdog *timer;
{
  register struct floppy *fdp;
  int rc;

  switch (timer->type) {
  case FDINT_WD:
    /*  In this case an ABIOS request that should have finished
     * did not within the recommended timeout.  We log the
     * error, and then try to keep going...
     */
    FDDEBUG(FDCMD,printf("interrupt watchdog!\n"));
    fdp = fdadapter.int_wd.fdp;

    /*  We need to finish up with the request block early
     * incase we have to do any ABIOS calls later.
     */
    rc = fdadapter.fd_req.r_return_code;
    abios_common_timeout(&fdadapter.fd_req, fdadapter.fd_flag);
    fdadapter.fd_req.r_return_code = rc;

    /*  If the watchdog timeout came on the 5 1/4" drive
     * then it could be that the media is not present,
     * because this drive cannot sense the diskette is not
     * there.  Unfortunatly there is no good way to test
     * this.
     *
     *  The best we can do is in fddoor_check() is to set
     * a flag in the fdp structure, and if we get a watchdog
     * on a 5 1/4" drive and the flag is set, then we return
     * EIO.
     *
     *  The other problem encountered on the 5 1/4 is when
     * doing a format, without the diskette in the drive.
     * It looks like ABIOS doesn't do the reset so we do
     * it ourselves.  This only happens when opening NDELAY
     * since the drive isn't checked with a read.
     */
    if ((minor(fdp->device_number) & FD_5) && ((fdp->open_check == TRUE) ||
	(fdadapter.fd_req.r_function == ABIOS_ADDITIONAL_XFER) ) ) {
      fdlog_error(fdp,FD_ERR_NOMEDIA);
      rc = EIO;
    }
    else {
      /* Log timeout error. 
       */
      fdlog_error(fdp, FD_ERR_LOSTINT);
      fdp->timeout_flag = TRUE;
      rc = EIO;
    }

    /*  If we need to recalibrate (5 1/4" disk) then set up and
     * do the reset (note that this will do the fdiodone() for
     * us.  Otherwise call fdiodone().
     */
    if (fdp->abios_flags & FD_RECALIBRATE_REQUIRED) {
      register struct Fd_request *rb;

      rb = &fdadapter.fd_req;
      rb->state |= FD_RB_RESET;
      rb->req_errno = rc;
      fdcall_abios(ABIOS_RESET, rb);
    }
    else
      fdiodone(fdp->headptr, &fdadapter.fd_req, fdp, rc);

    break;

  case FDMOT_WD:
    /*  Note:  In this case the light is on.  All care has
     * been taken not to let the watchdog continue if this
     * drive is busy.  Hence we don't (and can't sleep())
     * call fdget_adapter(), and can just turn off the light.
     */
    FDDEBUG(FDCMD,printf("fdtimer: motor watchdog!\n"));
    fdadapter.fdintr_status = FDBUSY;
    fdexecute_command(fdadapter.mot_wd.fdp,FDMOTOR_OFF);
    break;
  }
}

#ifdef DEBUG
#undef DEBUG_RB
#ifdef DEBUG_RB

/*  This can be compiled in for printing out AIBOS request blocks for driver
 * debugging.
 */
static void
fd_dump_rb(rb)
     struct Fd_request *rb;
{
  register int i,j;

  printf("RB: len %d, lid %d, unit %d, func %\n",
	 rb->r_current_req_blck_len,
	 rb->r_logical_id,
	 rb->r_unit,
	 rb->r_function);
  printf("flags %d, ELA %d, rc %d, timeout %d\n",
	 rb->request_header.Request_Block_Flags,
	 rb->request_header.ELA_Offset,
	 rb->r_return_code,
	 rb->r_time_out);
	
  for (i=0 ; i < 128 ; i += 24) {
    for (j = 0 ; j < 24 ; j += 4)
      printf("%02x%02x%02x%02x  ",
	     rb->un.uc[i+j],
	     rb->un.uc[i+j+1],
	     rb->un.uc[i+j+2],
	     rb->un.uc[i+j+3]);
    printf("\n");
  }
		
  printf("state %x, sleep %x, req_errno %d\n",
	 rb->state, rb->sleep_on_intr, rb->req_errno);
}
#endif
#endif


/*  If DEBUG is on we also want to leave some hooks in to test the
 * watchdog timers for losing interrupts.  To test losing an interrupt,
 * have fd_inttest set to a non-zero value.
 *
 *  The best way to do this is to set a breakpoint in fdintr() and then
 * set the flag to 1 on the interrupt you want to simulate the loss of.
 *
 * WARNING -- Don't forget to set this flag back to 0, or nothing works!
 */
#ifdef DEBUG
#define WATCHTEST
#endif
#ifdef WATCHTEST
int fd_inttest = 0;
#endif

/*  This is the front end to the common interrupt handler for ABIOS
 * stage on time (stot) events.  When ABIOS tells us to stage on 
 * time we call timeout() which will set a timer to call fd_stot()
 * after the period has expired.
 *
 *  We call fdcommon_intr() which will query ABIOS with what to do next.
 * The one parameter to this routine is a pointer to the ABIOS request
 * block.
 */
void
fd_stot()
{
  int pri;

  FDDEBUG(FDINTR, printf("fd_stot: entering...\n"));
  pri = splfd();
  fdcommon_intr(&fdadapter.fd_req);
  splx(pri);
}

/*  This is the interrupt handler for the diskette controller.  It calls
 * fdcommon_intr with the current ABIOS request block to do most of the
 * work.  If this call corresponds to our interrupt, then we i_reset()
 * it, and in either case we return the return value from fdcommon_intr().
 *
 * NOTES: Turn off the interrupt watchdog.
 */
fdintr(fdtmp)
     register struct intr *fdtmp;
{
  /*  We have gotten our interrupt so turn off the timer ASAP.
   */
  UNTIMEOUT(fdadapter.int_wd);

  FDDEBUG(FDINTR, printf("\nfdintr: entering...\n"));

  fdcommon_intr(&fdadapter.fd_req);

  return(1);		/* return to FLIH */
}

/*  Handles an interrupt for the floppy driver.  First we check
 * to make sure we are expecting an interrupt.  If so we call
 * abios_common_interrupt() to process the interrupt.  Next we
 * do what abios_common_interrupt() told us to do.
 *
 *  When we have an ABIOS_DONE condition we call fdiodone() to terminate
 * the i/o for us.
 */
static int
fdcommon_intr(fdreq)
     register struct Fd_request *fdreq;
{
  register struct buf *bp;
  register struct floppy *fdp;

  FDDEBUG(FDINTR, printf("\nfdcommon_intr: entering...\n"));

  /*  Get data structures pointing at the right place.  Note that
   * the bp may be NULL if FD_RB_SYNC bit is set in the request
   * block.  In anycase if FD_RB_SYNC is set the bp should not
   * be used.
   */
  fdp = &floppies[fdreq->r_unit];
  bp = fdp->headptr;

  /* Make sure we are expecting an interuupt
   */
  if ( (fdreq->state==FD_RB_IDLE) || (fdreq->r_return_code==ABIOS_UNDEFINED))
    return(0);			/* not me! */

  abios_common_interrupt(fdreq, fdadapter.fd_flag);
  FDDEBUG(FDINTR, {
	  printf("fdcommon_intr: after abios_common_interrupt rc=0x%x [",
		 fdreq->r_return_code);
          fd_error_decode(fdreq->r_return_code, "]\n"); });

#ifdef WATCHTEST
  /* For more info about watchdog testing, see above comment.
   */
  if (fd_inttest) {		/* if true loose interrupt */
    return(0);
  }
#endif

  /*  If ABIOS_NOT_MY_INT and ABIOS_STAGE_ON_INT are both set in the
   * return code from abios_common_start(), then the interrupt
   * is not ours.  Return and let the slih go through the list
   * of devices on this interrupt level.
   */
  if ( fdreq->r_return_code & (ABIOS_NOT_MY_INT & ABIOS_STAGE_ON_INT) ) {
    FDDEBUG(FDINTR, printf("fdcommon_intr: ABIOS_NOT_MY_INTR\n"));
    return(0);				/* not me either! */
  }

  /*  Now check the various cases of the return code and do the
   * appropriate action.
   */
  switch (fdreq->r_return_code) {
  case ABIOS_DONE:
    /*  The request has completed successfully.  For read
     * and write we have to update some counters, but for
     * all other cases we're done.
     */
    FDDEBUG(FDINTR,printf("fdcommon_intr: ABIOS_DONE\n"));

    if ((fdreq->r_function == ABIOS_READ) ||
	(fdreq->r_function == ABIOS_WRITE)) {

      /*  Keep track of the number of sectors moved,
       * as well as updating the modified_bcount (used
       * for counting the i/o on strategy requests).
       */
      fdp->sectors_done += fd_sectors_moved(fdreq);
      fdp->modified_bcount -= fd_sectors_moved(fdreq) * fdp->bytes_per_sector;

      /* check if all sectors were moved
       */
      if ( fd_sectors_moved(fdreq) <
	  fd_num_sector_rw(fdreq) ) {
	fdiodone(bp,fdreq,fdp,EIO);
      }
      /* Check for a split request.
       *
       *  A request is split if it goes from sector that
       * is one head and is continued on the next head.
       *
       *  The other case is when the place to dma to/from
       * crosses a page boundry that is not contiguous
       * in physical memory.
       *
       * See fdsetup_transfer()
       *
       *  Note that the convention for polled r/w calls
       * is that they will not set up split transfers,
       * hence it is legal for us to call fdstartio and
       * then indirect through the buf struct pointer.
       */
      else if (fdp->transfer_split != 0) {
	FDDEBUG(FDINTR,printf("split request\n"));

	/* Update the offset so the address to
	 * move to can be determined correctly
	 */
	fdp->buf_offset += fd_sectors_moved(fdreq) * fdp->bytes_per_sector;

	/*  If we have more of this transfer to do,
	 * then start the i/o for it.
	 */
	if (fdsetup_transfer(fdp))
	  fdstartio(fdp);
	break;		/* don't do fdiodone()! */
      }
    }

    /*  If the RESET flag is set, that means we have just
     * completed a special RESET for an error condition
     * (probably on the 5 1/4" drive).  Flip the RESET bit
     * off and call fdiodone with the set errno.
     */
    if (fdreq->state & FD_RB_RESET) {
      fdreq->state &= ~FD_RB_RESET;
      fdiodone(bp,fdreq,fdp,fdreq->req_errno);
    }
    else
      fdiodone(bp,fdreq,fdp,FDSUCCESS);
    break;

  case ABIOS_STAGE_ON_INT:
    /*  Discrete multi stages, just continue.  Eventualy
     * we will get a done or some sort of errors.
     */
    FDDEBUG(FDINTR, printf("fdcommon_intr: ABIOS_STAGE_ON_INT\n"));
    fdreq->state |= FD_RB_STAGING;
    break;

  case ABIOS_STAGE_ON_TIME:
    /*  In this case we want to call fd_stot in fd_wait_time()
     * micro seconds.
     */
    FDDEBUG(FDINTR, printf("fdcommon_intr: ABIOS_STAGE_ON_TIME\n"));
    timeout(fd_stot,(caddr_t)NULL,us2tics(fd_wait_time(fdreq)));
    break;

  case FDABIOS_MEDIACHANGED:
    /*  If this is our first request to the drive, this
     * this is a retryable error (and the only way to
     * clear the signal).
     *
     *  However, if this is not the first request for this open
     * it is an error to have changed media.
     */
    FDDEBUG(FDINTR, printf("fdcommon_intr: FDABIOS_MEDIACHANGED\n"));

    /* to boot from floppy, have to allow media to change... */
    fdissue_command(fdp,fdreq->r_function);
    break;
    if (fdp->fd_change == FDCHANGE_NEW) {
      fdp->fd_change = FDCHANGE_INIT;
      fdissue_command(fdp,fdreq->r_function);
    }
    else {
      /* return media changed error
       */
      fdlog_error(fdp,FD_ERR_CHMEDIA);
      fdiodone(bp,fdreq,fdp,EIO);
    }
    break;

  case FDABIOS_MEDIANOTPRESENT:
    /*  This is an obvious error condition.
     */
    fdlog_error(fdp,FD_ERR_NOMEDIA);
    fdiodone(bp,fdreq,fdp,EIO);
    break;

  case FDABIOS_WRITEPROTECTED:
    /*  The diskette was write protected and we attempted to
     * do a write to it.  Return EIO (no write protected errno).
     */
    FDDEBUG(FDINTR,printf("fdcommon_intr: WRITE PROTECTED\n"));
    fdiodone(bp,fdreq,fdp,EIO);
    break;

  default:
    /*  All the errors get handled here.  Retry the error.
     * up to FDMAXRETRIES, and if all is still lost, return
     * EIO.
     */
    FDDEBUG(FDINTR, printf("fdcommon_intr: error (rc=%x)\n",
			   fdreq->r_return_code));

    if (fdp->retry_count < FDMAXRETRIES) {
      fdp->retry_count++;
      fdissue_command(fdp,fdreq->r_function);
    }
    else {
      /*  We're out of retries, so log the error.  If
       * the error is on the 5 1/4" drive reset the	
       * drive.  Note that resetting the drive will
       * call fdiodone for us.
       */
      fdlog_error(fdp,FD_ERR_RETRY);

      if (fdp->abios_flags & FD_RECALIBRATE_REQUIRED) {
	fdreq->state |= FD_RB_RESET;
	fdreq->req_errno = EIO;
	fdcall_abios(ABIOS_RESET,fdreq);
      }
      else
	fdiodone(bp,fdreq,fdp,EIO);
    }
    break;

  }

  FDDEBUG(FDINTR, printf("fdcommon_intr: done\n"));
  return(1);
}

/* NEEDS WORK - are polled interrupt abios requests still done now
 * NEEDS WORK - that we call strategy to sense the media?
 */
/*  Completes the i/o for the specified request.  Handles two different
 * cases.  One for polled requests, and one for non-polled (from strategy)
 * requests.
 *
 * NOTES: This should only be used after i/o has been attemped on the
 * buf struct in question.  Before that time just call iodone() directly.
 */
static void
fdiodone(buf,fdreq,fdp,errno)
     register struct buf *buf;
     register struct Fd_request *fdreq;
     register struct floppy *fdp;
     int errno;
{
  if ( (fdreq->state & FD_RB_SYNC) || (buf == NULL) ) {
    /* Polled request:
     *
     *  The error code is returned to the caller in req_errno.
     *
     *  We need to set ABIOS_DONE, so that polled requests
     * will wake up even if we have an error.  req_errno is
     * used to handle the error conditions.
     *
     *  Also note that any outstanding requests will be started
     * up after the request is woken up.  It would be better to
     * be able to check the queues here, and then start the
     * requests, but we have to wait for the data to get back
     * to the caller of the polled request.
     */
    FDDEBUG(FDINTR, printf("fdiodone: entering...polled\n"));
    fdreq->req_errno = errno;
    fdreq->r_return_code = ABIOS_DONE;
    wakeup(&fdreq->sleep_on_intr);
  }
  else {
    /* Strategy request:
     *
     *  This is a little more complicated than the polled
     * request.  We check for errors, and for people to 
     * wake up, play with the queue's and then look for
     * more work to do.
     *
     *  Note that we can use the buf struct in this case.
     */
    FDDEBUG(FDINTR, printf("fdiodone: entering...intr\n"));

    /*  If this request resulted in an error, then set
     * the flags in the buf struct to indicate the error.
     */
    if (errno) {
      buf->b_resid += fdp->modified_bcount;
      buf->b_flags |= B_ERROR;
      buf->b_error = errno;
    }
    else {
      /*  we have had a succesful non-polled call, so 
       * note that the diskette is in ok.
       */
      fdp->fd_change = FDCHANGE_INIT;
    }

    /*  Tally i/o counts:
     *	- read_count_bytes is the number of bytes less
     *	than a megabyte read.  read_count_megabytes is
     *	the number of megabytes read.  By using both of
     *	them together, you get an accurate #.
     *
     *	- write_count* is the same, but for write operations.
     */
    if (buf->b_flags & B_READ) {
      fdp->read_count_bytes += buf->b_bcount - buf->b_resid;
      if (fdp->read_count_bytes >= 0x100000) {
	fdp->read_count_megabytes += fdp->read_count_bytes / 0x100000;
	fdp->read_count_bytes %= 0x100000;
      }
    }
    else {		/* B_WRITE */
      fdp->write_count_bytes += buf->b_bcount - buf->b_resid;
      if (fdp->write_count_bytes >= 1000000) {
	fdp->write_count_megabytes += fdp->write_count_bytes / 1000000;
	fdp->write_count_bytes %= 1000000;
      }
    }

    /* if this is the last buffer, dq everything, else
     * get the next one...
     */
    if (fdp->headptr == fdp->tailptr) {
      fdp->headptr = fdp->tailptr = NULL;

      /*  If fd_close() has been called with outstanding
       * i/o on this drive the the close_waiting flag
       * will be set.  We have to wake up the process
       * that is waiting on the close and indicate that
       * it's i/o is done.
       *
       *  Note that we only do this when the request
       * queue is empty, and there is no more
       * outstanding i/o for this drive.
       */
      if (fdp->close_waiting == TRUE) {
	fdp->close_waiting = FALSE;
	wakeup(&fdp->close_sleep);
      }
    }
    else
      fdp->headptr = fdp->headptr->av_forw;

    FDDEBUG(FDINTR,printf("fdiodone: fdp->headptr is %x\n", fdp->headptr));

    fd_sched(fdp);		/* look for more requests */
    iodone(buf);		/* done with this buffer */
  }
}

/* Looks for more work for the diskette adapter to work on.
 */
static void
fd_sched(fdp)
     register struct floppy *fdp;
{
  register struct floppy *tfdp, *lfdp;
  int unit, pri;

  /*  Handle watchdog timers:
   *	- Make sure the interrupt loss watchdog is off.  If we
   *	  are finishing a request via an interrupt it will be
   *	  off, but if we are doing a non interrupting ABIOS call
   *	  it may not.  It is low enough overhead, so we just
   *	  do it again.
   *
   *	- Start the motor off timer.  If we start another request this
   *	  timer will be turned off.
   */
  UNTIMEOUT(fdadapter.int_wd);
  fdadapter.mot_wd.fdp = fdp;
  TIMEOUT(fdadapter.mot_wd);

  /*  If we need the controller to process an open, give
   * that presedence over other q'ed requests.  Otherwise
   * go and look for other requests to do.
   */
  pri = splfd();
  if (fdadapter.need_controller == TRUE) {
    fdadapter.fdintr_status = FDFREE;
    wakeup(&fdadapter.need_sleep);
  }
  else {
    /*  Look for another request to start.  To be fair try
     * the other drives first.
     */
    tfdp = &floppies[unit=next_unit(fdadapter.fd_req.r_unit)];
    lfdp = (struct floppy *)NULL;
    while (lfdp != fdp) {
      if (tfdp->headptr != NULL) {
	/*  Do the request, and then
	 * don't loop looking for more
	 */
	fdio(tfdp->headptr);
	break;
      }
      lfdp = tfdp;
      tfdp = &floppies[next_unit(unit)];
    }

    /*  If we didn't run anything new, then set the intr
     * status to free and wake up any polled requests that
     * want to access the adapter.
     */
    if (lfdp == fdp)
      fdadapter.fdintr_status = FDFREE;
  }
  splx(pri);
}

/*
 * This does the work of an FDIOCFORMAT ioctl call. 
 *
 * The following ioctl operations are provided for the format
 * command. 
 *
 * FDIOCFORMAT - formats a diskette track.  A track is
 * formatted using values passed in an array of bytes
 * pointed to by the 'arg' parameter.  The buffer should be
 * 4 times the number of sectors per track long.  Four bytes
 * of data are needed for each sector on the track.  The
 * following shows the structure of the data buffer:
 *			 _________________
 *			|                 |
 *			| cylinder number | - byte 0
 *		S	|_________________|
 *		E       |                 |
 *		C       |   side number   | - byte 1
 *		T       |_________________|
 *		O       |		  |
 *		R       |  sector number  | - byte 2
 *			|_________________|
 *		0       |                 |
 *			| # bytes/sector  | - byte 3
 *			|_________________| 
 *
 *				.
 *				.
 *				.
 *			 _________________
 *			|                 |
 *			| cylinder number | - byte (n * 4) - 4
 *		S	|_________________|
 *		E	|		  |
 *		C	|   side number   | - byte (n * 4) - 3
 *		T	|_________________|
 *		O	|		  |
 *		R	| sector number	  | - byte (n * 4) - 2
 *			|_________________|
 *		n	|		  |
 *			| # bytes/sector  | - byte (n * 4) - 1
 *			|_________________| 
 *
 *    where n is the number of sectors per track.  In general,
 * the cylinder number, side number, and number of bytes per
 * sector should not change from sector to sector. The sector
 * number should be different for each sector. Usually the
 * sector numbers will correspond to the physical sector
 * numbers, but they can be different if some special copy
 * protection scheme is being used. 
 */
#ifdef NEEDS_WORK		/* NEEDS WORK -- more ioctls..later */
static int
fd_iocformat(devno,arg,fdp)
     dev_t devno;
     register long arg;
     register struct floppy *fdp;
{
  int rc = FDSUCCESS;
  caddr_t paddr;
  struct Fd_request *fdreq = &fdadpter.fd_req;

  fdp->format_size = fdp->sectors_per_track * 4;

  /* allocate a page for the format info.
   */
  fdp->format_buffer = (char *)
    xmalloc(fdp->format_size, PGSHIFT, pinned_heap);
  if (fdp->format_buffer == NULL)
    return(ENOMEM);

  /* pin the buffer.
   */
  rc = pin((caddr_t) fdp->format_buffer, (int) fdp->format_size);
  if (rc != 0) {
    xmfree((caddr_t) fdp->format_buffer, (caddr_t) pinned_heap);
    return(rc);		/* return pin's errno */
  }

  /* copy the buffer in our address space.
   */
  rc = copyin((char *) (arg), (char *) (fdp->format_buffer),
	      fdp->format_size);
  if (rc == -1) {
    unpin((caddr_t) fdp->format_buffer, (int) fdp->format_size);
    xmfree((caddr_t) fdp->format_buffer, (caddr_t) pinned_heap);
    return(EINVAL);
  }

  fdp->cylinder_id = *(fdp->format_buffer);		/* byte 0 */
  fdp->head_id = *((char *)(fdp->format_buffer)+1);	/* byte 1 */

  fdget_adapter();
	
  if ((paddr = pmap_extract(kernel_pmap,fdp->format_buffer)) == 0) {
    free(fdp->format_buffer);
    return(EIO);
  }
  fd_data_ptr_1(fdreq) = (u_int) fdp->format_buffer;
  fd_data_ptr_2(fdreq) = (u_int) paddr;

  rc = fdexecute_command(fdp,FDFORMAT_TRACK_POLL);

  unpin((caddr_t) fdp->format_buffer, (int) fdp->format_size);
  xmfree((caddr_t) fdp->format_buffer, (caddr_t) pinned_heap);

  return(rc);
}
#endif

/* Does the work for a FDIOCSINFO ioctl.
 *
 *	FDIOCSINFO - sets the current diskette characteristics.
 *	A structure of type fdinfo is loaded with the desired   
 *	values and a pointer to this structure must be passed
 *	to the device driver in the 'arg' parameter. 
 *
 * NOTES: This routine is fairly hardcoded with values for the various
 * formats of the 2 drives and will have to be updated when new drives
 * or formats are added.
 */
#ifdef NEEDS_WORK		/* NEEDS WORK -- ioctl stuff...later */
static int
fd_iocsinfo(fdp,arg)
     register struct floppy *fdp;
     register long arg;
{
  register struct fdinfo *fdinfop;
  int tmp;
  int rc = FDSUCCESS;

  /* Allocate an fdinfo structure. */
  fdinfop = (struct fdinfo *) malloc(sizeof(struct fdinfo));

  if (fdinfop == NULL)
    return(ENOMEM);

  /* Call copyin to get the fdinfo structure from the user. 
   */
  tmp = copyin((char *) (arg), (char *) (fdinfop), sizeof(struct fdinfo));
  if (tmp == -1) {
    free(fdinfop);
    return(EINVAL);
  }

  /*  Make sure the request has the correct drive type.  If not there
   * must be a user error.
   */
  if (fdp->drive_type != fdinfop->type) {
    free(fdinfop);
    return(EINVAL);
  }

  switch (fdinfop->type) {
  case D_48:		/* 360K, 5.25" drive */
    switch (fdinfop->ncyls) {
    case 40:	/* 360K diskette */
      if ( (fdinfop->sides == 2) && (fdinfop->nsects == 9) )
	rc = fdload_floppy(fdp,FDLOAD_360);
      else
	rc = EINVAL;
      break;
      /*  The 1.2M diskette is not currently supported, but this
       * will be handy if one does come up.
       */
    case 80:	/* 1.2M diskette -- not supported */
      if ( (fdinfop->nsects != 15) || (fdinfop->sides != 2) )
	rc = EINVAL;
      else
	rc = EINVAL;
      break;
    default:
      rc = EINVAL;
      break;
    }
    break;

  case D_135H:	/* 1.44M, 3.5" drive */
    if ((fdinfop->sides != 2) || (fdinfop->ncyls != 80))
      rc = EINVAL;
    else {
      switch (fdinfop->nsects) {
      case 9:		/* 720K diskette */
	rc = fdload_floppy(fdp, FDLOAD_720);
	break;
      case 18:	/* 1.44M diskette */
	rc = fdload_floppy(fdp,FDLOAD_144);
	break;
      default:
	rc = EINVAL;
	break;
      }
    }
    break;
  default:
    rc = EINVAL;
    break;
  }

  if (rc == FDSUCCESS)
    fdp->drive_type = (u_char) fdinfop->type;

  free((caddr_t) fdinfop);

  return(rc);
}
#endif

/* Does a FDIOCSTATUS ioctl.
 *
 *	FDIOCSTATUS - returns the status of the diskette drive and
 *	device driver.  A pointer to a structure of type fd_status
 *	should be be passed in the 'arg' parameter.  The driver
 * 	status will be loaded into the fd_status structure
 * 	The result phase portion of the status is only
 * 	valid if the device driver was opened in raw mode. 
 * 	otherwise, there is no way of knowing to which i/o
 * 	operation the result phase corresponds. 
 *
 * NEEDS WORK - In our ABIOS implementation there are fields in the fd_status
 * struct that are not used.  Let's see about cleaning them up.
 */
#ifdef NEEDS_WORK		/* NEEDS WORK -- ioctl's */
static int
fd_iocstatus(fdp,arg)
     register struct floppy *fdp;
     long arg;
{
  register struct fd_status *fdstatusp;
  int rc = FDSUCCESS;
  int tmp;

  FDDEBUG(FDIOCTL, printf("fdioctl: FDIOCSTATUS\n"));

  /* Allocate an fd_status structure. */

  fdstatusp = (struct fd_status *) malloc(sizeof(struct fd_status));

  if (fdstatusp == NULL)
    return(ENOMEM);

  /* Start building the status1 bitmask with the drive number.
   */
  tmp = fddrive_val[fd_drive(fdp->device_number)];

  fdstatusp->status1 = (u_char) fdp->data_rate;

  /* Start building the status2 bitmask with the retry flag.
   */
  if (fdp->retry_flag)
    tmp = FDRETRY;
  else
    tmp = 0;

  if (fdp->timeout_flag == TRUE)
    tmp |= FDTIMEOUT;

  switch (fdp->drive_type) {
  case D_48:
    tmp |= FD5INCHLOW;
    break;
  case D_135H:
    tmp |= FD3INCHHIGH;
    break;
  default:
    break;
  }

  fdstatusp->status2 = (u_char) tmp;
  fdstatusp->status3 = fdp->diskette_type;

  fdstatusp->head_settle_time = fdp->head_settle_time;
  fdstatusp->motor_speed = 300;		/* On R2 1.44 its 300 */
  fdstatusp->Mbytes_read = fdp->read_count_megabytes;
  fdstatusp->Mbytes_written = fdp->write_count_megabytes;

  fdstatusp->cylinder_num = fdp->start.cylinder;
  fdstatusp->head_num = fdp->start.head;
  fdstatusp->sector_num = fdp->start.sector;
		
  fdstatusp->bytes_num = fdp->sector_count * fdp->bytes_per_sector;
  fdstatusp->bytes_num = fdp->e_bytesps;

  fdget_adapter();
  tmp = fdexecute_command(fdp,FDDISK_SAME);

  switch(tmp) {
  case DISK_CHANGED:
    fdstatusp->dsktchng = TRUE;
    break;
  case DISK_SAME:
    fdstatusp->dsktchng = FALSE;
    break;
  default:
    /*  Unknown rc from command, so return an error.
     */
    free((caddr_t) fdstatusp);
    return(EIO);
  }

  /* Call copyout to copy the fd_status structure to the user
   */
  tmp = copyout((char *) (fdstatusp), (char *) (arg),
		sizeof(struct fd_status));
  if (tmp == -1) {
    rc = EINVAL;
  }

  free((caddr_t) fdstatusp);
  return(rc);
}
#endif


/*  Cleans up initialization stuff prior to exiting. This routine is
 * called just before exiting in fd_open and fd_close. 
 */
static void 
fdcleanup(fdp)
     struct floppy *fdp;
{
  register int unit;
  int pri;

  /*  If our drive is still open, we're done otherwise turn off
   * the light.  If the other drives are also closed then reclaim
   * some system resources.
   */
  if (fdp != NULL) {
    if (fdp->drive_state & FDSTAT_OPEN)
      return;

    /*  If the adapter is free, get it, and turn off the
     * motor.  If the adapter is not free, something else
     * is going on, and hence we won't have to turn off
     * the light.
     */
    pri = splfd();
    if (fdadapter.fdintr_status == FDFREE) {
      fdget_adapter();
      splx(pri);
      fdexecute_command(fdp,FDMOTOR_OFF);
    }
    else
      splx(pri);

    for (unit=0 ; unit < FDMAXDRIVES ; unit++)
      if (floppies[unit].drive_state & FDSTAT_OPEN)
	return;
  }

  /* Reclaim some of the resources we were using.  If there is
   * activity on the diskette adapter we will not get this far.
   */
  /* NEEDS WORK -- do we really want to untimeout() on the
   * NEEDS WORK -- watchdog timers here??  This might cause
   * NEEDS WORK -- problems with they system if they arn't
   * NEEDS WORK -- on, but probably not....
   */
  UNTIMEOUT(fdadapter.mot_wd);
  UNTIMEOUT(fdadapter.int_wd);

  return;
}

/*
 *  Checks to see if the drive door is open.  To do this with
 * an ABIOS interface we read the first sector into memory.  This sector
 * is written back if we need to check for write protect.
 *
 * NOTES: We set fdp->open_check to TRUE to know when the 5 1/4" drive
 * does not have a diskette in it.  fdcall_abios_poll() will turn this
 * flag off.  See fdtimer() for further explination.
 */
#if NEEDS_WORK
static
#endif
int 
fddoor_check(fdp, wrck)
     register struct floppy *fdp;
     int wrck;
{
  struct buf *bp;
  int rc;

  FDDEBUG(FDCMD, printf("fddoor_check: entering...\n"));

  bp = geteblk(512);
#ifdef OSF
  event_clear(&bp->b_iocomplete);
#endif OSF
  if (!bp) return(ENOMEM);
  bp->b_dev = fdp->device_number;
  bp->b_bcount = 512;
  bp->b_flags = B_READ;
  bp->b_blkno = 0;
  fdstrategy(bp);
  rc = biowait(bp);

  if (rc) goto dcdone;

  if (wrck) {
    bp->b_bcount = 512;
    bp->b_error = 0;
    bp->b_resid = 0;
				/* Added to fix B_INVAL for Mach 3.0 */

#ifdef	MACH_KERNEL
    bp->b_flags = B_WRITE;
#else 	MACH_KERNEL
    bp->b_flags = B_WRITE|B_INVAL;
#endif 	MACH_KERNEL
    bp->b_blkno = 0;
#ifdef OSF
    event_clear(&bp->b_iocomplete);
#endif OSF
    fdstrategy(bp);
    rc = biowait(bp);
  }

dcdone:
  brelse(bp);
  return(rc);
}

/*  Loads the proper diskette characteristics.  The 2nd parameter is a
 * function for fdexecute_command(), which will do the actual loading.
 *
 *  There are actually 2 different ways that the fdp structure can be
 * populated.  After fdexecute_command() is done we do some sanity
 * checks.
 */
static int 
fdload_floppy(fdp,cmd)
     register struct floppy *fdp;
     int cmd;
{
  int rc = FDSUCCESS;
  u_char drive_type;

  drive_type = fdp->drive_type;

  fdget_adapter();

  if ( (rc = fdexecute_command(fdp,cmd)) != FDSUCCESS )
    return(rc);

#ifdef NEEDS_WORK		/* NEEDS WORK */
  /* NEEDS WORK -- ok.  We probably won't check drive_type anymore to
   * NEEDS WORK -- compare what it was configured for.  This routine
   * NEEDS WORK -- can probably drop to a macro like fdtype(), and we
   * NEEDS WORK -- should also make sure both fdtype and load_floppy
   * NEEDS WORK -- are really infact needed.
   */
  /* Is the drive type what it was configured for?
   */
  if (drive_type != fdp->drive_type)
    return(EINVAL);

  /* NEEDS WORK -- since drive_type is no longer configed,
   * NEEDS WORK -- this will have to change eventually.
   */
  switch(fdp->drive_type) {
  case D_135H:
    /*  If they wanted a 5 1/4 inch drive explicitly, then
     * we have an error because this is a 3 1/2 inch drive.
     */
    if (minor(fdp->device_number) & FD_5)
      rc = EINVAL;
    break;
  case D_48:
    /*  If they wanted a 3 1/2 inch drive explicitly, then
     * we have an error because this is a 5 1/4 inch drive.
     */
    if ((minor(fdp->device_number) & FD_5) == 0)
      rc = EINVAL;
    break;
  }
#endif

  return (rc);
}

/*  Currently this is a no-op.
 */
int 
fdmincnt(bp)
     struct buf *bp;
{
  return (0);
}

/*
 *  Logs a hard diskette error.  It is called when we get an error that
 * should be reported to the user by more than an errno.
 *
 *  Usually it will occur after all the retries are done, and the error
 * still exists.
 *
 */
static void 
fdlog_error(fdp, value)
     register struct floppy *fdp;
     u_char value;
{
  char *msg;

  FDDEBUG(FDSTAT, printf("fdlog_error: entering...\n"));

  /* NEEDS WORK -- this should be syslog or whatever the heck
   * NEEDS WORK -- OSF/1 uses.  Took out the errsave() stuff.
   */

  /* Differentiate between some hardware and internal errors.
   */
  switch(fdadapter.fd_req.r_return_code) {
  case 0x8000:		/* Device Busy, Operation Fefused */
  case 0x800f:		/* Invalid Value in NVRAM */
  case 0x9009:		/* Controller Failure on Reset */
  case 0x9108:		/* DMA Overrun on Operation */
  case 0x9120:		/* Controller Failure */
  case 0x9140:		/* Seek Operation Failed */
  case 0x9180:		/* General Error */
  case 0xa120:		/* Controller Failure */
  case 0xb020:		/* Controller Failure */
    msg = "hardware error";
    break;
  case 0xc000:		/* Invalid Logical ID */
  case 0xc001:		/* Invalid Function */
  case 0xc003:		/* Invalid Unit Number */
  case 0xc004:		/* Invalid Request Block Length */
  case 0xc005:		/* Invalid Diskette Parameter */
    msg = "internal driver error";
    break;
  default:
    msg = fd_tmp_errors[value];
    break;
  }
  printf("***fd%d: %s (%x)!***\n",fdadapter.fd_req.r_unit, msg,
	 fdadapter.fd_req.r_return_code);
}

/*  Gains access of the diskette adapter for the caller, sleeping if
 * necessary.  This can be called from interrupt level, because FDBUSY
 * is set in this case.
 */
static void
fdget_adapter()
{
  register int pri;

  pri = splfd();

  while (fdadapter.fdintr_status != FDFREE) {
    FDDEBUG(FDCMD,printf("fdget_adapter: adapter not free!\n"));

    fdadapter.need_controller = TRUE;
    sleep(&fdadapter.need_sleep,PRIBIO);	/* NEEDS WORK */
  }

  fdadapter.fdintr_status = FDBUSY;
  fdadapter.need_controller = FALSE;

  splx(pri);
}

/* Checks for non-contiguous physical pages.  We can't dma across a virtual
 * page boundry that are not physically contiguous.
 */
static int
d_max_page(bp,baddr,paddr,max)
     struct buf *bp;
     u_int baddr;
     u_int *paddr;
     unsigned long max;
{
  unsigned long count = ((unsigned long) baddr) & (I386_PGBYTES-1);
  unsigned long old_addr;
  unsigned long new_addr;

  /* if count is aligned, we can transfer a whole page */
  if (count==0) count = I386_PGBYTES;

  if ( (*paddr = pmap_extract(get_pmap(bp),baddr)) == 0)
    return(0);

  /*  Now loop through and get the real address of each of the
   ** following pages.  Break on the first non-contiguous page,
   ** or when we exceed count.
   */
  baddr += I386_PGBYTES;
  old_addr = *paddr + I386_PGBYTES;
  while ( (count < max) &&
	 ((new_addr = pmap_extract(get_pmap(bp),baddr)) != 0) &&
	 (new_addr == old_addr) ) {
    count += I386_PGBYTES;
    baddr += I386_PGBYTES;
    old_addr += I386_PGBYTES;
  }
  return(count > max ? max : count);
}

fddump(){}			/* NEEDS WORK */
fdsize(){}			/* NEEDS WORK */

struct fd_errors { 
int	code;
char	*msg;
} fd_err_codes [] = {
	{0x00,	"timeout"},
	{0x01,	"bad-command"},
	{0x02,	"address-mark-not-found"},
	{0x04,	"record-not-found"},
	{0x05,	"reset-failed"},
	{0x07,	"activity-failed"},
	{0x0a,	"defective-sector"},
	{0x0b,	"bad-track"},
	{0x0d,	"invalid-sector"},
	{0x0e,	"CAM-detected"},
	{0x0f,	"DMA-arb-level-bad"},
	{0x10,	"bad-ecc-error"},
	{0x11,	"ecc-corrected"},
	{0x20,	"bad-controller"},
	{0x21,	"equipment-check"},
	{0x40,	"bad-seek"},
	{0x80,	"device-didn't-respond"},
	{0xaa,	"drive-not-ready"},
	{0xbb,	"undefined-error"},
	{0xcc,	"write-fault"},
	{0xff,	"incomplete-sense"},
	{0xc000,"invalid LID"},
	{0xc001,"invalid function"},
	{0xc003,"invalid unit number"},
	{0xc004,"invalid request block length"},
	{0xc005,"invalid parameter"}

};

fd_error_decode(code,msg)
char *msg;
{
	int i;
	int n = code&0x00ff;

	if ((code&0x8000) == 0) {
		char *p;
		switch(code)
		{
		case 0:
			p = "completed ok";
			break;
		case 1:
			p = "stage on int";
			break;
		case 2:
			p = "stage on time";
			break;
		case 5:
			p = "not my int";
			break;
		default:
			printf("unknown[0x%x]%s", code, msg);
			return;
		}
		printf("%s%s", p, msg);
		return;
	}
	if (code&0x4000) {
		printf("PARAMETER ");
		n = code;
	}
	if (code&0x2000)
		printf("TIME-OUT ");
	if (code&0x1000)
		printf("DEVICE ");
	if (code&0x100)
		printf("RETRYABLE ");
	printf("ERROR ");
	for (i=0; i<(sizeof fd_err_codes)/(sizeof fd_err_codes[0]); ++i)
		if (fd_err_codes[i].code == n)
			{
			printf("%s%s", fd_err_codes[i].msg, msg);
			return;
			}
	printf("unknown[0x%x]%s", code, msg);
}


#ifdef	MACH_KERNEL



/*
 * NAME: fdgetstat(dev, flavour, data, count) 
 *                                                                    
 * FUNCTION: This function implements one half of the functions implemented
 *	     by the IOCTL routine in the MACH 2.5 version of this floppy
 *	     driver. 
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *                                                                   
 * RETURNS: 
 */  



io_return_t fdgetstat(dev, flavor, data, count)
	dev_t		dev;		/* device */
	int		flavor;		/* command */
	int *		data;		/* pointer to OUT array */
	unsigned int	*count;		/* OUT */
{

	register struct floppy *fdp;
	register struct fdparms *fdparmsp;
	io_return_t	errcode = D_SUCCESS;

	fdp = &floppies[fd_drive(dev)];

	switch(flavor){

	case	FDIOCGETPARMS:

			/* Check to see if out buffer is large enough */
	
		if (*count < sizeof(struct fdparms)/sizeof(int))
			return(D_INVALID_OPERATION);	

		/* In mach 2.5 space for fdparmsp was allocated here */
		/* initialized, then a copyout would be done to the */
		/* user supplied buffer space. and then a copyout   */
		/* would be done to user space. But here the pointer */
		/* to data is in kernel space and so it need not be */
		/* done. */


		fdparmsp = (struct fdparms *)data;

		fdparmsp->diskette_type 	= fdp->diskette_type;
		fdparmsp->sector_size		= fdp->e_bytesps;
		fdparmsp->sectors_per_track	= fdp->sectors_per_track; 
		fdparmsp->sectors_per_cylinder	= fdp->sectors_per_cylinder;
		fdparmsp->tracks_per_cylinder	= fdp->tracks_per_cylinder;
		fdparmsp->cylinders_per_disk	= fdp->cylinders_per_disk;
		fdparmsp->data_rate		= fdp->data_rate;
		fdparmsp->head_settle_time	= fdp->head_settle_time;
		fdparmsp->head_load		= 0;
		fdparmsp->fill_byte		= fdp->fill_byte;
		fdparmsp->step_rate		= 0;
		fdparmsp->step_rate_time	= 0;
		fdparmsp->gap			= fdp->gap;
		fdparmsp->format_gap		= fdp->format_gap;
		fdparmsp->data_length		= fdp->data_length;
		fdparmsp->motor_off_time	= fdp->motor_off_time;	
		fdparmsp->bytes_per_sector	= fdp->bytes_per_sector;
		fdparmsp->number_of_blocks	= fdp->number_of_blocks;

		/* force the out buffer to be word alligned */

		*count = sizeof(struct fdparms)/sizeof(int);

		errcode = D_SUCCESS;
		break;

	default:

		errcode = D_INVALID_OPERATION;
	
	}  /* End of case flavor statement */

	return(errcode);

}



/*
 * NAME: fdsetstat(dev, flavour, data, count) 
 *                                                                    
 * FUNCTION: This function implements a set of the functions that were
 *	     being performed in the IOCT routine of the MACH 2.5 version 
 *	     of this floppy driver 
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *                                                                   
 * RETURNS: 
 */  


io_return_t fdsetstat(dev, flavor, data, count)
	dev_t		dev;
	int		flavor;
	int *		data;
	unsigned int	count;
{

	register struct floppy *fdp;
	register struct devinfo *devinfop;
	register struct fdinfo *fdinfop;
	register struct fdparms *fdparmsp;
	int	temp;
	io_return_t	errcode = D_SUCCESS;


	fdp = &floppies[fd_drive(dev)];

#define check(cnd, err)		if (!(cnd)) {		\
					errcode = (err);\
					break;		\
				}

#define cond(member)		(fdp->member == fdparmsp->member)	



	switch(flavor){
	
	case	FDIOCSETPARMS:

		/* define two macros that will be used to screen */
		/* requests to set protected parameters of the */
		/* floppy drive that ABIOS doesnt support changes */
		/* to by the user */

/*

#define check(cnd, err)		if (!(cnd)) {			\
					errcode = (err);	\ 
					break;			\
				}

#define cond(member)		(fdp->member == fdparmsp->member)

*/

		/* base the the fdparmsp on the user supplied data */

		fdparmsp = (struct fdparms *)data;
		
		if (fdparmsp == NULL)
		{
			errcode = ENOMEM;
			break;
		}

		/* Check to see if ABIOS protected fields are being */
		/* requested to be changed, if so retuen Error */


		check(fdp->diskette_type == fdparmsp->diskette_type,
			D_INVALID_OPERATION);
		check(fdparmsp->head_load == 0, D_INVALID_OPERATION);
		check(fdparmsp->step_rate == 0, D_INVALID_OPERATION);
		check(fdparmsp->step_rate_time == 0, D_INVALID_OPERATION);
		check(cond(sectors_per_track), D_INVALID_OPERATION);
		check(cond(sectors_per_cylinder), D_INVALID_OPERATION);
		check(cond(tracks_per_cylinder), D_INVALID_OPERATION);
		check(cond(cylinders_per_disk), D_INVALID_OPERATION);
		check(cond(data_rate), D_INVALID_OPERATION);
		check(cond(head_settle_time), D_INVALID_OPERATION);
		check(cond(number_of_blocks), D_INVALID_OPERATION);
		check(cond(bytes_per_sector), D_INVALID_OPERATION);

		/* If we are here it means that the request is a legal */
		/* ABIOS supported request. So we check to see which of */
		/* the legal parameters are being requested to be changed */
		/* save the original value, attempt a change, if the ABIOS */
		/* request fails then we reset the value to the original */
		/* legal value and return a FAILURE */


	/* Check to see if bytes_per_sector changed */


		if (fdp->e_bytesps != fdparmsp->sector_size)
		{

			/* 256 and 512 Bytes per sector are legal values */

			check((fdparmsp->sector_size==FD_256_BYTE_PER_SECTOR) ||
			       (fdparmsp->sector_size==FD_512_BYTE_PER_SECTOR),
				D_INVALID_OPERATION);

			temp = fdp->e_bytesps;  /* save original */

			fdp->e_bytesps = fdparmsp->sector_size;
	
			fdget_adapter();

			if (fdexecute_command(fdp,FD_SETDP_POLL) != FDSUCCESS)
			{

				fdp->e_bytesps = temp;
				errcode = D_INVALID_OPERATION;
				break;

			}

		/* When changing sector_size, bytes_per_sector will be */
		/* implicitly changed, so update the users view of */
		/* bytes_per_sector accordingly */

			fdparmsp->bytes_per_sector = fdp->bytes_per_sector;


			check(temp == 0, D_INVALID_OPERATION);  /* Magic */

		} 

	/* Check to see if gap has changed */

		if (fdp->gap != fdparmsp->gap)
		{
			temp = fdp->gap;	/* save original value */
			fdp->gap = fdparmsp->gap;
			fdget_adapter();
			if (fdexecute_command(fdp,FD_SETDP_POLL) != FDSUCCESS)
			{
				fdp->gap = temp;
				errcode = D_INVALID_OPERATION;
				break;

			}
		}


	/* Check to see if data_length has changed */

		
		if (fdp->data_length != fdparmsp->data_length)
		{
			temp = fdp->data_length;	/* save original */
			fdp->data_length = fdparmsp->data_length;
			fdget_adapter();
			if (fdexecute_command(fdp,FD_SETDP_POLL) != FDSUCCESS)
			{
				fdp->data_length = temp;
				errcode = D_INVALID_OPERATION;
				break;
			}
		}

	/* Check to see fill_byte has changed */

		
		if (fdp->fill_byte != fdparmsp->fill_byte)
		{
			temp = fdp->fill_byte;		/* save original */
			fdp->fill_byte = fdparmsp->fill_byte;
			fdget_adapter();
			if (fdexecute_command(fdp,FD_SETDP_POLL) != FDSUCCESS)
			{
				fdp->fill_byte = temp;
				errcode = D_INVALID_OPERATION;
				break;
			}
		}


	/* Check to see if format_gap has changed */

		if (fdp->format_gap != fdparmsp->format_gap)
		{
			temp = fdp->format_gap;		/* save original */
			fdp->format_gap = fdparmsp->format_gap;
			fdget_adapter();
			if (fdexecute_command(fdp,FD_SETDP_POLL) != FDSUCCESS)
			{
				fdp->format_gap = temp;
				errcode = D_INVALID_OPERATION;
				break;
			}
		}

	/* Check if motor_off_time has changed */

		if (fdp->motor_off_time != fdparmsp->motor_off_time)
		{
			fdadapter.mot_wd.time = fdparmsp->motor_off_time;
			fdp->motor_off_time = fdparmsp->motor_off_time;
			break;
		}


	case FDIOCFORMAT:

	default:

		errcode = D_INVALID_OPERATION;
		break;
			

	} /* End of switch statement for flavor */


	return(errcode);
}

/*
 * NAME: fdabios_init 
 *                                                                    
 * FUNCTION: This function initializes the floppy driver data structures
 *	     by calling ABIOS to querry for the parameters of the floppy
 *	     drive and controller in the system. 
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *                                                                   
 * This is called by fdinit() at system initialization time.                     *                                             
 * RETURNS: NONE
 */  

fdabios_init(fdp)
register struct floppy *fdp;
{

	register struct Fd_request *fdreq;		
	u_char	drive_number;
	int rc = FDSUCCESS;

	fdadapter.fdintr_status = FDBUSY; 

	fdp->timeout_flag = FALSE;
	UNTIMEOUT(fdadapter.mot_wd);

	fdreq = &fdadapter.fd_req;
	drive_number = fd_drive(fdp->device_number);
	fdadapter.fd_flag = 0;
	fdreq->state = FD_RB_IDLE; 	/* may not be needed */
	fdreq->sleep_on_intr = 0;      /* may not be needed */

	FD_SET_RESERVED_ABIOS_LOGICAL_PARAMETER(fdreq);

	fdreq->r_current_req_blck_len = sizeof(struct Fd_request);
	fdreq->r_logical_id = abios_next_LID(FD_ID, 2);
	fdreq->r_unit = 0;
	fdreq->request_header.Request_Block_Flags = 0;
	fdreq->request_header.ELA_Offset = 0;
	fd_no_retry(fdp);


	fdp->timeout_flag = FALSE;
	fdreq->state |= FD_RB_SYNC;
	fdreq->req_errno = 0;


	fdreq->r_function = FDABIOS_INIT;
	fdreq->r_return_code = ABIOS_UNDEFINED;
	fdreq->state |= FD_RB_STARTED;

	abios_common_start(fdreq, fdadapter.fd_flag);

	/* wait for ABIOS to get done */

	while (fdreq->r_return_code == ABIOS_UNDEFINED)
	{
		assert_wait(0, FALSE);
		thread_set_timeout(10);
		thread_block((continuation_t) 0);
	}

	/* Look for synchronous errors from ABIOS */

	if (fdreq->r_return_code & 0x8000)
	{
		if (fdreq->r_return_code == 0xc000)
		   printf("Req 1: Invalid Logical ID\n\n");
		if (fdreq->r_return_code == 0xc001)
		   printf("Req 1: Invalid Function\n\n");
		if (fdreq->r_return_code == 0xc003)
		   printf("Req 1: Invalid Unit Number\n\n");
		if (fdreq->r_return_code == 0xc004)
		   printf("Req 1: Invalid Request Block Length\n\n");
		else
		   printf("Req 1: Unkown Error\n\n");

		return(EIO);
	}

	while (fdreq->r_return_code != ABIOS_DONE)
	{
		assert_wait(0, FALSE);
		thread_set_timeout(10);
		thread_block((continuation_t) 0);
	}

	rc = fdreq->req_errno;

	if (rc != FDSUCCESS)
		return(rc);

	fdreq->r_current_req_blck_len = fdreq->r_request_block_length;
	fdadapter.fd_flag = fdreq->r_logical_id_flags;
	fdadapter.fd_intl = fdreq->r_hardware_intr;
	fdadapter.fd_numd = fdreq->r_number_units;


	if (fdadapter.fd_numd == 0)
#ifdef  MACH_KERNEL
	        return D_NO_SUCH_DEVICE;
#else   /* MACH_KERNEL */
		return(ENODEV);
#endif  /* MACH_KERNEL */

	if (fdadapter.fd_numd > FDMAXDRIVES)
		fdadapter.fd_numd = FDMAXDRIVES;


	FD_SET_RESERVED_ABIOS_READ_PARAMETER(fdreq);

	fd_no_retry(fdp);
	
	fdp->timeout_flag = FALSE;
	fdreq->state |= FD_RB_SYNC;
	fdreq->req_errno = 0;

	fdreq->r_function = ABIOS_READ_PARAMETER;
	fdreq->r_return_code = ABIOS_UNDEFINED;
	fdreq->state |= FD_RB_STARTED;


	abios_common_start(fdreq, fdadapter.fd_flag);

	/* wait for ABIOS to get done */

	while(fdreq->r_return_code == ABIOS_UNDEFINED)
	{
		assert_wait(0, FALSE);
		thread_set_timeout(10);
		thread_block((continuation_t) 0);
	}


	/* Look for synchronous errors from ABIOS */

	if (fdreq->r_return_code & 0x8000)
	{

		if (fdreq->r_return_code == 0x8000)
		   printf("Req 2: Device Busy Error\n\n");
		if (fdreq->r_return_code == 0x8001)
		   printf("Req 2: Diskette Not Started\n\n");
		if (fdreq->r_return_code == 0xc000)
		   printf("Req 2: Invalid Logical ID\n\n");
		if (fdreq->r_return_code == 0xc001)
		   printf("Req 2: Invalid Function\n\n");
		if (fdreq->r_return_code == 0xc003)
		   printf("Req 2: Invalid Unit Number\n\n");
		if (fdreq->r_return_code == 0xc004)
		   printf("Req 2: Invalid Request Block Length\n\n");
		else
	       	   printf("Req 2: Sync Error reported by ABIOS \n\n");

		return(EIO);
	}

	while (fdreq->r_return_code != ABIOS_DONE)
	{
		assert_wait(0, FALSE);
		thread_set_timeout(10);
		thread_block((continuation_t)0);
	}


	rc = fdreq->req_errno;

	if (rc != FDSUCCESS)
		return(rc);

	fdadapter.motor_off_delay = fd_motor_off_delay_time(fdreq);

	fdadapter.fdintr_status = FDFREE;

	return(rc);

}

#endif	MACH_KERNEL


	
