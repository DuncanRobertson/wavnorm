#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <math.h>
#include <limits.h>
#include <sys/time.h>



#include "wavfuncs.h"

/*
 * a simple progress meter 

 some of this code has been taken from the progress bar meter from the linux
 e2fsck tools:

 * unix.c - The unix-specific code for e2fsck
 *
 * Copyright (C) 1993, 1994, 1995, 1996, 1997 Theodore Ts'o.
 *

 */


int progress_last_time_p = 0;
int progress_last_time   = 0;

void progressbar_peak(short *start,short *current,short *audioend,short max)
{
   int     tick;
   struct timeval  tv;

   gettimeofday(&tv, NULL);
   tick = (tv.tv_sec << 3) + (tv.tv_usec / (1000000 / 8));
   if ((tick == progress_last_time_p) &&
      (current != audioend) && (current != start))
      return;
   progress_last_time_p = tick;

   printf("\rdone: \t %2.2f%%\tpeak: \t%0.3f",
       (double )(current - start) / (double )(audioend - start) * 100,
       (double )max / SHRT_MAX);
   fflush(stdout);
}

/** as this one is going backwards through the wav it works a bit
    differently **/

void progressbar(short *start,short *current,short *audioend)
{
   int     tick;
   struct timeval  tv;

   gettimeofday(&tv, NULL);
   tick = (tv.tv_sec << 3) + (tv.tv_usec / (1000000 / 8));
   if ((tick == progress_last_time) &&
      (current != audioend) && (current != start))
      return;
   progress_last_time = tick;

   printf("\rdone: \t %2.2f%%",
       (double )(audioend -  current) / (double )(audioend - start) * 100);
   fflush(stdout);
}
