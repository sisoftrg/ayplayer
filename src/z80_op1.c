//(c)2003 sisoft\trg - AYplayer.
/* $Id: z80_op1.c,v 1.1 2004/03/11 14:24:11 root Exp $ */

//original version of this file was taken from SpectEmu0.92 by Miklos Szeredi 

#include "z80_emu.h"
#ifdef EZ80

OPDEF(nop, 0x00)
{
  ENTIME(4);
}

OPDEF(ex_af_afb, 0x08)
{
  register byte *ptmp;
    
  ptmp = DANM(br)[ZI_AF].p;
  DANM(br)[ZI_AF].p = DANM(nr)[ZI_AF].p;
  DANM(nr)[ZI_AF].p = ptmp;

  ENTIME(4);
}

OPDEF(djnz_e, 0x10)
{
  if(!--RB) {
    PC++;
    ENTIME(8);
  }
  else {
    PC += IPCS; PC++;
    ENTIME(13);
  }
}

OPDEF(jr_e, 0x18)
{
  PC += IPCS; PC++;
  ENTIME(12);
}

#define JR_CC_E(ccn, cc, n) \
OPDEF(jr_ ## ccn ## _e, 0x20+n*8)  \
{                                  \
  if(!(cc)) {                      \
    PC++;                          \
    ENTIME(7);                     \
  }                                \
  else {                           \
    PC += IPCS; PC++;              \
    ENTIME(12);                    \
  }                                \
}

JR_CC_E(nz, !TESTZF, 0)
JR_CC_E(z,  TESTZF,  1)
JR_CC_E(nc, !TESTCF, 2)
JR_CC_E(c,  TESTCF,  3)


#define LD_RR_NN(rrn, rr, n) \
OPDEF(ld_ ## rrn ## _nn, 0x01+n*0x10)         \
{                                             \
  FETCHD(rr);                                 \
  ENTIME(10);                                 \
}

LD_RR_NN(bc, BC, 0)
LD_RR_NN(de, DE, 1)
LD_RR_NN(hl, HL, 2)
LD_RR_NN(sp, SP, 3)

#define DADD(rr1, rr2) \
  register dbyte dtmp;             \
  register int idx;                \
  dtmp = rr1;                      \
  rr1 = dtmp + rr2;                \
  idx = DIDXCALC(dtmp, rr2, rr1);  \
  SETFLAGS(CF | NF | HF, TAB(addf_tbl)[idx] & (CF | HF))
     

#define ADD_RR_RR(rrn1, rr1, rrn2, rr2, n) \
OPDEF(add_## rrn1 ## _ ## rrn2, 0x09+n*0x10) \
{                                  \
  DADD(rr1, rr2);                  \
  ENTIME(11);                      \
}

ADD_RR_RR(hl, HL, bc, BC, 0)
ADD_RR_RR(hl, HL, de, DE, 1)
ADD_RR_RR(hl, HL, hl, HL, 2)
ADD_RR_RR(hl, HL, sp, SP, 3)

#define INC_RR(rrn, rr, n) \
OPDEF(inc_ ## rrn, 0x03+n*0x10)    \
{                                  \
  rr++;                            \
  ENTIME(6);                       \
}

INC_RR(bc, BC, 0)
INC_RR(de, DE, 1)
INC_RR(hl, HL, 2)
INC_RR(sp, SP, 3)

#define DEC_RR(rrn, rr, n) \
OPDEF(dec_ ## rrn, 0x0B+n*0x10)    \
{                                  \
  rr--;                            \
  ENTIME(6);                       \
}

DEC_RR(bc, BC, 0)
DEC_RR(de, DE, 1)
DEC_RR(hl, HL, 2)
DEC_RR(sp, SP, 3)

OPDEF(ld_ibc_a, 0x02)
{
  PUTMEM(BC, BCP, RA);
  ENTIME(7);
}

OPDEF(ld_ide_a, 0x12)
{
  PUTMEM(DE, DEP, RA);
  ENTIME(7);
}

#define LD_INN_RR(rrn, rr) \
OPDEF(ld_inn_ ## rrn, 0x22)        \
{                                  \
  register dbyte dtmp;             \
  FETCHD(dtmp);                    \
  DWRITE(dtmp, rr);                \
  ENTIME(16);                      \
}

LD_INN_RR(hl, HL)


OPDEF(ld_inn_a, 0x32)
{
  register dbyte dtmp;
  FETCHD(dtmp);
  WRITE(dtmp, RA);
  ENTIME(13);
}

OPDEF(ld_a_ibc, 0x0A)
{
  RA = *BCP;
  ENTIME(7);
}

OPDEF(ld_a_ide, 0x1A)
{
  RA = *DEP;
  ENTIME(7);
}


#define LD_RR_INN(rrn, rr) \
OPDEF(ld_ ## rrn ## _inn, 0x2A)    \
{                                  \
  register dbyte dtmp;             \
  FETCHD(dtmp);                    \
  rr = DREAD(dtmp);                \
  ENTIME(16);                      \
}

LD_RR_INN(hl, HL)


OPDEF(ld_a_inn, 0x3A)
{
  register dbyte dtmp;
  FETCHD(dtmp);
  RA = READ(dtmp);
  ENTIME(13);
}


#define INC(r) \
  r++;                             \
  SETFLAGS(SF | ZF | PVF | B3F | B5F, TAB(incf_tbl)[r])



#define INC_R(rn, r, n) \
OPDEF(inc_ ## rn, 0x04+n*8)        \
{                                  \
  INC(r);                          \
  ENTIME(4);                       \
}

INC_R(b, RB, 0)
INC_R(c, RC, 1)
INC_R(d, RD, 2)
INC_R(e, RE, 3)
INC_R(h, RH, 4)
INC_R(l, RL, 5)
INC_R(a, RA, 7)


OPDEF(inc_ihl, 0x34)
{
  MODMEM(INC);
  ENTIME(11);
}


#define DEC(r) \
  r--;                             \
  SETFLAGS(SF | ZF | PVF | B3F | B5F, TAB(decf_tbl)[r])


#define DEC_R(rn, r, n) \
OPDEF(dec_ ## rn, 0x05+n*8)        \
{                                  \
  DEC(r);                          \
  ENTIME(4);                       \
}

DEC_R(b, RB, 0)
DEC_R(c, RC, 1)
DEC_R(d, RD, 2)
DEC_R(e, RE, 3)
DEC_R(h, RH, 4)
DEC_R(l, RL, 5)
DEC_R(a, RA, 7)


OPDEF(dec_ihl, 0x35)
{
  MODMEM(DEC);
  ENTIME(11);
}

#define LD_R_N(rn, r, n) \
OPDEF(ld_ ## rn ## _n, 0x06+n*8)   \
{                                  \
  r = *PCP;                        \
  PC++;                            \
  ENTIME(7);                       \
}

LD_R_N(b, RB, 0)
LD_R_N(c, RC, 1)
LD_R_N(d, RD, 2)
LD_R_N(e, RE, 3)
LD_R_N(h, RH, 4)
LD_R_N(l, RL, 5)
LD_R_N(a, RA, 7)


OPDEF(ld_ihl_n, 0x36)
{
  PUTMEM(HL, HLP, *PCP);
  PC++;
  ENTIME(10);
}

OPDEF(rlca, 0x07)
{
  register byte btmp;
  btmp = (RA & 0x80) >> 7;
  SETFLAGS(HF | NF | CF, btmp);
  RA = (RA << 1) | btmp;
  ENTIME(4);
}

OPDEF(rrca, 0x0F)
{
  register byte btmp;
  btmp = (RA & 0x01);
  SETFLAGS(HF | NF | CF, btmp);
  if(btmp) {
    RA = (RA >> 1) | 0x80;
    ENTIME(4);
  }
  else {
    RA >>= 1;
    ENTIME(4);
  }
}


OPDEF(rla, 0x17)
{
  register byte btmp;
  btmp = RA & 0x80;
  RA = (RA << 1) | (RF & CF);
  SETFLAGS(HF | NF | CF, btmp >> 7);
  ENTIME(4);
}

OPDEF(rra, 0x1F)
{
  register byte btmp;
  btmp = TESTCF;
  SETFLAGS(HF | NF | CF, RA & 0x01);
  if(btmp) {
    RA = (RA >> 1) | 0x80;
    ENTIME(4);
  }
  else {
    RA >>= 1;
    ENTIME(4);
  }
}

OPDEF(daa, 0x27)
{
  register int flag;
  flag = RF;

  if(!TESTNF) {
    if(flag & CF) RA += 0x60;
    else if(RA > 0x99) RA += 0x60, flag |= CF;

    if(flag & HF) RA += 0x06;
    else if((RA & 0x0F) > 9) RA += 0x06, flag |= HF;
  }
  else {
    if(flag & CF) RA -= 0x60;
    else if(RA > 0x99) RA -= 0x60, flag |= CF;

    if(flag & HF) RA -= 0x06;
    else if((RA & 0x0F) > 9) RA -= 0x06, flag |= HF;
  }
  flag = (flag & ~(SF | ZF | PVF | B3F | B5F)) | TAB(orf_tbl)[RA];
  RF = flag;

  ENTIME(4);
}

OPDEF(cpl, 0x2F)
{
  RA = ~RA;
  SET_FL(HF | NF);
  ENTIME(4);
}

OPDEF(scf, 0x37)
{
  SETFLAGS(HF | NF, CF);
  ENTIME(4);
}

OPDEF(ccf, 0x3F)
{
  RF = (RF ^ CF) & ~(NF);
/* HF undefined */
  ENTIME(4);
}

/* IX */

LD_RR_NN(ix, IX, 2)

ADD_RR_RR(ix, IX, bc, BC, 0)
ADD_RR_RR(ix, IX, de, DE, 1)
ADD_RR_RR(ix, IX, ix, IX, 2)
ADD_RR_RR(ix, IX, sp, SP, 3)

INC_RR(ix, IX, 2)

DEC_RR(ix, IX, 2)

LD_INN_RR(ix, IX)

LD_RR_INN(ix, IX)

INC_R(ixh, XH, 4)
INC_R(ixl, XL, 5)

OPDEF(inc_iixd, 0x34)
{
  register dbyte addr;
  IXDGET(IX, addr);
  MODMEMADDR(INC, addr);
  ENTIME(19);
}

DEC_R(ixh, XH, 4)
DEC_R(ixl, XL, 5)

OPDEF(dec_iixd, 0x35)
{
  register dbyte addr;
  IXDGET(IX, addr);
  MODMEMADDR(DEC, addr);
  ENTIME(19);
}


LD_R_N(ixh, XH, 4)
LD_R_N(ixl, XL, 5)

OPDEF(ld_iixd_n, 0x36)
{
  register dbyte addr;
  IXDGET(IX, addr);
  WRITE(addr, READ(PC)); 
  PC++;
  ENTIME(15);
}


/* IY */

LD_RR_NN(iy, IY, 2)

ADD_RR_RR(iy, IY, bc, BC, 0)
ADD_RR_RR(iy, IY, de, DE, 1)
ADD_RR_RR(iy, IY, iy, IY, 2)
ADD_RR_RR(iy, IY, sp, SP, 3)

INC_RR(iy, IY, 2)

DEC_RR(iy, IY, 2)

LD_INN_RR(iy, IY)

LD_RR_INN(iy, IY)

INC_R(iyh, YH, 4)
INC_R(iyl, YL, 5)

OPDEF(inc_iiyd, 0x34)
{
  register dbyte addr;
  IXDGET(IY, addr);
  MODMEMADDR(INC, addr);
  ENTIME(19);
}

DEC_R(iyh, YH, 4)
DEC_R(iyl, YL, 5)

OPDEF(dec_iiyd, 0x35)
{
  register dbyte addr;
  IXDGET(IY, addr);
  MODMEMADDR(DEC, addr);
  ENTIME(19);
}


LD_R_N(iyh, YH, 4)
LD_R_N(iyl, YL, 5)

OPDEF(ld_iiyd_n, 0x36)
{
  register dbyte addr;
  IXDGET(IY, addr);
  WRITE(addr, READ(PC)); 
  PC++;
  ENTIME(15);
}

#endif