//(c)2003 sisoft\trg - AYplayer.
/* $Id: ayplay.h,v 1.14 2003/10/30 18:20:14 root Exp $ */
#ifndef __AYPLAY_H_
#define __AYPLAY_H_

#include <stdio.h>
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>
#ifdef UNIX
#include <sys/io.h>
#include <signal.h>
#include <unistd.h>
#else
#include <dos.h>
#include <io.h>
#include <conio.h>
#endif

#define dPort LPT_PORT
#define Port (dPort+2)

#define _UC unsigned char
#define _UL unsigned long
#define _US unsigned short

#ifndef UNIX
#define strcasecmp stricmp
#define strncasecmp strnicmp
#endif

#ifdef WIN
#define Sleep 20
#define outb(d,p) outp(p,d)
#define usleep delay
#else
#define Sleep 2000
#endif

extern void unlh5(_UC*,_UC*,_UL,_UL);
extern void erro(char*);

#define DEMO_T VTX
#define DEMO_S 946
extern _UC DEMO_D[];

#define PT2_init 49152
#define PT2_play 49158
#define PT2_song 0xca1f
extern _UC pt2_player[];

#define PT3_init 49152
#define PT3_play 49157
#define PT3_song 0xcd86
#define PT3_table 49664
extern _UC pt3_player[];
extern _US pt3_tables[];

#define STP_init 49152
#define STP_play 49158
#define STP_song 51048
extern _UC stp_player[];

#define STC_start 49152
#define STC_init 49163
#define STC_play 49166
#define STC_song 50344
extern _UC stc_player[];

#define PSC_init 49152
#define PSC_play 49158
#define PSC_song 52130
extern _UC psc_player[];

#define ASC_start 49152
#define ASC_init 49163
#define ASC_play 49166
#define ASC_song 50801
extern _UC asc_player[];

#define GTR_init 49152
#define GTR_play 49158
#define GTR_song 50677
extern _UC gtr_player[];

#define FTC_init 49152
#define FTC_play 49158
#define FTC_song 52069
extern _UC ftc_player[];

#define SQT_init 49152
#define SQT_play 49200
#define SQT_song 50448
extern _UC sqt_player[];

#endif
