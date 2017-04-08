// license:BSD-3-Clause
// copyright-holders:Raphael Nabet
/*****************************************************************************
 *
 *   9900dasm.c
 *   TMS 9900 family disassembler
 *
 *   Copyright Raphael Nabet
 *   Based on previous work Copyright John Butler.
 *   Based on 6502dasm.c 6502/65c02/6510 disassembler by Juergen Buchmueller
 *
 *   - This source code is released as freeware for non-commercial purposes.
 *   - You are free to use and redistribute this code in modified or
 *     unmodified form, provided you list me in the credits.
 *   - If you modify this source code, you must add a notice to each modified
 *     source file that it has been changed.  If you're a nice person, you
 *     will clearly mark each change too.  :)
 *   - The author of this copywritten work reserves the right to change the
 *     terms of its usage and license at any time, including retroactively
 *   - This entire notice must remain in the source code.
 *
 *****************************************************************************/


//#include "emu.h"
//#include "debugger.h"
//#include "tms9900.h"

#define MASK    0x0000ffff
#define BITS(val,n1,n2) ((val>>(15-(n2))) & (MASK>>(15-((n2)-(n1)))))

enum format_t
{
	format_1,   /* 2 address instructions */
	format_2a,  /* jump instructions */
	format_2b,  /* bit I/O instructions */
	format_3_9, /* logical, multiply, and divide instructions */
	format_4,   /* CRU instructions */
	format_5,   /* register shift instructions */
	format_6,   /* single address instructions */
	format_7,   /* instructions without operands */
	format_8a,  /* immediate instructions (destination register) */
	format_8b,  /* immediate instructions (no destination register) */
	format_9,   /* extended operation instruction */
	format_10,  /* memory map file instruction */
	format_11,  /* multiple precision instructions */
	format_12,  /* string instructions */
	format_13,  /* multiple precision shift instructions */
	format_14,  /* bit testing instructions */
	format_15,  /* invert order of field instruction */
	format_16,  /* field instructions */
	format_17,  /* alter register and jump instructions */
	format_18,  /* single register operand instructions */
	format_liim,/* format for liim (looks like format 18) */
	format_19,  /* move address instruction */
	format_20,  /* list search instructions */
	format_21,  /* extend precision instruction */

	illegal
};

/* definitions for flags */
enum
{
	/* processor set on which opcodes are available */
	ps_any      = 0x01,     /* every processor in the tms9900/ti990 family */
	ps_mapper   = 0x02,     /* processors with memory mapper (ti990/10, ti990/12,
                                    and tms99000 with mapper coprocessor) */
	ps_tms9995  = 0x04,     /* ti990/12, tms9995, and later */
	ps_tms99000 = 0x08,     /* ti990/12, tms99000, and later */
	ps_ti990_12 = 0x10,     /* ti990/12 only */

	/* additional flags for special decoding */
	sd_11       = 0x100,    /* bit 11 should be cleared in li, ai, andi, ori, ci, stwp, stst */
	sd_11_15    = 0x200     /* bits 11-15 should be cleared in lwpi, limi, idle, rset, rtwp, ckon, ckof, lrex */
};

struct description_t
{
	const _TCHAR *mnemonic;
	format_t format;
	int flags;
};


enum opcodes {
	/* basic instruction set */
	_a=0,   _ab,    _c,     _cb,    _s,     _sb,    _soc,   _socb,  _szc,   _szcb,
	_mov,   _movb,  _coc,   _czc,   _xor,   _mpy,   _div,   _xop,   _b,     _bl,
	_blwp,  _clr,   _seto,  _inv,   _neg,   _abs,   _swpb,  _inc,   _inct,  _dec,
	_dect,  _x,     _ldcr,  _stcr,  _sbo,   _sbz,   _tb,    _jeq,   _jgt,   _jh,
	_jhe,   _jl,    _jle,   _jlt,   _jmp,   _jnc,   _jne,   _jno,   _joc,   _jop,
	_sla,   _sra,   _src,   _srl,   _ai,    _andi,  _ci,    _li,    _ori,   _lwpi,
	_limi,  _stst,  _stwp,  _rtwp,  _idle,  _rset,  _ckof,  _ckon,  _lrex,

	/* mapper instruction set */
	_lds,   _ldd,   _lmf,

	/* tms9995 instruction set */
	_divs,  _mpys,  _lst,   _lwp,

	/* tms99000 instruction set */
	_bind,

	/* ti990/12 instruction set */
	_sram,  _slam,  _rto,   _lto,   _cnto,  _slsl,  _slsp,  _bdc,   _dbc,   _swpm,
	_xorm,  _orm,   _andm,  _sm,    _am,    _mova,  _emd,   _eint,  _dint,  _stpc,
	_cs,    _seqb,  _movs,  _lim,   _lcs,   _blsk,  _mvsr,  _mvsk,  _pops,  _pshs,

