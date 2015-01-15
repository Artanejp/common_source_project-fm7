/*
	TOMY PyuTa Emulator 'ePyuTa'

	Author : Takeda.Toshiya
	Date   : 2007.07.15 -

	[ memory ]
*/

#include "memory.h"
#include "../datarec.h"
#include "../tms9995.h"
#include "../../fileio.h"

#define SET_BANK(s, e, w, r) { \
	int sb = (s) >> 12, eb = (e) >> 12; \
	for(int i = sb; i <= eb; i++) { \
		if((w) == wdmy) { \
			wbank[i] = wdmy; \
		} else { \
			wbank[i] = (w) + 0x1000 * (i - sb); \
		} \
		if((r) == rdmy) { \
			rbank[i] = rdmy; \
		} else { \
			rbank[i] = (r) + 0x1000 * (i - sb); \
		} \
	} \
}

#define ENABLE_CART() { \
	if(ctype == 3) { \
		SET_BANK(0x0000, 0x3fff, wdmy, ipl); \
		SET_BANK(0x4000, 0xbfff, wdmy, cart); \
	} else { \
		SET_BANK(0x0000, 0x7fff, wdmy, ipl); \
		SET_BANK(0x8000, 0xbfff, wdmy, cart); \
	} \
	cart_enabled = true; \
}

#define DISABLE_CART() { \
	SET_BANK(0x0000, 0x7fff, wdmy, ipl); \
	SET_BANK(0x8000, 0xbfff, wdmy, basic); \
	cart_enabled = false; \
}

void MEMORY::initialize()
{
	memset(ipl, 0xff, sizeof(ipl));
	memset(basic, 0xff, sizeof(basic));
	memset(cart, 0xff, sizeof(cart));
	memset(rdmy, 0xff, sizeof(rdmy));
	
	// load rom images
	FILEIO* fio = new FILEIO();
	if(fio->Fopen(emu->bios_path(_T("IPL.ROM")), FILEIO_READ_BINARY)) {
		fio->Fread(ipl, sizeof(ipl), 1);
		fio->Fclose();
	}
	if(fio->Fopen(emu->bios_path(_T("BASIC.ROM")), FILEIO_READ_BINARY)) {
		fio->Fread(basic, sizeof(basic), 1);
		fio->Fclose();
		has_extrom = true;
	} else {
		has_extrom = false;
	}
	delete fio;
	
	// set memory map
	DISABLE_CART();
	SET_BANK(0xc000, 0xffff, wdmy, rdmy);
	
	// get keyboard and joystick buffers
	key = emu->key_buffer();
	joy = emu->joy_buffer();
	
	ctype = 0;
}

void MEMORY::reset()
{
	cmt_signal = cmt_remote = false;
}

void MEMORY::write_data8(uint32 addr, uint32 data)
{
	addr &= 0xffff;
	if(addr < 0xe000) {
		wbank[addr >> 12][addr & 0xfff] = data;
	} else if(addr == 0xe000) {
		d_vdp->write_io8(0, data);
	} else if(addr == 0xe002) {
		d_vdp->write_io8(1, data);
	} else if(addr == 0xe108 && has_extrom) {
		DISABLE_CART();
	} else if(addr == 0xe10c && has_extrom) {
		ENABLE_CART();
	} else if(addr == 0xe810) {
		// printer data
	} else if(addr == 0xe840) {
		// printer strobe
	} else if(addr == 0xe200) {
		d_psg->write_io8(0, data);
	} else if(0xee00 <= addr && addr <= 0xeeff) {
		// cmt
		if(!(addr & 0x1f)) {
			bool signal = ((addr & 0x20) != 0);
			bool remote = (data == 0);
			switch((addr >> 6) & 3) {
			case 0:
				if(cmt_signal != signal) {
					d_cmt->write_signal(SIG_DATAREC_OUT, signal ? 1 : 0, 1);
					cmt_signal = signal;
				}
				break;
			case 1:
				if(cmt_remote != remote) {
					if(!remote) {
						d_cpu->write_signal(SIG_TMS9995_INT4, 0, 1);
					}
					d_cmt->write_signal(SIG_DATAREC_REMOTE, remote ? 1 : 0, 1);
					cmt_remote = remote;
				}
				break;
			}
		}
	}
}

