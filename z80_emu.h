//(c)2003 sisoft\trg - AYplayer.
/* $Id: z80_emu.h,v 1.2 2003/10/30 08:10:27 root Exp $ */

//original version of this file was taken from SpectEmu0.92 by Miklos Szeredi 
#ifndef __Z80_EMU_H_
#define __Z80_EMU_H_

#include "z80.h"

typedef void z80t;

#define OPDEF(name, num) void z80op_ ## name (z80t)
#define ENDOP() 

#define TAB(t) z80c_ ## t
#define PORT(t) PRNM(t)
#define SPECP(t) SPNM(t)


typedef z80t (*op_f)(z80t);
extern op_f z80c_op_tab[];
extern op_f z80c_op_tab_cb[];
extern op_f z80c_op_tab_dd[];
extern op_f z80c_op_tab_ed[];
extern op_f z80c_op_tab_fd[];

#define TIME(x) DANM(tc) -= (x)

#define ENTIME(x) { TIME(x); ENDOP(); }

#define READ(addr)  (DANM(mem)[addr])
#define WRITE(addr, val) PUTMEM(addr,  DANM(mem) + (dbyte) (addr), val)

#define DREAD(addr) (DANM(mem)[addr] | (DANM(mem)[(dbyte)(addr+1)] << 8))
#define DWRITE(addr, x) WRITE(addr, (byte) x); \
                        WRITE((dbyte) (addr+1), (byte) (x >> 8))

#define IPCS        ((sbyte) *PCP)


#define MODMEM(func) \
{                                        \
  register byte bdef;                    \
  bdef = *HLP;                           \
  func(bdef);                            \
  PUTMEM(HL, HLP, bdef);                 \
}

#define MODMEMADDR(func, addr) \
{                                        \
  register byte bdef;                    \
  bdef = READ(addr);                     \
  func(bdef);                            \
  WRITE(addr, bdef);                     \
}

#define IXDGET(ixy, addr) addr = ((dbyte) (ixy + IPCS)), PC++

#define FETCHD(x) \
{                                        \
  register dbyte ddef;                   \
  ddef = PC;                             \
  x = DREAD(ddef);                       \
  PC = ddef+2;                           \
}

#define POP(x) \
{                                        \
  register dbyte ddef;                   \
  ddef = SP;                             \
  x = DREAD(ddef);                       \
  SP = ddef+2;                           \
}

#define PUSH(x) \
{                                        \
  register dbyte ddef, dval;             \
  dval = x;                              \
  ddef = SP-2;                           \
  DWRITE(ddef, dval);                    \
  SP = ddef;                             \
}

#define PUTMEM(addr, ptr, val) *(ptr) = (val)
#define IN(porth, portl, dest) dest = PORT(inports)[portl]
//#define OUT(porth, portl, source) PORT(outports)[portl] = (source)
#define OUT(porth, portl, source) if((portl)==0xfd){if((porth)==0xff)DANM(r)=(source);else DANM(v)=(source);}

#define SF  0x80
#define ZF  0x40
#define B5F 0x20
#define HF  0x10
#define B3F 0x08
#define PVF 0x04
#define NF  0x02
#define CF  0x01

#define ALLF (SF | ZF | HF | PVF | NF | CF)
#define BUTCF (SF | ZF | HF | PVF | NF)

#define AALLF 0xff
#define ABUTCF 0xfe

#define SETFLAGS(mask, val) (RF = (RF & ~(mask)) | (val))
#define SET_FL(x)  (RF |= (x))
#define CLR_FL(x)  (RF &= ~(x))


#define TESTZF        (RF & ZF)
#define TESTCF        (RF & CF)
#define TESTSF        (RF & SF)
#define TESTPF        (RF & PVF)
#define TESTHF        (RF & HF)
#define TESTNF        (RF & NF)


#define IDXCALC(v1, v2, res) \
  ((res & 0xA8) | ((v1 & 0x88) >> 1) | ((v2 & 0x88) >> 3))

#define DIDXCALC(v1, v2, res) \
  (((res & 0x8800) >> 8) | ((v1 & 0x8800) >> 9) | ((v2 & 0x8800) >> 11))


extern byte z80c_incf_tbl[];
extern byte z80c_decf_tbl[];
extern byte z80c_addf_tbl[];
extern byte z80c_subf_tbl[];
extern byte z80c_orf_tbl[];

#define ADDSUB(r, op, tbl) \
{                                  \
  register byte res;               \
  register int idx;                \
  register int flag;               \
  res = RA op;                     \
  idx = IDXCALC(RA, r, res);       \
  RA = res;                        \
  flag = (RF & ~(AALLF)) |         \
         TAB(tbl)[idx];            \
  if(!res) flag |= ZF;             \
  RF = flag;			   \
}  


#define ADD(r) ADDSUB(r, + r, addf_tbl)
#define ADC(r) ADDSUB(r, + r + (RF & CF), addf_tbl) 
#define SUB(r) ADDSUB(r, - r, subf_tbl)
#define SBC(r) ADDSUB(r, - r - (RF & CF), subf_tbl)

#define AND(r) \
{                                  \
  register byte res;               \
  register byte flag;              \
  res = RA & r;                    \
  RA = res;                        \
  flag = (RF & ~(AALLF)) |          \
         TAB(orf_tbl)[res] | HF;   \
  RF = flag;                       \
}


#define XOR(r) \
{                                  \
  register byte res;               \
  register byte flag;              \
  res = RA ^ r;                    \
  RA = res;                        \
  flag = (RF & ~(AALLF)) |          \
         TAB(orf_tbl)[res];        \
  RF = flag;                       \
}


#define OR(r) \
{                                  \
  register byte res;               \
  register byte flag;              \
  res = RA | r;                    \
  RA = res;                        \
  flag = (RF & ~(AALLF)) |          \
         TAB(orf_tbl)[res];        \
  RF = flag;                       \
}