	_cri,   _cdi,   _negr,  _negd,  _cre,   _cde,   _cer,   _ced,   _nrm,   _tmb,
	_tcmb,  _tsmb,  _srj,   _arj,   _xit,   _insf,  _xv,    _xf,    _ar,    _cir,
	_sr,    _mr,    _dr,    _lr,    _str,   _iof,   _sneb,  _crc,   _ts,    _ad,
	_cid,   _sd,    _md,    _dd,    _ld,    _std,   _ep,

	/* tms9940-only instruction set */
	_liim,  _dca,   _dcs,

	_ill
};


static const description_t descriptions[144+3+1] =
{
	/* basic instruction set */
	{ _T("a"),      format_1,   ps_any },           { _T("ab"),     format_1,   ps_any },
	{ _T("c"),      format_1,   ps_any },           { _T("cb"),     format_1,   ps_any },
	{ _T("s"),      format_1,   ps_any },           { _T("sb"),     format_1,   ps_any },
	{ _T("soc"),    format_1,   ps_any },           { _T("socb"),   format_1,   ps_any },
	{ _T("szc"),    format_1,   ps_any },           { _T("szcb"),   format_1,   ps_any },
	{ _T("mov"),    format_1,   ps_any },           { _T("movb"),   format_1,   ps_any },
	{ _T("coc"),    format_3_9, ps_any },           { _T("czc"),    format_3_9, ps_any },
	{ _T("xor"),    format_3_9, ps_any },           { _T("mpy"),    format_3_9, ps_any },
	{ _T("div"),    format_3_9, ps_any },           { _T("xop"),    format_9,   ps_any },
	{ _T("b"),      format_6,   ps_any },           { _T("bl"),     format_6,   ps_any },
	{ _T("blwp"),   format_6,   ps_any },           { _T("clr"),    format_6,   ps_any },
	{ _T("seto"),   format_6,   ps_any },           { _T("inv"),    format_6,   ps_any },
	{ _T("neg"),    format_6,   ps_any },           { _T("abs"),    format_6,   ps_any },
	{ _T("swpb"),   format_6,   ps_any },           { _T("inc"),    format_6,   ps_any },
	{ _T("inct"),   format_6,   ps_any },           { _T("dec"),    format_6,   ps_any },
	{ _T("dect"),   format_6,   ps_any },           { _T("x"),      format_6,   ps_any },
	{ _T("ldcr"),   format_4,   ps_any },           { _T("stcr"),   format_4,   ps_any },
	{ _T("sbo"),    format_2b,  ps_any },           { _T("sbz"),    format_2b,  ps_any },
	{ _T("tb"),     format_2b,  ps_any },           { _T("jeq"),    format_2a,  ps_any },
	{ _T("jgt"),    format_2a,  ps_any },           { _T("jh"),     format_2a,  ps_any },
	{ _T("jhe"),    format_2a,  ps_any },           { _T("jl"),     format_2a,  ps_any },
	{ _T("jle"),    format_2a,  ps_any },           { _T("jlt"),    format_2a,  ps_any },
	{ _T("jmp"),    format_2a,  ps_any },           { _T("jnc"),    format_2a,  ps_any },
	{ _T("jne"),    format_2a,  ps_any },           { _T("jno"),    format_2a,  ps_any },
	{ _T("joc"),    format_2a,  ps_any },           { _T("jop"),    format_2a,  ps_any },
	{ _T("sla"),    format_5,   ps_any },           { _T("sra"),    format_5,   ps_any },
	{ _T("src"),    format_5,   ps_any },           { _T("srl"),    format_5,   ps_any },
	{ _T("ai"),     format_8a,  ps_any|sd_11 },     { _T("andi"),   format_8a,  ps_any|sd_11 },
	{ _T("ci"),     format_8a,  ps_any|sd_11 },     { _T("li"),     format_8a,  ps_any|sd_11 },
	{ _T("ori"),    format_8a,  ps_any|sd_11 },     { _T("lwpi"),   format_8b,  ps_any|sd_11|sd_11_15 },
	{ _T("limi"),   format_8b,  ps_any|sd_11|sd_11_15 },    { _T("stst"),   format_18,  ps_any|sd_11 },
	{ _T("stwp"),   format_18,  ps_any|sd_11 },     { _T("rtwp"),   format_7,   ps_any|sd_11|sd_11_15 },
	{ _T("idle"),   format_7,   ps_any|sd_11|sd_11_15 },    { _T("rset"),   format_7,   ps_any|sd_11|sd_11_15 },
	{ _T("ckof"),   format_7,   ps_any|sd_11|sd_11_15 },    { _T("ckon"),   format_7,   ps_any|sd_11|sd_11_15 },
	{ _T("lrex"),   format_7,   ps_any|sd_11|sd_11_15 },

	/* mapper instruction set */
	{ _T("lds"),    format_6,   ps_mapper },        { _T("ldd"),    format_6,   ps_mapper },
	{ _T("lmf"),    format_10,  ps_mapper },

	/* tms9995 instruction set */
	{ _T("divs"),   format_6,   ps_tms9995 },       { _T("mpys"),   format_6,   ps_tms9995 },
	{ _T("lst"),    format_18,  ps_tms9995 },       { _T("lwp"),    format_18,  ps_tms9995 },

	/* tms99000 instruction set */
	{ _T("bind"),   format_6,   ps_tms99000 },

	/* ti990/12 instruction set */
	{ _T("sram"),   format_13,  ps_ti990_12 },      { _T("slam"),   format_13,  ps_ti990_12 },
	{ _T("rto"),    format_11,  ps_ti990_12 },      { _T("lto"),    format_11,  ps_ti990_12 },
	{ _T("cnto"),   format_11,  ps_ti990_12 },      { _T("slsl"),   format_20,  ps_ti990_12 },
	{ _T("slsp"),   format_20,  ps_ti990_12 },      { _T("bdc"),    format_11,  ps_ti990_12 },
	{ _T("dbc"),    format_11,  ps_ti990_12 },      { _T("swpm"),   format_11,  ps_ti990_12 },
	{ _T("xorm"),   format_11,  ps_ti990_12 },      { _T("orm"),    format_11,  ps_ti990_12 },
	{ _T("andm"),   format_11,  ps_ti990_12 },      { _T("sm"),     format_11,  ps_ti990_12 },
	{ _T("am"),     format_11,  ps_ti990_12 },      { _T("mova"),   format_19,  ps_ti990_12 },
	{ _T("emd"),    format_7,   ps_ti990_12 },      { _T("eint"),   format_7,   ps_ti990_12 },
	{ _T("dint"),   format_7,   ps_ti990_12 },      { _T("stpc"),   format_18,  ps_ti990_12 },
	{ _T("cs"),     format_12,  ps_ti990_12 },      { _T("seqb"),   format_12,  ps_ti990_12 },
	{ _T("movs"),   format_12,  ps_ti990_12 },      { _T("lim"),    format_18,  ps_ti990_12 },
	{ _T("lcs"),    format_18,  ps_ti990_12 },      { _T("blsk"),   format_8a,  ps_ti990_12 },
	{ _T("mvsr"),   format_12,  ps_ti990_12 },      { _T("mvsk"),   format_12,  ps_ti990_12 },
	{ _T("pops"),   format_12,  ps_ti990_12 },      { _T("pshs"),   format_12,  ps_ti990_12 },
	{ _T("cri"),    format_7,   ps_ti990_12 },      { _T("cdi"),    format_7,   ps_ti990_12 },
	{ _T("negr"),   format_7,   ps_ti990_12 },      { _T("negd"),   format_7,   ps_ti990_12 },
	{ _T("cre"),    format_7,   ps_ti990_12 },      { _T("cde"),    format_7,   ps_ti990_12 },
	{ _T("cer"),    format_7,   ps_ti990_12 },      { _T("ced"),    format_7,   ps_ti990_12 },
	{ _T("nrm"),    format_11,  ps_ti990_12 },      { _T("tmb"),    format_14,  ps_ti990_12 },
	{ _T("tcmb"),   format_14,  ps_ti990_12 },      { _T("tsmb"),   format_14,  ps_ti990_12 },
	{ _T("srj"),    format_17,  ps_ti990_12 },      { _T("arj"),    format_17,  ps_ti990_12 },
	{ _T("xit"),    format_7,   ps_ti990_12 },      { _T("insf"),   format_16,  ps_ti990_12 },
	{ _T("xv"),     format_16,  ps_ti990_12 },      { _T("xf"),     format_16,  ps_ti990_12 },
	{ _T("ar"),     format_6,   ps_ti990_12 },      { _T("cir"),    format_6,   ps_ti990_12 },
	{ _T("sr"),     format_6,   ps_ti990_12 },      { _T("mr"),     format_6,   ps_ti990_12 },
	{ _T("dr"),     format_6,   ps_ti990_12 },      { _T("lr"),     format_6,   ps_ti990_12 },
	{ _T("str"),    format_6,   ps_ti990_12 },      { _T("iof"),    format_15,  ps_ti990_12 },
	{ _T("sneb"),   format_12,  ps_ti990_12 },      { _T("crc"),    format_12,  ps_ti990_12 },
	{ _T("ts"),     format_12,  ps_ti990_12 },      { _T("ad"),     format_6,   ps_ti990_12 },
	{ _T("cid"),    format_6,   ps_ti990_12 },      { _T("sd"),     format_6,   ps_ti990_12 },
	{ _T("md"),     format_6,   ps_ti990_12 },      { _T("dd"),     format_6,   ps_ti990_12 },
	{ _T("ld"),     format_6,   ps_ti990_12 },      { _T("std"),    format_6,   ps_ti990_12 },
	{ _T("ep"),     format_21,  ps_ti990_12 },

	/* tms9940-only instruction set */
	/* these instructions are said to be format 9 (xop), but since the xop
	level is interpreted as part of the opcode, dca and dcs should be handled
	like format 6.  liim looks like format 18, but slightly different,
	therefore it is handled like a special format. */
	{ _T("liim"),   format_liim,/*ps_tms9940*/0 },  { _T("dca"),    format_6,   /*ps_tms9940*/0 },
	{ _T("dcs"),    format_6,   /*ps_tms9940*/0 },

	{ NULL,     illegal,    ps_any }
};


