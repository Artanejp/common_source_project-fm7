/*****************************************************************************
 *
 *  6502dasm.c
 *  6502/65c02/6510 disassembler
 *
 *  Copyright Juergen Buchmueller, all rights reserved.
 *
 *  - This source code is released as freeware for non-commercial purposes.
 *  - You are free to use and redistribute this code in modified or
 *    unmodified form, provided you list me in the credits.
 *  - If you modify this source code, you must add a notice to each modified
 *    source file that it has been changed.  If you're a nice person, you
 *    will clearly mark each change too.  :)
 *  - If you wish to use this for commercial purposes, please contact me at
 *    pullmoll@t-online.de
 *  - The author of this copywritten work reserves the right to change the
 *     terms of its usage and license at any time, including retroactively
 *   - This entire notice must remain in the source code.
 *
 *****************************************************************************/
/* 2. February 2000 PeT added support for 65sc02 subtype */
/* 2. February 2000 PeT added support for 65ce02 variant */
/* 3. February 2000 PeT bbr bbs displayment */
/* 4. February 2000 PeT ply inw dew */
/* 4. February 2000 PeT fixed relative word operand */
/* 9. May 2000 PeT added m4510 */
/* 18.April 2008 Roberto Fresca fixed bbr, bbs, rmb & smb displayment,
   showing the correct bit to operate */

//#include "emu.h"
//#include "debugger.h"
//#include "m6502.h"
//#include "m65ce02.h"
//#include "m6509.h"
//#include "m4510.h"

enum addr_mode {
	non,	/* no additional arguments */
	imp,	/* implicit */
	acc,	/* accumulator */
	imm,	/* immediate */
	iw2,	/* immediate word (65ce02) */
	iw3,	/* augment (65ce02) */
	adr,	/* absolute address (jmp,jsr) */
	aba,	/* absolute */
	zpg,	/* zero page */
	zpx,	/* zero page + X */
	zpy,	/* zero page + Y */
	zpi,	/* zero page indirect (65c02) */
	zpb,	/* zero page and branch (65c02 bbr,bbs) */
	abx,	/* absolute + X */
	aby,	/* absolute + Y */
	rel,	/* relative */
	rw2,	/* relative word (65cs02, 65ce02) */
	idx,	/* zero page pre indexed */
	idy,	/* zero page post indexed */
	idz,	/* zero page post indexed (65ce02) */
	isy,	/* zero page pre indexed sp and post indexed y (65ce02) */
	ind,	/* indirect (jmp) */
	iax		/* indirect + X (65c02 jmp) */
};

enum opcodes {
	adc,  and_,asl,  bcc,  bcs,  beq,  bit,  bmi,
	bne,  bpl, m6502_brk,  bvc,  bvs,  clc,  cld,  cli,
	clv,  cmp, cpx,  cpy,  dec,  dex,  dey,  eor,
	inc,  inx, iny,  jmp,  jsr,  lda,  ldx,  ldy,
	lsr,  nop, ora,  pha,  php,  pla,  plp,  rol,
	ror,  rti, rts,  sbc,  sec,  sed,  sei,  sta,
	stx,  sty, tax,  tay,  tsx,  txa,  txs,  tya,
	ill,
/* 65c02 (only) mnemonics */
	bbr,  bbs, bra,  rmb,  smb,  stz,  trb,  tsb,
/* 65sc02 (only) mnemonics */
	bsr,
/* 6510 + 65c02 mnemonics */
	anc,  asr, ast,  arr,  asx,  axa,  dcp,  dea,
	dop,  ina, isb,  lax,  phx,  phy,  plx,  ply,
	rla,  rra, sax,  slo,  sre,  sah,  say,  ssh,
	sxh,  syh, top,  oal,  kil,
/* 65ce02 mnemonics */
	cle,  see,  rtn,  aug,
	tab,  tba,	taz,  tza, tys, tsy,
	ldz,  stz2/* real register store */,
	dez,  inz,	cpz,  phz,	plz,
	neg,  asr2/* arithmetic shift right */,
	asw,  row,	dew,  inw,	phw,
/* 4510 mnemonics */
	map,

/* Deco CPU16 mnemonics */
	vbl,  u13,  u8F,  uAB,  u87,  u0B,  uA3,  u4B,
	u3F,  uBB,  u23
};


static const _TCHAR *const token[]=
{
	_T("adc"), _T("and"), _T("asl"), _T("bcc"), _T("bcs"), _T("beq"), _T("bit"), _T("bmi"),
	_T("bne"), _T("bpl"), _T("brk"), _T("bvc"), _T("bvs"), _T("clc"), _T("cld"), _T("cli"),
	_T("clv"), _T("cmp"), _T("cpx"), _T("cpy"), _T("dec"), _T("dex"), _T("dey"), _T("eor"),
	_T("inc"), _T("inx"), _T("iny"), _T("jmp"), _T("jsr"), _T("lda"), _T("ldx"), _T("ldy"),
	_T("lsr"), _T("nop"), _T("ora"), _T("pha"), _T("php"), _T("pla"), _T("plp"), _T("rol"),
	_T("ror"), _T("rti"), _T("rts"), _T("sbc"), _T("sec"), _T("sed"), _T("sei"), _T("sta"),
	_T("stx"), _T("sty"), _T("tax"), _T("tay"), _T("tsx"), _T("txa"), _T("txs"), _T("tya"),
	_T("ill"),
/* 65c02 mnemonics */
	_T("bbr"), _T("bbs"), _T("bra"), _T("rmb"), _T("smb"), _T("stz"), _T("trb"), _T("tsb"),
/* 65sc02 mnemonics */
	_T("bsr"),
/* 6510 mnemonics */
	_T("anc"), _T("asr"), _T("ast"), _T("arr"), _T("asx"), _T("axa"), _T("dcp"), _T("dea"),
	_T("dop"), _T("ina"), _T("isb"), _T("lax"), _T("phx"), _T("phy"), _T("plx"), _T("ply"),
	_T("rla"), _T("rra"), _T("sax"), _T("slo"), _T("sre"), _T("sah"), _T("say"), _T("ssh"),
	_T("sxh"), _T("syh"), _T("top"), _T("oal"), _T("kil"),
/* 65ce02 mnemonics */
	_T("cle"), _T("see"), _T("rtn"), _T("aug"),
	_T("tab"), _T("tba"), _T("taz"), _T("tza"), _T("tys"), _T("tsy"),
	_T("ldz"), _T("stz"),
	_T("dez"), _T("inz"), _T("cpz"), _T("phz"), _T("plz"),
	_T("neg"), _T("asr"),
	_T("asw"), _T("row"), _T("dew"), _T("inw"), _T("phw"),
/* 4510 mnemonics */
	_T("map"),

/* Deco CPU16 mnemonics */
	_T("VBL"), _T("u13"), _T("u8F"), _T("uAB"), _T("u87"), _T("u0B"), _T("uA3"), _T("u4B"),
	_T("u3F"), _T("uBB"), _T("u23")
};