#define CP(r) \
{                                  \
  register byte res;               \
  register int idx;                \
  register int flag;               \
  res = RA - r;                    \
  idx = IDXCALC(RA, r, res);       \
  flag = (RF & ~(AALLF)) |          \
         TAB(subf_tbl)[idx];       \
  if(!res) flag |= ZF;             \
  RF = flag;			   \
}  

extern z80t z80op_nop(z80t);
extern z80t z80op_ld_bc_nn(z80t);
extern z80t z80op_ld_ibc_a(z80t);
extern z80t z80op_inc_bc(z80t);
extern z80t z80op_inc_b(z80t);
extern z80t z80op_dec_b(z80t);
extern z80t z80op_ld_b_n(z80t);
extern z80t z80op_rlca(z80t);
extern z80t z80op_ex_af_afb(z80t);
extern z80t z80op_add_hl_bc(z80t);
extern z80t z80op_ld_a_ibc(z80t);
extern z80t z80op_dec_bc(z80t);
extern z80t z80op_inc_c(z80t);
extern z80t z80op_dec_c(z80t);
extern z80t z80op_ld_c_n(z80t);
extern z80t z80op_rrca(z80t);

extern z80t z80op_djnz_e(z80t);
extern z80t z80op_ld_de_nn(z80t);
extern z80t z80op_ld_ide_a(z80t);
extern z80t z80op_inc_de(z80t);
extern z80t z80op_inc_d(z80t);
extern z80t z80op_dec_d(z80t);
extern z80t z80op_ld_d_n(z80t);
extern z80t z80op_rla(z80t);
extern z80t z80op_jr_e(z80t);
extern z80t z80op_add_hl_de(z80t);
extern z80t z80op_ld_a_ide(z80t);
extern z80t z80op_dec_de(z80t);
extern z80t z80op_inc_e(z80t);
extern z80t z80op_dec_e(z80t);
extern z80t z80op_ld_e_n(z80t);
extern z80t z80op_rra(z80t);

extern z80t z80op_jr_nz_e(z80t);
extern z80t z80op_ld_hl_nn(z80t);
extern z80t z80op_ld_inn_hl(z80t);
extern z80t z80op_inc_hl(z80t);
extern z80t z80op_inc_h(z80t);
extern z80t z80op_dec_h(z80t);
extern z80t z80op_ld_h_n(z80t);
extern z80t z80op_daa(z80t);
extern z80t z80op_jr_z_e(z80t);
extern z80t z80op_add_hl_hl(z80t);
extern z80t z80op_ld_hl_inn(z80t);
extern z80t z80op_dec_hl(z80t);
extern z80t z80op_inc_l(z80t);
extern z80t z80op_dec_l(z80t);
extern z80t z80op_ld_l_n(z80t);
extern z80t z80op_cpl(z80t);

extern z80t z80op_jr_nc_e(z80t);
extern z80t z80op_ld_sp_nn(z80t);
extern z80t z80op_ld_inn_a(z80t);
extern z80t z80op_inc_sp(z80t);
extern z80t z80op_inc_ihl(z80t);
extern z80t z80op_dec_ihl(z80t);
extern z80t z80op_ld_ihl_n(z80t);
extern z80t z80op_scf(z80t);
extern z80t z80op_jr_c_e(z80t);
extern z80t z80op_add_hl_sp(z80t);
extern z80t z80op_ld_a_inn(z80t);
extern z80t z80op_dec_sp(z80t);
extern z80t z80op_inc_a(z80t);
extern z80t z80op_dec_a(z80t);
extern z80t z80op_ld_a_n(z80t);
extern z80t z80op_ccf(z80t);

/* IX */

extern z80t z80op_add_ix_bc(z80t);
extern z80t z80op_add_ix_de(z80t);
extern z80t z80op_add_ix_ix(z80t);
extern z80t z80op_add_ix_sp(z80t);
extern z80t z80op_ld_ix_nn(z80t);
extern z80t z80op_ld_inn_ix(z80t);
extern z80t z80op_ld_ix_inn(z80t);
extern z80t z80op_inc_ix(z80t);
extern z80t z80op_dec_ix(z80t);
extern z80t z80op_inc_ixh(z80t);
extern z80t z80op_inc_ixl(z80t);
extern z80t z80op_inc_iixd(z80t);
extern z80t z80op_dec_ixh(z80t);
extern z80t z80op_dec_ixl(z80t);
extern z80t z80op_dec_iixd(z80t);
extern z80t z80op_ld_ixh_n(z80t);
extern z80t z80op_ld_ixl_n(z80t);
extern z80t z80op_ld_iixd_n(z80t);

/* IY */

extern z80t z80op_add_iy_bc(z80t);
extern z80t z80op_add_iy_de(z80t);
extern z80t z80op_add_iy_iy(z80t);
extern z80t z80op_add_iy_sp(z80t);
extern z80t z80op_ld_iy_nn(z80t);
extern z80t z80op_ld_inn_iy(z80t);
extern z80t z80op_ld_iy_inn(z80t);
extern z80t z80op_inc_iy(z80t);
extern z80t z80op_dec_iy(z80t);
extern z80t z80op_inc_iyh(z80t);
extern z80t z80op_inc_iyl(z80t);
extern z80t z80op_inc_iiyd(z80t);
extern z80t z80op_dec_iyh(z80t);
extern z80t z80op_dec_iyl(z80t);
extern z80t z80op_dec_iiyd(z80t);
extern z80t z80op_ld_iyh_n(z80t);
extern z80t z80op_ld_iyl_n(z80t);
extern z80t z80op_ld_iiyd_n(z80t);


