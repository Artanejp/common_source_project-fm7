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

#include "./i86_base.h"
#include "./i86_macros.h"
int necv_dasm_one(_TCHAR *buffer, UINT32 eip, const UINT8 *oprom);

void I86_BASE::initialize()
{
	static const BREGS reg_name[8] = {AL, CL, DL, BL, AH, CH, DH, BH};
	
	for(int i = 0; i < 256; i++) {
		Mod_RM.reg.b[i] = reg_name[(i & 0x38) >> 3];
		Mod_RM.reg.w[i] = (WREGS)((i & 0x38) >> 3);
	}
	for(int i = 0xc0; i < 0x100; i++) {
		Mod_RM.RM.w[i] = (WREGS)(i & 7);
		Mod_RM.RM.b[i] = (BREGS)reg_name[i & 7];
	}
	//memcpy(parity_table, _i80x86_parity_table, sizeof(uint8_t) * 256);
	timing = _i80x86_timing_tbl_i8086;

	d_mem_stored = d_mem;
	d_io_stored = d_io;
}

void I86_BASE::reset()
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
	seg_prefix = false;
}

int I86_BASE::run(int clock)
{
	/* return now if BUSREQ */
	if(busreq) {
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
		run_one_opecode();
		return -icount;
	} else {
		icount += clock;
		int first_icount = icount;
		
		/* adjust for any interrupts that came in */
		icount -= extra_icount;
		extra_icount = 0;
		
		/* run cpu while given clocks */
		while(icount > 0 && !busreq) {
			run_one_opecode();
		}
		/* if busreq is raised, spin cpu while remained clock */
		if(icount > 0 && busreq) {
			icount = 0;
		}
		return first_icount - icount;
	}
}


void I86_BASE::run_one_opecode_debugger()
{
}


void I86_BASE::run_one_opecode()
{
	seg_prefix = false;
	instruction(FETCHOP);
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

void I86_BASE::write_signal(int id, uint32_t data, uint32_t mask)
{
	if(id == SIG_CPU_NMI) {
		if(data & mask) {
			int_state |= NMI_REQ_BIT;
		} else {
			int_state &= ~NMI_REQ_BIT;
		}
	} else if(id == SIG_CPU_BUSREQ) {
		busreq = ((data & mask) != 0);
	} else if(id == SIG_I86_TEST) {
		test_state = ((data & mask) != 0);
	}
}

void I86_BASE::set_intr_line(bool line, bool pending, uint32_t bit)
{
	if(line) {
		int_state |= INT_REQ_BIT;
	} else {
		int_state &= ~INT_REQ_BIT;
	}
}

//#ifdef USE_DEBUGGER
void I86_BASE::write_debug_data8(uint32_t addr, uint32_t data)
{
	int wait;
	d_mem_stored->write_data8w(addr, data, &wait);
}

uint32_t I86_BASE::read_debug_data8(uint32_t addr)
{
	int wait;
	return d_mem_stored->read_data8w(addr, &wait);
}

void I86_BASE::write_debug_data16(uint32_t addr, uint32_t data)
{
	int wait;
	d_mem_stored->write_data16w(addr, data, &wait);
}

uint32_t I86_BASE::read_debug_data16(uint32_t addr)
{
	int wait;
	return d_mem_stored->read_data16w(addr, &wait);
}

void I86_BASE::write_debug_io8(uint32_t addr, uint32_t data)
{
	int wait;
	d_io_stored->write_io8w(addr, data, &wait);
}

uint32_t I86_BASE::read_debug_io8(uint32_t addr) {
	int wait;
	return d_io_stored->read_io8w(addr, &wait);
}

void I86_BASE::write_debug_io16(uint32_t addr, uint32_t data)
{
	int wait;
	d_io_stored->write_io16w(addr, data, &wait);
}

uint32_t I86_BASE::read_debug_io16(uint32_t addr) {
	int wait;
	return d_io_stored->read_io16w(addr, &wait);
}

bool I86_BASE::write_debug_reg(const _TCHAR *reg, uint32_t data)
{
	if(_tcsicmp(reg, _T("IP")) == 0) {
		pc = ((data & 0xffff) + base[CS]) & AMASK;
	} else if(_tcsicmp(reg, _T("AX")) == 0) {
		regs.w[AX] = data;
	} else if(_tcsicmp(reg, _T("BX")) == 0) {
		regs.w[BX] = data;
	} else if(_tcsicmp(reg, _T("CX")) == 0) {
		regs.w[CX] = data;
	} else if(_tcsicmp(reg, _T("DX")) == 0) {
		regs.w[DX] = data;
	} else if(_tcsicmp(reg, _T("SP")) == 0) {
		regs.w[SP] = data;
	} else if(_tcsicmp(reg, _T("BP")) == 0) {
		regs.w[BP] = data;
	} else if(_tcsicmp(reg, _T("SI")) == 0) {
		regs.w[SI] = data;
	} else if(_tcsicmp(reg, _T("DI")) == 0) {
		regs.w[DI] = data;
	} else if(_tcsicmp(reg, _T("AL")) == 0) {
		regs.b[AL] = data;
	} else if(_tcsicmp(reg, _T("AH")) == 0) {
		regs.b[AH] = data;
	} else if(_tcsicmp(reg, _T("BL")) == 0) {
		regs.b[BL] = data;
	} else if(_tcsicmp(reg, _T("BH")) == 0) {
		regs.b[BH] = data;
	} else if(_tcsicmp(reg, _T("CL")) == 0) {
		regs.b[CL] = data;
	} else if(_tcsicmp(reg, _T("CH")) == 0) {
		regs.b[CH] = data;
	} else if(_tcsicmp(reg, _T("DL")) == 0) {
		regs.b[DL] = data;
	} else if(_tcsicmp(reg, _T("DH")) == 0) {
		regs.b[DH] = data;
	} else {
		return false;
	}
	return true;
}

void I86_BASE::get_debug_regs_info(_TCHAR *buffer, size_t buffer_len)
{
	my_stprintf_s(buffer, buffer_len,
	_T("AX=%04X  BX=%04X CX=%04X DX=%04X SP=%04X  BP=%04X  SI=%04X  DI=%04X\nDS=%04X  ES=%04X SS=%04X CS=%04X IP=%04X  FLAG=[%c%c%c%c%c%c%c%c%c]"),
	regs.w[AX], regs.w[BX], regs.w[CX], regs.w[DX], regs.w[SP], regs.w[BP], regs.w[SI], regs.w[DI], sregs[DS], sregs[ES], sregs[SS], sregs[CS], (uint16_t)(pc - base[CS]),
	OF ? _T('O') : _T('-'), DF ? _T('D') : _T('-'), IF ? _T('I') : _T('-'), TF ? _T('T') : _T('-'),
	SF ? _T('S') : _T('-'), ZF ? _T('Z') : _T('-'), AF ? _T('A') : _T('-'), PF ? _T('P') : _T('-'), CF ? _T('C') : _T('-'));
}

int I86_BASE::debug_dasm(uint32_t pc, _TCHAR *buffer, size_t buffer_len)
{
	UINT32 eip = (UINT32)(uint16_t)(pc - base[CS]);
	UINT8 ops[16];
	for(int i = 0; i < 16; i++) {
		int wait;
		ops[i] = d_mem->read_data8w(pc + i, &wait);
	}
	return necv_dasm_one(buffer, eip, ops) & DASMFLAG_LENGTHMASK;
}
//#endif

void I86_BASE::interrupt(int int_num)
{
	unsigned dest_seg, dest_off;
	uint16_t ip = pc - base[CS];
	
	if(int_num == -1) {
		int_num = d_pic->get_intr_ack() & 0xff;
		int_state &= ~INT_REQ_BIT;
	}
	dest_off = ReadWord(int_num * 4);
	dest_seg = ReadWord(int_num * 4 + 2);
	
	_pushf();
	TF = IF = 0;
	PUSH(sregs[CS]);
	PUSH(ip);
	sregs[CS] = (uint16_t)dest_seg;
	base[CS] = SegBase(CS);
	pc = (base[CS] + dest_off) & AMASK;
	icount -= timing.exception;
}

void I86_BASE::trap()
{
	instruction(FETCHOP);
	interrupt(1);
}

unsigned I86_BASE::GetEA(unsigned ModRM)
{
	switch(ModRM) {
	case 0x00: case 0x08: case 0x10: case 0x18: case 0x20: case 0x28: case 0x30: case 0x38:
		icount -= 7; eo = (uint16_t)(regs.w[BX] + regs.w[SI]); ea_seg = DefaultSeg(DS); ea = DefaultBase(DS) + eo; return ea;
	case 0x01: case 0x09: case 0x11: case 0x19: case 0x21: case 0x29: case 0x31: case 0x39:
		icount -= 8; eo = (uint16_t)(regs.w[BX] + regs.w[DI]); ea_seg = DefaultSeg(DS); ea = DefaultBase(DS) + eo; return ea;
	case 0x02: case 0x0a: case 0x12: case 0x1a: case 0x22: case 0x2a: case 0x32: case 0x3a:
		icount -= 8; eo = (uint16_t)(regs.w[BP] + regs.w[SI]); ea_seg = DefaultSeg(SS); ea = DefaultBase(SS) + eo; return ea;
	case 0x03: case 0x0b: case 0x13: case 0x1b: case 0x23: case 0x2b: case 0x33: case 0x3b:
		icount -= 7; eo = (uint16_t)(regs.w[BP] + regs.w[DI]); ea_seg = DefaultSeg(SS); ea = DefaultBase(SS) + eo; return ea;
	case 0x04: case 0x0c: case 0x14: case 0x1c: case 0x24: case 0x2c: case 0x34: case 0x3c:
		icount -= 5; eo = regs.w[SI]; ea_seg = DefaultSeg(DS); ea = DefaultBase(DS) + eo; return ea;
	case 0x05: case 0x0d: case 0x15: case 0x1d: case 0x25: case 0x2d: case 0x35: case 0x3d:
		icount -= 5; eo = regs.w[DI]; ea_seg = DefaultSeg(DS); ea = DefaultBase(DS) + eo; return ea;
	case 0x06: case 0x0e: case 0x16: case 0x1e: case 0x26: case 0x2e: case 0x36: case 0x3e:
		icount -= 6; eo = FETCHOP; eo += FETCHOP << 8; ea_seg = DefaultSeg(DS); ea = DefaultBase(DS) + eo; return ea;
	case 0x07: case 0x0f: case 0x17: case 0x1f: case 0x27: case 0x2f: case 0x37: case 0x3f:
		icount -= 5; eo = regs.w[BX]; ea_seg = DefaultSeg(DS); ea = DefaultBase(DS) + eo; return ea;

	case 0x40: case 0x48: case 0x50: case 0x58: case 0x60: case 0x68: case 0x70: case 0x78:
		icount -= 11; eo = (uint16_t)(regs.w[BX] + regs.w[SI] + (int8_t)FETCHOP); ea_seg = DefaultSeg(DS); ea = DefaultBase(DS) + eo; return ea;
	case 0x41: case 0x49: case 0x51: case 0x59: case 0x61: case 0x69: case 0x71: case 0x79:
		icount -= 12; eo = (uint16_t)(regs.w[BX] + regs.w[DI] + (int8_t)FETCHOP); ea_seg = DefaultSeg(DS); ea = DefaultBase(DS) + eo; return ea;
	case 0x42: case 0x4a: case 0x52: case 0x5a: case 0x62: case 0x6a: case 0x72: case 0x7a:
		icount -= 12; eo = (uint16_t)(regs.w[BP] + regs.w[SI] + (int8_t)FETCHOP); ea_seg = DefaultSeg(SS); ea = DefaultBase(SS) + eo; return ea;
	case 0x43: case 0x4b: case 0x53: case 0x5b: case 0x63: case 0x6b: case 0x73: case 0x7b:
		icount -= 11; eo = (uint16_t)(regs.w[BP] + regs.w[DI] + (int8_t)FETCHOP); ea_seg = DefaultSeg(SS); ea = DefaultBase(SS) + eo; return ea;
	case 0x44: case 0x4c: case 0x54: case 0x5c: case 0x64: case 0x6c: case 0x74: case 0x7c:
		icount -= 9; eo = (uint16_t)(regs.w[SI] + (int8_t)FETCHOP); ea_seg = DefaultSeg(DS); ea = DefaultBase(DS) + eo; return ea;
	case 0x45: case 0x4d: case 0x55: case 0x5d: case 0x65: case 0x6d: case 0x75: case 0x7d:
		icount -= 9; eo = (uint16_t)(regs.w[DI] + (int8_t)FETCHOP); ea_seg = DefaultSeg(DS); ea = DefaultBase(DS) + eo; return ea;
	case 0x46: case 0x4e: case 0x56: case 0x5e: case 0x66: case 0x6e: case 0x76: case 0x7e:
		icount -= 9; eo = (uint16_t)(regs.w[BP] + (int8_t)FETCHOP); ea_seg = DefaultSeg(SS); ea = DefaultBase(SS) + eo; return ea;
	case 0x47: case 0x4f: case 0x57: case 0x5f: case 0x67: case 0x6f: case 0x77: case 0x7f:
		icount -= 9; eo = (uint16_t)(regs.w[BX] + (int8_t)FETCHOP); ea_seg = DefaultSeg(DS); ea = DefaultBase(DS) + eo; return ea;

	case 0x80: case 0x88: case 0x90: case 0x98: case 0xa0: case 0xa8: case 0xb0: case 0xb8:
		icount -= 11; eo = FETCHOP; eo += FETCHOP << 8; eo += regs.w[BX] + regs.w[SI]; ea_seg = DefaultSeg(DS); ea = DefaultBase(DS) + (uint16_t)eo; return ea;
	case 0x81: case 0x89: case 0x91: case 0x99: case 0xa1: case 0xa9: case 0xb1: case 0xb9:
		icount -= 12; eo = FETCHOP; eo += FETCHOP << 8; eo += regs.w[BX] + regs.w[DI]; ea_seg = DefaultSeg(DS); ea = DefaultBase(DS) + (uint16_t)eo; return ea;
	case 0x82: case 0x8a: case 0x92: case 0x9a: case 0xa2: case 0xaa: case 0xb2: case 0xba:
		icount -= 12; eo = FETCHOP; eo += FETCHOP << 8; eo += regs.w[BP] + regs.w[SI]; ea_seg = DefaultSeg(SS); ea = DefaultBase(SS) + (uint16_t)eo; return ea;
	case 0x83: case 0x8b: case 0x93: case 0x9b: case 0xa3: case 0xab: case 0xb3: case 0xbb:
		icount -= 11; eo = FETCHOP; eo += FETCHOP << 8; eo += regs.w[BP] + regs.w[DI]; ea_seg = DefaultSeg(DS); ea = DefaultBase(SS) + (uint16_t)eo; return ea;
	case 0x84: case 0x8c: case 0x94: case 0x9c: case 0xa4: case 0xac: case 0xb4: case 0xbc:
		icount -= 9; eo = FETCHOP; eo += FETCHOP << 8; eo += regs.w[SI]; ea_seg = DefaultSeg(DS); ea = DefaultBase(DS) + (uint16_t)eo; return ea;
	case 0x85: case 0x8d: case 0x95: case 0x9d: case 0xa5: case 0xad: case 0xb5: case 0xbd:
		icount -= 9; eo = FETCHOP; eo += FETCHOP << 8; eo += regs.w[DI]; ea_seg = DefaultSeg(DS); ea = DefaultBase(DS) + (uint16_t)eo; return ea;
	case 0x86: case 0x8e: case 0x96: case 0x9e: case 0xa6: case 0xae: case 0xb6: case 0xbe:
		icount -= 9; eo = FETCHOP; eo += FETCHOP << 8; eo += regs.w[BP]; ea_seg = DefaultSeg(SS); ea = DefaultBase(SS) + (uint16_t)eo; return ea;
	case 0x87: case 0x8f: case 0x97: case 0x9f: case 0xa7: case 0xaf: case 0xb7: case 0xbf:
		icount -= 9; eo = FETCHOP; eo += FETCHOP << 8; eo += regs.w[BX]; ea_seg = DefaultSeg(DS); ea = DefaultBase(DS) + (uint16_t)eo; return ea;
	}
	return 0;
}

void I86_BASE::rotate_shift_byte(unsigned ModRM, unsigned count)
{
	unsigned src = (unsigned)GetRMByte(ModRM);
	unsigned dst = src;
	
	if(count == 0) {
		icount -= (ModRM >= 0xc0) ? timing.rot_reg_base : timing.rot_m8_base;
	} else if(count == 1) {
		icount -= (ModRM >= 0xc0) ? timing.rot_reg_1 : timing.rot_m8_1;
		
		switch((ModRM >> 3) & 7) {
		case 0:	/* ROL eb, 1 */
			CarryVal = src & 0x80;
			dst = (src << 1) + CF;
			PutbackRMByte(ModRM, dst);
			OverVal = (src ^ dst) & 0x80;
			break;
		case 1:	/* ROR eb, 1 */
			CarryVal = src & 0x01;
			dst = ((CF << 8) + src) >> 1;
			PutbackRMByte(ModRM, dst);
			OverVal = (src ^ dst) & 0x80;
			break;
		case 2:	/* RCL eb, 1 */
			dst = (src << 1) + CF;
			PutbackRMByte(ModRM, dst);
			SetCFB(dst);
			OverVal = (src ^ dst) & 0x80;
			break;
		case 3:	/* RCR eb, 1 */
			dst = ((CF << 8) + src) >> 1;
			PutbackRMByte(ModRM, dst);
			CarryVal = src & 0x01;
			OverVal = (src ^ dst) & 0x80;
			break;
		case 4:	/* SHL eb, 1 */
		case 6:
			dst = src << 1;
			PutbackRMByte(ModRM, dst);
			SetCFB(dst);
			OverVal = (src ^ dst) & 0x80;
			AuxVal = 1;
			SetSZPF_Byte(dst);
			break;
		case 5:	/* SHR eb, 1 */
			dst = src >> 1;
			PutbackRMByte(ModRM, dst);
			CarryVal = src & 0x01;
			OverVal = src & 0x80;
			AuxVal = 1;
			SetSZPF_Byte(dst);
			break;
		case 7:	/* SAR eb, 1 */
			dst = ((int8_t)src) >> 1;
			PutbackRMByte(ModRM, dst);
			CarryVal = src & 0x01;
			OverVal = 0;
			AuxVal = 1;
			SetSZPF_Byte(dst);
			break;
#if defined(_MSC_VER) && (_MSC_VER >= 1200)
		default:
			__assume(0);
#endif
		}
	} else {
		icount -= (ModRM >= 0xc0) ? timing.rot_reg_base + timing.rot_reg_bit : timing.rot_m8_base + timing.rot_m8_bit;
		
		switch((ModRM >> 3) & 7) {
		case 0:	/* ROL eb, count */
			for(; count > 0; count--) {
				CarryVal = dst & 0x80;
				dst = (dst << 1) + CF;
			}
			PutbackRMByte(ModRM, (uint8_t)dst);
			break;
		case 1:	/* ROR eb, count */
			for(; count > 0; count--) {
				CarryVal = dst & 0x01;
				dst = (dst >> 1) + (CF << 7);
			}
			PutbackRMByte(ModRM, (uint8_t)dst);
			break;
		case 2:	/* RCL eb, count */
			for(; count > 0; count--) {
				dst = (dst << 1) + CF;
				SetCFB(dst);
			}
			PutbackRMByte(ModRM, (uint8_t)dst);
			break;
		case 3:	/* RCR eb, count */
			for(; count > 0; count--) {
				dst = (CF << 8) + dst;
				CarryVal = dst & 0x01;
				dst >>= 1;
			}
			PutbackRMByte(ModRM, (uint8_t)dst);
			break;
		case 4:	/* SHL eb, count */
		case 6:
			dst <<= count;
			SetCFB(dst);
			AuxVal = 1;
			SetSZPF_Byte(dst);
			PutbackRMByte(ModRM, (uint8_t)dst);
			break;
		case 5:	/* SHR eb, count */
			dst >>= count - 1;
			CarryVal = dst & 0x01;
			dst >>= 1;
			SetSZPF_Byte(dst);
			AuxVal = 1;
			PutbackRMByte(ModRM, (uint8_t)dst);
			break;
		case 7:	/* SAR eb, count */
			dst = ((int8_t)dst) >> (count - 1);
			CarryVal = dst & 0x01;
			dst = ((int8_t)((uint8_t)dst)) >> 1;
			SetSZPF_Byte(dst);
			AuxVal = 1;
			PutbackRMByte(ModRM, (uint8_t)dst);
			break;
#if defined(_MSC_VER) && (_MSC_VER >= 1200)
		default:
			__assume(0);
#endif
		}
	}
}

void I86_BASE::rotate_shift_word(unsigned ModRM, unsigned count)
{
	unsigned src = GetRMWord(ModRM);
	unsigned dst = src;
	
	if(count == 0) {
		icount -= (ModRM >= 0xc0) ? timing.rot_reg_base : timing.rot_m16_base;
	} else if(count == 1) {
		icount -= (ModRM >= 0xc0) ? timing.rot_reg_1 : timing.rot_m16_1;
		
		switch((ModRM >> 3) & 7) {
		case 0:	/* ROL ew, 1 */
			CarryVal = src & 0x8000;
			dst = (src << 1) + CF;
			PutbackRMWord(ModRM, dst);
			OverVal = (src ^ dst) & 0x8000;
			break;
		case 1:	/* ROR ew, 1 */
			CarryVal = src & 0x01;
			dst = ((CF << 16) + src) >> 1;
			PutbackRMWord(ModRM, dst);
			OverVal = (src ^ dst) & 0x8000;
			break;
		case 2:	/* RCL ew, 1 */
			dst = (src << 1) + CF;
			PutbackRMWord(ModRM, dst);
			SetCFW(dst);
			OverVal = (src ^ dst) & 0x8000;
			break;
		case 3:	/* RCR ew, 1 */
			dst = ((CF << 16) + src) >> 1;
			PutbackRMWord(ModRM, dst);
			CarryVal = src & 0x01;
			OverVal = (src ^ dst) & 0x8000;
			break;
		case 4:	/* SHL ew, 1 */
		case 6:
			dst = src << 1;
			PutbackRMWord(ModRM, dst);
			SetCFW(dst);
			OverVal = (src ^ dst) & 0x8000;
			AuxVal = 1;
			SetSZPF_Word(dst);
			break;
		case 5:	/* SHR ew, 1 */
			dst = src >> 1;
			PutbackRMWord(ModRM, dst);
			CarryVal = src & 0x01;
			OverVal = src & 0x8000;
			AuxVal = 1;
			SetSZPF_Word(dst);
			break;
		case 7:	/* SAR ew, 1 */
			dst = ((int16_t)src) >> 1;
			PutbackRMWord(ModRM, dst);
			CarryVal = src & 0x01;
			OverVal = 0;
			AuxVal = 1;
			SetSZPF_Word(dst);
			break;
#if defined(_MSC_VER) && (_MSC_VER >= 1200)
		default:
			__assume(0);
#endif
		}
	} else {
		icount -= (ModRM >= 0xc0) ? timing.rot_reg_base + timing.rot_reg_bit : timing.rot_m8_base + timing.rot_m16_bit;
		
		switch((ModRM >> 3) & 7) {
		case 0:	/* ROL ew, count */
			for(; count > 0; count--) {
				CarryVal = dst & 0x8000;
				dst = (dst << 1) + CF;
			}
			PutbackRMWord(ModRM, dst);
			break;
		case 1:	/* ROR ew, count */
			for(; count > 0; count--) {
				CarryVal = dst & 0x01;
				dst = (dst >> 1) + (CF << 15);
			}
			PutbackRMWord(ModRM, dst);
			break;
		case 2:	/* RCL ew, count */
			for(; count > 0; count--) {
				dst = (dst << 1) + CF;
				SetCFW(dst);
			}
			PutbackRMWord(ModRM, dst);
			break;
		case 3:	/* RCR ew, count */
			for(; count > 0; count--) {
				dst = dst + (CF << 16);
				CarryVal = dst & 0x01;
				dst >>= 1;
			}
			PutbackRMWord(ModRM, dst);
			break;
		case 4:	/* SHL ew, count */
		case 6:
			dst <<= count;
			SetCFW(dst);
			AuxVal = 1;
			SetSZPF_Word(dst);
			PutbackRMWord(ModRM, dst);
			break;
		case 5:	/* SHR ew, count */
			dst >>= count - 1;
			CarryVal = dst & 0x01;
			dst >>= 1;
			SetSZPF_Word(dst);
			AuxVal = 1;
			PutbackRMWord(ModRM, dst);
			break;
		case 7:	/* SAR ew, count */
			dst = ((int16_t)dst) >> (count - 1);
			CarryVal = dst & 0x01;
			dst = ((int16_t)((uint16_t)dst)) >> 1;
			SetSZPF_Word(dst);
			AuxVal = 1;
			PutbackRMWord(ModRM, dst);
			break;
#if defined(_MSC_VER) && (_MSC_VER >= 1200)
		default:
			__assume(0);
#endif
		}
	}
}

void I86_BASE::instruction(uint8_t code)
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
	case 0x0f: _invalid(); break;
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
	case 0x60: _invalid(); break;
	case 0x61: _invalid(); break;
	case 0x62: _invalid(); break;

	case 0x63: _invalid(); break;
	case 0x64: _invalid(); break;
	case 0x65: _invalid(); break;
	case 0x66: _invalid(); break;
	case 0x67: _invalid(); break;
	case 0x68: _invalid(); break;
	case 0x69: _invalid(); break;
	case 0x6a: _invalid(); break;
	case 0x6b: _invalid(); break;
	case 0x6c: _invalid(); break;
	case 0x6d: _invalid(); break;
	case 0x6e: _invalid(); break;
	case 0x6f: _invalid(); break;
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
	case 0xc0: _invalid(); break;
	case 0xc1: _invalid(); break;
	case 0xc2: _ret_d16(); break;
	case 0xc3: _ret(); break;
	case 0xc4: _les_dw(); break;
	case 0xc5: _lds_dw(); break;
	case 0xc6: _mov_bd8(); break;
	case 0xc7: _mov_wd16(); break;
	case 0xc8: _invalid(); break;
	case 0xc9: _invalid(); break;
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
	case 0xd6: _invalid(); break;
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

void I86_BASE::_repc(int flagval)
{
}

//!!
void I86_BASE::_call_far()    /* Opcode 0x9a */
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
	icount -= timing.call_far;
}

