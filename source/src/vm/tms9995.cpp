/*
	Skelton for retropc emulator

	Origin : MAME TMS99xx Core
	Author : Takeda.Toshiya
	Date   : 2007.07.14 -

	[ TMS9995 ]
*/

#include "tms9995.h"
#include "../fileio.h"

#define ST_LGT	0x8000
#define ST_AGT	0x4000
#define ST_EQ	0x2000
#define ST_C	0x1000
#define ST_OV	0x0800
#define ST_OP	0x0400
#define ST_X	0x0200
#define ST_OVIE	0x0020
#define ST_IM	0x000f

#define R0	0
#define R1	2
#define R2	4
#define R3	6
#define R4	8
#define R5	10
#define R6	12
#define R7	14
#define R8	16
#define R9	18
#define R10	20
#define R11	22
#define R12	24
#define R13	26
#define R14	28
#define R15	30

#define MEM_WAIT_BYTE	4
#define MEM_WAIT_WORD	12
#define BYTE_XOR_BE(a)	((a) ^ 1)
//#define BYTE_XOR_BE(a)	(a)
#define CRU_MASK_R	((1 << 12) - 1)
#define CRU_MASK_W	((1 << 15) - 1)

static const uint16 right_shift_mask_table[17] = {
	0xffff,
	0x7fff, 0x3fff, 0x1fff, 0x0fff, 0x07ff, 0x03ff, 0x01ff, 0x00ff,
	0x007f, 0x003f, 0x001f, 0x000f, 0x0007, 0x0003, 0x0001, 0x0000
};
static const uint16 inverted_right_shift_mask_table[17] = {
	0x0000,
	0x8000, 0xc000, 0xe000, 0xf000, 0xf800, 0xfc00, 0xfe00, 0xff00,
	0xff80, 0xffc0, 0xffe0, 0xfff0, 0xfff8, 0xfffc, 0xfffe, 0xffff
};

// memory
uint16 TMS9995::RM16(uint16 addr)
{
	if(addr < 0xf000) {
		period += MEM_WAIT_WORD;
		uint16 tmp = d_mem->read_data8(addr);
		return (tmp << 8) | d_mem->read_data8(addr + 1);
	} else if(addr < 0xf0fc) {
		return *(uint16 *)(&RAM[addr & 0xff]);
	} else if(addr < 0xfffa) {
		period += MEM_WAIT_WORD;
		uint16 tmp = d_mem->read_data8(addr);
		return (tmp << 8) | d_mem->read_data8(addr + 1);
	} else if(addr < 0xfffc) {
		if(dec_enabled && !(mode & 1)) {
			return (dec_timer >> 4) & 0xffff;
		} else {
			return dec_count;
		}
	} else {
		return *(uint16 *)(&RAM[addr & 0xff]);
	}
}

void TMS9995::WM16(uint16 addr, uint16 val)
{
	if(addr < 0xf000) {
		period += MEM_WAIT_WORD;
		d_mem->write_data8(addr, val >> 8);
		d_mem->write_data8(addr + 1, val & 0xff);
	} else if(addr < 0xf0fc) {
		*(uint16 *)(&RAM[addr & 0xff]) = val;
	} else if(addr < 0xfffa) {
		period += MEM_WAIT_WORD;
		d_mem->write_data8(addr, val >> 8);
		d_mem->write_data8(addr + 1, val & 0xff);
	} else if(addr < 0xfffc) {
		dec_interval = val;
		update_dec();
	} else {
		*(uint16 *)(&RAM[addr & 0xff]) = val;
	}
}

uint8 TMS9995::RM8(uint16 addr)
{
	if((addr < 0xf000)) {
		period += MEM_WAIT_BYTE;
		return d_mem->read_data8(addr);
	} else if(addr < 0xf0fc) {
		return RAM[BYTE_XOR_BE(addr & 0xff)];
	} else if(addr < 0xfffa) {
		period += MEM_WAIT_BYTE;
		return d_mem->read_data8(addr);
	} else if(addr < 0xfffc) {
		uint16 tmp;
		if(dec_enabled && !(mode & 1)) {
			tmp = (dec_timer >> 4) & 0xffff;
		} else {
			tmp = dec_count;
		}
		return (addr & 1) ? (tmp & 0xff) : (tmp >> 8);
	} else {
		return RAM[BYTE_XOR_BE(addr & 0xff)];
	}
}

void TMS9995::WM8(uint32 addr, uint8 val)
{
	if((addr < 0xf000)) {
		period += MEM_WAIT_BYTE;
		d_mem->write_data8(addr, val);
	} else if(addr < 0xf0fc) {
		RAM[BYTE_XOR_BE(addr & 0xff)] = val;
	} else if(addr < 0xfffa) {
		period += MEM_WAIT_BYTE;
		d_mem->write_data8(addr, val);
	} else if(addr < 0xfffc) {
		dec_interval = (val << 8) | val;
		update_dec();
	} else {
		RAM[BYTE_XOR_BE(addr & 0xff)] = val;
	}
}

inline uint16 TMS9995::FETCH16()
{
	uint16 tmp = RM16(PC);
	PC += 2;
	return tmp;
}

#define RREG(reg)	RM16((WP + (reg)) & 0xffff)
#define WREG(reg, val)	WM16((WP + (reg)) & 0xffff, (val))