/* extern z80t z80op_nop(z80t); */
extern z80t z80op_ld_b_c(z80t);
extern z80t z80op_ld_b_d(z80t);
extern z80t z80op_ld_b_e(z80t);
extern z80t z80op_ld_b_h(z80t);
extern z80t z80op_ld_b_l(z80t);
extern z80t z80op_ld_b_ihl(z80t);
extern z80t z80op_ld_b_a(z80t);
extern z80t z80op_ld_c_b(z80t);
/* extern z80t z80op_nop(z80t); */
extern z80t z80op_ld_c_d(z80t);
extern z80t z80op_ld_c_e(z80t);
extern z80t z80op_ld_c_h(z80t);
extern z80t z80op_ld_c_l(z80t);
extern z80t z80op_ld_c_ihl(z80t);
extern z80t z80op_ld_c_a(z80t);

extern z80t z80op_ld_d_b(z80t);
extern z80t z80op_ld_d_c(z80t);
/* extern z80t z80op_nop(z80t); */
extern z80t z80op_ld_d_e(z80t);
extern z80t z80op_ld_d_h(z80t);
extern z80t z80op_ld_d_l(z80t);
extern z80t z80op_ld_d_ihl(z80t);
extern z80t z80op_ld_d_a(z80t);
extern z80t z80op_ld_e_b(z80t);
extern z80t z80op_ld_e_c(z80t);
extern z80t z80op_ld_e_d(z80t);
/* extern z80t z80op_nop(z80t); */
extern z80t z80op_ld_e_h(z80t);
extern z80t z80op_ld_e_l(z80t);
extern z80t z80op_ld_e_ihl(z80t);
extern z80t z80op_ld_e_a(z80t);

extern z80t z80op_ld_h_b(z80t);
extern z80t z80op_ld_h_c(z80t);
extern z80t z80op_ld_h_d(z80t);
extern z80t z80op_ld_h_e(z80t);
/* extern z80t z80op_nop(z80t); */
extern z80t z80op_ld_h_l(z80t);
extern z80t z80op_ld_h_ihl(z80t);
extern z80t z80op_ld_h_a(z80t);
extern z80t z80op_ld_l_b(z80t);
extern z80t z80op_ld_l_c(z80t);
extern z80t z80op_ld_l_d(z80t);
extern z80t z80op_ld_l_e(z80t);
extern z80t z80op_ld_l_h(z80t);
/* extern z80t z80op_nop(z80t); */
extern z80t z80op_ld_l_ihl(z80t);
extern z80t z80op_ld_l_a(z80t);

extern z80t z80op_ld_ihl_b(z80t);
extern z80t z80op_ld_ihl_c(z80t);
extern z80t z80op_ld_ihl_d(z80t);
extern z80t z80op_ld_ihl_e(z80t);
extern z80t z80op_ld_ihl_h(z80t);
extern z80t z80op_ld_ihl_l(z80t);
extern z80t z80op_halt(z80t);
extern z80t z80op_ld_ihl_a(z80t);
extern z80t z80op_ld_a_b(z80t);
extern z80t z80op_ld_a_c(z80t);
extern z80t z80op_ld_a_d(z80t);
extern z80t z80op_ld_a_e(z80t);
extern z80t z80op_ld_a_h(z80t);
extern z80t z80op_ld_a_l(z80t);
extern z80t z80op_ld_a_ihl(z80t);
/* extern z80t z80op_nop(z80t); */


/* IX */

extern z80t z80op_ld_b_ixh(z80t);
extern z80t z80op_ld_b_ixl(z80t);

extern z80t z80op_ld_c_ixh(z80t);
extern z80t z80op_ld_c_ixl(z80t);

extern z80t z80op_ld_d_ixh(z80t);
extern z80t z80op_ld_d_ixl(z80t);

extern z80t z80op_ld_e_ixh(z80t);
extern z80t z80op_ld_e_ixl(z80t);

extern z80t z80op_ld_ixh_b(z80t);
extern z80t z80op_ld_ixh_c(z80t);
extern z80t z80op_ld_ixh_d(z80t);
extern z80t z80op_ld_ixh_e(z80t);
/* extern z80t z80op_ld_ixh_ixh(z80t); */
extern z80t z80op_ld_ixh_ixl(z80t);
extern z80t z80op_ld_ixh_a(z80t);


extern z80t z80op_ld_ixl_b(z80t);
extern z80t z80op_ld_ixl_c(z80t);
extern z80t z80op_ld_ixl_d(z80t);
extern z80t z80op_ld_ixl_e(z80t);
extern z80t z80op_ld_ixl_ixh(z80t);
/* extern z80t z80op_ld_ixl_ixl(z80t); */
extern z80t z80op_ld_ixl_a(z80t);

extern z80t z80op_ld_a_ixh(z80t);
extern z80t z80op_ld_a_ixl(z80t);

extern z80t z80op_ld_iixd_b(z80t);
extern z80t z80op_ld_iixd_c(z80t);
extern z80t z80op_ld_iixd_d(z80t);
extern z80t z80op_ld_iixd_e(z80t);
extern z80t z80op_ld_iixd_h(z80t);
extern z80t z80op_ld_iixd_l(z80t);
extern z80t z80op_ld_iixd_a(z80t);

extern z80t z80op_ld_b_iixd(z80t);
extern z80t z80op_ld_c_iixd(z80t);
extern z80t z80op_ld_d_iixd(z80t);
extern z80t z80op_ld_e_iixd(z80t);
extern z80t z80op_ld_h_iixd(z80t);
extern z80t z80op_ld_l_iixd(z80t);
extern z80t z80op_ld_a_iixd(z80t);

/* IY */

extern z80t z80op_ld_b_iyh(z80t);
extern z80t z80op_ld_b_iyl(z80t);

extern z80t z80op_ld_c_iyh(z80t);
extern z80t z80op_ld_c_iyl(z80t);

