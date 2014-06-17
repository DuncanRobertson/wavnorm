#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <linux/soundcard.h>
#include <newt.h>
#include <limits.h>

#include "wavfuncs.h"

/****

nrecord - simple wav recorder

duncan@zog.net.au

$Id: nrecord.c,v 1.3 1999/07/28 13:18:06 duncan Exp $

****/



/** this is for recording to CD so this is the only setting I want **/
#define SAMPLES_PER_SECOND 44100
#define BITS_PER_SAMPLE    16
#define NUMBER_OF_CHANNELS 2

#define DEFAULT_DSP_DEVNAME "/dev/dsp"

/** Needed to compile on RH 4.X for some reason **/

#ifndef MAP_FAILED
#define MAP_FAILED -1
#endif

double interval_s = 1.0;  /* interval in seconds */
char *dsp_devname = DEFAULT_DSP_DEVNAME;

int open_dsp_read(char *filename,size_t *blk_size);


void usage(char *progname);
void record_wav_file(char *filename);

int main(int argc,char *argv[])
{
   int c;

   while ((c = getopt(argc, argv, "i:d:")) != EOF)
   {
      switch(c)
      {
         case 'i':
            interval_s = atof(optarg);
            printf("warning : -i option doesnt do much right now!\n");
            break;
         case 'd':
            dsp_devname = strdup(optarg);
            break;
         default:
            usage(argv[0]);
            exit(1);
         }
   }


  /* right now we only record one wav at a time... */

   if ((optind + 1) != argc)
   {
      usage(argv[0]);
      exit(1);
   }



   for (c = optind; c < argc; c++)
   {
      record_wav_file(argv[c]);
   }


   return(0);
}


void record_wav_file(char *filename)
{
   char *wavbuf;
   int wavfd;
   int dspfd;
   size_t interval;
   int running = 1;
   size_t bytes_written = 0;

   WAVE_HEADER wav_header, *wav_header_on_disk;

   newtComponent vu_1sec;
   newtComponent vu_total;
   newtComponent label_length;
   newtComponent mainwaveform;
   newtComponent label_1sec;
   newtComponent label_total;
   newtComponent rf_result;

   char labelstr_1sec[10] = "0";
   char labelstr_total[10] = "0";
   char labelstr_length[10] = "";
   short one_sec_max, total_max;

   wavfd = open(filename,O_RDWR | O_CREAT | O_TRUNC,0600);
   if (wavfd == -1)
   {
      printf("Error: open() %s\n",strerror(errno));
      exit(1);
   }

   create_wav_header(&wav_header,
      NUMBER_OF_CHANNELS,SAMPLES_PER_SECOND,BITS_PER_SAMPLE);

   if (write(wavfd,(void *)&wav_header,sizeof(wav_header)) != sizeof(wav_header))
   {
      printf("Error: write() %s\n",strerror(errno));
      exit(1);
   }

   dspfd = open_dsp_read(dsp_devname,&interval);
   if (dspfd == -1)
   {
      printf("Error getting %s ready for recording\n",dsp_devname);
   }

   interval_s = wav_header.nAvgBytesPerSec * interval;


   if ((wavbuf = malloc(interval)) == NULL)
   {
      printf("Error: malloc() %s\n",strerror(errno));
      exit(1);
   }
   

   newtInit();
   newtCls();

   newtDrawRootText(0, 0, filename);
   mainwaveform = newtForm(NULL, NULL,  NEWT_FLAG_NOF12);
   vu_1sec    =  newtScale(9,5,68,(long long)(SHRT_MAX));
   label_1sec =  newtLabel(1,5,labelstr_1sec);
   label_length = newtLabel(1,3,labelstr_length);
   vu_total =  newtScale(9,8,68,(long long)(SHRT_MAX));
   label_total =  newtLabel(1,8,labelstr_total);
   
   newtFormAddComponent(mainwaveform,vu_1sec);
   newtFormAddComponent(mainwaveform,vu_total);
   newtFormAddComponent(mainwaveform,label_1sec);
   newtFormAddComponent(mainwaveform,label_total);

   one_sec_max = 0;
   total_max = 0;
   newtFormWatchFd(mainwaveform,dspfd,NEWT_FD_READ); 
   newtFormAddHotKey(mainwaveform,NEWT_KEY_ENTER);
   newtPushHelpLine("Hit Enter to end recording");
   newtCenteredWindow(78,10,"now recording .wav file");

   newtRefresh();

   /* presently every second */

   while (running)
   {
      if (read(dspfd,wavbuf,interval) != interval)
      {
         newtFinished();         
         printf("read(dsp) %s",strerror(errno));
         exit(1);
      }
      if (write(wavfd,wavbuf,interval) != interval)
      {
         newtFinished();
         printf("write(%s) %s",filename,strerror(errno));
         exit(1);
      }
      bytes_written += interval;
         
      one_sec_max = get_peak_value((short *)wavbuf,(short *)(wavbuf + interval));
      newtScaleSet(vu_1sec,one_sec_max);

      sprintf(labelstr_1sec,"%1.6f",((double )one_sec_max/ (double )SHRT_MAX));
      newtLabelSetText(label_1sec,labelstr_1sec);
      sprintf(labelstr_length,"%7ld",
         bytes_written / wav_header.nAvgBytesPerSec);
      newtLabelSetText(label_length,labelstr_length);
      if (one_sec_max > total_max)
      {
         total_max = one_sec_max;
         if (total_max > (SHRT_MAX - 1))
         {
            sprintf(labelstr_total,"CLIPPED");
         }
         else
         {
            sprintf(labelstr_total,"%1.6f",
               ((float )total_max/ (float )SHRT_MAX));
         }
         newtLabelSetText(label_total,labelstr_total);
         newtScaleSet(vu_total,total_max);
      }
      rf_result = newtRunForm(mainwaveform);
      
      if (rf_result == NULL)
      {
         running = 0;
      }

      newtRefresh();
   }
   newtFormDestroy(mainwaveform);
   newtFinished();         

   /* set the wav file header to have the right length */
   wav_header_on_disk = (WAVE_HEADER *)mmap(NULL,sizeof(WAVE_HEADER),
      PROT_READ | PROT_WRITE,MAP_SHARED,wavfd,0);
   if (wav_header_on_disk == MAP_FAILED)
   {
      printf("Error: mmap() %s\n",strerror(errno));
      printf("The .wav will be OK but you may have to fix the length\n");
      exit(1);
   }
   wav_header_on_disk->nDataBytes = bytes_written;
   munmap(wav_header_on_disk,sizeof(WAVE_HEADER));

   free(wavbuf);
   close(wavfd);               
   close(dspfd);


      
   return;
}

