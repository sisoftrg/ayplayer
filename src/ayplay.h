/* (c)2004 sisoft\trg - AYplayer.
\* $Id: ayplay.h,v 1.4 2004/08/02 09:44:26 root Exp $ */
#ifndef __AYPLAY_H_
#define __AYPLAY_H_

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>
#ifdef WIN32
#undef UNIX

#else
#ifdef UNIX
#include <sys/time.h>
#include <sys/io.h>
#include <signal.h>
#include <unistd.h>
#else
#include <dos.h>
#include <io.h>
#include <conio.h>
#include <bios.h>
#endif
#endif
#include "i18.h"

#define _UC unsigned char
#define _UL unsigned long
#define _US unsigned short

#ifndef UNIX
#define strcasecmp stricmp
#define strncasecmp strnicmp
#ifdef WIN32
#define XSLEEP
#ifdef LPT_PORT
#define outb(d,p)
#endif
#else
#define XSLEEP delay(20)
#ifdef LPT_PORT
#define outb(d,p) outp(p,d)
#endif
#endif
#define POINT 'ù'
#else
#ifdef LPT_PORT
#define XSLEEP { \
    struct timeval tv; \
    tv.tv_sec=0;tv.tv_usec=20000; \
    select(0,NULL,NULL,NULL,&tv); \
}
#else
#define XSLEEP
#endif
#define POINT '•'
#endif

extern void unlh5(_UC*,_UC*,_UL,_UL);
extern void erro(char*);

#include "players.h"

#endif
