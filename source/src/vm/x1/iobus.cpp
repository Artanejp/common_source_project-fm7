/*
	SHARP X1 Emulator 'eX1'
	SHARP X1twin Emulator 'eX1twin'
	SHARP X1turbo Emulator 'eX1turbo'
	SHARP X1turboZ Emulator 'eX1turboZ'

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
#ifdef USE_DEBUGGER
#include "../debugger.h"
#endif

#ifdef _X1TURBOZ
#define AEN	((zmode1 & 0x80) != 0)
#define APEN	((zmode2 & 0x80) != 0)
#define APRD	((zmode2 & 0x08) != 0)
#endif

void IOBUS::initialize()
{
	prev_clock = vram_wait_index = 0;
	column40 = true;
	memset(vram, 0, sizeof(vram));
#ifdef USE_DEBUGGER
	d_debugger->set_device_name(_T("Debugger (I/O Bus)"));
	d_debugger->set_context_mem(this);
	d_debugger->set_context_io(vm->dummy);
#endif
}

void IOBUS::reset()
{
	vram_ofs_b = 0x0000;
	vram_ofs_r = 0x4000;
	vram_ofs_g = 0x8000;
	vram_mode = signal = false;
	vdisp = 0;
#ifdef _X1TURBO_FEATURE
	memset(crtc_regs, 0, sizeof(crtc_regs));
	crtc_ch = 0;
	hireso = true;
#endif
}

void IOBUS::write_signal(int id, uint32_t data, uint32_t mask)
{
	// H -> L
	bool next = ((data & 0x20) != 0);
	if(signal && !next) {
		vram_mode = true;
	}
	signal = next;
	column40 = ((data & 0x40) != 0);
}

void IOBUS::write_io8w(uint32_t addr, uint32_t data, int* wait)
{
	write_port8(addr, data, false, wait);
}

uint32_t IOBUS::read_io8w(uint32_t addr, int* wait)
{
	return read_port8(addr, false, wait);
}

void IOBUS::write_dma_io8w(uint32_t addr, uint32_t data, int* wait)
{
	write_port8(addr, data, true, wait);
}

uint32_t IOBUS::read_dma_io8w(uint32_t addr, int* wait)
{
	return read_port8(addr, true, wait);
}

void IOBUS::write_via_debugger_data8(uint32_t addr, uint32_t data)
{
	vram[addr] = data;
}

uint32_t IOBUS::read_via_debugger_data8(uint32_t addr)
{
	return vram[addr];
}

void IOBUS::write_port8(uint32_t addr, uint32_t data, bool is_dma, int* wait)
{
	// vram access
	switch(addr & 0xc000) {
	case 0x0000:
		if(vram_mode) {
#ifdef USE_DEBUGGER
			if(d_debugger->now_device_debugging) {
				d_debugger->write_via_debugger_data8(vram_ofs_b + (addr & 0x3fff), data);
				d_debugger->write_via_debugger_data8(vram_ofs_r + (addr & 0x3fff), data);
				d_debugger->write_via_debugger_data8(vram_ofs_g + (addr & 0x3fff), data);
			} else
#endif
			{
				this->write_via_debugger_data8(vram_ofs_b + (addr & 0x3fff), data);
				this->write_via_debugger_data8(vram_ofs_r + (addr & 0x3fff), data);
				this->write_via_debugger_data8(vram_ofs_g + (addr & 0x3fff), data);
			}
			*wait = get_vram_wait();
			return;
		}
		break;
	case 0x4000:
		if(vram_mode) {
#ifdef USE_DEBUGGER
			if(d_debugger->now_device_debugging) {
				d_debugger->write_via_debugger_data8(vram_ofs_r + (addr & 0x3fff), data);
				d_debugger->write_via_debugger_data8(vram_ofs_g + (addr & 0x3fff), data);
			} else
#endif
			{
				this->write_via_debugger_data8(vram_ofs_r + (addr & 0x3fff), data);
				this->write_via_debugger_data8(vram_ofs_g + (addr & 0x3fff), data);
			}
		} else {
#ifdef USE_DEBUGGER
			if(d_debugger->now_device_debugging) {
				d_debugger->write_via_debugger_data8(vram_ofs_b + (addr & 0x3fff), data);
			} else
#endif
			{
				this->write_via_debugger_data8(vram_ofs_b + (addr & 0x3fff), data);
			}
		}
		*wait = get_vram_wait();
		return;
	case 0x8000:
		if(vram_mode) {
#ifdef USE_DEBUGGER
			if(d_debugger->now_device_debugging) {
				d_debugger->write_via_debugger_data8(vram_ofs_b + (addr & 0x3fff), data);
				d_debugger->write_via_debugger_data8(vram_ofs_g + (addr & 0x3fff), data);
			} else
#endif
			{
				this->write_via_debugger_data8(vram_ofs_b + (addr & 0x3fff), data);
				this->write_via_debugger_data8(vram_ofs_g + (addr & 0x3fff), data);
			}
		} else {
#ifdef USE_DEBUGGER
			if(d_debugger->now_device_debugging) {
				d_debugger->write_via_debugger_data8(vram_ofs_r + (addr & 0x3fff), data);
			} else
#endif
			{
				this->write_via_debugger_data8(vram_ofs_r + (addr & 0x3fff), data);
			}
		}
		*wait = get_vram_wait();
		return;
	case 0xc000:
		if(vram_mode) {
#ifdef USE_DEBUGGER
			if(d_debugger->now_device_debugging) {
				d_debugger->write_via_debugger_data8(vram_ofs_b + (addr & 0x3fff), data);
				d_debugger->write_via_debugger_data8(vram_ofs_r + (addr & 0x3fff), data);
			} else
#endif
			{
				this->write_via_debugger_data8(vram_ofs_b + (addr & 0x3fff), data);
				this->write_via_debugger_data8(vram_ofs_r + (addr & 0x3fff), data);
			}
		} else {
#ifdef USE_DEBUGGER
			if(d_debugger->now_device_debugging) {
				d_debugger->write_via_debugger_data8(vram_ofs_g + (addr & 0x3fff), data);
			} else
#endif
			{
				this->write_via_debugger_data8(vram_ofs_g + (addr & 0x3fff), data);
			}
		}
		*wait = get_vram_wait();
		return;
	}
#ifdef _X1TURBO_FEATURE
	if(addr == 0x1fd0) {
		int ofs = (data & 0x10) ? 0xc000 : 0;
		vram_ofs_b = 0x0000 + ofs;
		vram_ofs_r = 0x4000 + ofs;
		vram_ofs_g = 0x8000 + ofs;
	} else if((addr & 0xff0f) == 0x1800) {
		crtc_ch = data;
	} else if((addr & 0xff0f) == 0x1801 && crtc_ch < 18) {
		crtc_regs[crtc_ch] = data;
		// update vram wait
		int ch_height = (crtc_regs[9] & 0x1f) + 1;
		int vt_total = ((crtc_regs[4] & 0x7f) + 1) * ch_height + (crtc_regs[5] & 0x1f);
		hireso = (vt_total > 400);
#ifdef _X1TURBOZ
	} else if(addr == 0x1fb0) {
		zmode1 = data;
	} else if(addr == 0x1fc5) {
		if(AEN) {
			zmode2 = data;
		}
#endif
	}
#endif
	if(is_dma) {
		d_io->write_dma_io8(addr, data & 0xff);
	} else {
		d_io->write_io8(addr, data & 0xff);
	}
	switch(addr & 0xff00) {
#ifdef _X1TURBOZ
	case 0x1000:	// analog palette
	case 0x1100:
	case 0x1200:
		if(AEN && APEN && !APRD) {
//		if(AEN) {
			*wait = get_vram_wait(); // temporary
		} else {
			*wait = 0;
		}
		break;
#endif
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

uint32_t IOBUS::read_port8(uint32_t addr, bool is_dma, int* wait)
{
	// vram access
	vram_mode = false;
	switch(addr & 0xc000) {
	case 0x4000:
		*wait = get_vram_wait();
#ifdef USE_DEBUGGER
		if(d_debugger->now_device_debugging) {
			return d_debugger->read_via_debugger_data8(vram_ofs_b + (addr & 0x3fff));
		}
#endif
		return this->read_via_debugger_data8(vram_ofs_b + (addr & 0x3fff));
	case 0x8000:
		*wait = get_vram_wait();
#ifdef USE_DEBUGGER
		if(d_debugger->now_device_debugging) {
			return d_debugger->read_via_debugger_data8(vram_ofs_r + (addr & 0x3fff));
		}
#endif
		return this->read_via_debugger_data8(vram_ofs_r + (addr & 0x3fff));
	case 0xc000:
		*wait = get_vram_wait();
#ifdef USE_DEBUGGER
		if(d_debugger->now_device_debugging) {
			return d_debugger->read_via_debugger_data8(vram_ofs_g + (addr & 0x3fff));
		}
#endif
		return this->read_via_debugger_data8(vram_ofs_g + (addr & 0x3fff));
	}
	uint32_t val = is_dma ? d_io->read_dma_io8(addr) : d_io->read_io8(addr);;
	if((addr & 0xff0f) == 0x1a01) {
		// hack: cpu detects vblank
		if((vdisp & 0x80) && !(val & 0x80)) {
			d_display->write_signal(SIG_DISPLAY_DETECT_VBLANK, 1, 1);
		}
		vdisp = val;
	}
	switch(addr & 0xff00) {
#ifdef _X1TURBOZ
	case 0x1000:	// analog palette
	case 0x1100:
	case 0x1200:
		if(AEN && APEN && APRD) {
//		if(AEN) {
			*wait = get_vram_wait(); // temporary
		} else {
			*wait = 0;
		}
		break;
#endif
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
	vram_wait_index += get_passed_clock(prev_clock);
	vram_wait_index %= 2112;
	prev_clock = get_current_clock();
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

#define STATE_VERSION	3

bool IOBUS::process_state(FILEIO* state_fio, bool loading)
{
	if(!state_fio->StateCheckUint32(STATE_VERSION)) {
		return false;
	}
	if(!state_fio->StateCheckInt32(this_device_id)) {
		return false;
	}
	state_fio->StateArray(vram, sizeof(vram), 1);
	state_fio->StateValue(vram_mode);
	state_fio->StateValue(signal);
	state_fio->StateValue(vram_ofs_b);
	state_fio->StateValue(vram_ofs_r);
	state_fio->StateValue(vram_ofs_g);
	state_fio->StateValue(vdisp);
	state_fio->StateValue(prev_clock);
	state_fio->StateValue(vram_wait_index);
	state_fio->StateValue(column40);
#ifdef _X1TURBO_FEATURE
	state_fio->StateArray(crtc_regs, sizeof(crtc_regs), 1);
	state_fio->StateValue(crtc_ch);
	state_fio->StateValue(hireso);
#ifdef _X1TURBOZ
	state_fio->StateValue(zmode1);
	state_fio->StateValue(zmode2);
#endif
#endif
	return true;
}

