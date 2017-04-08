/*****************************************************************************

    6280dasm.c Hudsonsoft Hu6280 (HuC6280/Hu6280a) disassembler

    Copyright Bryan McPhail, mish@tendril.co.uk

    This source code is based (with permission!) on the 6502 emulator by
    Juergen Buchmueller.  It is released as part of the Mame emulator project.
    Let me know if you intend to use this code in any other project.


    Notes relating to Mame:

    The dasm window shows 'real' memory, as executed by the cpu
    The data windows show 'physical' memory, as defined in the memory map

******************************************************************************/

//#include "emu.h"

#define _RDOP(addr)   (oprom[addr - pc])
#define _RDBYTE(addr) (opram[addr - pc])
#define _RDWORD(addr) (opram[addr - pc] | ( oprom[(addr) + 1 - pc] << 8 ))

enum addr_mode {
	_non=0, 	 /* no additional arguments */
	_acc,		 /* accumulator */
	_imp,		 /* implicit */
	_imm,		 /* immediate */
	_abs,		 /* absolute */
	_zpg,		 /* zero page */
	_zpx,		 /* zero page + X */
	_zpy,		 /* zero page + Y */
	_zpi,		 /* zero page indirect */
	_abx,		 /* absolute + X */
	_aby,		 /* absolute + Y */
	_rel,		 /* relative */
	_idx,		 /* zero page pre indexed */
	_idy,		 /* zero page post indexed */
	_ind,		 /* indirect */
	_iax,		 /* indirect + X */
	_blk,        /* block */
	_zrl,        /* zero page relative */
	_imz,        /* immediate, zero page */
	_izx,        /* immediate, zero page + X */
	_ima,        /* immediate, absolute */
	_imx         /* immediate, absolute + X */
};

enum opcodes {

	/* 6502 opcodes */
	_adc=0,_and,  _asl,  _bcc,	_bcs,  _beq,  _bit,  _bmi,
	_bne,  _bpl,  _brk,  _bvc,	_bvs,  _clc,  _cld,  _cli,
	_clv,  _cmp,  _cpx,  _cpy,	_dec,  _dex,  _dey,  _eor,
	_inc,  _inx,  _iny,  _jmp,	_jsr,  _lda,  _ldx,  _ldy,
	_lsr,  _nop,  _ora,  _pha,	_php,  _pla,  _plp,  _rol,
	_ror,  _rti,  _rts,  _sbc,	_sec,  _sed,  _sei,  _sta,
	_stx,  _sty,  _tax,  _tay,	_tsx,  _txa,  _txs,  _tya,
	_ill,

	/* Hu6280 extensions */
	_bra,  _stz,  _trb,  _tsb,  _dea,  _ina,  _sax,  _bsr,
	_phx,  _phy,  _plx,  _ply,  _csh,  _csl,  _tam,  _tma,
	_cla,  _cly,  _clx,  _st0,  _st1,  _st2,  _tst,  _set,
	_tdd,  _tia,  _tii,  _tin,  _tai,  _say,  _sxy,

	_sm0,  _sm1,  _sm2,  _sm3,  _sm4,  _sm5,  _sm6,  _sm7,
	_rm0,  _rm1,  _rm2,  _rm3,  _rm4,  _rm5,  _rm6,  _rm7,

	_bs0,  _bs1,  _bs2,  _bs3,  _bs4,  _bs5,  _bs6,  _bs7,
	_br0,  _br1,  _br2,  _br3,  _br4,  _br5,  _br6,  _br7

};