// i/o
uint16 TMS9995::IN8(int addr)
{
	switch(addr) {
	case 0x1ee:
		return mode & 0xff;
	case 0x1ef:
		return (mode >> 8) & 0xff;
	case 0x1fd:
		if(mid) {
			return d_io->read_io8(addr) | 0x10;
		} else {
			return d_io->read_io8(addr) & ~ 0x10;
		}
	}
	return d_io->read_io8(addr);
}

void TMS9995::OUT8(uint16 addr, uint16 val)
{
	switch(addr) {
	case 0xf70:
		if(val & 1) {
			mode |= 1;
		} else {
			mode &= ~1;
		}
		update_dec();
		break;
	case 0xf71:
		if(val & 1) {
			mode |= 2;
		} else {
			mode &= ~2;
		}
		update_dec();
		break;
	case 0xf72:
	case 0xf73:
	case 0xf74:
		break;
	case 0xf75:
	case 0xf76:
	case 0xf77:
	case 0xf78:
	case 0xf79:
	case 0xf7a:
	case 0xf7b:
	case 0xf7c:
	case 0xf7d:
	case 0xf7e:
	case 0xf7f:
		if(val & 1) {
			mode |= (1 << (addr - 0xf70));
		} else {
			mode &= ~(1 << (addr - 0xf70));
		}
		break;
	case 0xfed:
		mid = ((val & 1) != 0);
		break;
	}
	d_io->write_io8(addr, val & 1);
}

inline void TMS9995::EXTOUT8(uint16 addr)
{
	d_io->write_io8(addr << 15, 0);	// or is it 1 ???
}

uint16 TMS9995::RCRU(uint16 addr, int bits)
{
	static const uint16 bitmask[] = {
		0,
		0x0001,0x0003,0x0007,0x000f,0x001f,0x003f,0x007f,0x00ff,
		0x01ff,0x03ff,0x07ff,0x0fff,0x1fff,0x3fff,0x7fff,0xffff
	};
	uint16 loc = (uint16)((addr >> 3) & CRU_MASK_R);
	uint16 ofs = addr & 7;
	uint16 val = IN8(loc);
	
	if((ofs + bits) > 8) {
		loc = (uint16)((loc + 1) & CRU_MASK_R);
		val |= IN8(loc) << 8;
		if((ofs + bits) > 16) {
			loc = (uint16)((loc + 1) & CRU_MASK_R);
			val |= IN8(loc) << 16;
		}
	}
	val >>= ofs;
	return val & bitmask[bits];
}

void TMS9995::WCRU(uint16 addr, int bits, uint16 val)
{
	addr &= CRU_MASK_W;
	for(int i = 0; i < bits; i++) {
		OUT8(addr, val);
		val >>= 1;
		addr = (uint16)((addr + 1) & CRU_MASK_W);
	}
}

void TMS9995::reset()
{
	ST = 0;
	getstat();
	mid = idle = false;
	
	mode &= ~3;
	update_dec();
	int_latch = 0;
	mode &= 0xffe3;
	update_int();
	contextswitch(0x0000);
//	period += 56;
	count = 0;
}

int TMS9995::run(int clock)
{
	// run cpu
	if(clock == -1) {
		// run only one opcode
		count = 0;
		run_one_opecode();
		return -count;
	} else {
		// run cpu while given clocks
		count += clock;
		int first_count = count;
		
		while(count > 0) {
			run_one_opecode();
		}
		return first_count - count;
	}
}

void TMS9995::run_one_opecode()
{
	period = 0;
	
	// update interrupt
	if(int_pending && int_enabled) {
		int level = irq_level;
		if(nmi) {
			contextswitch(0xfffc);
			ST &= ~ST_IM;
			idle = false;
			period += 56;
		} else if(level <= (ST & ST_IM)) {
			contextswitch(level*4);
			if(level) {
				ST = (ST & ~ST_IM) | (level -1);
				int_pending = false;
			} else {
				ST &= ~ST_IM;
			}
			ST &= 0xfe00;
			idle = false;
			
			if(level != 2) {
				int int_mask = 1 << level;
				int mode_mask = (level == 1) ? 4 : int_mask;
				int_latch &= ~int_mask;
				mode &= ~mode_mask;
			}
			period += 56;
		} else {
			int_pending = false;
		}
	}
	
	// execute opecode
	if(idle) {
		EXTOUT8(2);
		period += 8;
	} else {
		int_enabled = true;
		prevPC = PC;
		uint16 op = FETCH16();
		execute(op);
		if((ST & ST_OVIE) && (ST & ST_OV) && (irq_level > 2)) {
			irq_level = 2;
		}
	}
	count -= period;
	
	// update timer
	if(dec_enabled && !(mode & 1)) {
		dec_timer -= period;
		if(dec_timer <= 0) {
			int_latch |= 8;
			mode |= 8;
			update_int();
			
			// restart ???
			dec_timer += dec_interval << 4;
		}
	}
}

void TMS9995::write_signal(int id, uint32 data, uint32 mask)
{
	if(id == SIG_TMS9995_NMI) {
		nmi = ((data & mask) != 0);
		update_int();
	} else if(id == SIG_TMS9995_INT1) {
		set_irq_line(0, ((data & mask) != 0));
	} else if(id == SIG_TMS9995_INT4) {
		set_irq_line(1, ((data & mask) != 0));
	}
}

