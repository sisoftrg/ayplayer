#(c)2002 sisoft\trg - AYplayer.
# $Id: makefile,v 1.3 2003/06/24 19:48:20 root Exp $

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

pt2pl.o: pt2pl.c
	$(CC) $(CFLAGS) -c pt2pl.c

pt3pl.o: pt3pl.c
	$(CC) $(CFLAGS) -c pt3pl.c

ayplayer: ayplay.o unlzh.o z80.o z80emu.o pt2pl.o pt3pl.o
	$(CC) $(LFLAGS) -o ayplayer ayplay.o unlzh.o z80.o z80emu.o pt2pl.o pt3pl.o

clean:
	rm -f core *.o ayplayer *~

install: ayplayer
	install -c -m 664 ayplayer $(bindir)/ayplayer
