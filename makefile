#(c)2003 sisoft\trg - AYplayer.
# $Id: makefile,v 1.12 2003/10/24 08:00:06 root Exp $

BINDIR = /usr/local/bin
CC = gcc
CFLAGS = -g -O3 -fomit-frame-pointer -Wall -pedantic -c
LFLAGS = -g
AFLAGS = -Wall -c
CDEFS = -DUNIX -D__LPT_PORT_=0x378
INSTALL = install -c -m 664
RM = rm -f
OBJ = .o
EXE =
MFE =
DO_ASM = 1

all: ayplayer$(EXE)

ayplay$(OBJ): ayplay.c ayplay.h z80.h makefile$(MFE)
	$(CC) $(CFLAGS) $(CDEFS) ayplay.c

unlzh$(OBJ): unlzh.c ayplay.h makefile$(MFE)
	$(CC) $(CFLAGS) $(CDEFS) unlzh.c

z80$(OBJ): z80.c z80.h ayplay.h makefile$(MFE)
	$(CC) $(CFLAGS) $(CDEFS) z80.c

ifeq ($(DO_ASM),1)
Z80_OBJS = z80emu$(OBJ)
else
Z80_OBJS = z80_step$(OBJ) z80optab$(OBJ) z80_op1$(OBJ) z80_op2$(OBJ) z80_op3$(OBJ) z80_op4$(OBJ) z80_op5$(OBJ) z80_op6$(OBJ)
CDEFS += -DEZ80
endif

z80emu$(OBJ): z80emu.S makefile$(MFE)
	$(CC) $(AFLAGS) z80emu.S

z80_op1$(OBJ): z80_op1.c z80_emu.h
	$(CC) $(CFLAGS) $(CDEFS) z80_op1.c
z80_op2$(OBJ): z80_op2.c z80_emu.h
	$(CC) $(CFLAGS) $(CDEFS) z80_op2.c
z80_op3$(OBJ): z80_op3.c z80_emu.h
	$(CC) $(CFLAGS) $(CDEFS) z80_op3.c
z80_op4$(OBJ): z80_op4.c z80_emu.h
	$(CC) $(CFLAGS) $(CDEFS) z80_op4.c
z80_op5$(OBJ): z80_op5.c z80_emu.h
	$(CC) $(CFLAGS) $(CDEFS) z80_op5.c
z80_op6$(OBJ): z80_op6.c z80_emu.h
	$(CC) $(CFLAGS) $(CDEFS) z80_op6.c
z80optab$(OBJ): z80optab.c z80_emu.h z80.h makefile$(MFE)
	$(CC) $(CFLAGS) $(CDEFS) z80optab.c
z80_step$(OBJ): z80_step.c z80_emu.h z80.h makefile$(MFE)
	$(CC) $(CFLAGS) $(CDEFS) z80_step.c

pt2_pl$(OBJ): pt2_pl.c
	$(CC) $(CFLAGS) pt2_pl.c
pt3_pl$(OBJ): pt3_pl.c
	$(CC) $(CFLAGS) pt3_pl.c
stp_pl$(OBJ): stp_pl.c
	$(CC) $(CFLAGS) stp_pl.c
stc_pl$(OBJ): stc_pl.c
	$(CC) $(CFLAGS) stc_pl.c
psc_pl$(OBJ): psc_pl.c
	$(CC) $(CFLAGS) psc_pl.c
asc_pl$(OBJ): asc_pl.c
	$(CC) $(CFLAGS) asc_pl.c

ayplayer$(EXE): ayplay$(OBJ) unlzh$(OBJ) z80$(OBJ) $(Z80_OBJS) pt2_pl$(OBJ) pt3_pl$(OBJ) stp_pl$(OBJ) stc_pl$(OBJ) psc_pl$(OBJ) asc_pl$(OBJ)
	$(CC) $(LFLAGS) -o ayplayer$(EXE) ayplay$(OBJ) unlzh$(OBJ) z80$(OBJ) $(Z80_OBJS) pt2_pl$(OBJ) pt3_pl$(OBJ) stp_pl$(OBJ) stc_pl$(OBJ) psc_pl$(OBJ) asc_pl$(OBJ)

clean:
	$(RM) core *$(OBJ) ayplayer$(EXE) *~

install: ayplayer$(EXE)
	$(INSTALL) ayplayer$(EXE) $(BINDIR)/ayplayer$(EXE)