void I86_BASE::_int()    /* Opcode 0xcd */
{
	unsigned int_num = FETCH;
	icount -= timing.int_imm;
	interrupt(int_num);
}
void I86_BASE::_aad()    /* Opcode 0xd5 */
{
	unsigned mult = FETCH;
	icount -= timing.aad;
	regs.b[AL] = regs.b[AH] * mult + regs.b[AL];
	regs.b[AH] = 0;
	SetZF(regs.b[AL]);
	SetPF(regs.b[AL]);
	SignVal = 0;
}
void I86_BASE::_call_d16()    /* Opcode 0xe8 */
{
	uint16_t ip, tmp;
	
	FETCHWORD(tmp);
	ip = pc - base[CS];
	PUSH(ip);
	ip += tmp;
	pc = (ip + base[CS]) & AMASK;
	icount -= timing.call_near;
}

void I86_BASE::_rep(int flagval)
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
void I86_BASE::_ffpre()    /* Opcode 0xff */
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


/*
   NEC V-series Disassembler

   Originally Written for i386 by Ville Linde
   Converted to NEC-V by Aaron Giles
*/

enum
{
	PARAM_REG8 = 1,		/* 8-bit register */
	PARAM_REG16,		/* 16-bit register */
	PARAM_REG2_8,		/* 8-bit register */
	PARAM_REG2_16,		/* 16-bit register */
	PARAM_RM8,			/* 8-bit memory or register */
	PARAM_RM16,			/* 16-bit memory or register */
	PARAM_RMPTR8,		/* 8-bit memory or register */
	PARAM_RMPTR16,		/* 16-bit memory or register */
	PARAM_I3,			/* 3-bit immediate */
	PARAM_I4,			/* 4-bit immediate */
	PARAM_I8,			/* 8-bit signed immediate */
	PARAM_I16,			/* 16-bit signed immediate */
	PARAM_UI8,			/* 8-bit unsigned immediate */
	PARAM_IMM,			/* 16-bit immediate */
	PARAM_ADDR,			/* 16:16 address */
	PARAM_REL8,			/* 8-bit PC-relative displacement */
	PARAM_REL16,		/* 16-bit PC-relative displacement */
	PARAM_MEM_OFFS,		/* 16-bit mem offset */
	PARAM_SREG,			/* segment register */
	PARAM_SFREG,		/* V25/V35 special function register */
	PARAM_1,			/* used by shift/rotate instructions */
	PARAM_AL,
	PARAM_CL,
	PARAM_DL,
	PARAM_BL,
	PARAM_AH,
	PARAM_CH,
	PARAM_DH,
	PARAM_BH,
	PARAM_AW,
	PARAM_CW,
	PARAM_DW,
	PARAM_BW,
	PARAM_SP,
	PARAM_BP,
	PARAM_IX,
	PARAM_IY
};

