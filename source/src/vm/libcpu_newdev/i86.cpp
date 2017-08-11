/*
	Skelton for retropc emulator

	Origin : MAME 0.142
	Author : Takeda.Toshiya
	Date  : 2011.04.23-

	[ i86/v30 ]
*/

#if defined(_MSC_VER) && (_MSC_VER >= 1400)
#pragma warning( disable : 4146 )
#pragma warning( disable : 4996 )
#endif

#include "./i86.h"
#ifdef USE_DEBUGGER
#include "debugger.h"
#endif
#include "./i86_macros.h"

extern int necv_dasm_one(_TCHAR *buffer, UINT32 eip, const UINT8 *oprom);

void I86::initialize()
{
	static const BREGS reg_name[8] = {AL, CL, DL, BL, AH, CH, DH, BH};
	
	DEVICE::initialize();
	for(int i = 0; i < 256; i++) {
		Mod_RM.reg.b[i] = reg_name[(i & 0x38) >> 3];
		Mod_RM.reg.w[i] = (WREGS)((i & 0x38) >> 3);
	}
	for(int i = 0xc0; i < 0x100; i++) {
		Mod_RM.RM.w[i] = (WREGS)(i & 7);
		Mod_RM.RM.b[i] = (BREGS)reg_name[i & 7];
	}
	//memcpy(parity_table, _i80x86_parity_table, sizeof(uint8_t) * 256);
#if defined(HAS_I86)
	timing = _i80x86_timing_tbl_i8086;
#else
	timing = _i80x86_timing_tbl_i80186;
#endif
	d_mem_stored = d_mem;
	d_io_stored = d_io;
#ifdef USE_DEBUGGER
	d_debugger->set_context_mem(d_mem);
	d_debugger->set_context_io(d_io);
#endif
}

void I86::reset()
{
	for(int i = 0; i < 8; i++) {
		regs.w[i] = 0;
	}
	sregs[CS] = 0xf000;
	sregs[SS] = sregs[DS] = sregs[ES] = 0;
	
	base[CS] = SegBase(CS);
	base[SS] = base[DS] = base[ES] = 0;
	
	ea = 0;
	eo = 0;
	AuxVal = OverVal = SignVal = ZeroVal = CarryVal = 0;
	DirVal = 1;
	ParityVal = TF = IF = MF = 0;
	
	icount = extra_icount = 0;
	int_state = 0;
	test_state = false;
	halted = false;
	
	pc = 0xffff0 & AMASK;
	flags = 0;
	ExpandFlags(flags);
#ifdef HAS_V30
	SetMD(1);
#endif
	seg_prefix = false;
}

int I86::run(int clock)
{
	/* return now if BUSREQ */
	if(busreq) {
#ifdef SINGLE_MODE_DMA
		if(d_dma) {
			d_dma->do_dma();
		}
#endif
		if(clock == -1) {
			int passed_icount = max(1, extra_icount);
			// this is main cpu, icount is not used
			/*icount = */extra_icount = 0;
			return passed_icount;
		} else {
			icount += clock;
			int first_icount = icount;
			
			/* adjust for any interrupts that came in */
			icount -= extra_icount;
			extra_icount = 0;
			
			/* if busreq is raised, spin cpu while remained clock */
			if(icount > 0) {
				icount = 0;
			}
			return first_icount - icount;
		}
	}
	
	if(clock == -1) {
		/* run only one opcode */
		icount = -extra_icount;
		extra_icount = 0;
#ifdef USE_DEBUGGER
		run_one_opecode_debugger();
#else
		run_one_opecode();
#endif
		return -icount;
	} else {
		icount += clock;
		int first_icount = icount;
		
		/* adjust for any interrupts that came in */
		icount -= extra_icount;
		extra_icount = 0;
		
		/* run cpu while given clocks */
		while(icount > 0 && !busreq) {
#ifdef USE_DEBUGGER
			run_one_opecode_debugger();
#else
			run_one_opecode();
#endif
		}
		/* if busreq is raised, spin cpu while remained clock */
		if(icount > 0 && busreq) {
			icount = 0;
		}
		return first_icount - icount;
	}
}

void I86::run_one_opecode_debugger()
{
#ifdef USE_DEBUGGER
	bool now_debugging = d_debugger->now_debugging;
	if(now_debugging) {
		d_debugger->check_break_points(pc);
		if(d_debugger->now_suspended) {
			emu->mute_sound();
			d_debugger->now_waiting = true;
			while(d_debugger->now_debugging && d_debugger->now_suspended) {
				emu->sleep(10);
			}
			d_debugger->now_waiting = false;
		}
		if(d_debugger->now_debugging) {
			d_mem = d_io = d_debugger;
		} else {
			now_debugging = false;
		}
		
		run_one_opecode();
		if(now_debugging) {
			if(!d_debugger->now_going) {
				d_debugger->now_suspended = true;
			}
			d_mem = d_mem_stored;
			d_io = d_io_stored;
		}
	} else {
		run_one_opecode();
	}
#else
	run_one_opecode();
#endif
}


void I86::run_one_opecode()
{
	seg_prefix = false;
#ifdef _JX
	// ugly patch for PC/JX hardware diagnostics :-(
#ifdef TIMER_HACK
	if(pc == 0xff040) pc = 0xff04a;
	if(pc == 0xff17d) pc = 0xff18f;
#endif
#ifdef KEYBOARD_HACK
	if(pc == 0xfa909) { regs.b[BH] = read_port_byte(0xa1); pc = 0xfa97c; }
	if(pc == 0xff6e1) { regs.b[AL] = 0x0d; pc += 2; }
#endif
#endif
	instruction(FETCHOP);
#ifdef SINGLE_MODE_DMA
	if(d_dma) {
		d_dma->do_dma();
	}
#endif
	if(int_state & NMI_REQ_BIT) {
		if(halted) {
			pc++;
			halted = false;
		}
		int_state &= ~NMI_REQ_BIT;
		interrupt(NMI_INT_VECTOR);
	} else if((int_state & INT_REQ_BIT) && IF) {
		if(halted) {
			pc++;
			halted = false;
		}
		interrupt(-1);
	}
	icount -= extra_icount;
	extra_icount = 0;
}

