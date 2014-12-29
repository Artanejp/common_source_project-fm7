/*
	Skelton for retropc emulator

	Author : Takeda.Toshiya
	Date   : 2008.12.29 -

	[ i/o bus ]
*/

#include "io.h"
#include "../fileio.h"

void IO::write_io8(uint32 addr, uint32 data)
{
	write_port8(addr, data, false);
}

uint32 IO::read_io8(uint32 addr)
{
	return read_port8(addr, false);
}

void IO::write_io16(uint32 addr, uint32 data)
{
	write_port16(addr, data, false);
}

uint32 IO::read_io16(uint32 addr)
{
	return read_port16(addr, false);
}

void IO::write_io32(uint32 addr, uint32 data)
{
	write_port32(addr, data, false);
}

uint32 IO::read_io32(uint32 addr)
{
	return read_port32(addr, false);
}

void IO::write_io8w(uint32 addr, uint32 data, int* wait)
{
	*wait = wr_table[addr & IO_ADDR_MASK].wait;
	write_port8(addr, data, false);
}

uint32 IO::read_io8w(uint32 addr, int* wait)
{
	*wait = rd_table[addr & IO_ADDR_MASK].wait;
	return read_port8(addr, false);
}

void IO::write_io16w(uint32 addr, uint32 data, int* wait)
{
	*wait = wr_table[addr & IO_ADDR_MASK].wait;
	write_port16(addr, data, false);
}

uint32 IO::read_io16w(uint32 addr, int* wait)
{
	*wait = rd_table[addr & IO_ADDR_MASK].wait;
	return read_port16(addr, false);
}

void IO::write_io32w(uint32 addr, uint32 data, int* wait)
{
	*wait = wr_table[addr & IO_ADDR_MASK].wait;
	write_port32(addr, data, false);
}

uint32 IO::read_io32w(uint32 addr, int* wait)
{
	*wait = rd_table[addr & IO_ADDR_MASK].wait;
	return read_port32(addr, false);
}

void IO::write_dma_io8(uint32 addr, uint32 data)
{
	write_port8(addr, data, true);
}

uint32 IO::read_dma_io8(uint32 addr)
{
	return read_port8(addr, true);
}

void IO::write_dma_io16(uint32 addr, uint32 data)
{
	write_port16(addr, data, true);
}

uint32 IO::read_dma_io16(uint32 addr)
{
	return read_port16(addr, true);
}

void IO::write_dma_io32(uint32 addr, uint32 data)
{
	write_port32(addr, data, true);
}

uint32 IO::read_dma_io32(uint32 addr)
{
	return read_port32(addr, true);
}

void IO::write_port8(uint32 addr, uint32 data, bool is_dma)
{
	uint32 laddr = addr & IO_ADDR_MASK, haddr = addr & ~IO_ADDR_MASK;
	uint32 addr2 = haddr | wr_table[laddr].addr;
#ifdef _IO_DEBUG_LOG
	if(!wr_table[laddr].dev->this_device_id && !wr_table[laddr].is_flipflop) {
		emu->out_debug_log(_T("UNKNOWN:\t"));
	}
	emu->out_debug_log(_T("%06x\tOUT8\t%04x,%02x\n"), get_cpu_pc(0), addr, data);
#endif
	if(wr_table[laddr].is_flipflop) {
		rd_table[laddr].value = data & 0xff;
	} else if(is_dma) {
		wr_table[laddr].dev->write_dma_io8(addr2, data & 0xff);
	} else {
		wr_table[laddr].dev->write_io8(addr2, data & 0xff);
	}
}

uint32 IO::read_port8(uint32 addr, bool is_dma)
{
	uint32 laddr = addr & IO_ADDR_MASK, haddr = addr & ~IO_ADDR_MASK;
	uint32 addr2 = haddr | rd_table[laddr].addr;
	uint32 val = rd_table[laddr].value_registered ? rd_table[laddr].value : is_dma ? rd_table[laddr].dev->read_dma_io8(addr2) : rd_table[laddr].dev->read_io8(addr2);
#ifdef _IO_DEBUG_LOG
	if(!rd_table[laddr].dev->this_device_id && !rd_table[laddr].value_registered) {
		emu->out_debug_log(_T("UNKNOWN:\t"));
	}
	emu->out_debug_log(_T("%06x\tIN8\t%04x = %02x\n"), get_cpu_pc(0), addr, val);
#endif
	return val & 0xff;
}

