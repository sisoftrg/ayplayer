# $Id: Makefile.am,v 1.2 2003/10/30 08:54:14 root Exp $
AUTOMAKE_OPTIONS = foreign 1.5

bin_PROGRAMS = ayplayer
noinst_HEADERS = ayplay.h z80.h z80_emu.h

AYSRC = ayplay.c unlzh.c z80.c  demo_pl.c
PLAYERS = asc_pl.c psc_pl.c pt2_pl.c pt3_pl.c stc_pl.c stp_pl.c

if ASM_SRC
AYSRC_emu = z80emu.S
else
AYSRC_emu = z80_step.c z80optab.c z80_op1.c z80_op2.c \
	    z80_op3.c z80_op4.c z80_op5.c z80_op6.c
endif

ayplayer_SOURCES = $(AYSRC) $(AYSRC_emu) $(PLAYERS) 