enum
{
	MODRM = 1,
	GROUP,
	FPU,
	TWO_BYTE,
	PREFIX,
	SEG_PS,
	SEG_DS0,
	SEG_DS1,
	SEG_SS
};

struct I386_OPCODE {
	_TCHAR mnemonic[32];
	UINT32 flags;
	UINT32 param1;
	UINT32 param2;
	UINT32 param3;
	offs_t dasm_flags;
};

struct GROUP_OP {
	_TCHAR mnemonic[32];
	const I386_OPCODE *opcode;
};

static const UINT8 *opcode_ptr;
static const UINT8 *opcode_ptr_base;

static const I386_OPCODE necv_opcode_table1[256] =
{
	// 0x00
	{_T("add"),				MODRM,			PARAM_RM8,			PARAM_REG8,			0				},
	{_T("add"),				MODRM,			PARAM_RM16,			PARAM_REG16,		0				},
	{_T("add"),				MODRM,			PARAM_REG8,			PARAM_RM8,			0				},
	{_T("add"),				MODRM,			PARAM_REG16,		PARAM_RM16,			0				},
	{_T("add"),				0,				PARAM_AL,			PARAM_UI8,			0				},
	{_T("add"),				0,				PARAM_AW,			PARAM_IMM,			0				},
	{_T("push    ds1"),		0,				0,					0,					0				},
	{_T("pop     ds1"),		0,				0,					0,					0				},
	{_T("or"),				MODRM,			PARAM_RM8,			PARAM_REG8,			0				},
	{_T("or"),				MODRM,			PARAM_RM16,			PARAM_REG16,		0				},
	{_T("or"),				MODRM,			PARAM_REG8,			PARAM_RM8,			0				},
	{_T("or"),				MODRM,			PARAM_REG16,		PARAM_RM16,			0				},
	{_T("or"),				0,				PARAM_AL,			PARAM_UI8,			0				},
	{_T("or"),				0,				PARAM_AW,			PARAM_IMM,			0				},
	{_T("push    ps"),		0,				0,					0,					0				},
	{_T("two_byte"),		TWO_BYTE,		0,					0,					0				},
	// 0x10
	{_T("addc"),			MODRM,			PARAM_RM8,			PARAM_REG8,			0				},
	{_T("addc"),			MODRM,			PARAM_RM16,			PARAM_REG16,		0				},
	{_T("addc"),			MODRM,			PARAM_REG8,			PARAM_RM8,			0				},
	{_T("addc"),			MODRM,			PARAM_REG16,		PARAM_RM16,			0				},
	{_T("addc"),			0,				PARAM_AL,			PARAM_UI8,			0				},
	{_T("addc"),			0,				PARAM_AW,			PARAM_IMM,			0				},
	{_T("push    ss"),		0,				0,					0,					0				},
	{_T("pop     ss"),		0,				0,					0,					0				},
	{_T("subc"),			MODRM,			PARAM_RM8,			PARAM_REG8,			0				},
	{_T("subc"),			MODRM,			PARAM_RM16,			PARAM_REG16,		0				},
	{_T("subc"),			MODRM,			PARAM_REG8,			PARAM_RM8,			0				},
	{_T("subc"),			MODRM,			PARAM_REG16,		PARAM_RM16,			0				},
	{_T("subc"),			0,				PARAM_AL,			PARAM_UI8,			0				},
	{_T("subc"),			0,				PARAM_AW,			PARAM_IMM,			0				},
	{_T("push    ds0"),		0,				0,					0,					0				},
	{_T("pop     ds0"),		0,				0,					0,					0				},
	// 0x20
	{_T("and"),				MODRM,			PARAM_RM8,			PARAM_REG8,			0				},
	{_T("and"),				MODRM,			PARAM_RM16,			PARAM_REG16,		0				},
	{_T("and"),				MODRM,			PARAM_REG8,			PARAM_RM8,			0				},
	{_T("and"),				MODRM,			PARAM_REG16,		PARAM_RM16,			0				},
	{_T("and"),				0,				PARAM_AL,			PARAM_UI8,			0				},
	{_T("and"),				0,				PARAM_AW,			PARAM_IMM,			0				},
	{_T("ds1:"),			SEG_DS1,		0,					0,					0				},
	{_T("adj4a"),			0,				0,					0,					0				},
	{_T("sub"),				MODRM,			PARAM_RM8,			PARAM_REG8,			0				},
	{_T("sub"),				MODRM,			PARAM_RM16,			PARAM_REG16,		0				},
	{_T("sub"),				MODRM,			PARAM_REG8,			PARAM_RM8,			0				},
	{_T("sub"),				MODRM,			PARAM_REG16,		PARAM_RM16,			0				},
	{_T("sub"),				0,				PARAM_AL,			PARAM_UI8,			0				},
	{_T("sub"),				0,				PARAM_AW,			PARAM_IMM,			0				},
	{_T("ps:"),				SEG_PS,			0,					0,					0				},
	{_T("adj4s"),			0,				0,					0,					0				},
	// 0x30
	{_T("xor"),				MODRM,			PARAM_RM8,			PARAM_REG8,			0				},
	{_T("xor"),				MODRM,			PARAM_RM16,			PARAM_REG16,		0				},
	{_T("xor"),				MODRM,			PARAM_REG8,			PARAM_RM8,			0				},
	{_T("xor"),				MODRM,			PARAM_REG16,		PARAM_RM16,			0				},
	{_T("xor"),				0,				PARAM_AL,			PARAM_UI8,			0				},
	{_T("xor"),				0,				PARAM_AW,			PARAM_IMM,			0				},
	{_T("ss:"),				SEG_SS,			0,					0,					0				},
	{_T("adjba"),			0,				0,					0,					0				},
	{_T("cmp"),				MODRM,			PARAM_RM8,			PARAM_REG8,			0				},
	{_T("cmp"),				MODRM,			PARAM_RM16,			PARAM_REG16,		0				},
	{_T("cmp"),				MODRM,			PARAM_REG8,			PARAM_RM8,			0				},
	{_T("cmp"),				MODRM,			PARAM_REG16,		PARAM_RM16,			0				},
	{_T("cmp"),				0,				PARAM_AL,			PARAM_UI8,			0				},
	{_T("cmp"),				0,				PARAM_AW,			PARAM_IMM,			0				},
	{_T("ds0:"),			SEG_DS0,		0,					0,					0				},
	{_T("adjbs"),			0,				0,					0,					0				},
	// 0x40
	{_T("inc"),				0,				PARAM_AW,			0,					0				},
	{_T("inc"),				0,				PARAM_CW,			0,					0				},
	{_T("inc"),				0,				PARAM_DW,			0,					0				},
	{_T("inc"),				0,				PARAM_BW,			0,					0				},
	{_T("inc"),				0,				PARAM_SP,			0,					0				},
	{_T("inc"),				0,				PARAM_BP,			0,					0				},
	{_T("inc"),				0,				PARAM_IX,			0,					0				},
	{_T("inc"),				0,				PARAM_IY,			0,					0				},
	{_T("dec"),				0,				PARAM_AW,			0,					0				},
	{_T("dec"),				0,				PARAM_CW,			0,					0				},
	{_T("dec"),				0,				PARAM_DW,			0,					0				},
	{_T("dec"),				0,				PARAM_BW,			0,					0				},
	{_T("dec"),				0,				PARAM_SP,			0,					0				},
	{_T("dec"),				0,				PARAM_BP,			0,					0				},
	{_T("dec"),				0,				PARAM_IX,			0,					0				},
	{_T("dec"),				0,				PARAM_IY,			0,					0				},
	// 0x50
	{_T("push"),			0,				PARAM_AW,			0,					0				},
	{_T("push"),			0,				PARAM_CW,			0,					0				},
	{_T("push"),			0,				PARAM_DW,			0,					0				},
	{_T("push"),			0,				PARAM_BW,			0,					0				},
	{_T("push"),			0,				PARAM_SP,			0,					0				},
	{_T("push"),			0,				PARAM_BP,			0,					0				},
	{_T("push"),			0,				PARAM_IX,			0,					0				},
	{_T("push"),			0,				PARAM_IY,			0,					0				},
	{_T("pop"),				0,				PARAM_AW,			0,					0				},
	{_T("pop"),				0,				PARAM_CW,			0,					0				},
	{_T("pop"),				0,				PARAM_DW,			0,					0				},
	{_T("pop"),				0,				PARAM_BW,			0,					0				},
	{_T("pop"),				0,				PARAM_SP,			0,					0				},
	{_T("pop"),				0,				PARAM_BP,			0,					0				},
	{_T("pop"),				0,				PARAM_IX,			0,					0				},
	{_T("pop"),				0,				PARAM_IY,			0,					0				},
	// 0x60
	{_T("push    r"),		0,				0,					0,					0				},
	{_T("pop     r"),		0,				0,					0,					0				},
	{_T("chkind"),			MODRM,			PARAM_REG16,		PARAM_RM16,			0				},
	{_T("brkn"),			0,				PARAM_UI8,			0,					0,				DASMFLAG_STEP_OVER},	/* V25S/V35S only */
	{_T("repnc"),			PREFIX,			0,					0,					0				},
	{_T("repc"),			PREFIX,			0,					0,					0				},
	{_T("fpo2    0"),		0,				0,					0,					0				},	/* for a coprocessor that was never made */
	{_T("fpo2    1"),		0,				0,					0,					0				},	/* for a coprocessor that was never made */
	{_T("push"),			0,				PARAM_IMM,			0,					0				},
	{_T("mul"),				MODRM,			PARAM_REG16,		PARAM_RM16,			PARAM_IMM		},
	{_T("push"),			0,				PARAM_I8,			0,					0				},
	{_T("mul"),				MODRM,			PARAM_REG16,		PARAM_RM16,			PARAM_I8		},
	{_T("inmb"),			0,				0,					0,					0				},
	{_T("inmw"),			0,				0,					0,					0				},
	{_T("outmb"),			0,				0,					0,					0				},
	{_T("outmw"),			0,				0,					0,					0				},
	// 0x70
	{_T("bv"),				0,				PARAM_REL8,			0,					0				},
	{_T("bnv"),				0,				PARAM_REL8,			0,					0				},
	{_T("bc"),				0,				PARAM_REL8,			0,					0				},
	{_T("bnc"),				0,				PARAM_REL8,			0,					0				},
	{_T("be"),				0,				PARAM_REL8,			0,					0				},
	{_T("bne"),				0,				PARAM_REL8,			0,					0				},
	{_T("bnh"),				0,				PARAM_REL8,			0,					0				},
	{_T("bh"),				0,				PARAM_REL8,			0,					0				},
	{_T("bn"),				0,				PARAM_REL8,			0,					0				},
	{_T("bp"),				0,				PARAM_REL8,			0,					0				},
	{_T("bpe"),				0,				PARAM_REL8,			0,					0				},
	{_T("bpo"),				0,				PARAM_REL8,			0,					0				},
	{_T("blt"),				0,				PARAM_REL8,			0,					0				},
	{_T("bge"),				0,				PARAM_REL8,			0,					0				},
	{_T("ble"),				0,				PARAM_REL8,			0,					0				},
	{_T("bgt"),				0,				PARAM_REL8,			0,					0				},
	// 0x80
	{_T("immb"),			GROUP,			0,					0,					0				},
	{_T("immw"),			GROUP,			0,					0,					0				},
	{_T("immb"),			GROUP,			0,					0,					0				},
	{_T("immws"),			GROUP,			0,					0,					0				},
	{_T("test"),			MODRM,			PARAM_RM8,			PARAM_REG8,			0				},
	{_T("test"),			MODRM,			PARAM_RM16,			PARAM_REG16,		0				},
	{_T("xch"),				MODRM,			PARAM_REG8,			PARAM_RM8,			0				},
	{_T("xch"),				MODRM,			PARAM_REG16,		PARAM_RM16,			0				},
	{_T("mov"),				MODRM,			PARAM_RM8,			PARAM_REG8,			0				},
	{_T("mov"),				MODRM,			PARAM_RM16,			PARAM_REG16,		0				},
	{_T("mov"),				MODRM,			PARAM_REG8,			PARAM_RM8,			0				},
	{_T("mov"),				MODRM,			PARAM_REG16,		PARAM_RM16,			0				},
	{_T("mov"),				MODRM,			PARAM_RM16,			PARAM_SREG,			0				},
	{_T("ldea"),			MODRM,			PARAM_REG16,		PARAM_RM16,			0				},
	{_T("mov"),				MODRM,			PARAM_SREG,			PARAM_RM16,			0				},
	{_T("pop"),				MODRM,			PARAM_RM16,			0,					0				},
	// 0x90
	{_T("nop"),				0,				0,					0,					0				},
	{_T("xch"),				0,				PARAM_AW,			PARAM_CW,			0				},
	{_T("xch"),				0,				PARAM_AW,			PARAM_DW,			0				},
	{_T("xch"),				0,				PARAM_AW,			PARAM_BW,			0				},
	{_T("xch"),				0,				PARAM_AW,			PARAM_SP,			0				},
	{_T("xch"),				0,				PARAM_AW,			PARAM_BP,			0				},
	{_T("xch"),				0,				PARAM_AW,			PARAM_IX,			0				},
	{_T("xch"),				0,				PARAM_AW,			PARAM_IY,			0				},
	{_T("cvtbw"),			0,				0,					0,					0				},
	{_T("cvtwl"),			0,				0,					0,					0				},
	{_T("call"),			0,				PARAM_ADDR,			0,					0,				DASMFLAG_STEP_OVER},
	{_T("poll"),			0,				0,					0,					0				},
	{_T("push    psw"),		0,				0,					0,					0				},
	{_T("pop     psw"),		0,				0,					0,					0				},
	{_T("mov     psw,ah"),	0,				0,					0,					0				},
	{_T("mov     ah,psw"),	0,				0,					0,					0				},
	// 0xa0
	{_T("mov"),				0,				PARAM_AL,			PARAM_MEM_OFFS,		0				},
	{_T("mov"),				0,				PARAM_AW,			PARAM_MEM_OFFS,		0				},
	{_T("mov"),				0,				PARAM_MEM_OFFS,		PARAM_AL,			0				},
	{_T("mov"),				0,				PARAM_MEM_OFFS,		PARAM_AW,			0				},
	{_T("movbkb"),			0,				0,					0,					0				},
	{_T("movbkw"),			0,				0,					0,					0				},
	{_T("cmpbkb"),			0,				0,					0,					0				},
	{_T("cmpbkw"),			0,				0,					0,					0				},
	{_T("test"),			0,				PARAM_AL,			PARAM_UI8,			0				},
	{_T("test"),			0,				PARAM_AW,			PARAM_IMM,			0				},
	{_T("stmb"),			0,				0,					0,					0				},
	{_T("stmw"),			0,				0,					0,					0				},
	{_T("ldmb"),			0,				0,					0,					0				},
	{_T("ldmw"),			0,				0,					0,					0				},
	{_T("cmpmb"),			0,				0,					0,					0				},
	{_T("cmpmw"),			0,				0,					0,					0				},
	// 0xb0
	{_T("mov"),				0,				PARAM_AL,			PARAM_UI8,			0				},
	{_T("mov"),				0,				PARAM_CL,			PARAM_UI8,			0				},
	{_T("mov"),				0,				PARAM_DL,			PARAM_UI8,			0				},
	{_T("mov"),				0,				PARAM_BL,			PARAM_UI8,			0				},
	{_T("mov"),				0,				PARAM_AH,			PARAM_UI8,			0				},
	{_T("mov"),				0,				PARAM_CH,			PARAM_UI8,			0				},
	{_T("mov"),				0,				PARAM_DH,			PARAM_UI8,			0				},
	{_T("mov"),				0,				PARAM_BH,			PARAM_UI8,			0				},
	{_T("mov"),				0,				PARAM_AW,			PARAM_IMM,			0				},
	{_T("mov"),				0,				PARAM_CW,			PARAM_IMM,			0				},
	{_T("mov"),				0,				PARAM_DW,			PARAM_IMM,			0				},
	{_T("mov"),				0,				PARAM_BW,			PARAM_IMM,			0				},
	{_T("mov"),				0,				PARAM_SP,			PARAM_IMM,			0				},
	{_T("mov"),				0,				PARAM_BP,			PARAM_IMM,			0				},
	{_T("mov"),				0,				PARAM_IX,			PARAM_IMM,			0				},
	{_T("mov"),				0,				PARAM_IY,			PARAM_IMM,			0				},
	// 0xc0
	{_T("shiftbi"),			GROUP,			0,					0,					0				},
	{_T("shiftwi"),			GROUP,			0,					0,					0				},
	{_T("ret"),				0,				PARAM_I16,			0,					0,				DASMFLAG_STEP_OUT},
	{_T("ret"),				0,				0,					0,					0,				DASMFLAG_STEP_OUT},
	{_T("mov     ds1,"),	MODRM,			PARAM_REG16,		PARAM_RM16,			0				},
	{_T("mov     ds0,"),	MODRM,			PARAM_REG16,		PARAM_RM16,			0				},
	{_T("mov"),				MODRM,			PARAM_RMPTR8,		PARAM_UI8,			0				},
	{_T("mov"),				MODRM,			PARAM_RMPTR16,		PARAM_IMM,			0				},
	{_T("prepare"),			0,				PARAM_I16,			PARAM_UI8,			0				},
	{_T("dispose"),			0,				0,					0,					0				},
	{_T("retf"),			0,				PARAM_I16,			0,					0,				DASMFLAG_STEP_OUT},
	{_T("retf"),			0,				0,					0,					0,				DASMFLAG_STEP_OUT},
	{_T("brk     3"),		0,				0,					0,					0,				DASMFLAG_STEP_OVER},
	{_T("brk"),				0,				PARAM_UI8,			0,					0,				DASMFLAG_STEP_OVER},
	{_T("brkv"),			0,				0,					0,					0				},
	{_T("reti"),			0,				0,					0,					0,				DASMFLAG_STEP_OUT},
	// 0xd0
	{_T("shiftb"),			GROUP,			0,					0,					0				},
	{_T("shiftw"),			GROUP,			0,					0,					0				},
	{_T("shiftbv"),			GROUP,			0,					0,					0				},
	{_T("shiftwv"),			GROUP,			0,					0,					0				},
	{_T("cvtbd"),			0,				PARAM_I8,			0,					0				},
	{_T("cvtdb"),			0,				PARAM_I8,			0,					0				},
	{_T("???"),				0,				0,					0,					0				},
	{_T("trans"),			0,				0,					0,					0				},
	{_T("escape"),			FPU,			0,					0,					0				},
	{_T("escape"),			FPU,			0,					0,					0				},
	{_T("escape"),			FPU,			0,					0,					0				},
	{_T("escape"),			FPU,			0,					0,					0				},
	{_T("escape"),			FPU,			0,					0,					0				},
	{_T("escape"),			FPU,			0,					0,					0				},
	{_T("escape"),			FPU,			0,					0,					0				},
	{_T("escape"),			FPU,			0,					0,					0				},
	// 0xe0
	{_T("dbnzne"),			0,				PARAM_REL8,			0,					0,				DASMFLAG_STEP_OVER},
	{_T("dbnze"),			0,				PARAM_REL8,			0,					0,				DASMFLAG_STEP_OVER},
	{_T("dbnz"),			0,				PARAM_REL8,			0,					0,				DASMFLAG_STEP_OVER},
	{_T("bcwz"),			0,				PARAM_REL8,			0,					0				},
	{_T("in"),				0,				PARAM_AL,			PARAM_UI8,			0				},
	{_T("in"),				0,				PARAM_AW,			PARAM_UI8,			0				},
	{_T("out"),				0,				PARAM_UI8,			PARAM_AL,			0				},
	{_T("out"),				0,				PARAM_UI8,			PARAM_AW,			0				},
	{_T("call"),			0,				PARAM_REL16,		0,					0,				DASMFLAG_STEP_OVER},
	{_T("br"),				0,				PARAM_REL16,		0,					0				},
	{_T("br"),				0,				PARAM_ADDR,			0,					0				},
	{_T("br"),				0,				PARAM_REL8,			0,					0				},
	{_T("in"),				0,				PARAM_AL,			PARAM_DW,			0				},
	{_T("in"),				0,				PARAM_AW,			PARAM_DW,			0				},
	{_T("out"),				0,				PARAM_DW,			PARAM_AL,			0				},
	{_T("out"),				0,				PARAM_DW,			PARAM_AW,			0				},
	// 0xf0
	{_T("buslock"),			PREFIX,			0,					0,					0				},
	{_T("brks"),			0,				PARAM_UI8,			0,					0,				DASMFLAG_STEP_OVER},	/* V25S/V35S only */
	{_T("repne"),			PREFIX,			0,					0,					0				},
	{_T("rep"),				PREFIX,			0,					0,					0				},
	{_T("halt"),			0,				0,					0,					0				},
	{_T("not1    cy"),		0,				0,					0,					0				},
	{_T("group1b"),			GROUP,			0,					0,					0				},
	{_T("group1w"),			GROUP,			0,					0,					0				},
	{_T("clr1    cy"),		0,				0,					0,					0				},
	{_T("set1    cy"),		0,				0,					0,					0				},
	{_T("di"),				0,				0,					0,					0				},
	{_T("ei"),				0,				0,					0,					0				},
	{_T("clr1    dir"),		0,				0,					0,					0				},
	{_T("set1    dir"),		0,				0,					0,					0				},
	{_T("group2b"),			GROUP,			0,					0,					0				},
	{_T("group2w"),			GROUP,			0,					0,					0				}
};