void I86::instruction(uint8_t code)
{
	prevpc = pc - 1;
	
	switch(code) {
	case 0x00: _add_br8(); break;
	case 0x01: _add_wr16(); break;
	case 0x02: _add_r8b(); break;
	case 0x03: _add_r16w(); break;
	case 0x04: _add_ald8(); break;
	case 0x05: _add_axd16(); break;
	case 0x06: _push_es(); break;
	case 0x07: _pop_es(); break;
	case 0x08: _or_br8(); break;
	case 0x09: _or_wr16(); break;
	case 0x0a: _or_r8b(); break;
	case 0x0b: _or_r16w(); break;
	case 0x0c: _or_ald8(); break;
	case 0x0d: _or_axd16(); break;
	case 0x0e: _push_cs(); break;
#if defined(HAS_V30)
	case 0x0f: _0fpre(); break;
#else
	case 0x0f: _invalid(); break;
#endif
	case 0x10: _adc_br8(); break;
	case 0x11: _adc_wr16(); break;
	case 0x12: _adc_r8b(); break;
	case 0x13: _adc_r16w(); break;
	case 0x14: _adc_ald8(); break;
	case 0x15: _adc_axd16(); break;
	case 0x16: _push_ss(); break;
	case 0x17: _pop_ss(); break;
	case 0x18: _sbb_br8(); break;
	case 0x19: _sbb_wr16(); break;
	case 0x1a: _sbb_r8b(); break;
	case 0x1b: _sbb_r16w(); break;
	case 0x1c: _sbb_ald8(); break;
	case 0x1d: _sbb_axd16(); break;
	case 0x1e: _push_ds(); break;
	case 0x1f: _pop_ds(); break;
	case 0x20: _and_br8(); break;
	case 0x21: _and_wr16(); break;
	case 0x22: _and_r8b(); break;
	case 0x23: _and_r16w(); break;
	case 0x24: _and_ald8(); break;
	case 0x25: _and_axd16(); break;
	case 0x26: _es(); break;
	case 0x27: _daa(); break;
	case 0x28: _sub_br8(); break;
	case 0x29: _sub_wr16(); break;
	case 0x2a: _sub_r8b(); break;
	case 0x2b: _sub_r16w(); break;
	case 0x2c: _sub_ald8(); break;
	case 0x2d: _sub_axd16(); break;
	case 0x2e: _cs(); break;
	case 0x2f: _das(); break;
	case 0x30: _xor_br8(); break;
	case 0x31: _xor_wr16(); break;
	case 0x32: _xor_r8b(); break;
	case 0x33: _xor_r16w(); break;
	case 0x34: _xor_ald8(); break;
	case 0x35: _xor_axd16(); break;
	case 0x36: _ss(); break;
	case 0x37: _aaa(); break;
	case 0x38: _cmp_br8(); break;
	case 0x39: _cmp_wr16(); break;
	case 0x3a: _cmp_r8b(); break;
	case 0x3b: _cmp_r16w(); break;
	case 0x3c: _cmp_ald8(); break;
	case 0x3d: _cmp_axd16(); break;
	case 0x3e: _ds(); break;
	case 0x3f: _aas(); break;
	case 0x40: _inc_ax(); break;
	case 0x41: _inc_cx(); break;
	case 0x42: _inc_dx(); break;
	case 0x43: _inc_bx(); break;
	case 0x44: _inc_sp(); break;
	case 0x45: _inc_bp(); break;
	case 0x46: _inc_si(); break;
	case 0x47: _inc_di(); break;
	case 0x48: _dec_ax(); break;
	case 0x49: _dec_cx(); break;
	case 0x4a: _dec_dx(); break;
	case 0x4b: _dec_bx(); break;
	case 0x4c: _dec_sp(); break;
	case 0x4d: _dec_bp(); break;
	case 0x4e: _dec_si(); break;
	case 0x4f: _dec_di(); break;
	case 0x50: _push_ax(); break;
	case 0x51: _push_cx(); break;
	case 0x52: _push_dx(); break;
	case 0x53: _push_bx(); break;
	case 0x54: _push_sp(); break;
	case 0x55: _push_bp(); break;
	case 0x56: _push_si(); break;
	case 0x57: _push_di(); break;
	case 0x58: _pop_ax(); break;
	case 0x59: _pop_cx(); break;
	case 0x5a: _pop_dx(); break;
	case 0x5b: _pop_bx(); break;
	case 0x5c: _pop_sp(); break;
	case 0x5d: _pop_bp(); break;
	case 0x5e: _pop_si(); break;
	case 0x5f: _pop_di(); break;
#if defined(HAS_V30)
	case 0x60: _pusha(); break;
	case 0x61: _popa(); break;
	case 0x62: _bound(); break;
#else
	case 0x60: _invalid(); break;
	case 0x61: _invalid(); break;
	case 0x62: _invalid(); break;
#endif
	case 0x63: _invalid(); break;
#if defined(HAS_V30)
	case 0x64: _repc(0); break;
	case 0x65: _repc(1); break;
#else
	case 0x64: _invalid(); break;
	case 0x65: _invalid(); break;
#endif
	case 0x66: _invalid(); break;
	case 0x67: _invalid(); break;
#if defined(HAS_V30)
	case 0x68: _push_d16(); break;
	case 0x69: _imul_d16(); break;
	case 0x6a: _push_d8(); break;
	case 0x6b: _imul_d8(); break;
	case 0x6c: _insb(); break;
	case 0x6d: _insw(); break;
	case 0x6e: _outsb(); break;
	case 0x6f: _outsw(); break;
#else
	case 0x68: _invalid(); break;
	case 0x69: _invalid(); break;
	case 0x6a: _invalid(); break;
	case 0x6b: _invalid(); break;
	case 0x6c: _invalid(); break;
	case 0x6d: _invalid(); break;
	case 0x6e: _invalid(); break;
	case 0x6f: _invalid(); break;
#endif
	case 0x70: _jo(); break;
	case 0x71: _jno(); break;
	case 0x72: _jb(); break;
	case 0x73: _jnb(); break;
	case 0x74: _jz(); break;
	case 0x75: _jnz(); break;
	case 0x76: _jbe(); break;
	case 0x77: _jnbe(); break;
	case 0x78: _js(); break;
	case 0x79: _jns(); break;
	case 0x7a: _jp(); break;
	case 0x7b: _jnp(); break;
	case 0x7c: _jl(); break;
	case 0x7d: _jnl(); break;
	case 0x7e: _jle(); break;
	case 0x7f: _jnle(); break;
	case 0x80: _80pre(); break;
	case 0x81: _81pre(); break;
	case 0x82: _82pre(); break;
	case 0x83: _83pre(); break;
	case 0x84: _test_br8(); break;
	case 0x85: _test_wr16(); break;
	case 0x86: _xchg_br8(); break;
	case 0x87: _xchg_wr16(); break;
	case 0x88: _mov_br8(); break;
	case 0x89: _mov_wr16(); break;
	case 0x8a: _mov_r8b(); break;
	case 0x8b: _mov_r16w(); break;
	case 0x8c: _mov_wsreg(); break;
	case 0x8d: _lea(); break;
	case 0x8e: _mov_sregw(); break;
	case 0x8f: _popw(); break;
	case 0x90: _nop(); break;
	case 0x91: _xchg_axcx(); break;
	case 0x92: _xchg_axdx(); break;
	case 0x93: _xchg_axbx(); break;
	case 0x94: _xchg_axsp(); break;
	case 0x95: _xchg_axbp(); break;
	case 0x96: _xchg_axsi(); break;
	case 0x97: _xchg_axdi(); break;
	case 0x98: _cbw(); break;
	case 0x99: _cwd(); break;
	case 0x9a: _call_far(); break;
	case 0x9b: _wait(); break;
	case 0x9c: _pushf(); break;
	case 0x9d: _popf(); break;
	case 0x9e: _sahf(); break;
	case 0x9f: _lahf(); break;
	case 0xa0: _mov_aldisp(); break;
	case 0xa1: _mov_axdisp(); break;
	case 0xa2: _mov_dispal(); break;
	case 0xa3: _mov_dispax(); break;
	case 0xa4: _movsb(); break;
	case 0xa5: _movsw(); break;
	case 0xa6: _cmpsb(); break;
	case 0xa7: _cmpsw(); break;
	case 0xa8: _test_ald8(); break;
	case 0xa9: _test_axd16(); break;
	case 0xaa: _stosb(); break;
	case 0xab: _stosw(); break;
	case 0xac: _lodsb(); break;
	case 0xad: _lodsw(); break;
	case 0xae: _scasb(); break;
	case 0xaf: _scasw(); break;
	case 0xb0: _mov_ald8(); break;
	case 0xb1: _mov_cld8(); break;
	case 0xb2: _mov_dld8(); break;
	case 0xb3: _mov_bld8(); break;
	case 0xb4: _mov_ahd8(); break;
	case 0xb5: _mov_chd8(); break;
	case 0xb6: _mov_dhd8(); break;
	case 0xb7: _mov_bhd8(); break;
	case 0xb8: _mov_axd16(); break;
	case 0xb9: _mov_cxd16(); break;
	case 0xba: _mov_dxd16(); break;
	case 0xbb: _mov_bxd16(); break;
	case 0xbc: _mov_spd16(); break;
	case 0xbd: _mov_bpd16(); break;
	case 0xbe: _mov_sid16(); break;
	case 0xbf: _mov_did16(); break;
#if defined(HAS_V30)
	case 0xc0: _rotshft_bd8(); break;
	case 0xc1: _rotshft_wd8(); break;
#else
	case 0xc0: _invalid(); break;
	case 0xc1: _invalid(); break;
#endif
	case 0xc2: _ret_d16(); break;
	case 0xc3: _ret(); break;
	case 0xc4: _les_dw(); break;
	case 0xc5: _lds_dw(); break;
	case 0xc6: _mov_bd8(); break;
	case 0xc7: _mov_wd16(); break;
#if defined(HAS_V30)
	case 0xc8: _enter(); break;
	case 0xc9: _leav(); break;	/* _leave() */
#else
	case 0xc8: _invalid(); break;
	case 0xc9: _invalid(); break;
#endif
	case 0xca: _retf_d16(); break;
	case 0xcb: _retf(); break;
	case 0xcc: _int3(); break;
	case 0xcd: _int(); break;
	case 0xce: _into(); break;
	case 0xcf: _iret(); break;
	case 0xd0: _rotshft_b(); break;
	case 0xd1: _rotshft_w(); break;
	case 0xd2: _rotshft_bcl(); break;
	case 0xd3: _rotshft_wcl(); break;
	case 0xd4: _aam(); break;
	case 0xd5: _aad(); break;
#if defined(HAS_V30)
	case 0xd6: _setalc(); break;
#else
	case 0xd6: _invalid(); break;
#endif
	case 0xd7: _xlat(); break;
	case 0xd8: _escape(); break;
	case 0xd9: _escape(); break;
	case 0xda: _escape(); break;
	case 0xdb: _escape(); break;
	case 0xdc: _escape(); break;
	case 0xdd: _escape(); break;
	case 0xde: _escape(); break;
	case 0xdf: _escape(); break;
	case 0xe0: _loopne(); break;
	case 0xe1: _loope(); break;
	case 0xe2: _loop(); break;
	case 0xe3: _jcxz(); break;
	case 0xe4: _inal(); break;
	case 0xe5: _inax(); break;
	case 0xe6: _outal(); break;
	case 0xe7: _outax(); break;
	case 0xe8: _call_d16(); break;
	case 0xe9: _jmp_d16(); break;
	case 0xea: _jmp_far(); break;
	case 0xeb: _jmp_d8(); break;
	case 0xec: _inaldx(); break;
	case 0xed: _inaxdx(); break;
	case 0xee: _outdxal(); break;
	case 0xef: _outdxax(); break;
	case 0xf0: _lock(); break;
	case 0xf1: _invalid(); break;
	case 0xf2: _repne(); break;
	case 0xf3: _repe(); break;
	case 0xf4: _hlt(); break;
	case 0xf5: _cmc(); break;
	case 0xf6: _f6pre(); break;
	case 0xf7: _f7pre(); break;
	case 0xf8: _clc(); break;
	case 0xf9: _stc(); break;
	case 0xfa: _cli(); break;
	case 0xfb: _sti(); break;
	case 0xfc: _cld(); break;
	case 0xfd: _std(); break;
	case 0xfe: _fepre(); break;
	case 0xff: _ffpre(); break;
#if defined(_MSC_VER) && (_MSC_VER >= 1200)
	default: __assume(0);
#endif
	}
}

