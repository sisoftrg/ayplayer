//(c)2003 sisoft\trg - AYplayer.
/* $Id: z80emu.S,v 1.9 2003/11/02 18:31:40 root Exp $ */

//original version of this file was taken from SpectEmu0.92 by Miklos Szeredi 
#ifndef EZ80

.text
	
#define NAME(s) s
#define ALIGN .align 4
#define TC  %edi

#define Z80(s) NAME(z80_ ## s)

#define NR 0
#define BR 36
#define OT 52

#define DAT(t, o) Z80(proc)+t+o

#define NUMDREGS 9

#define BBC DAT(BR,0)
#define BDE DAT(BR,4)
#define BHL DAT(BR,8)
#define BAF DAT(BR,12)
	
#define MBC DAT(NR,0)
#define MDE DAT(NR,4)
#define MHL DAT(NR,8)
#define MAF DAT(NR,12)
#define MIX DAT(NR,20)
#define MIY DAT(NR,24)
#define MPC DAT(NR,28)
#define MSP DAT(NR,32)

#define MC  DAT(NR,0)
#define MB  DAT(NR,1)
#define ME  DAT(NR,4)
#define MD  DAT(NR,5)
#define ML  DAT(NR,8)
#define MH  DAT(NR,9)
#define MF  DAT(NR,12)
#define MA  DAT(NR,13)
#define MR  DAT(NR,16)
#define MI  DAT(NR,17)

#define MIXL DAT(NR,20)
#define MIXH DAT(NR,21)
#define MIYL DAT(NR,24)
#define MIYH DAT(NR,25)

#define HALTSTATE DAT(OT,0)
#define IT_MODE   DAT(OT,4)
#define IFF1      DAT(OT,8)
#define IFF2      DAT(OT,12)
/* #define MEM       DAT(OT,16) */
/* #define REAL_TC   DAT(OT,20) */
#define RL7       DAT(OT,24)
#define RRR       DAT(OT,28)
#define RRV       DAT(OT,29)
		
#define RV %ah
#define RW %al
#define RVW %ax
#define VWP %eax
	
#define RA  %ch
#define RF  %cl
#define RAF %cx
#define SCRP %ecx

#define RB  %bh
#define RC  %bl
#define RBC %bx
#define BCP %ebx

#define RH  %dh
#define RL  %dl
#define RHL %dx
#define HLP %edx

#define RPC %si
#define PCP %esi

#define RSP %bp
#define SPP %ebp

#define INC_R incl RL7

#define DEC_R decl RL7

	
#define EOI(x) \
	INC_R			; \
	subl $ ## x,TC		; \
	jng loop_end

#define IBR \
	jmp *intr_table(,VWP,4)
	
#define TIME(x) \
	EOI(x)			; \
	FETCHL			; \
	IBR			; \
	ALIGN 

#define TIMEX(x) \
	EOI((x-4))              ; \
	FETCHL			; \
	IBR			; \
	ALIGN 
	
	
#define TIME_NC(x) \
	EOI(x)			; \
	FETCH			; \
	IBR			; \
	ALIGN
			
#define SF   128
#define ZF   64
#define HF   16
#define PVF  4
#define NF   2
#define CF   1

#define ALLF 0xD7
#define BUTCF 0xD6
#define SZHF 0xD0
#define SZF 0xC0
#define SZHCF 0xD1
#define SHCF 0x91
#define HNCF 0x13
#define HNF 0x12
#define ZNF 0x42
#define HPNF 0x16
#define HCF 0x11
#define BUTNF 0xD5
#define SZPF 0xC4
#define SZPCF 0xC5
#define ZPF  0x44
#define SZPNF 0xC6
#define SZNF 0xC2

#define VCFLAG(clear, change) \
	lahf			; \
	seto RW			; \
	andb $~ ## clear, RF	; \
	andb $ ## change, RV	; \
	orb RV, RF		; \
	shlb $2, RW		; \
	orb RW, RF

#define VSFLAG(set, change) \
	lahf			; \
	setno RW		; \
	orb $ ## set, RF	; \
	orb $~ ## change, RV	; \
	andb RV, RF		; \
	shlb $2, RW		; \
	notb RW			; \
	andb RW, RF	

#define ALLFLAG() \
	lahf			; \
	movb RV, RF
	
#define CFLAG(clear, change) \
	lahf			; \
	andb $~ ## clear, RF	; \
	andb $ ## change, RV	; \
	orb RV, RF

#define SFLAG(set, change) \
	lahf			; \
	orb $ ## set, RF	; \
	orb $~ ## change, RV	; \
	andb RV, RF

#define CSFLAG(clear, set, change) \
	lahf			; \
	andb $~ ## clear, RF	; \
	orb $ ## set, RF	; \
	andb $ ## change, RV	; \
	orb RV, RF
		
#define COND(fl) testb $ ## fl, RF

#define JCOND(fl, l) \
	COND(fl)		; \
	jnz l

#define JNCOND(fl, l) \
	COND(fl)		; \
	jz l
		

#define INCPC incw RPC
	
#define DINCPC \
	INCPC			; \
	INCPC
	
#define SFLAGS \
	movb RF, RV		; \
	sahf
			
#define FETCHR(ir) \
	movb (PCP), ir		; \
	INCPC

#define FETCHS \
	movsbw (PCP), RVW	; \
	INCPC

#define FETCHL \
	movzbl (PCP), VWP	; \
	INCPC
	
#define FETCH FETCHR(RW)
		
#define DFETCH \
	FETCHR(RW)		; \
	FETCHR(RV)

#define DFETCHP \
	movl HLP, VWP	; \
	DFETCH

#define PUTMEM_REG(l, reg, regh, regp, val, endf) \
	movb val, (regp)	; \
	endf
	
#define PUTMEM_REG_NE(l, reg, regh, regp, val) \
	movb val, (regp)
	
#define MODMEMF(l, func, endf) \
	func((HLP))		; \
	endf

#define PUSH(l, rh, rl) \
	decw RSP		; \
	movb rh, (SPP)		; \
	decw RSP		; \
	movb rl, (SPP)

#define PUSHT(l, rh, rl, t) \
	PUSH(l, rh, rl)		; \
	TIME(t)

#define IN(l, porth, portl, rd) \
	movzbl porth, VWP	; \
	movb Z80(inports)(VWP), rd

#define OUT(l, porth, portl, rs) \
	cmpb $0xfd, portl ;\
	jnz l ## _o3 ;\
	cmpb $0xff, porth ;\
	jnz l ## _o2 ;\
	movb rs, RRR ;\
	jmp l ## _o3 ;\
l ## _o2: ;\
	movb rs, RRV ;\
l ## _o3:

#define RSHLTIME(time) \
	RSHL			; \
	TIMEX(time)
	
#define MODMEM(func, time) \
	MODMEMF(func, func, TIME(time))
	