static const enum opcodes ops_4000_ffff_s12[12]=
{
	_szc,   _szcb,  _s,     _sb,                                    /*4000-7000*/
	_c,     _cb,    _a,     _ab,    _mov,   _movb,  _soc,   _socb   /*8000-f000*/
};


static const enum opcodes ops_2000_3fff_s10[8]=
{
	_coc,   _czc,   _xor,   _xop,   _ldcr,  _stcr,  _mpy,   _div    /*2000-3800*/
};


static const enum opcodes ops_1000_1fff_s8[16]=
{
	_jmp,   _jlt,   _jle,   _jeq,   _jhe,   _jgt,   _jne,   _jnc,   /*1000-1700*/
	_joc,   _jno,   _jl,    _jh,    _jop,   _sbo,   _sbz,   _tb     /*1800-1f00*/
};


static const enum opcodes ops_0e40_0fff_s6[7]=
{
			_ad,    _cid,   _sd,    _md,    _dd,    _ld,    _std    /*0e40-0fc0*/
};


static const enum opcodes ops_0e00_0e3f_s4[4]=
{
	_iof,   _sneb,  _crc,   _ts                                     /*0e00-0e30*/
};


static const enum opcodes ops_0c40_0dff_s6[7]=
{
			_ar,    _cir,   _sr,    _mr,    _dr,    _lr,    _str    /*0c40-0dc0*/
};