static const _TCHAR *const token[]=
{
	/* 6502 opcodes */
	_T("adc"), _T("and"), _T("asl"), _T("bcc"), _T("bcs"), _T("beq"), _T("bit"), _T("bmi"),
	_T("bne"), _T("bpl"), _T("brk"), _T("bvc"), _T("bvs"), _T("clc"), _T("cld"), _T("cli"),
	_T("clv"), _T("cmp"), _T("cpx"), _T("cpy"), _T("dec"), _T("dex"), _T("dey"), _T("eor"),
	_T("inc"), _T("inx"), _T("iny"), _T("jmp"), _T("jsr"), _T("lda"), _T("ldx"), _T("ldy"),
	_T("lsr"), _T("nop"), _T("ora"), _T("pha"), _T("php"), _T("pla"), _T("plp"), _T("rol"),
	_T("ror"), _T("rti"), _T("rts"), _T("sbc"), _T("sec"), _T("sed"), _T("sei"), _T("sta"),
	_T("stx"), _T("sty"), _T("tax"), _T("tay"), _T("tsx"), _T("txa"), _T("txs"), _T("tya"),
	_T("ill"),

	/* Hu6280 extensions */
	_T("bra"), _T("stz"), _T("trb"), _T("tsb"), _T("dea"), _T("ina"), _T("sax"), _T("bsr"),
	_T("phx"), _T("phy"), _T("plx"), _T("ply"), _T("csh"), _T("csl"), _T("tam"), _T("tma"),
	_T("cla"), _T("cly"), _T("clx"), _T("st0"), _T("st1"), _T("st2"), _T("tst"), _T("set"),
	_T("tdd"), _T("tia"), _T("tii"), _T("tin"), _T("tai"), _T("say"), _T("sxy"),

	_T("smb0"), _T("smb1"), _T("smb2"), _T("smb3"), _T("smb4"), _T("smb5"), _T("smb6"), _T("smb7"),
	_T("rmb0"), _T("rmb1"), _T("rmb2"), _T("rmb3"), _T("rmb4"), _T("rmb5"), _T("rmb6"), _T("rmb7"),

	_T("bbs0"), _T("bbs1"), _T("bbs2"), _T("bbs3"), _T("bbs4"), _T("bbs5"), _T("bbs6"), _T("bbs7"),
	_T("bbr0"), _T("bbr1"), _T("bbr2"), _T("bbr3"), _T("bbr4"), _T("bbr5"), _T("bbr6"), _T("bbr7")
};