struct op6502_info
{
	UINT8 opc;
	UINT8 arg;
};

static const struct op6502_info op6502[256] = {
	{m6502_brk,imm},{ora,idx},{kil,non},{slo,idx},/* 00 */
	{nop,zpg},{ora,zpg},{asl,zpg},{slo,zpg},
	{php,imp},{ora,imm},{asl,acc},{anc,imm},
	{nop,aba},{ora,aba},{asl,aba},{slo,aba},
	{bpl,rel},{ora,idy},{kil,non},{slo,idy},/* 10 */
	{nop,zpx},{ora,zpx},{asl,zpx},{slo,zpx},
	{clc,imp},{ora,aby},{nop,imp},{slo,aby},
	{nop,abx},{ora,abx},{asl,abx},{slo,abx},
	{jsr,adr},{and_,idx},{kil,non},{rla,idx},/* 20 */
	{bit,zpg},{and_,zpg},{rol,zpg},{rla,zpg},
	{plp,imp},{and_,imm},{rol,acc},{anc,imm},
	{bit,aba},{and_,aba},{rol,aba},{rla,aba},
	{bmi,rel},{and_,idy},{kil,non},{rla,idy},/* 30 */
	{nop,zpx},{and_,zpx},{rol,zpx},{rla,zpx},
	{sec,imp},{and_,aby},{nop,imp},{rla,aby},
	{nop,abx},{and_,abx},{rol,abx},{rla,abx},
	{rti,imp},{eor,idx},{kil,non},{sre,idx},/* 40 */
	{nop,zpg},{eor,zpg},{lsr,zpg},{sre,zpg},
	{pha,imp},{eor,imm},{lsr,acc},{asr,imm},
	{jmp,adr},{eor,aba},{lsr,aba},{sre,aba},
	{bvc,rel},{eor,idy},{kil,non},{sre,idy},/* 50 */
	{nop,zpx},{eor,zpx},{lsr,zpx},{sre,zpx},
	{cli,imp},{eor,aby},{nop,imp},{sre,aby},
	{nop,abx},{eor,abx},{lsr,abx},{sre,abx},
	{rts,imp},{adc,idx},{kil,non},{rra,idx},/* 60 */
	{nop,zpg},{adc,zpg},{ror,zpg},{rra,zpg},
	{pla,imp},{adc,imm},{ror,acc},{arr,imm},
	{jmp,ind},{adc,aba},{ror,aba},{rra,aba},
	{bvs,rel},{adc,idy},{kil,non},{rra,idy},/* 70 */
	{nop,zpx},{adc,zpx},{ror,zpx},{rra,zpx},
	{sei,imp},{adc,aby},{nop,imp},{rra,aby},
	{nop,abx},{adc,abx},{ror,abx},{rra,abx},
	{nop,imm},{sta,idx},{nop,imm},{sax,idx},/* 80 */
	{sty,zpg},{sta,zpg},{stx,zpg},{sax,zpg},
	{dey,imp},{nop,imm},{txa,imp},{axa,imm},
	{sty,aba},{sta,aba},{stx,aba},{sax,aba},
	{bcc,rel},{sta,idy},{kil,non},{say,idy},/* 90 */
	{sty,zpx},{sta,zpx},{stx,zpy},{sax,zpy},
	{tya,imp},{sta,aby},{txs,imp},{ssh,aby},
	{syh,abx},{sta,abx},{sxh,aby},{sah,aby},
	{ldy,imm},{lda,idx},{ldx,imm},{lax,idx},/* a0 */
	{ldy,zpg},{lda,zpg},{ldx,zpg},{lax,zpg},
	{tay,imp},{lda,imm},{tax,imp},{oal,imm},
	{ldy,aba},{lda,aba},{ldx,aba},{lax,aba},
	{bcs,rel},{lda,idy},{kil,non},{lax,idy},/* b0 */
	{ldy,zpx},{lda,zpx},{ldx,zpy},{lax,zpy},
	{clv,imp},{lda,aby},{tsx,imp},{ast,aby},
	{ldy,abx},{lda,abx},{ldx,aby},{lax,aby},
	{cpy,imm},{cmp,idx},{nop,imm},{dcp,idx},/* c0 */
	{cpy,zpg},{cmp,zpg},{dec,zpg},{dcp,zpg},
	{iny,imp},{cmp,imm},{dex,imp},{asx,imm},
	{cpy,aba},{cmp,aba},{dec,aba},{dcp,aba},
	{bne,rel},{cmp,idy},{kil,non},{dcp,idy},/* d0 */
	{nop,zpx},{cmp,zpx},{dec,zpx},{dcp,zpx},
	{cld,imp},{cmp,aby},{nop,imp},{dcp,aby},
	{nop,abx},{cmp,abx},{dec,abx},{dcp,abx},
	{cpx,imm},{sbc,idx},{nop,imm},{isb,idx},/* e0 */
	{cpx,zpg},{sbc,zpg},{inc,zpg},{isb,zpg},
	{inx,imp},{sbc,imm},{nop,imp},{sbc,imm},
	{cpx,aba},{sbc,aba},{inc,aba},{isb,aba},
	{beq,rel},{sbc,idy},{kil,non},{isb,idy},/* f0 */
	{nop,zpx},{sbc,zpx},{inc,zpx},{isb,zpx},
	{sed,imp},{sbc,aby},{nop,imp},{isb,aby},
	{nop,abx},{sbc,abx},{inc,abx},{isb,abx}
};

