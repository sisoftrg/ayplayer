//(c)2003 sisoft\trg - AYplayer.
/* $Id: z80.c,v 1.1 2004/03/11 14:24:10 root Exp $ */

//original version of this file was taken from SpectEmu0.92 by Miklos Szeredi 

#include "z80.h"
#include "ayplay.h"

Z80 PRNM(proc);
byte PRNM(inports)[PORTNUM];
byte PRNM(outports)[PORTNUM];

static byte *a64kmalloc(int num64ksegs)
{
  byte *bigmem;
  bigmem=(byte*)calloc(0x10000*(num64ksegs + 1),1);
  if(bigmem==NULL)erro("z80: out of memory");
  return(byte*)(((long)bigmem&~((long)0xFFFF))+0x10000);
}

void PRNM(init)() 
{
  qbyte i;

  DANM(mem) = a64kmalloc(1);
  srand((unsigned int) time(NULL));
//  for(i = 0; i < 0x10000; i++) DANM(mem)[i] = (byte) rand();
  for(i = 0; i < NUMDREGS; i++) {
    DANM(nr)[i].p = DANM(mem);
    DANM(nr)[i].d.d = (dbyte) rand();
  }
  for(i = 0; i < BACKDREGS; i++) {
    DANM(br)[i].p = DANM(mem);
    DANM(br)[i].d.d = (dbyte) rand();
  }
  for(i = 0; i < PORTNUM; i++) PRNM(inports)[i] = PRNM(outports)[i] = 0;
  PRNM(local_init)();
  return;
}

void PRNM(reset)()
{
  DANM(haltstate) = 0;
  DANM(iff1) = DANM(iff2) = 1;
  DANM(it_mode) = 0;
  RI = 0;
  RR = 0;
  PC = 0;
}
