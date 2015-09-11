# AYplayer (c)2001-2006 sisoft//trg.
# $Id: Makefile.am,v 1.5 2006/08/10 03:13:51 root Exp $
players=asc_pl.c demo_pl.c fls_pl.c ftc_pl.c fxm_pl.c gtr_pl.c psc_pl.c \
	pt1_pl.c pt2_pl.c pt3_pl.c sqt_pl.c stc_pl.c stp_pl.c
EXTRA_DIST = sndemu_w.c makefile.win $(players)
noinst_HEADERS = ayplay.h players.h z80.h z80_emu.h i18.h
bin_PROGRAMS = ayplayer

ayplayer_SOURCES = ayplay.c unlzh.c players.c malloc.c \
	sndemu.c z80.c z80_step.c z80emu.S z80optab.c \
	z80_op1.c z80_op2.c z80_op3.c z80_op4.c z80_op5.c z80_op6.c

localedir=$(datadir)/locale
DEFS += -DLOCALEDIR=\"$(localedir)\"
AM_CCASFLAGS = $(DEFS) -I..
AM_CFLAGS = -Wno-unused-result
