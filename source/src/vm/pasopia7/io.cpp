/*
	TOSHIBA PASOPIA 7 Emulator 'EmuPIA7'

	Author : Takeda.Toshiya
	Date   : 2006.09.20 -

	[ 8bit i/o bus ]
*/

#include "io.h"

void IO::write_io8(uint32 addr, uint32 data)
{
	if(mio) {
		mio = false;
		ram[addr & 0xffff] = data;
		return;
	}
	// i/o
	uint32 laddr = addr & IO_ADDR_MASK, haddr = addr & ~IO_ADDR_MASK;
#ifdef _IO_DEBUG_LOG
	if(!write_table[laddr].dev->this_device_id) {
		emu->out_debug_log("UNKNOWN:\t");
	}
	emu->out_debug_log("%6x\tOUT8\t%4x,%2x\n", get_cpu_pc(0), laddr | haddr, data);
#endif
	write_table[laddr].dev->write_io8(haddr | write_table[laddr].addr, data & 0xff);
}

uint32 IO::read_io8(uint32 addr)
{
	if(mio) {
		mio = false;
		return ram[addr & 0xffff];
	}
	// i/o
	uint32 laddr = addr & IO_ADDR_MASK, haddr = addr & ~IO_ADDR_MASK;
	uint32 val = read_table[laddr].dev->read_io8(haddr | read_table[laddr].addr);
#ifdef _IO_DEBUG_LOG
	if(!read_table[laddr].dev->this_device_id && !read_table[laddr].value_registered) {
		emu->out_debug_log("UNKNOWN:\t");
	}
	emu->out_debug_log("%6x\tIN8\t%4x = %2x\n", get_cpu_pc(0), laddr | haddr, val);
#endif
	return val & 0xff;
}

void IO::write_signal(int id, uint32 data, uint32 mask)
{
	if(id == SIG_IO_MIO) {
		mio = ((data & mask) != 0);
	}
}

// register

void IO::set_iomap_single_r(uint32 addr, DEVICE* device)
{
	read_table[addr & IO_ADDR_MASK].dev = device;
	read_table[addr & IO_ADDR_MASK].addr = addr & IO_ADDR_MASK;
}

void IO::set_iomap_single_w(uint32 addr, DEVICE* device)
{
	write_table[addr & IO_ADDR_MASK].dev = device;
	write_table[addr & IO_ADDR_MASK].addr = addr & IO_ADDR_MASK;
}

void IO::set_iomap_single_rw(uint32 addr, DEVICE* device)
{
	set_iomap_single_r(addr, device);
	set_iomap_single_w(addr, device);
}

void IO::set_iomap_alias_r(uint32 addr, DEVICE* device, uint32 alias)
{
	read_table[addr & IO_ADDR_MASK].dev = device;
	read_table[addr & IO_ADDR_MASK].addr = alias & IO_ADDR_MASK;
}

void IO::set_iomap_alias_w(uint32 addr, DEVICE* device, uint32 alias)
{
	write_table[addr & IO_ADDR_MASK].dev = device;
	write_table[addr & IO_ADDR_MASK].addr = alias & IO_ADDR_MASK;
}

void IO::set_iomap_range_r(uint32 s, uint32 e, DEVICE* device)
{
	for(uint32 i = s; i <= e; i++) {
		read_table[i & IO_ADDR_MASK].dev = device;
		read_table[i & IO_ADDR_MASK].addr = i & IO_ADDR_MASK;
	}
}

void IO::set_iomap_alias_rw(uint32 addr, DEVICE* device, uint32 alias)
{
	set_iomap_alias_r(addr, device, alias);
	set_iomap_alias_w(addr, device, alias);
}

void IO::set_iomap_range_w(uint32 s, uint32 e, DEVICE* device)
{
	for(uint32 i = s; i <= e; i++) {
		write_table[i & IO_ADDR_MASK].dev = device;
		write_table[i & IO_ADDR_MASK].addr = i & IO_ADDR_MASK;
	}
}

void IO::set_iomap_range_rw(uint32 s, uint32 e, DEVICE* device)
{
	set_iomap_range_r(s, e, device);
	set_iomap_range_w(s, e, device);
}