static const struct op6502_info op65c02[256] = {
	{m6502_brk,imm},{ora,idx},{ill,non},{ill,non},/* 00 */
	{tsb,zpg},{ora,zpg},{asl,zpg},{rmb,zpg},
	{php,imp},{ora,imm},{asl,acc},{ill,non},
	{tsb,aba},{ora,aba},{asl,aba},{bbr,zpb},
	{bpl,rel},{ora,idy},{ora,zpi},{ill,non},/* 10 */
	{trb,zpg},{ora,zpx},{asl,zpx},{rmb,zpg},
	{clc,imp},{ora,aby},{ina,imp},{ill,non},
	{trb,aba},{ora,abx},{asl,abx},{bbr,zpb},
	{jsr,adr},{and_,idx},{ill,non},{ill,non},/* 20 */
	{bit,zpg},{and_,zpg},{rol,zpg},{rmb,zpg},
	{plp,imp},{and_,imm},{rol,acc},{ill,non},
	{bit,aba},{and_,aba},{rol,aba},{bbr,zpb},
	{bmi,rel},{and_,idy},{and_,zpi},{ill,non},/* 30 */
	{bit,zpx},{and_,zpx},{rol,zpx},{rmb,zpg},
	{sec,imp},{and_,aby},{dea,imp},{ill,non},
	{bit,abx},{and_,abx},{rol,abx},{bbr,zpb},
	{rti,imp},{eor,idx},{ill,non},{ill,non},/* 40 */
	{ill,non},{eor,zpg},{lsr,zpg},{rmb,zpg},
	{pha,imp},{eor,imm},{lsr,acc},{ill,non},
	{jmp,adr},{eor,aba},{lsr,aba},{bbr,zpb},
	{bvc,rel},{eor,idy},{eor,zpi},{ill,non},/* 50 */
	{ill,non},{eor,zpx},{lsr,zpx},{rmb,zpg},
	{cli,imp},{eor,aby},{phy,imp},{ill,non},
	{ill,non},{eor,abx},{lsr,abx},{bbr,zpb},
	{rts,imp},{adc,idx},{ill,non},{ill,non},/* 60 */
	{stz,zpg},{adc,zpg},{ror,zpg},{rmb,zpg},
	{pla,imp},{adc,imm},{ror,acc},{ill,non},
	{jmp,ind},{adc,aba},{ror,aba},{bbr,zpb},
	{bvs,rel},{adc,idy},{adc,zpi},{ill,non},/* 70 */
	{stz,zpx},{adc,zpx},{ror,zpx},{rmb,zpg},
	{sei,imp},{adc,aby},{ply,imp},{ill,non},
	{jmp,iax},{adc,abx},{ror,abx},{bbr,zpb},
	{bra,rel},{sta,idx},{ill,non},{ill,non},/* 80 */
	{sty,zpg},{sta,zpg},{stx,zpg},{smb,zpg},
	{dey,imp},{bit,imm},{txa,imp},{ill,non},
	{sty,aba},{sta,aba},{stx,aba},{bbs,zpb},
	{bcc,rel},{sta,idy},{sta,zpi},{ill,non},/* 90 */
	{sty,zpx},{sta,zpx},{stx,zpy},{smb,zpg},
	{tya,imp},{sta,aby},{txs,imp},{ill,non},
	{stz,aba},{sta,abx},{stz,abx},{bbs,zpb},
	{ldy,imm},{lda,idx},{ldx,imm},{ill,non},/* a0 */
	{ldy,zpg},{lda,zpg},{ldx,zpg},{smb,zpg},
	{tay,imp},{lda,imm},{tax,imp},{ill,non},
	{ldy,aba},{lda,aba},{ldx,aba},{bbs,zpb},
	{bcs,rel},{lda,idy},{lda,zpi},{ill,non},/* b0 */
	{ldy,zpx},{lda,zpx},{ldx,zpy},{smb,zpg},
	{clv,imp},{lda,aby},{tsx,imp},{ill,non},
	{ldy,abx},{lda,abx},{ldx,aby},{bbs,zpb},
	{cpy,imm},{cmp,idx},{ill,non},{ill,non},/* c0 */
	{cpy,zpg},{cmp,zpg},{dec,zpg},{smb,zpg},
	{iny,imp},{cmp,imm},{dex,imp},{ill,non},
	{cpy,aba},{cmp,aba},{dec,aba},{bbs,zpb},
	{bne,rel},{cmp,idy},{cmp,zpi},{ill,non},/* d0 */
	{ill,non},{cmp,zpx},{dec,zpx},{smb,zpg},
	{cld,imp},{cmp,aby},{phx,imp},{ill,non},
	{ill,non},{cmp,abx},{dec,abx},{bbs,zpb},
	{cpx,imm},{sbc,idx},{ill,non},{ill,non},/* e0 */
	{cpx,zpg},{sbc,zpg},{inc,zpg},{smb,zpg},
	{inx,imp},{sbc,imm},{nop,imp},{ill,non},
	{cpx,aba},{sbc,aba},{inc,aba},{bbs,zpb},
	{beq,rel},{sbc,idy},{sbc,zpi},{ill,non},/* f0 */
	{ill,non},{sbc,zpx},{inc,zpx},{smb,zpg},
	{sed,imp},{sbc,aby},{plx,imp},{ill,non},
	{ill,non},{sbc,abx},{inc,abx},{bbs,zpb}
};

