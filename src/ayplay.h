/* (c)2004 sisoft\trg - AYplayer.
\* $Id: ayplay.h,v 1.3 2004/04/26 12:18:51 root Exp $ */
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
#include "i18.h"

#define _UC unsigned char
#define _UL unsigned long
#define _US unsigned short

#ifndef UNIX
#define strcasecmp stricmp
#define strncasecmp strnicmp
#endif

#ifdef WIN
#define XSLEEP delay(20)
#define outb(d,p) outp(p,d)
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
#endif

#ifdef WIN
#define POINT 'ù'
#else
#define POINT '•'
#endif

extern void unlh5(_UC*,_UC*,_UL,_UL);
extern void erro(char*);

#include "players.h"

#endif