void TMS9995::set_irq_line(int irqline, bool state)
{
	int int_mask = (irqline == 0) ? 2 : 0x10;
	int mode_mask = (irqline == 0) ? 4 : 0x10;
	bool prev_state = ((int_state & int_mask) != 0);
	
	if(prev_state != state) {
		if(state) {
			int_state |= int_mask;
			if((irqline == 1) && (mode & 1)) {
				if(dec_enabled) {
					if((--dec_count) == 0) {
						int_latch |= 8;
						mode |= 8;
						update_int();
						dec_count = dec_interval;
					}
				}
			} else {
				int_latch |= int_mask;
				mode |= mode_mask;
			}
		} else {
			int_state &= ~int_mask;
		}
		update_int();
	}
}

void TMS9995::update_int()
{
	if(nmi) {
		int_pending = true;
	} else {
		int cur_int, level;
		if(mode & 1) {
			cur_int = (int_state & ~0x10) | int_latch;
		} else {
			cur_int = int_state | int_latch;
		}
		if(cur_int) {
			for (level = 0; !(cur_int & 1); cur_int >>= 1, level++) {
				;
			}
		} else {
			level = 16;
		}
		irq_level = level;
		int_pending = (level <= (ST & ST_IM));
	}
}

void TMS9995::update_dec()
{
	dec_count = dec_interval;
	dec_enabled = ((mode & 2) && dec_interval);
	dec_timer = dec_interval << 4;
}

void TMS9995::contextswitch(uint16 addr)
{
	uint16 oldWP = WP;
	uint16 oldPC = PC;
	
	WP = RM16(addr) & ~1;
	PC = RM16(addr+2) & ~1;
	WREG(R13, oldWP);
	WREG(R14, oldPC);
	setstat();
	WREG(R15, ST);
}

void TMS9995::execute(uint16 op)
{
	switch(op >> 8) {
	case 0x00:
		h0040(op);
		break;
	case 0x01:
		h0100(op);
		break;
	case 0x02: case 0x03:
		h0200(op);
		break;
	case 0x04: case 0x05: case 0x06: case 0x07:
		h0400(op);
		break;
	case 0x08: case 0x09: case 0x0a: case 0x0b:
		h0800(op);
		break;
	case 0x0c: case 0x0d: case 0x0e: case 0x0f:
		illegal(op);
		break;
	case 0x10: case 0x11: case 0x12: case 0x13: case 0x14: case 0x15: case 0x16: case 0x17:
	case 0x18: case 0x19: case 0x1a: case 0x1b: case 0x1c: case 0x1d: case 0x1e: case 0x1f:
		h1000(op);
		break;
	case 0x20: case 0x21: case 0x22: case 0x23: case 0x24: case 0x25: case 0x26: case 0x27:
	case 0x28: case 0x29: case 0x2a: case 0x2b:
	case 0x38: case 0x39: case 0x3a: case 0x3b: case 0x3c: case 0x3d: case 0x3e: case 0x3f:
		h2000(op);
		break;
	case 0x2c: case 0x2d: case 0x2e: case 0x2f:
		xop(op);
		break;
	case 0x30: case 0x31: case 0x32: case 0x33: case 0x34: case 0x35: case 0x36: case 0x37:
		ldcr_stcr(op);
		break;
	case 0x40: case 0x41: case 0x42: case 0x43: case 0x44: case 0x45: case 0x46: case 0x47:
	case 0x48: case 0x49: case 0x4a: case 0x4b: case 0x4c: case 0x4d: case 0x4e: case 0x4f:
	case 0x60: case 0x61: case 0x62: case 0x63: case 0x64: case 0x65: case 0x66: case 0x67:
	case 0x68: case 0x69: case 0x6a: case 0x6b: case 0x6c: case 0x6d: case 0x6e: case 0x6f:
	case 0x80: case 0x81: case 0x82: case 0x83: case 0x84: case 0x85: case 0x86: case 0x87:
	case 0x88: case 0x89: case 0x8a: case 0x8b: case 0x8c: case 0x8d: case 0x8e: case 0x8f:
	case 0xa0: case 0xa1: case 0xa2: case 0xa3: case 0xa4: case 0xa5: case 0xa6: case 0xa7:
	case 0xa8: case 0xa9: case 0xaa: case 0xab: case 0xac: case 0xad: case 0xae: case 0xaf:
	case 0xc0: case 0xc1: case 0xc2: case 0xc3: case 0xc4: case 0xc5: case 0xc6: case 0xc7:
	case 0xc8: case 0xc9: case 0xca: case 0xcb: case 0xcc: case 0xcd: case 0xce: case 0xcf:
	case 0xe0: case 0xe1: case 0xe2: case 0xe3: case 0xe4: case 0xe5: case 0xe6: case 0xe7:
	case 0xe8: case 0xe9: case 0xea: case 0xeb: case 0xec: case 0xed: case 0xee: case 0xef:
		h4000w(op);
		break;
	case 0x50: case 0x51: case 0x52: case 0x53: case 0x54: case 0x55: case 0x56: case 0x57:
	case 0x58: case 0x59: case 0x5a: case 0x5b: case 0x5c: case 0x5d: case 0x5e: case 0x5f:
	case 0x70: case 0x71: case 0x72: case 0x73: case 0x74: case 0x75: case 0x76: case 0x77:
	case 0x78: case 0x79: case 0x7a: case 0x7b: case 0x7c: case 0x7d: case 0x7e: case 0x7f:
	case 0x90: case 0x91: case 0x92: case 0x93: case 0x94: case 0x95: case 0x96: case 0x97:
	case 0x98: case 0x99: case 0x9a: case 0x9b: case 0x9c: case 0x9d: case 0x9e: case 0x9f:
	case 0xb0: case 0xb1: case 0xb2: case 0xb3: case 0xb4: case 0xb5: case 0xb6: case 0xb7:
	case 0xb8: case 0xb9: case 0xba: case 0xbb: case 0xbc: case 0xbd: case 0xbe: case 0xbf:
	case 0xd0: case 0xd1: case 0xd2: case 0xd3: case 0xd4: case 0xd5: case 0xd6: case 0xd7:
	case 0xd8: case 0xd9: case 0xda: case 0xdb: case 0xdc: case 0xdd: case 0xde: case 0xdf:
	case 0xf0: case 0xf1: case 0xf2: case 0xf3: case 0xf4: case 0xf5: case 0xf6: case 0xf7:
	case 0xf8: case 0xf9: case 0xfa: case 0xfb: case 0xfc: case 0xfd: case 0xfe: case 0xff:
		h4000b(op);
		break;
	}
}