extern z80t z80op_ld_d_iyh(z80t);
extern z80t z80op_ld_d_iyl(z80t);

extern z80t z80op_ld_e_iyh(z80t);
extern z80t z80op_ld_e_iyl(z80t);

extern z80t z80op_ld_iyh_b(z80t);
extern z80t z80op_ld_iyh_c(z80t);
extern z80t z80op_ld_iyh_d(z80t);
extern z80t z80op_ld_iyh_e(z80t);
/* extern z80t z80op_ld_iyh_iyh(z80t); */
extern z80t z80op_ld_iyh_iyl(z80t);
extern z80t z80op_ld_iyh_a(z80t);


extern z80t z80op_ld_iyl_b(z80t);
extern z80t z80op_ld_iyl_c(z80t);
extern z80t z80op_ld_iyl_d(z80t);
extern z80t z80op_ld_iyl_e(z80t);
extern z80t z80op_ld_iyl_iyh(z80t);
/* extern z80t z80op_ld_iyl_iyl(z80t); */
extern z80t z80op_ld_iyl_a(z80t);

extern z80t z80op_ld_a_iyh(z80t);
extern z80t z80op_ld_a_iyl(z80t);

extern z80t z80op_ld_iiyd_b(z80t);
extern z80t z80op_ld_iiyd_c(z80t);
extern z80t z80op_ld_iiyd_d(z80t);
extern z80t z80op_ld_iiyd_e(z80t);
extern z80t z80op_ld_iiyd_h(z80t);
extern z80t z80op_ld_iiyd_l(z80t);
extern z80t z80op_ld_iiyd_a(z80t);

extern z80t z80op_ld_b_iiyd(z80t);
extern z80t z80op_ld_c_iiyd(z80t);
extern z80t z80op_ld_d_iiyd(z80t);
extern z80t z80op_ld_e_iiyd(z80t);
extern z80t z80op_ld_h_iiyd(z80t);
extern z80t z80op_ld_l_iiyd(z80t);
extern z80t z80op_ld_a_iiyd(z80t);

extern z80t z80op_add_a_b(z80t);
extern z80t z80op_add_a_c(z80t);
extern z80t z80op_add_a_d(z80t);
extern z80t z80op_add_a_e(z80t);
extern z80t z80op_add_a_h(z80t);
extern z80t z80op_add_a_l(z80t);
extern z80t z80op_add_a_ihl(z80t);
extern z80t z80op_add_a_a(z80t);
extern z80t z80op_adc_a_b(z80t);
extern z80t z80op_adc_a_c(z80t);
extern z80t z80op_adc_a_d(z80t);
extern z80t z80op_adc_a_e(z80t);
extern z80t z80op_adc_a_h(z80t);
extern z80t z80op_adc_a_l(z80t);
extern z80t z80op_adc_a_ihl(z80t);
extern z80t z80op_adc_a_a(z80t);

extern z80t z80op_sub_b(z80t);
extern z80t z80op_sub_c(z80t);
extern z80t z80op_sub_d(z80t);
extern z80t z80op_sub_e(z80t);
extern z80t z80op_sub_h(z80t);
extern z80t z80op_sub_l(z80t);
extern z80t z80op_sub_ihl(z80t);
extern z80t z80op_sub_a(z80t);
extern z80t z80op_sbc_a_b(z80t);
extern z80t z80op_sbc_a_c(z80t);
extern z80t z80op_sbc_a_d(z80t);
extern z80t z80op_sbc_a_e(z80t);
extern z80t z80op_sbc_a_h(z80t);
extern z80t z80op_sbc_a_l(z80t);
extern z80t z80op_sbc_a_ihl(z80t);
extern z80t z80op_sbc_a_a(z80t);

extern z80t z80op_and_b(z80t);
extern z80t z80op_and_c(z80t);
extern z80t z80op_and_d(z80t);
extern z80t z80op_and_e(z80t);
extern z80t z80op_and_h(z80t);
extern z80t z80op_and_l(z80t);
extern z80t z80op_and_ihl(z80t);
extern z80t z80op_and_a(z80t);
extern z80t z80op_xor_b(z80t);
extern z80t z80op_xor_c(z80t);
extern z80t z80op_xor_d(z80t);
extern z80t z80op_xor_e(z80t);
extern z80t z80op_xor_h(z80t);
extern z80t z80op_xor_l(z80t);
extern z80t z80op_xor_ihl(z80t);
extern z80t z80op_xor_a(z80t);

extern z80t z80op_or_b(z80t);
extern z80t z80op_or_c(z80t);
extern z80t z80op_or_d(z80t);
extern z80t z80op_or_e(z80t);
extern z80t z80op_or_h(z80t);
extern z80t z80op_or_l(z80t);
extern z80t z80op_or_ihl(z80t);
extern z80t z80op_or_a(z80t);
extern z80t z80op_cp_b(z80t);
extern z80t z80op_cp_c(z80t);
extern z80t z80op_cp_d(z80t);
extern z80t z80op_cp_e(z80t);
extern z80t z80op_cp_h(z80t);
extern z80t z80op_cp_l(z80t);
extern z80t z80op_cp_ihl(z80t);
extern z80t z80op_cp_a(z80t);


/* IX */

extern z80t z80op_add_a_ixh(z80t);
extern z80t z80op_add_a_ixl(z80t);
extern z80t z80op_add_a_iixd(z80t);

extern z80t z80op_adc_a_ixh(z80t);
extern z80t z80op_adc_a_ixl(z80t);
extern z80t z80op_adc_a_iixd(z80t);

extern z80t z80op_sub_ixh(z80t);
extern z80t z80op_sub_ixl(z80t);
extern z80t z80op_sub_iixd(z80t);

