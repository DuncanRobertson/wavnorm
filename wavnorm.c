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

#include "wavfuncs.h"

/**

wavnorm - a 16 bit stereo .wav file normaliser

duncan@zog.net.au 19/7/1999

$Id: wavnorm.c,v 1.5 1999/07/28 13:18:06 duncan Exp $

**/



/** Needed to compile on RH 4.X for some reason **/

#ifndef MAP_FAILED
#define MAP_FAILED -1
#endif

int wavfd;

/** command line options - stored as globals **/
int verbose = 0;             /* -v */ 
int histogram = 0;           /* -h */
int destructive = 0;         /* -d */
double threshold = -1.0;     /* -t */
double ratio = 1.0;          /* -r */
double scale = 0.0;          /* -s */
long overs_allowed = 0;      /* -o */

/** a big ugly array to store the map of peaks **/
long levels[SHRT_MAX];

void usage(char *progname);
void normalise_wav_file(char *filename);

int main(int argc,char *argv[])
{
   int c;

   while ((c = getopt(argc, argv, "vhdt:r:o:s:")) != EOF) 
   {
      switch(c) 
      {
         case 'v':
            verbose = 1;
            printf("%s version %s duncan@zog.net.au 1999\n",argv[0],VERSION);
            break;
         case 'h':
            /* i know this isnt really a histogram, but it gives that
               kind of info... */
            histogram = 1;
         case 'd':
            destructive = 1;
            break;
         case 't':
            threshold = atof(optarg);
            if (verbose)
               printf("Using threshold of ratio of %f\n",threshold);
            break;
         case 'r':
            ratio = atof(optarg);
            break;
         case 'o':
            overs_allowed = atol(optarg);
            break;
         case 's':
            scale = atof(optarg);
            break;

         default:
            usage(argv[0]);
            exit(1);
         }
   }

   if ((optind + 1) > argc)
   {
      usage(argv[0]);
      exit(1);
   }

   for (c = optind; c < argc; c++)
   {
      normalise_wav_file(argv[c]);
   }

   return(0);
}