#if defined(HAS_V30)
void I86::_0fpre()    /* Opcode 0x0f */
{
	static const uint16_t bytes[] = {
		1, 2, 4, 8, 16, 32, 64, 128, 256, 512, 1024, 2048, 4096, 8192, 16384, 32768
	};
	unsigned code = FETCH;
	unsigned ModRM;
	unsigned tmp;
	unsigned tmp2;
	
	switch(code) {
	case 0x10:  /* 0F 10 47 30 - TEST1 [bx+30h], cl */
		ModRM = FETCH;
		if(ModRM >= 0xc0) {
			tmp = regs.b[Mod_RM.RM.b[ModRM]];
			icount -= 3;
		} else {
			int old = icount;
			GetEA(ModRM);
			tmp = ReadByte(ea);
			icount = old - 12;
		}
		tmp2 = regs.b[CL] & 7;
		SetZF(tmp & bytes[tmp2]);
		break;
	case 0x11:  /* 0F 11 47 30 - TEST1 [bx+30h], cl */
		ModRM = FETCH;
		if(ModRM >= 0xc0) {
			tmp = regs.w[Mod_RM.RM.w[ModRM]];
			icount -= 3;
		} else {
			int old = icount;
			GetEA(ModRM);
			tmp = ReadWord(ea);
			icount = old - 12;
		}
		tmp2 = regs.b[CL] & 0xf;
		SetZF(tmp & bytes[tmp2]);
		break;
	case 0x12:  /* 0F 12 [mod:000:r/m] - CLR1 reg/m8, cl */
		ModRM = FETCH;
		if(ModRM >= 0xc0) {
			tmp = regs.b[Mod_RM.RM.b[ModRM]];
			icount -= 5;
		} else {
			int old = icount;
			GetEA(ModRM);
			tmp = ReadByte(ea);
			icount = old - 14;
		}
		tmp2 = regs.b[CL] & 7;
		tmp &= ~bytes[tmp2];
		PutbackRMByte(ModRM, tmp);
		break;
	case 0x13:  /* 0F 13 [mod:000:r/m] - CLR1 reg/m16, cl */
		ModRM = FETCH;
		if(ModRM >= 0xc0) {
			tmp = regs.w[Mod_RM.RM.w[ModRM]];
			icount -= 5;
		} else {
			int old = icount;
			GetEA(ModRM);
			tmp = ReadWord(ea);
			icount = old - 14;
		}
		tmp2 = regs.b[CL] & 0xf;
		tmp &= ~bytes[tmp2];
		PutbackRMWord(ModRM, tmp);
		break;
	case 0x14:  /* 0F 14 47 30 - SET1 [bx+30h], cl */
		ModRM = FETCH;
		if(ModRM >= 0xc0) {
			tmp = regs.b[Mod_RM.RM.b[ModRM]];
			icount -= 4;
		} else {
			int old = icount;
			GetEA(ModRM);
			tmp = ReadByte(ea);
			icount = old - 13;
		}
		tmp2 = regs.b[CL] & 7;
		tmp |= bytes[tmp2];
		PutbackRMByte(ModRM, tmp);
		break;
	case 0x15:  /* 0F 15 C6 - SET1 si, cl */
		ModRM = FETCH;
		if(ModRM >= 0xc0) {
			tmp = regs.w[Mod_RM.RM.w[ModRM]];
			icount -= 4;
		} else {
			int old = icount;
			GetEA(ModRM);
			tmp = ReadWord(ea);
			icount = old - 13;
		}
		tmp2 = regs.b[CL] & 0xf;
		tmp |= bytes[tmp2];
		PutbackRMWord(ModRM, tmp);
		break;
	case 0x16:  /* 0F 16 C6 - NOT1 si, cl */
		ModRM = FETCH;
		if(ModRM >= 0xc0) {
			tmp = regs.b[Mod_RM.RM.b[ModRM]];
			icount -= 4;
		} else {
			int old = icount;
			GetEA(ModRM);
			tmp = ReadByte(ea);
			icount = old - 18;
		}
		tmp2 = regs.b[CL] & 7;
		if(tmp & bytes[tmp2]) {
			tmp &= ~bytes[tmp2];
		} else {
			tmp |= bytes[tmp2];
		}
		PutbackRMByte(ModRM, tmp);
		break;
	case 0x17:  /* 0F 17 C6 - NOT1 si, cl */
		ModRM = FETCH;
		if(ModRM >= 0xc0) {
			tmp = regs.w[Mod_RM.RM.w[ModRM]];
			icount -= 4;
		} else {
			int old = icount;
			GetEA(ModRM);
			tmp = ReadWord(ea);
			icount = old - 18;
		}
		tmp2 = regs.b[CL] & 0xf;
		if(tmp & bytes[tmp2]) {
			tmp &= ~bytes[tmp2];
		} else {
			tmp |= bytes[tmp2];
		}
		PutbackRMWord(ModRM, tmp);
		break;
	case 0x18:  /* 0F 18 XX - TEST1 [bx+30h], 07 */
		ModRM = FETCH;
		if(ModRM >= 0xc0) {
			tmp = regs.b[Mod_RM.RM.b[ModRM]];
			icount -= 4;
		} else {
			int old = icount;
			GetEA(ModRM);
			tmp = ReadByte(ea);
			icount = old - 13;
		}
		tmp2 = FETCH;
		tmp2 &= 0xf;
		SetZF(tmp & bytes[tmp2]);
		break;
	case 0x19:  /* 0F 19 XX - TEST1 [bx+30h], 07 */
		ModRM = FETCH;
		if(ModRM >= 0xc0) {
			tmp = regs.w[Mod_RM.RM.w[ModRM]];
			icount -= 4;
		} else {
			int old = icount;
			GetEA(ModRM);
			tmp = ReadWord(ea);
			icount = old - 13;
		}
		tmp2 = FETCH;
		tmp2 &= 0xf;
		SetZF(tmp & bytes[tmp2]);
		break;
	case 0x1a:  /* 0F 1A 06 - CLR1 si, cl */
		ModRM = FETCH;
		if(ModRM >= 0xc0) {
			tmp = regs.b[Mod_RM.RM.b[ModRM]];
			icount -= 6;
		} else {
			int old = icount;
			GetEA(ModRM);
			tmp = ReadByte(ea);
			icount = old - 15;
		}
		tmp2 = FETCH;
		tmp2 &= 7;
		tmp &= ~bytes[tmp2];
		PutbackRMByte(ModRM, tmp);
		break;
	case 0x1B:  /* 0F 1B 06 - CLR1 si, cl */
		ModRM = FETCH;
		if(ModRM >= 0xc0) {
			tmp = regs.w[Mod_RM.RM.w[ModRM]];
			icount -= 6;
		} else {
			int old = icount;
			GetEA(ModRM);
			tmp = ReadWord(ea);
			icount = old - 15;
		}
		tmp2 = FETCH;
		tmp2 &= 0xf;
		tmp &= ~bytes[tmp2];
		PutbackRMWord(ModRM, tmp);
		break;
	case 0x1C:  /* 0F 1C 47 30 - SET1 [bx+30h], cl */
		ModRM = FETCH;
		if(ModRM >= 0xc0) {
			tmp = regs.b[Mod_RM.RM.b[ModRM]];
			icount -= 5;
		} else {
			int old = icount;
			GetEA(ModRM);
			tmp = ReadByte(ea);
			icount = old - 14;
		}
		tmp2 = FETCH;
		tmp2 &= 7;
		tmp |= bytes[tmp2];
		PutbackRMByte(ModRM, tmp);
		break;
	case 0x1D:  /* 0F 1D C6 - SET1 si, cl */
		ModRM = FETCH;
		if(ModRM >= 0xc0) {
			tmp = regs.w[Mod_RM.RM.w[ModRM]];
			icount -= 5;
		} else {
			int old = icount;
			GetEA(ModRM);
			tmp = ReadWord(ea);
			icount = old - 14;
		}
		tmp2 = FETCH;
		tmp2 &= 0xf;
		tmp |= bytes[tmp2];
		PutbackRMWord(ModRM, tmp);
		break;
	case 0x1e:  /* 0F 1e C6 - NOT1 si, 07 */
		ModRM = FETCH;
		if(ModRM >= 0xc0) {
			tmp = regs.b[Mod_RM.RM.b[ModRM]];
			icount -= 5;
		} else {
			int old = icount;
			GetEA(ModRM);
			tmp = ReadByte(ea);
			icount = old - 19;
		}
		tmp2 = FETCH;
		tmp2 &= 7;
		if(tmp & bytes[tmp2]) {
			tmp &= ~bytes[tmp2];
		} else {
			tmp |= bytes[tmp2];
		}
		PutbackRMByte(ModRM, tmp);
		break;
	case 0x1f:  /* 0F 1f C6 - NOT1 si, 07 */
		ModRM = FETCH;
		if(ModRM >= 0xc0) {
			tmp = regs.w[Mod_RM.RM.w[ModRM]];
			icount -= 5;
		} else {
			int old = icount;
			GetEA(ModRM);
			tmp = ReadWord(ea);
			icount = old - 19;
		}
		tmp2 = FETCH;
		tmp2 &= 0xf;
		if(tmp & bytes[tmp2]) {
			tmp &= ~bytes[tmp2];
		} else {
			tmp |= bytes[tmp2];
		}
		PutbackRMWord(ModRM, tmp);
		break;
	case 0x20:  /* 0F 20 59 - add4s */
		{
			/* length in words ! */
			int count = (regs.b[CL] + 1) / 2;
			unsigned di = regs.w[DI];
			unsigned si = regs.w[SI];
			
			ZeroVal = 1;
			CarryVal = 0;	/* NOT ADC */
			for(int i = 0; i < count; i++) {
				tmp = GetMemB(DS, si);
				tmp2 = GetMemB(ES, di);
				int v1 = (tmp >> 4) * 10 + (tmp & 0xf);
				int v2 = (tmp2 >> 4) * 10 + (tmp2 & 0xf);
				int result = v1 + v2 + CarryVal;
				CarryVal = result > 99 ? 1 : 0;
				result = result % 100;
				v1 = ((result / 10) << 4) | (result % 10);
				PutMemB(ES, di, v1);
				if(v1) {
					ZeroVal = 0;
				}
				si++;
				di++;
			}
			OverVal = CarryVal;
			icount -= 7 + 19 * count;
		}
		break;
	case 0x22:  /* 0F 22 59 - sub4s */
		{
			int count = (regs.b[CL] + 1) / 2;
			unsigned di = regs.w[DI];
			unsigned si = regs.w[SI];
			
			ZeroVal = 1;
			CarryVal = 0;  /* NOT ADC */
			for(int i = 0; i < count; i++) {
				tmp = GetMemB(ES, di);
				tmp2 = GetMemB(DS, si);
				int v1 = (tmp >> 4) * 10 + (tmp & 0xf);
				int v2 = (tmp2 >> 4) * 10 + (tmp2 & 0xf), result;
				if(v1 < (v2 + CarryVal)) {
					v1 += 100;
					result = v1 - (v2 + CarryVal);
					CarryVal = 1;
				} else {
					result = v1 - (v2 + CarryVal);
					CarryVal = 0;
				}
				v1 = ((result / 10) << 4) | (result % 10);
				PutMemB(ES, di, v1);
				if(v1) {
					ZeroVal = 0;
				}
				si++;
				di++;
			}
			OverVal = CarryVal;
			icount -= 7 + 19 * count;
		}
		break;
	case 0x25:
		icount -= 16;
		break;
	case 0x26:  /* 0F 22 59 - cmp4s */
		{
			int count = (regs.b[CL] + 1) / 2;
			unsigned di = regs.w[DI];
			unsigned si = regs.w[SI];
			
			ZeroVal = 1;
			CarryVal = 0;	/* NOT ADC */
			for(int i = 0; i < count; i++) {
				tmp = GetMemB(ES, di);
				tmp2 = GetMemB(DS, si);
				int v1 = (tmp >> 4) * 10 + (tmp & 0xf);
				int v2 = (tmp2 >> 4) * 10 + (tmp2 & 0xf), result;
				if(v1 < (v2 + CarryVal)) {
					v1 += 100;
					result = v1 - (v2 + CarryVal);
					CarryVal = 1;
				} else {
					result = v1 - (v2 + CarryVal);
					CarryVal = 0;
				}
				v1 = ((result / 10) << 4) | (result % 10);
				/* PutMemB(ES, di, v1);	/* no store, only compare */
				if(v1) {
					ZeroVal = 0;
				}
				si++;
				di++;
			}
			OverVal = CarryVal;
			icount -= 7 + 19 * (regs.b[CL] + 1);
		}
		break;
	case 0x28:  /* 0F 28 C7 - ROL4 bh */
		ModRM = FETCH;
		if(ModRM >= 0xc0) {
			tmp = regs.b[Mod_RM.RM.b[ModRM]];
			icount -= 25;
		} else {
			int old = icount;
			GetEA(ModRM);
			tmp = ReadByte(ea);
			icount = old - 28;
		}
		tmp <<= 4;
		tmp |= regs.b[AL] & 0xf;
		regs.b[AL] = (regs.b[AL] & 0xf0) | ((tmp >> 8) & 0xf);
		tmp &= 0xff;
		PutbackRMByte(ModRM, tmp);
		break;
	case 0x29:  /* 0F 29 C7 - ROL4 bx */
		ModRM = FETCH;
		break;
	case 0x2A:  /* 0F 2a c2 - ROR4 bh */
		ModRM = FETCH;
		if(ModRM >= 0xc0) {
			tmp = regs.b[Mod_RM.RM.b[ModRM]];
			icount -= 29;
		} else {
			int old = icount;
			GetEA(ModRM);
			tmp = ReadByte(ea);
			icount = old - 33;
		}
		tmp2 = (regs.b[AL] & 0xf) << 4;
		regs.b[AL] = (regs.b[AL] & 0xf0) | (tmp & 0xf);
		tmp = tmp2 | (tmp >> 4);
		PutbackRMByte(ModRM, tmp);
		break;
	case 0x2B:  /* 0F 2b c2 - ROR4 bx */
		ModRM = FETCH;
		break;
	case 0x2D:  /* 0Fh 2Dh < 1111 1RRR> */
		ModRM = FETCH;
		icount -= 15;
		break;
	case 0x31:  /* 0F 31 [mod:reg:r/m] - INS reg8, reg8 or INS reg8, imm4 */
		ModRM = FETCH;
		if(ModRM >= 0xc0) {
			tmp = regs.b[Mod_RM.RM.b[ModRM]];
			icount -= 29;
		} else {
			int old = icount;
			GetEA(ModRM);
			tmp = ReadByte(ea);
			icount = old - 33;
		}
		break;
	case 0x33:  /* 0F 33 [mod:reg:r/m] - EXT reg8, reg8 or EXT reg8, imm4 */
		ModRM = FETCH;
		if(ModRM >= 0xc0) {
			tmp = regs.b[Mod_RM.RM.b[ModRM]];
			icount -= 29;
		} else {
			int old = icount;
			GetEA(ModRM);
			tmp = ReadByte(ea);
			icount = old - 33;
		}
		break;
	case 0x91:
		icount -= 12;
		break;
	case 0x94:
		ModRM = FETCH;
		icount -= 11;
		break;
	case 0x95:
		ModRM = FETCH;
		icount -= 11;
		break;
	case 0xbe:
		icount -= 2;
		break;
	case 0xe0:
		ModRM = FETCH;
		icount -= 12;
		break;
	case 0xf0:
		ModRM = FETCH;
		icount -= 12;
		break;
	case 0xff:  /* 0F ff imm8 - BRKEM */
		ModRM = FETCH;
		icount -= 38;
		interrupt(ModRM);
		break;
	}
}
#endif