/* only bsr additional to 65c02 yet */
static const struct op6502_info op65sc02[256] = {
	{m6502_brk,imm},{ora,idx},{ill,non},{ill,non},/* 00 */
	{tsb,zpg},{ora,zpg},{asl,zpg},{rmb,zpg},
	{php,imp},{ora,imm},{asl,acc},{ill,non},
	{tsb,aba},{ora,aba},{asl,aba},{bbr,zpb},
	{bpl,rel},{ora,idy},{ora,zpi},{ill,non},/* 10 */
	{trb,zpg},{ora,zpx},{asl,zpx},{rmb,zpg},
	{clc,imp},{ora,aby},{ina,imp},{ill,non},
	{trb,aba},{ora,abx},{asl,abx},{bbr,zpb},
	{jsr,adr},{and_,idx},{ill,non},{ill,non},/* 20 */
	{bit,zpg},{and_,zpg},{rol,zpg},{rmb,zpg},
	{plp,imp},{and_,imm},{rol,acc},{ill,non},
	{bit,aba},{and_,aba},{rol,aba},{bbr,zpb},
	{bmi,rel},{and_,idy},{and_,zpi},{ill,non},/* 30 */
	{bit,zpx},{and_,zpx},{rol,zpx},{rmb,zpg},
	{sec,imp},{and_,aby},{dea,imp},{ill,non},
	{bit,abx},{and_,abx},{rol,abx},{bbr,zpb},
	{rti,imp},{eor,idx},{ill,non},{ill,non},/* 40 */
	{ill,non},{eor,zpg},{lsr,zpg},{rmb,zpg},
	{pha,imp},{eor,imm},{lsr,acc},{ill,non},
	{jmp,adr},{eor,aba},{lsr,aba},{bbr,zpb},
	{bvc,rel},{eor,idy},{eor,zpi},{ill,non},/* 50 */
	{ill,non},{eor,zpx},{lsr,zpx},{rmb,zpg},
	{cli,imp},{eor,aby},{phy,imp},{ill,non},
	{ill,non},{eor,abx},{lsr,abx},{bbr,zpb},
	{rts,imp},{adc,idx},{ill,non},{bsr,rw2},/* 60 */
	{stz,zpg},{adc,zpg},{ror,zpg},{rmb,zpg},
	{pla,imp},{adc,imm},{ror,acc},{ill,non},
	{jmp,ind},{adc,aba},{ror,aba},{bbr,zpb},
	{bvs,rel},{adc,idy},{adc,zpi},{ill,non},/* 70 */
	{stz,zpx},{adc,zpx},{ror,zpx},{rmb,zpg},
	{sei,imp},{adc,aby},{ply,imp},{ill,non},
	{jmp,iax},{adc,abx},{ror,abx},{bbr,zpb},
	{bra,rel},{sta,idx},{ill,non},{ill,non},/* 80 */
	{sty,zpg},{sta,zpg},{stx,zpg},{smb,zpg},
	{dey,imp},{bit,imm},{txa,imp},{ill,non},
	{sty,aba},{sta,aba},{stx,aba},{bbs,zpb},
	{bcc,rel},{sta,idy},{sta,zpi},{ill,non},/* 90 */
	{sty,zpx},{sta,zpx},{stx,zpy},{smb,zpg},
	{tya,imp},{sta,aby},{txs,imp},{ill,non},
	{stz,aba},{sta,abx},{stz,abx},{bbs,zpb},
	{ldy,imm},{lda,idx},{ldx,imm},{ill,non},/* a0 */
	{ldy,zpg},{lda,zpg},{ldx,zpg},{smb,zpg},
	{tay,imp},{lda,imm},{tax,imp},{ill,non},
	{ldy,aba},{lda,aba},{ldx,aba},{bbs,zpb},
	{bcs,rel},{lda,idy},{lda,zpi},{ill,non},/* b0 */
	{ldy,zpx},{lda,zpx},{ldx,zpy},{smb,zpg},
	{clv,imp},{lda,aby},{tsx,imp},{ill,non},
	{ldy,abx},{lda,abx},{ldx,aby},{bbs,zpb},
	{cpy,imm},{cmp,idx},{ill,non},{ill,non},/* c0 */
	{cpy,zpg},{cmp,zpg},{dec,zpg},{smb,zpg},
	{iny,imp},{cmp,imm},{dex,imp},{ill,non},
	{cpy,aba},{cmp,aba},{dec,aba},{bbs,zpb},
	{bne,rel},{cmp,idy},{cmp,zpi},{ill,non},/* d0 */
	{ill,non},{cmp,zpx},{dec,zpx},{smb,zpg},
	{cld,imp},{cmp,aby},{phx,imp},{ill,non},
	{ill,non},{cmp,abx},{dec,abx},{bbs,zpb},
	{cpx,imm},{sbc,idx},{ill,non},{ill,non},/* e0 */
	{cpx,zpg},{sbc,zpg},{inc,zpg},{smb,zpg},
	{inx,imp},{sbc,imm},{nop,imp},{ill,non},
	{cpx,aba},{sbc,aba},{inc,aba},{bbs,zpb},
	{beq,rel},{sbc,idy},{sbc,zpi},{ill,non},/* f0 */
	{ill,non},{sbc,zpx},{inc,zpx},{smb,zpg},
	{sed,imp},{sbc,aby},{plx,imp},{ill,non},
	{ill,non},{sbc,abx},{inc,abx},{bbs,zpb}
};