void TMS9995::h0040(uint16 op)
{
	uint16 addr = (((op & 0xf) << 1) + WP) & ~1;
	
	switch((op & 0xf0) >> 4) {
	case 8: // LST
		ST = RM16(addr);
		getstat();
		break;
	case 9: // LWP
		WP = RM16(addr) & ~1;
		break;
	default:
		illegal(op);
		break;
	}
}

void TMS9995::h0100(uint16 op)
{
	uint16 src = decipheraddr(op) & ~1;
	int16 d;
	int32 p, q;
	
	switch((op & 0xc0) >> 6) {
	case 2: // DIVS
		d = (int16)RM16(src);
		p = (int32)((RREG(R0) << 16) | RREG(R1));
		q = d ? (p / d) : 0;
		if((d == 0) || (q < -32768L) || (q > 32767L)) {
			ST |= ST_OV;
			period += 40;
		} else {
			ST &= ~ST_OV;
			setst_lae(q);
			WREG(R0, q);
			WREG(R1, p % d);
			period += 132;
		}
		break;
	case 3: // MPYS
		p = ((int32)(int16)RM16(src)) * ((int32)(int16)RREG(R0));
		ST &= ~(ST_LGT | ST_AGT | ST_EQ);
		if(p > 0) {
			ST |= (ST_LGT | ST_AGT);
		} else if(p < 0) {
			ST |= ST_LGT;
		} else {
			ST |= ST_EQ;
		}
		WREG(R0, p >> 16);
		WREG(R1, p);
		period += 100;
		break;
	default:
		illegal(op);
		break;
	}
}

void TMS9995::h0200(uint16 op)
{
	uint16 addr = (((op & 0xf) << 1) + WP) & ~1, val;
	
	if(((op < 0x2e0) && (op & 0x10)) || ((op >= 0x2e0) && (op & 0x1f))) {
		illegal(op);
		return;
	}
	switch((op & 0x1e0) >> 5)  {
	case 0: // LI
		val = FETCH16();
		WM16(addr, val);
		setst_lae(val);
		period += 12;
		break;
	case 1: //AI
		val = FETCH16();
		val = setst_add_laeco(RM16(addr), val);
		WM16(addr, val);
		period += 16;
		break;
	case 2: // ANDI
		val = FETCH16();
		val = RM16(addr) & val;
		WM16(addr, val);
		setst_lae(val);
		period += 16;
		break;
	case 3: // ORI
		val = FETCH16();
		val = RM16(addr) | val;
		WM16(addr, val);
		setst_lae(val);
		period += 16;
		break;
	case 4: // CI
		val = FETCH16();
		setst_c_lae(val, RM16(addr));
		period += 16;
		break;
	case 5: // STWP
		WM16(addr, WP);
		period += 12;
		break;
	case 6: // STST
		setstat();
		WM16(addr, ST);
		period += 12;
		break;
	case 7: // LWPI
		WP = FETCH16() & ~1;
		period += 16;
		break;
	case 8: // LIMI
		val = FETCH16();
		ST = (ST & ~ST_IM) | (val & ST_IM);
		update_int();
		period += 20;
		break;
	case 9: // LMF
		illegal(op);
		break;
	case 10: // idle
		idle = true;
		EXTOUT8(2);
		period += 28;
		break;
	case 11: // RSET
		ST &= 0xfff0;
		update_int();
		EXTOUT8(3);
		period += 28;
		break;
	case 12: // RTWP
		addr = (WP + R13) & ~1;
		WP = RM16(addr) & ~1;
		addr += 2;
		PC = RM16(addr) & ~1;
		addr += 2;
		ST = RM16(addr);
		getstat();
		update_int();
		period += 24;
		break;
	case 13: // CKON
	case 14: // CKOF
	case 15: // LREX
		EXTOUT8((op & 0xe0) >> 5);
		period += 28;
		break;
	}
}