static const unsigned char op6280[512]=
{
  _brk,_imp, _ora,_idx, _sxy,_imp, _st0,_imm, _tsb,_zpg, _ora,_zpg, _asl,_zpg, _rm0,_zpg, /* 00 */
  _php,_imp, _ora,_imm, _asl,_acc, _ill,_non, _tsb,_abs, _ora,_abs, _asl,_abs, _br0,_zrl,
  _bpl,_rel, _ora,_idy, _ora,_zpi, _st1,_imm, _trb,_zpg, _ora,_zpx, _asl,_zpx, _rm1,_zpg, /* 10 */
  _clc,_imp, _ora,_aby, _ina,_imp, _ill,_non, _trb,_abs, _ora,_abx, _asl,_abx, _br1,_zrl,
  _jsr,_abs, _and,_idx, _sax,_imp, _st2,_imm, _bit,_zpg, _and,_zpg, _rol,_zpg, _rm2,_zpg, /* 20 */
  _plp,_imp, _and,_imm, _rol,_acc, _ill,_non, _bit,_abs, _and,_abs, _rol,_abs, _br2,_zrl,
  _bmi,_rel, _and,_idy, _and,_zpi, _ill,_non, _bit,_zpx, _and,_zpx, _rol,_zpx, _rm3,_zpg, /* 30 */
  _sec,_imp, _and,_aby, _dea,_imp, _ill,_non, _bit,_abx, _and,_abx, _rol,_abx, _br3,_zrl,
  _rti,_imp, _eor,_idx, _say,_imp, _tma,_imm, _bsr,_rel, _eor,_zpg, _lsr,_zpg, _rm4,_zpg, /* 40 */
  _pha,_imp, _eor,_imm, _lsr,_acc, _ill,_non, _jmp,_abs, _eor,_abs, _lsr,_abs, _br4,_zrl,
  _bvc,_rel, _eor,_idy, _eor,_zpi, _tam,_imm, _csl,_imp, _eor,_zpx, _lsr,_zpx, _rm5,_zpg, /* 50 */
  _cli,_imp, _eor,_aby, _phy,_imp, _ill,_non, _ill,_non, _eor,_abx, _lsr,_abx, _br5,_zrl,
  _rts,_imp, _adc,_idx, _cla,_imp, _ill,_non, _stz,_zpg, _adc,_zpg, _ror,_zpg, _rm6,_zpg, /* 60 */
  _pla,_imp, _adc,_imm, _ror,_acc, _ill,_non, _jmp,_ind, _adc,_abs, _ror,_abs, _br6,_zrl,
  _bvs,_rel, _adc,_idy, _adc,_zpi, _tii,_blk, _stz,_zpx, _adc,_zpx, _ror,_zpx, _rm7,_zpg, /* 70 */
  _sei,_imp, _adc,_aby, _ply,_imp, _ill,_non, _jmp,_iax, _adc,_abx, _ror,_abx, _br7,_zrl,
  _bra,_rel, _sta,_idx, _clx,_imp, _tst,_imz, _sty,_zpg, _sta,_zpg, _stx,_zpg, _sm0,_zpg, /* 80 */
  _dey,_imp, _bit,_imm, _txa,_imp, _ill,_non, _sty,_abs, _sta,_abs, _stx,_abs, _bs0,_zrl,
  _bcc,_rel, _sta,_idy, _sta,_zpi, _tst,_ima, _sty,_zpx, _sta,_zpx, _stx,_zpy, _sm1,_zpg, /* 90 */
  _tya,_imp, _sta,_aby, _txs,_imp, _ill,_non, _stz,_abs, _sta,_abx, _stz,_abx, _bs1,_zrl,
  _ldy,_imm, _lda,_idx, _ldx,_imm, _tst,_izx, _ldy,_zpg, _lda,_zpg, _ldx,_zpg, _sm2,_zpg, /* a0 */
  _tay,_imp, _lda,_imm, _tax,_imp, _ill,_non, _ldy,_abs, _lda,_abs, _ldx,_abs, _bs2,_zrl,
  _bcs,_rel, _lda,_idy, _lda,_zpi, _tst,_imx, _ldy,_zpx, _lda,_zpx, _ldx,_zpy, _sm3,_zpg, /* b0 */
  _clv,_imp, _lda,_aby, _tsx,_imp, _ill,_non, _ldy,_abx, _lda,_abx, _ldx,_aby, _bs3,_zrl,
  _cpy,_imm, _cmp,_idx, _cly,_imp, _tdd,_blk, _cpy,_zpg, _cmp,_zpg, _dec,_zpg, _sm4,_zpg, /* c0 */
  _iny,_imp, _cmp,_imm, _dex,_imp, _ill,_non, _cpy,_abs, _cmp,_abs, _dec,_abs, _bs4,_zrl,
  _bne,_rel, _cmp,_idy, _cmp,_zpi, _tin,_blk, _csh,_imp, _cmp,_zpx, _dec,_zpx, _sm5,_zpg, /* d0 */
  _cld,_imp, _cmp,_aby, _phx,_imp, _ill,_non, _ill,_non, _cmp,_abx, _dec,_abx, _bs5,_zrl,
  _cpx,_imm, _sbc,_idx, _ill,_non, _tia,_blk, _cpx,_zpg, _sbc,_zpg, _inc,_zpg, _sm6,_zpg, /* e0 */
  _inx,_imp, _sbc,_imm, _nop,_imp, _ill,_non, _cpx,_abs, _sbc,_abs, _inc,_abs, _bs6,_zrl,
  _beq,_rel, _sbc,_idy, _sbc,_zpi, _tai,_blk, _set,_imp, _sbc,_zpx, _inc,_zpx, _sm7,_zpg, /* f0 */
  _sed,_imp, _sbc,_aby, _plx,_imp, _ill,_non, _ill,_non, _sbc,_abx, _inc,_abx, _bs7,_zrl
};

/*****************************************************************************
 *  Disassemble a single command and return the number of bytes it uses.
 *****************************************************************************/
