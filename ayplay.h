/* $Id: ayplay.h,v 1.4 2003/06/24 19:48:20 root Exp $ */
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
extern _UC pt3_player[];