void TMS9995::h0400(uint16 op)
{
	uint16 addr = decipheraddr(op) & ~1, val;
	
	switch((op & 0x3c0) >> 6) {
	case 0: // BLWP
		contextswitch(addr);
		period += 44;
		int_enabled = false;
		break;
	case 1: // B
		PC = addr;
		period += 12;
		break;
	case 2: // X
		execute(RM16(addr));
		period += 8;
		break;
	case 3: // CLR
		WM16(addr, 0);
		period += 12;
		break;
	case 4: // NEG
		val = -(int16)RM16(addr);
		if(val) {
			ST &= ~ ST_C;
		} else {
			ST |= ST_C;
		}
		setst_laeo(val);
		WM16(addr, val);
		period += 12;
		break;
	case 5: // INV
		val = ~ RM16(addr);
		WM16(addr, val);
		setst_lae(val);
		period += 12;
		break;
	case 6: // INC
		val = setst_add_laeco(RM16(addr), 1);
		WM16(addr, val);
		period += 12;
		break;
	case 7: // INCT
		val = setst_add_laeco(RM16(addr), 2);
		WM16(addr, val);
		period += 12;
		break;
	case 8: // DEC
		val = setst_sub_laeco(RM16(addr), 1);
		WM16(addr, val);
		period += 12;
		break;
	case 9: // DECT
		val = setst_sub_laeco(RM16(addr), 2);
		WM16(addr, val);
		period += 12;
		break;
	case 10: // BL
		WREG(R11, PC);
		PC = addr;
		period += 20;
		break;
	case 11: // SWPB
		val = RM16(addr);
		val = logical_right_shift(val, 8) | (val << 8);
		WM16(addr, val);
		period += 52;
		break;
	case 12: // SETO
		WM16(addr, 0xffff);
		period += 12;
		break;
	case 13: // ABS
		ST &= ~ (ST_LGT | ST_AGT | ST_EQ | ST_C | ST_OV);
		val = RM16(addr);
		period += 12;
		if(((int16)val) > 0) {
			ST |= ST_LGT | ST_AGT;
		} else if(((int16)val) < 0) {
			ST |= ST_LGT;
			if(val == 0x8000) {
				ST |= ST_OV;
			}
			val = -((int16)val);
		} else {
			ST |= ST_EQ;
		}
		WM16(addr, val);
		break;
	default:
		illegal(op);
		break;
	}
}

void TMS9995::h0800(uint16 op)
{
	uint16 cnt = (op & 0xf0) >> 4;
	uint16 addr = (((op & 0xf) << 1) + WP) & ~1, val;
	
	period += 20;
	if(cnt == 0) {
		period += 8;
		cnt = RREG(R0) & 0xf;
		if(cnt == 0) {
			cnt = 16;
		}
	}
	period += 4 * cnt;
	
	switch((op & 0x300) >> 8) {
	case 0: // SRA
		val = setst_sra_laec(RM16(addr), cnt);
		WM16(addr, val);
		break;
	case 1: // SRL
		val = setst_srl_laec(RM16(addr), cnt);
		WM16(addr, val);
		break;
	case 2: // SLA
		val = setst_sla_laeco(RM16(addr), cnt);
		WM16(addr, val);
		break;
	case 3: // SRC
		val = setst_src_laec(RM16(addr), cnt);
		WM16(addr, val);
		break;
	}
}

void TMS9995::h1000(uint16 op)
{
	int16 ofs = (int8)op;
	
	switch((op & 0xf00) >> 8) {
	case 0: // JMP
		PC += (ofs + ofs);
		period += 12;
		break;
	case 1: // JLT
		if(!(ST & (ST_AGT | ST_EQ))) {
			PC += (ofs + ofs);
		}
		period += 12;
		break;
	case 2: // JLE
		if((!(ST & ST_LGT)) || (ST & ST_EQ)) {
			PC += (ofs + ofs);
		}
		period += 12;
		break;
	case 3: // JEQ
		if(ST & ST_EQ) {
			PC += (ofs + ofs);
		}
		period += 12;
		break;
	case 4: // JHE
		if(ST & (ST_LGT | ST_EQ)) {
			PC += (ofs + ofs);
		}
		period += 12;
		break;
	case 5: // JGT
		if(ST & ST_AGT) {
			PC += (ofs + ofs);
		}
		period += 12;
		break;
	case 6: // JNE
		if(!(ST & ST_EQ)) {
			PC += (ofs + ofs);
		}
		period += 12;
		break;
	case 7: // JNC
		if(!(ST & ST_C)) {
			PC += (ofs + ofs);
		}
		period += 12;
		break;
	case 8: // JOC
		if(ST & ST_C) {
			PC += (ofs + ofs);
		}
		period += 12;
		break;
	case 9: // JNO
		if(!(ST & ST_OV)) {
			PC += (ofs + ofs);
		}
		period += 12;
		break;
	case 10: // JL
		if(!(ST & (ST_LGT | ST_EQ))) {
			PC += (ofs + ofs);
		}
		period += 12;
		break;
	case 11: // JH
		if((ST & ST_LGT) && !(ST & ST_EQ)) {
			PC += (ofs + ofs);
		}
		period += 12;
		break;
	case 12: { // JOP
		int i = 0;
		uint8 a = lastparity;
		while(a != 0) {
			if(a & 1) {
				i++;
			}
			a >>= 1;
		}
		if(i & 1) {
			PC += (ofs + ofs);
		}
		period += 12;
		break;
	}
	case 13: // SBO
		WCRU((RREG(R12) >> 1) + ofs, 1, 1);
		period += 32;
		break;
	case 14: // SBZ
		WCRU((RREG(R12) >> 1) + ofs, 1, 0);
		period += 32;
		break;
	case 15: // TB
		setst_e(RCRU((RREG(R12)>> 1) + ofs, 1) & 1, 1);
		period += 32;
		break;
	}
}

