/* (c)2005 sisoft\trg - AYplayer.
\* $Id: ayplay.h,v 1.6 2005/12/11 11:39:20 root Exp $ */
#ifndef __AYPLAY_H_
#define __AYPLAY_H_

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif/*have_c.h*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>
#ifdef WIN32
#undef UNIX
#else/*win32*/
#ifdef UNIX
#include <sys/time.h>
#include <sys/io.h>
#include <signal.h>
#include <unistd.h>
#else/*unix*/
#include <dos.h>
#include <io.h>
#include <conio.h>
#include <bios.h>
#endif/*unix*/
#endif/*win32*/
#include "i18.h"

#define _UC unsigned char
#define _UL unsigned long
#define _US unsigned short

#ifndef UNIX
#define strcasecmp stricmp
#define strncasecmp strnicmp
#ifdef WIN32
#define XSLEEP
#define usleep(x) Sleep(x)
#ifdef LPT_PORT
#define outb(d,p)
#endif/*lpt*/
#else/*win32*/
#define XSLEEP delay(20)
#ifdef LPT_PORT
#define outb(d,p) outp(p,d)
#endif/*lpt*/
#endif/*win32*/
#define POINT 'ù'
#else/*!unix*/
#ifdef LPT_PORT
#define XSLEEP pause()
#define USE_ITIMER
#else/*lpt*/
#define XSLEEP
#endif/*lpt*/
#define POINT '•'
#endif/*!unix*/

extern void unlh5(_UC*,_UC*,_UL,_UL);
extern void erro(char*);

#include "players.h"

#endif
