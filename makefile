#(c)2002 sisoft\trg - AYplayer.
# $Id: makefile,v 1.6 2003/06/25 00:39:25 root Exp $

bindir = /usr/local/bin
CC = gcc
CFLAGS = -g -O2 -fomit-frame-pointer -Wall
LFLAGS = -g
AFLAGS =
CDEFS = -D__LPT_PORT_=0x378

all: ayplayer

ayplay.o: ayplay.c ayplay.h z80.h makefile
	$(CC) $(CFLAGS) $(CDEFS) -c ayplay.c

unlzh.o: unlzh.c ayplay.h
	$(CC) $(CFLAGS) -c unlzh.c

z80.o: z80.c z80.h ayplay.h makefile
	$(CC) $(CFLAGS) -c z80.c

z80emu.o: z80emu.S
	$(CC) $(AFLAGS) -c z80emu.S

pt2_pl.o: pt2_pl.c
	$(CC) $(CFLAGS) -c pt2_pl.c
pt3_pl.o: pt3_pl.c
	$(CC) $(CFLAGS) -c pt3_pl.c
stp_pl.o: stp_pl.c
	$(CC) $(CFLAGS) -c stp_pl.c
stc_pl.o: stc_pl.c
	$(CC) $(CFLAGS) -c stc_pl.c
psc_pl.o: psc_pl.c
	$(CC) $(CFLAGS) -c psc_pl.c
asc_pl.o: asc_pl.c
	$(CC) $(CFLAGS) -c asc_pl.c

ayplayer: ayplay.o unlzh.o z80.o z80emu.o pt2_pl.o pt3_pl.o stp_pl.o stc_pl.o psc_pl.o asc_pl.o
	$(CC) $(LFLAGS) -o ayplayer ayplay.o unlzh.o z80.o z80emu.o pt2_pl.o pt3_pl.o stp_pl.o stc_pl.o psc_pl.o asc_pl.o

clean:
	rm -f core *.o ayplayer *~

install: ayplayer
	install -c -m 664 ayplayer $(bindir)/ayplayer