/**

Opens up /dev/dsp to read audio data 

presently forces 16bit stereo 44100 hz (what else would we want?).

Returns the fd to thedsp, and sets the block size in blk_size.

**/

int open_dsp_read(char *dsp_filename,size_t *blk_size)
{
   int dspfd;
   int status;
   int arg;

   /** open the sound device and set it up **/
   dspfd = open(dsp_filename,O_RDONLY);
   if (dspfd < 0) 
   {
      printf("Error: open(%s) %s\n", dsp_filename,strerror(errno));
      return(-1);
   }
   arg = BITS_PER_SAMPLE;
   status = ioctl(dspfd, SOUND_PCM_WRITE_BITS, &arg);

   if (status == -1)
   {
      perror("SOUND_PCM_WRITE_BITS ioctl failed");
      return(-1);
   }
   if (arg != BITS_PER_SAMPLE)
   {
      perror("unable to set sample size");
      return(-1);
   }

   arg = NUMBER_OF_CHANNELS;  /* mono or stereo */
   status = ioctl(dspfd, SOUND_PCM_WRITE_CHANNELS, &arg);
   if (status == -1)
   {
      perror("SOUND_PCM_WRITE_CHANNELS ioctl failed");
      return(-1);
   }
   if (arg != NUMBER_OF_CHANNELS)
   {
      perror("unable to set number of channels");
      return(-1);
   }

   arg = SAMPLES_PER_SECOND;      /* sampling rate */
   status = ioctl(dspfd, SOUND_PCM_WRITE_RATE, &arg);
   if (status == -1)
   {
      perror("SOUND_PCM_WRITE_RATE ioctl failed");
      return(-1);
   }
   status = ioctl(dspfd, SNDCTL_DSP_GETBLKSIZE,&arg);
   /* printf("status %d, blksize %d\n",status,arg); */
   *blk_size = arg;
   return(dspfd);
}

void usage(char *progname)
{
   printf("%s [ -d device ] file.wav\n", progname);
   printf(" Version %s duncan@zog.net.au (c) 1999\n",VERSION);
}