static const I386_OPCODE necv_opcode_table2[256] =
{
	// 0x00
	{_T("???"),				0,				0,					0,					0				},
	{_T("???"),				0,				0,					0,					0				},
	{_T("???"),				0,				0,					0,					0				},
	{_T("???"),				0,				0,					0,					0				},
	{_T("???"),				0,				0,					0,					0				},
	{_T("???"),				0,				0,					0,					0				},
	{_T("???"),				0,				0,					0,					0				},
	{_T("???"),				0,				0,					0,					0				},
	{_T("???"),				0,				0,					0,					0				},
	{_T("???"),				0,				0,					0,					0				},
	{_T("???"),				0,				0,					0,					0				},
	{_T("???"),				0,				0,					0,					0				},
	{_T("???"),				0,				0,					0,					0				},
	{_T("???"),				0,				0,					0,					0				},
	{_T("???"),				0,				0,					0,					0				},
	{_T("???"),				0,				0,					0,					0				},
	// 0x10
	{_T("test1"),			MODRM,			PARAM_RMPTR8,		PARAM_CL,			0				},
	{_T("test1"),			MODRM,			PARAM_RMPTR16,		PARAM_CL,			0				},
	{_T("clr1"),			MODRM,			PARAM_RMPTR8,		PARAM_CL,			0				},
	{_T("clr1"),			MODRM,			PARAM_RMPTR16,		PARAM_CL,			0				},
	{_T("set1"),			MODRM,			PARAM_RMPTR8,		PARAM_CL,			0				},
	{_T("set1"),			MODRM,			PARAM_RMPTR16,		PARAM_CL,			0				},
	{_T("not1"),			MODRM,			PARAM_RMPTR8,		PARAM_CL,			0				},
	{_T("not1"),			MODRM,			PARAM_RMPTR16,		PARAM_CL,			0				},
	{_T("test1"),			MODRM,			PARAM_RMPTR8,		PARAM_I3,			0				},
	{_T("test1"),			MODRM,			PARAM_RMPTR16,		PARAM_I4,			0				},
	{_T("clr1"),			MODRM,			PARAM_RMPTR8,		PARAM_I3,			0				},
	{_T("clr1"),			MODRM,			PARAM_RMPTR16,		PARAM_I4,			0				},
	{_T("set1"),			MODRM,			PARAM_RMPTR8,		PARAM_I3,			0				},
	{_T("set1"),			MODRM,			PARAM_RMPTR16,		PARAM_I4,			0				},
	{_T("not1"),			MODRM,			PARAM_RMPTR8,		PARAM_I3,			0				},
	{_T("not1"),			MODRM,			PARAM_RMPTR16,		PARAM_I4,			0				},
	// 0x20
	{_T("add4s"),			0,				0,					0,					0				},
	{_T("???"),				0,				0,					0,					0				},
	{_T("sub4s"),			0,				0,					0,					0				},
	{_T("???"),				0,				0,					0,					0				},
	{_T("???"),				0,				0,					0,					0				},
	{_T("movspa"),			0,				0,					0,					0				},	/* V25/V35 only */
	{_T("cmp4s"),			0,				0,					0,					0				},
	{_T("???"),				0,				0,					0,					0				},
	{_T("rol4"),			MODRM,			PARAM_RMPTR8,		0,					0				},
	{_T("???"),				0,				0,					0,					0				},
	{_T("ror4"),			MODRM,			PARAM_RMPTR8,		0,					0				},
	{_T("???"),				0,				0,					0,					0				},
	{_T("???"),				0,				0,					0,					0				},
	{_T("brkcs"),			MODRM,			PARAM_REG2_16,		0,					0,				DASMFLAG_STEP_OVER},	/* V25/V35 only */
	{_T("???"),				0,				0,					0,					0				},
	{_T("???"),				0,				0,					0,					0				},
	// 0x30
	{_T("???"),				0,				0,					0,					0				},
	{_T("ins"),				MODRM,			PARAM_REG2_8,		PARAM_REG8,			0				},
	{_T("???"),				0,				0,					0,					0				},
	{_T("ext"),				MODRM,			PARAM_REG2_8,		PARAM_REG8,			0				},
	{_T("???"),				0,				0,					0,					0				},
	{_T("???"),				0,				0,					0,					0				},
	{_T("???"),				0,				0,					0,					0				},
	{_T("???"),				0,				0,					0,					0				},
	{_T("???"),				0,				0,					0,					0				},
	{_T("ins"),				MODRM,			PARAM_REG2_8,		PARAM_I4,			0				},
	{_T("???"),				0,				0,					0,					0				},
	{_T("ext"),				MODRM,			PARAM_REG2_8,		PARAM_I4,			0				},
	{_T("???"),				0,				0,					0,					0				},
	{_T("???"),				0,				0,					0,					0				},
	{_T("???"),				0,				0,					0,					0				},
	{_T("???"),				0,				0,					0,					0				},
	// 0x40
	{_T("???"),				0,				0,					0,					0				},
	{_T("???"),				0,				0,					0,					0				},
	{_T("???"),				0,				0,					0,					0				},
	{_T("???"),				0,				0,					0,					0				},
	{_T("???"),				0,				0,					0,					0				},
	{_T("???"),				0,				0,					0,					0				},
	{_T("???"),				0,				0,					0,					0				},
	{_T("???"),				0,				0,					0,					0				},
	{_T("???"),				0,				0,					0,					0				},
	{_T("???"),				0,				0,					0,					0				},
	{_T("???"),				0,				0,					0,					0				},
	{_T("???"),				0,				0,					0,					0				},
	{_T("???"),				0,				0,					0,					0				},
	{_T("???"),				0,				0,					0,					0				},
	{_T("???"),				0,				0,					0,					0				},
	{_T("???"),				0,				0,					0,					0				},
	// 0x50
	{_T("???"),				0,				0,					0,					0				},
	{_T("???"),				0,				0,					0,					0				},
	{_T("???"),				0,				0,					0,					0				},
	{_T("???"),				0,				0,					0,					0				},
	{_T("???"),				0,				0,					0,					0				},
	{_T("???"),				0,				0,					0,					0				},
	{_T("???"),				0,				0,					0,					0				},
	{_T("???"),				0,				0,					0,					0				},
	{_T("???"),				0,				0,					0,					0				},
	{_T("???"),				0,				0,					0,					0				},
	{_T("???"),				0,				0,					0,					0				},
	{_T("???"),				0,				0,					0,					0				},
	{_T("???"),				0,				0,					0,					0				},
	{_T("???"),				0,				0,					0,					0				},
	{_T("???"),				0,				0,					0,					0				},
	{_T("???"),				0,				0,					0,					0				},
	// 0x60
	{_T("???"),				0,				0,					0,					0				},
	{_T("???"),				0,				0,					0,					0				},
	{_T("???"),				0,				0,					0,					0				},
	{_T("???"),				0,				0,					0,					0				},
	{_T("???"),				0,				0,					0,					0				},
	{_T("???"),				0,				0,					0,					0				},
	{_T("???"),				0,				0,					0,					0				},
	{_T("???"),				0,				0,					0,					0				},
	{_T("???"),				0,				0,					0,					0				},
	{_T("???"),				0,				0,					0,					0				},
	{_T("???"),				0,				0,					0,					0				},
	{_T("???"),				0,				0,					0,					0				},
	{_T("???"),				0,				0,					0,					0				},
	{_T("???"),				0,				0,					0,					0				},
	{_T("???"),				0,				0,					0,					0				},
	{_T("???"),				0,				0,					0,					0				},
	// 0x70
	{_T("???"),				0,				0,					0,					0				},
	{_T("???"),				0,				0,					0,					0				},
	{_T("???"),				0,				0,					0,					0				},
	{_T("???"),				0,				0,					0,					0				},
	{_T("???"),				0,				0,					0,					0				},
	{_T("???"),				0,				0,					0,					0				},
	{_T("???"),				0,				0,					0,					0				},
	{_T("???"),				0,				0,					0,					0				},
	{_T("???"),				0,				0,					0,					0				},
	{_T("???"),				0,				0,					0,					0				},
	{_T("???"),				0,				0,					0,					0				},
	{_T("???"),				0,				0,					0,					0				},
	{_T("???"),				0,				0,					0,					0				},
	{_T("???"),				0,				0,					0,					0				},
	{_T("???"),				0,				0,					0,					0				},
	{_T("???"),				0,				0,					0,					0				},
	// 0x80
	{_T("???"),				0,				0,					0,					0				},
	{_T("???"),				0,				0,					0,					0				},
	{_T("???"),				0,				0,					0,					0				},
	{_T("???"),				0,				0,					0,					0				},
	{_T("???"),				0,				0,					0,					0				},
	{_T("???"),				0,				0,					0,					0				},
	{_T("???"),				0,				0,					0,					0				},
	{_T("???"),				0,				0,					0,					0				},
	{_T("???"),				0,				0,					0,					0				},
	{_T("???"),				0,				0,					0,					0				},
	{_T("???"),				0,				0,					0,					0				},
	{_T("???"),				0,				0,					0,					0				},
	{_T("???"),				0,				0,					0,					0				},
	{_T("???"),				0,				0,					0,					0				},
	{_T("???"),				0,				0,					0,					0				},
	{_T("???"),				0,				0,					0,					0				},
	// 0x90
	{_T("???"),				0,				0,					0,					0				},
	{_T("retrbi"),			0,				0,					0,					0				},	/* V25/V35 only */
	{_T("fint"),			0,				0,					0,					0				},	/* V25/V35 only */
	{_T("???"),				0,				0,					0,					0				},
	{_T("tsksw"),			MODRM,			PARAM_REG2_16,		0,					0				},	/* V25/V35 only */
	{_T("movspb"),			MODRM,			PARAM_REG2_16,		0,					0				},	/* V25/V35 only */
	{_T("???"),				0,				0,					0,					0				},
	{_T("???"),				0,				0,					0,					0				},
	{_T("???"),				0,				0,					0,					0				},
	{_T("???"),				0,				0,					0,					0				},
	{_T("???"),				0,				0,					0,					0				},
	{_T("???"),				0,				0,					0,					0				},
	{_T("btclr"),			0,				PARAM_SFREG,		PARAM_I3,			PARAM_REL8		},	/* V25/V35 only */
	{_T("???"),				0,				0,					0,					0				},
	{_T("stop"),			0,				0,					0,					0				},	/* V25/V35 only */
	{_T("???"),				0,				0,					0,					0				},
	// 0xa0
	{_T("???"),				0,				0,					0,					0				},
	{_T("???"),				0,				0,					0,					0				},
	{_T("???"),				0,				0,					0,					0				},
	{_T("???"),				0,				0,					0,					0				},
	{_T("???"),				0,				0,					0,					0				},
	{_T("???"),				0,				0,					0,					0				},
	{_T("???"),				0,				0,					0,					0				},
	{_T("???"),				0,				0,					0,					0				},
	{_T("???"),				0,				0,					0,					0				},
	{_T("???"),				0,				0,					0,					0				},
	{_T("???"),				0,				0,					0,					0				},
	{_T("???"),				0,				0,					0,					0				},
	{_T("???"),				0,				0,					0,					0				},
	{_T("???"),				0,				0,					0,					0				},
	{_T("???"),				0,				0,					0,					0				},
	{_T("???"),				0,				0,					0,					0				},
	// 0xb0
	{_T("???"),				0,				0,					0,					0				},
	{_T("???"),				0,				0,					0,					0				},
	{_T("???"),				0,				0,					0,					0				},
	{_T("???"),				0,				0,					0,					0				},
	{_T("???"),				0,				0,					0,					0				},
	{_T("???"),				0,				0,					0,					0				},
	{_T("???"),				0,				0,					0,					0				},
	{_T("???"),				0,				0,					0,					0				},
	{_T("???"),				0,				0,					0,					0				},
	{_T("???"),				0,				0,					0,					0				},
	{_T("???"),				0,				0,					0,					0				},
	{_T("???"),				0,				0,					0,					0				},
	{_T("???"),				0,				0,					0,					0				},
	{_T("???"),				0,				0,					0,					0				},
	{_T("???"),				0,				0,					0,					0				},
	{_T("???"),				0,				0,					0,					0				},
	// 0xc0
	{_T("???"),				0,				0,					0,					0				},
	{_T("???"),				0,				0,					0,					0				},
	{_T("???"),				0,				0,					0,					0				},
	{_T("???"),				0,				0,					0,					0				},
	{_T("???"),				0,				0,					0,					0				},
	{_T("???"),				0,				0,					0,					0				},
	{_T("???"),				0,				0,					0,					0				},
	{_T("???"),				0,				0,					0,					0				},
	{_T("???"),				0,				0,					0,					0				},
	{_T("???"),				0,				0,					0,					0				},
	{_T("???"),				0,				0,					0,					0				},
	{_T("???"),				0,				0,					0,					0				},
	{_T("???"),				0,				0,					0,					0				},
	{_T("???"),				0,				0,					0,					0				},
	{_T("???"),				0,				0,					0,					0				},
	{_T("???"),				0,				0,					0,					0				},
	// 0xd0
	{_T("???"),				0,				0,					0,					0				},
	{_T("???"),				0,				0,					0,					0				},
	{_T("???"),				0,				0,					0,					0				},
	{_T("???"),				0,				0,					0,					0				},
	{_T("???"),				0,				0,					0,					0				},
	{_T("???"),				0,				0,					0,					0				},
	{_T("???"),				0,				0,					0,					0				},
	{_T("???"),				0,				0,					0,					0				},
	{_T("???"),				0,				0,					0,					0				},
	{_T("???"),				0,				0,					0,					0				},
	{_T("???"),				0,				0,					0,					0				},
	{_T("???"),				0,				0,					0,					0				},
	{_T("???"),				0,				0,					0,					0				},
	{_T("???"),				0,				0,					0,					0				},
	{_T("???"),				0,				0,					0,					0				},
	{_T("???"),				0,				0,					0,					0				},
	// 0xe0
	{_T("brkxa"),			0,				PARAM_UI8,			0,					0				},	/* V33,53 only */
	{_T("???"),				0,				0,					0,					0				},
	{_T("???"),				0,				0,					0,					0				},
	{_T("???"),				0,				0,					0,					0				},
	{_T("???"),				0,				0,					0,					0				},
	{_T("???"),				0,				0,					0,					0				},
	{_T("???"),				0,				0,					0,					0				},
	{_T("???"),				0,				0,					0,					0				},
	{_T("???"),				0,				0,					0,					0				},
	{_T("???"),				0,				0,					0,					0				},
	{_T("???"),				0,				0,					0,					0				},
	{_T("???"),				0,				0,					0,					0				},
	{_T("???"),				0,				0,					0,					0				},
	{_T("???"),				0,				0,					0,					0				},
	{_T("???"),				0,				0,					0,					0				},
	{_T("???"),				0,				0,					0,					0				},
	// 0xf0
	{_T("retxa"),			0,				PARAM_UI8,			0,					0				},	/* V33,53 only */
	{_T("???"),				0,				0,					0,					0				},
	{_T("???"),				0,				0,					0,					0				},
	{_T("???"),				0,				0,					0,					0				},
	{_T("???"),				0,				0,					0,					0				},
	{_T("???"),				0,				0,					0,					0				},
	{_T("???"),				0,				0,					0,					0				},
	{_T("???"),				0,				0,					0,					0				},
	{_T("???"),				0,				0,					0,					0				},
	{_T("???"),				0,				0,					0,					0				},
	{_T("???"),				0,				0,					0,					0				},
	{_T("???"),				0,				0,					0,					0				},
	{_T("???"),				0,				0,					0,					0				},
	{_T("???"),				0,				0,					0,					0				},
	{_T("???"),				0,				0,					0,					0				},
	{_T("brkem"),			0,				PARAM_UI8,			0,					0				}	/* V20,30,40,50 only */
};

