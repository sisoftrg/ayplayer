/* (c)2003 sisoft\trg - AYplayer.
\* $Id: z80_op2.c,v 1.2 2004/04/26 12:18:52 root Exp $
 \ original version of this file was taken from SpectEmu0.92 by Miklos Szeredi
  */
#include "z80_emu.h"
#ifdef EZ80

OPDEF(halt, 0x76)
{
  register int nn;

  DANM(haltstate) = 1;
  nn = (DANM(tc) - 1) / 4 + 1;
  
  DANM(tc) -= 4 * nn;
  RR += nn-1;
  ENDOP();
}

#define LD_R_R(rdn, rsn, rd, rs, n1, n2) \
OPDEF(ld_ ## rdn ## _ ## rsn, 0x40 + n1 * 8 + n2)          \
{                                                          \
  rd = rs;                                                 \
  ENTIME(4);                                               \
}

LD_R_R(b, c, RB, RC, 0, 1)
LD_R_R(b, d, RB, RD, 0, 2)
LD_R_R(b, e, RB, RE, 0, 3)
LD_R_R(b, h, RB, RH, 0, 4)
LD_R_R(b, l, RB, RL, 0, 5)
LD_R_R(b, a, RB, RA, 0, 7)

LD_R_R(c, b, RC, RB, 1, 0)
LD_R_R(c, d, RC, RD, 1, 2)
LD_R_R(c, e, RC, RE, 1, 3)
LD_R_R(c, h, RC, RH, 1, 4)
LD_R_R(c, l, RC, RL, 1, 5)
LD_R_R(c, a, RC, RA, 1, 7)

LD_R_R(d, b, RD, RB, 2, 0)
LD_R_R(d, c, RD, RC, 2, 1)
LD_R_R(d, e, RD, RE, 2, 3)
LD_R_R(d, h, RD, RH, 2, 4)
LD_R_R(d, l, RD, RL, 2, 5)
LD_R_R(d, a, RD, RA, 2, 7)


LD_R_R(e, b, RE, RB, 3, 0)
LD_R_R(e, c, RE, RC, 3, 1)
LD_R_R(e, d, RE, RD, 3, 2)
LD_R_R(e, h, RE, RH, 3, 4)
LD_R_R(e, l, RE, RL, 3, 5)
LD_R_R(e, a, RE, RA, 3, 7)

LD_R_R(h, b, RH, RB, 4, 0)
LD_R_R(h, c, RH, RC, 4, 1)
LD_R_R(h, d, RH, RD, 4, 2)
LD_R_R(h, e, RH, RE, 4, 3)
LD_R_R(h, l, RH, RL, 4, 5)
LD_R_R(h, a, RH, RA, 4, 7)

LD_R_R(l, b, RL, RB, 5, 0)
LD_R_R(l, c, RL, RC, 5, 1)
LD_R_R(l, d, RL, RD, 5, 2)
LD_R_R(l, e, RL, RE, 5, 3)
LD_R_R(l, h, RL, RH, 5, 4)
LD_R_R(l, a, RL, RA, 5, 7)

LD_R_R(a, b, RA, RB, 7, 0)
LD_R_R(a, c, RA, RC, 7, 1)
LD_R_R(a, d, RA, RD, 7, 2)
LD_R_R(a, e, RA, RE, 7, 3)
LD_R_R(a, h, RA, RH, 7, 4)
LD_R_R(a, l, RA, RL, 7, 5)


#define LD_R_IHL(rdn, rd, n) \
OPDEF(ld_ ## rdn ## _ihl, 0x46+n*8)          \
{                                            \
  rd = *HLP;                                 \
  ENTIME(7);                                 \
}

#define LD_R_ID(ixyn, ixy, rsn, rs, n) \
OPDEF(ld_ ## rsn ## _i ## ixyn ## d, 0x46+n*8) \
{                                  \
  register dbyte addr;             \
  IXDGET(ixy, addr);               \
  rs = READ(addr);                 \
  ENTIME(15);                      \
}


LD_R_IHL(b, RB, 0)
LD_R_IHL(c, RC, 1)
LD_R_IHL(d, RD, 2)
LD_R_IHL(e, RE, 3)
LD_R_IHL(h, RH, 4)
LD_R_IHL(l, RL, 5)
LD_R_IHL(a, RA, 7)

#define LD_IHL_R(rsn, rs, n) \
OPDEF(ld_ihl_ ## rsn, 0x70+n)      \
{                                  \
  PUTMEM(HL, HLP, rs);             \
  ENTIME(7);                       \
}

#define LD_ID_R(ixyn, ixy, rsn, rs, n) \
OPDEF(ld_i ## ixyn ## d_ ## rsn, 0x70+n) \
{                                  \
  register dbyte addr;             \
  IXDGET(ixy, addr);               \
  WRITE(addr, rs);                 \
  ENTIME(15);                      \
}



LD_IHL_R(b, RB, 0)
LD_IHL_R(c, RC, 1)
LD_IHL_R(d, RD, 2)
LD_IHL_R(e, RE, 3)
LD_IHL_R(h, RH, 4)
LD_IHL_R(l, RL, 5)
LD_IHL_R(a, RA, 7)

/* IX */

LD_R_R(b, ixh, RB, XH, 0, 4)
LD_R_R(b, ixl, RB, XL, 0, 5)

LD_R_R(c, ixh, RC, XH, 1, 4)
LD_R_R(c, ixl, RC, XL, 1, 5)

LD_R_R(d, ixh, RD, XH, 2, 4)
LD_R_R(d, ixl, RD, XL, 2, 5)

LD_R_R(e, ixh, RE, XH, 3, 4)
LD_R_R(e, ixl, RE, XL, 3, 5)

LD_R_R(ixh, b,   XH, RB, 4, 0)
LD_R_R(ixh, c,   XH, RC, 4, 1)
LD_R_R(ixh, d,   XH, RD, 4, 2)
LD_R_R(ixh, e,   XH, RE, 4, 3)
LD_R_R(ixh, ixl, XH, XL, 4, 5)
LD_R_R(ixh, a,   XH, RA, 4, 7)

LD_R_R(ixl, b,   XL, RB, 5, 0)
LD_R_R(ixl, c,   XL, RC, 5, 1)
LD_R_R(ixl, d,   XL, RD, 5, 2)
LD_R_R(ixl, e,   XL, RE, 5, 3)
LD_R_R(ixl, ixh, XL, XH, 5, 4)
LD_R_R(ixl, a,   XL, RA, 5, 7)


LD_R_R(a, ixh, RA, XH, 7, 4)
LD_R_R(a, ixl, RA, XL, 7, 5)

LD_ID_R(ix, IX, b, RB, 0)
LD_ID_R(ix, IX, c, RC, 1)
LD_ID_R(ix, IX, d, RD, 2)
LD_ID_R(ix, IX, e, RE, 3)
LD_ID_R(ix, IX, h, RH, 4)
LD_ID_R(ix, IX, l, RL, 5)
LD_ID_R(ix, IX, a, RA, 6)

LD_R_ID(ix, IX, b, RB, 0)
LD_R_ID(ix, IX, c, RC, 1)
LD_R_ID(ix, IX, d, RD, 2)
LD_R_ID(ix, IX, e, RE, 3)
LD_R_ID(ix, IX, h, RH, 4)
LD_R_ID(ix, IX, l, RL, 5)
LD_R_ID(ix, IX, a, RA, 6)


/* IY */

LD_R_R(b, iyh, RB, YH, 0, 4)
LD_R_R(b, iyl, RB, YL, 0, 5)

LD_R_R(c, iyh, RC, YH, 1, 4)
LD_R_R(c, iyl, RC, YL, 1, 5)

LD_R_R(d, iyh, RD, YH, 2, 4)
LD_R_R(d, iyl, RD, YL, 2, 5)

LD_R_R(e, iyh, RE, YH, 3, 4)
LD_R_R(e, iyl, RE, YL, 3, 5)

LD_R_R(iyh, b,   YH, RB, 4, 0)
LD_R_R(iyh, c,   YH, RC, 4, 1)
LD_R_R(iyh, d,   YH, RD, 4, 2)
LD_R_R(iyh, e,   YH, RE, 4, 3)
LD_R_R(iyh, iyl, YH, YL, 4, 5)
LD_R_R(iyh, a,   YH, RA, 4, 7)

LD_R_R(iyl, b,   YL, RB, 5, 0)
LD_R_R(iyl, c,   YL, RC, 5, 1)
LD_R_R(iyl, d,   YL, RD, 5, 2)
LD_R_R(iyl, e,   YL, RE, 5, 3)
LD_R_R(iyl, iyh, YL, YH, 5, 4)
LD_R_R(iyl, a,   YL, RA, 5, 7)


LD_R_R(a, iyh, RA, YH, 7, 4)
LD_R_R(a, iyl, RA, YL, 7, 5)

LD_ID_R(iy, IY, b, RB, 0)
LD_ID_R(iy, IY, c, RC, 1)
LD_ID_R(iy, IY, d, RD, 2)
LD_ID_R(iy, IY, e, RE, 3)
LD_ID_R(iy, IY, h, RH, 4)
LD_ID_R(iy, IY, l, RL, 5)
LD_ID_R(iy, IY, a, RA, 6)

LD_R_ID(iy, IY, b, RB, 0)
LD_R_ID(iy, IY, c, RC, 1)
LD_R_ID(iy, IY, d, RD, 2)
LD_R_ID(iy, IY, e, RE, 3)
LD_R_ID(iy, IY, h, RH, 4)
LD_R_ID(iy, IY, l, RL, 5)
LD_R_ID(iy, IY, a, RA, 6)

#endif
