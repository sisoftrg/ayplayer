/* $Id */
#include "z80.h"
#include "ayplay.h"

Z80 PRNM(proc);

byte PRNM(inports)[PORTNUM];
byte PRNM(outports)[PORTNUM];

#define NUM64KSEGS 1

static byte *a64kmalloc(int num64ksegs)
{
  byte *bigmem;
  
  bigmem = (byte *) calloc(0x10000 * (num64ksegs + 1),1);
  if(bigmem == NULL) {
	erro("z80: out of memory");
  }

  return (byte *) (( (long) bigmem & ~((long) 0xFFFF)) + 0x10000);
}



void PRNM(init)() 
{
  qbyte i;

  DANM(mem) = a64kmalloc(NUM64KSEGS);

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

void PRNM(interrupt)()
{
  if(DANM(iff1)) {

    DANM(haltstate) = 0;
    DANM(iff1) = DANM(iff2) = 0;

    switch(DANM(it_mode)) {
    case 0:
      PRNM(pushpc)();
      PC = 0x0038;
      break;
    case 1:
      PRNM(pushpc)();
      PC = 0x0038;
      break;
    case 2:
      PRNM(pushpc)();
      PCL = DANM(mem)[(dbyte) (((int) RI << 8) + 0xFF)];
      PCH = DANM(mem)[(dbyte) (((int) RI << 8) + 0xFF + 1)];
      break;
    }
  }
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