void I86::_repc(int flagval)
{
#ifdef HAS_V30
	unsigned next = FETCHOP;
	unsigned count = regs.w[CX];
	
	switch(next) {
	case 0x26:	/* ES: */
		seg_prefix = true;
		prefix_seg = ES;
		icount -= 2;
		_repc(flagval);
		break;
	case 0x2e:	/* CS: */
		seg_prefix = true;
		prefix_seg = CS;
		icount -= 2;
		_repc(flagval);
		break;
	case 0x36:	/* SS: */
		seg_prefix = true;
		prefix_seg = SS;
		icount -= 2;
		_repc(flagval);
		break;
	case 0x3e:	/* DS: */
		seg_prefix = true;
		prefix_seg = DS;
		icount -= 2;
		_repc(flagval);
		break;
	case 0x6c:	/* REP INSB */
		icount -= 9 - count;
		for(; (CF == flagval) && (count > 0); count--) {
			_insb();
		}
		regs.w[CX] = count;
		break;
	case 0x6d:	/* REP INSW */
		icount -= 9 - count;
		for(; (CF == flagval) && (count > 0); count--) {
			_insw();
		}
		regs.w[CX] = count;
		break;
	case 0x6e:	/* REP OUTSB */
		icount -= 9 - count;
		for(; (CF == flagval) && (count > 0); count--) {
			_outsb();
		}
		regs.w[CX] = count;
		break;
	case 0x6f:	/* REP OUTSW */
		icount -= 9 - count;
		for(; (CF == flagval) && (count > 0); count--) {
			_outsw();
		}
		regs.w[CX] = count;
		break;
	case 0xa4:	/* REP MOVSB */
		icount -= 9 - count;
		for(; (CF == flagval) && (count > 0); count--) {
			_movsb();
		}
		regs.w[CX] = count;
		break;
	case 0xa5:	/* REP MOVSW */
		icount -= 9 - count;
		for(; (CF == flagval) && (count > 0); count--) {
			_movsw();
		}
		regs.w[CX] = count;
		break;
	case 0xa6:	/* REP(N)E CMPSB */
		icount -= 9;
		for(ZeroVal = !flagval; (ZF == flagval) && (CF == flagval) && (count > 0); count--) {
			_cmpsb();
		}
		regs.w[CX] = count;
		break;
	case 0xa7:	/* REP(N)E CMPSW */
		icount -= 9;
		for(ZeroVal = !flagval; (ZF == flagval) && (CF == flagval) && (count > 0); count--) {
			_cmpsw();
		}
		regs.w[CX] = count;
		break;
	case 0xaa:	/* REP STOSB */
		icount -= 9 - count;
		for(; (CF == flagval) && (count > 0); count--) {
			_stosb();
		}
		regs.w[CX] = count;
		break;
	case 0xab:	/* REP STOSW */
		icount -= 9 - count;
		for(; (CF == flagval) && (count > 0); count--) {
			_stosw();
		}
		regs.w[CX] = count;
		break;
	case 0xac:	/* REP LODSB */
		icount -= 9;
		for(; (CF == flagval) && (count > 0); count--) {
			_lodsb();
		}
		regs.w[CX] = count;
		break;
	case 0xad:	/* REP LODSW */
		icount -= 9;
		for(; (CF == flagval) && (count > 0); count--) {
			_lodsw();
		}
		regs.w[CX] = count;
		break;
	case 0xae:	/* REP(N)E SCASB */
		icount -= 9;
		for(ZeroVal = !flagval; (ZF == flagval) && (CF == flagval) && (count > 0); count--) {
			_scasb();
		}
		regs.w[CX] = count;
		break;
	case 0xaf:	/* REP(N)E SCASW */
		icount -= 9;
		for(ZeroVal = !flagval; (ZF == flagval) && (CF == flagval) && (count > 0); count--) {
			_scasw();
		}
		regs.w[CX] = count;
		break;
	default:
		instruction(next);
	}
#endif
}