uint32 MEMORY::read_data8(uint32 addr)
{
	uint32 val = 0;
	
	addr &= 0xffff;
	if(addr < 0xe000) {
		return rbank[addr >> 12][addr & 0xfff];
	} else if(addr == 0xe000) {
		return d_vdp->read_io8(0);
	} else if(addr == 0xe002) {
		return d_vdp->read_io8(1);
	} else if(addr == 0xe110) {
		return 0;	// tutor 0x42 ???
	} else if(addr == 0xe800) {
		// PyuTa Jr. JOY1
		if(joy[0] & 0x10) val |= 0x04;	// JOY1 B1
		if(joy[0] & 0x20) val |= 0x08;	// JOY1 B2
		if(joy[0] & 0x02) val |= 0x10;	// JOY1 DOWN
		if(joy[0] & 0x04) val |= 0x20;	// JOY1 LEFT
		if(joy[0] & 0x01) val |= 0x40;	// JOY1 UP
		if(joy[0] & 0x08) val |= 0x80;	// JOY1 RIGHT
		return val;
	} else if(addr == 0xe820) {
		// printer busy
		return 0;
	} else if(addr == 0xea00) {
		// PyuTa Jr. KEY
		if(key[0x0d]             ) val |= 0x04;	// RETURN
		if(key[0x20]             ) val |= 0x08;	// PALLETE -> SPACE
		if(key[0x71]             ) val |= 0x10;	// MOD -> F2
		if(key[0x70]             ) val |= 0x20;	// MON -> F1
		if(key[0x31] || key[0x61]) val |= 0x40;	// 1
		if(key[0x32] || key[0x62]) val |= 0x80;	// 2
		return val;
	} else if(addr == 0xec00) {
		// PyuTa Jr. KEY
		if(key[0x5a]) val |= 0x04;	// COLOR SELECT << -> Z
		if(key[0x58]) val |= 0x08;	// COLOR SELECT >> -> X
		if(key[0x25]) val |= 0x10;	// LEFT
		if(key[0x26]) val |= 0x20;	// UP
		if(key[0x28]) val |= 0x40;	// DOWN
		if(key[0x27]) val |= 0x80;	// RIGHT
		return val;
	} else if(addr == 0xee00) {
		// PyuTa Jr. JOY2
		if(joy[1] & 0x10) val |= 0x04;	// JOY2 B1
		if(joy[1] & 0x20) val |= 0x08;	// JOY2 B2
		if(joy[1] & 0x02) val |= 0x10;	// JOY2 DOWN
		if(joy[1] & 0x04) val |= 0x20;	// JOY2 LEFT
		if(joy[1] & 0x01) val |= 0x40;	// JOY2 UP
		if(joy[1] & 0x08) val |= 0x80;	// JOY2 RIGHT
		return val;
	} else {
		return 0xff;	// pull up ?
	}
}

void MEMORY::write_io8(uint32 addr, uint32 data)
{
	// CRU OUT
}