extern z80t z80op_sbc_a_ixh(z80t);
extern z80t z80op_sbc_a_ixl(z80t);
extern z80t z80op_sbc_a_iixd(z80t);

extern z80t z80op_and_ixh(z80t);
extern z80t z80op_and_ixl(z80t);
extern z80t z80op_and_iixd(z80t);

extern z80t z80op_xor_ixh(z80t);
extern z80t z80op_xor_ixl(z80t);
extern z80t z80op_xor_iixd(z80t);

extern z80t z80op_or_ixh(z80t);
extern z80t z80op_or_ixl(z80t);
extern z80t z80op_or_iixd(z80t);

extern z80t z80op_cp_ixh(z80t);
extern z80t z80op_cp_ixl(z80t);
extern z80t z80op_cp_iixd(z80t);


/* IY */

extern z80t z80op_add_a_iyh(z80t);
extern z80t z80op_add_a_iyl(z80t);
extern z80t z80op_add_a_iiyd(z80t);

extern z80t z80op_adc_a_iyh(z80t);
extern z80t z80op_adc_a_iyl(z80t);
extern z80t z80op_adc_a_iiyd(z80t);

extern z80t z80op_sub_iyh(z80t);
extern z80t z80op_sub_iyl(z80t);
extern z80t z80op_sub_iiyd(z80t);

extern z80t z80op_sbc_a_iyh(z80t);
extern z80t z80op_sbc_a_iyl(z80t);
extern z80t z80op_sbc_a_iiyd(z80t);

extern z80t z80op_and_iyh(z80t);
extern z80t z80op_and_iyl(z80t);
extern z80t z80op_and_iiyd(z80t);

extern z80t z80op_xor_iyh(z80t);
extern z80t z80op_xor_iyl(z80t);
extern z80t z80op_xor_iiyd(z80t);

extern z80t z80op_or_iyh(z80t);
extern z80t z80op_or_iyl(z80t);
extern z80t z80op_or_iiyd(z80t);

extern z80t z80op_cp_iyh(z80t);
extern z80t z80op_cp_iyl(z80t);
extern z80t z80op_cp_iiyd(z80t);

extern z80t z80op_ret_nz(z80t);
extern z80t z80op_pop_bc(z80t);
extern z80t z80op_jp_nz_nn(z80t);
extern z80t z80op_jp_nn(z80t);
extern z80t z80op_call_nz_nn(z80t);
extern z80t z80op_push_bc(z80t);
extern z80t z80op_add_a_n(z80t);
extern z80t z80op_rst_0(z80t);
extern z80t z80op_ret_z(z80t);
extern z80t z80op_ret(z80t);
extern z80t z80op_jp_z_nn(z80t);
extern z80t z80op_special_cb(z80t);
extern z80t z80op_call_z_nn(z80t);
extern z80t z80op_call_nn(z80t);
extern z80t z80op_adc_a_n(z80t);
extern z80t z80op_rst_8(z80t);

extern z80t z80op_ret_nc(z80t);
extern z80t z80op_pop_de(z80t);
extern z80t z80op_jp_nc_nn(z80t);
extern z80t z80op_out_in_a(z80t);
extern z80t z80op_call_nc_nn(z80t);
extern z80t z80op_push_de(z80t);
extern z80t z80op_sub_n(z80t);
extern z80t z80op_rst_10(z80t);
extern z80t z80op_ret_c(z80t);
extern z80t z80op_exx(z80t);
extern z80t z80op_jp_c_nn(z80t);
extern z80t z80op_in_a_in(z80t);
extern z80t z80op_call_c_nn(z80t);
extern z80t z80op_special_dd(z80t);
extern z80t z80op_sbc_a_n(z80t);
extern z80t z80op_rst_18(z80t);

extern z80t z80op_ret_po(z80t);
extern z80t z80op_pop_hl(z80t);
extern z80t z80op_jp_po_nn(z80t);
extern z80t z80op_ex_isp_hl(z80t);
extern z80t z80op_call_po_nn(z80t);
extern z80t z80op_push_hl(z80t);
extern z80t z80op_and_n(z80t);
extern z80t z80op_rst_20(z80t);
extern z80t z80op_ret_pe(z80t);
extern z80t z80op_jp_hl(z80t);
extern z80t z80op_jp_pe_nn(z80t);
extern z80t z80op_ex_de_hl(z80t);
extern z80t z80op_call_pe_nn(z80t);
extern z80t z80op_special_ed(z80t);
extern z80t z80op_xor_n(z80t);
extern z80t z80op_rst_28(z80t);

extern z80t z80op_ret_p(z80t);
extern z80t z80op_pop_af(z80t);
extern z80t z80op_jp_p_nn(z80t);
extern z80t z80op_di(z80t);
extern z80t z80op_call_p_nn(z80t);
extern z80t z80op_push_af(z80t);
extern z80t z80op_or_n(z80t);
extern z80t z80op_rst_30(z80t);
extern z80t z80op_ret_m(z80t);
extern z80t z80op_ld_sp_hl(z80t);
extern z80t z80op_jp_m_nn(z80t);
extern z80t z80op_ei(z80t);
extern z80t z80op_call_m_nn(z80t);
extern z80t z80op_special_fd(z80t);
extern z80t z80op_cp_n(z80t);
extern z80t z80op_rst_38(z80t);

/* IX */

extern z80t z80op_pop_ix(z80t);
extern z80t z80op_push_ix(z80t);

extern z80t z80op_jp_ix(z80t);
extern z80t z80op_ld_sp_ix(z80t);
extern z80t z80op_ex_isp_ix(z80t);

/* IY */

extern z80t z80op_pop_iy(z80t);
extern z80t z80op_push_iy(z80t);

extern z80t z80op_jp_iy(z80t);
extern z80t z80op_ld_sp_iy(z80t);
extern z80t z80op_ex_isp_iy(z80t);