void TMS9995::h2000(uint16 op)
{
	uint16 src = decipheraddr(op) & ~1;
	uint16 dst = ((((op & 0x3c0) >> 6) << 1) + WP) & ~1, val, d, h;
	uint32 p;
	
	switch((op & 0x1c00) >> 10) {
	case 0: // COC
		val = RM16(src);
		setst_e(val & RM16(dst), val);
		period += 16;
		break;
	case 1: // CZC
		val = RM16(src);
		setst_e(val & ~RM16(dst), val);
		period += 16;
		break;
	case 2: // XOR
		val = RM16(src) ^ RM16(dst);
		setst_lae(val);
		WM16(dst,val);
		period += 16;
		break;
//	case 3: // XOP
//	case 4: // LDCR
//	case 5: // STCR
	case 6: // MPY
		p = (uint32)RM16(src) * (uint32)RM16(dst);
		WM16(dst, p >> 16);
		WM16((dst + 2) & 0xffff, p);
		period += 92;
		break;
	case 7: // DIV
		d = RM16(src);
		h = RM16(dst);
		p = (((uint32)h) << 16) | RM16((dst + 2) & 0xffff);
		if(d == 0 || d <= h) {
			ST |= ST_OV;
			period += 24;
		} else {
			ST &= ~ST_OV;
			WM16(dst, p / d);
			WM16((dst + 2) & 0xffff, p % d);
			period += 112;
		}
		break;
	}
}

void TMS9995::xop(uint16 op)
{
	uint16 tmp = (op & 0x3c0) >> 6;
	uint16 val = decipheraddr(op);
	
	contextswitch(0x40 + (tmp << 2));
	ST |= ST_X;
	WREG(R11, val);
	period += 60;
	int_enabled = false;
}

void TMS9995::ldcr_stcr(uint16 op)
{
	uint16 cnt = (op & 0x3c0) >> 6, addr;
	int val;
	
	if(cnt == 0) {
		cnt = 16;
	}
	if(cnt <= 8) {
		addr = decipheraddrbyte(op);
	} else {
		addr = decipheraddr(op) & ~1;
	}
	if(op < 0x3400) {
		// LDCR
		if(cnt <= 8) {
			val = RM16(addr & ~1);
			if(addr & 1) {
				val &= 0xff;
			} else {
				val = (val >> 8) & 0xff;
			}
			(void)RREG(cnt + cnt);
			setst_byte_laep(val);
		} else {
			val = RM16(addr);
			(void)RREG(cnt + cnt);
			setst_lae(val);
		}
		WCRU((RREG(R12) >> 1), cnt, val);
		period += 36 + cnt + cnt;
	} else {
		// STCR
		if(cnt <= 8) {
			int val2 = RM16(addr & ~1);
			(void)RREG(cnt + cnt);
			val = RCRU((RREG(R12) >> 1), cnt);
			setst_byte_laep(val);
			if(addr & 1) {
				WM16(addr & ~1, (val & 0xff) | (val2 & 0xff00));
			} else {
				WM16(addr & ~1, (val2 & 0xff) | ((val << 8) & 0xff00));
			}
			period += 76 + cnt;
		} else {
			(void)RM16(addr);
			(void)RREG(cnt + cnt);
			val = RCRU((RREG(R12) >> 1), cnt);
			setst_lae(val);
			WM16(addr, val);
			period += 108 + cnt;
		}
	}
}

void TMS9995::h4000w(uint16 op)
{
	uint16 src = decipheraddr(op) & ~1;
	uint16 dst = decipheraddr(op >> 6) & ~1;
	uint16 val = RM16(src);
	
	switch((op >> 13) & 7) {
	case 2: // SZC
		val = RM16(dst) & ~val;
		setst_lae(val);
		WM16(dst, val);
		period += 16;
		break;
	case 3: // S
		val = setst_sub_laeco(RM16(dst), val);
		WM16(dst, val);
		period += 16;
		break;
	case 4: // C
		setst_c_lae(RM16(dst), val);
		period += 16;
		break;
	case 5: // A
		val = setst_add_laeco(RM16(dst), val);
		WM16(dst, val);
		period += 16;
		break;
	case 6: // MOV
		setst_lae(val);
		WM16(dst, val);
		period += 12;
		break;
	case 7: // SOC
		val = val | RM16(dst);
		setst_lae(val);
		WM16(dst, val);
		period += 16;
		break;
	}
}

void TMS9995::h4000b(uint16 op)
{
	uint16 src = decipheraddrbyte(op);
	uint16 dst = decipheraddrbyte(op >> 6);
	uint8 val = RM8(src);
	
	switch((op >> 13) & 7) {
	case 2: // SZCB
		val = RM8(dst) & ~val;
		setst_byte_laep(val);
		WM8(dst, val);
		period += 16;
		break;
	case 3: // SB
		val = setst_subbyte_laecop(RM8(dst), val);
		WM8(dst, val);
		period += 16;
		break;
	case 4: // CB
		setst_c_lae(RM8(dst) << 8, val << 8);
		lastparity = val;
		period += 16;
		break;
	case 5: // AB
		val = setst_addbyte_laecop(RM8(dst), val);
		WM8(dst, val);
		period += 16;
		break;
	case 6: // MOVB
		setst_byte_laep(val);
		WM8(dst, val);
		period += 12;
		break;
	case 7: // SOCB
		val = val | RM8(dst);
		setst_byte_laep(val);
		WM8(dst, val);
		period += 16;
		break;
	}
}

void TMS9995::illegal(uint16 op)
{
	mid = true;
	contextswitch(8);
	ST = (ST & 0xfe00) | 1;
	int_enabled = false;
}

uint16 TMS9995::decipheraddr(uint16 op)
{
	uint16 reg = (op & 0xf) << 1, tmp;
	
	switch(op & 0x30) {
	case 0x00:
		return reg + WP;
	case 0x10:
		period += 4;
		return RM16(reg + WP);
	case 0x20:
		tmp = FETCH16();
		if(reg) {
			period += 12;
			return RM16(reg + WP) + tmp;
		} else {
			period += 4;
			return tmp;
		}
	default:
		reg += WP;
		period += 12;
		tmp = RM16(reg);
		WM16(reg, tmp + 2);
		return tmp;
	}
}

