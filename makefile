#(c)2002 sisoft\trg - AYplayer.
# $Id: makefile,v 1.2 2003/05/27 12:21:08 root Exp $

bindir = /usr/local/bin
CC = gcc
CFLAGS = -g -O2 -fomit-frame-pointer -Wall
CDEFS = -D__LPT_PORT_=0x378

all: ayplayer

ayplay.o: ayplay.c ayplay.h makefile
	$(CC) $(CFLAGS) $(CDEFS) -c ayplay.c

unlzh.o: unlzh.c ayplay.h
	$(CC) $(CFLAGS) $(CDEFS) -c unlzh.c

ayplayer: ayplay.o unlzh.o
	$(CC) $(CFLAGS) -o ayplayer ayplay.o unlzh.o

clean:
	rm -f core *.o ayplayer *~

install: ayplayer
	install -c -m 664 ayplayer $(bindir)/ayplayer
