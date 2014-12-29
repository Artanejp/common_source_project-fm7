/*
	SHARP X1 Emulator 'eX1'
	SHARP X1twin Emulator 'eX1twin'
	SHARP X1turbo Emulator 'eX1turbo'

	Author : Takeda.Toshiya
	Date   : 2009.03.14-

	[ 8bit i/o bus ]
*/

#include "io.h"
#include "io_wait.h"
#ifdef _X1TURBO_FEATURE
#include "io_wait_hireso.h"
#endif
#include "display.h"
#include "../../fileio.h"

void IO::initialize()
{
	prev_clock = vram_wait_index = 0;
	column40 = true;
}

void IO::reset()
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

void IO::write_signal(int id, uint32 data, uint32 mask)
{
	// H -> L
	bool next = ((data & 0x20) != 0);
	if(signal && !next) {
		vram_mode = true;
	}
	signal = next;
	column40 = ((data & 0x40) != 0);
}

void IO::write_io8w(uint32 addr, uint32 data, int* wait)
{
	write_port8(addr, data, false, wait);
}

uint32 IO::read_io8w(uint32 addr, int* wait)
{
	return read_port8(addr, false, wait);
}

void IO::write_dma_io8w(uint32 addr, uint32 data, int* wait)
{
	write_port8(addr, data, true, wait);
}

uint32 IO::read_dma_io8w(uint32 addr, int* wait)
{
	return read_port8(addr, true, wait);
}

void IO::write_port8(uint32 addr, uint32 data, bool is_dma, int* wait)
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
	// i/o
	uint32 laddr = addr & IO_ADDR_MASK, haddr = addr & ~IO_ADDR_MASK;
	uint32 addr2 = haddr | wr_table[laddr].addr;
#ifdef _IO_DEBUG_LOG
	if(!wr_table[laddr].dev->this_device_id && !wr_table[laddr].is_flipflop) {
		emu->out_debug_log("UNKNOWN:\t");
	}
	emu->out_debug_log("%6x\tOUT8\t%4x,%2x\n", get_cpu_pc(0), addr, data);