uint16 TMS9995::decipheraddrbyte(uint16 op)
{
	uint16 reg = (op & 0xf) << 1, tmp;
	
	switch(op & 0x30) {
	case 0x00:
		return reg + WP;
	case 0x10:
		period += 4;
		return RM16(reg + WP);
	case 0x20:
		tmp = FETCH16();
		if(reg) {
			period += 12;
			return RM16(reg + WP) + tmp;
		} else {
			period += 4;
			return tmp;
		}
	default:
		reg += WP;
		period += 12;
		tmp = RM16(reg);
		WM16(reg, tmp + 1);
		return tmp;
	}
}

inline void TMS9995::setstat()
{
	ST &= ~ ST_OP;
	uint8 p = lastparity;
	for(int i = 0; i < 8; i++) {
		if(p & 1) {
			ST ^= ST_OP;
		}
		p >>= 1;
	}
}

inline void TMS9995::getstat()
{
	if(ST & ST_OP) {
		lastparity = 1;
	} else {
		lastparity = 0;
	}
}

inline uint16 TMS9995::logical_right_shift(uint16 val, int c)
{
	return (val >> c) & right_shift_mask_table[c];
}

inline int16 TMS9995::arithmetic_right_shift(int16 val, int c)
{
	if(val < 0) {
		return (val >> c) | inverted_right_shift_mask_table[c];
	} else {
		return (val >> c) & right_shift_mask_table[c];
	}
}

inline void TMS9995::setst_lae(int16 val)
{
	ST &= ~(ST_LGT | ST_AGT | ST_EQ);
	if(val > 0) {
		ST |= (ST_LGT | ST_AGT);
	} else if(val < 0) {
		ST |= ST_LGT;
	} else {
		ST |= ST_EQ;
	}
}

inline void TMS9995::setst_byte_laep(int8 val)
{
	ST &= ~(ST_LGT | ST_AGT | ST_EQ);
	if(val > 0) {
		ST |= (ST_LGT | ST_AGT);
	} else if(val < 0) {
		ST |= ST_LGT;
	} else {
		ST |= ST_EQ;
	}
	lastparity = val;
}

inline void TMS9995::setst_e(uint16 val, uint16 to)
{
	if(val == to) {
		ST |= ST_EQ;
	} else {
		ST &= ~ ST_EQ;
	}
}

inline void TMS9995::setst_c_lae(uint16 to, uint16 val)
{
	ST &= ~(ST_LGT | ST_AGT | ST_EQ);
	if(val == to) {
		ST |= ST_EQ;
	} else {
		if(((int16)val) > ((int16)to)) {
			ST |= ST_AGT;
		}
		if(((uint16)val) > ((uint16)to)) {
			ST |= ST_LGT;
		}
	}
}

inline int16 TMS9995::setst_add_laeco(int a, int b)
{
	ST &= ~(ST_LGT | ST_AGT | ST_EQ | ST_C | ST_OV);
	uint32 res = (a & 0xffff) + (b & 0xffff);
	if(res & 0x10000) {
		ST |= ST_C;
	}
	if((res ^ b) & (res ^ a) & 0x8000) {
		ST |= ST_OV;
	}
	int16 res2 = (int16)res;
	if(res2 > 0) {
		ST |= ST_LGT | ST_AGT;
	} else if(res2 < 0) {
		ST |= ST_LGT;
	} else {
		ST |= ST_EQ;
	}
	return res2;
}

inline int16 TMS9995::setst_sub_laeco(int a, int b)
{
	ST &= ~(ST_LGT | ST_AGT | ST_EQ | ST_C | ST_OV);
	uint32 res = (a & 0xffff) - (b & 0xffff);
	if(!(res & 0x10000)) {
		ST |= ST_C;
	}
	if((a ^ b) & (a ^ res) & 0x8000) {
		ST |= ST_OV;
	}
	int16 res2 = (int16)res;
	if(res2 > 0) {
		ST |= ST_LGT | ST_AGT;
	} else if(res2 < 0) {
		ST |= ST_LGT;
	} else {
		ST |= ST_EQ;
	}
	return res2;
}

inline int8 TMS9995::setst_addbyte_laecop(int a, int b)
{
	ST &= ~(ST_LGT | ST_AGT | ST_EQ | ST_C | ST_OV | ST_OP);
	uint32 res = (a & 0xff) + (b & 0xff);
	if(res & 0x100) {
		ST |= ST_C;
	}
	if((res ^ b) & (res ^ a) & 0x80) {
		ST |= ST_OV;
	}
	int8 res2 = (int8)res;
	if(res2 > 0) {
		ST |= ST_LGT | ST_AGT;
	} else if(res2 < 0) {
		ST |= ST_LGT;
	} else {
		ST |= ST_EQ;
	}
	lastparity = res2;
	return res2;
}

inline int8 TMS9995::setst_subbyte_laecop(int a, int b)
{
	ST &= ~(ST_LGT | ST_AGT | ST_EQ | ST_C | ST_OV | ST_OP);
	uint32 res = (a & 0xff) - (b & 0xff);
	if(!(res & 0x100)) {
		ST |= ST_C;
	}
	if((a ^ b) & (a ^ res) & 0x80) {
		ST |= ST_OV;
	}
	int8 res2 = (int8)res;
	if(res2 > 0) {
		ST |= ST_LGT | ST_AGT;
	} else if(res2 < 0) {
		ST |= ST_LGT;
	} else {
		ST |= ST_EQ;
	}
	lastparity = res2;
	return res2;
}

