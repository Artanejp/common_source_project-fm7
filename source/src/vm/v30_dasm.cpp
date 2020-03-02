/*
	Skelton for retropc emulator

	Origin : MAME V30 core
	Author : Takeda.Toshiya
	Date   : 2020.02.02-

	[ V30 disassembler ]
*/

#include "i386_dasm.h"
#include "debugger.h"

#if defined(_MSC_VER) && (_MSC_VER >= 1400)
#pragma warning( disable : 4146 )
#endif

/*****************************************************************************/
/* src/emu/devcpu.h */

// CPU interface functions
#define CPU_DISASSEMBLE_NAME(name)		cpu_disassemble_##name
#define CPU_DISASSEMBLE(name)			int CPU_DISASSEMBLE_NAME(name)(_TCHAR *buffer, offs_t eip, const UINT8 *oprom)
#define CPU_DISASSEMBLE_CALL(name)		CPU_DISASSEMBLE_NAME(name)(buffer, eip, oprom)

/*****************************************************************************/
/* src/emu/didisasm.h */

// Disassembler constants
const UINT32 DASMFLAG_SUPPORTED     = 0x80000000;   // are disassembly flags supported?
const UINT32 DASMFLAG_STEP_OUT      = 0x40000000;   // this instruction should be the end of a step out sequence
const UINT32 DASMFLAG_STEP_OVER     = 0x20000000;   // this instruction should be stepped over by setting a breakpoint afterwards
const UINT32 DASMFLAG_OVERINSTMASK  = 0x18000000;   // number of extra instructions to skip when stepping over
const UINT32 DASMFLAG_OVERINSTSHIFT = 27;           // bits to shift after masking to get the value
const UINT32 DASMFLAG_LENGTHMASK    = 0x0000ffff;   // the low 16-bits contain the actual length

// offsets and addresses are 32-bit (for now...)
typedef UINT32	offs_t;

/*****************************************************************************/
/* src/osd/osdcomm.h */

/* Highly useful macro for compile-time knowledge of an array size */
#define ARRAY_LENGTH(x)     (sizeof(x) / sizeof(x[0]))

#ifndef INLINE
#define INLINE inline
#endif

#include "mame/emu/cpu/nec/necdasm.c"

