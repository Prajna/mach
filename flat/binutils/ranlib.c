/* 
 * Mach Operating System
 * Copyright (c) 1993 Carnegie Mellon University
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
 * $Log:	ranlib.c,v $
 * Revision 2.1.1.1  94/06/01  10:19:52  rvb
 * 	From BSDSS
 * 
 * Revision 2.2  93/05/11  11:42:39  rvb
 * 		Gnu code originally used by Mach I386 release -> BSDSS
 * 	[93/05/05  15:21:41  rvb]
 * 
 *
 */
/* Dummy ranlib program for GNU.  All it does is
   `ar rs LIBRARY' for each library specified.  */

#include <ar.h>
#include <sys/time.h>
#include <sys/file.h>
#include <stdio.h>

void touch_symdefs ();

/* The makefile generates a -D switch to define AR_PROG
   as the location of the GNU AR program.  */

char *prog = AR_PROG;

main (argc, argv)
     int argc;
     char **argv;
{
  int i;
  int touch = 0;

  if (argv[1][0] == '-' && argv[1][1] == 't' && argv[1][2] == '\0')
    touch_symdefs (argc - 2, argv + 2);
  else
    for (i = 1; i < argc; i++)
      {
	int pid = fork ();
	if (pid == 0)
	  {
	    execl (prog, prog, "rs", argv[i], 0);
	    perror (prog);
	    exit (1);
	  }
	wait (0);
      }
  exit (0);
}

/* Take a list of archive files to "touch".  This subroutine will then
   find the first symdef member in them and update the date to the
   current one.  */

void
touch_symdefs (largc, largv)
     int largc;
     char **largv;
{
  struct timeval tv;
  struct timezone tz;
  struct ar_hdr archive_header;
  int i, rr;

  gettimeofday (&tv, &tz);

  while (largc--)
    {
      int fd = open (*largv, O_RDWR);

      if (fd < 0)
	{
	  fprintf (stderr, "Couldn't open \"%s\" read/write", *largv);
	  perror ("");
	  largv++;
	  continue;
	}

      lseek (fd, SARMAG, L_SET);

      rr = read (fd, &archive_header, sizeof (archive_header));

      /* In the general case this loop would be sped up by buffering,
         but in almost all cases the symdef will be the first member, so
	 I'm not going to bother.  */
      
      while (rr == sizeof (archive_header))
	{
	  if (!strncmp ("__.SYMDEF", archive_header.ar_name,
			sizeof ("__.SYMDEF") - 1))
	    {
	      bzero ((char *) archive_header.ar_date,
		     sizeof (archive_header.ar_date));

	      sprintf (archive_header.ar_date, "%d", tv.tv_sec);
	      
	      for (i = 0; i < sizeof archive_header.ar_date; i++)
		if (archive_header.ar_date[i] == '\0')
		  archive_header.ar_date[i] = ' ';

	      lseek (fd, - rr, L_INCR);
	      write (fd, &archive_header, sizeof (archive_header));
	      close (fd);
	      break;
	    }

	  lseek (fd, atoi (archive_header.ar_size), L_INCR);
	  rr = read (fd, &archive_header, sizeof (archive_header));
	}

      if (rr != sizeof (archive_header))
	/* We reached the end of the file.  */
	fprintf (stderr, "Couldn't find \"__.SYMDEF\" member in %s.\n",
		 *largv);

      largv++;
    }
}