static const I386_OPCODE immb_table[8] =
{
	{_T("add"),				0,				PARAM_RMPTR8,		PARAM_UI8,			0				},
	{_T("or"),				0,				PARAM_RMPTR8,		PARAM_UI8,			0				},
	{_T("addc"),			0,				PARAM_RMPTR8,		PARAM_UI8,			0				},
	{_T("subc"),			0,				PARAM_RMPTR8,		PARAM_UI8,			0				},
	{_T("and"),				0,				PARAM_RMPTR8,		PARAM_UI8,			0				},
	{_T("sub"),				0,				PARAM_RMPTR8,		PARAM_UI8,			0				},
	{_T("xor"),				0,				PARAM_RMPTR8,		PARAM_UI8,			0				},
	{_T("cmp"),				0,				PARAM_RMPTR8,		PARAM_UI8,			0				}
};

static const I386_OPCODE immw_table[8] =
{
	{_T("add"),				0,				PARAM_RMPTR16,		PARAM_IMM,			0				},
	{_T("or"),				0,				PARAM_RMPTR16,		PARAM_IMM,			0				},
	{_T("addc"),			0,				PARAM_RMPTR16,		PARAM_IMM,			0				},
	{_T("subc"),			0,				PARAM_RMPTR16,		PARAM_IMM,			0				},
	{_T("and"),				0,				PARAM_RMPTR16,		PARAM_IMM,			0				},
	{_T("sub"),				0,				PARAM_RMPTR16,		PARAM_IMM,			0				},
	{_T("xor"),				0,				PARAM_RMPTR16,		PARAM_IMM,			0				},
	{_T("cmp"),				0,				PARAM_RMPTR16,		PARAM_IMM,			0				}
};

static const I386_OPCODE immws_table[8] =
{
	{_T("add"),				0,				PARAM_RMPTR16,		PARAM_I8,			0				},
	{_T("or"),				0,				PARAM_RMPTR16,		PARAM_I8,			0				},
	{_T("addc"),			0,				PARAM_RMPTR16,		PARAM_I8,			0				},
	{_T("subc"),			0,				PARAM_RMPTR16,		PARAM_I8,			0				},
	{_T("and"),				0,				PARAM_RMPTR16,		PARAM_I8,			0				},
	{_T("sub"),				0,				PARAM_RMPTR16,		PARAM_I8,			0				},
	{_T("xor"),				0,				PARAM_RMPTR16,		PARAM_I8,			0				},
	{_T("cmp"),				0,				PARAM_RMPTR16,		PARAM_I8,			0				}
};

static const I386_OPCODE shiftbi_table[8] =
{
	{_T("rol"),				0,				PARAM_RMPTR8,		PARAM_I8,			0				},
	{_T("ror"),				0,				PARAM_RMPTR8,		PARAM_I8,			0				},
	{_T("rolc"),			0,				PARAM_RMPTR8,		PARAM_I8,			0				},
	{_T("rorc"),			0,				PARAM_RMPTR8,		PARAM_I8,			0				},
	{_T("shl"),				0,				PARAM_RMPTR8,		PARAM_I8,			0				},
	{_T("shr"),				0,				PARAM_RMPTR8,		PARAM_I8,			0				},
	{_T("???"),				0,				PARAM_RMPTR8,		PARAM_I8,			0				},
	{_T("shra"),			0,				PARAM_RMPTR8,		PARAM_I8,			0				}
};

static const I386_OPCODE shiftwi_table[8] =
{
	{_T("rol"),				0,				PARAM_RMPTR16,		PARAM_I8,			0				},
	{_T("ror"),				0,				PARAM_RMPTR16,		PARAM_I8,			0				},
	{_T("rolc"),			0,				PARAM_RMPTR16,		PARAM_I8,			0				},
	{_T("rorc"),			0,				PARAM_RMPTR16,		PARAM_I8,			0				},
	{_T("shl"),				0,				PARAM_RMPTR16,		PARAM_I8,			0				},
	{_T("shr"),				0,				PARAM_RMPTR16,		PARAM_I8,			0				},
	{_T("???"),				0,				PARAM_RMPTR16,		PARAM_I8,			0				},
	{_T("shra"),			0,				PARAM_RMPTR16,		PARAM_I8,			0				}
};

static const I386_OPCODE shiftb_table[8] =
{
	{_T("rol"),				0,				PARAM_RMPTR8,		PARAM_1,			0				},
	{_T("ror"),				0,				PARAM_RMPTR8,		PARAM_1,			0				},
	{_T("rolc"),			0,				PARAM_RMPTR8,		PARAM_1,			0				},
	{_T("rorc"),			0,				PARAM_RMPTR8,		PARAM_1,			0				},
	{_T("shl"),				0,				PARAM_RMPTR8,		PARAM_1,			0				},
	{_T("shr"),				0,				PARAM_RMPTR8,		PARAM_1,			0				},
	{_T("???"),				0,				PARAM_RMPTR8,		PARAM_1,			0				},
	{_T("shra"),			0,				PARAM_RMPTR8,		PARAM_1,			0				}
};

static const I386_OPCODE shiftw_table[8] =
{
	{_T("rol"),				0,				PARAM_RMPTR16,		PARAM_1,			0				},
	{_T("ror"),				0,				PARAM_RMPTR16,		PARAM_1,			0				},
	{_T("rolc"),			0,				PARAM_RMPTR16,		PARAM_1,			0				},
	{_T("rorc"),			0,				PARAM_RMPTR16,		PARAM_1,			0				},
	{_T("shl"),				0,				PARAM_RMPTR16,		PARAM_1,			0				},
	{_T("shr"),				0,				PARAM_RMPTR16,		PARAM_1,			0				},
	{_T("???"),				0,				PARAM_RMPTR16,		PARAM_1,			0				},
	{_T("shra"),			0,				PARAM_RMPTR16,		PARAM_1,			0				}
};

