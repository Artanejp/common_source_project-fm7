/*
	Skelton for retropc emulator

	Origin : MAME
	Author : Takeda.Toshiya
	Date   : 2008.11.04 -

	[ i8080 / i8085 ]
*/

#include "i8080_base.h"
#include "i8080_regdef.h"
#include "debugger.h"

void I8080_BASE::initialize()
{
	DEVICE::initialize();
}

void I8080_BASE::reset()
{
	// reset
	AF = BC = DE = HL = 0;
	PC = 0;
	SP = 0;
	IM = IM_M5 | IM_M6 | IM_M7;
	HALT = BUSREQ = false;
	
	count = 0;
}

void I8080_BASE::write_signal(int id, uint32_t data, uint32_t mask)
{
	// Dummy Function.
}

uint32_t I8080_BASE::read_signal(int ch)
{
	if(ch == SIG_I8080_INTE) {
		if(!afterEI && (IM & IM_IEN)) {
			return 0xffffffff;
		}
	}
	return 0;
}

void I8080_BASE::set_intr_line(bool line, bool pending, uint32_t bit)
{
	if(line) {
		IM |= IM_INT;
	} else {
		IM &= ~IM_INT;
	}
}

int I8080_BASE::run(int clock)
{
	// Dummy Function
	return 0;
}

//#ifdef USE_DEBUGGER
void I8080_BASE::write_debug_data8(uint32_t addr, uint32_t data)
{
	int wait;
	d_mem_stored->write_data8w(addr, data, &wait);
}

uint32_t I8080_BASE::read_debug_data8(uint32_t addr)
{
	int wait;
	return d_mem_stored->read_data8w(addr, &wait);
}

void I8080_BASE::write_debug_io8(uint32_t addr, uint32_t data)
{
	int wait;
	d_io_stored->write_io8w(addr, data, &wait);
}

uint32_t I8080_BASE::read_debug_io8(uint32_t addr)
{
	int wait;
	return d_io_stored->read_io8w(addr, &wait);
}

bool I8080_BASE::write_debug_reg(const _TCHAR *reg, uint32_t data)
{
	if(_tcsicmp(reg, _T("PC")) == 0) {
		PC = data;
	} else if(_tcsicmp(reg, _T("SP")) == 0) {
		SP = data;
	} else if(_tcsicmp(reg, _T("AF")) == 0) {
		AF = data;
	} else if(_tcsicmp(reg, _T("BC")) == 0) {
		BC = data;
	} else if(_tcsicmp(reg, _T("DE")) == 0) {
		DE = data;
	} else if(_tcsicmp(reg, _T("HL")) == 0) {
		HL = data;
	} else if(_tcsicmp(reg, _T("A")) == 0) {
		_A = data;
	} else if(_tcsicmp(reg, _T("F")) == 0) {
		_F = data;
	} else if(_tcsicmp(reg, _T("B")) == 0) {
		_B = data;
	} else if(_tcsicmp(reg, _T("C")) == 0) {
		_C = data;
	} else if(_tcsicmp(reg, _T("D")) == 0) {
		_D = data;
	} else if(_tcsicmp(reg, _T("E")) == 0) {
		_E = data;
	} else if(_tcsicmp(reg, _T("H")) == 0) {
		_H = data;
	} else if(_tcsicmp(reg, _T("L")) == 0) {
		_L = data;
	} else {
		return false;
	}
	return true;
}

void I8080_BASE::get_debug_regs_info(_TCHAR *buffer, size_t buffer_len)
{
/*
F = [--------]  A = 00  BC = 0000  DE = 0000  HL = 0000  SP = 0000  PC = 0000
IM= [--------]         (BC)= 0000 (DE)= 0000 (HL)= 0000 (SP)= 0000
*/
	int wait;
	my_stprintf_s(buffer, buffer_len,
	_T("F = [%c%c%c%c%c%c%c%c]  A = %02X  BC = %04X  DE = %04X  HL = %04X  SP = %04X  PC = %04X\nIM= [%c%c%c%c%c%c%c%c]         (BC)= %04X (DE)= %04X (HL)= %04X (SP)= %04X"),
	(_F & CF) ? _T('C') : _T('-'), (_F & NF) ? _T('N') : _T('-'), (_F & VF) ? _T('V') : _T('-'), (_F & XF) ? _T('X') : _T('-'),
	(_F & HF) ? _T('H') : _T('-'), (_F & YF) ? _T('Y') : _T('-'), (_F & ZF) ? _T('Z') : _T('-'), (_F & SF) ? _T('S') : _T('-'),
	_A, BC, DE, HL, SP, PC,
	(IM & 0x80) ? _T('S') : _T('-'), (IM & 0x40) ? _T('7') : _T('-'), (IM & 0x20) ? _T('6') : _T('-'), (IM & 0x10) ? _T('5') : _T('-'),
	(IM & 0x08) ? _T('E') : _T('-'), (IM & 0x04) ? _T('7') : _T('-'), (IM & 0x02) ? _T('6') : _T('-'), (IM & 0x01) ? _T('5') : _T('-'),
	d_mem_stored->read_data16w(BC, &wait), d_mem_stored->read_data16w(DE, &wait), d_mem_stored->read_data16w(HL, &wait), d_mem_stored->read_data16w(SP, &wait));
}

