# (c)2004 sisoft\trg - AYplayer.
# $Id: Makefile.am,v 1.2 2004/03/11 17:27:58 root Exp $
EXTRA_DIST = sndemu_w.c
bin_PROGRAMS = ayplayer
noinst_HEADERS = ayplay.h players.h z80.h z80_emu.h

ayplayer_SOURCES = ayplay.c unlzh.c z80.c players.c \
	sndemu.c z80_step.c z80optab.c z80_op1.c z80_op2.c \
	z80_op3.c z80_op4.c z80_op5.c z80_op6.c z80emu.S

localedir=$(datadir)/locale
DEFS += -DLOCALEDIR=\"$(localedir)\"