extern z80t z80op_ill_ed(z80t);

extern z80t z80op_in_b_ic(z80t);
extern z80t z80op_out_ic_b(z80t);
extern z80t z80op_sbc_hl_bc(z80t);
extern z80t z80op_ld_inn_bc(z80t);
extern z80t z80op_neg(z80t);
extern z80t z80op_retn(z80t);
extern z80t z80op_im_0(z80t);
extern z80t z80op_ld_i_a(z80t);
extern z80t z80op_in_c_ic(z80t);
extern z80t z80op_out_ic_c(z80t);
extern z80t z80op_adc_hl_bc(z80t);
extern z80t z80op_ld_bc_inn(z80t);
/* extern z80t z80op_neg(z80t); */
extern z80t z80op_reti(z80t);
/*extern z80t z80op_im_0(z80t); */
extern z80t z80op_ld_r_a(z80t);

extern z80t z80op_in_d_ic(z80t);
extern z80t z80op_out_ic_d(z80t);
extern z80t z80op_sbc_hl_de(z80t);
extern z80t z80op_ld_inn_de(z80t);
/* extern z80t z80op_neg(z80t); */
/* extern z80t z80op_retn(z80t); */
extern z80t z80op_im_1(z80t);
extern z80t z80op_ld_a_i(z80t);
extern z80t z80op_in_e_ic(z80t);
extern z80t z80op_out_ic_e(z80t);
extern z80t z80op_adc_hl_de(z80t);
extern z80t z80op_ld_de_inn(z80t);
/* extern z80t z80op_neg(z80t); */
/* extern z80t z80op_retn(z80t); */
extern z80t z80op_im_2(z80t);
extern z80t z80op_ld_a_r(z80t);

extern z80t z80op_in_h_ic(z80t);
extern z80t z80op_out_ic_h(z80t);
extern z80t z80op_sbc_hl_hl(z80t);
extern z80t z80op_ld_inn_hl_ed(z80t);
/* extern z80t z80op_neg(z80t); */
/* extern z80t z80op_retn(z80t); */
/* extern z80t z80op_im_0(z80t); */
extern z80t z80op_rrd(z80t);
extern z80t z80op_in_l_ic(z80t);
extern z80t z80op_out_ic_l(z80t);
extern z80t z80op_adc_hl_hl(z80t);
extern z80t z80op_ld_hl_inn_ed(z80t);
/* extern z80t z80op_neg(z80t); */
/* extern z80t z80op_retn(z80t); */
/* extern z80t z80op_im_0(z80t); */
extern z80t z80op_rld(z80t);

extern z80t z80op_in_f_ic(z80t);
extern z80t z80op_out_ic_0(z80t);
extern z80t z80op_sbc_hl_sp(z80t);
extern z80t z80op_ld_inn_sp(z80t);
/* extern z80t z80op_neg(z80t); */
/* extern z80t z80op_retn(z80t); */
/* extern z80t z80op_im_1(z80t); */
/* extern z80t z80op_ill_ed(z80t); */
extern z80t z80op_in_a_ic(z80t);
extern z80t z80op_out_ic_a(z80t);
extern z80t z80op_adc_hl_sp(z80t);
extern z80t z80op_ld_sp_inn(z80t);
/* extern z80t z80op_neg(z80t); */
/* extern z80t z80op_retn(z80t); */
/* extern z80t z80op_im_2(z80t); */
/* extern z80t z80op_ill_ed(z80t); */

extern z80t z80op_ldi(z80t);
extern z80t z80op_cpi(z80t);
extern z80t z80op_ini(z80t);
extern z80t z80op_outi(z80t);

extern z80t z80op_ldd(z80t);
extern z80t z80op_cpd(z80t);
extern z80t z80op_ind(z80t);
extern z80t z80op_outd(z80t);

extern z80t z80op_ldir(z80t);
extern z80t z80op_cpir(z80t);
extern z80t z80op_inir(z80t);
extern z80t z80op_otir(z80t);

extern z80t z80op_lddr(z80t);
extern z80t z80op_cpdr(z80t);
extern z80t z80op_indr(z80t);
extern z80t z80op_otdr(z80t);

extern z80t z80op_rlc_b(z80t);
extern z80t z80op_rlc_c(z80t);
extern z80t z80op_rlc_d(z80t);
extern z80t z80op_rlc_e(z80t);
extern z80t z80op_rlc_h(z80t);
extern z80t z80op_rlc_l(z80t);
extern z80t z80op_rlc_ihl(z80t);
extern z80t z80op_rlc_a(z80t);
extern z80t z80op_rrc_b(z80t);
extern z80t z80op_rrc_c(z80t);
extern z80t z80op_rrc_d(z80t);
extern z80t z80op_rrc_e(z80t);
extern z80t z80op_rrc_h(z80t);
extern z80t z80op_rrc_l(z80t);
extern z80t z80op_rrc_ihl(z80t);
extern z80t z80op_rrc_a(z80t);

extern z80t z80op_rl_b(z80t);
extern z80t z80op_rl_c(z80t);
extern z80t z80op_rl_d(z80t);
extern z80t z80op_rl_e(z80t);
extern z80t z80op_rl_h(z80t);
extern z80t z80op_rl_l(z80t);
extern z80t z80op_rl_ihl(z80t);
extern z80t z80op_rl_a(z80t);
extern z80t z80op_rr_b(z80t);
extern z80t z80op_rr_c(z80t);
extern z80t z80op_rr_d(z80t);
extern z80t z80op_rr_e(z80t);
extern z80t z80op_rr_h(z80t);
extern z80t z80op_rr_l(z80t);
extern z80t z80op_rr_ihl(z80t);
extern z80t z80op_rr_a(z80t);