CPU_DISASSEMBLE( h6280 )
{
	UINT32 flags = 0;
	int PC, OP, opc, arg;

	PC = pc;
	OP = _RDOP(PC);
	OP = OP << 1;
	PC++;

	opc = op6280[OP];
	arg = op6280[OP+1];

	if (opc == _jsr || opc == _bsr)
		flags = DASMFLAG_STEP_OVER;
	else if (opc == _rts)
		flags = DASMFLAG_STEP_OUT;

	switch(arg)
	{
		case _acc:
			_stprintf(buffer,_T("%-5sa"), token[opc]);
			break;
		case _imp:
			_stprintf(buffer,_T("%s"), token[opc]);
			break;
		case _rel:
			_stprintf(buffer,_T("%-5s%s"), token[opc], get_value_or_symbol(first_symbol, _T("$%04X"), (PC + 1 + (signed char)_RDBYTE(PC)) & 0xffff));
			PC+=1;
			break;
		case _imm:
			_stprintf(buffer,_T("%-5s#$%02X"), token[opc], _RDBYTE(PC));
			PC+=1;
			break;
		case _zpg:
			_stprintf(buffer,_T("%-5s$%02X"), token[opc], _RDBYTE(PC));
			PC+=1;
			break;
		case _zpx:
			_stprintf(buffer,_T("%-5s$%02X,x"), token[opc], _RDBYTE(PC));
			PC+=1;
			break;
		case _zpy:
			_stprintf(buffer,_T("%-5s$%02X,y"), token[opc], _RDBYTE(PC));
			PC+=1;
			break;
		case _idx:
			_stprintf(buffer,_T("%-5s($%02X,x)"), token[opc], _RDBYTE(PC));
			PC+=1;
			break;
		case _idy:
			_stprintf(buffer,_T("%-5s($%02X),y"), token[opc], _RDBYTE(PC));
			PC+=1;
			break;
		case _zpi:
			_stprintf(buffer,_T("%-5s($%02X)"), token[opc], _RDBYTE(PC));
			PC+=1;
			break;
		case _abs:
			_stprintf(buffer,_T("%-5s%s"), token[opc], get_value_or_symbol(first_symbol, _T("$%04X"), _RDWORD(PC)));
			PC+=2;
			break;
		case _abx:
			_stprintf(buffer,_T("%-5s%s,x"), token[opc], get_value_or_symbol(first_symbol, _T("$%04X"), _RDWORD(PC)));
			PC+=2;
			break;
		case _aby:
			_stprintf(buffer,_T("%-5s%s,y"), token[opc], get_value_or_symbol(first_symbol, _T("$%04X"), _RDWORD(PC)));
			PC+=2;
			break;
		case _ind:
			_stprintf(buffer,_T("%-5s(%s)"), token[opc], get_value_or_symbol(first_symbol, _T("$%04X"), _RDWORD(PC)));
			PC+=2;
			break;
		case _iax:
			_stprintf(buffer,_T("%-5s(%s),X"), token[opc], get_value_or_symbol(first_symbol, _T("$%04X"), _RDWORD(PC)));
			PC+=2;
			break;
		case _blk:
			_stprintf(buffer,_T("%-5s%s %s %s"), token[opc], get_value_or_symbol(first_symbol, _T("$%04X"), _RDWORD(PC)), get_value_or_symbol(first_symbol, _T("$%04X"), _RDWORD(PC+2)), get_value_or_symbol(first_symbol, _T("$%04X"), _RDWORD(PC+4)));
			PC+=6;
			break;
		case _zrl:
			_stprintf(buffer,_T("%-5s$%02X %s"), token[opc], _RDBYTE(PC), get_value_or_symbol(first_symbol, _T("$%04X"), (PC + 2 + (signed char)_RDBYTE(PC+1)) & 0xffff));
			PC+=2;
			break;
		case _imz:
			_stprintf(buffer,_T("%-5s#$%02X $%02X"), token[opc], _RDBYTE(PC), _RDBYTE(PC+1));
			PC+=2;
			break;
		case _izx:
			_stprintf(buffer,_T("%-5s#$%02X $%02X,x"), token[opc], _RDBYTE(PC), _RDBYTE(PC+1));
			PC+=2;
			break;
		case _ima:
			_stprintf(buffer,_T("%-5s#$%02X %s"), token[opc], _RDBYTE(PC), get_value_or_symbol(first_symbol, _T("$%04X"), _RDWORD(PC+1)));
			PC+=3;
			break;
		case _imx:
			_stprintf(buffer,_T("%-5s#$%02X %s,x"), token[opc], _RDBYTE(PC), get_value_or_symbol(first_symbol, _T("$%04X"), _RDWORD(PC+1)));
			PC+=3;
			break;

		default:
			_stprintf(buffer,_T("%-5s$%02X"), token[opc], OP >> 1);
	}
	return (PC - pc) | flags | DASMFLAG_SUPPORTED;
}
