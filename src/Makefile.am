# (c)2004 sisoft\trg - AYplayer.
# $Id: Makefile.am,v 1.3 2004/03/11 17:45:53 root Exp $
players=asc_pl.c demo_pl.c fls_pl.c ftc_pl.c fxm_pl.c gtr_pl.c psc_pl.c \
	pt1_pl.c pt2_pl.c pt3_pl.c sqt_pl.c stc_pl.c stp_pl.c
EXTRA_DIST = sndemu_w.c makefile.win $(players)
noinst_HEADERS = ayplay.h players.h z80.h z80_emu.h i18.h
bin_PROGRAMS = ayplayer

ayplayer_SOURCES = ayplay.c unlzh.c z80.c players.c \
	sndemu.c z80_step.c z80optab.c z80_op1.c z80_op2.c \
	z80_op3.c z80_op4.c z80_op5.c z80_op6.c z80emu.S

localedir=$(datadir)/locale
DEFS += -DLOCALEDIR=\"$(localedir)\"