extern z80t z80op_sla_b(z80t);
extern z80t z80op_sla_c(z80t);
extern z80t z80op_sla_d(z80t);
extern z80t z80op_sla_e(z80t);
extern z80t z80op_sla_h(z80t);
extern z80t z80op_sla_l(z80t);
extern z80t z80op_sla_ihl(z80t);
extern z80t z80op_sla_a(z80t);
extern z80t z80op_sra_b(z80t);
extern z80t z80op_sra_c(z80t);
extern z80t z80op_sra_d(z80t);
extern z80t z80op_sra_e(z80t);
extern z80t z80op_sra_h(z80t);
extern z80t z80op_sra_l(z80t);
extern z80t z80op_sra_ihl(z80t);
extern z80t z80op_sra_a(z80t);

extern z80t z80op_sll_b(z80t);
extern z80t z80op_sll_c(z80t);
extern z80t z80op_sll_d(z80t);
extern z80t z80op_sll_e(z80t);
extern z80t z80op_sll_h(z80t);
extern z80t z80op_sll_l(z80t);
extern z80t z80op_sll_ihl(z80t);
extern z80t z80op_sll_a(z80t);
extern z80t z80op_srl_b(z80t);
extern z80t z80op_srl_c(z80t);
extern z80t z80op_srl_d(z80t);
extern z80t z80op_srl_e(z80t);
extern z80t z80op_srl_h(z80t);
extern z80t z80op_srl_l(z80t);
extern z80t z80op_srl_ihl(z80t);
extern z80t z80op_srl_a(z80t);

extern z80t z80op_bit_0_b(z80t);
extern z80t z80op_bit_0_c(z80t);
extern z80t z80op_bit_0_d(z80t);
extern z80t z80op_bit_0_e(z80t);
extern z80t z80op_bit_0_h(z80t);
extern z80t z80op_bit_0_l(z80t);
extern z80t z80op_bit_0_ihl(z80t);
extern z80t z80op_bit_0_a(z80t);
extern z80t z80op_bit_1_b(z80t);
extern z80t z80op_bit_1_c(z80t);
extern z80t z80op_bit_1_d(z80t);
extern z80t z80op_bit_1_e(z80t);
extern z80t z80op_bit_1_h(z80t);
extern z80t z80op_bit_1_l(z80t);
extern z80t z80op_bit_1_ihl(z80t);
extern z80t z80op_bit_1_a(z80t);

extern z80t z80op_bit_2_b(z80t);
extern z80t z80op_bit_2_c(z80t);
extern z80t z80op_bit_2_d(z80t);
extern z80t z80op_bit_2_e(z80t);
extern z80t z80op_bit_2_h(z80t);
extern z80t z80op_bit_2_l(z80t);
extern z80t z80op_bit_2_ihl(z80t);
extern z80t z80op_bit_2_a(z80t);
extern z80t z80op_bit_3_b(z80t);
extern z80t z80op_bit_3_c(z80t);
extern z80t z80op_bit_3_d(z80t);
extern z80t z80op_bit_3_e(z80t);
extern z80t z80op_bit_3_h(z80t);
extern z80t z80op_bit_3_l(z80t);
extern z80t z80op_bit_3_ihl(z80t);
extern z80t z80op_bit_3_a(z80t);

extern z80t z80op_bit_4_b(z80t);
extern z80t z80op_bit_4_c(z80t);
extern z80t z80op_bit_4_d(z80t);
extern z80t z80op_bit_4_e(z80t);
extern z80t z80op_bit_4_h(z80t);
extern z80t z80op_bit_4_l(z80t);
extern z80t z80op_bit_4_ihl(z80t);
extern z80t z80op_bit_4_a(z80t);
extern z80t z80op_bit_5_b(z80t);
extern z80t z80op_bit_5_c(z80t);
extern z80t z80op_bit_5_d(z80t);
extern z80t z80op_bit_5_e(z80t);
extern z80t z80op_bit_5_h(z80t);
extern z80t z80op_bit_5_l(z80t);
extern z80t z80op_bit_5_ihl(z80t);
extern z80t z80op_bit_5_a(z80t);

extern z80t z80op_bit_6_b(z80t);
extern z80t z80op_bit_6_c(z80t);
extern z80t z80op_bit_6_d(z80t);
extern z80t z80op_bit_6_e(z80t);
extern z80t z80op_bit_6_h(z80t);
extern z80t z80op_bit_6_l(z80t);
extern z80t z80op_bit_6_ihl(z80t);
extern z80t z80op_bit_6_a(z80t);
extern z80t z80op_bit_7_b(z80t);
extern z80t z80op_bit_7_c(z80t);
extern z80t z80op_bit_7_d(z80t);
extern z80t z80op_bit_7_e(z80t);
extern z80t z80op_bit_7_h(z80t);
extern z80t z80op_bit_7_l(z80t);
extern z80t z80op_bit_7_ihl(z80t);
extern z80t z80op_bit_7_a(z80t);

extern z80t z80op_res_0_b(z80t);
extern z80t z80op_res_0_c(z80t);
extern z80t z80op_res_0_d(z80t);
extern z80t z80op_res_0_e(z80t);
extern z80t z80op_res_0_h(z80t);
extern z80t z80op_res_0_l(z80t);
extern z80t z80op_res_0_ihl(z80t);
extern z80t z80op_res_0_a(z80t);
extern z80t z80op_res_1_b(z80t);
extern z80t z80op_res_1_c(z80t);
extern z80t z80op_res_1_d(z80t);
extern z80t z80op_res_1_e(z80t);
extern z80t z80op_res_1_h(z80t);
extern z80t z80op_res_1_l(z80t);
extern z80t z80op_res_1_ihl(z80t);
extern z80t z80op_res_1_a(z80t);

