# 
# Simple makefile for wavnorm, nplay and nrecord
#
#

CC=gcc
CFLAGS=-Wall -g -O3

OUTS=wavnorm nplay nrecord

INSTALLDIR=
INSTALLBASE=$(INSTALLDIR)/usr/local

# default rule for .c -> .o
.c.o:
	$(CC) $(CFLAGS) -c -o $*.o $<

all: $(OUTS) depend

include depend

wavnorm: wavnorm.o wavfuncs.o progressbar.o
	$(CC) -g -o wavnorm wavnorm.o wavfuncs.o progressbar.o

nplay: nplay.o wavfuncs.o
	$(CC) -g -o nplay nplay.o wavfuncs.o -lnewt

nrecord: nrecord.o wavfuncs.o
	$(CC) -g -o nrecord nrecord.o wavfuncs.o -lnewt

clean:
	rm -f *.o $(OUTS) core depend

install: $(OUTS)
	strip $(OUTS)
	mkdir -p $(INSTALLBASE)/bin/
	mkdir -p $(INSTALLBASE)/man/man1/
	install -D -m755 $(OUTS) $(INSTALLBASE)/bin/
	install -D -m644 wavnorm.1 nplay.1 nrecord.1 $(INSTALLBASE)/man/man1/

# default rule for dependencies
depend:
	$(CC) $(CFLAGS) -MM *.c > depend
