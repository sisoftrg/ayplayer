/* (c)2003 sisoft\trg - AYplayer.
\* $Id: z80_op3.c,v 1.2 2004/04/26 12:18:52 root Exp $
 \ original version of this file was taken from SpectEmu0.92 by Miklos Szeredi
  */
#include "z80_emu.h"
#ifdef EZ80

#define ARIR(arin, func, an, rn, r, n) \
OPDEF(arin ## _ ## rn, 0x80+an*8+n)      \
{                                        \
  func(r);                               \
  ENTIME(4);                             \
}

#define ARIIHL(arin, func, an) \
OPDEF(arin ## _ihl, 0x86+an*8)           \
{                                        \
  func(*HLP);                            \
  ENTIME(7);                             \
}

#define ARIID(arin, func, an, ixyn, ixy) \
OPDEF(arin ## _i ## ixyn ## d, 0x86+an*8)\
{                                        \
  register dbyte addr;                   \
  register byte val;                     \
  IXDGET(ixy, addr);                     \
  val = READ(addr);                      \
  func(val);                             \
  ENTIME(15);                            \
}


#define ADD_A_R(rn, r, n) ARIR(add_a, ADD, 0, rn, r, n)
#define ADC_A_R(rn, r, n) ARIR(adc_a, ADC, 1, rn, r, n)
#define SUB_R(rn, r, n)   ARIR(sub,   SUB, 2, rn, r, n)
#define SBC_A_R(rn, r, n) ARIR(sbc_a, SBC, 3, rn, r, n)
#define AND_R(rn, r, n)   ARIR(and,   AND, 4, rn, r, n)
#define XOR_R(rn, r, n)   ARIR(xor,   XOR, 5, rn, r, n)
#define OR_R(rn, r, n)    ARIR(or,    OR,  6, rn, r, n)
#define CP_R(rn, r, n)    ARIR(cp,    CP,  7, rn, r, n)

ADD_A_R(b, RB, 0)
ADD_A_R(c, RC, 1)
ADD_A_R(d, RD, 2)
ADD_A_R(e, RE, 3)
ADD_A_R(h, RH, 4)
ADD_A_R(l, RL, 5)
ADD_A_R(a, RA, 7)

ADC_A_R(b, RB, 0)
ADC_A_R(c, RC, 1)
ADC_A_R(d, RD, 2)
ADC_A_R(e, RE, 3)
ADC_A_R(h, RH, 4)
ADC_A_R(l, RL, 5)
ADC_A_R(a, RA, 7)

SUB_R(b, RB, 0)
SUB_R(c, RC, 1)
SUB_R(d, RD, 2)
SUB_R(e, RE, 3)
SUB_R(h, RH, 4)
SUB_R(l, RL, 5)
SUB_R(a, RA, 7)

SBC_A_R(b, RB, 0)
SBC_A_R(c, RC, 1)
SBC_A_R(d, RD, 2)
SBC_A_R(e, RE, 3)
SBC_A_R(h, RH, 4)
SBC_A_R(l, RL, 5)
SBC_A_R(a, RA, 7)

AND_R(b, RB, 0)
AND_R(c, RC, 1)
AND_R(d, RD, 2)
AND_R(e, RE, 3)
AND_R(h, RH, 4)
AND_R(l, RL, 5)
AND_R(a, RA, 7)

XOR_R(b, RB, 0)
XOR_R(c, RC, 1)
XOR_R(d, RD, 2)
XOR_R(e, RE, 3)
XOR_R(h, RH, 4)
XOR_R(l, RL, 5)
/* XOR_R(a, RA, 7) */

OPDEF(xor_a, 0xAF)
{
  RA = 0;
  RF = (RF & ~(ALLF)) | (ZF | PVF);
  ENTIME(4);
}

OR_R(b, RB, 0)
OR_R(c, RC, 1)
OR_R(d, RD, 2)
OR_R(e, RE, 3)
OR_R(h, RH, 4)
OR_R(l, RL, 5)
OR_R(a, RA, 7)

CP_R(b, RB, 0)
CP_R(c, RC, 1)
CP_R(d, RD, 2)
CP_R(e, RE, 3)
CP_R(h, RH, 4)
CP_R(l, RL, 5)
CP_R(a, RA, 7)

ARIIHL(add_a, ADD, 0)
ARIIHL(adc_a, ADC, 1)
ARIIHL(sub,   SUB, 2)
ARIIHL(sbc_a, SBC, 3)
ARIIHL(and,   AND, 4)
ARIIHL(xor,   XOR, 5)
ARIIHL(or,    OR,  6)
ARIIHL(cp,    CP,  7)

/* IX */

ADD_A_R(ixh, XH, 4)
ADD_A_R(ixl, XL, 5)

ADC_A_R(ixh, XH, 4)
ADC_A_R(ixl, XL, 5)

SUB_R(ixh, XH, 4)
SUB_R(ixl, XL, 5)

SBC_A_R(ixh, XH, 4)
SBC_A_R(ixl, XL, 5)

AND_R(ixh, XH, 4)
AND_R(ixl, XL, 5)

XOR_R(ixh, XH, 4)
XOR_R(ixl, XL, 5)

OR_R(ixh, XH, 4)
OR_R(ixl, XL, 5)

CP_R(ixh, XH, 4)
CP_R(ixl, XL, 5)

ARIID(add_a, ADD, 0, ix, IX)
ARIID(adc_a, ADC, 1, ix, IX)
ARIID(sub,   SUB, 2, ix, IX)
ARIID(sbc_a, SBC, 3, ix, IX)
ARIID(and,   AND, 4, ix, IX)
ARIID(xor,   XOR, 5, ix, IX)
ARIID(or,    OR,  6, ix, IX)
ARIID(cp,    CP,  7, ix, IX)

/* IY */

ADD_A_R(iyh, YH, 4)
ADD_A_R(iyl, YL, 5)

ADC_A_R(iyh, YH, 4)
ADC_A_R(iyl, YL, 5)

SUB_R(iyh, YH, 4)
SUB_R(iyl, YL, 5)

SBC_A_R(iyh, YH, 4)
SBC_A_R(iyl, YL, 5)

AND_R(iyh, YH, 4)
AND_R(iyl, YL, 5)

XOR_R(iyh, YH, 4)
XOR_R(iyl, YL, 5)

OR_R(iyh, YH, 4)
OR_R(iyl, YL, 5)

CP_R(iyh, YH, 4)
CP_R(iyl, YL, 5)

ARIID(add_a, ADD, 0, iy, IY)
ARIID(adc_a, ADC, 1, iy, IY)
ARIID(sub,   SUB, 2, iy, IY)
ARIID(sbc_a, SBC, 3, iy, IY)
ARIID(and,   AND, 4, iy, IY)
ARIID(xor,   XOR, 5, iy, IY)
ARIID(or,    OR,  6, iy, IY)
ARIID(cp,    CP,  7, iy, IY)

#else
#define for_suppress_warning
#endif