inline void TMS9995::setst_laeo(int16 val)
{
	ST &= ~(ST_LGT | ST_AGT | ST_EQ | ST_OV);
	if(val > 0) {
		ST |= ST_LGT | ST_AGT;
	} else if(val < 0) {
		ST |= ST_LGT;
		if(((uint16)val) == 0x8000) {
			ST |= ST_OV;
		}
	} else {
		ST |= ST_EQ;
	}
}

inline uint16 TMS9995::setst_sra_laec(int16 a, uint16 c)
{
	ST &= ~(ST_LGT | ST_AGT | ST_EQ | ST_C);
	if(c != 0) {
		a = arithmetic_right_shift(a, c-1);
		if(a & 1) {
			ST |= ST_C;
		}
		a = arithmetic_right_shift(a, 1);
	}
	if(a > 0) {
		ST |= ST_LGT | ST_AGT;
	} else if(a < 0) {
		ST |= ST_LGT;
	} else {
		ST |= ST_EQ;
	}
	return a;
}

inline uint16 TMS9995::setst_srl_laec(uint16 a,uint16 c)
{
	ST &= ~(ST_LGT | ST_AGT | ST_EQ | ST_C);
	if(c != 0) {
		a = logical_right_shift(a, c-1);
		if(a & 1) {
			ST |= ST_C;
		}
		a = logical_right_shift(a, 1);
	}
	if(((int16)a) > 0) {
		ST |= ST_LGT | ST_AGT;
	} else if(((int16)a) < 0) {
		ST |= ST_LGT;
	} else {
		ST |= ST_EQ;
	}
	return a;
}

inline uint16 TMS9995::setst_src_laec(uint16 a,uint16 c)
{
	ST &= ~(ST_LGT | ST_AGT | ST_EQ | ST_C);
	if(c != 0) {
		a = logical_right_shift(a, c) | (a << (16-c));
		if(a & 0x8000) {
			ST |= ST_C;
		}
	}
	if(((int16)a) > 0) {
		ST |= ST_LGT | ST_AGT;
	} else if(((int16)a) < 0) {
		ST |= ST_LGT;
	} else {
		ST |= ST_EQ;
	}
	return a;
}

inline uint16 TMS9995::setst_sla_laeco(uint16 a, uint16 c)
{
	ST &= ~(ST_LGT | ST_AGT | ST_EQ | ST_C | ST_OV);
	if(c != 0) {
		uint16 mask = 0xFFFF << (16-c-1);
		uint16 bits = a & mask;
		if(bits) {
			if(bits ^ mask) {
				ST |= ST_OV;
			}
		}
		a <<= c - 1;
		if(a & 0x8000) {
			ST |= ST_C;
		}
		a <<= 1;
	}
	if(((int16)a) > 0) {
		ST |= ST_LGT | ST_AGT;
	} else if(((int16)a) < 0) {
		ST |= ST_LGT;
	} else {
		ST |= ST_EQ;
	}
	return a;
}

#define STATE_VERSION	1

void TMS9995::save_state(FILEIO* state_fio)
{
	state_fio->FputUint32(STATE_VERSION);
	state_fio->FputInt32(this_device_id);
	
	state_fio->FputInt32(count);
	state_fio->FputInt32(period);
	state_fio->FputUint16(WP);
	state_fio->FputUint16(PC);
	state_fio->FputUint16(prevPC);
	state_fio->FputUint16(ST);
	state_fio->Fwrite(RAM, sizeof(RAM), 1);
	state_fio->FputUint8(irq_level);
	state_fio->FputUint8(int_state);
	state_fio->FputUint8(int_latch);
	state_fio->FputBool(int_pending);
	state_fio->FputBool(int_enabled);
	state_fio->FputUint16(dec_count);
	state_fio->FputUint16(dec_interval);
	state_fio->FputInt32(dec_timer);
	state_fio->FputBool(dec_enabled);
	state_fio->FputUint16(mode);
	state_fio->FputUint8(lastparity);
	state_fio->FputBool(nmi);
	state_fio->FputBool(mid);
	state_fio->FputBool(idle);
}

bool TMS9995::load_state(FILEIO* state_fio)
{
	if(state_fio->FgetUint32() != STATE_VERSION) {
		return false;
	}
	if(state_fio->FgetInt32() != this_device_id) {
		return false;
	}
	count = state_fio->FgetInt32();
	period = state_fio->FgetInt32();
	WP = state_fio->FgetUint16();
	PC = state_fio->FgetUint16();
	prevPC = state_fio->FgetUint16();
	ST = state_fio->FgetUint16();
	state_fio->Fread(RAM, sizeof(RAM), 1);
	irq_level = state_fio->FgetUint8();
	int_state = state_fio->FgetUint8();
	int_latch = state_fio->FgetUint8();
	int_pending = state_fio->FgetBool();
	int_enabled = state_fio->FgetBool();
	dec_count = state_fio->FgetUint16();
	dec_interval = state_fio->FgetUint16();
	dec_timer = state_fio->FgetInt32();
	dec_enabled = state_fio->FgetBool();
	mode = state_fio->FgetUint16();
	lastparity = state_fio->FgetUint8();
	nmi = state_fio->FgetBool();
	mid = state_fio->FgetBool();
	idle = state_fio->FgetBool();
	return true;
}