uint32 MEMORY::read_io8(uint32 addr)
{
	// CRU IN
	uint32 val = 0;
	
	switch(addr) {
	case 0xec0:
		if(key[0x31] || key[0x61]) val |= 0x01;	// 1
		if(key[0x32] || key[0x62]) val |= 0x02;	// 2
		if(key[0x51]             ) val |= 0x04;	// Q
		if(key[0x57]             ) val |= 0x08;	// W
		if(key[0x41]             ) val |= 0x10;	// A
		if(key[0x53]             ) val |= 0x20;	// S
		if(key[0x5a]             ) val |= 0x40;	// Z
		if(key[0x58]             ) val |= 0x80;	// X
		return val;
	case 0xec1:
		if(key[0x33] || key[0x63]) val |= 0x01;	// 3
		if(key[0x34] || key[0x64]) val |= 0x02;	// 4
		if(key[0x45]             ) val |= 0x04;	// E
		if(key[0x52]             ) val |= 0x08;	// R
		if(key[0x44]             ) val |= 0x10;	// D
		if(key[0x46]             ) val |= 0x20;	// F
		if(key[0x43]             ) val |= 0x40;	// C
		if(key[0x56]             ) val |= 0x80;	// V
		return val;
	case 0xec2:
		if(key[0x35] || key[0x65]) val |= 0x01;	// 5
		if(key[0x36] || key[0x66]) val |= 0x02;	// 6
		if(key[0x54]             ) val |= 0x04;	// T
		if(key[0x59]             ) val |= 0x08;	// Y
		if(key[0x47]             ) val |= 0x10;	// G
		if(key[0x48]             ) val |= 0x20;	// H
		if(key[0x42]             ) val |= 0x40;	// B
		if(key[0x4e]             ) val |= 0x80;	// N
		return val;
	case 0xec3:
		if(key[0x37] || key[0x67]) val |= 0x01;	// 7
		if(key[0x38] || key[0x68]) val |= 0x02;	// 8
		if(key[0x39] || key[0x69]) val |= 0x04;	// 9
		if(key[0x55]             ) val |= 0x08;	// U
		if(key[0x49]             ) val |= 0x10;	// I
		if(key[0x4a]             ) val |= 0x20;	// J
		if(key[0x4b]             ) val |= 0x40;	// K
		if(key[0x4d]             ) val |= 0x80;	// M
		return val;
	case 0xec4:
		if(key[0x30] || key[0x60]      ) val |= 0x01;	// 0
		if(key[0xbd]                   ) val |= 0x02;	// -
		if(key[0x4f] || (joy[0] & 0x10)) val |= 0x04;	// O	JOY1 B1
		if(key[0x50] || (joy[0] & 0x20)) val |= 0x08;	// P	JOY1 B2
		if(key[0x4c] || (joy[0] & 0x02)) val |= 0x10;	// L	JOY1 DOWN
		if(key[0xbb] || (joy[0] & 0x04)) val |= 0x20;	// ;	JOY1 LEFT
		if(key[0xbc] || (joy[0] & 0x01)) val |= 0x40;	// ,	JOY1 UP
		if(key[0xbe] || (joy[0] & 0x08)) val |= 0x80;	// .	JOY1 RIGHT
		return val;
	case 0xec5:
		if(key[0xdc] || (joy[1] & 0x10)) val |= 0x04;	// YEN	JOY2 B1
		if(key[0xc0] || (joy[1] & 0x20)) val |= 0x08;	// @	JOY2 B2
		if(key[0xba] || (joy[1] & 0x02)) val |= 0x10;	// :	JOY2 DOWN
		if(key[0xdd] || (joy[1] & 0x04)) val |= 0x20;	// ]	JOY2 LEFT
		if(key[0xbf] || (joy[1] & 0x01)) val |= 0x40;	// /	JOY2 UP
		if(key[0xe2] || (joy[1] & 0x08)) val |= 0x80;	// _	JOY2 RIGHT
		return val;
	case 0xec6:
		if(key[0x11]) val |= 0x02;	// EISUU -> CTRL
		if(key[0x10]) val |= 0x04;	// KIGOU -> SHIFT
		if(key[0x70]) val |= 0x08;	// MON -> F1
		if(key[0x0d]) val |= 0x10;	// RETURN
		if(key[0x71]) val |= 0x40;	// MOD -> F2
		if(key[0x20]) val |= 0x80;	// SPACE
		return val;
	case 0xec7:
		if(key[0x25]) val |= 0x01;	// LEFT
		if(key[0x26]) val |= 0x02;	// UP
		if(key[0x28]) val |= 0x04;	// DOWN
		if(key[0x27]) val |= 0x08;	// RIGHT
		return val;
	case 0xed0:
		// cmt
		return cmt_signal ? 1 : 0;
	}
	return 0xff;	// pull down ?
}

void MEMORY::write_signal(int id, uint32 data, uint32 mask)
{
	// from cmt
	bool signal = ((data & mask) != 0);
	if(cmt_signal != signal) {
		if(cmt_remote) {
			d_cpu->write_signal(SIG_TMS9995_INT4, signal ? 1 : 0, 1);
		}
		cmt_signal = signal;
	}
}

void MEMORY::open_cart(_TCHAR* file_path)
{
	// open cart
	FILEIO* fio = new FILEIO();
	
	if(fio->Fopen(file_path, FILEIO_READ_BINARY)) {
		// 8kb
		ctype = fio->Fread(cart, 0x2000, 1);
		memcpy(cart + 0x2000, cart, 0x2000);
		// 16kb
		ctype += fio->Fread(cart + 0x2000, 0x2000, 1);
		// 32kb
		ctype += fio->Fread(cart + 0x4000, 0x4000, 1);
		fio->Fclose();
		
		ENABLE_CART();
	}
	delete fio;
}

void MEMORY::close_cart()
{
	ctype = 0;
	DISABLE_CART();
}

#define STATE_VERSION	1

void MEMORY::save_state(FILEIO* state_fio)
{
	state_fio->FputUint32(STATE_VERSION);
	state_fio->FputInt32(this_device_id);
	
	state_fio->FputBool(cmt_signal);
	state_fio->FputBool(cmt_remote);
	state_fio->FputBool(has_extrom);
	state_fio->FputBool(cart_enabled);
	state_fio->FputInt32(ctype);
}

bool MEMORY::load_state(FILEIO* state_fio)
{
	if(state_fio->FgetUint32() != STATE_VERSION) {
		return false;
	}
	if(state_fio->FgetInt32() != this_device_id) {
		return false;
	}
	cmt_signal = state_fio->FgetBool();
	cmt_remote = state_fio->FgetBool();
	has_extrom = state_fio->FgetBool();
	cart_enabled = state_fio->FgetBool();
	ctype = state_fio->FgetInt32();
	
	if(cart_enabled) {
		ENABLE_CART();
	} else {
		DISABLE_CART();
	}
	return true;
}