static const struct op6502_info op65ce02[256] = {
	{m6502_brk,imm},{ora,idx},{cle,imp},{see,imp},/* 00 */
	{tsb,zpg},{ora,zpg},{asl,zpg},{rmb,zpg},
	{php,imp},{ora,imm},{asl,acc},{tsy,imp},
	{tsb,aba},{ora,aba},{asl,aba},{bbr,zpb},
	{bpl,rel},{ora,idy},{ora,idz},{bpl,rw2},/* 10 */
	{trb,zpg},{ora,zpx},{asl,zpx},{rmb,zpg},
	{clc,imp},{ora,aby},{ina,imp},{inz,imp},
	{trb,aba},{ora,abx},{asl,abx},{bbr,zpb},
	{jsr,adr},{and_,idx},{jsr,ind},{jsr,iax},/* 20 */
	{bit,zpg},{and_,zpg},{rol,zpg},{rmb,zpg},
	{plp,imp},{and_,imm},{rol,acc},{tys,imp},
	{bit,aba},{and_,aba},{rol,aba},{bbr,zpb},
	{bmi,rel},{and_,idz},{and_,zpi},{bmi,rw2},/* 30 */
	{bit,zpx},{and_,zpx},{rol,zpx},{rmb,zpg},
	{sec,imp},{and_,aby},{dea,imp},{dez,imp},
	{bit,abx},{and_,abx},{rol,abx},{bbr,zpb},
	{rti,imp},{eor,idx},{neg,imp},{asr2,imp},/* 40 */
	{asr2,zpg},{eor,zpg},{lsr,zpg},{rmb,zpg},
	{pha,imp},{eor,imm},{lsr,acc},{taz,imp},
	{jmp,adr},{eor,aba},{lsr,aba},{bbr,zpb},
	{bvc,rel},{eor,idy},{eor,idz},{bvc,rw2},/* 50 */
	{asr2,zpx},{eor,zpx},{lsr,zpx},{rmb,zpg},
	{cli,imp},{eor,aby},{phy,imp},{tab,imp},
	{aug,iw3},{eor,abx},{lsr,abx},{bbr,zpb},
	{rts,imp},{adc,idx},{rtn,imm},{bsr,rw2},/* 60 */
	{stz2,zpg},{adc,zpg},{ror,zpg},{rmb,zpg},
	{pla,imp},{adc,imm},{ror,acc},{tza,imp},
	{jmp,ind},{adc,aba},{ror,aba},{bbr,zpb},
	{bvs,rel},{adc,idy},{adc,zpi},{bvs,rw2},/* 70 */
	{stz2,zpx},{adc,zpx},{ror,zpx},{rmb,zpg},
	{sei,imp},{adc,aby},{ply,imp},{tba,imp},
	{jmp,iax},{adc,abx},{ror,abx},{bbr,zpb},
	{bra,rel},{sta,idx},{sta,isy},{bra,rw2},/* 80 */
	{sty,zpg},{sta,zpg},{stx,zpg},{smb,zpg},
	{dey,imp},{bit,imm},{txa,imp},{sty,abx},
	{sty,aba},{sta,aba},{stx,aba},{bbs,zpb},
	{bcc,rel},{sta,idy},{sta,inz},{bcc,rw2},/* 90 */
	{sty,zpx},{sta,zpx},{stx,zpy},{smb,zpg},
	{tya,imp},{sta,aby},{txs,imp},{stx,aby},
	{stz2,aba},{sta,abx},{stz2,abx},{bbs,zpb},
	{ldy,imm},{lda,idx},{ldx,imm},{ldz,imm},/* a0 */
	{ldy,zpg},{lda,zpg},{ldx,zpg},{smb,zpg},
	{tay,imp},{lda,imm},{tax,imp},{ldz,aba},
	{ldy,aba},{lda,aba},{ldx,aba},{bbs,zpb},
	{bcs,rel},{lda,idy},{lda,inz},{bcs,rw2},/* b0 */
	{ldy,zpx},{lda,zpx},{ldx,zpy},{smb,zpg},
	{clv,imp},{lda,aby},{tsx,imp},{ldz,abx},
	{ldy,abx},{lda,abx},{ldx,aby},{bbs,zpb},
	{cpy,imm},{cmp,idx},{cpz,imm},{dew,zpg},/* c0 */
	{cpy,zpg},{cmp,zpg},{dec,zpg},{smb,zpg},
	{iny,imp},{cmp,imm},{dex,imp},{asw,aba},
	{cpy,aba},{cmp,aba},{dec,aba},{bbs,zpb},
	{bne,rel},{cmp,idy},{cmp,idz},{bne,rw2},/* d0 */
	{cpz,zpg},{cmp,zpx},{dec,zpx},{smb,zpg},
	{cld,imp},{cmp,aby},{phx,imp},{phz,imp},
	{cpz,aba},{cmp,abx},{dec,abx},{bbs,zpb},
	{cpx,imm},{sbc,idx},{lda,isy},{inw,zpg},/* e0 */
	{cpx,zpg},{sbc,zpg},{inc,zpg},{smb,zpg},
	{inx,imp},{sbc,imm},{nop,imp},{row,aba},
	{cpx,aba},{sbc,aba},{inc,aba},{bbs,zpb},
	{beq,rel},{sbc,idy},{sbc,idz},{beq,rw2},/* f0 */
	{phw,iw2},{sbc,zpx},{inc,zpx},{smb,zpg},
	{sed,imp},{sbc,aby},{plx,imp},{plz,imp},
	{phw,aba},{sbc,abx},{inc,abx},{bbs,zpb}
};

