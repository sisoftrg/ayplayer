# AYplayer (c)2001-2006 sisoft//trg.
dnl Process this file with autoconf to produce a configure script.
AC_INIT([ayplayer],[0.16],[sisoftrg@gmail.com])
AM_INIT_AUTOMAKE
AC_PREREQ(2.53)
AC_CONFIG_SRCDIR([src/ayplay.c])
AM_CONFIG_HEADER([config.h])
AC_CANONICAL_HOST
AM_MAINTAINER_MODE

dnl Checks for programs.
AC_PROG_CC
AC_PROG_INSTALL
AM_PROG_AS

dnl Internationalize
AM_GNU_GETTEXT([external])
AM_GNU_GETTEXT_VERSION([0.19.6])

dnl Checks for header files.
AC_HEADER_STDC
AC_CHECK_HEADERS([stdlib.h string.h unistd.h malloc.h])
AC_CHECK_HEADER([sys/soundcard.h],oss=yes,oss=no)
AC_CHECK_HEADER([windows.h],windows=yes,windows=no)
AC_CHECK_HEADER([mmsystem.h],winmm=yes,winmm=no,[#include <windows.h>])
AC_CHECK_HEADER([dsound.h],dsound=yes,dsound=no,[#include <windows.h>])

dnl Checks for library functions.
AC_FUNC_MALLOC
AC_FUNC_MEMCMP
AC_TYPE_SIGNAL
AC_FUNC_STAT
AC_CHECK_FUNCS([memset strcasecmp strchr strncasecmp strrchr ioperm])

dnl ALSA sound
AM_PATH_ALSA

dnl Checks asm use.
AC_MSG_CHECKING([for Z80 emulation on ASM])
AC_ARG_ENABLE(asm,AC_HELP_STRING([--enable-asm],[enable Z80 emulation on assembler]),,enable_asm=no)
if test "${enable_asm}x" != "nox"; then
	AC_MSG_RESULT(yes)
else
	ASFLAGS="-DEZ80"
	AC_MSG_RESULT(no)
	AC_DEFINE(EZ80,1,[Disable Z80 emulation on assembler])
fi

dnl for sound output
AC_MSG_CHECKING([for sound output])
AC_ARG_WITH(lptport,AC_HELP_STRING([--with-lptport=hexaddr],[specifies LPT port address (none)]),,with_lptport=no)
AC_ARG_ENABLE(directsound,AC_HELP_STRING([--enable-directsound],[enable use of DirectSound on Win32]),,with_dsound=no)
case "$with_lptport" in
	no)	if test "x$oss" = "xyes"; then
			AC_MSG_RESULT(OSS emulaion)
		else
		    if test "x$dsound" = "xyes" -a "x$enable_directsound" = "xyes"; then
			AC_MSG_RESULT(Win32 DirectSound)
			AC_DEFINE_UNQUOTED(DSOUND,1,[use DirectSound])
			LDADD="-ldsound -lwinmm $LDADD"
		    else
			if test "x$winmm" = "xyes"; then
			    AC_MSG_RESULT(Win32 WaveOut)
			    LDADD="-lwinmm $LDADD"
			else
			    AC_MSG_ERROR(no sound outputs found)
			fi
		    fi
		fi;;
	0x378)	AC_MSG_RESULT(LPT1 (0x378));dig="no";;
	0x278)	AC_MSG_RESULT(LPT2 (0x278));dig="no";;
	0x3bc)	AC_MSG_RESULT(LPT3 (0x3bc));dig="no";;
	0x???)	AC_MSG_RESULT(LPT on $with_lptport);dig="no";;
	*)	AC_MSG_ERROR([Address '$with_lock_style' must be in hex style, ex. 0x378]);;
esac
if test "x$dig" = "xno"; then
	AC_DEFINE_UNQUOTED(LPT_PORT,$with_lptport,[LPT port address])
fi

dnl add warn flags in maintainer mode
if test "x$USE_MAINTAINER_MODE" = "xyes"; then
	CFLAGS="$CFLAGS -Wall -pedantic -O3"
fi

if test "x$windows" = "xyes"; then
    AC_DEFINE_UNQUOTED(WIN32,1,[Compile with Win32-style includes])
else
    AC_DEFINE_UNQUOTED(UNIX,1,[Compile with UNIX-style includes])
fi

AC_SUBST(LDADD)

AC_CONFIG_FILES([Makefile src/Makefile po/Makefile.in])

AC_OUTPUT