void I86::_call_far()    /* Opcode 0x9a */
{
	unsigned tmp, tmp2;
	uint16_t ip;
	
	tmp = FETCH;
	tmp += FETCH << 8;
	
	tmp2 = FETCH;
	tmp2 += FETCH << 8;
	
	ip = pc - base[CS];
	PUSH(sregs[CS]);
	PUSH(ip);
	sregs[CS] = (uint16_t)tmp2;
	base[CS] = SegBase(CS);
	pc = (base[CS] + (uint16_t)tmp) & AMASK;
#ifdef I86_PSEUDO_BIOS
	if(d_bios && d_bios->bios_call_i86(pc, regs.w, sregs, &ZeroVal, &CarryVal)) {
		/* bios call */
		_retf();
	}
#endif
	icount -= timing.call_far;
}

void I86::_int()    /* Opcode 0xcd */
{
	unsigned int_num = FETCH;
	icount -= timing.int_imm;
#ifdef I86_PSEUDO_BIOS
	if(d_bios && d_bios->bios_int_i86(int_num, regs.w, sregs, &ZeroVal, &CarryVal)) {
		/* bios call */
		return;
	}
#endif
	interrupt(int_num);
}

void I86::_aad()    /* Opcode 0xd5 */
{
	unsigned mult = FETCH;
	icount -= timing.aad;
#ifdef HAS_V30
	regs.b[AL] = regs.b[AH] * 10 + regs.b[AL];
#else
	regs.b[AL] = regs.b[AH] * mult + regs.b[AL];
#endif
	regs.b[AH] = 0;
	SetZF(regs.b[AL]);
	SetPF(regs.b[AL]);
	SignVal = 0;
}