extern z80t z80op_res_2_b(z80t);
extern z80t z80op_res_2_c(z80t);
extern z80t z80op_res_2_d(z80t);
extern z80t z80op_res_2_e(z80t);
extern z80t z80op_res_2_h(z80t);
extern z80t z80op_res_2_l(z80t);
extern z80t z80op_res_2_ihl(z80t);
extern z80t z80op_res_2_a(z80t);
extern z80t z80op_res_3_b(z80t);
extern z80t z80op_res_3_c(z80t);
extern z80t z80op_res_3_d(z80t);
extern z80t z80op_res_3_e(z80t);
extern z80t z80op_res_3_h(z80t);
extern z80t z80op_res_3_l(z80t);
extern z80t z80op_res_3_ihl(z80t);
extern z80t z80op_res_3_a(z80t);

extern z80t z80op_res_4_b(z80t);
extern z80t z80op_res_4_c(z80t);
extern z80t z80op_res_4_d(z80t);
extern z80t z80op_res_4_e(z80t);
extern z80t z80op_res_4_h(z80t);
extern z80t z80op_res_4_l(z80t);
extern z80t z80op_res_4_ihl(z80t);
extern z80t z80op_res_4_a(z80t);
extern z80t z80op_res_5_b(z80t);
extern z80t z80op_res_5_c(z80t);
extern z80t z80op_res_5_d(z80t);
extern z80t z80op_res_5_e(z80t);
extern z80t z80op_res_5_h(z80t);
extern z80t z80op_res_5_l(z80t);
extern z80t z80op_res_5_ihl(z80t);
extern z80t z80op_res_5_a(z80t);

extern z80t z80op_res_6_b(z80t);
extern z80t z80op_res_6_c(z80t);
extern z80t z80op_res_6_d(z80t);
extern z80t z80op_res_6_e(z80t);
extern z80t z80op_res_6_h(z80t);
extern z80t z80op_res_6_l(z80t);
extern z80t z80op_res_6_ihl(z80t);
extern z80t z80op_res_6_a(z80t);
extern z80t z80op_res_7_b(z80t);
extern z80t z80op_res_7_c(z80t);
extern z80t z80op_res_7_d(z80t);
extern z80t z80op_res_7_e(z80t);
extern z80t z80op_res_7_h(z80t);
extern z80t z80op_res_7_l(z80t);
extern z80t z80op_res_7_ihl(z80t);
extern z80t z80op_res_7_a(z80t);

extern z80t z80op_set_0_b(z80t);
extern z80t z80op_set_0_c(z80t);
extern z80t z80op_set_0_d(z80t);
extern z80t z80op_set_0_e(z80t);
extern z80t z80op_set_0_h(z80t);
extern z80t z80op_set_0_l(z80t);
extern z80t z80op_set_0_ihl(z80t);
extern z80t z80op_set_0_a(z80t);
extern z80t z80op_set_1_b(z80t);
extern z80t z80op_set_1_c(z80t);
extern z80t z80op_set_1_d(z80t);
extern z80t z80op_set_1_e(z80t);
extern z80t z80op_set_1_h(z80t);
extern z80t z80op_set_1_l(z80t);
extern z80t z80op_set_1_ihl(z80t);
extern z80t z80op_set_1_a(z80t);

extern z80t z80op_set_2_b(z80t);
extern z80t z80op_set_2_c(z80t);
extern z80t z80op_set_2_d(z80t);
extern z80t z80op_set_2_e(z80t);
extern z80t z80op_set_2_h(z80t);
extern z80t z80op_set_2_l(z80t);
extern z80t z80op_set_2_ihl(z80t);
extern z80t z80op_set_2_a(z80t);
extern z80t z80op_set_3_b(z80t);
extern z80t z80op_set_3_c(z80t);
extern z80t z80op_set_3_d(z80t);
extern z80t z80op_set_3_e(z80t);
extern z80t z80op_set_3_h(z80t);
extern z80t z80op_set_3_l(z80t);
extern z80t z80op_set_3_ihl(z80t);
extern z80t z80op_set_3_a(z80t);

extern z80t z80op_set_4_b(z80t);
extern z80t z80op_set_4_c(z80t);
extern z80t z80op_set_4_d(z80t);
extern z80t z80op_set_4_e(z80t);
extern z80t z80op_set_4_h(z80t);
extern z80t z80op_set_4_l(z80t);
extern z80t z80op_set_4_ihl(z80t);
extern z80t z80op_set_4_a(z80t);
extern z80t z80op_set_5_b(z80t);
extern z80t z80op_set_5_c(z80t);
extern z80t z80op_set_5_d(z80t);
extern z80t z80op_set_5_e(z80t);
extern z80t z80op_set_5_h(z80t);
extern z80t z80op_set_5_l(z80t);
extern z80t z80op_set_5_ihl(z80t);
extern z80t z80op_set_5_a(z80t);

extern z80t z80op_set_6_b(z80t);
extern z80t z80op_set_6_c(z80t);
extern z80t z80op_set_6_d(z80t);
extern z80t z80op_set_6_e(z80t);
extern z80t z80op_set_6_h(z80t);
extern z80t z80op_set_6_l(z80t);
extern z80t z80op_set_6_ihl(z80t);
extern z80t z80op_set_6_a(z80t);
extern z80t z80op_set_7_b(z80t);
extern z80t z80op_set_7_c(z80t);
extern z80t z80op_set_7_d(z80t);
extern z80t z80op_set_7_e(z80t);
extern z80t z80op_set_7_h(z80t);
extern z80t z80op_set_7_l(z80t);
extern z80t z80op_set_7_ihl(z80t);
extern z80t z80op_set_7_a(z80t);

extern z80t z80op_special_xx(z80t);
extern z80t z80op_special_dd_cb(z80t);
extern z80t z80op_special_fd_cb(z80t);

#endif
