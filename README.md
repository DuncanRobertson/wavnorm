wavnorm
=======

wavnorm - automatically maximizes the level of a .wav file, destructively, without using a temp file.
nplay - simple text mode .wav file player.
nrecord - simple text mode .wav file recorder, with level and clip monitor.



wavnorm, nplay, nrecord

These are some simple but handy audio tools that I wrote to fill a need
where I couldn't find similar tools that would do the job. All programs are
presently limited to 16bit stereo wav files at 44100 Hz, as they are
intended for preparing music for making demo CDs and/or mp3 files.

THESE PROGRAMS HAVE NO WARRANTY! They work fine for me but if they don't for
you you are welcome to read the code and send me a patch! 

They are placed under the GPL (see the file COPYING for details) and made
available "as is" in case anyone else finds it handy as well.

Compilation: wavnorm should compile on anything. nplay and nrecord need the
newt user interface library that is shipped with RedHat Linux. This is GPL
code and should be readily available for other systems.

wavnorm - automatically maximize the level of a .wav file.

This is a simple command line, destructive "normalizer" for 16 bit, stereo
wav files. By destructive I mean it works on the ORIGINAL file specified on
the command line, it does NOT make a copy. If you want more flexible
options, the sox package can do all this and more, but with a bit more
fiddling with command line options.

What it actually does is find the highest level in a .wav file, and then
scale the entire .wav file so that the loudest point is as loud as possible
without clipping. It does this "in place" on the original .wav file, so
space consuming temp files aren't created. This is handy when creating demo
compilation CDs of home recorded material from different sources, which can
vary widely in levels. wavnorm can impose SOME degree of consistency.

I make no claims about the audio quality - it sounds fine to me, if that is
your concern hire a mastering house to do it properly.

Usage: note that the program only actually alters anything if you specify
the "-d" (for DESTRUCTIVE) command line option.

The "-t" option can be used to specify a threshold under which it wont make
a change.. for example if the calculated ratio is 1.03 scaling up the .wav
wont make it that much louder, but probably will alter the sound a tiny bit,
so we may as well skip scaling up the level.

If wavnorm finds the length of the file as indicated by the .wav header is
not what it should be, it will update the length as well if run in
destructive mode.

wavnorm runs very quickly but is quite intensive in its use of system
resources. It is not a good idea to record audio with nrecord while running
wavnorm.

nplay - play a wav file

This is a very simple .wav player. It uses a newt/ncurses interface and will
play all the .wav files specified on the command line. It has a simple bar
graph to indicate how far through the file it is, plus 2 VU meter graphs one
to indicate the current max level in the current interval, the other to
indicate the total peak level hit so far. The interval in which the screen
is updated is set by the -i option, but defaults to 0.1 (10 times per
second).

nrecord - record a wav file, while monitoring the levels

nrecord records 16 bit stereo audio at 44100 Hz from the /dev/dsp device (or
another device if specified by the -d option) and saves it as a .wav file,
until enter is pressed. It uses a newt/ncurses interface to give a count of
elapsed time in seconds, and 2 VU meters, one indicating the current level,
and one indicated the highest level attained so far. If the level has hit
maximum, the word CLIPPED is displayed on the screen. The advantage of this
program is it's simplicity, as it allows simple stereo wav recording with
level monitoring without fiddling around with resource hungry multitrack
software. Run aumix or a similar mixer program in another xterm or console,
adjust the levels so there is enough headroom that clipping wont occur and
record.

LINUX KERNEL ISSUES

When using wavnorm with the Linux 2.2.17 kernel I get strange errors and
occasional lockups. This did not occur with the 2.2.18 kernel, which has 
some fixes for the VM stuff. wavnorm relies heavily on mmap(), so presumably
it was hitting the VM bugs that were fixed in 2.2.18.

CURRENT STATUS

Although I could add lots of features to this program, there are other
bits of software such as ecasound that do a range of command line audio
tasks very well. This program is intended to do just one thing but do it
quickly, and without needing temporary files. 
I have been very busy with other stuff (music, software, life) in the year
since 0.4, and have not had much time to add to this project, although it
is essentially feature complete. Thanks to all who have sent in patches,
suggestions and encouragment, I have tried to take your ideas on board
where possible.


duncan@zog.net.au 19/07/1999 - 14/07/2001

http://www.linuxbandwagon.com/
http://www.zog.net.au/

addendum, June 2014:
I've checked these into github "as is" to make the code available. Right now I havent checked that it builds or functions on a modern distro, but there is no reason it shouldn't.