void I86::_call_d16()    /* Opcode 0xe8 */
{
	uint16_t ip, tmp;
	
	FETCHWORD(tmp);
	ip = pc - base[CS];
	PUSH(ip);
	ip += tmp;
	pc = (ip + base[CS]) & AMASK;
#ifdef I86_PSEUDO_BIOS
	if(d_bios && d_bios->bios_call_i86(pc, regs.w, sregs, &ZeroVal, &CarryVal)) {
		/* bios call */
		_ret();
	}
#endif
	icount -= timing.call_near;
}

void I86::_rep(int flagval)
{
	/* Handles rep- and repnz- prefixes. flagval is the value of ZF for the
	   loop  to continue for CMPS and SCAS instructions. */
	
	unsigned next = FETCHOP;
	unsigned count = regs.w[CX];
	
	switch(next) {
	case 0x26:  /* ES: */
		seg_prefix = true;
		prefix_seg = ES;
		icount -= timing.override;
		_rep(flagval);
		break;
	case 0x2e:  /* CS: */
		seg_prefix = true;
		prefix_seg = CS;
		icount -= timing.override;
		_rep(flagval);
		break;
	case 0x36:  /* SS: */
		seg_prefix = true;
		prefix_seg = SS;
		icount -= timing.override;
		_rep(flagval);
		break;
	case 0x3e:  /* DS: */
		seg_prefix = true;
		prefix_seg = DS;
		icount -= timing.override;
		_rep(flagval);
		break;
#ifndef HAS_I86
	case 0x6c:  /* REP INSB */
		icount -= timing.rep_ins8_base;
		for(; count > 0; count--) {
			PutMemB(ES, regs.w[DI], read_port_byte(regs.w[DX]));
			regs.w[DI] += DirVal;
			icount -= timing.rep_ins8_count;
		}
		regs.w[CX] = count;
		break;
	case 0x6d:  /* REP INSW */
		icount -= timing.rep_ins16_base;
		for(; count > 0; count--) {
			PutMemW(ES, regs.w[DI], read_port_word(regs.w[DX]));
			regs.w[DI] += 2 * DirVal;
			icount -= timing.rep_ins16_count;
		}
		regs.w[CX] = count;
		break;
	case 0x6e:  /* REP OUTSB */
		icount -= timing.rep_outs8_base;
		for(; count > 0; count--) {
			write_port_byte(regs.w[DX], GetMemB(DS, regs.w[SI]));
			regs.w[SI] += DirVal; /* GOL 11/27/01 */
			icount -= timing.rep_outs8_count;
		}
		regs.w[CX] = count;
		break;
	case 0x6f:  /* REP OUTSW */
		icount -= timing.rep_outs16_base;
		for(; count > 0; count--) {
			write_port_word(regs.w[DX], GetMemW(DS, regs.w[SI]));
			regs.w[SI] += 2 * DirVal; /* GOL 11/27/01 */
			icount -= timing.rep_outs16_count;
		}
		regs.w[CX] = count;
		break;
#endif
	case 0xa4:	/* REP MOVSB */
		icount -= timing.rep_movs8_base;
		for(; count > 0; count--) {
			uint8_t tmp;
			tmp = GetMemB(DS, regs.w[SI]);
			PutMemB(ES, regs.w[DI], tmp);
			regs.w[DI] += DirVal;
			regs.w[SI] += DirVal;
			icount -= timing.rep_movs8_count;
		}
		regs.w[CX] = count;
		break;
	case 0xa5:  /* REP MOVSW */
		icount -= timing.rep_movs16_base;
		for(; count > 0; count--) {
			uint16_t tmp;
			tmp = GetMemW(DS, regs.w[SI]);
			PutMemW(ES, regs.w[DI], tmp);
			regs.w[DI] += 2 * DirVal;
			regs.w[SI] += 2 * DirVal;
			icount -= timing.rep_movs16_count;
		}
		regs.w[CX] = count;
		break;
	case 0xa6:  /* REP(N)E CMPSB */
		icount -= timing.rep_cmps8_base;
		for(ZeroVal = !flagval; (ZF == flagval) && (count > 0); count--) {
			unsigned dst, src;
			dst = GetMemB(ES, regs.w[DI]);
			src = GetMemB(DS, regs.w[SI]);
			SUBB(src, dst); /* opposite of the usual convention */
			regs.w[DI] += DirVal;
			regs.w[SI] += DirVal;
			icount -= timing.rep_cmps8_count;
		}
		regs.w[CX] = count;
		break;
	case 0xa7:  /* REP(N)E CMPSW */
		icount -= timing.rep_cmps16_base;
		for(ZeroVal = !flagval; (ZF == flagval) && (count > 0); count--) {
			unsigned dst, src;
			dst = GetMemW(ES, regs.w[DI]);
			src = GetMemW(DS, regs.w[SI]);
			SUBW(src, dst); /* opposite of the usual convention */
			regs.w[DI] += 2 * DirVal;
			regs.w[SI] += 2 * DirVal;
			icount -= timing.rep_cmps16_count;
		}
		regs.w[CX] = count;
		break;
	case 0xaa:  /* REP STOSB */
		icount -= timing.rep_stos8_base;
		for(; count > 0; count--) {
			PutMemB(ES, regs.w[DI], regs.b[AL]);
			regs.w[DI] += DirVal;
			icount -= timing.rep_stos8_count;
		}
		regs.w[CX] = count;
		break;
	case 0xab:  /* REP STOSW */
		icount -= timing.rep_stos16_base;
		for(; count > 0; count--) {
			PutMemW(ES, regs.w[DI], regs.w[AX]);
			regs.w[DI] += 2 * DirVal;
			icount -= timing.rep_stos16_count;
		}
		regs.w[CX] = count;
		break;
	case 0xac:  /* REP LODSB */
		icount -= timing.rep_lods8_base;
		for(; count > 0; count--) {
			regs.b[AL] = GetMemB(DS, regs.w[SI]);
			regs.w[SI] += DirVal;
			icount -= timing.rep_lods8_count;
		}
		regs.w[CX] = count;
		break;
	case 0xad:  /* REP LODSW */
		icount -= timing.rep_lods16_base;
		for(; count > 0; count--) {
			regs.w[AX] = GetMemW(DS, regs.w[SI]);
			regs.w[SI] += 2 * DirVal;
			icount -= timing.rep_lods16_count;
		}
		regs.w[CX] = count;
		break;
	case 0xae:  /* REP(N)E SCASB */
		icount -= timing.rep_scas8_base;
		for(ZeroVal = !flagval; (ZF == flagval) && (count > 0); count--) {
			unsigned src, dst;
			src = GetMemB(ES, regs.w[DI]);
			dst = regs.b[AL];
			SUBB(dst, src);
			regs.w[DI] += DirVal;
			icount -= timing.rep_scas8_count;
		}
		regs.w[CX] = count;
		break;
	case 0xaf:  /* REP(N)E SCASW */
		icount -= timing.rep_scas16_base;
		for(ZeroVal = !flagval; (ZF == flagval) && (count > 0); count--) {
			unsigned src, dst;
			src = GetMemW(ES, regs.w[DI]);
			dst = regs.w[AX];
			SUBW(dst, src);
			regs.w[DI] += 2 * DirVal;
			icount -= timing.rep_scas16_count;
		}
		regs.w[CX] = count;
		break;
	default:
		instruction(next);
	}
}


