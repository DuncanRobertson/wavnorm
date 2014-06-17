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

#include "wavfuncs.h"


/**

$Id: wavfuncs.c,v 1.2 1999/07/28 12:49:48 duncan Exp $

duncan@zog.net.au 19/7/1999


**/



/**

uses info from 
http://www-ccrma.stanford.edu/CCRMA/Courses/422/projects/WaveFormat/

returns a pointer to the start of the audio data, or NULL if the
wav header isnt what we want.

**/

char *validate_wav_header(char *header, int verbose)
{
   WAVE_HEADER *wheader = (WAVE_HEADER *)header;

   /* first 4 bytes should be RIFF */

   if (strncmp(wheader->RiffID,"RIFF",4) != 0)
   {
      if (verbose)
         printf(".wav header dont have RIFF\n");
      return(NULL);
   }

   /* if (verbose) printf("RiffSize is %ld\n",wheader->RiffSize); */

   /* next 4 bytes should be WAVE */
   if (strncmp(wheader->WaveID,"WAVE",4) != 0)
   {
      if (verbose) printf("header dont have WAVE\n");
      return(NULL);
   }          

   /* next 4 bytes should be "fmt " */
   if (strncmp(wheader->FmtID,"fmt ",4) != 0)
   {
      if (verbose)
         printf("header dont have fmt\n");
      return(NULL);
   }           

   if (wheader->FmtSize != 16)
   {
      if (verbose)
         printf("this program only deals with 16 bit audio data\n");
      return(NULL);
   }

   if (wheader->wFormatTag != 1)
   {
      if (verbose)
         printf("This wav is not PCM, we cant deal with it\n");
      return(NULL);
   }

   if (wheader->nChannels != 2)
   {
      if (verbose)
          printf("Presently we only deal with two channels, this has %d\n",
             wheader->nChannels);
      return(NULL);
   }        
   if (wheader->nSamplesPerSec != 44100)
   {
      if (verbose) 
         printf("Presently we only deal with 44100 hz, this has %ld\n",
            wheader->nSamplesPerSec);
      return(NULL);
   }  

   /* next 4 bytes should be "data" */
   if (strncmp(wheader->DataID,"data",4) != 0)
   {
      if (verbose) printf("header dont have data\n");
      return(NULL);
   }           

   return (header + sizeof(WAVE_HEADER));
}

short get_peak_value(short *start, short *end)
{
   short the_max = 0;

   short *current = start;

   while (current < end)
   {
      if (abs(*current) > the_max)
         the_max = abs(*current);
      current++;
   }
   return the_max;
}

void create_wav_header(WAVE_HEADER *wheader,
   int nChannels,int nSamplesPerSec,int wBitsPerSample)
{
   /* first 4 bytes should be RIFF */

   strncpy(wheader->RiffID,"RIFF",4);

   /* next 4 bytes should be WAVE */
   strncpy(wheader->WaveID,"WAVE",4);

   /* next 4 bytes should be "fmt " */
   strncpy(wheader->FmtID,"fmt ",4);

   wheader->FmtSize = 16;

   wheader->wFormatTag = 1;

   wheader->nChannels = nChannels;
   wheader->nSamplesPerSec = nSamplesPerSec;
   wheader->nAvgBytesPerSec = nSamplesPerSec * nChannels * wBitsPerSample/8;
   wheader->wBitsPerSample = wBitsPerSample;
   wheader->nBlockAlign = nChannels * wBitsPerSample/8;

   strncpy(wheader->DataID,"data",4);
}