// disassembler

int I8080_BASE::debug_dasm(uint32_t pc, _TCHAR *buffer, size_t buffer_len)
{
	uint8_t ops[4];
	int ptr = 0;
	
	for(int i = 0; i < 4; i++) {
		int wait;
		ops[i] = d_mem_stored->read_data8w(pc + i, &wait);
	}
	switch(ops[ptr++])
	{
		case 0x00: my_stprintf_s(buffer, buffer_len, _T("nop")); break;
		case 0x01: my_stprintf_s(buffer, buffer_len, _T("lxi  b,%s"), get_value_or_symbol(d_debugger->first_symbol, _T("$%04x"), ops[ptr] | (ops[ptr + 1] << 8))); ptr += 2; break;
		case 0x02: my_stprintf_s(buffer, buffer_len, _T("stax b")); break;
		case 0x03: my_stprintf_s(buffer, buffer_len, _T("inx  b")); break;
		case 0x04: my_stprintf_s(buffer, buffer_len, _T("inr  b")); break;
		case 0x05: my_stprintf_s(buffer, buffer_len, _T("dcr  b")); break;
		case 0x06: my_stprintf_s(buffer, buffer_len, _T("mvi  b,$%02x"), ops[ptr++]); break;
		case 0x07: my_stprintf_s(buffer, buffer_len, _T("rlc")); break;
		case 0x08: my_stprintf_s(buffer, buffer_len, _T("dsub (*)")); break;
		case 0x09: my_stprintf_s(buffer, buffer_len, _T("dad  b")); break;
		case 0x0a: my_stprintf_s(buffer, buffer_len, _T("ldax b")); break;
		case 0x0b: my_stprintf_s(buffer, buffer_len, _T("dcx  b")); break;
		case 0x0c: my_stprintf_s(buffer, buffer_len, _T("inr  c")); break;
		case 0x0d: my_stprintf_s(buffer, buffer_len, _T("dcr  c")); break;
		case 0x0e: my_stprintf_s(buffer, buffer_len, _T("mvi  c,$%02x"), ops[ptr++]); break;
		case 0x0f: my_stprintf_s(buffer, buffer_len, _T("rrc")); break;
		case 0x10: my_stprintf_s(buffer, buffer_len, _T("asrh (*)")); break;
		case 0x11: my_stprintf_s(buffer, buffer_len, _T("lxi  d,%s"), get_value_or_symbol(d_debugger->first_symbol, _T("$%04x"), ops[ptr] | (ops[ptr + 1] << 8))); ptr += 2; break;
		case 0x12: my_stprintf_s(buffer, buffer_len, _T("stax d")); break;
		case 0x13: my_stprintf_s(buffer, buffer_len, _T("inx  d")); break;
		case 0x14: my_stprintf_s(buffer, buffer_len, _T("inr  d")); break;
		case 0x15: my_stprintf_s(buffer, buffer_len, _T("dcr  d")); break;
		case 0x16: my_stprintf_s(buffer, buffer_len, _T("mvi  d,$%02x"), ops[ptr++]); break;
		case 0x17: my_stprintf_s(buffer, buffer_len, _T("ral")); break;
		case 0x18: my_stprintf_s(buffer, buffer_len, _T("rlde (*)")); break;
		case 0x19: my_stprintf_s(buffer, buffer_len, _T("dad  d")); break;
		case 0x1a: my_stprintf_s(buffer, buffer_len, _T("ldax d")); break;
		case 0x1b: my_stprintf_s(buffer, buffer_len, _T("dcx  d")); break;
		case 0x1c: my_stprintf_s(buffer, buffer_len, _T("inr  e")); break;
		case 0x1d: my_stprintf_s(buffer, buffer_len, _T("dcr  e")); break;
		case 0x1e: my_stprintf_s(buffer, buffer_len, _T("mvi  e,$%02x"), ops[ptr++]); break;
		case 0x1f: my_stprintf_s(buffer, buffer_len, _T("rar")); break;
		case 0x20: my_stprintf_s(buffer, buffer_len, _T("rim")); break;
		case 0x21: my_stprintf_s(buffer, buffer_len, _T("lxi  h,%s"), get_value_or_symbol(d_debugger->first_symbol, _T("$%04x"), ops[ptr] | (ops[ptr + 1] << 8))); ptr += 2; break;
		case 0x22: my_stprintf_s(buffer, buffer_len, _T("shld %s"), get_value_or_symbol(d_debugger->first_symbol, _T("$%04x"), ops[ptr] | (ops[ptr + 1] << 8))); ptr += 2; break;
		case 0x23: my_stprintf_s(buffer, buffer_len, _T("inx  h")); break;
		case 0x24: my_stprintf_s(buffer, buffer_len, _T("inr  h")); break;
		case 0x25: my_stprintf_s(buffer, buffer_len, _T("dcr  h")); break;
		case 0x26: my_stprintf_s(buffer, buffer_len, _T("mvi  h,$%02x"), ops[ptr++]); break;
		case 0x27: my_stprintf_s(buffer, buffer_len, _T("daa")); break;
		case 0x28: my_stprintf_s(buffer, buffer_len, _T("ldeh $%02x (*)"), ops[ptr++]); break;
		case 0x29: my_stprintf_s(buffer, buffer_len, _T("dad  h")); break;
		case 0x2a: my_stprintf_s(buffer, buffer_len, _T("lhld %s"), get_value_or_symbol(d_debugger->first_symbol, _T("$%04x"), ops[ptr] | (ops[ptr + 1] << 8))); ptr += 2; break;
		case 0x2b: my_stprintf_s(buffer, buffer_len, _T("dcx  h")); break;
		case 0x2c: my_stprintf_s(buffer, buffer_len, _T("inr  l")); break;
		case 0x2d: my_stprintf_s(buffer, buffer_len, _T("dcr  l")); break;
		case 0x2e: my_stprintf_s(buffer, buffer_len, _T("mvi  l,$%02x"), ops[ptr++]); break;
		case 0x2f: my_stprintf_s(buffer, buffer_len, _T("cma")); break;
		case 0x30: my_stprintf_s(buffer, buffer_len, _T("sim")); break;
		case 0x31: my_stprintf_s(buffer, buffer_len, _T("lxi  sp,%s"), get_value_or_symbol(d_debugger->first_symbol, _T("$%04x"), ops[ptr] | (ops[ptr + 1] << 8))); ptr += 2; break;
		case 0x32: my_stprintf_s(buffer, buffer_len, _T("stax %s"), get_value_or_symbol(d_debugger->first_symbol, _T("$%04x"), ops[ptr] | (ops[ptr + 1] << 8))); ptr += 2; break;
		case 0x33: my_stprintf_s(buffer, buffer_len, _T("inx  sp")); break;
		case 0x34: my_stprintf_s(buffer, buffer_len, _T("inr  m")); break;
		case 0x35: my_stprintf_s(buffer, buffer_len, _T("dcr  m")); break;
		case 0x36: my_stprintf_s(buffer, buffer_len, _T("mvi  m,$%02x"), ops[ptr++]); break;
		case 0x37: my_stprintf_s(buffer, buffer_len, _T("stc")); break;
		case 0x38: my_stprintf_s(buffer, buffer_len, _T("ldes $%02x"), ops[ptr++]); break;
		case 0x39: my_stprintf_s(buffer, buffer_len, _T("dad sp")); break;
		case 0x3a: my_stprintf_s(buffer, buffer_len, _T("ldax %s"), get_value_or_symbol(d_debugger->first_symbol, _T("$%04x"), ops[ptr] | (ops[ptr + 1] << 8))); ptr += 2; break;
		case 0x3b: my_stprintf_s(buffer, buffer_len, _T("dcx  sp")); break;
		case 0x3c: my_stprintf_s(buffer, buffer_len, _T("inr  a")); break;
		case 0x3d: my_stprintf_s(buffer, buffer_len, _T("dcr  a")); break;
		case 0x3e: my_stprintf_s(buffer, buffer_len, _T("mvi  a,$%02x"), ops[ptr++]); break;
		case 0x3f: my_stprintf_s(buffer, buffer_len, _T("cmf")); break;
		case 0x40: my_stprintf_s(buffer, buffer_len, _T("mov  b,b")); break;
		case 0x41: my_stprintf_s(buffer, buffer_len, _T("mov  b,c")); break;
		case 0x42: my_stprintf_s(buffer, buffer_len, _T("mov  b,d")); break;
		case 0x43: my_stprintf_s(buffer, buffer_len, _T("mov  b,e")); break;
		case 0x44: my_stprintf_s(buffer, buffer_len, _T("mov  b,h")); break;
		case 0x45: my_stprintf_s(buffer, buffer_len, _T("mov  b,l")); break;
		case 0x46: my_stprintf_s(buffer, buffer_len, _T("mov  b,m")); break;
		case 0x47: my_stprintf_s(buffer, buffer_len, _T("mov  b,a")); break;
		case 0x48: my_stprintf_s(buffer, buffer_len, _T("mov  c,b")); break;
		case 0x49: my_stprintf_s(buffer, buffer_len, _T("mov  c,c")); break;
		case 0x4a: my_stprintf_s(buffer, buffer_len, _T("mov  c,d")); break;
		case 0x4b: my_stprintf_s(buffer, buffer_len, _T("mov  c,e")); break;
		case 0x4c: my_stprintf_s(buffer, buffer_len, _T("mov  c,h")); break;
		case 0x4d: my_stprintf_s(buffer, buffer_len, _T("mov  c,l")); break;
		case 0x4e: my_stprintf_s(buffer, buffer_len, _T("mov  c,m")); break;
		case 0x4f: my_stprintf_s(buffer, buffer_len, _T("mov  c,a")); break;
		case 0x50: my_stprintf_s(buffer, buffer_len, _T("mov  d,b")); break;
		case 0x51: my_stprintf_s(buffer, buffer_len, _T("mov  d,c")); break;
		case 0x52: my_stprintf_s(buffer, buffer_len, _T("mov  d,d")); break;
		case 0x53: my_stprintf_s(buffer, buffer_len, _T("mov  d,e")); break;
		case 0x54: my_stprintf_s(buffer, buffer_len, _T("mov  d,h")); break;
		case 0x55: my_stprintf_s(buffer, buffer_len, _T("mov  d,l")); break;
		case 0x56: my_stprintf_s(buffer, buffer_len, _T("mov  d,m")); break;
		case 0x57: my_stprintf_s(buffer, buffer_len, _T("mov  d,a")); break;
		case 0x58: my_stprintf_s(buffer, buffer_len, _T("mov  e,b")); break;
		case 0x59: my_stprintf_s(buffer, buffer_len, _T("mov  e,c")); break;
		case 0x5a: my_stprintf_s(buffer, buffer_len, _T("mov  e,d")); break;
		case 0x5b: my_stprintf_s(buffer, buffer_len, _T("mov  e,e")); break;
		case 0x5c: my_stprintf_s(buffer, buffer_len, _T("mov  e,h")); break;
		case 0x5d: my_stprintf_s(buffer, buffer_len, _T("mov  e,l")); break;
		case 0x5e: my_stprintf_s(buffer, buffer_len, _T("mov  e,m")); break;
		case 0x5f: my_stprintf_s(buffer, buffer_len, _T("mov  e,a")); break;
		case 0x60: my_stprintf_s(buffer, buffer_len, _T("mov  h,b")); break;
		case 0x61: my_stprintf_s(buffer, buffer_len, _T("mov  h,c")); break;
		case 0x62: my_stprintf_s(buffer, buffer_len, _T("mov  h,d")); break;
		case 0x63: my_stprintf_s(buffer, buffer_len, _T("mov  h,e")); break;
		case 0x64: my_stprintf_s(buffer, buffer_len, _T("mov  h,h")); break;
		case 0x65: my_stprintf_s(buffer, buffer_len, _T("mov  h,l")); break;
		case 0x66: my_stprintf_s(buffer, buffer_len, _T("mov  h,m")); break;
		case 0x67: my_stprintf_s(buffer, buffer_len, _T("mov  h,a")); break;
		case 0x68: my_stprintf_s(buffer, buffer_len, _T("mov  l,b")); break;
		case 0x69: my_stprintf_s(buffer, buffer_len, _T("mov  l,c")); break;
		case 0x6a: my_stprintf_s(buffer, buffer_len, _T("mov  l,d")); break;
		case 0x6b: my_stprintf_s(buffer, buffer_len, _T("mov  l,e")); break;
		case 0x6c: my_stprintf_s(buffer, buffer_len, _T("mov  l,h")); break;
		case 0x6d: my_stprintf_s(buffer, buffer_len, _T("mov  l,l")); break;
		case 0x6e: my_stprintf_s(buffer, buffer_len, _T("mov  l,m")); break;
		case 0x6f: my_stprintf_s(buffer, buffer_len, _T("mov  l,a")); break;
		case 0x70: my_stprintf_s(buffer, buffer_len, _T("mov  m,b")); break;
		case 0x71: my_stprintf_s(buffer, buffer_len, _T("mov  m,c")); break;
		case 0x72: my_stprintf_s(buffer, buffer_len, _T("mov  m,d")); break;
		case 0x73: my_stprintf_s(buffer, buffer_len, _T("mov  m,e")); break;
		case 0x74: my_stprintf_s(buffer, buffer_len, _T("mov  m,h")); break;
		case 0x75: my_stprintf_s(buffer, buffer_len, _T("mov  m,l")); break;
		case 0x76: my_stprintf_s(buffer, buffer_len, _T("hlt")); break;
		case 0x77: my_stprintf_s(buffer, buffer_len, _T("mov  m,a")); break;
		case 0x78: my_stprintf_s(buffer, buffer_len, _T("mov  a,b")); break;
		case 0x79: my_stprintf_s(buffer, buffer_len, _T("mov  a,c")); break;
		case 0x7a: my_stprintf_s(buffer, buffer_len, _T("mov  a,d")); break;
		case 0x7b: my_stprintf_s(buffer, buffer_len, _T("mov  a,e")); break;
		case 0x7c: my_stprintf_s(buffer, buffer_len, _T("mov  a,h")); break;
		case 0x7d: my_stprintf_s(buffer, buffer_len, _T("mov  a,l")); break;
		case 0x7e: my_stprintf_s(buffer, buffer_len, _T("mov  a,m")); break;
		case 0x7f: my_stprintf_s(buffer, buffer_len, _T("mov  a,a")); break;
		case 0x80: my_stprintf_s(buffer, buffer_len, _T("add  b")); break;
		case 0x81: my_stprintf_s(buffer, buffer_len, _T("add  c")); break;
		case 0x82: my_stprintf_s(buffer, buffer_len, _T("add  d")); break;
		case 0x83: my_stprintf_s(buffer, buffer_len, _T("add  e")); break;
		case 0x84: my_stprintf_s(buffer, buffer_len, _T("add  h")); break;
		case 0x85: my_stprintf_s(buffer, buffer_len, _T("add  l")); break;
		case 0x86: my_stprintf_s(buffer, buffer_len, _T("add  m")); break;
		case 0x87: my_stprintf_s(buffer, buffer_len, _T("add  a")); break;
		case 0x88: my_stprintf_s(buffer, buffer_len, _T("adc  b")); break;
		case 0x89: my_stprintf_s(buffer, buffer_len, _T("adc  c")); break;
		case 0x8a: my_stprintf_s(buffer, buffer_len, _T("adc  d")); break;
		case 0x8b: my_stprintf_s(buffer, buffer_len, _T("adc  e")); break;
		case 0x8c: my_stprintf_s(buffer, buffer_len, _T("adc  h")); break;
		case 0x8d: my_stprintf_s(buffer, buffer_len, _T("adc  l")); break;
		case 0x8e: my_stprintf_s(buffer, buffer_len, _T("adc  m")); break;
		case 0x8f: my_stprintf_s(buffer, buffer_len, _T("adc  a")); break;
		case 0x90: my_stprintf_s(buffer, buffer_len, _T("sub  b")); break;
		case 0x91: my_stprintf_s(buffer, buffer_len, _T("sub  c")); break;
		case 0x92: my_stprintf_s(buffer, buffer_len, _T("sub  d")); break;
		case 0x93: my_stprintf_s(buffer, buffer_len, _T("sub  e")); break;
		case 0x94: my_stprintf_s(buffer, buffer_len, _T("sub  h")); break;
		case 0x95: my_stprintf_s(buffer, buffer_len, _T("sub  l")); break;
		case 0x96: my_stprintf_s(buffer, buffer_len, _T("sub  m")); break;
		case 0x97: my_stprintf_s(buffer, buffer_len, _T("sub  a")); break;
		case 0x98: my_stprintf_s(buffer, buffer_len, _T("sbb  b")); break;
		case 0x99: my_stprintf_s(buffer, buffer_len, _T("sbb  c")); break;
		case 0x9a: my_stprintf_s(buffer, buffer_len, _T("sbb  d")); break;
		case 0x9b: my_stprintf_s(buffer, buffer_len, _T("sbb  e")); break;
		case 0x9c: my_stprintf_s(buffer, buffer_len, _T("sbb  h")); break;
		case 0x9d: my_stprintf_s(buffer, buffer_len, _T("sbb  l")); break;
		case 0x9e: my_stprintf_s(buffer, buffer_len, _T("sbb  m")); break;
		case 0x9f: my_stprintf_s(buffer, buffer_len, _T("sbb  a")); break;
		case 0xa0: my_stprintf_s(buffer, buffer_len, _T("ana  b")); break;
		case 0xa1: my_stprintf_s(buffer, buffer_len, _T("ana  c")); break;
		case 0xa2: my_stprintf_s(buffer, buffer_len, _T("ana  d")); break;
		case 0xa3: my_stprintf_s(buffer, buffer_len, _T("ana  e")); break;
		case 0xa4: my_stprintf_s(buffer, buffer_len, _T("ana  h")); break;
		case 0xa5: my_stprintf_s(buffer, buffer_len, _T("ana  l")); break;
		case 0xa6: my_stprintf_s(buffer, buffer_len, _T("ana  m")); break;
		case 0xa7: my_stprintf_s(buffer, buffer_len, _T("ana  a")); break;
		case 0xa8: my_stprintf_s(buffer, buffer_len, _T("xra  b")); break;
		case 0xa9: my_stprintf_s(buffer, buffer_len, _T("xra  c")); break;
		case 0xaa: my_stprintf_s(buffer, buffer_len, _T("xra  d")); break;
		case 0xab: my_stprintf_s(buffer, buffer_len, _T("xra  e")); break;
		case 0xac: my_stprintf_s(buffer, buffer_len, _T("xra  h")); break;
		case 0xad: my_stprintf_s(buffer, buffer_len, _T("xra  l")); break;
		case 0xae: my_stprintf_s(buffer, buffer_len, _T("xra  m")); break;
		case 0xaf: my_stprintf_s(buffer, buffer_len, _T("xra  a")); break;
		case 0xb0: my_stprintf_s(buffer, buffer_len, _T("ora  b")); break;
		case 0xb1: my_stprintf_s(buffer, buffer_len, _T("ora  c")); break;
		case 0xb2: my_stprintf_s(buffer, buffer_len, _T("ora  d")); break;
		case 0xb3: my_stprintf_s(buffer, buffer_len, _T("ora  e")); break;
		case 0xb4: my_stprintf_s(buffer, buffer_len, _T("ora  h")); break;
		case 0xb5: my_stprintf_s(buffer, buffer_len, _T("ora  l")); break;
		case 0xb6: my_stprintf_s(buffer, buffer_len, _T("ora  m")); break;
		case 0xb7: my_stprintf_s(buffer, buffer_len, _T("ora  a")); break;
		case 0xb8: my_stprintf_s(buffer, buffer_len, _T("cmp  b")); break;
		case 0xb9: my_stprintf_s(buffer, buffer_len, _T("cmp  c")); break;
		case 0xba: my_stprintf_s(buffer, buffer_len, _T("cmp  d")); break;
		case 0xbb: my_stprintf_s(buffer, buffer_len, _T("cmp  e")); break;
		case 0xbc: my_stprintf_s(buffer, buffer_len, _T("cmp  h")); break;
		case 0xbd: my_stprintf_s(buffer, buffer_len, _T("cmp  l")); break;
		case 0xbe: my_stprintf_s(buffer, buffer_len, _T("cmp  m")); break;
		case 0xbf: my_stprintf_s(buffer, buffer_len, _T("cmp  a")); break;
		case 0xc0: my_stprintf_s(buffer, buffer_len, _T("rnz")); break;
		case 0xc1: my_stprintf_s(buffer, buffer_len, _T("pop  b")); break;
		case 0xc2: my_stprintf_s(buffer, buffer_len, _T("jnz  %s"), get_value_or_symbol(d_debugger->first_symbol, _T("$%04x"), ops[ptr] | (ops[ptr + 1] << 8))); ptr += 2; break;
		case 0xc3: my_stprintf_s(buffer, buffer_len, _T("jmp  %s"), get_value_or_symbol(d_debugger->first_symbol, _T("$%04x"), ops[ptr] | (ops[ptr + 1] << 8))); ptr += 2; break;
		case 0xc4: my_stprintf_s(buffer, buffer_len, _T("cnz  %s"), get_value_or_symbol(d_debugger->first_symbol, _T("$%04x"), ops[ptr] | (ops[ptr + 1] << 8))); ptr += 2; break;
		case 0xc5: my_stprintf_s(buffer, buffer_len, _T("push b")); break;
		case 0xc6: my_stprintf_s(buffer, buffer_len, _T("adi  $%02x"), ops[ptr++]); break;
		case 0xc7: my_stprintf_s(buffer, buffer_len, _T("rst  0")); break;
		case 0xc8: my_stprintf_s(buffer, buffer_len, _T("rz")); break;
		case 0xc9: my_stprintf_s(buffer, buffer_len, _T("ret")); break;
		case 0xca: my_stprintf_s(buffer, buffer_len, _T("jz   %s"), get_value_or_symbol(d_debugger->first_symbol, _T("$%04x"), ops[ptr] | (ops[ptr + 1] << 8))); ptr += 2; break;
		case 0xcb: my_stprintf_s(buffer, buffer_len, _T("rstv 8 (*)")); break;
		case 0xcc: my_stprintf_s(buffer, buffer_len, _T("cz   %s"), get_value_or_symbol(d_debugger->first_symbol, _T("$%04x"), ops[ptr] | (ops[ptr + 1] << 8))); ptr += 2; break;
		case 0xcd: my_stprintf_s(buffer, buffer_len, _T("call %s"), get_value_or_symbol(d_debugger->first_symbol, _T("$%04x"), ops[ptr] | (ops[ptr + 1] << 8))); ptr += 2; break;
		case 0xce: my_stprintf_s(buffer, buffer_len, _T("aci  $%02x"), ops[ptr++]); break;
		case 0xcf: my_stprintf_s(buffer, buffer_len, _T("rst  1")); break;
		case 0xd0: my_stprintf_s(buffer, buffer_len, _T("rnc")); break;
		case 0xd1: my_stprintf_s(buffer, buffer_len, _T("pop  d")); break;
		case 0xd2: my_stprintf_s(buffer, buffer_len, _T("jnc  %s"), get_value_or_symbol(d_debugger->first_symbol, _T("$%04x"), ops[ptr] | (ops[ptr + 1] << 8))); ptr += 2; break;
		case 0xd3: my_stprintf_s(buffer, buffer_len, _T("out  $%02x"), ops[ptr++]); break;
		case 0xd4: my_stprintf_s(buffer, buffer_len, _T("cnc  %s"), get_value_or_symbol(d_debugger->first_symbol, _T("$%04x"), ops[ptr] | (ops[ptr + 1] << 8))); ptr += 2; break;
		case 0xd5: my_stprintf_s(buffer, buffer_len, _T("push d")); break;
		case 0xd6: my_stprintf_s(buffer, buffer_len, _T("sui  $%02x"), ops[ptr++]); break;
		case 0xd7: my_stprintf_s(buffer, buffer_len, _T("rst  2")); break;
		case 0xd8: my_stprintf_s(buffer, buffer_len, _T("rc")); break;
		case 0xd9: my_stprintf_s(buffer, buffer_len, _T("shlx d (*)")); break;
		case 0xda: my_stprintf_s(buffer, buffer_len, _T("jc   %s"), get_value_or_symbol(d_debugger->first_symbol, _T("$%04x"), ops[ptr] | (ops[ptr + 1] << 8))); ptr += 2; break;
		case 0xdb: my_stprintf_s(buffer, buffer_len, _T("in   $%02x"), ops[ptr++]); break;
		case 0xdc: my_stprintf_s(buffer, buffer_len, _T("cc   %s"), get_value_or_symbol(d_debugger->first_symbol, _T("$%04x"), ops[ptr] | (ops[ptr + 1] << 8))); ptr += 2; break;
		case 0xdd: my_stprintf_s(buffer, buffer_len, _T("jnx  %s (*)"), get_value_or_symbol(d_debugger->first_symbol, _T("$%04x"), ops[ptr] | (ops[ptr + 1] << 8))); ptr += 2; break;
		case 0xde: my_stprintf_s(buffer, buffer_len, _T("sbi  $%02x"), ops[ptr++]); break;
		case 0xdf: my_stprintf_s(buffer, buffer_len, _T("rst  3")); break;
		case 0xe0: my_stprintf_s(buffer, buffer_len, _T("rpo")); break;
		case 0xe1: my_stprintf_s(buffer, buffer_len, _T("pop  h")); break;
		case 0xe2: my_stprintf_s(buffer, buffer_len, _T("jpo  %s"), get_value_or_symbol(d_debugger->first_symbol, _T("$%04x"), ops[ptr] | (ops[ptr + 1] << 8))); ptr += 2; break;
		case 0xe3: my_stprintf_s(buffer, buffer_len, _T("xthl")); break;
		case 0xe4: my_stprintf_s(buffer, buffer_len, _T("cpo  %s"), get_value_or_symbol(d_debugger->first_symbol, _T("$%04x"), ops[ptr] | (ops[ptr + 1] << 8))); ptr += 2; break;
		case 0xe5: my_stprintf_s(buffer, buffer_len, _T("push h")); break;
		case 0xe6: my_stprintf_s(buffer, buffer_len, _T("ani  $%02x"), ops[ptr++]); break;
		case 0xe7: my_stprintf_s(buffer, buffer_len, _T("rst  4")); break;
		case 0xe8: my_stprintf_s(buffer, buffer_len, _T("rpe")); break;
		case 0xe9: my_stprintf_s(buffer, buffer_len, _T("PChl")); break;
		case 0xea: my_stprintf_s(buffer, buffer_len, _T("jpe  %s"), get_value_or_symbol(d_debugger->first_symbol, _T("$%04x"), ops[ptr] | (ops[ptr + 1] << 8))); ptr += 2; break;
		case 0xeb: my_stprintf_s(buffer, buffer_len, _T("xchg")); break;
		case 0xec: my_stprintf_s(buffer, buffer_len, _T("cpe  %s"), get_value_or_symbol(d_debugger->first_symbol, _T("$%04x"), ops[ptr] | (ops[ptr + 1] << 8))); ptr += 2; break;
		case 0xed: my_stprintf_s(buffer, buffer_len, _T("lhlx d (*)")); break;
		case 0xee: my_stprintf_s(buffer, buffer_len, _T("xri  $%02x"), ops[ptr++]); break;
		case 0xef: my_stprintf_s(buffer, buffer_len, _T("rst  5")); break;
		case 0xf0: my_stprintf_s(buffer, buffer_len, _T("rp")); break;
		case 0xf1: my_stprintf_s(buffer, buffer_len, _T("pop  a")); break;
		case 0xf2: my_stprintf_s(buffer, buffer_len, _T("jp   %s"), get_value_or_symbol(d_debugger->first_symbol, _T("$%04x"), ops[ptr] | (ops[ptr + 1] << 8))); ptr += 2; break;
		case 0xf3: my_stprintf_s(buffer, buffer_len, _T("di")); break;
		case 0xf4: my_stprintf_s(buffer, buffer_len, _T("cp   %s"), get_value_or_symbol(d_debugger->first_symbol, _T("$%04x"), ops[ptr] | (ops[ptr + 1] << 8))); ptr += 2; break;
		case 0xf5: my_stprintf_s(buffer, buffer_len, _T("push a")); break;
		case 0xf6: my_stprintf_s(buffer, buffer_len, _T("ori  $%02x"), ops[ptr++]); break;
		case 0xf7: my_stprintf_s(buffer, buffer_len, _T("rst  6")); break;
		case 0xf8: my_stprintf_s(buffer, buffer_len, _T("rm")); break;
		case 0xf9: my_stprintf_s(buffer, buffer_len, _T("sphl")); break;
		case 0xfa: my_stprintf_s(buffer, buffer_len, _T("jm   %s"), get_value_or_symbol(d_debugger->first_symbol, _T("$%04x"), ops[ptr] | (ops[ptr + 1] << 8))); ptr += 2; break;
		case 0xfb: my_stprintf_s(buffer, buffer_len, _T("ei")); break;
		case 0xfc: my_stprintf_s(buffer, buffer_len, _T("cm   %s"), get_value_or_symbol(d_debugger->first_symbol, _T("$%04x"), ops[ptr] | (ops[ptr + 1] << 8))); ptr += 2; break;
		case 0xfd: my_stprintf_s(buffer, buffer_len, _T("jx   %s (*)"), get_value_or_symbol(d_debugger->first_symbol, _T("$%04x"), ops[ptr] | (ops[ptr + 1] << 8))); ptr += 2; break;
		case 0xfe: my_stprintf_s(buffer, buffer_len, _T("cpi  $%02x"), ops[ptr++]); break;
		case 0xff: my_stprintf_s(buffer, buffer_len, _T("rst  7")); break;
	}
	return ptr;
}
//#endif