void normalise_wav_file(char *filename)
{      
   char *inwavbuf;
   short *current;
   short *audio;
   short *audioend;
   WAVE_HEADER *wav_info;
   int wavfd;
   struct stat input_fstat;
   size_t increment;
   int loudest = 0;
   double loudestd;
   double loudr;
   int i;

   printf("processing %s\n",filename);

   wavfd = open(filename,O_RDWR,0600);
   if (wavfd == -1)
   {
      printf("Error: open() %s\n",strerror(errno));
      return;
   }

   if (fstat(wavfd,&input_fstat) != 0)
   {
      printf("Error: fstat() %s\n",strerror(errno));
      if (close(wavfd) != 0)
      {
         printf("Error: close() %s\n",strerror(errno));
         exit(1);
      }
      return;
   }

   if (input_fstat.st_size < sizeof(WAVE_HEADER))
   {
      printf("Error: %s not large enough to hold a .wav file header!\n",
         filename);
      return;
   }

   inwavbuf = mmap(NULL,input_fstat.st_size,
      PROT_READ | PROT_WRITE,MAP_SHARED,wavfd,0);
   if (inwavbuf == MAP_FAILED)
   {
      printf("Error: mmap() %s\n",strerror(errno));
      return;
   }

   audio = (short *)validate_wav_header(inwavbuf,verbose);

   if (audio == NULL)
   {
      printf("Error: %s not in suitable .wav format for this program\n",
         filename);
      return;
   }

   wav_info = (WAVE_HEADER *)inwavbuf;

   increment = wav_info->wBitsPerSample / 8;

   if (wav_info->nDataBytes != input_fstat.st_size - sizeof(WAVE_HEADER))
   {
      printf("   Warning: file length does not match length in header.\n");

      if (destructive)
      {
         printf("   --> fixing length to correct value\n");
         wav_info->nDataBytes = input_fstat.st_size - sizeof(WAVE_HEADER);
      }
   }

   audioend = (short *)((char *)audio + wav_info->nDataBytes);

   if (scale == 0.0)
   {
      current = audio;

      if (verbose)
         printf("   scanning levels in wav file:\n");

      memset(levels,0,sizeof(levels));

      /** research phase - run through the .wav and gather some info **/

      while (current < audioend)
      {
         levels[abs(*current)]++;
         if ((verbose) && ! ((long )current % 21))
            progressbar_peak(audio,current,audioend,loudest);
         if (abs(*current) > loudest)
         {
            loudest = abs(*current);
         }
         current++;
      }
      if (verbose)
      {
         progressbar_peak(audio,current,audioend,loudest);
         printf("\n");
      }
      current--;

      /** development phase - use our info to figure out what we're gonna do
       *  then put out some info as well
       */

      if (1) /* but we might not always do this bit? */
      {
         long total = 0;
         long last  = 0;
         int increment = 1;
         int nextstep = 1;

         if (histogram)
            printf("level\t\t%%of points\t\t# of points\n");

         for (i = SHRT_MAX; i > 0; i--)
         {
            if (levels[i] > 0)
            {
               last = total;
	       total = levels[i] + total;
               if ((overs_allowed <= total) && (overs_allowed > last))
               {
                  loudest = i;
                  if (histogram)
                     printf("%3.4f\t\t%3.4f\t\t\t%6li <-- found %ld overs\n",
                        (double )i/(double )SHRT_MAX,
                        (double )total / (double )(audioend - audio) * 100,
                        total,
                        overs_allowed);
               }
            }
            if (total >= nextstep)
            {
               if (histogram)
                  printf("%3.4f\t\t%3.4f\t\t\t%6li\n",
                     (double )i/(double )SHRT_MAX,
                     (double )total / (double )(audioend - audio) * 100,
                     total);
               nextstep += (increment * 3);
               if (total >= increment * 10)
               {
                  nextstep = total;
                  increment = increment * 10;
               }
            }
         }
      }
      loudestd = fabs((double )loudest);
      loudr =  SHRT_MAX / loudestd * ratio;
      if (verbose)
         printf("    with %ld digital overs allowed\n",overs_allowed);
   }
   else
   {
      current = audioend;
      threshold = 0;
      loudr = scale;
   }

   if (verbose)
   {
      printf("    scaling to ratio %f of max\n",ratio);
      printf("    normalise ratio is %f\n",loudr);
   }

   /** production - lets mess with the file! **/

   if (destructive)
   {
      if (loudr > threshold)
      {
         if (verbose)
            printf("    normalising with ratio %f now...\n",loudr);

         /** we go backwards through the audio now, as presumably the
             last bytes we read are more likely to be buffered, so
             hopefully this is a bit faster? **/

  
         while (current > audio)
         {
            if ((verbose) && ! ((long )current % 21))
            {
               progressbar(audio,current,audioend);
            }

            *current = (int )((double )*current * loudr);
            current--;
         }
         if (verbose)
         {
            printf("\rdone: \t %2.2f%%     ",100.000);
            printf("\n");
         }
      }
      else
      {
         if (verbose)
            printf("   threshold not hit, file not altered\n");
      }
   }

   munmap(inwavbuf,input_fstat.st_size);

   if (close(wavfd) != 0)
   {
      printf("Error: close() %s\n",strerror(errno));
      exit(1);
   }
      
}

void usage(char *progname)
{
   printf("%s: version %s\n",progname,VERSION);
   printf("%s <options> file.wav [file2.wav ...]\n",
      progname);
   printf("   where <options> are one or more of:\n");
   printf("   -v verbose, and version info\n");
   printf("   -d destructive (actually alter the .wav!)\n");
   printf("   -t <threshold> threshhold ratio under which dont alter\n");
   printf("   -r <ratio>     ratio of max possible to scale (default 1.0)\n");
   printf("   -s <scale>     force scaling to this ratio, dont look for a max\n");
   printf("   -o <overs>     allow <overs> number of peaks to exceed max when looking for highest\n");
   printf("   -h             print histogram type diagram of where the highest points are in the file\n");

}