// only map instead of aug and 20 bit memory management
static const struct op6502_info op4510[256] = {
	{m6502_brk,imm},{ora,idx},{cle,imp},{see,imp},/* 00 */
	{tsb,zpg},{ora,zpg},{asl,zpg},{rmb,zpg},
	{php,imp},{ora,imm},{asl,acc},{tsy,imp},
	{tsb,aba},{ora,aba},{asl,aba},{bbr,zpb},
	{bpl,rel},{ora,idy},{ora,idz},{bpl,rw2},/* 10 */
	{trb,zpg},{ora,zpx},{asl,zpx},{rmb,zpg},
	{clc,imp},{ora,aby},{ina,imp},{inz,imp},
	{trb,aba},{ora,abx},{asl,abx},{bbr,zpb},
	{jsr,adr},{and_,idx},{jsr,ind},{jsr,iax},/* 20 */
	{bit,zpg},{and_,zpg},{rol,zpg},{rmb,zpg},
	{plp,imp},{and_,imm},{rol,acc},{tys,imp},
	{bit,aba},{and_,aba},{rol,aba},{bbr,zpb},
	{bmi,rel},{and_,idz},{and_,zpi},{bmi,rw2},/* 30 */
	{bit,zpx},{and_,zpx},{rol,zpx},{rmb,zpg},
	{sec,imp},{and_,aby},{dea,imp},{dez,imp},
	{bit,abx},{and_,abx},{rol,abx},{bbr,zpb},
	{rti,imp},{eor,idx},{neg,imp},{asr2,imp},/* 40 */
	{asr2,zpg},{eor,zpg},{lsr,zpg},{rmb,zpg},
	{pha,imp},{eor,imm},{lsr,acc},{taz,imp},
	{jmp,adr},{eor,aba},{lsr,aba},{bbr,zpb},
	{bvc,rel},{eor,idy},{eor,idz},{bvc,rw2},/* 50 */
	{asr2,zpx},{eor,zpx},{lsr,zpx},{rmb,zpg},
	{cli,imp},{eor,aby},{phy,imp},{tab,imp},
	{map,imp},{eor,abx},{lsr,abx},{bbr,zpb},
	{rts,imp},{adc,idx},{rtn,imm},{bsr,rw2},/* 60 */
	{stz2,zpg},{adc,zpg},{ror,zpg},{rmb,zpg},
	{pla,imp},{adc,imm},{ror,acc},{tza,imp},
	{jmp,ind},{adc,aba},{ror,aba},{bbr,zpb},
	{bvs,rel},{adc,idy},{adc,zpi},{bvs,rw2},/* 70 */
	{stz2,zpx},{adc,zpx},{ror,zpx},{rmb,zpg},
	{sei,imp},{adc,aby},{ply,imp},{tba,imp},
	{jmp,iax},{adc,abx},{ror,abx},{bbr,zpb},
	{bra,rel},{sta,idx},{sta,isy},{bra,rw2},/* 80 */
	{sty,zpg},{sta,zpg},{stx,zpg},{smb,zpg},
	{dey,imp},{bit,imm},{txa,imp},{sty,abx},
	{sty,aba},{sta,aba},{stx,aba},{bbs,zpb},
	{bcc,rel},{sta,idy},{sta,inz},{bcc,rw2},/* 90 */
	{sty,zpx},{sta,zpx},{stx,zpy},{smb,zpg},
	{tya,imp},{sta,aby},{txs,imp},{stx,aby},
	{stz2,aba},{sta,abx},{stz2,abx},{bbs,zpb},
	{ldy,imm},{lda,idx},{ldx,imm},{ldz,imm},/* a0 */
	{ldy,zpg},{lda,zpg},{ldx,zpg},{smb,zpg},
	{tay,imp},{lda,imm},{tax,imp},{ldz,aba},
	{ldy,aba},{lda,aba},{ldx,aba},{bbs,zpb},
	{bcs,rel},{lda,idy},{lda,inz},{bcs,rw2},/* b0 */
	{ldy,zpx},{lda,zpx},{ldx,zpy},{smb,zpg},
	{clv,imp},{lda,aby},{tsx,imp},{ldz,abx},
	{ldy,abx},{lda,abx},{ldx,aby},{bbs,zpb},
	{cpy,imm},{cmp,idx},{cpz,imm},{dew,zpg},/* c0 */
	{cpy,zpg},{cmp,zpg},{dec,zpg},{smb,zpg},
	{iny,imp},{cmp,imm},{dex,imp},{asw,aba},
	{cpy,aba},{cmp,aba},{dec,aba},{bbs,zpb},
	{bne,rel},{cmp,idy},{cmp,idz},{bne,rw2},/* d0 */
	{cpz,zpg},{cmp,zpx},{dec,zpx},{smb,zpg},
	{cld,imp},{cmp,aby},{phx,imp},{phz,imp},
	{cpz,aba},{cmp,abx},{dec,abx},{bbs,zpb},
	{cpx,imm},{sbc,idx},{lda,isy},{inw,zpg},/* e0 */
	{cpx,zpg},{sbc,zpg},{inc,zpg},{smb,zpg},
	{inx,imp},{sbc,imm},{nop,imp},{row,aba},
	{cpx,aba},{sbc,aba},{inc,aba},{bbs,zpb},
	{beq,rel},{sbc,idy},{sbc,idz},{beq,rw2},/* f0 */
	{phw,iw2},{sbc,zpx},{inc,zpx},{smb,zpg},
	{sed,imp},{sbc,aby},{plx,imp},{plz,imp},
	{phw,aba},{sbc,abx},{inc,abx},{bbs,zpb}
};