#endif
	if(wr_table[laddr].is_flipflop) {
		rd_table[laddr].value = data & 0xff;
	} else if(is_dma) {
		wr_table[laddr].dev->write_dma_io8(addr2, data & 0xff);
	} else {
		wr_table[laddr].dev->write_io8(addr2, data & 0xff);
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

uint32 IO::read_port8(uint32 addr, bool is_dma, int* wait)
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
	// i/o
	uint32 laddr = addr & IO_ADDR_MASK, haddr = addr & ~IO_ADDR_MASK;
	uint32 addr2 = haddr | rd_table[laddr].addr;
	uint32 val = rd_table[laddr].value_registered ? rd_table[laddr].value : is_dma ? rd_table[laddr].dev->read_dma_io8(addr2) : rd_table[laddr].dev->read_io8(addr2);
	if((addr2 & 0xff0f) == 0x1a01) {
		// hack: cpu detects vblank
		if((vdisp & 0x80) && !(val & 0x80)) {
			rd_table[0x1000].dev->write_signal(SIG_DISPLAY_DETECT_VBLANK, 1, 1);
		}
		vdisp = val;
	}
#ifdef _IO_DEBUG_LOG
	if(!rd_table[laddr].dev->this_device_id && !rd_table[laddr].value_registered) {
		emu->out_debug_log("UNKNOWN:\t");
	}
	emu->out_debug_log("%6x\tIN8\t%4x = %2x\n", get_cpu_pc(0), addr, val);
#endif
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

int IO::get_vram_wait()
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

// register

void IO::set_iomap_single_r(uint32 addr, DEVICE* device)
{
	rd_table[addr & IO_ADDR_MASK].dev = device;
	rd_table[addr & IO_ADDR_MASK].addr = addr & IO_ADDR_MASK;
}

void IO::set_iomap_single_w(uint32 addr, DEVICE* device)
{
	wr_table[addr & IO_ADDR_MASK].dev = device;
	wr_table[addr & IO_ADDR_MASK].addr = addr & IO_ADDR_MASK;
}

void IO::set_iomap_single_rw(uint32 addr, DEVICE* device)
{
	set_iomap_single_r(addr, device);
	set_iomap_single_w(addr, device);
}

void IO::set_iomap_alias_r(uint32 addr, DEVICE* device, uint32 alias)
{
	rd_table[addr & IO_ADDR_MASK].dev = device;
	rd_table[addr & IO_ADDR_MASK].addr = alias & IO_ADDR_MASK;
}

void IO::set_iomap_alias_w(uint32 addr, DEVICE* device, uint32 alias)
{
	wr_table[addr & IO_ADDR_MASK].dev = device;
	wr_table[addr & IO_ADDR_MASK].addr = alias & IO_ADDR_MASK;
}

void IO::set_iomap_alias_rw(uint32 addr, DEVICE* device, uint32 alias)
{
	set_iomap_alias_r(addr, device, alias);
	set_iomap_alias_w(addr, device, alias);
}

void IO::set_iomap_range_r(uint32 s, uint32 e, DEVICE* device)
{
	for(uint32 i = s; i <= e; i++) {
		rd_table[i & IO_ADDR_MASK].dev = device;
		rd_table[i & IO_ADDR_MASK].addr = i & IO_ADDR_MASK;
	}
}

void IO::set_iomap_range_w(uint32 s, uint32 e, DEVICE* device)
{
	for(uint32 i = s; i <= e; i++) {
		wr_table[i & IO_ADDR_MASK].dev = device;
		wr_table[i & IO_ADDR_MASK].addr = i & IO_ADDR_MASK;
	}
}

void IO::set_iomap_range_rw(uint32 s, uint32 e, DEVICE* device)
{
	set_iomap_range_r(s, e, device);
	set_iomap_range_w(s, e, device);
}

void IO::set_iovalue_single_r(uint32 addr, uint32 value) {
	rd_table[addr & IO_ADDR_MASK].value = value;
	rd_table[addr & IO_ADDR_MASK].value_registered = true;
}

void IO::set_iovalue_range_r(uint32 s, uint32 e, uint32 value)
{
	for(uint32 i = s; i <= e; i++) {
		rd_table[i & IO_ADDR_MASK].value = value;
		rd_table[i & IO_ADDR_MASK].value_registered = true;
	}
}

void IO::set_flipflop_single_rw(uint32 addr, uint32 value)
{
	wr_table[addr & IO_ADDR_MASK].is_flipflop = true;
	rd_table[addr & IO_ADDR_MASK].value = value;
	rd_table[addr & IO_ADDR_MASK].value_registered = true;
}

void IO::set_flipflop_range_rw(uint32 s, uint32 e, uint32 value)
{
	for(uint32 i = s; i <= e; i++) {
		wr_table[i & IO_ADDR_MASK].is_flipflop = true;
		rd_table[i & IO_ADDR_MASK].value = value;
		rd_table[i & IO_ADDR_MASK].value_registered = true;
	}
}

#define STATE_VERSION	1

void IO::save_state(FILEIO* state_fio)
{
	state_fio->FputUint32(STATE_VERSION);
	state_fio->FputInt32(this_device_id);
	
	for(int i = 0; i < IO_ADDR_MAX; i++) {
		state_fio->FputUint32(rd_table[i].value);
	}
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

bool IO::load_state(FILEIO* state_fio)
{
	if(state_fio->FgetUint32() != STATE_VERSION) {
		return false;
	}
	if(state_fio->FgetInt32() != this_device_id) {
		return false;
	}
	for(int i = 0; i < IO_ADDR_MAX; i++) {
		rd_table[i].value = state_fio->FgetUint32();
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

