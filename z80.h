//(c)2003 sisoft\trg - AYplayer.
/* $Id: z80.h,v 1.10 2003/11/05 12:41:23 root Exp $ */
//original version of this file was taken from SpectEmu0.92 by Miklos Szeredi 
#ifndef __Z80_H_
#define __Z80_H_

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#define PRNM(x) z80_ ## x
#define DANM(x) PRNM(proc).x

typedef unsigned char  byte;
typedef signed   char  sbyte;
typedef unsigned short dbyte;
typedef unsigned int   qbyte;

union dregp {
  struct { byte l, h, _b2, _b3; } s;
  struct { dbyte d, _d1; }        d;
  byte*                           p;
};

#define NUMDREGS  9
#define BACKDREGS 4
#define PORTNUM 256

typedef struct {
  union dregp nr[NUMDREGS];
  union dregp br[BACKDREGS];
  
  int haltstate;
  int it_mode;
  int iff1, iff2;
  
  byte *mem;

  int tc;
  int rl7;

#ifdef EZ80
  dbyte cbaddr;
#endif

  unsigned char r;
  unsigned char v;
} Z80;

extern Z80 PRNM(proc);

extern byte PRNM(inports)[];
extern byte PRNM(outports)[];

#ifndef LPT_PORT
#ifdef UNIX
extern int sound_freq;
extern int sound_stereo;
extern int sound_stereo_ay_narrow;
extern int soundfd;
extern int sixteenbit;
extern int sound_frame(int really);
extern void sound_frame_blank();
#endif
extern int sound_init();
extern void sound_end();
extern void sound_ay_write(int reg,int val);
extern void sound_ay_reset();
#endif

#define ZI_BC 0
#define ZI_DE 1
#define ZI_HL 2
#define ZI_AF 3
#define ZI_IR 4
#define ZI_IX 5
#define ZI_IY 6
#define ZI_PC 7
#define ZI_SP 8

#define BC  (DANM(nr)[ZI_BC].d.d)
#define DE  (DANM(nr)[ZI_DE].d.d)
#define HL  (DANM(nr)[ZI_HL].d.d)
#define AF  (DANM(nr)[ZI_AF].d.d)
#define IR  (DANM(nr)[ZI_IR].d.d)
#define IX  (DANM(nr)[ZI_IX].d.d)
#define IY  (DANM(nr)[ZI_IY].d.d)
#define PC  (DANM(nr)[ZI_PC].d.d)
#define SP  (DANM(nr)[ZI_SP].d.d)

#define BCP (DANM(nr)[ZI_BC].p)
#define DEP (DANM(nr)[ZI_DE].p)
#define HLP (DANM(nr)[ZI_HL].p)
#define PCP (DANM(nr)[ZI_PC].p)
#define SPP (DANM(nr)[ZI_SP].p)
#define IXP (DANM(nr)[ZI_IX].p)
#define IYP (DANM(nr)[ZI_IY].p)

#define RB  (DANM(nr)[ZI_BC].s.h)
#define RC  (DANM(nr)[ZI_BC].s.l)
#define RD  (DANM(nr)[ZI_DE].s.h)
#define RE  (DANM(nr)[ZI_DE].s.l)
#define RH  (DANM(nr)[ZI_HL].s.h)
#define RL  (DANM(nr)[ZI_HL].s.l)
#define RA  (DANM(nr)[ZI_AF].s.h)
#define RF  (DANM(nr)[ZI_AF].s.l)
#define RI  (DANM(nr)[ZI_IR].s.h)
#define RR  (DANM(nr)[ZI_IR].s.l)
#define XH  (DANM(nr)[ZI_IX].s.h)
#define XL  (DANM(nr)[ZI_IX].s.l)
#define YH  (DANM(nr)[ZI_IY].s.h)
#define YL  (DANM(nr)[ZI_IY].s.l)
#define PCH (DANM(nr)[ZI_PC].s.h)
#define PCL (DANM(nr)[ZI_PC].s.l)
#define SPH (DANM(nr)[ZI_SP].s.h)
#define SPL (DANM(nr)[ZI_SP].s.l)

#define BCBK (DANM(br)[ZI_BC].d.d)
#define DEBK (DANM(br)[ZI_DE].d.d)
#define HLBK (DANM(br)[ZI_HL].d.d)
#define AFBK (DANM(br)[ZI_AF].d.d)

#define BBK  (DANM(br)[ZI_BC].s.h)
#define CBK  (DANM(br)[ZI_BC].s.l)
#define DBK  (DANM(br)[ZI_DE].s.h)
#define EBK  (DANM(br)[ZI_DE].s.l)
#define HBK  (DANM(br)[ZI_HL].s.h)
#define LBK  (DANM(br)[ZI_HL].s.l)
#define ABK  (DANM(br)[ZI_AF].s.h)
#define FBK  (DANM(br)[ZI_AF].s.l)

extern void PRNM(init)();
extern int  PRNM(step)(int ticknum);
extern void PRNM(reset)();

extern void PRNM(pushpc)();
extern void PRNM(local_init)();

#endif