#define MODMEMID(l, func, time) \
	MODMEMF(func ## l, func, RSHLTIME(time))
	

#define PUTMEM_RVW(l, val, time) \
	PUTMEM_REG(l, RVW, RV, VWP, val, TIME(time))

#define PUTMEM_RHL(l, val, time) \
	PUTMEM_REG(l, RHL, RH, HLP, val, TIME(time))

#define PUTMEM_RBC(l, val, time) \
	PUTMEM_REG(l, RBC, RB, BCP, val, TIME(time))

#define PUTMEMID_RHL(l, val, time) \
	PUTMEM_REG(l, RHL, RH, HLP, val, RSHLTIME(time)) 
		
#define PUTMEM_RVW_NE(l, val) \
	PUTMEM_REG_NE(l, RVW, RV, VWP, val)

#define PUTMEM_RHL_NE(l, val) \
	PUTMEM_REG_NE(l, RHL, RH, HLP, val)


#define PUSHD(l, dr) \
	movw dr, RVW		; \
	PUSH(l, RV, RW)
	
#define POP(rh, rl) \
	movb (SPP), rl		; \
	incw RSP		; \
	movb (SPP), rh		; \
	incw RSP

#define POPD(dr) \
	POP(RV, RW)		; \
	movw RVW, dr

	
#define ADDFL VCFLAG(ALLF, SZHCF)
#define SUBFL VSFLAG(ALLF, SZHCF)
#define ORFL CFLAG(ALLF, SZPF)
#define ANDFL CSFLAG(ALLF, HF, SZPF)

#define ADDFLAGS ADDFL
#define SUBFLAGS SUBFL
#define ORFLAGS ORFL
#define ANDFLAGS ANDFL

#define IXDGET \
	pushl HLP		; \
	movw MIX, RHL		; \
	FETCHS			; \
	addw RVW, RHL		; \

#define IYDGET \
	pushl HLP		; \
	movw MIY, RHL		; \
	FETCHS			; \
	addw RVW, RHL		; \

#define RSHL \
	popl HLP
		
#define ADD_A_R(r) \
	addb r, RA		; \
	ADDFLAGS
	
#define ADC_A_R(r) \
	SFLAGS			; \
	adcb r, RA		; \
	ADDFLAGS

#define SUB_R(r) \
	subb r, RA		; \
	SUBFLAGS
	
#define SBC_A_R(r) \
	SFLAGS			; \
	sbbb r, RA		; \
	SUBFLAGS

#define AND_R(r) \
	andb r, RA		; \
	ANDFLAGS

#define XOR_R(r) \
	xorb r, RA		; \
	ORFLAGS
	
#define OR_R(r) \
	orb r, RA		; \
	ORFLAGS
	
#define CP_R(r)	\
	cmpb r, RA		; \
	SUBFLAGS
		

	ALIGN

.globl Z80(local_init)
/* .type    Z80(local_init),@function */
Z80(local_init):
/* Nothing to initialize */
	ret
				
	ALIGN

.globl Z80(pushpc)
/* .type    Z80(pushpc),@function */
Z80(pushpc):
	pushl SPP
	pushl PCP

	movl MPC, PCP
	movl MSP, SPP
	xorl SCRP, SCRP

	PUSHD(z80_pushpci, RPC)
	
	movl SPP, MSP
	
	popl PCP
	popl SPP

	ret

	ALIGN

.globl Z80(step)
/* .type    Z80(step),@function */
Z80(step):
	movl 4(%esp), %eax
	
	pushl %ebp
	pushl %edi
	pushl %esi
	pushl %ebx

	movl %eax, TC
	
	call load_regs
	call step_loop
	call store_regs

	movl TC, %eax	

	popl %ebx
	popl %esi
	popl %edi
	popl %ebp
	ret
	
	ALIGN
	
load_regs:
	movzwl MAF, SCRP
	movl MBC, BCP
	movl MHL, HLP
	movl MPC, PCP
	movl MSP, SPP

	movb MR, RW
	andb $0x80, MR
	movb RW, RL7
	ret
	ALIGN

store_regs:
	movw RAF, MAF
	movl BCP, MBC
	movl HLP, MHL
	movl PCP, MPC
	movl SPP, MSP

	movb RL7, RW
	andb $0x7F, RW
	orb RW, MR
	ret
	ALIGN
	
step_loop:
	cmpl $0, HALTSTATE
	jnz haltstate
	FETCHL
	IBR
	ALIGN
	
loop_end:
	ret
	ALIGN
	
special_cb:
	INC_R
	FETCHL
	jmp *intr_table_cb(,VWP,4)
	ALIGN

special_dd_cb:
	IXDGET
	FETCHL
	jmp *intr_table_dd_fd_cb(,VWP,4)
	ALIGN

special_fd_cb:
	IYDGET
	FETCHL
	jmp *intr_table_dd_fd_cb(,VWP,4)
	ALIGN	
	
special_dd:
	INC_R
	subl $4,TC
	FETCHL
	jmp *intr_table_dd(,VWP,4)
	ALIGN

	
special_ed:
	INC_R
	FETCHL
	jmp *intr_table_ed(,VWP,4)
	ALIGN

special_fd:
	INC_R
	subl $4,TC
	FETCHL
	jmp *intr_table_fd(,VWP,4)
	ALIGN

special_xx:
	DEC_R
	cmpl $0, TC
	jng loop_end
	IBR
	ALIGN
	
#define INC(m) \
	incb m			; \
	VCFLAG(SZPF, SZF)       /* Nagy Dani says N and H don't change */
/*	VCFLAG(BUTCF, SZHF) */  

#define DEC(m) \
	decb m			; \
	VCFLAG(SZPF, SZF)       /* Nagy Dani says N and H don't change */
/*	VSFLAG(BUTCF, SZHF) */

#define DADD(h1, l1, h2, l2) \
	addb l2, l1		; \
	adcb h2, h1		; \
	CFLAG(HNCF, HCF)


/* instructions from 0x00 to 0x3F */
		
nop:
	TIME_NC(4)
	
ex_af_afb:
	xchgw BAF, RAF
	TIME_NC(4)
	
djnz_e:
	decb RB
	jne djnz_e_1
	INCPC
	TIME_NC(8)
djnz_e_1:
	FETCHS
	addw RVW, RPC
	TIME(13)
	
jr_e:
	FETCHS
	addw RVW, RPC
	TIME(12)
	
jr_nz_e:
	JNCOND(ZF, jr_e)
	INCPC
	TIME_NC(7)
	
jr_z_e:
	JCOND(ZF, jr_e)
	INCPC
	TIME_NC(7)
	
jr_nc_e:
	JNCOND(CF, jr_e)
	INCPC
	TIME_NC(7)

jr_c_e:
	JCOND(CF, jr_e)
	INCPC
	TIME_NC(7)

ld_bc_nn:
	FETCHR(RC)
	FETCHR(RB)	
	TIME_NC(10)
	
ld_de_nn:
	DFETCH
	movw RVW, MDE
	TIME(10)
	
ld_hl_nn:
	FETCHR(RL)
	FETCHR(RH)
	TIME(10)

ld_sp_nn:
	DFETCH
	movw RVW, RSP
	TIME(10)

add_hl_bc:
	DADD(RH, RL, RB, RC)
	TIME(11)


add_hl_de:
	DADD(RH, RL, MD, ME)
	TIME(11)

add_hl_hl:
	DADD(RH, RL, RH, RL)
	TIME(11)

add_hl_sp:
	movw RSP, RVW
	DADD(RH, RL, RV, RW)
	TIME(11)

inc_bc:
	incw RBC
	TIME_NC(6)
	
inc_de:
	incw MDE
	TIME_NC(6)
	
inc_hl:
	incw RHL
	TIME_NC(6)

inc_sp:
	incw RSP
	TIME_NC(6)

dec_bc:
	decw RBC
	TIME_NC(6)
	
dec_de:
	decw MDE
	TIME_NC(6)
	
dec_hl:
	decw RHL
	TIME_NC(6)

dec_sp:
	decw RSP
	TIME_NC(6)

ld_ibc_a:
	PUTMEM_RBC(ld_ibc_a, RA, 7)
	
ld_ide_a:
	movl MDE, VWP
	PUTMEM_RVW(ld_ide_a, RA, 7)

ld_inn_hl:
	DFETCHP
	PUTMEM_RVW_NE(ld_inn_hl, RL)
	incw RVW
	PUTMEM_RVW(ld_inn_hl, RH, 16)

ld_inn_a:
	DFETCHP
	PUTMEM_RVW(ld_inn_a, RA, 13)
	
ld_a_ibc:
	movb (BCP), RA
	TIME_NC(7)

ld_a_ide:
	movl MDE, VWP
	movb (VWP), RA
	TIME(7)
			
ld_hl_inn:
	DFETCHP
	movb (VWP), RL
	incw RVW
	movb (VWP), RH
	TIME(16)

ld_a_inn:
	DFETCHP
	movb (VWP), RA
	TIME(13)

inc_b:	
	INC(RB)
	TIME(4)
	
inc_c:  
	INC(RC)
	TIME(4)
	
inc_d:	
	INC(MD)
	TIME(4)

inc_e:	
	INC(ME)
	TIME(4)
	
inc_h:	
	INC(RH)
	TIME(4)
		
inc_l:	
	INC(RL)
	TIME(4)

inc_ihl:
	MODMEM(INC, 11)
				
inc_a:	
	INC(RA)
	TIME(4)

		
dec_b:	
	DEC(RB)
	TIME(4)
	
dec_c:  
	DEC(RC)
	TIME(4)

dec_d:	
	DEC(MD)
	TIME(4)
	
dec_e:	
	DEC(ME)
	TIME(4)
	
dec_h:	
	DEC(RH)
	TIME(4)
	
dec_l:	
	DEC(RL)
	TIME(4)

dec_ihl:
	MODMEM(DEC, 11)

dec_a:	
	DEC(RA)
	TIME(4)
	
ld_b_n:
	FETCHR(RB)
	TIME(7)
	
ld_c_n:
	FETCHR(RC)
	TIME(7)
	
ld_d_n:
	FETCH
	movb RW, MD
	TIME(7)
	
ld_e_n:
	FETCH
	movb RW, ME
	TIME(7)
	
ld_h_n:
	FETCHR(RH)
	TIME(7)
	
ld_l_n:
	FETCHR(RL)
	TIME(7)
	
ld_ihl_n:
	FETCH
	PUTMEM_RHL(ld_ihl_n, RW, 10)

ld_a_n:
	FETCHR(RA)
	TIME_NC(7)
	

rlca:
	rolb RA
	CFLAG(HNCF, CF)
	TIME(4)
						
rrca:
	rorb RA
	CFLAG(HNCF, CF)
	TIME(4)
						
rla:
	SFLAGS
	rclb RA
	CFLAG(HNCF, CF)
	TIME(4)
						
rra:
	SFLAGS
	rcrb RA
	CFLAG(HNCF, CF)
	TIME(4)

	
#define DAAFLAGS \
	CFLAG(BUTNF, BUTNF) 
	
daa:
	movb RA, RW
	testb $NF, RF
	jnz daa_s
	SFLAGS
	daa
	movb RW, RA
	DAAFLAGS
	TIME(4)
daa_s:	
	SFLAGS
	das
	movb RW, RA
	DAAFLAGS
	TIME(4)
						
cpl:
	notb RA
	orb $HNF, RF	
	TIME_NC(4)
						
scf:
	andb $~HNF, RF
	orb $CF, RF
	TIME_NC(4)
						
ccf:
	xorb $CF, RF
	andb $~NF, RF
	TIME_NC(4)

/* IX */
	
ld_ix_nn:
	DFETCH
	movw RVW, MIX
	TIMEX(14)

add_ix_bc:
	xchgw RHL, MIX
	DADD(RH, RL, RB, RC)
	xchgw RHL, MIX
	TIMEX(15)

add_ix_de:
	xchgw RHL, MIX
	DADD(RH, RL, MD, ME)
	xchgw RHL, MIX
	TIMEX(15)

add_ix_ix:
	xchgw RHL, MIX
	DADD(RH, RL, RH, RL)
	xchgw RHL, MIX
	TIMEX(15)

add_ix_sp:
	movw RSP, RVW
	xchgw RHL, MIX
	DADD(RH, RL, RV, RW)
	xchgw RHL, MIX
	TIMEX(15)

inc_ix:
	incw MIX
	TIMEX(10)
	
dec_ix:
	decw MIX
	TIMEX(10)

ld_inn_ix:
	DFETCHP
	xchgw RHL, MIX
	PUTMEM_RVW_NE(ld_inn_ix1, RL)
	incw RVW
	PUTMEM_RVW_NE(ld_inn_ix2, RH)
	xchgw RHL, MIX
	TIMEX(20)

ld_ix_inn:
	DFETCHP
	xchgw RHL, MIX
	movb (VWP), RL
	incw RVW
	movb (VWP), RH
	xchgw RHL, MIX
	TIMEX(20)

inc_iixd:
	IXDGET
	MODMEMID(ix, INC, 23)
	
inc_ixh:	
	INC(MIXH)
	TIMEX(8)
	
inc_ixl:	
	INC(MIXL)
	TIMEX(8)

dec_iixd:
	IXDGET
	MODMEMID(ix, DEC, 23)

dec_ixh:	
	DEC(MIXH)
	TIMEX(8)
	
dec_ixl:	
	DEC(MIXL)
	TIMEX(8)

	
ld_iixd_n:
	IXDGET
	FETCH
	PUTMEMID_RHL(ld_iixd_n, RW, 19)

ld_ixh_n:
	FETCH
	movb RW, MIXH
	TIMEX(11)

ld_ixl_n:
	FETCH
	movb RW, MIXL
	TIMEX(11)
	



/* IY */

ld_iy_nn:
	DFETCH
	movw RVW, MIY
	TIMEX(14)

add_iy_bc:
	xchgw RHL, MIY
	DADD(RH, RL, RB, RC)
	xchgw RHL, MIY
	TIMEX(15)

add_iy_de:
	xchgw RHL, MIY
	DADD(RH, RL, MD, ME)
	xchgw RHL, MIY
	TIMEX(15)

add_iy_iy:
	xchgw RHL, MIY
	DADD(RH, RL, RH, RL)
	xchgw RHL, MIY
	TIMEX(15)

add_iy_sp:
	movw RSP, RVW
	xchgw RHL, MIY
	DADD(RH, RL, RV, RW)
	xchgw RHL, MIY
	TIMEX(15)

inc_iy:
	incw MIY
	TIMEX(10)
	
dec_iy:
	decw MIY
	TIMEX(10)

ld_inn_iy:
	DFETCHP
	xchgw RHL, MIY
	PUTMEM_RVW_NE(ld_inn_iy1, RL)
	incw RVW
	PUTMEM_RVW_NE(ld_inn_iy2, RH)
	xchgw RHL, MIY
	TIMEX(20)

ld_iy_inn:
	DFETCHP
	xchgw RHL, MIY
	movb (VWP), RL
	incw RVW
	movb (VWP), RH
	xchgw RHL, MIY
	TIMEX(20)

inc_iiyd:
	IYDGET
	MODMEMID(iy, INC, 23)
	
inc_iyh:	
	INC(MIYH)
	TIMEX(8)
	
inc_iyl:	
	INC(MIYL)
	TIMEX(8)

dec_iiyd:
	IYDGET
	MODMEMID(iy, DEC, 23)

dec_iyh:	
	DEC(MIYH)
	TIMEX(8)
	
dec_iyl:	
	DEC(MIYL)
	TIMEX(8)

	
ld_iiyd_n:
	IYDGET
	FETCH
	PUTMEMID_RHL(ld_iiyd_n, RW, 19)

ld_iyh_n:
	FETCH
	movb RW, MIYH
	TIMEX(11)

ld_iyl_n:
	FETCH
	movb RW, MIYL
	TIMEX(11)
	
/* instructions from 0x40 to 0x7F */

halt:
	movl $1, HALTSTATE
haltstate:
	movl TC, VWP
	testl VWP, VWP
	jz haltstate_tc0
	decl VWP
haltstate_tc0:
	sarl $2, VWP
	incl VWP
	addl VWP, RL7
	leal 0(,VWP,4), VWP
	subl VWP, TC
	jmp loop_end
	ALIGN
	
#define LD_R_R(rd, rs) \
	movb rs, rd		; \
	TIME(4)

#define LD_R_IHL(rd) \
	movb (HLP), rd		; \
	TIME(7)

#define LD_IHL_R(rs) \
	PUTMEM_RHL(ld_ihl_ ## rs, rs, 7)


ld_b_c: 
	LD_R_R(RB, RC)
ld_b_d: 
	LD_R_R(RB, MD)
ld_b_e: 
	LD_R_R(RB, ME)
ld_b_h: 
	LD_R_R(RB, RH)
ld_b_l: 
	LD_R_R(RB, RL)
ld_b_ihl:
	LD_R_IHL(RB)
ld_b_a: 
	LD_R_R(RB, RA)


ld_c_b: 
	LD_R_R(RC, RB)
ld_c_d: 
	LD_R_R(RC, MD)
ld_c_e: 
	LD_R_R(RC, ME)
ld_c_h: 
	LD_R_R(RC, RH)
ld_c_l: 
	LD_R_R(RC, RL)
ld_c_ihl:
	LD_R_IHL(RC)
ld_c_a: 
	LD_R_R(RC, RA)

ld_d_b: 
	LD_R_R(MD, RB)
ld_d_c: 
	LD_R_R(MD, RC)
ld_d_e:
	movb ME, RW
	LD_R_R(MD, RW)
ld_d_h: 
	LD_R_R(MD, RH)
ld_d_l: 
	LD_R_R(MD, RL)
ld_d_ihl:
	movb (HLP), RW
	movb RW, MD
	TIME(7)
ld_d_a: 
	LD_R_R(MD, RA)

	
ld_e_b: 
	LD_R_R(ME, RB)
ld_e_c: 
	LD_R_R(ME, RC)
ld_e_d: 
	movb MD, RW
	LD_R_R(ME, RW)
ld_e_h: 
	LD_R_R(ME, RH)
ld_e_l: 
	LD_R_R(ME, RL)
ld_e_ihl:
	movb (HLP), RW
	movb RW, ME
	TIME(7)
ld_e_a: 
	LD_R_R(ME, RA)


ld_h_b: 
	LD_R_R(RH, RB)
ld_h_c: 
	LD_R_R(RH, RC)
ld_h_d: 
	LD_R_R(RH, MD)
ld_h_e: 
	LD_R_R(RH, ME)
ld_h_l: 
	LD_R_R(RH, RL)
ld_h_ihl:
	LD_R_IHL(RH)
ld_h_a: 
	LD_R_R(RH, RA)


ld_l_b: 
	LD_R_R(RL, RB)
ld_l_c: 
	LD_R_R(RL, RC)
ld_l_d: 
	LD_R_R(RL, MD)
ld_l_e: 
	LD_R_R(RL, ME)
ld_l_h: 
	LD_R_R(RL, RH)
ld_l_ihl:
	LD_R_IHL(RL)
ld_l_a: 
	LD_R_R(RL, RA)


ld_ihl_b: 
	LD_IHL_R(RB)
ld_ihl_c: 
	LD_IHL_R(RC)
ld_ihl_d:
	movb MD, RV
	LD_IHL_R(RV)
ld_ihl_e:
	movb ME, RW
	LD_IHL_R(RW)
ld_ihl_h: 
	LD_IHL_R(RH)
ld_ihl_l: 
	LD_IHL_R(RL)
ld_ihl_a: 
	LD_IHL_R(RA)
	
ld_a_b: 
	LD_R_R(RA, RB)
ld_a_c: 
	LD_R_R(RA, RC)
ld_a_d: 
	LD_R_R(RA, MD)
ld_a_e: 
	LD_R_R(RA, ME)
ld_a_h: 
	LD_R_R(RA, RH)
ld_a_l: 
	LD_R_R(RA, RL)
ld_a_ihl:
	LD_R_IHL(RA)

/* IX */
	
#define LD_R_IIXD(rd) \
	movl HLP, VWP		; \
	FETCHS			; \
	addw MIX, RVW		; \
	movb (VWP), rd

#define LD_IIXD_R(rs) \
	movl HLP, VWP		; \
	FETCHS			; \
	addw MIX, RVW		; \
	PUTMEM_RVW(ld_iixd_ ## rs, rs, 15)

#define LD_IIXD_M(rs) \
	movl HLP, VWP		; \
	FETCHS			; \
	addw MIX, RVW		; \
	pushl HLP		; \
	movb rs, RL		; \
	PUTMEM_RVW_NE(ld_iixd_ ## rs, RL); \
	popl HLP		; \
	TIMEX(19)

ld_b_iixd:
	LD_R_IIXD(RB)
	TIMEX(19)

ld_b_ixh: 
	movb MIXH, RB
	TIMEX(8)

ld_b_ixl: 
	movb MIXL, RB
	TIMEX(8)


ld_c_iixd:
	LD_R_IIXD(RC)
	TIMEX(19)

ld_c_ixh: 
	movb MIXH, RC
	TIMEX(8)

ld_c_ixl: 
	movb MIXL, RC
	TIMEX(8)

ld_d_iixd:
	LD_R_IIXD(RW)
	movb RW, MD
	TIMEX(19)

ld_d_ixh: 
	movb MIXH, RW
	movb RW, MD
	TIMEX(8)

ld_d_ixl: 
	movb MIXL, RW
	movb RW, MD
	TIMEX(8)

ld_e_iixd:
	LD_R_IIXD(RW)
	movb RW, ME
	TIMEX(19)

ld_e_ixh: 
	movb MIXH, RW
	movb RW, ME
	TIMEX(8)

ld_e_ixl: 
	movb MIXL, RW
	movb RW, ME
	TIMEX(8)


ld_h_iixd:
	LD_R_IIXD(RH)
	TIMEX(19)

ld_ixh_b:
	movb RB, MIXH 
	TIMEX(8)
ld_ixh_c: 
	movb RC, MIXH
	TIMEX(8)
ld_ixh_d: 
	movb MD, RW
	movb RW, MIXH
	TIMEX(8)
ld_ixh_e: 
	movb ME, RW
	movb RW, MIXH
	TIMEX(8)
ld_ixh_ixl: 
	movb MIXL, RW
	movb RW, MIXH
	TIMEX(8)
ld_ixh_a: 
	movb RA, MIXH
	TIMEX(8)

ld_l_iixd:
	LD_R_IIXD(RL)
	TIMEX(19)

ld_ixl_b:
	movb RB, MIXL 
	TIMEX(8)
ld_ixl_c: 
	movb RC, MIXL
	TIMEX(8)
ld_ixl_d: 
	movb MD, RW
	movb RW, MIXL
	TIMEX(8)
ld_ixl_e: 
	movb ME, RW
	movb RW, MIXL
	TIMEX(8)
ld_ixl_ixh: 
	movb MIXH, RW
	movb RW, MIXL
	TIMEX(8)
ld_ixl_a: 
	movb RA, MIXL
	TIMEX(8)

ld_iixd_b: 
	LD_IIXD_R(RB)
ld_iixd_c: 
	LD_IIXD_R(RC)
ld_iixd_d:
	LD_IIXD_M(MD)
ld_iixd_e:
	LD_IIXD_M(ME)
ld_iixd_h: 
	LD_IIXD_R(RH)
ld_iixd_l: 
	LD_IIXD_R(RL)
ld_iixd_a: 
	LD_IIXD_R(RA)

ld_a_iixd:
	LD_R_IIXD(RA)
	TIMEX(19)

ld_a_ixh: 
	movb MIXH, RA
	TIMEX(8)

ld_a_ixl: 
	movb MIXL, RA
	TIMEX(8)

/* IY */
	
#define LD_R_IIYD(rd) \
	movl HLP, VWP		; \
	FETCHS			; \
	addw MIY, RVW		; \
	movb (VWP), rd

#define LD_IIYD_R(rs) \
	movl HLP, VWP		; \
	FETCHS			; \
	addw MIY, RVW		; \
	PUTMEM_RVW(ld_iiyd_ ## rs, rs, 15)

#define LD_IIYD_M(rs) \
	movl HLP, VWP		; \
	FETCHS			; \
	addw MIY, RVW		; \
	pushl HLP		; \
	movb rs, RL		; \
	PUTMEM_RVW_NE(ld_iiyd_ ## rs, RL); \
	popl HLP		; \
	TIMEX(19)

ld_b_iiyd:
	LD_R_IIYD(RB)
	TIMEX(19)

ld_b_iyh: 
	movb MIYH, RB
	TIMEX(8)

ld_b_iyl: 
	movb MIYL, RB
	TIMEX(8)


ld_c_iiyd:
	LD_R_IIYD(RC)
	TIMEX(19)

ld_c_iyh: 
	movb MIYH, RC
	TIMEX(8)

ld_c_iyl: 
	movb MIYL, RC
	TIMEX(8)

ld_d_iiyd:
	LD_R_IIYD(RW)
	movb RW, MD
	TIMEX(19)

ld_d_iyh: 
	movb MIYH, RW
	movb RW, MD
	TIMEX(8)

ld_d_iyl: 
	movb MIYL, RW
	movb RW, MD
	TIMEX(8)

ld_e_iiyd:
	LD_R_IIYD(RW)
	movb RW, ME
	TIMEX(19)

ld_e_iyh: 
	movb MIYH, RW
	movb RW, ME
	TIMEX(8)

ld_e_iyl: 
	movb MIYL, RW
	movb RW, ME
	TIMEX(8)


ld_h_iiyd:
	LD_R_IIYD(RH)
	TIMEX(19)

ld_iyh_b:
	movb RB, MIYH 
	TIMEX(8)
ld_iyh_c: 
	movb RC, MIYH
	TIMEX(8)
ld_iyh_d: 
	movb MD, RW
	movb RW, MIYH
	TIMEX(8)
ld_iyh_e: 
	movb ME, RW
	movb RW, MIYH
	TIMEX(8)
ld_iyh_iyl: 
	movb MIYL, RW
	movb RW, MIYH
	TIMEX(8)
ld_iyh_a: 
	movb RA, MIYH
	TIMEX(8)

ld_l_iiyd:
	LD_R_IIYD(RL)
	TIMEX(19)

ld_iyl_b:
	movb RB, MIYL 
	TIMEX(8)
ld_iyl_c: 
	movb RC, MIYL
	TIMEX(8)
ld_iyl_d: 
	movb MD, RW
	movb RW, MIYL
	TIMEX(8)
ld_iyl_e: 
	movb ME, RW
	movb RW, MIYL
	TIMEX(8)
ld_iyl_iyh: 
	movb MIYH, RW
	movb RW, MIYL
	TIMEX(8)
ld_iyl_a: 
	movb RA, MIYL
	TIMEX(8)

ld_iiyd_b: 
	LD_IIYD_R(RB)
ld_iiyd_c: 
	LD_IIYD_R(RC)
ld_iiyd_d:
	LD_IIYD_M(MD)
ld_iiyd_e:
	LD_IIYD_M(ME)
ld_iiyd_h: 
	LD_IIYD_R(RH)
ld_iiyd_l: 
	LD_IIYD_R(RL)
ld_iiyd_a: 
	LD_IIYD_R(RA)

ld_a_iiyd:
	LD_R_IIYD(RA)
	TIMEX(19)

ld_a_iyh: 
	movb MIYH, RA
	TIMEX(8)

ld_a_iyl: 
	movb MIYL, RA
	TIMEX(8)

/* instructions from 0x80 to 0xBF */

add_a_b:
	ADD_A_R(RB)
	TIME(4)

add_a_c:
	ADD_A_R(RC)
	TIME(4)

add_a_d:
	ADD_A_R(MD)
	TIME(4)

add_a_e:
	ADD_A_R(ME)
	TIME(4)

add_a_h:
	ADD_A_R(RH)
	TIME(4)

add_a_l:
	ADD_A_R(RL)
	TIME(4)

add_a_ihl:
	ADD_A_R((HLP))
	TIME(7)

add_a_a:
	ADD_A_R(RA)
	TIME(4)



adc_a_b:
	ADC_A_R(RB)
	TIME(4)

adc_a_c:
	ADC_A_R(RC)
	TIME(4)

adc_a_d:
	ADC_A_R(MD)
	TIME(4)

adc_a_e:
	ADC_A_R(ME)
	TIME(4)

adc_a_h:
	ADC_A_R(RH)
	TIME(4)

adc_a_l:
	ADC_A_R(RL)
	TIME(4)

adc_a_ihl:
	ADC_A_R((HLP))
	TIME(7)
	
adc_a_a:
	ADC_A_R(RA)
	TIME(4)



sub_b:
	SUB_R(RB)
	TIME(4)

sub_c:
	SUB_R(RC)
	TIME(4)

sub_d:
	SUB_R(MD)
	TIME(4)

sub_e:
	SUB_R(ME)
	TIME(4)

sub_h:
	SUB_R(RH)
	TIME(4)

sub_l:
	SUB_R(RL)
	TIME(4)

sub_ihl:
	SUB_R((HLP))
	TIME(7)

sub_a:
	SUB_R(RA)
	TIME(4)


sbc_a_b:
	SBC_A_R(RB)
	TIME(4)

sbc_a_c:
	SBC_A_R(RC)
	TIME(4)

sbc_a_d:
	SBC_A_R(MD)
	TIME(4)

sbc_a_e:
	SBC_A_R(ME)
	TIME(4)

sbc_a_h:
	SBC_A_R(RH)
	TIME(4)

sbc_a_l:
	SBC_A_R(RL)
	TIME(4)

sbc_a_ihl:
	SBC_A_R((HLP))
	TIME(7)

sbc_a_a:
	SBC_A_R(RA)
	TIME(4)


and_b:
	AND_R(RB)
	TIME(4)

and_c:
	AND_R(RC)
	TIME(4)

and_d:
	AND_R(MD)
	TIME(4)

and_e:
	AND_R(ME)
	TIME(4)

and_h:
	AND_R(RH)
	TIME(4)

and_l:
	AND_R(RL)
	TIME(4)

and_ihl:
	AND_R((HLP))
	TIME(7)

and_a:
	AND_R(RA)
	TIME(4)


xor_b:
	XOR_R(RB)
	TIME(4)

xor_c:
	XOR_R(RC)
	TIME(4)

xor_d:
	XOR_R(MD)
	TIME(4)

xor_e:
	XOR_R(ME)
	TIME(4)

xor_h:
	XOR_R(RH)
	TIME(4)

xor_l:
	XOR_R(RL)
	TIME(4)

xor_ihl:
	XOR_R((HLP))
	TIME(7)

xor_a:
	xorb RA, RA
	andb $~ALLF, RF
	orb $ZPF, RF
	TIME(4)


or_b:
	OR_R(RB)
	TIME(4)

or_c:
	OR_R(RC)
	TIME(4)

or_d:
	OR_R(MD)
	TIME(4)

or_e:
	OR_R(ME)
	TIME(4)

or_h:
	OR_R(RH)
	TIME(4)

or_l:
	OR_R(RL)
	TIME(4)

or_ihl:
	OR_R((HLP))
	TIME(7)

or_a:
	OR_R(RA)
	TIME(4)


cp_b:
	CP_R(RB)
	TIME(4)

cp_c:
	CP_R(RC)
	TIME(4)

cp_d:
	CP_R(MD)
	TIME(4)

cp_e:
	CP_R(ME)
	TIME(4)

cp_h:
	CP_R(RH)
	TIME(4)

cp_l:
	CP_R(RL)
	TIME(4)

cp_ihl:
	CP_R((HLP))
	TIME(7)

cp_a:
	CP_R(RA)
	TIME(4)

/* IX */

add_a_iixd:
	IXDGET
	ADD_A_R((HLP))
	RSHL
	TIMEX(19)

add_a_ixh:
	ADD_A_R(MIXH)
	TIMEX(8)

add_a_ixl:
	ADD_A_R(MIXL)
	TIMEX(8)

adc_a_iixd:
	IXDGET
	ADC_A_R((HLP))
	RSHL
	TIMEX(19)

adc_a_ixh:
	ADC_A_R(MIXH)
	TIMEX(8)

adc_a_ixl:
	ADC_A_R(MIXL)
	TIMEX(8)

sub_iixd:
	IXDGET
	SUB_R((HLP))
	RSHL
	TIMEX(19)

sub_ixh:
	SUB_R(MIXH)
	TIMEX(8)

sub_ixl:
	SUB_R(MIXL)
	TIMEX(8)


sbc_a_iixd:
	IXDGET
	SBC_A_R((HLP))
	RSHL
	TIMEX(19)

sbc_a_ixh:
	SBC_A_R(MIXH)
	TIMEX(8)

sbc_a_ixl:
	SBC_A_R(MIXL)
	TIMEX(8)


and_iixd:
	IXDGET
	AND_R((HLP))
	RSHL
	TIMEX(19)

and_ixh:
	AND_R(MIXH)
	TIMEX(8)

and_ixl:
	AND_R(MIXL)
	TIMEX(8)

xor_iixd:
	IXDGET
	XOR_R((HLP))
	RSHL
	TIMEX(19)

xor_ixh:
	XOR_R(MIXH)
	TIMEX(8)

xor_ixl:
	XOR_R(MIXL)
	TIMEX(8)

or_iixd:
	IXDGET
	OR_R((HLP))
	RSHL
	TIMEX(19)

or_ixh:
	OR_R(MIXH)
	TIMEX(8)

or_ixl:
	OR_R(MIXL)
	TIMEX(8)

cp_iixd:
	IXDGET
	CP_R((HLP))
	RSHL
	TIMEX(19)

cp_ixh:
	CP_R(MIXH)
	TIMEX(8)

cp_ixl:
	CP_R(MIXL)
	TIMEX(8)


/* IY */

add_a_iiyd:
	IYDGET
	ADD_A_R((HLP))
	RSHL
	TIMEX(19)

add_a_iyh:
	ADD_A_R(MIYH)
	TIMEX(8)

add_a_iyl:
	ADD_A_R(MIYL)
	TIMEX(8)

adc_a_iiyd:
	IYDGET
	ADC_A_R((HLP))
	RSHL
	TIMEX(19)

adc_a_iyh:
	ADC_A_R(MIYH)
	TIMEX(8)

adc_a_iyl:
	ADC_A_R(MIYL)
	TIMEX(8)

sub_iiyd:
	IYDGET
	SUB_R((HLP))
	RSHL
	TIMEX(19)

sub_iyh:
	SUB_R(MIYH)
	TIMEX(8)

sub_iyl:
	SUB_R(MIYL)
	TIMEX(8)


sbc_a_iiyd:
	IYDGET
	SBC_A_R((HLP))
	RSHL
	TIMEX(19)

sbc_a_iyh:
	SBC_A_R(MIYH)
	TIMEX(8)

sbc_a_iyl:
	SBC_A_R(MIYL)
	TIMEX(8)


and_iiyd:
	IYDGET
	AND_R((HLP))
	RSHL
	TIMEX(19)

and_iyh:
	AND_R(MIYH)
	TIMEX(8)

and_iyl:
	AND_R(MIYL)
	TIMEX(8)

xor_iiyd:
	IYDGET
	XOR_R((HLP))
	RSHL
	TIMEX(19)

xor_iyh:
	XOR_R(MIYH)
	TIMEX(8)

xor_iyl:
	XOR_R(MIYL)
	TIMEX(8)

or_iiyd:
	IYDGET
	OR_R((HLP))
	RSHL
	TIMEX(19)

or_iyh:
	OR_R(MIYH)
	TIMEX(8)

or_iyl:
	OR_R(MIYL)
	TIMEX(8)

cp_iiyd:
	IYDGET
	CP_R((HLP))
	RSHL
	TIMEX(19)

cp_iyh:
	CP_R(MIYH)
	TIMEX(8)

cp_iyl:
	CP_R(MIYL)
	TIMEX(8)

/* instructions from 0xC0 to 0xFF */

ret_cc:
	POPD(RPC)
	TIME(11)

ret:
	POPD(RPC)
	TIME(10)

ret_nz:
	JNCOND(ZF, ret_cc)
	TIME(5)

ret_z:
	JCOND(ZF, ret_cc)
	TIME(5)

ret_nc:
	JNCOND(CF, ret_cc)
	TIME(5)

ret_c:
	JCOND(CF, ret_cc)
	TIME(5)

ret_po:
	JNCOND(PVF, ret_cc)
	TIME(5)

ret_pe:
	JCOND(PVF, ret_cc)
	TIME(5)

ret_p:
	JNCOND(SF, ret_cc)
	TIME(5)

ret_m:
	JCOND(SF, ret_cc)
	TIME(5)

pop_bc:
	POP(RB, RC)
	TIME(10)

pop_de:
	POPD(MDE)
	TIME(10)
	
pop_hl:
	POP(RH, RL)
	TIME(10)

pop_ix:
	POPD(MIX)
	TIMEX(14)
	
pop_iy:
	POPD(MIY)
	TIMEX(14)
	
pop_af:	
	POP(RA, RF)
	TIME(10)

exx:
	xchgw BBC, RBC
	movw MDE, RVW
	xchgw BDE, RVW
	movw RVW, MDE
	xchgw BHL, RHL
	TIME(4)

jp_hl:	
	movw RHL, RPC
	TIME(4)

jp_ix:	
	movw MIX, RPC
	TIMEX(8)

jp_iy:	
	movw MIY, RPC
	TIMEX(8)

	
ld_sp_hl:
	movw RHL, RSP
	TIME(6)

ld_sp_ix:
	movw MIX, RSP
	TIMEX(10)

ld_sp_iy:
	movw MIY, RSP
	TIMEX(10)

jp_cc_nn:
jp_nn:
	DFETCH
	movw RVW, RPC
	TIME(10)

jp_nz_nn:
	JNCOND(ZF, jp_cc_nn)
	DINCPC
	TIME(10)

jp_z_nn:
	JCOND(ZF, jp_cc_nn)
	DINCPC
	TIME(10)

jp_nc_nn:
	JNCOND(CF, jp_cc_nn)
	DINCPC
	TIME(10)

jp_c_nn:
	JCOND(CF, jp_cc_nn)
	DINCPC
	TIME(10)

jp_po_nn:
	JNCOND(PVF, jp_cc_nn)
	DINCPC
	TIME(10)

jp_pe_nn:
	JCOND(PVF, jp_cc_nn)
	DINCPC
	TIME(10)

jp_p_nn:
	JNCOND(SF, jp_cc_nn)
	DINCPC
	TIME(10)

jp_m_nn:
	JCOND(SF, jp_cc_nn)
	DINCPC
	TIME(10)


out_in_a:
	OUT(out_in_a, RA, (PCP), RA)
	INCPC
	TIME(11)

in_a_in:
	IN(in_a_in, RA, (PCP), RA)
	INCPC
	TIME(11)

ex_isp_hl:
	POP(RV, RW)
	PUSH(ex_isp_hl, RH, RL)
	movw RVW, RHL
	TIME(19)

ex_isp_ix:
	POP(RV, RW)
	xchgw MIX, RHL
	PUSH(ex_isp_ix, RH, RL)
	movw RVW, RHL
	xchgw MIX, RHL	
	TIMEX(23)

ex_isp_iy:
	POP(RV, RW)
	xchgw MIY, RHL
	PUSH(ex_isp_iy, RH, RL)
	movw RVW, RHL
	xchgw MIY, RHL	
	TIMEX(23)
	
ex_de_hl:
	xchgw MDE, RHL
	TIME(4)

di:
//	movw $0, IFF1
//	movw $0, IFF2
	TIME(4)

ei:	
//	movw $1, IFF1
//	movw $1, IFF2
	TIME(4)

call_cc_nn:
call_nn:
	DFETCH
	xchgw RPC, RHL
	PUSH(call_nn, RH, RL)
	xchgw RPC, RHL
	movw RVW, RPC
	TIME(17)

call_nz_nn:
	JNCOND(ZF, call_cc_nn)
	DINCPC
	TIME(10)

call_z_nn:
	JCOND(ZF, call_cc_nn)
	DINCPC
	TIME(10)

call_nc_nn:
	JNCOND(CF, call_cc_nn)
	DINCPC
	TIME(10)

call_c_nn:
	JCOND(CF, call_cc_nn)
	DINCPC
	TIME(10)

call_po_nn:
	JNCOND(PVF, call_cc_nn)
	DINCPC
	TIME(10)

call_pe_nn:
	JCOND(PVF, call_cc_nn)
	DINCPC
	TIME(10)

call_p_nn:
	JNCOND(SF, call_cc_nn)
	DINCPC
	TIME(10)

call_m_nn:
	JCOND(SF, call_cc_nn)
	DINCPC
	TIME(10)

	
push_bc:
	PUSH(push_bc, RB, RC)
	TIME(11)

push_de:
	PUSHD(push_de, MDE)
	TIME(11)
	
push_hl:
	PUSH(push_hl, RH, RL)
	TIME(11)

push_ix:
	movw MIX, RVW
	PUSH(push_ix, RV, RW)
	TIMEX(15)

push_iy:
	movw MIY, RVW
	PUSH(push_iy, RV, RW)
	TIMEX(15)

	
push_af:	
	PUSH(push_af, RA, RF)
	TIME(11)


add_a_n:
	ADD_A_R((PCP))
	INCPC
	TIME(7)

adc_a_n:
	ADC_A_R((PCP))
	INCPC
	TIME(7)

sub_n:
	SUB_R((PCP))
	INCPC
	TIME(7)

sbc_a_n:
	SBC_A_R((PCP))
	INCPC
	TIME(7)

and_n:
	AND_R((PCP))
	INCPC
	TIME(7)

xor_n:
	XOR_R((PCP))
	INCPC
	TIME(7)

or_n:
	OR_R((PCP))
	INCPC
	TIME(7)

cp_n:
	CP_R((PCP))
	INCPC
	TIME(7)

	
#define RST_NN(n) \
	movw RPC, RVW		; \
	movw $0x ## n, RPC	; \
	jmp rst_nn		; \
	ALIGN
	
rst_nn:
	PUSH(rst_nn, RV, RW)
	TIME(11)
		
rst_0:
	RST_NN(0)

rst_8:
	RST_NN(8)

rst_10:
	RST_NN(10)

rst_18:
	RST_NN(18)

rst_20:
	RST_NN(20)

rst_28:
	RST_NN(28)

rst_30:
	RST_NN(30)

rst_38:
	RST_NN(38)

/* instructions prefixed by ED */

#define IN_R_IC(rd) \
	IN(in_ ## rd ## _ic, RB, RC, rd); \
	testb rd, rd		; \
	CFLAG(BUTCF, SZPF)	; \
	TIME(12)
	
#define IN_M_IC(rd) \
	xchgb rd, RA		; \
	IN(in_ ## rd ## _ic, RB, RC, RA); \
	testb RA, RA		; \
	CFLAG(BUTCF, SZPF)	; \
	xchgb rd, RA		; \
	TIME(12)

in_b_ic:
	IN_R_IC(RB)

in_c_ic:
	IN_R_IC(RC)

in_d_ic:
	IN_M_IC(MD)

in_e_ic:
	IN_M_IC(ME)

in_h_ic:
	IN_R_IC(RH)

in_l_ic:
	IN_R_IC(RL)

in_f_ic:
	IN_R_IC(RW) /* No load in registers */

in_a_ic:
	IN_R_IC(RA)


	
#define OUT_IC_R(rs) \
	OUT(out_ic_ ## rs, RB, RC, rs); \
	TIME(12)

#define OUT_IC_M(rs) \
	xchgb rs, RA		; \
	OUT(out_ic_ ## rs, RB, RC, RA); \
	xchgb rs, RA		; \
	TIME(12)

	
out_ic_b:
	OUT_IC_R(RB)

out_ic_c:
	OUT_IC_R(RC)

out_ic_d:
	OUT_IC_M(MD)

out_ic_e:
	OUT_IC_M(ME)

out_ic_h:
	OUT_IC_R(RH)

out_ic_l:
	OUT_IC_R(RL)

out_ic_0:
	xorb RW,RW
	OUT_IC_R(RW)

out_ic_a:
	OUT_IC_R(RA)


#define SBC_HL_RR(rh, rl) \
	sbbb rl, RL		; \
	sbbb rh, RH		; \
	pushfl			; \
	testw RHL, RHL		; \
	SFLAG(ALLF, ZF)		; \
	popfl			; \
	VSFLAG(SHCF, SHCF)	; \
	TIME(15)
	
sbc_hl_bc:
	SFLAGS
	SBC_HL_RR(RB, RC)

sbc_hl_de:
	SFLAGS
	SBC_HL_RR(MD, ME)

sbc_hl_hl:
	SFLAGS
	SBC_HL_RR(RH, RL)

sbc_hl_sp:
	SFLAGS
	movw RSP, RVW
	SBC_HL_RR(RV, RW)



#define ADC_HL_RR(rh, rl) \
	adcb rl, RL		; \
	adcb rh, RH		; \
	pushfl			; \
	testw RHL, RHL		; \
	CFLAG(ALLF, ZF)		; \
	popfl			; \
	VCFLAG(SHCF, SHCF)	; \
	TIME(15)

adc_hl_bc:
	SFLAGS
	ADC_HL_RR(RB, RC)

adc_hl_de:
	SFLAGS
	ADC_HL_RR(MD, ME)

adc_hl_hl:
	SFLAGS
	ADC_HL_RR(RH, RL)

adc_hl_sp:
	movw RSP, RVW
	SFLAGS
	ADC_HL_RR(RV, RW)


#define LD_INN_RR(rn, rh, rl) \
	DFETCHP			; \
	PUTMEM_RVW_NE(ld_inn1_ ## rn, rl); \
	incw RVW		; \
	PUTMEM_RVW_NE(ld_inn2_ ## rn, rh)

ld_inn_bc:
	LD_INN_RR(bc, RB, RC)
	TIME(20)

ld_inn_de:
	xchgw MDE, RHL
	LD_INN_RR(de, RH, RL)
	xchgw MDE, RHL
	TIME(20)

ld_inn_hl_ed:
	LD_INN_RR(hl, RH, RL)
	TIME(20)

ld_inn_sp:
	xchgw RSP, RHL
	LD_INN_RR(sp, RH, RL)
	xchgw RSP, RHL
	TIME(20)

#define LD_RR_INN(rh, rl) \
	DFETCHP			; \
	movb (VWP), rl		; \
	incw RVW		; \
	movb (VWP), rh
		

ld_bc_inn:
	LD_RR_INN(RB, RC)
	TIME(20)
	
ld_de_inn:
	xchgw MDE, RHL
	LD_RR_INN(RH, RL)
	xchgw MDE, RHL
	TIME(20)
	
ld_hl_inn_ed:
	LD_RR_INN(RH, RL)
	TIME(20)
	
ld_sp_inn:
	xchgw RSP, RHL
	LD_RR_INN(RH, RL)
	xchgw RSP, RHL
	TIME(20)
	
neg:
	negb RA
	SUBFLAGS
	TIME(8)


retn:
	movl IFF2, VWP
	movl VWP, IFF1
	POPD(RPC)
	TIME(14)

reti:
	POPD(RPC)
	TIME(14)

im_0:
	movl $0, IT_MODE
	TIME(8)
	
im_1:
	movl $1, IT_MODE
	TIME(8)
	
im_2:
	movl $2, IT_MODE
	TIME(8)
	

ld_i_a:
	movb RA, MI
	TIME(9)

ld_r_a:
	movb RA, RL7
	movb RA, RW
	andb $0x80, RW
	movb RW, MR
	TIME(9)


#define AIRFLAGS \
	testb RA, RA		; \
	CFLAG(BUTCF, SZF)	; \
	movl IFF2, VWP	        ; \
	testl VWP, VWP		; \
	jz ld_air_end		; \
	orb $PVF, RF		; \
	TIME(9)			; \
	ALIGN

ld_air_end:
	andb $~PVF, RF
	TIME(9)
	
ld_a_i:		
	movb MI, RA
	AIRFLAGS
	
ld_a_r:
	movb MR, RA
	movb RL7, RW
	andb $0x7F, RW
	orb RW, RA
	AIRFLAGS
	
rrd:
	movb RA, RV
	movb (HLP), RW
	shlw $4, RVW
	shrb $4, RW
	andb $0xF0, RA
	orb RW, RA
	movb RV, RW
	CFLAG(BUTCF, SZPF)
	PUTMEM_RHL(rrd, RW, 18)

rld:	
	movb RA, RW
	movb (HLP), RV
	shlb $4, RW
	shrw $4, RVW
	andb $0xF0, RA
	orb RV, RA
	CFLAG(BUTCF, SZPF)
	PUTMEM_RHL(rld, RW, 18)

#define REPEAT \
	subw $2, RPC
	
#define LDID(l, dir) \
	movb (HLP), RW		; \
	xchgw RHL, MDE		; \
	PUTMEM_RHL_NE(l, RW)	; \
	dir RHL			; \
	xchgw RHL, MDE		; \
	dir RHL			; \
	andb $~HPNF, RF		; \
	decw RBC		; \
	jz ldid			; \
	orb $PVF, RF


ldid:
	TIME(16)
	
		
ldi:
	LDID(ldi, incw)
	TIME(16)

ldir:
	LDID(ldir, incw)
	REPEAT
	TIME(21)

ldd:
	LDID(ldd, decw)
	TIME(16)
	
lddr:
	LDID(lddr, decw)
	REPEAT
	TIME(21)

#define CPID(dir) \
	cmpb (HLP), RA		; \
	SFLAG(BUTCF, SZHF)	; \
	dir RHL			; \
	decw RBC		; \
	jz cpid			; \
	JCOND(ZF, ldid)

cpid:
	andb $~PVF, RF
	TIME(16)
	
cpi:
	CPID(incw)
	TIME(16)

cpir:
	CPID(incw)
	REPEAT
	TIME(21)
	
cpd:
	CPID(decw)
	TIME(16)

cpdr:
	CPID(decw)
	REPEAT
	TIME(21)
	
#define INID(l, dir) \
	decb RB			; \
	VSFLAG(BUTCF, SZHF)	; \
	IN(l, RB, RC, RW)	; \
	PUTMEM_RHL_NE(l, RW)	; \
	dir RHL			; \
	testb RB, RB		; \
	jz ldid
	
ini:
	INID(ini, incw)
	TIME(16)

inir:
	INID(inir, incw)
	REPEAT
	TIME(21)
	
ind:
	INID(ind, decw)
	TIME(16)

indr:
	INID(indr, decw)
	REPEAT
	TIME(21)
	
#define OTID(l, dir) \
	decb RB			; \
	VSFLAG(BUTCF, SZHF)	; \
	movb (HLP), RW		; \
	OUT(l, RB, RC, RW)	; \
	dir RHL			; \
	testb RB, RB		; \
	jz ldid
	
outi:
	OTID(outi, incw)
	TIME(16)

otir:
	OTID(otir, incw)
	REPEAT
	TIME(21)
	
outd:
	OTID(outd, decw)
	TIME(16)

otdr:
	INID(otdr, decw)
	REPEAT
	TIME(21)

ill_ed:
	TIME(8)

/* instructions prefixed by CB */


#define BITFL \
	lahf			; \
	andb $SF, RW		; \
	andb $~SZNF, RF		; \
	andb $ZF, RV		; \
	orb RV, RF		; \
	orb RW, RF

#define ROTSHFL \
	CFLAG(ALLF, CF)		; \
	testb RW,RW		; \
	CFLAG(SZPF, SZPF)
	
#define BITFLAGS BITFL
#define ROTSHFLAGS ROTSHFL
	
#define RLC_R(r) \
	rolb r			; \
	movb r, RW		; \
	ROTSHFLAGS

#define RRC_R(r) \
	rorb r			; \
	movb r, RW		; \
	ROTSHFLAGS

#define RL_R(r) \
	SFLAGS			; \
	rclb r			; \
	movb r, RW		; \
	ROTSHFLAGS

#define RR_R(r) \
	SFLAGS			; \
	rcrb r			; \
	movb r, RW		; \
	ROTSHFLAGS

#define SLA_R(r) \
	salb r			; \
	movb r, RW		; \
	ROTSHFLAGS

#define SRA_R(r) \
	sarb r			; \
	movb r, RW		; \
	ROTSHFLAGS

#define SLL_R(r) \
	shlb r			; \
	CFLAG(ALLF, CF)		; \
	orb $1, r		; \
	CFLAG(SZPF, SZPF)
	
#define SRL_R(r) \
	shrb r			; \
	movb r, RW		; \
	ROTSHFLAGS


#define BIT(n, r) \
	movb r, RW		; \
	testb $(1 << n), RW	; \
	BITFLAGS

#define SET(n, r) \
	orb $(1 << n), r

#define RES(n, r) \
	andb $~(1 << n), r



rlc_b:
	RLC_R(RB)
	TIME(8)

rlc_c:
	RLC_R(RC)
	TIME(8)

rlc_d:
	RLC_R(MD)
	TIME(8)

rlc_e:
	RLC_R(ME)
	TIME(8)

rlc_h:
	RLC_R(RH)
	TIME(8)

rlc_l:
	RLC_R(RL)
	TIME(8)

rlc_ihl:
	MODMEM(RLC_R, 15)

rlc_id:
	MODMEMID(id, RLC_R, 23)

rlc_a:
	RLC_R(RA)
	TIME(8)


rrc_b:
	RRC_R(RB)
	TIME(8)

rrc_c:
	RRC_R(RC)
	TIME(8)

rrc_d:
	RRC_R(MD)
	TIME(8)

rrc_e:
	RRC_R(ME)
	TIME(8)

rrc_h:
	RRC_R(RH)
	TIME(8)

rrc_l:
	RRC_R(RL)
	TIME(8)

rrc_ihl:
	MODMEM(RRC_R, 15)

rrc_id:
	MODMEMID(id, RRC_R, 23)

rrc_a:
	RRC_R(RA)
	TIME(8)

rl_b:
	RL_R(RB)
	TIME(8)

rl_c:
	RL_R(RC)
	TIME(8)

rl_d:
	RL_R(MD)
	TIME(8)

rl_e:
	RL_R(ME)
	TIME(8)

rl_h:
	RL_R(RH)
	TIME(8)

rl_l:
	RL_R(RL)
	TIME(8)

rl_ihl:
	MODMEM(RL_R, 15)

rl_id:
	MODMEMID(id, RL_R, 23)
	
rl_a:
	RL_R(RA)
	TIME(8)

rr_b:
	RR_R(RB)
	TIME(8)

rr_c:
	RR_R(RC)
	TIME(8)

rr_d:
	RR_R(MD)
	TIME(8)

rr_e:
	RR_R(ME)
	TIME(8)

rr_h:
	RR_R(RH)
	TIME(8)

rr_l:
	RR_R(RL)
	TIME(8)

rr_ihl:
	MODMEM(RR_R, 15)

rr_id:
	MODMEMID(id, RR_R, 23)

rr_a:
	RR_R(RA)
	TIME(8)


sla_b:
	SLA_R(RB)
	TIME(8)

sla_c:
	SLA_R(RC)
	TIME(8)

sla_d:
	SLA_R(MD)
	TIME(8)

sla_e:
	SLA_R(ME)
	TIME(8)

sla_h:
	SLA_R(RH)
	TIME(8)

sla_l:
	SLA_R(RL)
	TIME(8)

sla_ihl:
	MODMEM(SLA_R, 15)

sla_id:
	MODMEMID(id, SLA_R, 23)
	
sla_a:
	SLA_R(RA)
	TIME(8)


sra_b:
	SRA_R(RB)
	TIME(8)

sra_c:
	SRA_R(RC)
	TIME(8)

sra_d:
	SRA_R(MD)
	TIME(8)

sra_e:
	SRA_R(ME)
	TIME(8)

sra_h:
	SRA_R(RH)
	TIME(8)

sra_l:
	SRA_R(RL)
	TIME(8)

sra_ihl:
	MODMEM(SRA_R, 15)

sra_id:
	MODMEMID(id, SRA_R, 23)

sra_a:
	SRA_R(RA)
	TIME(8)


sll_b:
	SLL_R(RB)
	TIME(8)

sll_c:
	SLL_R(RC)
	TIME(8)

sll_d:
	SLL_R(MD)
	TIME(8)

sll_e:
	SLL_R(ME)
	TIME(8)

sll_h:
	SLL_R(RH)
	TIME(8)

sll_l:
	SLL_R(RL)
	TIME(8)

sll_ihl:
	MODMEM(SLL_R, 15)

sll_id:
	MODMEMID(id, SLL_R, 23)
	
sll_a:
	SLL_R(RA)
	TIME(8)


srl_b:
	SRL_R(RB)
	TIME(8)

srl_c:
	SRL_R(RC)
	TIME(8)

srl_d:
	SRL_R(MD)
	TIME(8)

srl_e:
	SRL_R(ME)
	TIME(8)

srl_h:
	SRL_R(RH)
	TIME(8)

srl_l:
	SRL_R(RL)
	TIME(8)

srl_ihl:
	MODMEM(SRL_R, 15)

srl_id:
	MODMEMID(id, SRL_R, 23)
	
srl_a:
	SRL_R(RA)
	TIME(8)



bit_0_b:
	BIT(0, RB)
	TIME(8)

bit_0_c:
	BIT(0, RC)
	TIME(8)

bit_0_d:
	BIT(0, MD)
	TIME(8)

bit_0_e:
	BIT(0, ME)
	TIME(8)

bit_0_h:
	BIT(0, RH)
	TIME(8)

bit_0_l:
	BIT(0, RL)
	TIME(8)

bit_0_ihl:
	BIT(0, (HLP))
	TIME(12)
	
bit_0_id:
	BIT(0, (HLP))
	RSHL
	TIMEX(20)

bit_0_a:
	BIT(0, RA)
	TIME(8)


bit_1_b:
	BIT(1, RB)
	TIME(8)

bit_1_c:
	BIT(1, RC)
	TIME(8)

bit_1_d:
	BIT(1, MD)
	TIME(8)

bit_1_e:
	BIT(1, ME)
	TIME(8)

bit_1_h:
	BIT(1, RH)
	TIME(8)

bit_1_l:
	BIT(1, RL)
	TIME(8)

bit_1_ihl:
	BIT(1, (HLP))
	TIME(12)
	
bit_1_id:
	BIT(1, (HLP))
	RSHL
	TIMEX(20)

bit_1_a:
	BIT(1, RA)
	TIME(8)



bit_2_b:
	BIT(2, RB)
	TIME(8)

bit_2_c:
	BIT(2, RC)
	TIME(8)

bit_2_d:
	BIT(2, MD)
	TIME(8)

bit_2_e:
	BIT(2, ME)
	TIME(8)

bit_2_h:
	BIT(2, RH)
	TIME(8)

bit_2_l:
	BIT(2, RL)
	TIME(8)

bit_2_ihl:
	BIT(2, (HLP))
	TIME(12)
	
bit_2_id:
	BIT(2, (HLP))
	RSHL
	TIMEX(20)

bit_2_a:
	BIT(2, RA)
	TIME(8)


bit_3_b:
	BIT(3, RB)
	TIME(8)

bit_3_c:
	BIT(3, RC)
	TIME(8)

bit_3_d:
	BIT(3, MD)
	TIME(8)

bit_3_e:
	BIT(3, ME)
	TIME(8)

bit_3_h:
	BIT(3, RH)
	TIME(8)

bit_3_l:
	BIT(3, RL)
	TIME(8)

bit_3_ihl:
	BIT(3, (HLP))
	TIME(12)
	
bit_3_id:
	BIT(3, (HLP))
	RSHL
	TIMEX(20)

bit_3_a:
	BIT(3, RA)
	TIME(8)


bit_4_b:
	BIT(4, RB)
	TIME(8)

bit_4_c:
	BIT(4, RC)
	TIME(8)

bit_4_d:
	BIT(4, MD)
	TIME(8)

bit_4_e:
	BIT(4, ME)
	TIME(8)

bit_4_h:
	BIT(4, RH)
	TIME(8)

bit_4_l:
	BIT(4, RL)
	TIME(8)

bit_4_ihl:
	BIT(4, (HLP))
	TIME(12)
	
bit_4_id:
	BIT(4, (HLP))
	RSHL
	TIMEX(20)

bit_4_a:
	BIT(4, RA)
	TIME(8)



bit_5_b:
	BIT(5, RB)
	TIME(8)

bit_5_c:
	BIT(5, RC)
	TIME(8)

bit_5_d:
	BIT(5, MD)
	TIME(8)

bit_5_e:
	BIT(5, ME)
	TIME(8)

bit_5_h:
	BIT(5, RH)
	TIME(8)

bit_5_l:
	BIT(5, RL)
	TIME(8)

bit_5_ihl:
	BIT(5, (HLP))
	TIME(12)
	
bit_5_id:
	BIT(5, (HLP))
	RSHL
	TIMEX(20)

bit_5_a:
	BIT(5, RA)
	TIME(8)



bit_6_b:
	BIT(6, RB)
	TIME(8)

bit_6_c:
	BIT(6, RC)
	TIME(8)

bit_6_d:
	BIT(6, MD)
	TIME(8)

bit_6_e:
	BIT(6, ME)
	TIME(8)

bit_6_h:
	BIT(6, RH)
	TIME(8)

bit_6_l:
	BIT(6, RL)
	TIME(8)

bit_6_ihl:
	BIT(6, (HLP))
	TIME(12)
	
bit_6_id:
	BIT(6, (HLP))
	RSHL
	TIMEX(20)

bit_6_a:
	BIT(6, RA)
	TIME(8)



bit_7_b:
	BIT(7, RB)
	TIME(8)

bit_7_c:
	BIT(7, RC)
	TIME(8)

bit_7_d:
	BIT(7, MD)
	TIME(8)

bit_7_e:
	BIT(7, ME)
	TIME(8)

bit_7_h:
	BIT(7, RH)
	TIME(8)

bit_7_l:
	BIT(7, RL)
	TIME(8)

bit_7_ihl:
	BIT(7, (HLP))
	TIME(12)
	
bit_7_id:
	BIT(7, (HLP))
	RSHL
	TIMEX(20)

bit_7_a:
	BIT(7, RA)
	TIME(8)


#define PUTMEM_HLP_RW_15 \
	jmp pm15

#define PUTMEMID_HLP_RW_23 \
	jmp pm23

pm15:
	PUTMEM_RHL(pm15, RW, 15)
	
pm23:
	PUTMEMID_RHL(pm23, RW, 23)


res_0_b:
	RES(0, RB)
	TIME(8)

res_0_c:
	RES(0, RC)
	TIME(8)

res_0_d:
	RES(0, MD)
	TIME(8)

res_0_e:
	RES(0, ME)
	TIME(8)

res_0_h:
	RES(0, RH)
	TIME(8)

res_0_l:
	RES(0, RL)
	TIME(8)

res_0_ihl:
	movb (HLP), RW
	RES(0, RW)
	PUTMEM_HLP_RW_15
	
res_0_id:
	movb (HLP), RW
	RES(0, RW)
	PUTMEMID_HLP_RW_23

res_0_a:
	RES(0, RA)
	TIME(8)



res_1_b:
	RES(1, RB)
	TIME(8)

res_1_c:
	RES(1, RC)
	TIME(8)

res_1_d:
	RES(1, MD)
	TIME(8)

res_1_e:
	RES(1, ME)
	TIME(8)

res_1_h:
	RES(1, RH)
	TIME(8)

res_1_l:
	RES(1, RL)
	TIME(8)

res_1_ihl:
	movb (HLP), RW
	RES(1, RW)
	PUTMEM_HLP_RW_15
	
res_1_id:
	movb (HLP), RW
	RES(1, RW)
	PUTMEMID_HLP_RW_23

res_1_a:
	RES(1, RA)
	TIME(8)



res_2_b:
	RES(2, RB)
	TIME(8)

res_2_c:
	RES(2, RC)
	TIME(8)

res_2_d:
	RES(2, MD)
	TIME(8)

res_2_e:
	RES(2, ME)
	TIME(8)

res_2_h:
	RES(2, RH)
	TIME(8)

res_2_l:
	RES(2, RL)
	TIME(8)

res_2_ihl:
	movb (HLP), RW
	RES(2, RW)
	PUTMEM_HLP_RW_15
	
res_2_id:
	movb (HLP), RW
	RES(2, RW)
	PUTMEMID_HLP_RW_23

res_2_a:
	RES(2, RA)
	TIME(8)



res_3_b:
	RES(3, RB)
	TIME(8)

res_3_c:
	RES(3, RC)
	TIME(8)

res_3_d:
	RES(3, MD)
	TIME(8)

res_3_e:
	RES(3, ME)
	TIME(8)

res_3_h:
	RES(3, RH)
	TIME(8)

res_3_l:
	RES(3, RL)
	TIME(8)

res_3_ihl:
	movb (HLP), RW
	RES(3, RW)
	PUTMEM_HLP_RW_15
	
res_3_id:
	movb (HLP), RW
	RES(3, RW)
	PUTMEMID_HLP_RW_23

res_3_a:
	RES(3, RA)
	TIME(8)



res_4_b:
	RES(4, RB)
	TIME(8)

res_4_c:
	RES(4, RC)
	TIME(8)

res_4_d:
	RES(4, MD)
	TIME(8)

res_4_e:
	RES(4, ME)
	TIME(8)

res_4_h:
	RES(4, RH)
	TIME(8)

res_4_l:
	RES(4, RL)
	TIME(8)

res_4_ihl:
	movb (HLP), RW
	RES(4, RW)
	PUTMEM_HLP_RW_15
	
res_4_id:
	movb (HLP), RW
	RES(4, RW)
	PUTMEMID_HLP_RW_23

res_4_a:
	RES(4, RA)
	TIME(8)



res_5_b:
	RES(5, RB)
	TIME(8)

res_5_c:
	RES(5, RC)
	TIME(8)

res_5_d:
	RES(5, MD)
	TIME(8)

res_5_e:
	RES(5, ME)
	TIME(8)

res_5_h:
	RES(5, RH)
	TIME(8)

res_5_l:
	RES(5, RL)
	TIME(8)

res_5_ihl:
	movb (HLP), RW
	RES(5, RW)
	PUTMEM_HLP_RW_15
	
res_5_id:
	movb (HLP), RW
	RES(5, RW)
	PUTMEMID_HLP_RW_23

res_5_a:
	RES(5, RA)
	TIME(8)



res_6_b:
	RES(6, RB)
	TIME(8)

res_6_c:
	RES(6, RC)
	TIME(8)

res_6_d:
	RES(6, MD)
	TIME(8)

res_6_e:
	RES(6, ME)
	TIME(8)

res_6_h:
	RES(6, RH)
	TIME(8)

res_6_l:
	RES(6, RL)
	TIME(8)

res_6_ihl:
	movb (HLP), RW
	RES(6, RW)
	PUTMEM_HLP_RW_15
	
res_6_id:
	movb (HLP), RW
	RES(6, RW)
	PUTMEMID_HLP_RW_23

res_6_a:
	RES(6, RA)
	TIME(8)



res_7_b:
	RES(7, RB)
	TIME(8)

res_7_c:
	RES(7, RC)
	TIME(8)

res_7_d:
	RES(7, MD)
	TIME(8)

res_7_e:
	RES(7, ME)
	TIME(8)

res_7_h:
	RES(7, RH)
	TIME(8)

res_7_l:
	RES(7, RL)
	TIME(8)

res_7_ihl:
	movb (HLP), RW
	RES(7, RW)
	PUTMEM_HLP_RW_15
	
res_7_id:
	movb (HLP), RW
	RES(7, RW)
	PUTMEMID_HLP_RW_23

res_7_a:
	RES(7, RA)
	TIME(8)



set_0_b:
	SET(0, RB)
	TIME(8)

set_0_c:
	SET(0, RC)
	TIME(8)

set_0_d:
	SET(0, MD)
	TIME(8)

set_0_e:
	SET(0, ME)
	TIME(8)

set_0_h:
	SET(0, RH)
	TIME(8)

set_0_l:
	SET(0, RL)
	TIME(8)

set_0_ihl:
	movb (HLP), RW
	SET(0, RW)
	PUTMEM_HLP_RW_15
	
set_0_id:
	movb (HLP), RW
	SET(0, RW)
	PUTMEMID_HLP_RW_23

set_0_a:
	SET(0, RA)
	TIME(8)



set_1_b:
	SET(1, RB)
	TIME(8)

set_1_c:
	SET(1, RC)
	TIME(8)

set_1_d:
	SET(1, MD)
	TIME(8)

set_1_e:
	SET(1, ME)
	TIME(8)

set_1_h:
	SET(1, RH)
	TIME(8)

set_1_l:
	SET(1, RL)
	TIME(8)

set_1_ihl:
	movb (HLP), RW
	SET(1, RW)
	PUTMEM_HLP_RW_15
	
set_1_id:
	movb (HLP), RW
	SET(1, RW)
	PUTMEMID_HLP_RW_23

set_1_a:
	SET(1, RA)
	TIME(8)



set_2_b:
	SET(2, RB)
	TIME(8)

set_2_c:
	SET(2, RC)
	TIME(8)

set_2_d:
	SET(2, MD)
	TIME(8)

set_2_e:
	SET(2, ME)
	TIME(8)

set_2_h:
	SET(2, RH)
	TIME(8)

set_2_l:
	SET(2, RL)
	TIME(8)

set_2_ihl:
	movb (HLP), RW
	SET(2, RW)
	PUTMEM_HLP_RW_15
	
set_2_id:
	movb (HLP), RW
	SET(2, RW)
	PUTMEMID_HLP_RW_23

set_2_a:
	SET(2, RA)
	TIME(8)



set_3_b:
	SET(3, RB)
	TIME(8)

set_3_c:
	SET(3, RC)
	TIME(8)

set_3_d:
	SET(3, MD)
	TIME(8)

set_3_e:
	SET(3, ME)
	TIME(8)

set_3_h:
	SET(3, RH)
	TIME(8)

set_3_l:
	SET(3, RL)
	TIME(8)

set_3_ihl:
	movb (HLP), RW
	SET(3, RW)
	PUTMEM_HLP_RW_15
	
set_3_id:
	movb (HLP), RW
	SET(3, RW)
	PUTMEMID_HLP_RW_23

set_3_a:
	SET(3, RA)
	TIME(8)



set_4_b:
	SET(4, RB)
	TIME(8)

set_4_c:
	SET(4, RC)
	TIME(8)

set_4_d:
	SET(4, MD)
	TIME(8)

set_4_e:
	SET(4, ME)
	TIME(8)

set_4_h:
	SET(4, RH)
	TIME(8)

set_4_l:
	SET(4, RL)
	TIME(8)

set_4_ihl:
	movb (HLP), RW
	SET(4, RW)
	PUTMEM_HLP_RW_15
	
set_4_id:
	movb (HLP), RW
	SET(4, RW)
	PUTMEMID_HLP_RW_23

set_4_a:
	SET(4, RA)
	TIME(8)



set_5_b:
	SET(5, RB)
	TIME(8)

set_5_c:
	SET(5, RC)
	TIME(8)

set_5_d:
	SET(5, MD)
	TIME(8)

set_5_e:
	SET(5, ME)
	TIME(8)

set_5_h:
	SET(5, RH)
	TIME(8)

set_5_l:
	SET(5, RL)
	TIME(8)

set_5_ihl:
	movb (HLP), RW
	SET(5, RW)
	PUTMEM_HLP_RW_15
	
set_5_id:
	movb (HLP), RW
	SET(5, RW)
	PUTMEMID_HLP_RW_23

set_5_a:
	SET(5, RA)
	TIME(8)



set_6_b:
	SET(6, RB)
	TIME(8)

set_6_c:
	SET(6, RC)
	TIME(8)

set_6_d:
	SET(6, MD)
	TIME(8)

set_6_e:
	SET(6, ME)
	TIME(8)

set_6_h:
	SET(6, RH)
	TIME(8)

set_6_l:
	SET(6, RL)
	TIME(8)

set_6_ihl:
	movb (HLP), RW
	SET(6, RW)
	PUTMEM_HLP_RW_15
	
set_6_id:
	movb (HLP), RW
	SET(6, RW)
	PUTMEMID_HLP_RW_23

set_6_a:
	SET(6, RA)
	TIME(8)



set_7_b:
	SET(7, RB)
	TIME(8)

set_7_c:
	SET(7, RC)
	TIME(8)

set_7_d:
	SET(7, MD)
	TIME(8)

set_7_e:
	SET(7, ME)
	TIME(8)

set_7_h:
	SET(7, RH)
	TIME(8)

set_7_l:
	SET(7, RL)
	TIME(8)

set_7_ihl:
	movb (HLP), RW
	SET(7, RW)
	PUTMEM_HLP_RW_15
	
set_7_id:
	movb (HLP), RW
	SET(7, RW)
	PUTMEMID_HLP_RW_23

set_7_a:
	SET(7, RA)
	TIME(8)


	.data
			
intr_table:
	.long nop		# 00
	.long ld_bc_nn
	.long ld_ibc_a
	.long inc_bc
	.long inc_b
	.long dec_b
	.long ld_b_n
	.long rlca
	.long ex_af_afb
	.long add_hl_bc
	.long ld_a_ibc
	.long dec_bc
	.long inc_c
	.long dec_c
	.long ld_c_n
	.long rrca

	.long djnz_e		# 10
	.long ld_de_nn
	.long ld_ide_a
	.long inc_de
	.long inc_d
	.long dec_d
	.long ld_d_n
	.long rla
	.long jr_e
	.long add_hl_de
	.long ld_a_ide
	.long dec_de
	.long inc_e
	.long dec_e
	.long ld_e_n
	.long rra

	.long jr_nz_e		# 20
	.long ld_hl_nn
	.long ld_inn_hl
	.long inc_hl
	.long inc_h
	.long dec_h
	.long ld_h_n
	.long daa
	.long jr_z_e
	.long add_hl_hl
	.long ld_hl_inn
	.long dec_hl
	.long inc_l
	.long dec_l
	.long ld_l_n
	.long cpl

	.long jr_nc_e		# 30
	.long ld_sp_nn
	.long ld_inn_a
	.long inc_sp
	.long inc_ihl
	.long dec_ihl
	.long ld_ihl_n
	.long scf
	.long jr_c_e
	.long add_hl_sp
	.long ld_a_inn
	.long dec_sp
	.long inc_a
	.long dec_a
	.long ld_a_n
	.long ccf

	.long nop		# 40
	.long ld_b_c
	.long ld_b_d
	.long ld_b_e
	.long ld_b_h
	.long ld_b_l
	.long ld_b_ihl
	.long ld_b_a
	.long ld_c_b
	.long nop
	.long ld_c_d
	.long ld_c_e
	.long ld_c_h
	.long ld_c_l
	.long ld_c_ihl
	.long ld_c_a

	.long ld_d_b		# 50
	.long ld_d_c
	.long nop
	.long ld_d_e
	.long ld_d_h
	.long ld_d_l
	.long ld_d_ihl
	.long ld_d_a
	.long ld_e_b
	.long ld_e_c
	.long ld_e_d
	.long nop
	.long ld_e_h
	.long ld_e_l
	.long ld_e_ihl
	.long ld_e_a

	.long ld_h_b		# 60
	.long ld_h_c
	.long ld_h_d
	.long ld_h_e
	.long nop
	.long ld_h_l
	.long ld_h_ihl
	.long ld_h_a
	.long ld_l_b
	.long ld_l_c
	.long ld_l_d
	.long ld_l_e
	.long ld_l_h
	.long nop
	.long ld_l_ihl
	.long ld_l_a
	
	.long ld_ihl_b		# 70
	.long ld_ihl_c
	.long ld_ihl_d
	.long ld_ihl_e
	.long ld_ihl_h
	.long ld_ihl_l
	.long halt
	.long ld_ihl_a
	.long ld_a_b
	.long ld_a_c
	.long ld_a_d
	.long ld_a_e
	.long ld_a_h
	.long ld_a_l
	.long ld_a_ihl
	.long nop
	
	.long add_a_b		# 80
	.long add_a_c
	.long add_a_d
	.long add_a_e
	.long add_a_h
	.long add_a_l
	.long add_a_ihl
	.long add_a_a
	.long adc_a_b
	.long adc_a_c
	.long adc_a_d
	.long adc_a_e
	.long adc_a_h
	.long adc_a_l
	.long adc_a_ihl
	.long adc_a_a
	
	.long sub_b		# 90
	.long sub_c
	.long sub_d
	.long sub_e
	.long sub_h
	.long sub_l
	.long sub_ihl
	.long sub_a
	.long sbc_a_b
	.long sbc_a_c
	.long sbc_a_d
	.long sbc_a_e
	.long sbc_a_h
	.long sbc_a_l
	.long sbc_a_ihl
	.long sbc_a_a

	.long and_b		# A0
	.long and_c
	.long and_d
	.long and_e
	.long and_h
	.long and_l
	.long and_ihl
	.long and_a
	.long xor_b
	.long xor_c
	.long xor_d
	.long xor_e
	.long xor_h
	.long xor_l
	.long xor_ihl
	.long xor_a

	.long or_b		# B0
	.long or_c
	.long or_d
	.long or_e
	.long or_h
	.long or_l
	.long or_ihl
	.long or_a
	.long cp_b
	.long cp_c
	.long cp_d
	.long cp_e
	.long cp_h
	.long cp_l
	.long cp_ihl
	.long cp_a

	.long ret_nz		# C0
	.long pop_bc
	.long jp_nz_nn
	.long jp_nn
	.long call_nz_nn
	.long push_bc
	.long add_a_n
	.long rst_0
	.long ret_z
	.long ret
	.long jp_z_nn
	.long special_cb
	.long call_z_nn
	.long call_nn
	.long adc_a_n
	.long rst_8

	.long ret_nc		# D0
	.long pop_de
	.long jp_nc_nn
	.long out_in_a
	.long call_nc_nn
	.long push_de
	.long sub_n
	.long rst_10
	.long ret_c
	.long exx
	.long jp_c_nn
	.long in_a_in
	.long call_c_nn
	.long special_dd
	.long sbc_a_n
	.long rst_18

	.long ret_po		# E0
	.long pop_hl
	.long jp_po_nn
	.long ex_isp_hl
	.long call_po_nn
	.long push_hl
	.long and_n
	.long rst_20
	.long ret_pe
	.long jp_hl
	.long jp_pe_nn
	.long ex_de_hl
	.long call_pe_nn
	.long special_ed
	.long xor_n
	.long rst_28

	.long ret_p		# F0
	.long pop_af
	.long jp_p_nn
	.long di
	.long call_p_nn
	.long push_af
	.long or_n
	.long rst_30
	.long ret_m
	.long ld_sp_hl
	.long jp_m_nn
	.long ei
	.long call_m_nn
	.long special_fd
	.long cp_n
	.long rst_38

	ALIGN



intr_table_dd:
	.long nop		# 00
	.long ld_bc_nn
	.long ld_ibc_a
	.long inc_bc
	.long inc_b
	.long dec_b
	.long ld_b_n
	.long rlca
	.long ex_af_afb
	.long add_ix_bc
	.long ld_a_ibc
	.long dec_bc
	.long inc_c
	.long dec_c
	.long ld_c_n
	.long rrca

	.long djnz_e		# 10
	.long ld_de_nn
	.long ld_ide_a
	.long inc_de
	.long inc_d
	.long dec_d
	.long ld_d_n
	.long rla
	.long jr_e
	.long add_ix_de
	.long ld_a_ide
	.long dec_de
	.long inc_e
	.long dec_e
	.long ld_e_n
	.long rra

	.long jr_nz_e		# 20
	.long ld_ix_nn
	.long ld_inn_ix
	.long inc_ix
	.long inc_ixh
	.long dec_ixh
	.long ld_ixh_n
	.long daa
	.long jr_z_e
	.long add_ix_ix
	.long ld_ix_inn
	.long dec_ix
	.long inc_ixl
	.long dec_ixl
	.long ld_ixl_n
	.long cpl

	.long jr_nc_e		# 30
	.long ld_sp_nn
	.long ld_inn_a
	.long inc_sp
	.long inc_iixd
	.long dec_iixd
	.long ld_iixd_n
	.long scf
	.long jr_c_e
	.long add_ix_sp
	.long ld_a_inn
	.long dec_sp
	.long inc_a
	.long dec_a
	.long ld_a_n
	.long ccf

	.long nop		# 40
	.long ld_b_c
	.long ld_b_d
	.long ld_b_e
	.long ld_b_ixh
	.long ld_b_ixl
	.long ld_b_iixd
	.long ld_b_a
	.long ld_c_b
	.long nop
	.long ld_c_d
	.long ld_c_e
	.long ld_c_ixh
	.long ld_c_ixl
	.long ld_c_iixd
	.long ld_c_a

	.long ld_d_b		# 50
	.long ld_d_c
	.long nop
	.long ld_d_e
	.long ld_d_ixh
	.long ld_d_ixl
	.long ld_d_iixd
	.long ld_d_a
	.long ld_e_b
	.long ld_e_c
	.long ld_e_d
	.long nop
	.long ld_e_ixh
	.long ld_e_ixl
	.long ld_e_iixd
	.long ld_e_a

	.long ld_ixh_b		# 60
	.long ld_ixh_c
	.long ld_ixh_d
	.long ld_ixh_e
	.long nop
	.long ld_ixh_ixl
	.long ld_h_iixd
	.long ld_ixh_a
	.long ld_ixl_b
	.long ld_ixl_c
	.long ld_ixl_d
	.long ld_ixl_e
	.long ld_ixl_ixh
	.long nop
	.long ld_l_iixd
	.long ld_ixl_a
	
	.long ld_iixd_b		# 70
	.long ld_iixd_c
	.long ld_iixd_d
	.long ld_iixd_e
	.long ld_iixd_h
	.long ld_iixd_l
	.long halt
	.long ld_iixd_a
	.long ld_a_b
	.long ld_a_c
	.long ld_a_d
	.long ld_a_e
	.long ld_a_ixh
	.long ld_a_ixl
	.long ld_a_iixd
	.long nop
	
	.long add_a_b		# 80
	.long add_a_c
	.long add_a_d
	.long add_a_e
	.long add_a_ixh
	.long add_a_ixl
	.long add_a_iixd
	.long add_a_a
	.long adc_a_b
	.long adc_a_c
	.long adc_a_d
	.long adc_a_e
	.long adc_a_ixh
	.long adc_a_ixl
	.long adc_a_iixd
	.long adc_a_a
	
	.long sub_b		# 90
	.long sub_c
	.long sub_d
	.long sub_e
	.long sub_ixh
	.long sub_ixl
	.long sub_iixd
	.long sub_a
	.long sbc_a_b
	.long sbc_a_c
	.long sbc_a_d
	.long sbc_a_e
	.long sbc_a_ixh
	.long sbc_a_ixl
	.long sbc_a_iixd
	.long sbc_a_a

	.long and_b		# A0
	.long and_c
	.long and_d
	.long and_e
	.long and_ixh
	.long and_ixl
	.long and_iixd
	.long and_a
	.long xor_b
	.long xor_c
	.long xor_d
	.long xor_e
	.long xor_ixh
	.long xor_ixl
	.long xor_iixd
	.long xor_a

	.long or_b		# B0
	.long or_c
	.long or_d
	.long or_e
	.long or_ixh
	.long or_ixl
	.long or_iixd
	.long or_a
	.long cp_b
	.long cp_c
	.long cp_d
	.long cp_e
	.long cp_ixh
	.long cp_ixl
	.long cp_iixd
	.long cp_a

	.long ret_nz		# C0
	.long pop_bc
	.long jp_nz_nn
	.long jp_nn
	.long call_nz_nn
	.long push_bc
	.long add_a_n
	.long rst_0
	.long ret_z
	.long ret
	.long jp_z_nn
	.long special_dd_cb
	.long call_z_nn
	.long call_nn
	.long adc_a_n
	.long rst_8

	.long ret_nc		# D0
	.long pop_de
	.long jp_nc_nn
	.long out_in_a
	.long call_nc_nn
	.long push_de
	.long sub_n
	.long rst_10
	.long ret_c
	.long exx
	.long jp_c_nn
	.long in_a_in
	.long call_c_nn
	.long special_xx
	.long sbc_a_n
	.long rst_18

	.long ret_po		# E0
	.long pop_ix
	.long jp_po_nn
	.long ex_isp_ix
	.long call_po_nn
	.long push_ix
	.long and_n
	.long rst_20
	.long ret_pe
	.long jp_ix
	.long jp_pe_nn
	.long ex_de_hl
	.long call_pe_nn
	.long special_ed
	.long xor_n
	.long rst_28

	.long ret_p		# F0
	.long pop_af
	.long jp_p_nn
	.long di
	.long call_p_nn
	.long push_af
	.long or_n
	.long rst_30
	.long ret_m
	.long ld_sp_ix
	.long jp_m_nn
	.long ei
	.long call_m_nn
	.long special_xx
	.long cp_n
	.long rst_38

	ALIGN


intr_table_fd:
	.long nop		# 00
	.long ld_bc_nn
	.long ld_ibc_a
	.long inc_bc
	.long inc_b
	.long dec_b
	.long ld_b_n
	.long rlca
	.long ex_af_afb
	.long add_iy_bc
	.long ld_a_ibc
	.long dec_bc
	.long inc_c
	.long dec_c
	.long ld_c_n
	.long rrca

	.long djnz_e		# 10
	.long ld_de_nn
	.long ld_ide_a
	.long inc_de
	.long inc_d
	.long dec_d
	.long ld_d_n
	.long rla
	.long jr_e
	.long add_iy_de
	.long ld_a_ide
	.long dec_de
	.long inc_e
	.long dec_e
	.long ld_e_n
	.long rra

	.long jr_nz_e		# 20
	.long ld_iy_nn
	.long ld_inn_iy
	.long inc_iy
	.long inc_iyh
	.long dec_iyh
	.long ld_iyh_n
	.long daa
	.long jr_z_e
	.long add_iy_iy
	.long ld_iy_inn
	.long dec_iy
	.long inc_iyl
	.long dec_iyl
	.long ld_iyl_n
	.long cpl

	.long jr_nc_e		# 30
	.long ld_sp_nn
	.long ld_inn_a
	.long inc_sp
	.long inc_iiyd
	.long dec_iiyd
	.long ld_iiyd_n
	.long scf
	.long jr_c_e
	.long add_iy_sp
	.long ld_a_inn
	.long dec_sp
	.long inc_a
	.long dec_a
	.long ld_a_n
	.long ccf

	.long nop		# 40
	.long ld_b_c
	.long ld_b_d
	.long ld_b_e
	.long ld_b_iyh
	.long ld_b_iyl
	.long ld_b_iiyd
	.long ld_b_a
	.long ld_c_b
	.long nop
	.long ld_c_d
	.long ld_c_e
	.long ld_c_iyh
	.long ld_c_iyl
	.long ld_c_iiyd
	.long ld_c_a

	.long ld_d_b		# 50
	.long ld_d_c
	.long nop
	.long ld_d_e
	.long ld_d_iyh
	.long ld_d_iyl
	.long ld_d_iiyd
	.long ld_d_a
	.long ld_e_b
	.long ld_e_c
	.long ld_e_d
	.long nop
	.long ld_e_iyh
	.long ld_e_iyl
	.long ld_e_iiyd
	.long ld_e_a

	.long ld_iyh_b		# 60
	.long ld_iyh_c
	.long ld_iyh_d
	.long ld_iyh_e
	.long nop
	.long ld_iyh_iyl
	.long ld_h_iiyd
	.long ld_iyh_a
	.long ld_iyl_b
	.long ld_iyl_c
	.long ld_iyl_d
	.long ld_iyl_e
	.long ld_iyl_iyh
	.long nop
	.long ld_l_iiyd
	.long ld_iyl_a
	
	.long ld_iiyd_b		# 70
	.long ld_iiyd_c
	.long ld_iiyd_d
	.long ld_iiyd_e
	.long ld_iiyd_h
	.long ld_iiyd_l
	.long halt
	.long ld_iiyd_a
	.long ld_a_b
	.long ld_a_c
	.long ld_a_d
	.long ld_a_e
	.long ld_a_iyh
	.long ld_a_iyl
	.long ld_a_iiyd
	.long nop
	
	.long add_a_b		# 80
	.long add_a_c
	.long add_a_d
	.long add_a_e
	.long add_a_iyh
	.long add_a_iyl
	.long add_a_iiyd
	.long add_a_a
	.long adc_a_b
	.long adc_a_c
	.long adc_a_d
	.long adc_a_e
	.long adc_a_iyh
	.long adc_a_iyl
	.long adc_a_iiyd
	.long adc_a_a
	
	.long sub_b		# 90
	.long sub_c
	.long sub_d
	.long sub_e
	.long sub_iyh
	.long sub_iyl
	.long sub_iiyd
	.long sub_a
	.long sbc_a_b
	.long sbc_a_c
	.long sbc_a_d
	.long sbc_a_e
	.long sbc_a_iyh
	.long sbc_a_iyl
	.long sbc_a_iiyd
	.long sbc_a_a

	.long and_b		# A0
	.long and_c
	.long and_d
	.long and_e
	.long and_iyh
	.long and_iyl
	.long and_iiyd
	.long and_a
	.long xor_b
	.long xor_c
	.long xor_d
	.long xor_e
	.long xor_iyh
	.long xor_iyl
	.long xor_iiyd
	.long xor_a

	.long or_b		# B0
	.long or_c
	.long or_d
	.long or_e
	.long or_iyh
	.long or_iyl
	.long or_iiyd
	.long or_a
	.long cp_b
	.long cp_c
	.long cp_d
	.long cp_e
	.long cp_iyh
	.long cp_iyl
	.long cp_iiyd
	.long cp_a

	.long ret_nz		# C0
	.long pop_bc
	.long jp_nz_nn
	.long jp_nn
	.long call_nz_nn
	.long push_bc
	.long add_a_n
	.long rst_0
	.long ret_z
	.long ret
	.long jp_z_nn
	.long special_fd_cb
	.long call_z_nn
	.long call_nn
	.long adc_a_n
	.long rst_8

	.long ret_nc		# D0
	.long pop_de
	.long jp_nc_nn
	.long out_in_a
	.long call_nc_nn
	.long push_de
	.long sub_n
	.long rst_10
	.long ret_c
	.long exx
	.long jp_c_nn
	.long in_a_in
	.long call_c_nn
	.long special_xx
	.long sbc_a_n
	.long rst_18

	.long ret_po		# E0
	.long pop_iy
	.long jp_po_nn
	.long ex_isp_iy
	.long call_po_nn
	.long push_iy
	.long and_n
	.long rst_20
	.long ret_pe
	.long jp_iy
	.long jp_pe_nn
	.long ex_de_hl
	.long call_pe_nn
	.long special_ed
	.long xor_n
	.long rst_28

	.long ret_p		# F0
	.long pop_af
	.long jp_p_nn
	.long di
	.long call_p_nn
	.long push_af
	.long or_n
	.long rst_30
	.long ret_m
	.long ld_sp_iy
	.long jp_m_nn
	.long ei
	.long call_m_nn
	.long special_xx
	.long cp_n
	.long rst_38

	ALIGN


intr_table_ed:
	.long ill_ed		# 00
	.long ill_ed
	.long ill_ed
	.long ill_ed
	.long ill_ed
	.long ill_ed
	.long ill_ed
	.long ill_ed
	.long ill_ed
	.long ill_ed
	.long ill_ed
	.long ill_ed
	.long ill_ed
	.long ill_ed
	.long ill_ed
	.long ill_ed

	.long ill_ed		# 10
	.long ill_ed
	.long ill_ed
	.long ill_ed
	.long ill_ed
	.long ill_ed
	.long ill_ed
	.long ill_ed
	.long ill_ed
	.long ill_ed
	.long ill_ed
	.long ill_ed
	.long ill_ed
	.long ill_ed
	.long ill_ed
	.long ill_ed

	.long ill_ed		# 20
	.long ill_ed
	.long ill_ed
	.long ill_ed
	.long ill_ed
	.long ill_ed
	.long ill_ed
	.long ill_ed
	.long ill_ed
	.long ill_ed
	.long ill_ed
	.long ill_ed
	.long ill_ed
	.long ill_ed
	.long ill_ed
	.long ill_ed

	.long ill_ed		# 30
	.long ill_ed
	.long ill_ed
	.long ill_ed
	.long ill_ed
	.long ill_ed
	.long ill_ed
	.long ill_ed
	.long ill_ed
	.long ill_ed
	.long ill_ed
	.long ill_ed
	.long ill_ed
	.long ill_ed
	.long ill_ed
	.long ill_ed

	.long in_b_ic		# 40
	.long out_ic_b
	.long sbc_hl_bc
	.long ld_inn_bc
	.long neg
	.long retn
	.long im_0
	.long ld_i_a
	.long in_c_ic
	.long out_ic_c
	.long adc_hl_bc
	.long ld_bc_inn
	.long neg
	.long reti
	.long im_0
	.long ld_r_a

	.long in_d_ic		# 50
	.long out_ic_d
	.long sbc_hl_de
	.long ld_inn_de
	.long neg
	.long retn
	.long im_1
	.long ld_a_i
	.long in_e_ic
	.long out_ic_e
	.long adc_hl_de
	.long ld_de_inn
	.long neg
	.long retn
	.long im_2
	.long ld_a_r

	.long in_h_ic		# 60
	.long out_ic_h
	.long sbc_hl_hl
	.long ld_inn_hl_ed
	.long neg
	.long retn
	.long im_0
	.long rrd
	.long in_l_ic
	.long out_ic_l
	.long adc_hl_hl
	.long ld_hl_inn_ed
	.long neg
	.long retn
	.long im_0
	.long rld

	.long in_f_ic		# 70
	.long out_ic_0
	.long sbc_hl_sp
	.long ld_inn_sp
	.long neg
	.long retn
	.long im_1
	.long ill_ed
	.long in_a_ic
	.long out_ic_a
	.long adc_hl_sp
	.long ld_sp_inn
	.long neg
	.long retn
	.long im_2
	.long ill_ed

	.long ill_ed		# 80
	.long ill_ed
	.long ill_ed
	.long ill_ed
	.long ill_ed
	.long ill_ed
	.long ill_ed
	.long ill_ed
	.long ill_ed
	.long ill_ed
	.long ill_ed
	.long ill_ed
	.long ill_ed
	.long ill_ed
	.long ill_ed
	.long ill_ed

	.long ill_ed		# 90
	.long ill_ed
	.long ill_ed
	.long ill_ed
	.long ill_ed
	.long ill_ed
	.long ill_ed
	.long ill_ed
	.long ill_ed
	.long ill_ed
	.long ill_ed
	.long ill_ed
	.long ill_ed
	.long ill_ed
	.long ill_ed
	.long ill_ed

	.long ldi		# A0
	.long cpi
	.long ini
	.long outi
	.long ill_ed
	.long ill_ed
	.long ill_ed
	.long ill_ed
	.long ldd
	.long cpd
	.long ind
	.long outd
	.long ill_ed
	.long ill_ed
	.long ill_ed
	.long ill_ed

	.long ldir		# B0
	.long cpir
	.long inir
	.long otir
	.long ill_ed
	.long ill_ed
	.long ill_ed
	.long ill_ed
	.long lddr
	.long cpdr
	.long indr
	.long otdr
	.long ill_ed
	.long ill_ed
	.long ill_ed
	.long ill_ed

	.long ill_ed		# C0
	.long ill_ed
	.long ill_ed
	.long ill_ed
	.long ill_ed
	.long ill_ed
	.long ill_ed
	.long ill_ed
	.long ill_ed
	.long ill_ed
	.long ill_ed
	.long ill_ed
	.long ill_ed
	.long ill_ed
	.long ill_ed
	.long ill_ed

	.long ill_ed		# D0
	.long ill_ed
	.long ill_ed
	.long ill_ed
	.long ill_ed
	.long ill_ed
	.long ill_ed
	.long ill_ed
	.long ill_ed
	.long ill_ed
	.long ill_ed
	.long ill_ed
	.long ill_ed
	.long ill_ed
	.long ill_ed
	.long ill_ed

	.long ill_ed		# E0
	.long ill_ed
	.long ill_ed
	.long ill_ed
	.long ill_ed
	.long ill_ed
	.long ill_ed
	.long ill_ed
	.long ill_ed
	.long ill_ed
	.long ill_ed
	.long ill_ed
	.long ill_ed
	.long ill_ed
	.long ill_ed
	.long ill_ed

	.long ill_ed		# F0
	.long ill_ed
	.long ill_ed
	.long ill_ed
	.long ill_ed
	.long ill_ed
	.long ill_ed
	.long ill_ed
	.long ill_ed
	.long ill_ed
	.long ill_ed
	.long ill_ed
	.long ill_ed
	.long ill_ed
	.long ill_ed
	.long ill_ed

	ALIGN


intr_table_cb:
	.long rlc_b		# 00
	.long rlc_c
	.long rlc_d
	.long rlc_e
	.long rlc_h
	.long rlc_l
	.long rlc_ihl
	.long rlc_a
	.long rrc_b
	.long rrc_c
	.long rrc_d
	.long rrc_e
	.long rrc_h
	.long rrc_l
	.long rrc_ihl
	.long rrc_a

	.long rl_b		# 10
	.long rl_c
	.long rl_d
	.long rl_e
	.long rl_h
	.long rl_l
	.long rl_ihl
	.long rl_a
	.long rr_b
	.long rr_c
	.long rr_d
	.long rr_e
	.long rr_h
	.long rr_l
	.long rr_ihl
	.long rr_a

	.long sla_b		# 20
	.long sla_c
	.long sla_d
	.long sla_e
	.long sla_h
	.long sla_l
	.long sla_ihl
	.long sla_a
	.long sra_b
	.long sra_c
	.long sra_d
	.long sra_e
	.long sra_h
	.long sra_l
	.long sra_ihl
	.long sra_a

	.long sll_b		# 30
	.long sll_c
	.long sll_d
	.long sll_e
	.long sll_h
	.long sll_l
	.long sll_ihl
	.long sll_a
	.long srl_b
	.long srl_c
	.long srl_d
	.long srl_e
	.long srl_h
	.long srl_l
	.long srl_ihl
	.long srl_a

	.long bit_0_b		# 40
	.long bit_0_c
	.long bit_0_d
	.long bit_0_e
	.long bit_0_h
	.long bit_0_l
	.long bit_0_ihl
	.long bit_0_a
	.long bit_1_b
	.long bit_1_c
	.long bit_1_d
	.long bit_1_e
	.long bit_1_h
	.long bit_1_l
	.long bit_1_ihl
	.long bit_1_a

	.long bit_2_b		# 50
	.long bit_2_c
	.long bit_2_d
	.long bit_2_e
	.long bit_2_h
	.long bit_2_l
	.long bit_2_ihl
	.long bit_2_a
	.long bit_3_b
	.long bit_3_c
	.long bit_3_d
	.long bit_3_e
	.long bit_3_h
	.long bit_3_l
	.long bit_3_ihl
	.long bit_3_a

	.long bit_4_b		# 60
	.long bit_4_c
	.long bit_4_d
	.long bit_4_e
	.long bit_4_h
	.long bit_4_l
	.long bit_4_ihl
	.long bit_4_a
	.long bit_5_b
	.long bit_5_c
	.long bit_5_d
	.long bit_5_e
	.long bit_5_h
	.long bit_5_l
	.long bit_5_ihl
	.long bit_5_a

	.long bit_6_b		# 70
	.long bit_6_c
	.long bit_6_d
	.long bit_6_e
	.long bit_6_h
	.long bit_6_l
	.long bit_6_ihl
	.long bit_6_a
	.long bit_7_b
	.long bit_7_c
	.long bit_7_d
	.long bit_7_e
	.long bit_7_h
	.long bit_7_l
	.long bit_7_ihl
	.long bit_7_a

	.long res_0_b		# 80
	.long res_0_c
	.long res_0_d
	.long res_0_e
	.long res_0_h
	.long res_0_l
	.long res_0_ihl
	.long res_0_a
	.long res_1_b
	.long res_1_c
	.long res_1_d
	.long res_1_e
	.long res_1_h
	.long res_1_l
	.long res_1_ihl
	.long res_1_a

	.long res_2_b		# 90
	.long res_2_c
	.long res_2_d
	.long res_2_e
	.long res_2_h
	.long res_2_l
	.long res_2_ihl
	.long res_2_a
	.long res_3_b
	.long res_3_c
	.long res_3_d
	.long res_3_e
	.long res_3_h
	.long res_3_l
	.long res_3_ihl
	.long res_3_a

	.long res_4_b		# A0
	.long res_4_c
	.long res_4_d
	.long res_4_e
	.long res_4_h
	.long res_4_l
	.long res_4_ihl
	.long res_4_a
	.long res_5_b
	.long res_5_c
	.long res_5_d
	.long res_5_e
	.long res_5_h
	.long res_5_l
	.long res_5_ihl
	.long res_5_a

	.long res_6_b		# B0
	.long res_6_c
	.long res_6_d
	.long res_6_e
	.long res_6_h
	.long res_6_l
	.long res_6_ihl
	.long res_6_a
	.long res_7_b
	.long res_7_c
	.long res_7_d
	.long res_7_e
	.long res_7_h
	.long res_7_l
	.long res_7_ihl
	.long res_7_a

	.long set_0_b		# C0
	.long set_0_c
	.long set_0_d
	.long set_0_e
	.long set_0_h
	.long set_0_l
	.long set_0_ihl
	.long set_0_a
	.long set_1_b
	.long set_1_c
	.long set_1_d
	.long set_1_e
	.long set_1_h
	.long set_1_l
	.long set_1_ihl
	.long set_1_a

	.long set_2_b		# D0
	.long set_2_c
	.long set_2_d
	.long set_2_e
	.long set_2_h
	.long set_2_l
	.long set_2_ihl
	.long set_2_a
	.long set_3_b
	.long set_3_c
	.long set_3_d
	.long set_3_e
	.long set_3_h
	.long set_3_l
	.long set_3_ihl
	.long set_3_a

	.long set_4_b		# E0
	.long set_4_c
	.long set_4_d
	.long set_4_e
	.long set_4_h
	.long set_4_l
	.long set_4_ihl
	.long set_4_a
	.long set_5_b
	.long set_5_c
	.long set_5_d
	.long set_5_e
	.long set_5_h
	.long set_5_l
	.long set_5_ihl
	.long set_5_a

	.long set_6_b		# F0
	.long set_6_c
	.long set_6_d
	.long set_6_e
	.long set_6_h
	.long set_6_l
	.long set_6_ihl
	.long set_6_a
	.long set_7_b
	.long set_7_c
	.long set_7_d
	.long set_7_e
	.long set_7_h
	.long set_7_l
	.long set_7_ihl
	.long set_7_a

	ALIGN

intr_table_dd_fd_cb:
	.long rlc_b		# 00
	.long rlc_c
	.long rlc_d
	.long rlc_e
	.long rlc_h
	.long rlc_l
	.long rlc_id
	.long rlc_a
	.long rrc_b
	.long rrc_c
	.long rrc_d
	.long rrc_e
	.long rrc_h
	.long rrc_l
	.long rrc_id
	.long rrc_a

	.long rl_b		# 10
	.long rl_c
	.long rl_d
	.long rl_e
	.long rl_h
	.long rl_l
	.long rl_id
	.long rl_a
	.long rr_b
	.long rr_c
	.long rr_d
	.long rr_e
	.long rr_h
	.long rr_l
	.long rr_id
	.long rr_a

	.long sla_b		# 20
	.long sla_c
	.long sla_d
	.long sla_e
	.long sla_h
	.long sla_l
	.long sla_id
	.long sla_a
	.long sra_b
	.long sra_c
	.long sra_d
	.long sra_e
	.long sra_h
	.long sra_l
	.long sra_id
	.long sra_a

	.long sll_b		# 30
	.long sll_c
	.long sll_d
	.long sll_e
	.long sll_h
	.long sll_l
	.long sll_id
	.long sll_a
	.long srl_b
	.long srl_c
	.long srl_d
	.long srl_e
	.long srl_h
	.long srl_l
	.long srl_id
	.long srl_a

	.long bit_0_b		# 40
	.long bit_0_c
	.long bit_0_d
	.long bit_0_e
	.long bit_0_h
	.long bit_0_l
	.long bit_0_id
	.long bit_0_a
	.long bit_1_b
	.long bit_1_c
	.long bit_1_d
	.long bit_1_e
	.long bit_1_h
	.long bit_1_l
	.long bit_1_id
	.long bit_1_a

	.long bit_2_b		# 50
	.long bit_2_c
	.long bit_2_d
	.long bit_2_e
	.long bit_2_h
	.long bit_2_l
	.long bit_2_id
	.long bit_2_a
	.long bit_3_b
	.long bit_3_c
	.long bit_3_d
	.long bit_3_e
	.long bit_3_h
	.long bit_3_l
	.long bit_3_id
	.long bit_3_a

	.long bit_4_b		# 60
	.long bit_4_c
	.long bit_4_d
	.long bit_4_e
	.long bit_4_h
	.long bit_4_l
	.long bit_4_id
	.long bit_4_a
	.long bit_5_b
	.long bit_5_c
	.long bit_5_d
	.long bit_5_e
	.long bit_5_h
	.long bit_5_l
	.long bit_5_id
	.long bit_5_a

	.long bit_6_b		# 70
	.long bit_6_c
	.long bit_6_d
	.long bit_6_e
	.long bit_6_h
	.long bit_6_l
	.long bit_6_id
	.long bit_6_a
	.long bit_7_b
	.long bit_7_c
	.long bit_7_d
	.long bit_7_e
	.long bit_7_h
	.long bit_7_l
	.long bit_7_id
	.long bit_7_a

	.long res_0_b		# 80
	.long res_0_c
	.long res_0_d
	.long res_0_e
	.long res_0_h
	.long res_0_l
	.long res_0_id
	.long res_0_a
	.long res_1_b
	.long res_1_c
	.long res_1_d
	.long res_1_e
	.long res_1_h
	.long res_1_l
	.long res_1_id
	.long res_1_a

	.long res_2_b		# 90
	.long res_2_c
	.long res_2_d
	.long res_2_e
	.long res_2_h
	.long res_2_l
	.long res_2_id
	.long res_2_a
	.long res_3_b
	.long res_3_c
	.long res_3_d
	.long res_3_e
	.long res_3_h
	.long res_3_l
	.long res_3_id
	.long res_3_a

	.long res_4_b		# A0
	.long res_4_c
	.long res_4_d
	.long res_4_e
	.long res_4_h
	.long res_4_l
	.long res_4_id
	.long res_4_a
	.long res_5_b
	.long res_5_c
	.long res_5_d
	.long res_5_e
	.long res_5_h
	.long res_5_l
	.long res_5_id
	.long res_5_a

	.long res_6_b		# B0
	.long res_6_c
	.long res_6_d
	.long res_6_e
	.long res_6_h
	.long res_6_l
	.long res_6_id
	.long res_6_a
	.long res_7_b
	.long res_7_c
	.long res_7_d
	.long res_7_e
	.long res_7_h
	.long res_7_l
	.long res_7_id
	.long res_7_a

	.long set_0_b		# C0
	.long set_0_c
	.long set_0_d
	.long set_0_e
	.long set_0_h
	.long set_0_l
	.long set_0_id
	.long set_0_a
	.long set_1_b
	.long set_1_c
	.long set_1_d
	.long set_1_e
	.long set_1_h
	.long set_1_l
	.long set_1_id
	.long set_1_a

	.long set_2_b		# D0
	.long set_2_c
	.long set_2_d
	.long set_2_e
	.long set_2_h
	.long set_2_l
	.long set_2_id
	.long set_2_a
	.long set_3_b
	.long set_3_c
	.long set_3_d
	.long set_3_e
	.long set_3_h
	.long set_3_l
	.long set_3_id
	.long set_3_a

	.long set_4_b		# E0
	.long set_4_c
	.long set_4_d
	.long set_4_e
	.long set_4_h
	.long set_4_l
	.long set_4_id
	.long set_4_a
	.long set_5_b
	.long set_5_c
	.long set_5_d
	.long set_5_e
	.long set_5_h
	.long set_5_l
	.long set_5_id
	.long set_5_a

	.long set_6_b		# F0
	.long set_6_c
	.long set_6_d
	.long set_6_e
	.long set_6_h
	.long set_6_l
	.long set_6_id
	.long set_6_a
	.long set_7_b
	.long set_7_c
	.long set_7_d
	.long set_7_e
	.long set_7_h
	.long set_7_l
	.long set_7_id
	.long set_7_a

	ALIGN

	.text

#endif
