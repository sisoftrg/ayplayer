/* AYplayer (c)2001-2006 sisoft//trg.
\* $Id: z80_step.c,v 1.4 2006/08/10 03:13:59 root Exp $
 \ original version of this file was taken from SpectEmu0.92 by Miklos Szeredi
  */
#include "z80_emu.h"
#ifdef EZ80

byte z80c_incf_tbl[256];
byte z80c_decf_tbl[256];
byte z80c_addf_tbl[256];
byte z80c_subf_tbl[256];
byte z80c_orf_tbl[256];


void PRNM(pushpc)()
{
  SP--;
  PUTMEM(SP, SPP, PCH);
  SP--;
  PUTMEM(SP, SPP, PCL);
}

static int parity(byte b)
{
  int i;
  int par;
  
  par = 0;
  for(i = 8; i; i--) par ^= (b & 1), b >>= 1;
  return par;
}

void PRNM(local_init)()
{
  int i;
  for(i = 0; i < 0x100; i++) {
    z80c_incf_tbl[i] = z80c_decf_tbl[i] = z80c_orf_tbl[i] = 0;

    z80c_orf_tbl[i] |= i & (SF | B3F | B5F);
    z80c_incf_tbl[i] |= i & (SF | B3F | B5F);
    z80c_decf_tbl[i] |= i & (SF | B3F | B5F);

    if(!parity(i)) z80c_orf_tbl[i] |= PVF;
  }

  z80c_incf_tbl[0] |= ZF;
  z80c_decf_tbl[0] |= ZF;
  z80c_orf_tbl[0] |= ZF;

  z80c_incf_tbl[0x80] |= PVF;
  z80c_decf_tbl[0x7F] |= PVF;

  for(i = 0; i < 0x100; i++) {
    int cr, c1, c2;
    int hr, h1, h2;
    int b5r;
    
    cr  = i & 0x80;
    c1  = i & 0x40;
    b5r = i & 0x20;
    c2  = i & 0x10;

    hr  = i & 0x08;
    h1  = i & 0x04;
    h2  = i & 0x01;

    z80c_addf_tbl[i] = 0;
    z80c_subf_tbl[i] = 0;
    if(cr) {
      z80c_addf_tbl[i] |= SF;
      z80c_subf_tbl[i] |= SF;
    }
    if(b5r) {
      z80c_addf_tbl[i] |= B5F;
      z80c_subf_tbl[i] |= B5F;
    }
    if(hr) {
      z80c_addf_tbl[i] |= B3F;
      z80c_subf_tbl[i] |= B3F;
    }
  
    if((c1 && c2) || (!cr && (c1 || c2))) z80c_addf_tbl[i] |= CF;
    if((h1 && h2) || (!hr && (h1 || h2))) z80c_addf_tbl[i] |= HF;

    if((!c1 && !c2 && cr) || (c1 && c2 && !cr)) z80c_addf_tbl[i] |= PVF;


    if((c2 && cr) || (!c1 && (c2 || cr))) z80c_subf_tbl[i] |= CF;
    if((h2 && hr) || (!h1 && (h2 || hr))) z80c_subf_tbl[i] |= HF;
    
    if((!c2 && !cr && c1) || (c2 && cr && !c1)) z80c_subf_tbl[i] |= PVF;

    
    z80c_subf_tbl[i] |= NF;
  }
}


int PRNM(step)(int t)
{
  DANM(tc) = 0;
  DANM(rl7) = RR & 0x80;

  if(DANM(haltstate)) {
    register int nn;
    nn = (DANM(tc) - 1) / 4 + 1;
    
    DANM(tc) -= 4 * nn;
    RR += nn;
  }
  else do {
    register byte nextop;
    nextop = *PCP; 
    PC++;
    (*z80c_op_tab[nextop])();
    RR++; 
  } while(DANM(tc) > 0);

  RR = (RR & 0x7F) | DANM(rl7);
  return DANM(tc);
}

#endif
