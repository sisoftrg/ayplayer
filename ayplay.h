/* $Id: ayplay.h,v 1.1.1.1 2003/05/25 14:15:11 root Exp $ */
#include <stdio.h>
#include <stdlib.h>
#include <sys/io.h>
#include <time.h>
#include <signal.h>

#define dPort __LPT_PORT_
#define Port (dPort+2)

#define _UC unsigned char
#define _UL unsigned long
#define _US unsigned short

extern void unlh5(_UC*,_UC*,_UL,_UL);
extern void erro(char*);