static const enum opcodes ops_0c10_0c3f_s4[3]=
{
			_insf,  _xv,    _xf                                     /*0c10-0c30*/
};


static const enum opcodes ops_0c00_0c0f_s0[16]=
{
	_cri,   _cdi,   _negr,  _negd,  _cre,   _cde,   _cer,   _ced,   /*0c00-0c07*/
	_nrm,   _tmb,   _tcmb,  _tsmb,  _srj,   _arj,   _xit,   _xit    /*0c08-0c0f*/
};



static const enum opcodes ops_0800_0bff_s8[4]=
{
	_sra,   _srl,   _sla,   _src                                    /*0800-0b00*/
};


static const enum opcodes ops_0400_07ff_s6[16]=
{
	_blwp,  _b,     _x,     _clr,   _neg,   _inv,   _inc,   _inct,  /*0400-05c0*/
	_dec,   _dect,  _bl,    _swpb,  _seto,  _abs,   _lds,   _ldd    /*0600-07c0*/
};


static const enum opcodes ops_0200_03ff_s5[16]=
{
	_li,    _ai,    _andi,  _ori,   _ci,    _stwp,  _stst,  _lwpi,  /*0200-02e0*/
	_limi,  _lmf,   _idle,  _rset,  _rtwp,  _ckon,  _ckof,  _lrex   /*0300-03e0*/
};


static const enum opcodes ops_0100_01ff_s6[4]=
{
	_ill,   _bind,  _divs,  _mpys                                   /*0100-01c0*/
};


static const enum opcodes ops_0030_00ff_s4[13]=
{
							_stpc,  _cs,    _seqb,  _movs,  _lim,   /*0030-0070*/
	_lst,   _lwp,   _lcs,   _blsk,  _mvsr,  _mvsk,  _pops,  _pshs   /*0080-00f0*/
};


static const enum opcodes ops_001c_002f_s0[20]=
{
									_sram,  _slam,  _rto,   _lto,   /*001c-001f*/
	_cnto,  _slsl,  _slsp,  _bdc,   _dbc,   _swpm,  _xorm,  _orm,   /*0020-0027*/
	_andm,  _sm,    _am,    _mova,  _ill,   _emd,   _eint,  _dint   /*0028-002f*/
};



static int PC;


INLINE UINT16 readop_arg(const UINT8 *opram, unsigned pc)
{
	UINT16 result = opram[PC++ - pc] << 8;
	return result | opram[PC++ - pc];
}

