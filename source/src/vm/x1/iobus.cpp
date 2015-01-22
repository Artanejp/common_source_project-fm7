/*
	SHARP X1 Emulator 'eX1'
	SHARP X1twin Emulator 'eX1twin'
	SHARP X1turbo Emulator 'eX1turbo'

	Author : Takeda.Toshiya
	Date   : 2009.03.14-

	[ 8bit i/o bus ]
*/

#include "iobus.h"
#include "io_wait.h"
#ifdef _X1TURBO_FEATURE
#include "io_wait_hireso.h"
#endif
#include "display.h"
#include "../../fileio.h"

void IOBUS::initialize()
{
	prev_clock = vram_wait_index = 0;
	column40 = true;
}

void IOBUS::reset()
{
	memset(vram, 0, sizeof(vram));
	vram_b = vram + 0x0000;
	vram_r = vram + 0x4000;
	vram_g = vram + 0x8000;
	vram_mode = signal = false;
	vdisp = 0;
#ifdef _X1TURBO_FEATURE
	memset(crtc_regs, 0, sizeof(crtc_regs));
	crtc_ch = 0;
	hireso = true;
#endif
}

void IOBUS::write_signal(int id, uint32 data, uint32 mask)
{
	// H -> L
	bool next = ((data & 0x20) != 0);
	if(signal && !next) {
		vram_mode = true;
	}
	signal = next;
	column40 = ((data & 0x40) != 0);
}

void IOBUS::write_io8w(uint32 addr, uint32 data, int* wait)
{
	write_port8(addr, data, false, wait);
}

uint32 IOBUS::read_io8w(uint32 addr, int* wait)
{
	return read_port8(addr, false, wait);
}

void IOBUS::write_dma_io8w(uint32 addr, uint32 data, int* wait)
{
	write_port8(addr, data, true, wait);
}

uint32 IOBUS::read_dma_io8w(uint32 addr, int* wait)
{
	return read_port8(addr, true, wait);
}

void IOBUS::write_port8(uint32 addr, uint32 data, bool is_dma, int* wait)
{
	// vram access
	switch(addr & 0xc000) {
	case 0x0000:
		if(vram_mode) {
			vram_b[addr & 0x3fff] = data;
			vram_r[addr & 0x3fff] = data;
			vram_g[addr & 0x3fff] = data;
			*wait = get_vram_wait();
			return;
		}
		break;
	case 0x4000:
		if(vram_mode) {
			vram_r[addr & 0x3fff] = data;
			vram_g[addr & 0x3fff] = data;
		} else {
			vram_b[addr & 0x3fff] = data;
		}
		*wait = get_vram_wait();
		return;
	case 0x8000:
		if(vram_mode) {
			vram_b[addr & 0x3fff] = data;
			vram_g[addr & 0x3fff] = data;
		} else {
			vram_r[addr & 0x3fff] = data;
		}
		*wait = get_vram_wait();
		return;
	case 0xc000:
		if(vram_mode) {
			vram_b[addr & 0x3fff] = data;
			vram_r[addr & 0x3fff] = data;
		} else {
			vram_g[addr & 0x3fff] = data;
		}
		*wait = get_vram_wait();
		return;
	}
#ifdef _X1TURBO_FEATURE
	if(addr == 0x1fd0) {
		int ofs = (data & 0x10) ? 0xc000 : 0;
		vram_b = vram + 0x0000 + ofs;
		vram_r = vram + 0x4000 + ofs;
		vram_g = vram + 0x8000 + ofs;
	} else if((addr & 0xff0f) == 0x1800) {
		crtc_ch = data;
	} else if((addr & 0xff0f) == 0x1801 && crtc_ch < 18) {
		crtc_regs[crtc_ch] = data;
		// update vram wait
		int ch_height = (crtc_regs[9] & 0x1f) + 1;
		int vt_total = ((crtc_regs[4] & 0x7f) + 1) * ch_height + (crtc_regs[5] & 0x1f);
		hireso = (vt_total > 400);
	}
#endif
	if(is_dma) {
		d_io->write_dma_io8(addr, data & 0xff);
	} else {
		d_io->write_io8(addr, data & 0xff);
	}
	switch(addr & 0xff00) {
	case 0x1900:	// sub cpu
	case 0x1b00:	// psg
	case 0x1c00:	// psg
		*wait = 1;
		break;
	default:
		*wait = 0;
		break;
	}
}

