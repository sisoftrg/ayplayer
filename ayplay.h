/* $Id: ayplay.h,v 1.3 2003/05/28 09:39:52 root Exp $ */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/io.h>
#include <time.h>
#include <signal.h>
#include <sys/stat.h>
#include <unistd.h>

#define dPort __LPT_PORT_
#define Port (dPort+2)

#define _UC unsigned char
#define _UL unsigned long
#define _US unsigned short

extern void unlh5(_UC*,_UC*,_UL,_UL);
extern void erro(char*);