static int print_arg (_TCHAR *dest, int mode, int arg, const UINT8 *opram, unsigned pc, symbol_t *first_symbol)
{
	int base;

	switch (mode)
	{
		case 0x0:   /* workspace register */
			return _stprintf (dest, _T("R%d"), arg);
		case 0x1:   /* workspace register indirect */
			return _stprintf (dest, _T("*R%d"), arg);
		case 0x2:   /* symbolic|indexed */
			base = readop_arg(opram, pc);
			if (arg)    /* indexed */
				return _stprintf (dest, _T("@>%s(R%d)"), get_value_or_symbol(first_symbol, _T("%04x"), base), arg);
			else        /* symbolic (direct) */
				return _stprintf (dest, _T("@>%s"), get_value_or_symbol(first_symbol, _T("%04x"), base));
		case 0x3:   /* workspace register indirect auto increment */
			return _stprintf (dest, _T("*R%d+"), arg);
	}

	return 0;
}


/*****************************************************************************
 *  Disassemble a single command and return the number of bytes it uses.
 *****************************************************************************/
unsigned Dasm9900 (_TCHAR *buffer, unsigned pc, int model_id, const UINT8 *oprom, const UINT8 *opram, symbol_t *first_symbol)
{
	int OP, OP2, opc;
	int sarg, darg, smode, dmode;
	signed char displacement;
	int byte_count, checkpoint;
	int bit_position, bit_width;
	unsigned dasmflags = 0;

	const _TCHAR *mnemonic;
	format_t format;
	int flags;

	/*
	    Under tms9900, opcodes >0400->07FF are incompletely decoded: bits 11 is ignored, and so are
	    bits 12-15 for instructions which do not require a register.  On the other hand, ti990/10
	    generates an illegal instruction error when bit 11 is set, but still ignores bits 12-15.
	    Additionally, ti990/12 and tms9995 will generate an illegal error when bits 12-15 are
	    non-zero.
	*/
	#define BETTER_0200_DECODING (model_id == TI990_10_ID)
	#define COMPLETE_0200_DECODING (/*(model_id == TI990_12_ID) ||*/ (model_id >= TMS9995_ID))

	int processor_mask = ps_any;

	if ((model_id == TI990_10_ID) /*|| (model_id == TI990_12_ID)*/ || (model_id >= TMS99000_ID))
		processor_mask |= ps_mapper;        /* processors with memory mapper (ti990/10, ti990/12,
                                                and tms99000 with mapper coprocessor) */
	if (/*(model_id == TI990_12_ID) ||*/ (model_id >= TMS9995_ID))
		processor_mask |= ps_tms9995;       /* ti990/12, tms9995, and later */

	if (/*(model_id == TI990_12_ID) ||*/ (model_id >= TMS99000_ID))
		processor_mask |= ps_tms99000;      /* ti990/12, tms99000, and later */

	/*if ((model_id == TI990_12_ID))
	    processor_mask |= ps_ti990_12;*/    /* ti990/12, tms99000, and later */

	PC = pc;
	OP = oprom[PC++ - pc] << 8;
	OP |= oprom[PC++ - pc];

	/* let's identify the opcode */
	if (OP >= 0x4000)
		opc = ops_4000_ffff_s12[(OP - 0x4000) >> 12];
	else if (OP >= 0x2000)
		opc = ops_2000_3fff_s10[(OP - 0x2000) >> 10];
	else if (OP >= 0x1000)
		opc = ops_1000_1fff_s8[(OP - 0x1000) >> 8];
	else if (OP >= 0x0C00)
	{
		if (OP >= 0x0E40)
			opc = ops_0e40_0fff_s6[(OP - 0x0E40) >> 6];
		else if (OP >= 0x0E00)
			opc = ops_0e00_0e3f_s4[(OP - 0x0E00) >> 4];
		else if (OP >= 0x0C40)
			opc = ops_0c40_0dff_s6[(OP - 0x0C40) >> 6];
		else if (OP >= 0x0C10)
			opc = ops_0c10_0c3f_s4[(OP - 0x0C10) >> 4];
		else
			opc = ops_0c00_0c0f_s0[OP - 0x0C00];
	}
	else if (OP >= 0x0800)
		opc = ops_0800_0bff_s8[(OP - 0x0800) >> 8];
	else if (OP >= 0x0400)
		opc = ops_0400_07ff_s6[(OP - 0x0400) >> 6];
	else if (OP >= 0x0200)
	{
		opc = ops_0200_03ff_s5[(OP - 0x0200) >> 5];
		if (BETTER_0200_DECODING || COMPLETE_0200_DECODING)
		{
			flags = descriptions[opc].flags;
			if ( (COMPLETE_0200_DECODING && (flags & sd_11_15) && (OP & 0x001f))
					|| ((flags & sd_11) && (OP & 0x0010)) )
			{
				opc = _ill;
				if (OP >= 0x03f0)
					opc = _ep;  /* the ep opcode is located in a "hole" */
			}
		}
	}
	else if (OP >= 0x0100)
		opc = ops_0100_01ff_s6[(OP - 0x0100) >> 6];
	else if (OP >= 0x0030)
		opc = ops_0030_00ff_s4[(OP - 0x0030) >> 4];
	else if (OP >= 0x001C)
		opc = ops_001c_002f_s0[OP - 0x001C];
	else
		opc = _ill;

	/* read flags */
	flags = descriptions[opc].flags;
	/* set as illegal if the processor does not implement this instruction */
	if (! (flags & processor_mask))
	{
		opc = _ill;
		flags = descriptions[opc].flags;    /* read new flags */
	}

	/* tms9940 replace a few xops with custom instructions */
	if ((opc == _xop) && ((model_id == TMS9940_ID) || (model_id == TMS9985_ID)))
	{
		switch (BITS(OP,6,9))
		{
		case 0:
			/* opcode is dca */
			opc = _dca;
			break;

		case 1:
			/* opcode is dcs */
			opc = _dcs;
				break;

		case 2:
		case 3: /* should be 2, but instruction decoding is incomplete */
			/* opcode is liim */
			if (BITS(OP,12,15) == 0)
				/* ts must be == 0 */
				opc = _liim;
			else
				/* I don't know what happens when ts != 0.  Maybe the CPU does
				the complete address decoding, and liim gets a bogus value
				instead of the immediate.  Since I do not know, I handle this
				as an illegal instruction. */
				opc = _ill;
				break;

		default:
			/* this is still a software xop */
				break;
		}
	}

	mnemonic = descriptions[opc].mnemonic;
	format = descriptions[opc].format;

	/* bl and blwp instructions are subroutines */
	if (mnemonic != NULL && mnemonic[0] == _T('b') && mnemonic[1] == _T('l'))
		dasmflags = DASMFLAG_STEP_OVER;

	/* b *r11 and rtwp are returns */
	else if (opc == 0x045b || (mnemonic != NULL && _tcscmp(mnemonic, _T("rtwp")) == 0))
		dasmflags = DASMFLAG_STEP_OUT;

	switch (format)
	{
	case format_1:      /* 2 address instructions */
		smode = BITS(OP,10,11);
		sarg = BITS(OP,12,15);
		dmode = BITS(OP,4,5);
		darg = BITS(OP,6,9);

		buffer += _stprintf (buffer, _T("%-4s "), mnemonic);
		buffer += print_arg (buffer, smode, sarg, opram, pc, first_symbol);
		buffer += _stprintf (buffer, _T(","));
		buffer += print_arg (buffer, dmode, darg, opram, pc, first_symbol);
		break;

	case format_2a:     /* jump instructions */
		displacement = (signed char)BITS(OP,8,15);
		_stprintf (buffer, _T("%-4s >%s"), mnemonic, get_value_or_symbol(first_symbol, _T("%04x"), 0xffff & (PC + displacement * 2)));
		break;

	case format_2b:     /* bit I/O instructions */
		displacement = (signed char)BITS(OP,8,15);
		_stprintf (buffer, _T("%-4s >%04x"), mnemonic, 0xffff & displacement);
		break;

	case format_3_9:    /* logical, multiply, and divide instructions */
	case format_4:      /* CRU instructions */
	case format_9:      /* extended operation instruction */
		smode = BITS(OP,10,11);
		sarg = BITS(OP,12,15);
		darg = BITS(OP,6,9);

		if ((darg == 0) && (format == format_4))
			darg = 16;

		if (format == format_3_9)
		{
			buffer += _stprintf (buffer, _T("%-4s "), mnemonic);
			buffer += print_arg (buffer, smode, sarg, opram, pc, first_symbol);
			buffer += _stprintf (buffer, _T(",R%d"), darg);
		}
		else
		{
			buffer += _stprintf (buffer, _T("%-4s "), mnemonic);
			buffer += print_arg (buffer, smode, sarg, opram, pc, first_symbol);
			buffer += _stprintf (buffer, _T(",%d"), darg);
		}
		break;

	case format_5:      /* register shift instructions */
		sarg = BITS(OP,12,15);
		darg = BITS(OP,8,11);

		_stprintf (buffer, darg ? _T("%-4s R%d,%d") : _T("%-4s R%d,R%d"), mnemonic, sarg, darg);
		break;

	case format_6:      /* single address instructions */
		smode = BITS(OP,10,11);
		sarg = BITS(OP,12,15);

		buffer += _stprintf (buffer, _T("%-4s "), mnemonic);
		buffer += print_arg (buffer, smode, sarg, opram, pc, first_symbol);
		break;

	case format_7:      /* instructions without operands */
		_stprintf (buffer, _T("%s"), mnemonic);
		break;

	case format_8a:     /* immediate instructions (destination register) */
		darg = BITS(OP,12,15);
		sarg = readop_arg(opram, pc);

		_stprintf (buffer, _T("%-4s R%d,>%s"), mnemonic, darg, get_value_or_symbol(first_symbol, _T("%04x"), sarg));
		break;

	case format_8b:     /* immediate instructions (no destination register) */
		sarg = readop_arg(opram, pc);

		_stprintf (buffer, _T("%-4s >%s"), mnemonic, get_value_or_symbol(first_symbol, _T("%04x"), sarg));
		break;

	case format_10:     /* memory map file instruction */
		sarg = BITS(OP,12,15);
		darg = BITS(OP,11,11);

		_stprintf (buffer, _T("%-4s R%d,%d"), mnemonic, sarg, darg);
		break;

	case format_11:     /* multiple precision instructions */
		OP2 = readop_arg(opram, pc);

		smode = BITS(OP2,10,11);
		sarg = BITS(OP2,12,15);
		dmode = BITS(OP2,4,5);
		darg = BITS(OP2,6,9);
		byte_count = BITS(OP2,0,3);

		buffer += _stprintf (buffer, _T("%-4s "), mnemonic);
		buffer += print_arg (buffer, smode, sarg, opram, pc, first_symbol);
		buffer += _stprintf (buffer, _T(","));
		buffer += print_arg (buffer, dmode, darg, opram, pc, first_symbol);
		buffer += _stprintf (buffer, byte_count ? _T(",%d") : _T(",R%d"), byte_count);
		break;

	case format_12:     /* string instructions */
		OP2 = readop_arg(opram, pc);

		smode = BITS(OP2,10,11);
		sarg = BITS(OP2,12,15);
		dmode = BITS(OP2,4,5);
		darg = BITS(OP2,6,9);
		byte_count = BITS(OP2,0,3);
		checkpoint = BITS(OP,12,15);

		buffer += _stprintf (buffer, _T("%-4s "), mnemonic);
		buffer += print_arg (buffer, smode, sarg, opram, pc, first_symbol);
		buffer += _stprintf (buffer, _T(","));
		buffer += print_arg (buffer, dmode, darg, opram, pc, first_symbol);
		buffer += _stprintf (buffer, byte_count ? _T(",%d,R%d") : _T(",R%d,R%d"), byte_count, checkpoint);
		break;

	case format_13:     /* multiple precision shift instructions */
		OP2 = readop_arg(opram, pc);

		smode = BITS(OP2,10,11);
		sarg = BITS(OP2,12,15);
		darg = BITS(OP2,6,9);
		byte_count = BITS(OP2,0,3);

		buffer += _stprintf (buffer, _T("%-4s "), mnemonic);
		buffer += print_arg (buffer, smode, sarg, opram, pc, first_symbol);
		buffer += _stprintf (buffer, byte_count ? _T(",%d") : _T(",R%d"), byte_count);
		buffer += _stprintf (buffer, darg ? _T(",%d") : _T(",R%d"), darg);
		break;

	case format_14:     /* bit testing instructions */
		OP2 = readop_arg(opram, pc);

		smode = BITS(OP2,10,11);
		sarg = BITS(OP2,12,15);
		darg = BITS(OP2,0,9);

		buffer += _stprintf (buffer, _T("%-4s "), mnemonic);
		buffer += print_arg (buffer, smode, sarg, opram, pc, first_symbol);
		if (darg == 0x3ff)
			buffer += _stprintf (buffer, _T(",R0"));
		else
			buffer += _stprintf (buffer, _T(",%d"), darg);
		break;

	case format_15:     /* invert order of field instruction */
		OP2 = readop_arg(opram, pc);

		smode = BITS(OP2,10,11);
		sarg = BITS(OP2,12,15);
		bit_position = BITS(OP2,0,3);
		bit_width = BITS(OP,12,15);

		buffer += _stprintf (buffer, _T("%-4s "), mnemonic);
		buffer += print_arg (buffer, smode, sarg, opram, pc, first_symbol);
		buffer += _stprintf (buffer, bit_position ? _T(",(%d,") : _T(",(R%d,"), bit_position);
		buffer += _stprintf (buffer, bit_width ? _T("%d)") : _T("R%d)"), bit_width);
		break;

	case format_16:     /* field instructions */
		OP2 = readop_arg(opram, pc);

		smode = BITS(OP2,10,11);
		sarg = BITS(OP2,12,15);
		dmode = BITS(OP2,4,5);
		darg = BITS(OP2,6,9);
		bit_position = BITS(OP2,0,3);
		bit_width = BITS(OP,12,15);

		buffer += _stprintf (buffer, _T("%-4s "), mnemonic);
		buffer += print_arg (buffer, smode, sarg, opram, pc, first_symbol);
		buffer += _stprintf (buffer, _T(","));
		buffer += print_arg (buffer, dmode, darg, opram, pc, first_symbol);
		buffer += _stprintf (buffer, bit_position ? _T(",(%d,") : _T(",(%d,"), bit_position);
		buffer += _stprintf (buffer, bit_width ? _T("%d)") : _T("R%d)"), bit_width);
		break;

	case format_17:     /* alter register and jump instructions */
		OP2 = readop_arg(opram, pc);

		displacement = (signed char)BITS(OP2,8,15);
		sarg = BITS(OP2,4,7);
		darg = BITS(OP2,0,3);
		if (darg)
		_stprintf (buffer, darg ? _T("%-4s >%s,%d,R%d") : _T("%-4s >%s,R%d,R%d"),
							mnemonic, get_value_or_symbol(first_symbol, _T("%04x"), 0xffff & (PC + displacement * 2)), sarg, darg);
		break;

	case format_18:     /* single register operand instructions */
		sarg = BITS(OP,12,15);

		_stprintf (buffer, _T("%-4s R%d"), mnemonic, sarg);
				break;

	case format_liim:   /* liim instruction */
		sarg = BITS(OP,14,15);

		_stprintf (buffer, _T("%-4s %d"), mnemonic, sarg);
		break;

	case format_19:     /* move address instruction */
		OP2 = readop_arg(opram, pc);

		smode = BITS(OP2,10,11);
		sarg = BITS(OP2,12,15);
		dmode = BITS(OP2,4,5);
		darg = BITS(OP2,6,9);

		buffer += _stprintf (buffer, _T("%-4s "), mnemonic);
		buffer += print_arg (buffer, smode, sarg, opram, pc, first_symbol);
		buffer += _stprintf (buffer, _T(","));
		buffer += print_arg (buffer, dmode, darg, opram, pc, first_symbol);
				break;

	case format_20:     /* list search instructions */
	{
			const _TCHAR *condition_code;

			OP2 = readop_arg(opram, pc);

			smode = BITS(OP2,10,11);
			sarg = BITS(OP2,12,15);
			dmode = BITS(OP2,4,5);
			darg = BITS(OP2,6,9);

			switch (BITS(OP2,0,3))
			{
			case 0:
				condition_code = _T("eq");
				break;
			case 1:
				condition_code = _T("ne");
				break;
			case 2:
				condition_code = _T("he");
				break;
			case 3:
				condition_code = _T("l");
				break;
			case 4:
				condition_code = _T("ge");
				break;
			case 5:
				condition_code = _T("lt");
				break;
			case 6:
				condition_code = _T("le");
				break;
			case 7:
				condition_code = _T("h");
				break;
			case 8:
				condition_code = _T("lte");
				break;
			case 9:
				condition_code = _T("gt");
				break;
			default:
				condition_code = _T("??");
				break;
			}

			buffer += _stprintf (buffer, _T("%-4s %s,"), mnemonic, condition_code);
			buffer += print_arg (buffer, smode, sarg, opram, pc, first_symbol);
			buffer += _stprintf (buffer, _T(","));
			buffer += print_arg (buffer, dmode, darg, opram, pc, first_symbol);
			break;
	}

	case format_21:     /* extend precision instruction */
	{
		int dest_byte_count;

		OP2 = readop_arg(opram, pc);

		smode = BITS(OP2,10,11);
		sarg = BITS(OP2,12,15);
		dmode = BITS(OP2,4,5);
		darg = BITS(OP2,6,9);
		byte_count = BITS(OP2,0,3);
		dest_byte_count = BITS(OP,12,15);

		buffer += _stprintf (buffer, _T("%-4s "), mnemonic);
		buffer += print_arg (buffer, smode, sarg, opram, pc, first_symbol);
		buffer += _stprintf (buffer, _T(","));
		buffer += print_arg (buffer, dmode, darg, opram, pc, first_symbol);
		buffer += _stprintf (buffer, byte_count ? _T(",%d") : _T(",R%d"), byte_count);
		buffer += _stprintf (buffer, dest_byte_count ? _T(",%d") : _T(",R%d"), dest_byte_count);
		break;
	}

	default:
		logerror(_T("debbugger internal error, file %s, line %d\n"), __FILE__, __LINE__);
	case illegal:
		_stprintf (buffer, _T("data >%04x"), OP);
		break;
	}

	return (PC - pc) | DASMFLAG_SUPPORTED | dasmflags;
}

CPU_DISASSEMBLE( tms9900 )
{
	return Dasm9900(buffer, pc, TMS9900_ID, oprom, opram, first_symbol);
}

CPU_DISASSEMBLE( tms9980 )
{
	return Dasm9900(buffer, pc, TMS9980_ID, oprom, opram, first_symbol);
}

CPU_DISASSEMBLE( tms9995 )
{
	return Dasm9900(buffer, pc, TMS9995_ID, oprom, opram, first_symbol);
}