void IO::write_port16(uint32 addr, uint32 data, bool is_dma)
{
	uint32 laddr = addr & IO_ADDR_MASK, haddr = addr & ~IO_ADDR_MASK;
	uint32 addr2 = haddr | wr_table[laddr].addr;
#ifdef _IO_DEBUG_LOG
	if(!wr_table[laddr].dev->this_device_id && !wr_table[laddr].is_flipflop) {
		emu->out_debug_log(_T("UNKNOWN:\t"));
	}
	emu->out_debug_log(_T("%06x\tOUT16\t%04x,%04x\n"), get_cpu_pc(0), addr, data);
#endif
	if(wr_table[laddr].is_flipflop) {
		rd_table[laddr].value = data & 0xffff;
	} else if(is_dma) {
		wr_table[laddr].dev->write_dma_io16(addr2, data & 0xffff);
	} else {
		wr_table[laddr].dev->write_io16(addr2, data & 0xffff);
	}
}

uint32 IO::read_port16(uint32 addr, bool is_dma)
{
	uint32 laddr = addr & IO_ADDR_MASK, haddr = addr & ~IO_ADDR_MASK;
	uint32 addr2 = haddr | rd_table[laddr].addr;
	uint32 val = rd_table[laddr].value_registered ? rd_table[laddr].value : is_dma ? rd_table[laddr].dev->read_dma_io16(addr2) : rd_table[laddr].dev->read_io16(addr2);
#ifdef _IO_DEBUG_LOG
	if(!rd_table[laddr].dev->this_device_id && !rd_table[laddr].value_registered) {
		emu->out_debug_log(_T("UNKNOWN:\t"));
	}
	emu->out_debug_log(_T("%06x\tIN16\t%04x = %04x\n"), get_cpu_pc(0), addr, val);
#endif
	return val & 0xffff;
}

void IO::write_port32(uint32 addr, uint32 data, bool is_dma)
{
	uint32 laddr = addr & IO_ADDR_MASK, haddr = addr & ~IO_ADDR_MASK;
	uint32 addr2 = haddr | wr_table[laddr].addr;
#ifdef _IO_DEBUG_LOG
	if(!wr_table[laddr].dev->this_device_id && !wr_table[laddr].is_flipflop) {
		emu->out_debug_log(_T("UNKNOWN:\t"));
	}
	emu->out_debug_log(_T("%06x\tOUT32\t%04x,%08x\n"), get_cpu_pc(0), addr, data);
#endif
	if(wr_table[laddr].is_flipflop) {
		rd_table[laddr].value = data;
	} else if(is_dma) {
		wr_table[laddr].dev->write_dma_io32(addr2, data);
	} else {
		wr_table[laddr].dev->write_io32(addr2, data);
	}
}

uint32 IO::read_port32(uint32 addr, bool is_dma)
{
	uint32 laddr = addr & IO_ADDR_MASK, haddr = addr & ~IO_ADDR_MASK;
	uint32 addr2 = haddr | rd_table[laddr].addr;
	uint32 val = rd_table[laddr].value_registered ? rd_table[laddr].value : is_dma ? rd_table[laddr].dev->read_dma_io32(addr2) : rd_table[laddr].dev->read_io32(addr2);
#ifdef _IO_DEBUG_LOG
	if(!rd_table[laddr].dev->this_device_id && !rd_table[laddr].value_registered) {
		emu->out_debug_log(_T("UNKNOWN:\t"));
	}
	emu->out_debug_log(_T("%06x\tIN32\t%04x = %08x\n"), get_cpu_pc(0), laddr | haddr, val);
#endif
	return val;
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

void IO::set_iovalue_single_r(uint32 addr, uint32 value)
{
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

void IO::set_iowait_single_r(uint32 addr, int wait)
{
	rd_table[addr & IO_ADDR_MASK].wait = wait;
}

void IO::set_iowait_single_w(uint32 addr, int wait)
{
	wr_table[addr & IO_ADDR_MASK].wait = wait;
}

void IO::set_iowait_single_rw(uint32 addr, int wait)
{
	set_iowait_single_r(addr, wait);
	set_iowait_single_w(addr, wait);
}

void IO::set_iowait_range_r(uint32 s, uint32 e, int wait)
{
	for(uint32 i = s; i <= e; i++) {
		rd_table[i & IO_ADDR_MASK].wait = wait;
	}
}

void IO::set_iowait_range_w(uint32 s, uint32 e, int wait)
{
	for(uint32 i = s; i <= e; i++) {
		wr_table[i & IO_ADDR_MASK].wait = wait;
	}
}

void IO::set_iowait_range_rw(uint32 s, uint32 e, int wait)
{
	set_iowait_range_r(s, e, wait);
	set_iowait_range_w(s, e, wait);
}

#define STATE_VERSION	1

void IO::save_state(FILEIO* state_fio)
{
	state_fio->FputUint32(STATE_VERSION);
	state_fio->FputInt32(this_device_id);
	
	for(int i = 0; i < IO_ADDR_MAX; i++) {
		state_fio->FputUint32(rd_table[i].value);
	}
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
	return true;
}

