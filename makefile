#(c)2002 sisoft\trg - AYplayer.
# $Id: makefile,v 1.1 2003/05/25 14:15:11 root Exp $

bindir = /usr/local/bin
LPTport = 0x378
CC = gcc
CFLAGS = -g -O2 -Wall
CDEFS = -D__LPT_PORT_=$(LPTport)

all: ayplayer

ayplay.o: ayplay.c ayplay.h
	$(CC) $(CFLAGS) $(CDEFS) -c ayplay.c

unlzh.o: unlzh.c ayplay.h
	$(CC) $(CFLAGS) $(CDEFS) -c unlzh.c

ayplayer: ayplay.o unlzh.o
	$(CC) $(CFLAGS) -o ayplayer ayplay.o unlzh.o

clean:
	rm -f core *.o ayplayer

install: ayplayer
	cp ayplayer $(bindir)/ayplayer
