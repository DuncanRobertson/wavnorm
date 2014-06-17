/*

$Id: wavfuncs.h,v 1.2 1999/07/28 12:49:48 duncan Exp $

some common functions we do on wav files in this prog...

*/

#define VERSION "0.5"


/** data structure that holds the wav head info in a form we can use **/

typedef struct
{       char            RiffID[4];
        u_long          RiffSize;
        char            WaveID[4];
        char            FmtID[4];
        u_long          FmtSize;
        u_short         wFormatTag;
        u_short         nChannels;
        u_long          nSamplesPerSec;
        u_long          nAvgBytesPerSec;
        u_short         nBlockAlign;
        u_short         wBitsPerSample;
        char            DataID[4];
        u_long          nDataBytes;
} WAVE_HEADER;

void create_wav_header(WAVE_HEADER *wav_header,
   int nChannels,int nSamplesPerSec,int wBitsPerSample);
char *validate_wav_header(char *header, int verbose);
short get_peak_value(short *start, short *end);
void progressbar_peak(short *start,short *current,short *audioend, short max);
void progressbar(short *start,short *current,short *audioend);