uint32 IOBUS::read_port8(uint32 addr, bool is_dma, int* wait)
{
	// vram access
	vram_mode = false;
	switch(addr & 0xc000) {
	case 0x4000:
		*wait = get_vram_wait();
		return vram_b[addr & 0x3fff];
	case 0x8000:
		*wait = get_vram_wait();
		return vram_r[addr & 0x3fff];
	case 0xc000:
		*wait = get_vram_wait();
		return vram_g[addr & 0x3fff];
	}
	uint32 val = is_dma ? d_io->read_dma_io8(addr) : d_io->read_io8(addr);;
	if((addr & 0xff0f) == 0x1a01) {
		// hack: cpu detects vblank
		if((vdisp & 0x80) && !(val & 0x80)) {
			d_display->write_signal(SIG_DISPLAY_DETECT_VBLANK, 1, 1);
		}
		vdisp = val;
	}
	switch(addr & 0xff00) {
	case 0x1900:	// sub cpu
	case 0x1b00:	// psg
	case 0x1c00:	// psg
		*wait = 1;
		break;
	default:
		*wait = 0;
		break;
	}
	return val & 0xff;
}

int IOBUS::get_vram_wait()
{
	vram_wait_index += passed_clock(prev_clock);
	vram_wait_index %= 2112;
	prev_clock = current_clock();
#ifdef _X1TURBO_FEATURE
	int tmp_index = (vram_wait_index + d_cpu->get_extra_clock()) % 2112; // consider dma access
	if(hireso) {
		return column40 ? vram_wait_40_hireso[tmp_index] : vram_wait_80_hireso[tmp_index];
	} else
#else
	#define tmp_index vram_wait_index
#endif
	return column40 ? vram_wait_40[tmp_index] : vram_wait_80[tmp_index];
}

#define STATE_VERSION	2

void IOBUS::save_state(FILEIO* state_fio)
{
	state_fio->FputUint32(STATE_VERSION);
	state_fio->FputInt32(this_device_id);
	
	state_fio->Fwrite(vram, sizeof(vram), 1);
	state_fio->FputBool(vram_mode);
	state_fio->FputBool(signal);
	state_fio->FputInt32((int)(vram_b - vram));
	state_fio->FputInt32((int)(vram_r - vram));
	state_fio->FputInt32((int)(vram_g - vram));
	state_fio->FputUint8(vdisp);
	state_fio->FputUint32(prev_clock);
	state_fio->FputUint32(vram_wait_index);
	state_fio->FputBool(column40);
#ifdef _X1TURBO_FEATURE
	state_fio->Fwrite(crtc_regs, sizeof(crtc_regs), 1);
	state_fio->FputInt32(crtc_ch);
	state_fio->FputBool(hireso);
#endif
}

bool IOBUS::load_state(FILEIO* state_fio)
{
	if(state_fio->FgetUint32() != STATE_VERSION) {
		return false;
	}
	if(state_fio->FgetInt32() != this_device_id) {
		return false;
	}
	state_fio->Fread(vram, sizeof(vram), 1);
	vram_mode = state_fio->FgetBool();
	signal = state_fio->FgetBool();
	vram_b = vram + state_fio->FgetInt32();
	vram_r = vram + state_fio->FgetInt32();
	vram_g = vram + state_fio->FgetInt32();
	vdisp = state_fio->FgetUint8();
	prev_clock = state_fio->FgetUint32();
	vram_wait_index = state_fio->FgetUint32();
	column40 = state_fio->FgetBool();
#ifdef _X1TURBO_FEATURE
	state_fio->Fread(crtc_regs, sizeof(crtc_regs), 1);
	crtc_ch = state_fio->FgetInt32();
	hireso = state_fio->FgetBool();
#endif
	return true;
}

