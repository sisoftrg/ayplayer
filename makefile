#(c)2003 sisoft\trg - AYplayer.
# $Id: makefile,v 1.8 2003/07/01 08:12:56 root Exp $

BINDIR = /usr/local/bin
CC = gcc
CFLAGS = -g -O2 -fomit-frame-pointer -Wall -c
LFLAGS = -g
AFLAGS = -Wall -c
CDEFS = -D__LPT_PORT_=0x378 -DUNIX
INSTALL = install -c -m 664
RM = rm -f

all: ayplayer

ayplay.o: ayplay.c ayplay.h z80.h makefile
	$(CC) $(CFLAGS) $(CDEFS) ayplay.c

unlzh.o: unlzh.c ayplay.h makefile
	$(CC) $(CFLAGS) $(CDEFS) unlzh.c

z80.o: z80.c z80.h ayplay.h makefile
	$(CC) $(CFLAGS) $(CDEFS) z80.c

z80emu.o: z80emu.S makefile
	$(CC) $(AFLAGS) z80emu.S

pt2_pl.o: pt2_pl.c
	$(CC) $(CFLAGS) pt2_pl.c
pt3_pl.o: pt3_pl.c
	$(CC) $(CFLAGS) pt3_pl.c
stp_pl.o: stp_pl.c
	$(CC) $(CFLAGS) stp_pl.c
stc_pl.o: stc_pl.c
	$(CC) $(CFLAGS) stc_pl.c
psc_pl.o: psc_pl.c
	$(CC) $(CFLAGS) psc_pl.c
asc_pl.o: asc_pl.c
	$(CC) $(CFLAGS) asc_pl.c

ayplayer: ayplay.o unlzh.o z80.o z80emu.o pt2_pl.o pt3_pl.o stp_pl.o stc_pl.o psc_pl.o asc_pl.o
	$(CC) $(LFLAGS) -o ayplayer ayplay.o unlzh.o z80.o z80emu.o pt2_pl.o pt3_pl.o stp_pl.o stc_pl.o psc_pl.o asc_pl.o

clean:
	$(RM) core *.o ayplayer *~

install: ayplayer
	$(INSTALL) ayplayer $(BINDIR)/ayplayer