void I86::_ffpre()    /* Opcode 0xff */
{
	unsigned ModRM = FETCHOP;
	unsigned tmp;
	unsigned tmp1;
	uint16_t ip;
	
	switch((ModRM >> 3) & 7) {
	case 0:  /* INC ew */
		icount -= (ModRM >= 0xc0) ? timing.incdec_r16 : timing.incdec_m16;
		tmp = GetRMWord(ModRM);
		tmp1 = tmp + 1;
		SetOFW_Add(tmp1, tmp, 1);
		SetAF(tmp1, tmp, 1);
		SetSZPF_Word(tmp1);
		PutbackRMWord(ModRM, (uint16_t)tmp1);
		break;
	case 1:  /* DEC ew */
		icount -= (ModRM >= 0xc0) ? timing.incdec_r16 : timing.incdec_m16;
		tmp = GetRMWord(ModRM);
		tmp1 = tmp - 1;
		SetOFW_Sub(tmp1, 1, tmp);
		SetAF(tmp1, tmp, 1);
		SetSZPF_Word(tmp1);
		PutbackRMWord(ModRM, (uint16_t)tmp1);
		break;
	case 2:  /* CALL ew */
		icount -= (ModRM >= 0xc0) ? timing.call_r16 : timing.call_m16;
		tmp = GetRMWord(ModRM);
		ip = pc - base[CS];
		PUSH(ip);
		pc = (base[CS] + (uint16_t)tmp) & AMASK;
#ifdef I86_PSEUDO_BIOS
		if(d_bios && d_bios->bios_call_i86(pc, regs.w, sregs, &ZeroVal, &CarryVal)) {
			/* bios call */
			_ret();
		}
#endif
		break;
	case 3:  /* CALL FAR ea */
		icount -= timing.call_m32;
		tmp = sregs[CS];	/* need to skip displacements of ea */
		tmp1 = GetRMWord(ModRM);
		ip = pc - base[CS];
		PUSH(tmp);
		PUSH(ip);
		sregs[CS] = GetNextRMWord;
		base[CS] = SegBase(CS);
		pc = (base[CS] + tmp1) & AMASK;
#ifdef I86_PSEUDO_BIOS
		if(d_bios && d_bios->bios_call_i86(pc, regs.w, sregs, &ZeroVal, &CarryVal)) {
			/* bios call */
			_ret();
		}
#endif
		break;
	case 4:  /* JMP ea */
		icount -= (ModRM >= 0xc0) ? timing.jmp_r16 : timing.jmp_m16;
		ip = GetRMWord(ModRM);
		pc = (base[CS] + ip) & AMASK;
		break;
	case 5:  /* JMP FAR ea */
		icount -= timing.jmp_m32;
		pc = GetRMWord(ModRM);
		sregs[CS] = GetNextRMWord;
		base[CS] = SegBase(CS);
		pc = (pc + base[CS]) & AMASK;
		break;
	case 6:  /* PUSH ea */
		icount -= (ModRM >= 0xc0) ? timing.push_r16 : timing.push_m16;
		tmp = GetRMWord(ModRM);
		PUSH(tmp);
		break;
	case 7:  /* invalid ??? */
		icount -= 10;
		break;
#if defined(_MSC_VER) && (_MSC_VER >= 1200)
	default:
		__assume(0);
#endif
	}
}