static const I386_OPCODE shiftbv_table[8] =
{
	{_T("rol"),				0,				PARAM_RMPTR8,		PARAM_CL,			0				},
	{_T("ror"),				0,				PARAM_RMPTR8,		PARAM_CL,			0				},
	{_T("rolc"),			0,				PARAM_RMPTR8,		PARAM_CL,			0				},
	{_T("rorc"),			0,				PARAM_RMPTR8,		PARAM_CL,			0				},
	{_T("shl"),				0,				PARAM_RMPTR8,		PARAM_CL,			0				},
	{_T("shr"),				0,				PARAM_RMPTR8,		PARAM_CL,			0				},
	{_T("???"),				0,				PARAM_RMPTR8,		PARAM_CL,			0				},
	{_T("shra"),			0,				PARAM_RMPTR8,		PARAM_CL,			0				}
};

static const I386_OPCODE shiftwv_table[8] =
{
	{_T("rol"),				0,				PARAM_RMPTR16,		PARAM_CL,			0				},
	{_T("ror"),				0,				PARAM_RMPTR16,		PARAM_CL,			0				},
	{_T("rolc"),			0,				PARAM_RMPTR16,		PARAM_CL,			0				},
	{_T("rorc"),			0,				PARAM_RMPTR16,		PARAM_CL,			0				},
	{_T("shl"),				0,				PARAM_RMPTR16,		PARAM_CL,			0				},
	{_T("shr"),				0,				PARAM_RMPTR16,		PARAM_CL,			0				},
	{_T("???"),				0,				PARAM_RMPTR16,		PARAM_CL,			0				},
	{_T("shra"),			0,				PARAM_RMPTR16,		PARAM_CL,			0				}
};

static const I386_OPCODE group1b_table[8] =
{
	{_T("test"),			0,				PARAM_RMPTR8,		PARAM_UI8,			0				},
	{_T("???"),				0,				0,					0,					0				},
	{_T("not"),				0,				PARAM_RMPTR8,		0,					0				},
	{_T("neg"),				0,				PARAM_RMPTR8,		0,					0				},
	{_T("mulu"),			0,				PARAM_RMPTR8,		0,					0				},
	{_T("mul"),				0,				PARAM_RMPTR8,		0,					0				},
	{_T("divu"),			0,				PARAM_RMPTR8,		0,					0				},
	{_T("div"),				0,				PARAM_RMPTR8,		0,					0				}
};

static const I386_OPCODE group1w_table[8] =
{
	{_T("test"),			0,				PARAM_RMPTR16,		PARAM_IMM,			0				},
	{_T("???"),				0,				0,					0,					0				},
	{_T("not"),				0,				PARAM_RMPTR16,		0,					0				},
	{_T("neg"),				0,				PARAM_RMPTR16,		0,					0				},
	{_T("mulu"),			0,				PARAM_RMPTR16,		0,					0				},
	{_T("mul"),				0,				PARAM_RMPTR16,		0,					0				},
	{_T("divu"),			0,				PARAM_RMPTR16,		0,					0				},
	{_T("div"),				0,				PARAM_RMPTR16,		0,					0				}
};

static const I386_OPCODE group2b_table[8] =
{
	{_T("inc"),				0,				PARAM_RMPTR8,		0,					0				},
	{_T("dec"),				0,				PARAM_RMPTR8,		0,					0				},
	{_T("???"),				0,				0,					0,					0				},
	{_T("???"),				0,				0,					0,					0				},
	{_T("???"),				0,				0,					0,					0				},
	{_T("???"),				0,				0,					0,					0				},
	{_T("???"),				0,				0,					0,					0				},
	{_T("???"),				0,				0,					0,					0				}
};

static const I386_OPCODE group2w_table[8] =
{
	{_T("inc"),				0,				PARAM_RMPTR16,		0,					0				},
	{_T("dec"),				0,				PARAM_RMPTR16,		0,					0				},
	{_T("call"),			0,				PARAM_RMPTR16,		0,					0,				DASMFLAG_STEP_OVER},
	{_T("call    far ptr "),0,				PARAM_RM16,			0,					0,				DASMFLAG_STEP_OVER},
	{_T("br"),				0,				PARAM_RMPTR16,		0,					0				},
	{_T("br      far ptr "),0,				PARAM_RM16,			0,					0				},
	{_T("push"),			0,				PARAM_RMPTR16,		0,					0				},
	{_T("???"),				0,				0,					0,					0				}
};

static const GROUP_OP group_op_table[] =
{
	{_T("immb"),				immb_table				},
	{_T("immw"),				immw_table				},
	{_T("immws"),				immws_table				},
	{_T("shiftbi"),			shiftbi_table			},
	{_T("shiftwi"),			shiftwi_table			},
	{_T("shiftb"),				shiftb_table			},
	{_T("shiftw"),				shiftw_table			},
	{_T("shiftbv"),			shiftbv_table			},
	{_T("shiftwv"),			shiftwv_table			},
	{_T("group1b"),			group1b_table			},
	{_T("group1w"),			group1w_table			},
	{_T("group2b"),			group2b_table			},
	{_T("group2w"),			group2w_table			}
};



static const _TCHAR *const nec_reg[8] = { _T("aw"), _T("cw"), _T("dw"), _T("bw"), _T("sp"), _T("bp"), _T("ix"), _T("iy") };
static const _TCHAR *const nec_reg8[8] = { _T("al"), _T("cl"), _T("dl"), _T("bl"), _T("ah"), _T("ch"), _T("dh"), _T("bh") };
static const _TCHAR *const nec_sreg[8] = { _T("ds1"), _T("ps"), _T("ss"), _T("ds0"), _T("???"), _T("???"), _T("???"), _T("???") };
static const _TCHAR *const nec_sfreg[256] =
{
	/* 0x00 */
	_T("p0"),	_T("pm0"),	_T("pmc0"),	_T("???"),	_T("???"),	_T("???"),	_T("???"),	_T("???"),
	_T("p1"),	_T("pm1"),	_T("pmc1"),	_T("???"),	_T("???"),	_T("???"),	_T("???"),	_T("???"),
	/* 0x10 */
	_T("p2"),	_T("pm2"),	_T("pmc2"),	_T("???"),	_T("???"),	_T("???"),	_T("???"),	_T("???"),
	_T("???"),	_T("???"),	_T("???"),	_T("???"),	_T("???"),	_T("???"),	_T("???"),	_T("???"),
	/* 0x20 */
	_T("???"),	_T("???"),	_T("???"),	_T("???"),	_T("???"),	_T("???"),	_T("???"),	_T("???"),
	_T("???"),	_T("???"),	_T("???"),	_T("???"),	_T("???"),	_T("???"),	_T("???"),	_T("???"),
	/* 0x30 */
	_T("???"),	_T("???"),	_T("???"),	_T("???"),	_T("???"),	_T("???"),	_T("???"),	_T("???"),
	_T("pt"),	_T("???"),	_T("???"),	_T("pmt"),	_T("???"),	_T("???"),	_T("???"),	_T("???"),
	/* 0x40 */
	_T("intm"),	_T("???"),	_T("???"),	_T("???"),	_T("ems0"),	_T("ems1"),	_T("ems2"),	_T("???"),
	_T("???"),	_T("???"),	_T("???"),	_T("???"),	_T("exic0"),	_T("exic1"),	_T("exic2"),	_T("???"),
	/* 0x50 */
	_T("???"),	_T("???"),	_T("???"),	_T("???"),	_T("???"),	_T("???"),	_T("???"),	_T("???"),
	_T("???"),	_T("???"),	_T("???"),	_T("???"),	_T("???"),	_T("???"),	_T("???"),	_T("???"),
	/* 0x60 */
	_T("rxb0"),	_T("???"),	_T("txb0"),	_T("???"),	_T("???"),	_T("srms0"),	_T("stms0"),	_T("???"),
	_T("scm0"),	_T("scc0"),	_T("brg0"),	_T("scs0"),	_T("seic0"),	_T("sric0"),	_T("stic0"),	_T("???"),
	/* 0x70 */
	_T("rxb1"),	_T("???"),	_T("txb1"),	_T("???"),	_T("???"),	_T("srms1"),	_T("stms1"),	_T("???"),
	_T("scm1"),	_T("scc1"),	_T("brg1"),	_T("scs1"),	_T("seic1"),	_T("sric1"),	_T("stic1"),	_T("???"),
	/* 0x80 */
	_T("tm0"),	_T("???"),	_T("md0"),	_T("???"),	_T("???"),	_T("???"),	_T("???"),	_T("???"),
	_T("tm1"),	_T("???"),	_T("md1"),	_T("???"),	_T("???"),	_T("???"),	_T("???"),	_T("???"),
	/* 0x90 */
	_T("tmc0"),	_T("tmc1"),	_T("???"),	_T("???"),	_T("tmms0"),	_T("tmms1"),	_T("tmms2"),	_T("???"),
	_T("???"),	_T("???"),	_T("???"),	_T("???"),	_T("tmic0"),	_T("tmic1"),	_T("tmic2"),	_T("???"),
	/* 0xa0 */
	_T("dmac0"),	_T("dmam0"),	_T("dmac1"),	_T("dmam1"),	_T("???"),	_T("???"),	_T("???"),	_T("???"),
	_T("???"),	_T("???"),	_T("???"),	_T("???"),	_T("dic0"),	_T("dic1"),	_T("???"),	_T("???"),
	/* 0xb0 */
	_T("???"),	_T("???"),	_T("???"),	_T("???"),	_T("???"),	_T("???"),	_T("???"),	_T("???"),
	_T("???"),	_T("???"),	_T("???"),	_T("???"),	_T("???"),	_T("???"),	_T("???"),	_T("???"),
	/* 0xc0 */
	_T("sar0l"),	_T("sar0m"),	_T("sar0h"),	_T("???"),	_T("dar0l"),	_T("dar0m"),	_T("dar0h"),	_T("???"),
	_T("tc0l"),	_T("tc0h"),	_T("???"),	_T("???"),	_T("???"),	_T("???"),	_T("???"),	_T("???"),
	/* 0xd0 */
	_T("sar1l"),	_T("sar1m"),	_T("sar1h"),	_T("???"),	_T("dar1l"),	_T("dar1m"),	_T("dar1h"),	_T("???"),
	_T("tc1l"),	_T("tc1h"),	_T("???"),	_T("???"),	_T("???"),	_T("???"),	_T("???"),	_T("???"),
	/* 0xe0 */
	_T("stbc"),	_T("rfm"),	_T("???"),	_T("???"),	_T("???"),	_T("???"),	_T("???"),	_T("???"),
	_T("wtc"),	_T("???"),	_T("flag"),	_T("prc"),	_T("tbic"),	_T("???"),	_T("???"),	_T("irqs"),
	/* 0xf0 */
	_T("???"),	_T("???"),	_T("???"),	_T("???"),	_T("???"),	_T("???"),	_T("???"),	_T("???"),
	_T("???"),	_T("???"),	_T("???"),	_T("???"),	_T("ispr"),	_T("???"),	_T("???"),	_T("idb")
};

static UINT32 pc;
static UINT8 modrm;
static UINT32 segment;
static offs_t dasm_flags;
static _TCHAR modrm_string[256];

#define MODRM_REG1	((modrm >> 3) & 0x7)
#define MODRM_REG2	(modrm & 0x7)

#define MAX_LENGTH	8

INLINE UINT8 FETCHD(void)
{
	if ((opcode_ptr - opcode_ptr_base) + 1 > MAX_LENGTH)
		return 0xff;
	pc++;
	return *opcode_ptr++;
}

INLINE UINT16 FETCHD16(void)
{
	UINT16 d;
	if ((opcode_ptr - opcode_ptr_base) + 2 > MAX_LENGTH)
		return 0xffff;
	d = opcode_ptr[0] | (opcode_ptr[1] << 8);
	opcode_ptr += 2;
	pc += 2;
	return d;
}

static _TCHAR *hexstring(UINT32 value, int digits)
{
	static _TCHAR buffer[20];
	buffer[0] = _T('0');
	if (digits) {
		_stprintf(&buffer[1], _T("%0*Xh"), digits, value);
	} else {
		_stprintf(&buffer[1], _T("%Xh"), value);
	}
	return (buffer[1] >= _T('0') && buffer[1] <= _T('9')) ? &buffer[1] : &buffer[0];
}

static _TCHAR *shexstring(UINT32 value, int digits, int always)
{
	static _TCHAR buffer[20];
	if (value >= 0x80000000) {
		_stprintf(buffer, _T("-%s"), hexstring(-value, digits));
	} else if (always) {
		_stprintf(buffer, _T("+%s"), hexstring(value, digits));
	} else {
		return hexstring(value, digits);
	}
	return buffer;
}

static void handle_modrm(_TCHAR* s)
{
	INT8 disp8;
	INT16 disp16;
	UINT8 mod, rm;

	modrm = FETCHD();
	mod = (modrm >> 6) & 0x3;
	rm = (modrm & 0x7);

	if( modrm >= 0xc0 )
		return;

	switch(segment)
	{
		case SEG_PS: s += _stprintf( s, _T("ps:") ); break;
		case SEG_DS0: s += _stprintf( s, _T("ds0:") ); break;
		case SEG_DS1: s += _stprintf( s, _T("ds1:") ); break;
		case SEG_SS: s += _stprintf( s, _T("ss:") ); break;
	}

	s += _stprintf( s, _T("[") );
	switch( rm )
	{
		case 0: s += _stprintf( s, _T("bw+ix") ); break;
		case 1: s += _stprintf( s, _T("bw+iy") ); break;
		case 2: s += _stprintf( s, _T("bp+ix") ); break;
		case 3: s += _stprintf( s, _T("bp+iy") ); break;
		case 4: s += _stprintf( s, _T("ix") ); break;
		case 5: s += _stprintf( s, _T("iy") ); break;
		case 6:
			if( mod == 0 ) {
				disp16 = FETCHD16();
				s += _stprintf( s, _T("%s"), hexstring((unsigned) (UINT16) disp16, 0) );
			} else {
				s += _stprintf( s, _T("bp") );
			}
			break;
		case 7: s += _stprintf( s, _T("bw") ); break;
	}
	if( mod == 1 ) {
		disp8 = FETCHD();
		s += _stprintf( s, _T("%s"), shexstring((INT32)disp8, 0, TRUE) );
	} else if( mod == 2 ) {
		disp16 = FETCHD16();
		s += _stprintf( s, _T("%s"), shexstring((INT32)disp16, 0, TRUE) );
	}
	s += _stprintf( s, _T("]") );
}

