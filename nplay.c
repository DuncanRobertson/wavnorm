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

nplay - simple wav player

duncan@zog.net.au

$Id: nplay.c,v 1.3 1999/07/28 13:18:06 duncan Exp $

*****/

/** Needed to compile on RH 4.X for some reason **/

#ifndef MAP_FAILED
#define MAP_FAILED -1
#endif

double interval_s = 0.1;  /* default interval in seconds */

int play_buffer(int dspfd, short *start, short *end);
int open_dsp(WAVE_HEADER *wav_info);

void usage(char *progname);
void play_wav_file(char *filename);

int main(int argc,char *argv[])
{
   int c;

   while ((c = getopt(argc, argv, "i:")) != EOF)
   {
      switch(c)
      {
         case 'i':
            interval_s = atof(optarg);
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

   newtInit();

   for (c = optind; c < argc; c++)
   {
      play_wav_file(argv[c]);
   }

   newtFinished();         

   return(0);
}


void play_wav_file(char *filename)
{
   char *inwavbuf;
   short *current;
   short *audioend;
   short *audio;
   WAVE_HEADER *wav_info;
   int wavfd;
   int dspfd;
   struct stat input_fstat;
   size_t interval;

   newtComponent vu_1sec;
   newtComponent vu_total;
   newtComponent wav_length;
   newtComponent label_length;
   newtComponent mainwaveform;
   newtComponent label_1sec;
   newtComponent label_total;
   newtComponent rf_result;

   char labelstr_1sec[10] = "0";
   char labelstr_total[10] = "0";
   char labelstr_length[10] = "";
   short one_sec_max, total_max;

   wavfd = open(filename,O_RDONLY,0600);
   if (wavfd == -1)
   {
      printf("Error: open() %s\n",strerror(errno));
      exit(1);
   }

   if (fstat(wavfd,&input_fstat) != 0)
   {
      printf("Error: fstat() %s\n",strerror(errno));
      return;
   }

   if (input_fstat.st_size < sizeof(WAVE_HEADER))
   {
      printf("File is not large enough to hold a .wav file header even!\n");
      return;
   }


   inwavbuf = mmap(NULL,input_fstat.st_size,PROT_READ,MAP_SHARED,wavfd,0);
   if (inwavbuf == MAP_FAILED)
   {
      printf("Error: mmap() %s\n",strerror(errno));
      exit(1);
   }


   audio = (short *)validate_wav_header(inwavbuf,0);
   current = audio;

   if (current == NULL)
   {
      printf("This program didn't like the wav file\n");
      exit(1);
   }

   wav_info = (WAVE_HEADER *)inwavbuf;
   audioend =  (short *)((char *)audio + wav_info->nDataBytes);

   dspfd = open_dsp(wav_info);

   newtCls();

   newtDrawRootText(0, 0, filename);
   mainwaveform = newtForm(NULL, NULL,  NEWT_FLAG_NOF12);
   vu_1sec    =  newtScale(9,5,68,(long long)(SHRT_MAX));
   label_1sec =  newtLabel(1,5,labelstr_1sec);
   wav_length  =  newtScale(9,3,68,audioend - audio);
   label_length = newtLabel(1,3,labelstr_length);
   vu_total =  newtScale(9,8,68,(long long)(SHRT_MAX));
   label_total =  newtLabel(1,8,labelstr_total);
   
   newtFormAddComponent(mainwaveform,vu_1sec);
   newtFormAddComponent(mainwaveform,vu_total);
   newtFormAddComponent(mainwaveform,label_1sec);
   newtFormAddComponent(mainwaveform,label_total);

   one_sec_max = 0;
   total_max = 0;
   newtFormWatchFd(mainwaveform,dspfd,NEWT_FD_WRITE); 
   newtFormAddHotKey(mainwaveform,NEWT_KEY_ENTER);
   newtPushHelpLine("Hit Enter to end playing");
   newtCenteredWindow(78,10,"now playing .wav file");

   newtRefresh();

   /* presently every second */
   interval = (size_t )((double )wav_info->nSamplesPerSec * interval_s * 2);

   while ((current) < audioend)
   {
      short *endcurrent = current + interval;

      if (endcurrent > audioend)
      {
         endcurrent = audioend;
      }
         
      one_sec_max = get_peak_value(current,endcurrent);
      newtScaleSet(vu_1sec,one_sec_max);
      sprintf(labelstr_1sec,"%1.6f",((float )one_sec_max/ (float )SHRT_MAX));
      newtLabelSetText(label_1sec,labelstr_1sec);
      newtScaleSet(wav_length,current - audio);
      sprintf(labelstr_length,"%4.2f",
         ((double )(current - audio) / 88200));
      newtLabelSetText(label_length,labelstr_length);
      if (one_sec_max > total_max)
      {
         total_max = one_sec_max;
         sprintf(labelstr_total,"%1.6f",((float )total_max/ (float )SHRT_MAX));
         newtLabelSetText(label_total,labelstr_total);
         newtScaleSet(vu_total,total_max);
      }
      rf_result = newtRunForm(mainwaveform);
      if (play_buffer(dspfd,current,endcurrent) == -1)
      {
         current = audioend;
      }
      
      current = endcurrent;
      if (rf_result == NULL)
         current = audioend;

      newtRefresh();
   }

   newtFormDestroy(mainwaveform);

   munmap(inwavbuf,input_fstat.st_size);
   close(wavfd);               
   close(dspfd);
      
   return;
}

/**

Opens up /dev/dsp to play audio data in a way that is
compatible with the given .wav header.

**/

int open_dsp(WAVE_HEADER *wav_info)
{
   int dspfd;
   int status;
   int arg;

   /** open the sound device and set it up **/
   dspfd = open("/dev/dsp",O_WRONLY);
   if (dspfd < 0) 
   {
      perror("open of /dev/dsp failed");
      return(-1);
   }
   arg = wav_info->wBitsPerSample;
   status = ioctl(dspfd, SOUND_PCM_WRITE_BITS, &arg);

   if (status == -1)
   {
      perror("SOUND_PCM_WRITE_BITS ioctl failed");
      return(-1);
   }
   if (arg != wav_info->wBitsPerSample)
   {
      perror("unable to set sample size");
      return(-1);
   }

   arg = wav_info->nChannels;  /* mono or stereo */
   status = ioctl(dspfd, SOUND_PCM_WRITE_CHANNELS, &arg);
   if (status == -1)
   {
      perror("SOUND_PCM_WRITE_CHANNELS ioctl failed");
      return(-1);
   }
   if (arg != wav_info->nChannels)
   {
      perror("unable to set number of channels");
      return(-1);
   }

   arg = wav_info->nSamplesPerSec;      /* sampling rate */
   status = ioctl(dspfd, SOUND_PCM_WRITE_RATE, &arg);
   if (status == -1)
   {
      perror("SOUND_PCM_WRITE_WRITE ioctl failed");
      return(-1);
   }
   return(dspfd);
}

int play_buffer(int dspfd, short *start, short *end)
{
   int status;

   status = write(dspfd,(char *)start,(char *)end - (char *)start); 
          /* play it back */

   if (status != (char *)end - (char *)start)
   {
      perror("wrote wrong number of bytes");
      return(-1);
   }
   
   return(0);
}

void usage(char *progname)
{
   printf("%s [-i interval] file.wav [file.wav ...]\n",
      progname);
   printf("   -i interval in seconds to update VU & screen, default 1.0\n");
}