int v30_dasm(DEBUGGER *debugger, uint8_t *oprom, uint32_t eip, bool emulation_mode, _TCHAR *buffer, size_t buffer_len)
{
	if(!emulation_mode) {
		return CPU_DISASSEMBLE_CALL(nec_generic) & DASMFLAG_LENGTHMASK;
	}
	int ptr = 0;
	
	switch(oprom[ptr++]) {
		case 0x00: my_stprintf_s(buffer, buffer_len, _T("nop")); break;
		case 0x01: my_stprintf_s(buffer, buffer_len, _T("lxi  b,%s"), get_value_or_symbol(debugger->first_symbol, _T("$%04x"), oprom[ptr] | (oprom[ptr + 1] << 8))); ptr += 2; break;
		case 0x02: my_stprintf_s(buffer, buffer_len, _T("stax b")); break;
		case 0x03: my_stprintf_s(buffer, buffer_len, _T("inx  b")); break;
		case 0x04: my_stprintf_s(buffer, buffer_len, _T("inr  b")); break;
		case 0x05: my_stprintf_s(buffer, buffer_len, _T("dcr  b")); break;
		case 0x06: my_stprintf_s(buffer, buffer_len, _T("mvi  b,$%02x"), oprom[ptr++]); break;
		case 0x07: my_stprintf_s(buffer, buffer_len, _T("rlc")); break;
		case 0x08: my_stprintf_s(buffer, buffer_len, _T("nop")); break;
		case 0x09: my_stprintf_s(buffer, buffer_len, _T("dad  b")); break;
		case 0x0a: my_stprintf_s(buffer, buffer_len, _T("ldax b")); break;
		case 0x0b: my_stprintf_s(buffer, buffer_len, _T("dcx  b")); break;
		case 0x0c: my_stprintf_s(buffer, buffer_len, _T("inr  c")); break;
		case 0x0d: my_stprintf_s(buffer, buffer_len, _T("dcr  c")); break;
		case 0x0e: my_stprintf_s(buffer, buffer_len, _T("mvi  c,$%02x"), oprom[ptr++]); break;
		case 0x0f: my_stprintf_s(buffer, buffer_len, _T("rrc")); break;
		case 0x10: my_stprintf_s(buffer, buffer_len, _T("nop")); break;
		case 0x11: my_stprintf_s(buffer, buffer_len, _T("lxi  d,%s"), get_value_or_symbol(debugger->first_symbol, _T("$%04x"), oprom[ptr] | (oprom[ptr + 1] << 8))); ptr += 2; break;
		case 0x12: my_stprintf_s(buffer, buffer_len, _T("stax d")); break;
		case 0x13: my_stprintf_s(buffer, buffer_len, _T("inx  d")); break;
		case 0x14: my_stprintf_s(buffer, buffer_len, _T("inr  d")); break;
		case 0x15: my_stprintf_s(buffer, buffer_len, _T("dcr  d")); break;
		case 0x16: my_stprintf_s(buffer, buffer_len, _T("mvi  d,$%02x"), oprom[ptr++]); break;
		case 0x17: my_stprintf_s(buffer, buffer_len, _T("ral")); break;
		case 0x18: my_stprintf_s(buffer, buffer_len, _T("nop")); break;
		case 0x19: my_stprintf_s(buffer, buffer_len, _T("dad  d")); break;
		case 0x1a: my_stprintf_s(buffer, buffer_len, _T("ldax d")); break;
		case 0x1b: my_stprintf_s(buffer, buffer_len, _T("dcx  d")); break;
		case 0x1c: my_stprintf_s(buffer, buffer_len, _T("inr  e")); break;
		case 0x1d: my_stprintf_s(buffer, buffer_len, _T("dcr  e")); break;
		case 0x1e: my_stprintf_s(buffer, buffer_len, _T("mvi  e,$%02x"), oprom[ptr++]); break;
		case 0x1f: my_stprintf_s(buffer, buffer_len, _T("rar")); break;
		case 0x20: my_stprintf_s(buffer, buffer_len, _T("rim")); break;
		case 0x21: my_stprintf_s(buffer, buffer_len, _T("lxi  h,%s"), get_value_or_symbol(debugger->first_symbol, _T("$%04x"), oprom[ptr] | (oprom[ptr + 1] << 8))); ptr += 2; break;
		case 0x22: my_stprintf_s(buffer, buffer_len, _T("shld %s"), get_value_or_symbol(debugger->first_symbol, _T("$%04x"), oprom[ptr] | (oprom[ptr + 1] << 8))); ptr += 2; break;
		case 0x23: my_stprintf_s(buffer, buffer_len, _T("inx  h")); break;
		case 0x24: my_stprintf_s(buffer, buffer_len, _T("inr  h")); break;
		case 0x25: my_stprintf_s(buffer, buffer_len, _T("dcr  h")); break;
		case 0x26: my_stprintf_s(buffer, buffer_len, _T("mvi  h,$%02x"), oprom[ptr++]); break;
		case 0x27: my_stprintf_s(buffer, buffer_len, _T("daa")); break;
		case 0x28: my_stprintf_s(buffer, buffer_len, _T("nop")); break;
		case 0x29: my_stprintf_s(buffer, buffer_len, _T("dad  h")); break;
		case 0x2a: my_stprintf_s(buffer, buffer_len, _T("lhld %s"), get_value_or_symbol(debugger->first_symbol, _T("$%04x"), oprom[ptr] | (oprom[ptr + 1] << 8))); ptr += 2; break;
		case 0x2b: my_stprintf_s(buffer, buffer_len, _T("dcx  h")); break;
		case 0x2c: my_stprintf_s(buffer, buffer_len, _T("inr  l")); break;
		case 0x2d: my_stprintf_s(buffer, buffer_len, _T("dcr  l")); break;
		case 0x2e: my_stprintf_s(buffer, buffer_len, _T("mvi  l,$%02x"), oprom[ptr++]); break;
		case 0x2f: my_stprintf_s(buffer, buffer_len, _T("cma")); break;
		case 0x30: my_stprintf_s(buffer, buffer_len, _T("sim")); break;
		case 0x31: my_stprintf_s(buffer, buffer_len, _T("lxi  sp,%s"), get_value_or_symbol(debugger->first_symbol, _T("$%04x"), oprom[ptr] | (oprom[ptr + 1] << 8))); ptr += 2; break;
		case 0x32: my_stprintf_s(buffer, buffer_len, _T("stax %s"), get_value_or_symbol(debugger->first_symbol, _T("$%04x"), oprom[ptr] | (oprom[ptr + 1] << 8))); ptr += 2; break;
		case 0x33: my_stprintf_s(buffer, buffer_len, _T("inx  sp")); break;
		case 0x34: my_stprintf_s(buffer, buffer_len, _T("inr  m")); break;
		case 0x35: my_stprintf_s(buffer, buffer_len, _T("dcr  m")); break;
		case 0x36: my_stprintf_s(buffer, buffer_len, _T("mvi  m,$%02x"), oprom[ptr++]); break;
		case 0x37: my_stprintf_s(buffer, buffer_len, _T("stc")); break;
		case 0x38: my_stprintf_s(buffer, buffer_len, _T("ldes $%02x"), oprom[ptr++]); break;
		case 0x39: my_stprintf_s(buffer, buffer_len, _T("dad sp")); break;
		case 0x3a: my_stprintf_s(buffer, buffer_len, _T("ldax %s"), get_value_or_symbol(debugger->first_symbol, _T("$%04x"), oprom[ptr] | (oprom[ptr + 1] << 8))); ptr += 2; break;
		case 0x3b: my_stprintf_s(buffer, buffer_len, _T("dcx  sp")); break;
		case 0x3c: my_stprintf_s(buffer, buffer_len, _T("inr  a")); break;
		case 0x3d: my_stprintf_s(buffer, buffer_len, _T("dcr  a")); break;
		case 0x3e: my_stprintf_s(buffer, buffer_len, _T("mvi  a,$%02x"), oprom[ptr++]); break;
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
		case 0xc2: my_stprintf_s(buffer, buffer_len, _T("jnz  %s"), get_value_or_symbol(debugger->first_symbol, _T("$%04x"), oprom[ptr] | (oprom[ptr + 1] << 8))); ptr += 2; break;
		case 0xc3: my_stprintf_s(buffer, buffer_len, _T("jmp  %s"), get_value_or_symbol(debugger->first_symbol, _T("$%04x"), oprom[ptr] | (oprom[ptr + 1] << 8))); ptr += 2; break;
		case 0xc4: my_stprintf_s(buffer, buffer_len, _T("cnz  %s"), get_value_or_symbol(debugger->first_symbol, _T("$%04x"), oprom[ptr] | (oprom[ptr + 1] << 8))); ptr += 2; break;
		case 0xc5: my_stprintf_s(buffer, buffer_len, _T("push b")); break;
		case 0xc6: my_stprintf_s(buffer, buffer_len, _T("adi  $%02x"), oprom[ptr++]); break;
		case 0xc7: my_stprintf_s(buffer, buffer_len, _T("rst  0")); break;
		case 0xc8: my_stprintf_s(buffer, buffer_len, _T("rz")); break;
		case 0xc9: my_stprintf_s(buffer, buffer_len, _T("ret")); break;
		case 0xca: my_stprintf_s(buffer, buffer_len, _T("jz   %s"), get_value_or_symbol(debugger->first_symbol, _T("$%04x"), oprom[ptr] | (oprom[ptr + 1] << 8))); ptr += 2; break;
		case 0xcb: my_stprintf_s(buffer, buffer_len, _T("jmp  %s"), get_value_or_symbol(debugger->first_symbol, _T("$%04x"), oprom[ptr] | (oprom[ptr + 1] << 8))); ptr += 2; break;
		case 0xcc: my_stprintf_s(buffer, buffer_len, _T("cz   %s"), get_value_or_symbol(debugger->first_symbol, _T("$%04x"), oprom[ptr] | (oprom[ptr + 1] << 8))); ptr += 2; break;
		case 0xcd: my_stprintf_s(buffer, buffer_len, _T("call %s"), get_value_or_symbol(debugger->first_symbol, _T("$%04x"), oprom[ptr] | (oprom[ptr + 1] << 8))); ptr += 2; break;
		case 0xce: my_stprintf_s(buffer, buffer_len, _T("aci  $%02x"), oprom[ptr++]); break;
		case 0xcf: my_stprintf_s(buffer, buffer_len, _T("rst  1")); break;
		case 0xd0: my_stprintf_s(buffer, buffer_len, _T("rnc")); break;
		case 0xd1: my_stprintf_s(buffer, buffer_len, _T("pop  d")); break;
		case 0xd2: my_stprintf_s(buffer, buffer_len, _T("jnc  %s"), get_value_or_symbol(debugger->first_symbol, _T("$%04x"), oprom[ptr] | (oprom[ptr + 1] << 8))); ptr += 2; break;
		case 0xd3: my_stprintf_s(buffer, buffer_len, _T("out  $%02x"), oprom[ptr++]); break;
		case 0xd4: my_stprintf_s(buffer, buffer_len, _T("cnc  %s"), get_value_or_symbol(debugger->first_symbol, _T("$%04x"), oprom[ptr] | (oprom[ptr + 1] << 8))); ptr += 2; break;
		case 0xd5: my_stprintf_s(buffer, buffer_len, _T("push d")); break;
		case 0xd6: my_stprintf_s(buffer, buffer_len, _T("sui  $%02x"), oprom[ptr++]); break;
		case 0xd7: my_stprintf_s(buffer, buffer_len, _T("rst  2")); break;
		case 0xd8: my_stprintf_s(buffer, buffer_len, _T("rc")); break;
		case 0xd9: my_stprintf_s(buffer, buffer_len, _T("ret")); break;
		case 0xda: my_stprintf_s(buffer, buffer_len, _T("jc   %s"), get_value_or_symbol(debugger->first_symbol, _T("$%04x"), oprom[ptr] | (oprom[ptr + 1] << 8))); ptr += 2; break;
		case 0xdb: my_stprintf_s(buffer, buffer_len, _T("in   $%02x"), oprom[ptr++]); break;
		case 0xdc: my_stprintf_s(buffer, buffer_len, _T("cc   %s"), get_value_or_symbol(debugger->first_symbol, _T("$%04x"), oprom[ptr] | (oprom[ptr + 1] << 8))); ptr += 2; break;
		case 0xdd: my_stprintf_s(buffer, buffer_len, _T("call %s"), get_value_or_symbol(debugger->first_symbol, _T("$%04x"), oprom[ptr] | (oprom[ptr + 1] << 8))); ptr += 2; break;
		case 0xde: my_stprintf_s(buffer, buffer_len, _T("sbi  $%02x"), oprom[ptr++]); break;
		case 0xdf: my_stprintf_s(buffer, buffer_len, _T("rst  3")); break;
		case 0xe0: my_stprintf_s(buffer, buffer_len, _T("rpo")); break;
		case 0xe1: my_stprintf_s(buffer, buffer_len, _T("pop  h")); break;
		case 0xe2: my_stprintf_s(buffer, buffer_len, _T("jpo  %s"), get_value_or_symbol(debugger->first_symbol, _T("$%04x"), oprom[ptr] | (oprom[ptr + 1] << 8))); ptr += 2; break;
		case 0xe3: my_stprintf_s(buffer, buffer_len, _T("xthl")); break;
		case 0xe4: my_stprintf_s(buffer, buffer_len, _T("cpo  %s"), get_value_or_symbol(debugger->first_symbol, _T("$%04x"), oprom[ptr] | (oprom[ptr + 1] << 8))); ptr += 2; break;
		case 0xe5: my_stprintf_s(buffer, buffer_len, _T("push h")); break;
		case 0xe6: my_stprintf_s(buffer, buffer_len, _T("ani  $%02x"), oprom[ptr++]); break;
		case 0xe7: my_stprintf_s(buffer, buffer_len, _T("rst  4")); break;
		case 0xe8: my_stprintf_s(buffer, buffer_len, _T("rpe")); break;
		case 0xe9: my_stprintf_s(buffer, buffer_len, _T("PChl")); break;
		case 0xea: my_stprintf_s(buffer, buffer_len, _T("jpe  %s"), get_value_or_symbol(debugger->first_symbol, _T("$%04x"), oprom[ptr] | (oprom[ptr + 1] << 8))); ptr += 2; break;
		case 0xeb: my_stprintf_s(buffer, buffer_len, _T("xchg")); break;
		case 0xec: my_stprintf_s(buffer, buffer_len, _T("cpe  %s"), get_value_or_symbol(debugger->first_symbol, _T("$%04x"), oprom[ptr] | (oprom[ptr + 1] << 8))); ptr += 2; break;
		case 0xed: 
			if(oprom[ptr] == 0xed) {
				my_stprintf_s(buffer, buffer_len, _T("calln $%02x"), oprom[ptr + 1]);
			} else if(oprom[ptr] == 0xfd) {
				my_stprintf_s(buffer, buffer_len, _T("retem"));
			} else {
				my_stprintf_s(buffer, buffer_len, _T("call %s"), get_value_or_symbol(debugger->first_symbol, _T("$%04x"), oprom[ptr] | (oprom[ptr + 1] << 8)));
			}
			 ptr += 2;
			 break;
		case 0xee: my_stprintf_s(buffer, buffer_len, _T("xri  $%02x"), oprom[ptr++]); break;
		case 0xef: my_stprintf_s(buffer, buffer_len, _T("rst  5")); break;
		case 0xf0: my_stprintf_s(buffer, buffer_len, _T("rp")); break;
		case 0xf1: my_stprintf_s(buffer, buffer_len, _T("pop  a")); break;
		case 0xf2: my_stprintf_s(buffer, buffer_len, _T("jp   %s"), get_value_or_symbol(debugger->first_symbol, _T("$%04x"), oprom[ptr] | (oprom[ptr + 1] << 8))); ptr += 2; break;
		case 0xf3: my_stprintf_s(buffer, buffer_len, _T("di")); break;
		case 0xf4: my_stprintf_s(buffer, buffer_len, _T("cp   %s"), get_value_or_symbol(debugger->first_symbol, _T("$%04x"), oprom[ptr] | (oprom[ptr + 1] << 8))); ptr += 2; break;
		case 0xf5: my_stprintf_s(buffer, buffer_len, _T("push a")); break;
		case 0xf6: my_stprintf_s(buffer, buffer_len, _T("ori  $%02x"), oprom[ptr++]); break;
		case 0xf7: my_stprintf_s(buffer, buffer_len, _T("rst  6")); break;
		case 0xf8: my_stprintf_s(buffer, buffer_len, _T("rm")); break;
		case 0xf9: my_stprintf_s(buffer, buffer_len, _T("sphl")); break;
		case 0xfa: my_stprintf_s(buffer, buffer_len, _T("jm   %s"), get_value_or_symbol(debugger->first_symbol, _T("$%04x"), oprom[ptr] | (oprom[ptr + 1] << 8))); ptr += 2; break;
		case 0xfb: my_stprintf_s(buffer, buffer_len, _T("ei")); break;
		case 0xfc: my_stprintf_s(buffer, buffer_len, _T("cm   %s"), get_value_or_symbol(debugger->first_symbol, _T("$%04x"), oprom[ptr] | (oprom[ptr + 1] << 8))); ptr += 2; break;
		case 0xfd: my_stprintf_s(buffer, buffer_len, _T("cm   %s"), get_value_or_symbol(debugger->first_symbol, _T("$%04x"), oprom[ptr] | (oprom[ptr + 1] << 8))); ptr += 2; break;
		case 0xfe: my_stprintf_s(buffer, buffer_len, _T("cpi  $%02x"), oprom[ptr++]); break;
		case 0xff: my_stprintf_s(buffer, buffer_len, _T("rst  7")); break;
	}
	return ptr;
}