static _TCHAR* handle_param(_TCHAR* s, UINT32 param)
{
	UINT8 i8;
	UINT16 i16;
	UINT16 ptr;
	UINT32 addr;
	INT8 d8;
	INT16 d16;

	switch(param)
	{
		case PARAM_REG8:
			s += _stprintf( s, _T("%s"), nec_reg8[MODRM_REG1] );
			break;

		case PARAM_REG16:
			s += _stprintf( s, _T("%s"), nec_reg[MODRM_REG1] );
			break;

		case PARAM_REG2_8:
			s += _stprintf( s, _T("%s"), nec_reg8[MODRM_REG2] );
			break;

		case PARAM_REG2_16:
			s += _stprintf( s, _T("%s"), nec_reg[MODRM_REG2] );
			break;

		case PARAM_RM8:
		case PARAM_RMPTR8:
			if( modrm >= 0xc0 ) {
				s += _stprintf( s, _T("%s"), nec_reg8[MODRM_REG2] );
			} else {
				if (param == PARAM_RMPTR8)
					s += _stprintf( s, _T("byte ptr ") );
				s += _stprintf( s, _T("%s"), modrm_string );
			}
			break;

		case PARAM_RM16:
		case PARAM_RMPTR16:
			if( modrm >= 0xc0 ) {
				s += _stprintf( s, _T("%s"), nec_reg[MODRM_REG2] );
			} else {
				if (param == PARAM_RMPTR16)
					s += _stprintf( s, _T("word ptr ") );
				s += _stprintf( s, _T("%s"), modrm_string );
			}
			break;

		case PARAM_I3:
			i8 = FETCHD();
			s += _stprintf( s, _T("%d"), i8 & 0x07 );
			break;

		case PARAM_I4:
			i8 = FETCHD();
			s += _stprintf( s, _T("%d"), i8 & 0x0f );
			break;

		case PARAM_I8:
			i8 = FETCHD();
			s += _stprintf( s, _T("%s"), shexstring((INT8)i8, 0, FALSE) );
			break;

		case PARAM_I16:
			i16 = FETCHD16();
			s += _stprintf( s, _T("%s"), shexstring((INT16)i16, 0, FALSE) );
			break;

		case PARAM_UI8:
			i8 = FETCHD();
			s += _stprintf( s, _T("%s"), shexstring((UINT8)i8, 0, FALSE) );
			break;

		case PARAM_IMM:
			i16 = FETCHD16();
			s += _stprintf( s, _T("%s"), hexstring(i16, 0) );
			break;

		case PARAM_ADDR:
			addr = FETCHD16();
			ptr = FETCHD16();
			s += _stprintf( s, _T("%s:"), hexstring(ptr, 4) );
			s += _stprintf( s, _T("%s"), hexstring(addr, 0) );
			break;

		case PARAM_REL16:
			/* make sure to keep the relative offset within the segment */
			d16 = FETCHD16();
			s += _stprintf( s, _T("%s"), hexstring((pc & 0xFFFF0000) | ((pc + d16) & 0x0000FFFF), 0) );
			break;

		case PARAM_REL8:
			d8 = FETCHD();
			s += _stprintf( s, _T("%s"), hexstring(pc + d8, 0) );
			break;

		case PARAM_MEM_OFFS:
			switch(segment)
			{
				case SEG_PS: s += _stprintf( s, _T("ps:") ); break;
				case SEG_DS0: s += _stprintf( s, _T("ds0:") ); break;
				case SEG_DS1: s += _stprintf( s, _T("ds1:") ); break;
				case SEG_SS: s += _stprintf( s, _T("ss:") ); break;
			}

			i16 = FETCHD16();
			s += _stprintf( s, _T("[%s]"), hexstring(i16, 0) );
			break;

		case PARAM_SREG:
			s += _stprintf( s, _T("%s"), nec_sreg[MODRM_REG1] );
			break;

		case PARAM_SFREG:
			i8 = FETCHD();
			s += _stprintf( s, _T("%s"), nec_sfreg[i8] );
			break;

		case PARAM_1:
			s += _stprintf( s, _T("1") );
			break;

		case PARAM_AL: s += _stprintf( s, _T("al") ); break;
		case PARAM_CL: s += _stprintf( s, _T("cl") ); break;
		case PARAM_DL: s += _stprintf( s, _T("dl") ); break;
		case PARAM_BL: s += _stprintf( s, _T("bl") ); break;
		case PARAM_AH: s += _stprintf( s, _T("ah") ); break;
		case PARAM_CH: s += _stprintf( s, _T("ch") ); break;
		case PARAM_DH: s += _stprintf( s, _T("dh") ); break;
		case PARAM_BH: s += _stprintf( s, _T("bh") ); break;

		case PARAM_AW: s += _stprintf( s, _T("aw") ); break;
		case PARAM_CW: s += _stprintf( s, _T("cw") ); break;
		case PARAM_DW: s += _stprintf( s, _T("dw") ); break;
		case PARAM_BW: s += _stprintf( s, _T("bw") ); break;
		case PARAM_SP: s += _stprintf( s, _T("sp") ); break;
		case PARAM_BP: s += _stprintf( s, _T("bp") ); break;
		case PARAM_IX: s += _stprintf( s, _T("ix") ); break;
		case PARAM_IY: s += _stprintf( s, _T("iy") ); break;
	}
	return s;
}

static void handle_fpu(_TCHAR *s, UINT8 op1, UINT8 op2)
{
	switch (op1 & 0x7)
	{
		case 0:		// Group D8
		{
			if (op2 < 0xc0)
			{
				pc--;		// adjust fetch pointer, so modrm byte read again
				opcode_ptr--;
				handle_modrm( modrm_string );
				switch ((op2 >> 3) & 0x7)
				{
					case 0: _stprintf(s, _T("fadd    dword ptr %s"), modrm_string); break;
					case 1: _stprintf(s, _T("fmul    dword ptr %s"), modrm_string); break;
					case 2: _stprintf(s, _T("fcom    dword ptr %s"), modrm_string); break;
					case 3: _stprintf(s, _T("fcomp   dword ptr %s"), modrm_string); break;
					case 4: _stprintf(s, _T("fsub    dword ptr %s"), modrm_string); break;
					case 5: _stprintf(s, _T("fsubr   dword ptr %s"), modrm_string); break;
					case 6: _stprintf(s, _T("fdiv    dword ptr %s"), modrm_string); break;
					case 7: _stprintf(s, _T("fdivr   dword ptr %s"), modrm_string); break;
				}
			} else {
				switch ((op2 >> 3) & 0x7)
				{
					case 0: _stprintf(s, _T("fadd    st(0),st(%d)"), op2 & 0x7); break;
					case 1: _stprintf(s, _T("fcom    st(0),st(%d)"), op2 & 0x7); break;
					case 2: _stprintf(s, _T("fsub    st(0),st(%d)"), op2 & 0x7); break;
					case 3: _stprintf(s, _T("fdiv    st(0),st(%d)"), op2 & 0x7); break;
					case 4: _stprintf(s, _T("fmul    st(0),st(%d)"), op2 & 0x7); break;
					case 5: _stprintf(s, _T("fcomp   st(0),st(%d)"), op2 & 0x7); break;
					case 6: _stprintf(s, _T("fsubr   st(0),st(%d)"), op2 & 0x7); break;
					case 7: _stprintf(s, _T("fdivr   st(0),st(%d)"), op2 & 0x7); break;
				}
			}
			break;
		}

		case 1:		// Group D9
		{
			if (op2 < 0xc0)
			{
				pc--;		// adjust fetch pointer, so modrm byte read again
				opcode_ptr--;
				handle_modrm( modrm_string );
				switch ((op2 >> 3) & 0x7)
				{
					case 0: _stprintf(s, _T("fld     dword ptr %s"), modrm_string); break;
					case 1: _stprintf(s, _T("??? (FPU)")); break;
					case 2: _stprintf(s, _T("fst     dword ptr %s"), modrm_string); break;
					case 3: _stprintf(s, _T("fstp    dword ptr %s"), modrm_string); break;
					case 4: _stprintf(s, _T("fldenv  word ptr %s"), modrm_string); break;
					case 5: _stprintf(s, _T("fldcw   word ptr %s"), modrm_string); break;
					case 6: _stprintf(s, _T("fstenv  word ptr %s"), modrm_string); break;
					case 7: _stprintf(s, _T("fstcw   word ptr %s"), modrm_string); break;
				}
			} else {
				switch (op2 & 0x3f)
				{
					case 0x00: case 0x01: case 0x02: case 0x03: case 0x04: case 0x05: case 0x06: case 0x07:
						_stprintf(s, _T("fld     st(0),st(%d)"), op2 & 0x7); break;

					case 0x08: case 0x09: case 0x0a: case 0x0b: case 0x0c: case 0x0d: case 0x0e: case 0x0f:
						_stprintf(s, _T("fxch    st(0),st(%d)"), op2 & 0x7); break;

					case 0x10: _stprintf(s, _T("fnop")); break;
					case 0x20: _stprintf(s, _T("fchs")); break;
					case 0x21: _stprintf(s, _T("fabs")); break;
					case 0x24: _stprintf(s, _T("ftst")); break;
					case 0x25: _stprintf(s, _T("fxam")); break;
					case 0x28: _stprintf(s, _T("fld1")); break;
					case 0x29: _stprintf(s, _T("fldl2t")); break;
					case 0x2a: _stprintf(s, _T("fldl2e")); break;
					case 0x2b: _stprintf(s, _T("fldpi")); break;
					case 0x2c: _stprintf(s, _T("fldlg2")); break;
					case 0x2d: _stprintf(s, _T("fldln2")); break;
					case 0x2e: _stprintf(s, _T("fldz")); break;
					case 0x30: _stprintf(s, _T("f2xm1")); break;
					case 0x31: _stprintf(s, _T("fyl2x")); break;
					case 0x32: _stprintf(s, _T("fptan")); break;
					case 0x33: _stprintf(s, _T("fpatan")); break;
					case 0x34: _stprintf(s, _T("fxtract")); break;
					case 0x35: _stprintf(s, _T("fprem1")); break;
					case 0x36: _stprintf(s, _T("fdecstp")); break;
					case 0x37: _stprintf(s, _T("fincstp")); break;
					case 0x38: _stprintf(s, _T("fprem")); break;
					case 0x39: _stprintf(s, _T("fyl2xp1")); break;
					case 0x3a: _stprintf(s, _T("fsqrt")); break;
					case 0x3b: _stprintf(s, _T("fsincos")); break;
					case 0x3c: _stprintf(s, _T("frndint")); break;
					case 0x3d: _stprintf(s, _T("fscale")); break;
					case 0x3e: _stprintf(s, _T("fsin")); break;
					case 0x3f: _stprintf(s, _T("fcos")); break;

					default: _stprintf(s, _T("??? (FPU)")); break;
				}
			}
			break;
		}

		case 2:		// Group DA
		{
			if (op2 < 0xc0)
			{
				pc--;		// adjust fetch pointer, so modrm byte read again
				opcode_ptr--;
				handle_modrm( modrm_string );
				switch ((op2 >> 3) & 0x7)
				{
					case 0: _stprintf(s, _T("fiadd   dword ptr %s"), modrm_string); break;
					case 1: _stprintf(s, _T("fimul   dword ptr %s"), modrm_string); break;
					case 2: _stprintf(s, _T("ficom   dword ptr %s"), modrm_string); break;
					case 3: _stprintf(s, _T("ficomp  dword ptr %s"), modrm_string); break;
					case 4: _stprintf(s, _T("fisub   dword ptr %s"), modrm_string); break;
					case 5: _stprintf(s, _T("fisubr  dword ptr %s"), modrm_string); break;
					case 6: _stprintf(s, _T("fidiv   dword ptr %s"), modrm_string); break;
					case 7: _stprintf(s, _T("fidivr  dword ptr %s"), modrm_string); break;
				}
			} else {
				switch (op2 & 0x3f)
				{
					case 0x00: case 0x01: case 0x02: case 0x03: case 0x04: case 0x05: case 0x06: case 0x07:
						_stprintf(s, _T("fcmovb  st(0),st(%d)"), op2 & 0x7); break;

					case 0x08: case 0x09: case 0x0a: case 0x0b: case 0x0c: case 0x0d: case 0x0e: case 0x0f:
						_stprintf(s, _T("fcmove  st(0),st(%d)"), op2 & 0x7); break;

					case 0x10: case 0x11: case 0x12: case 0x13: case 0x14: case 0x15: case 0x16: case 0x17:
						_stprintf(s, _T("fcmovbe st(0),st(%d)"), op2 & 0x7); break;

					case 0x18: case 0x19: case 0x1a: case 0x1b: case 0x1c: case 0x1d: case 0x1e: case 0x1f:
						_stprintf(s, _T("fcmovu  st(0),st(%d)"), op2 & 0x7); break;

					default: _stprintf(s, _T("??? (FPU)")); break;

				}
			}
			break;
		}

		case 3:		// Group DB
		{
			if (op2 < 0xc0)
			{
				pc--;		// adjust fetch pointer, so modrm byte read again
				opcode_ptr--;
				handle_modrm( modrm_string );
				switch ((op2 >> 3) & 0x7)
				{
					case 0: _stprintf(s, _T("fild    dword ptr %s"), modrm_string); break;
					case 1: _stprintf(s, _T("??? (FPU)")); break;
					case 2: _stprintf(s, _T("fist    dword ptr %s"), modrm_string); break;
					case 3: _stprintf(s, _T("fistp   dword ptr %s"), modrm_string); break;
					case 4: _stprintf(s, _T("??? (FPU)")); break;
					case 5: _stprintf(s, _T("fld     tword ptr %s"), modrm_string); break;
					case 6: _stprintf(s, _T("??? (FPU)")); break;
					case 7: _stprintf(s, _T("fstp    tword ptr %s"), modrm_string); break;
				}
			} else {
				switch (op2 & 0x3f)
				{
					case 0x00: case 0x01: case 0x02: case 0x03: case 0x04: case 0x05: case 0x06: case 0x07:
						_stprintf(s, _T("fcmovnb st(0),st(%d)"), op2 & 0x7); break;

					case 0x08: case 0x09: case 0x0a: case 0x0b: case 0x0c: case 0x0d: case 0x0e: case 0x0f:
						_stprintf(s, _T("fcmovne st(0),st(%d)"), op2 & 0x7); break;

					case 0x10: case 0x11: case 0x12: case 0x13: case 0x14: case 0x15: case 0x16: case 0x17:
						_stprintf(s, _T("fcmovnbe st(0),st(%d)"), op2 & 0x7); break;

					case 0x18: case 0x19: case 0x1a: case 0x1b: case 0x1c: case 0x1d: case 0x1e: case 0x1f:
						_stprintf(s, _T("fcmovnu st(0),st(%d)"), op2 & 0x7); break;

					case 0x22: _stprintf(s, _T("fclex")); break;
					case 0x23: _stprintf(s, _T("finit")); break;

					case 0x28: case 0x29: case 0x2a: case 0x2b: case 0x2c: case 0x2d: case 0x2e: case 0x2f:
						_stprintf(s, _T("fucomi  st(0),st(%d)"), op2 & 0x7); break;

					case 0x30: case 0x31: case 0x32: case 0x33: case 0x34: case 0x35: case 0x36: case 0x37:
						_stprintf(s, _T("fcomi   st(0),st(%d)"), op2 & 0x7); break;

					default: _stprintf(s, _T("??? (FPU)")); break;
				}
			}
			break;
		}

		case 4:		// Group DC
		{
			if (op2 < 0xc0)
			{
				pc--;		// adjust fetch pointer, so modrm byte read again
				opcode_ptr--;
				handle_modrm( modrm_string );
				switch ((op2 >> 3) & 0x7)
				{
					case 0: _stprintf(s, _T("fadd    qword ptr %s"), modrm_string); break;
					case 1: _stprintf(s, _T("fmul    qword ptr %s"), modrm_string); break;
					case 2: _stprintf(s, _T("fcom    qword ptr %s"), modrm_string); break;
					case 3: _stprintf(s, _T("fcomp   qword ptr %s"), modrm_string); break;
					case 4: _stprintf(s, _T("fsub    qword ptr %s"), modrm_string); break;
					case 5: _stprintf(s, _T("fsubr   qword ptr %s"), modrm_string); break;
					case 6: _stprintf(s, _T("fdiv    qword ptr %s"), modrm_string); break;
					case 7: _stprintf(s, _T("fdivr   qword ptr %s"), modrm_string); break;
				}
			} else {
				switch (op2 & 0x3f)
				{
					case 0x00: case 0x01: case 0x02: case 0x03: case 0x04: case 0x05: case 0x06: case 0x07:
						_stprintf(s, _T("fadd    st(%d),st(0)"), op2 & 0x7); break;

					case 0x08: case 0x09: case 0x0a: case 0x0b: case 0x0c: case 0x0d: case 0x0e: case 0x0f:
						_stprintf(s, _T("fmul    st(%d),st(0)"), op2 & 0x7); break;

					case 0x20: case 0x21: case 0x22: case 0x23: case 0x24: case 0x25: case 0x26: case 0x27:
						_stprintf(s, _T("fsubr   st(%d),st(0)"), op2 & 0x7); break;

					case 0x28: case 0x29: case 0x2a: case 0x2b: case 0x2c: case 0x2d: case 0x2e: case 0x2f:
						_stprintf(s, _T("fsub    st(%d),st(0)"), op2 & 0x7); break;

					case 0x30: case 0x31: case 0x32: case 0x33: case 0x34: case 0x35: case 0x36: case 0x37:
						_stprintf(s, _T("fdivr   st(%d),st(0)"), op2 & 0x7); break;

					case 0x38: case 0x39: case 0x3a: case 0x3b: case 0x3c: case 0x3d: case 0x3e: case 0x3f:
						_stprintf(s, _T("fdiv    st(%d),st(0)"), op2 & 0x7); break;

					default: _stprintf(s, _T("??? (FPU)")); break;
				}
			}
			break;
		}

		case 5:		// Group DD
		{
			if (op2 < 0xc0)
			{
				pc--;		// adjust fetch pointer, so modrm byte read again
				opcode_ptr--;
				handle_modrm( modrm_string );
				switch ((op2 >> 3) & 0x7)
				{
					case 0: _stprintf(s, _T("fld     qword ptr %s"), modrm_string); break;
					case 1: _stprintf(s, _T("??? (FPU)")); break;
					case 2: _stprintf(s, _T("fst     qword ptr %s"), modrm_string); break;
					case 3: _stprintf(s, _T("fstp    qword ptr %s"), modrm_string); break;
					case 4: _stprintf(s, _T("frstor  %s"), modrm_string); break;
					case 5: _stprintf(s, _T("??? (FPU)")); break;
					case 6: _stprintf(s, _T("fsave   %s"), modrm_string); break;
					case 7: _stprintf(s, _T("fstsw   word ptr %s"), modrm_string); break;
				}
			} else {
				switch (op2 & 0x3f)
				{
					case 0x00: case 0x01: case 0x02: case 0x03: case 0x04: case 0x05: case 0x06: case 0x07:
						_stprintf(s, _T("ffree   st(%d)"), op2 & 0x7); break;

					case 0x10: case 0x11: case 0x12: case 0x13: case 0x14: case 0x15: case 0x16: case 0x17:
						_stprintf(s, _T("fst     st(%d)"), op2 & 0x7); break;

					case 0x18: case 0x19: case 0x1a: case 0x1b: case 0x1c: case 0x1d: case 0x1e: case 0x1f:
						_stprintf(s, _T("fstp    st(%d)"), op2 & 0x7); break;

					case 0x20: case 0x21: case 0x22: case 0x23: case 0x24: case 0x25: case 0x26: case 0x27:
						_stprintf(s, _T("fucom   st(%d), st(0)"), op2 & 0x7); break;

					case 0x28: case 0x29: case 0x2a: case 0x2b: case 0x2c: case 0x2d: case 0x2e: case 0x2f:
						_stprintf(s, _T("fucomp  st(%d)"), op2 & 0x7); break;

					default: _stprintf(s, _T("??? (FPU)")); break;
				}
			}
			break;
		}

		case 6:		// Group DE
		{
			if (op2 < 0xc0)
			{
				pc--;		// adjust fetch pointer, so modrm byte read again
				opcode_ptr--;
				handle_modrm( modrm_string );
				switch ((op2 >> 3) & 0x7)
				{
					case 0: _stprintf(s, _T("fiadd   word ptr %s"), modrm_string); break;
					case 1: _stprintf(s, _T("fimul   word ptr %s"), modrm_string); break;
					case 2: _stprintf(s, _T("ficom   word ptr %s"), modrm_string); break;
					case 3: _stprintf(s, _T("ficomp  word ptr %s"), modrm_string); break;
					case 4: _stprintf(s, _T("fisub   word ptr %s"), modrm_string); break;
					case 5: _stprintf(s, _T("fisubr  word ptr %s"), modrm_string); break;
					case 6: _stprintf(s, _T("fidiv   word ptr %s"), modrm_string); break;
					case 7: _stprintf(s, _T("fidivr  word ptr %s"), modrm_string); break;
				}
			} else {
				switch (op2 & 0x3f)
				{
					case 0x00: case 0x01: case 0x02: case 0x03: case 0x04: case 0x05: case 0x06: case 0x07:
						_stprintf(s, _T("faddp   st(%d)"), op2 & 0x7); break;

					case 0x08: case 0x09: case 0x0a: case 0x0b: case 0x0c: case 0x0d: case 0x0e: case 0x0f:
						_stprintf(s, _T("fmulp   st(%d)"), op2 & 0x7); break;

					case 0x19: _stprintf(s, _T("fcompp")); break;

					case 0x20: case 0x21: case 0x22: case 0x23: case 0x24: case 0x25: case 0x26: case 0x27:
						_stprintf(s, _T("fsubrp  st(%d)"), op2 & 0x7); break;

					case 0x28: case 0x29: case 0x2a: case 0x2b: case 0x2c: case 0x2d: case 0x2e: case 0x2f:
						_stprintf(s, _T("fsubp   st(%d)"), op2 & 0x7); break;

					case 0x30: case 0x31: case 0x32: case 0x33: case 0x34: case 0x35: case 0x36: case 0x37:
						_stprintf(s, _T("fdivrp  st(%d), st(0)"), op2 & 0x7); break;

					case 0x38: case 0x39: case 0x3a: case 0x3b: case 0x3c: case 0x3d: case 0x3e: case 0x3f:
						_stprintf(s, _T("fdivp   st(%d)"), op2 & 0x7); break;

					default: _stprintf(s, _T("??? (FPU)")); break;
				}
			}
			break;
		}

		case 7:		// Group DF
		{
			if (op2 < 0xc0)
			{
				pc--;		// adjust fetch pointer, so modrm byte read again
				opcode_ptr--;
				handle_modrm( modrm_string );
				switch ((op2 >> 3) & 0x7)
				{
					case 0: _stprintf(s, _T("fild    word ptr %s"), modrm_string); break;
					case 1: _stprintf(s, _T("??? (FPU)")); break;
					case 2: _stprintf(s, _T("fist    word ptr %s"), modrm_string); break;
					case 3: _stprintf(s, _T("fistp   word ptr %s"), modrm_string); break;
					case 4: _stprintf(s, _T("fbld    %s"), modrm_string); break;
					case 5: _stprintf(s, _T("fild    qword ptr %s"), modrm_string); break;
					case 6: _stprintf(s, _T("fbstp   %s"), modrm_string); break;
					case 7: _stprintf(s, _T("fistp   qword ptr %s"), modrm_string); break;
				}
			} else {
				switch (op2 & 0x3f)
				{
					case 0x20: _stprintf(s, _T("fstsw   aw")); break;

					case 0x28: case 0x29: case 0x2a: case 0x2b: case 0x2c: case 0x2d: case 0x2e: case 0x2f:
						_stprintf(s, _T("fucomip st(%d)"), op2 & 0x7); break;

					case 0x30: case 0x31: case 0x32: case 0x33: case 0x34: case 0x35: case 0x36: case 0x37:
						_stprintf(s, _T("fcomip  st(%d),st(0)"), op2 & 0x7); break;

					default: _stprintf(s, _T("??? (FPU)")); break;
				}
			}
			break;
		}
	}
}

