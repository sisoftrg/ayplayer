# (c)2004 sisoft\trg - AYplayer.
# $Id: Makefile.am,v 1.11 2004/02/06 09:04:52 root Exp $
AUTOMAKE_OPTIONS = foreign 1.5
EXTRA_DIST = makefile.win sndemu_w.c
bin_PROGRAMS = ayplayer
noinst_HEADERS = ayplay.h players.h z80.h z80_emu.h

ayplayer_SOURCES = ayplay.c unlzh.c z80.c players.c \
	sndemu.c z80_step.c z80optab.c z80_op1.c z80_op2.c \
	z80_op3.c z80_op4.c z80_op5.c z80_op6.c z80emu.S