static const struct op6502_info opdeco16[256] =
{
	{m6502_brk,imp},{ora,idx},{ill,non},{ill,non},/* 00 */
	{ill,non},{ora,zpg},{asl,zpg},{ill,non},
	{php,imp},{ora,imm},{asl,acc},{u0B,zpg},
	{ill,non},{ora,aba},{asl,aba},{ill,non},
	{bpl,rel},{ora,idy},{ill,non},{u13,zpg},/* 10 */
	{ill,non},{ora,zpx},{asl,zpx},{ill,non},
	{clc,imp},{ora,aby},{ill,non},{ill,non},
	{ill,non},{ora,abx},{asl,abx},{ill,non},
	{jsr,adr},{and_,idx},{ill,non},{u23,zpg},/* 20 */
	{bit,zpg},{and_,zpg},{rol,zpg},{ill,non},
	{plp,imp},{and_,imm},{rol,acc},{ill,non},
	{bit,aba},{and_,aba},{rol,aba},{ill,non},
	{bmi,rel},{and_,idy},{ill,non},{ill,non},/* 30 */
	{ill,non},{and_,zpx},{rol,zpx},{ill,non},
	{sec,imp},{and_,aby},{ill,non},{ill,non},
	{ill,non},{and_,abx},{rol,abx},{u3F,zpg},
	{rti,imp},{eor,idx},{ill,non},{ill,non},/* 40 */
	{ill,non},{eor,zpg},{lsr,zpg},{ill,non},
	{pha,imp},{eor,imm},{lsr,acc},{u4B,zpg},
	{jmp,adr},{eor,aba},{lsr,aba},{ill,non},
	{bvc,rel},{eor,idy},{ill,non},{ill,non},/* 50 */
	{ill,non},{eor,zpx},{lsr,zpx},{ill,non},
	{cli,imp},{eor,aby},{ill,non},{ill,non},
	{ill,non},{eor,abx},{lsr,abx},{ill,non},
	{rts,imp},{adc,idx},{ill,non},{ill,non},/* 60 */
	{ill,non},{adc,zpg},{ror,zpg},{vbl,zpg},		// MISH
	{pla,imp},{adc,imm},{ror,acc},{ill,non},
	{jmp,ind},{adc,aba},{ror,aba},{ill,non},
	{bvs,rel},{adc,idy},{ill,non},{ill,non},/* 70 */
	{ill,non},{adc,zpx},{ror,zpx},{ill,non},
	{sei,imp},{adc,aby},{ill,non},{ill,non},
	{ill,non},{adc,abx},{ror,abx},{ill,non},
	{ill,non},{sta,idx},{ill,non},{ill,non},/* 80 */
	{sty,zpg},{sta,zpg},{stx,zpg},{u87,zpg},
	{dey,imp},{ill,non},{txa,imp},{ill,non},
	{sty,aba},{sta,aba},{stx,aba},{u8F,zpg},
	{bcc,rel},{sta,idy},{ill,non},{ill,non},/* 90 */
	{sty,zpx},{sta,zpx},{stx,zpy},{ill,non},
	{tya,imp},{sta,aby},{txs,imp},{ill,non},
	{ill,non},{sta,abx},{ill,non},{ill,non},
	{ldy,imm},{lda,idx},{ldx,imm},{uA3,zpg},/* a0 */
	{ldy,zpg},{lda,zpg},{ldx,zpg},{ill,non},
	{tay,imp},{lda,imm},{tax,imp},{uAB,zpg},
	{ldy,aba},{lda,aba},{ldx,aba},{ill,non},
	{bcs,rel},{lda,idy},{ill,non},{ill,non},/* b0 */
	{ldy,zpx},{lda,zpx},{ldx,zpy},{ill,non},
	{clv,imp},{lda,aby},{tsx,imp},{uBB,zpg},
	{ldy,abx},{lda,abx},{ldx,aby},{ill,non},
	{cpy,imm},{cmp,idx},{ill,non},{ill,non},/* c0 */
	{cpy,zpg},{cmp,zpg},{dec,zpg},{ill,non},
	{iny,imp},{cmp,imm},{dex,imp},{ill,non},
	{cpy,aba},{cmp,aba},{dec,aba},{ill,non},
	{bne,rel},{cmp,idy},{ill,non},{ill,non},/* d0 */
	{ill,non},{cmp,zpx},{dec,zpx},{ill,non},
	{cld,imp},{cmp,aby},{ill,non},{ill,non},
	{ill,non},{cmp,abx},{dec,abx},{ill,non},
	{cpx,imm},{sbc,idx},{ill,non},{ill,non},/* e0 */
	{cpx,zpg},{sbc,zpg},{inc,zpg},{ill,non},
	{inx,imp},{sbc,imm},{nop,imp},{ill,non},
	{cpx,aba},{sbc,aba},{inc,aba},{ill,non},
	{beq,rel},{sbc,idy},{ill,non},{ill,non},/* f0 */
	{ill,non},{sbc,zpx},{inc,zpx},{ill,non},
	{sed,imp},{sbc,aby},{ill,non},{ill,non},
	{ill,non},{sbc,abx},{inc,abx},{ill,non}
};

/*****************************************************************************
 * Disassemble a single opcode starting at pc
 *****************************************************************************/
