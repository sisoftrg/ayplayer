/* $Id: ayplay.h,v 1.6 2003/06/24 23:31:52 root Exp $ */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/io.h>
#include <signal.h>
#include <sys/stat.h>
#include <unistd.h>
#include <time.h>

#define dPort __LPT_PORT_
#define Port (dPort+2)

#define _UC unsigned char
#define _UL unsigned long
#define _US unsigned short

extern void unlh5(_UC*,_UC*,_UL,_UL);
extern void erro(char*);

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