static void decode_opcode(_TCHAR *s, const I386_OPCODE *op, UINT8 op1 )
{
	int i;
	UINT8 op2;

	switch( op->flags )
	{
		case TWO_BYTE:
			op2 = FETCHD();
			decode_opcode( s, &necv_opcode_table2[op2], op1 );
			return;

		case SEG_PS:
		case SEG_DS0:
		case SEG_DS1:
		case SEG_SS:
			segment = op->flags;
			op2 = FETCHD();
			decode_opcode( s, &necv_opcode_table1[op2], op1 );
			return;

		case PREFIX:
			s += _stprintf( s, _T("%-8s"), op->mnemonic );
			op2 = FETCHD();
			decode_opcode( s, &necv_opcode_table1[op2], op1 );
			return;

		case GROUP:
			handle_modrm( modrm_string );
			for( i=0; i < ARRAY_LENGTH(group_op_table); i++ ) {
				if( _tcscmp(op->mnemonic, group_op_table[i].mnemonic) == 0 )
				{
					decode_opcode( s, &group_op_table[i].opcode[MODRM_REG1], op1 );
					return;
				}
			}
			goto handle_unknown;

		case FPU:
			op2 = FETCHD();
			handle_fpu( s, op1, op2);
			return;

		case MODRM:
			handle_modrm( modrm_string );
			break;
	}

	s += _stprintf( s, _T("%-8s"), op->mnemonic );
	dasm_flags = op->dasm_flags;

	if( op->param1 != 0 ) {
		s = handle_param( s, op->param1 );
	}

	if( op->param2 != 0 ) {
		s += _stprintf( s, _T(",") );
		s = handle_param( s, op->param2 );
	}

	if( op->param3 != 0 ) {
		s += _stprintf( s, _T(",") );
		s = handle_param( s, op->param3 );
	}
	return;

handle_unknown:
	_stprintf(s, _T("???"));
}

int necv_dasm_one(_TCHAR *buffer, UINT32 eip, const UINT8 *oprom)
{
	UINT8 op;

	opcode_ptr = opcode_ptr_base = oprom;
	pc = eip;
	dasm_flags = 0;
	segment = 0;

	op = FETCHD();

	decode_opcode( buffer, &necv_opcode_table1[op], op );
	return (pc-eip) | dasm_flags | DASMFLAG_SUPPORTED;
}

#define STATE_VERSION	1

void I86_BASE::save_state(FILEIO* state_fio)
{
	state_fio->FputUint32(STATE_VERSION);
	state_fio->FputInt32(this_device_id);
	
	state_fio->Fwrite(&regs, sizeof(regs), 1);
	state_fio->FputUint32(pc);
	state_fio->FputUint32(prevpc);
	state_fio->Fwrite(base, sizeof(base), 1);
	state_fio->Fwrite(sregs, sizeof(sregs), 1);
	state_fio->FputUint16(flags);
	state_fio->FputInt32(AuxVal);
	state_fio->FputInt32(OverVal);
	state_fio->FputInt32(SignVal);
	state_fio->FputInt32(ZeroVal);
	state_fio->FputInt32(CarryVal);
	state_fio->FputInt32(DirVal);
	state_fio->FputUint8(ParityVal);
	state_fio->FputUint8(TF);
	state_fio->FputUint8(IF);
	state_fio->FputUint8(MF);
	state_fio->FputInt32(int_state);
	state_fio->FputBool(test_state);
	state_fio->FputBool(busreq);
	state_fio->FputBool(halted);
	state_fio->FputInt32(icount);
	state_fio->FputInt32(extra_icount);
	state_fio->FputBool(seg_prefix);
	state_fio->FputUint8(prefix_seg);
	state_fio->FputUint32(ea);
	state_fio->FputUint16(eo);
	state_fio->FputUint8(ea_seg);
}

bool I86_BASE::load_state(FILEIO* state_fio)
{
	if(state_fio->FgetUint32() != STATE_VERSION) {
		return false;
	}
	if(state_fio->FgetInt32() != this_device_id) {
		return false;
	}
	state_fio->Fread(&regs, sizeof(regs), 1);
	pc = state_fio->FgetUint32();
	prevpc = state_fio->FgetUint32();
	state_fio->Fread(base, sizeof(base), 1);
	state_fio->Fread(sregs, sizeof(sregs), 1);
	flags = state_fio->FgetUint16();
	AuxVal = state_fio->FgetInt32();
	OverVal = state_fio->FgetInt32();
	SignVal = state_fio->FgetInt32();
	ZeroVal = state_fio->FgetInt32();
	CarryVal = state_fio->FgetInt32();
	DirVal = state_fio->FgetInt32();
	ParityVal = state_fio->FgetUint8();
	TF = state_fio->FgetUint8();
	IF = state_fio->FgetUint8();
	MF = state_fio->FgetUint8();
	int_state = state_fio->FgetInt32();
	test_state = state_fio->FgetBool();
	busreq = state_fio->FgetBool();
	halted = state_fio->FgetBool();
	icount = state_fio->FgetInt32();
	extra_icount = state_fio->FgetInt32();
	seg_prefix = state_fio->FgetBool();
	prefix_seg = state_fio->FgetUint8();
	ea = state_fio->FgetUint32();
	eo = state_fio->FgetUint16();
	ea_seg = state_fio->FgetUint8();
	return true;
}