static unsigned internal_m6502_dasm(const struct op6502_info *opinfo, _TCHAR *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram, symbol_t *first_symbol)
{
	_TCHAR *dst = buffer;
	INT8 offset;
	INT16 offset16;
	unsigned PC = pc;
	UINT16 addr;
	UINT8 op, opc, arg, value;
	UINT32 flags;
	int pos = 0;

	op = oprom[pos++];
	pc++;

	opc = opinfo[op].opc;
	arg = opinfo[op].arg;

	/* determine dasmflags */
	switch(opc)
	{
		case jsr:
		case bsr:
			flags = DASMFLAG_SUPPORTED | DASMFLAG_STEP_OVER;
			break;

		case rts:
		case rti:
		case rtn:
			flags = DASMFLAG_SUPPORTED | DASMFLAG_STEP_OUT;
			break;

		default:
			flags = DASMFLAG_SUPPORTED;
			break;
	}

	dst += _stprintf(dst, _T("%-5s"), token[opc]);
	if( opc == bbr || opc == bbs || opc == rmb || opc == smb )
		dst += _stprintf(dst, _T("%d,"), (op >> 4) & 7);

	switch(arg)
	{
	case imp:
		break;

	case acc:
		dst += _stprintf(dst, _T("a"));
		break;

	case rel:
		offset = (INT8) opram[pos++];
		pc++;
		dst += _stprintf(dst, _T("%s"), get_value_or_symbol(first_symbol, _T("$%04X"), (pc + offset) & 0xFFFF));
		break;

	case rw2:
		offset16 = (opram[pos] | (opram[pos+1] << 8)) -1;
		pos += 2;
		pc += 2;
		dst += _stprintf(dst, _T("%s"), get_value_or_symbol(first_symbol, _T("$%04X"), (pc + offset16) & 0xFFFF));
		break;

	case imm:
		value = opram[pos++];
		pc++;
		dst += _stprintf(dst, _T("#$%02X"), value);
		break;

	case zpg:
		addr = opram[pos++];
		pc++;
		dst += _stprintf(dst, _T("$%02X"), addr);
		break;

	case zpx:
		addr = opram[pos++];
		pc++;
		dst += _stprintf(dst, _T("$%02X,x"), addr);
		break;

	case zpy:
		addr = opram[pos++];
		pc++;
		dst += _stprintf(dst, _T("$%02X,y"), addr);
		break;

	case idx:
		addr = opram[pos++];
		pc++;
		dst += _stprintf(dst, _T("($%02X,x)"), addr);
		break;

	case idy:
		addr = opram[pos++];
		pc++;
		dst += _stprintf(dst, _T("($%02X),y"), addr);
		break;

	case zpi:
		addr = opram[pos++];
		pc++;
		dst += _stprintf(dst, _T("($%02X)"), addr);
		break;

	case zpb:
		addr = opram[pos++];
		pc++;
		dst += _stprintf(dst, _T("$%02X"), addr);
		offset = (INT8) opram[pos++];
		pc++;
		dst += _stprintf(dst, _T(",%s"), get_value_or_symbol(first_symbol, _T("$%04X"), pc + offset));
		break;

	case adr:
		addr = (opram[pos] | (opram[pos+1] << 8));
		pos += 2;
		pc += 2;
		dst += _stprintf(dst, _T("%s"), get_value_or_symbol(first_symbol, _T("$%04X"), addr));
		break;

	case aba:
		addr = (opram[pos] | (opram[pos+1] << 8));
		pos += 2;
		pc += 2;
		dst += _stprintf(dst, _T("%s"), get_value_or_symbol(first_symbol, _T("$%04X"), addr));
		break;

	case abx:
		addr = (opram[pos] | (opram[pos+1] << 8));
		pos += 2;
		pc += 2;
		dst += _stprintf(dst, _T("%s,x"), get_value_or_symbol(first_symbol, _T("$%04X"), addr));
		break;

	case aby:
		addr = (opram[pos] | (opram[pos+1] << 8));
		pos += 2;
		pc += 2;
		dst += _stprintf(dst, _T("%s,y"), get_value_or_symbol(first_symbol, _T("$%04X"), addr));
		break;

	case ind:
		addr = (opram[pos] | (opram[pos+1] << 8));
		pos += 2;
		pc += 2;
		dst += _stprintf(dst, _T("(%s)"), get_value_or_symbol(first_symbol, _T("$%04X"), addr));
		break;

	case iax:
		addr = (opram[pos] | (opram[pos+1] << 8));
		pos += 2;
		pc += 2;
		dst += _stprintf(dst, _T("(%s),X"), get_value_or_symbol(first_symbol, _T("$%04X"), addr));
		break;

	case iw2:
		addr = (opram[pos] | (opram[pos+1] << 8));
		pos += 2;
		pc += 2;
		dst += _stprintf(dst, _T("%s"), get_value_or_symbol(first_symbol, _T("$%04X"), addr));
		break;

	case iw3:
		addr = (opram[pos] | (opram[pos+1] << 8) | (opram[pos+2] << 16));
		pos += 3;
		pc += 3;
		dst += _stprintf(dst, _T("#%s"), get_value_or_symbol(first_symbol, _T("$%06X"), addr));
		break;

	case idz:
		addr = (INT8) opram[pos++];
		pc++;
		dst += _stprintf(dst, _T("($%02X),z"), addr);
		break;

	case isy:
		op = opram[pos++];
		pc++;
		addr = op;
		dst += _stprintf(dst, _T("(s,$%02X),y"), addr);
		break;

	default:
		dst += _stprintf(dst, _T("$%02X"), op);
		break;
	}
	return (pc - PC) | flags;
}

CPU_DISASSEMBLE( m6502 )
{
	return internal_m6502_dasm(op6502, buffer, pc, oprom, opram, first_symbol);
}

CPU_DISASSEMBLE( m65sc02 )
{
	return internal_m6502_dasm(op65sc02, buffer, pc, oprom, opram, first_symbol);
}

CPU_DISASSEMBLE( m65c02 )
{
	return internal_m6502_dasm(op65c02, buffer, pc, oprom, opram, first_symbol);
}

CPU_DISASSEMBLE( m65ce02 )
{
	return internal_m6502_dasm(op65ce02, buffer, pc, oprom, opram, first_symbol);
}

CPU_DISASSEMBLE( m6510 )
{
	return internal_m6502_dasm(op6502, buffer, pc, oprom, opram, first_symbol);
}

CPU_DISASSEMBLE( deco16 )
{
	return internal_m6502_dasm(opdeco16, buffer, pc, oprom, opram, first_symbol);
}

CPU_DISASSEMBLE( m4510 )
{
	return internal_m6502_dasm(op4510, buffer, pc, oprom, opram, first_symbol);
}
