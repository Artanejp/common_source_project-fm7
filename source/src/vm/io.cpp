/*
	Skelton for retropc emulator

	Author : Takeda.Toshiya
	Date   : 2008.12.29 -

	[ i/o bus ]
*/

#include "io.h"

#define IO_ADDR_MASK (space - 1)

void IO::initialize()
{
	// allocate tables here to support multiple instances with different address range
	if(wr_table == NULL) {
		wr_table = (wr_bank_t *)calloc(space, sizeof(wr_bank_t));
		rd_table = (rd_bank_t *)calloc(space, sizeof(rd_bank_t));
		
		// vm->dummy must be generated first !
		for(uint32_t i = 0; i < space; i++) {
			wr_table[i].dev = rd_table[i].dev = vm->dummy;
			wr_table[i].addr = rd_table[i].addr = i;
		}
	}
}

void IO::release()
{
	free(wr_table);
	free(rd_table);
}

void IO::write_io8(uint32_t addr, uint32_t data)
{
	int wait = 0;
	write_port8(addr, data, false, &wait);
}

uint32_t IO::read_io8(uint32_t addr)
{
	int wait = 0;
	return read_port8(addr, false, &wait);
}

void IO::write_io16(uint32_t addr, uint32_t data)
{
	int wait = 0;
	write_port16(addr, data, false, &wait);
}

uint32_t IO::read_io16(uint32_t addr)
{
	int wait = 0;
	return read_port16(addr, false, &wait);
}

void IO::write_io32(uint32_t addr, uint32_t data)
{
	int wait = 0;
	write_port32(addr, data, false, &wait);
}

uint32_t IO::read_io32(uint32_t addr)
{
	int wait = 0;
	return read_port32(addr, false, &wait);
}

void IO::write_io8w(uint32_t addr, uint32_t data, int* wait)
{
	write_port8(addr, data, false, wait);
}

uint32_t IO::read_io8w(uint32_t addr, int* wait)
{
	return read_port8(addr, false, wait);
}

void IO::write_io16w(uint32_t addr, uint32_t data, int* wait)
{
	write_port16(addr, data, false, wait);
}

uint32_t IO::read_io16w(uint32_t addr, int* wait)
{
	return read_port16(addr, false, wait);
}

void IO::write_io32w(uint32_t addr, uint32_t data, int* wait)
{
	write_port32(addr, data, false, wait);
}

uint32_t IO::read_io32w(uint32_t addr, int* wait)
{
	return read_port32(addr, false, wait);
}

void IO::write_dma_io8(uint32_t addr, uint32_t data)
{
	int wait = 0;
	write_port8(addr, data, true, &wait);
}

uint32_t IO::read_dma_io8(uint32_t addr)
{
	int wait = 0;
	return read_port8(addr, true, &wait);
}

void IO::write_dma_io16(uint32_t addr, uint32_t data)
{
	int wait = 0;
	write_port16(addr, data, true, &wait);
}

uint32_t IO::read_dma_io16(uint32_t addr)
{
	int wait = 0;
	return read_port16(addr, true, &wait);
}

void IO::write_dma_io32(uint32_t addr, uint32_t data)
{
	int wait = 0;
	write_port32(addr, data, true, &wait);
}

uint32_t IO::read_dma_io32(uint32_t addr)
{
	int wait = 0;
	return read_port32(addr, true, &wait);
}

void IO::write_dma_io8w(uint32_t addr, uint32_t data, int* wait)
{
	write_port8(addr, data, true, wait);
}

uint32_t IO::read_dma_io8w(uint32_t addr, int* wait)
{
	return read_port8(addr, true, wait);
}

void IO::write_dma_io16w(uint32_t addr, uint32_t data, int* wait)
{
	write_port16(addr, data, true, wait);
}

uint32_t IO::read_dma_io16w(uint32_t addr, int* wait)
{
	return read_port16(addr, true, wait);
}

void IO::write_dma_io32w(uint32_t addr, uint32_t data, int* wait)
{
	write_port32(addr, data, true, wait);
}

uint32_t IO::read_dma_io32w(uint32_t addr, int* wait)
{
	return read_port32(addr, true, wait);
}

void IO::write_port8(uint32_t addr, uint32_t data, bool is_dma, int *wait)
{
	uint32_t laddr = addr & IO_ADDR_MASK, haddr = addr & ~IO_ADDR_MASK;
	uint32_t addr2 = haddr | wr_table[laddr].addr;
	int wait_tmp = 0;
	
	*wait = wr_table[laddr].wait;
	
#ifdef _IO_DEBUG_LOG
	_TCHAR tmp[256] = {0};
	if(!wr_table[laddr].dev->this_device_id && !wr_table[laddr].is_flipflop) {
		my_sprintf_s(tmp + _tcslen(tmp), array_length(tmp) - _tcslen(tmp), _T("UNKNOWN:\t"));
	}
	if(cpu_index != 0) {
		my_sprintf_s(tmp + _tcslen(tmp), array_length(tmp) - _tcslen(tmp), _T("CPU=%d\t"), cpu_index);
	}
	my_sprintf_s(tmp + _tcslen(tmp), array_length(tmp) - _tcslen(tmp), _T("%06x\tOUT8\t%04x,%02x\n"), get_cpu_pc(cpu_index), addr, data & 0xff);
	this->out_debug_log(tmp);
#endif
	if(wr_table[laddr].is_flipflop) {
		rd_table[laddr].value = data & 0xff;
	} else if(is_dma) {
		wr_table[laddr].dev->write_dma_io8w(addr2, data & 0xff, &wait_tmp);
		if(!wr_table[laddr].wait_registered) *wait = wait_tmp;
	} else {
		wr_table[laddr].dev->write_io8w(addr2, data & 0xff, &wait_tmp);
		if(!wr_table[laddr].wait_registered) *wait = wait_tmp;
	}
}

uint32_t IO::read_port8(uint32_t addr, bool is_dma, int *wait)
{
	uint32_t laddr = addr & IO_ADDR_MASK, haddr = addr & ~IO_ADDR_MASK;
	uint32_t addr2 = haddr | rd_table[laddr].addr;
	uint32_t val;
	int wait_tmp = 0;
	
	*wait = rd_table[laddr].wait;
	
	if(rd_table[laddr].value_registered) {
		val = rd_table[laddr].value;
	} else if(is_dma) {
		val = rd_table[laddr].dev->read_dma_io8w(addr2, &wait_tmp);
		if(!rd_table[laddr].wait_registered) *wait = wait_tmp;
	} else {
		val = rd_table[laddr].dev->read_io8w(addr2, &wait_tmp);
		if(!rd_table[laddr].wait_registered) *wait = wait_tmp;
	}
#ifdef _IO_DEBUG_LOG
	_TCHAR tmp[256] = {0};
	if(!rd_table[laddr].dev->this_device_id && !rd_table[laddr].value_registered) {
		my_sprintf_s(tmp + _tcslen(tmp), array_length(tmp) - _tcslen(tmp), _T("UNKNOWN:\t"));
	}
	if(cpu_index != 0) {
		my_sprintf_s(tmp + _tcslen(tmp), array_length(tmp) - _tcslen(tmp), _T("CPU=%d\t"), cpu_index);
	}
	my_sprintf_s(tmp + _tcslen(tmp), array_length(tmp) - _tcslen(tmp), _T("%06x\tIN8\t%04x = %02x\n"), get_cpu_pc(cpu_index), addr, val & 0xff);
	this->out_debug_log(tmp);
#endif
	return val & 0xff;
}

void IO::write_port16(uint32_t addr, uint32_t data, bool is_dma, int *wait)
{
	if(bus_width >= 16 && !(addr & 1)) {
		uint32_t laddr = addr & IO_ADDR_MASK, haddr = addr & ~IO_ADDR_MASK;
		uint32_t addr2 = haddr | wr_table[laddr].addr;
		int wait_tmp = 0;
		
		*wait = wr_table[laddr].wait;
		
#ifdef _IO_DEBUG_LOG
		_TCHAR tmp[256] = {0};
		if(!wr_table[laddr].dev->this_device_id && !wr_table[laddr].is_flipflop) {
			my_sprintf_s(tmp + _tcslen(tmp), array_length(tmp) - _tcslen(tmp), _T("UNKNOWN:\t"));
		}
		if(cpu_index != 0) {
			my_sprintf_s(tmp + _tcslen(tmp), array_length(tmp) - _tcslen(tmp), _T("CPU=%d\t"), cpu_index);
		}
		my_sprintf_s(tmp + _tcslen(tmp), array_length(tmp) - _tcslen(tmp), _T("%06x\tOUT16\t%04x,%04x\n"), get_cpu_pc(cpu_index), addr, data & 0xffff);
		this->out_debug_log(tmp);
#endif
		if(wr_table[laddr].is_flipflop) {
			rd_table[laddr].value = data & 0xffff;
		} else if(is_dma) {
			wr_table[laddr].dev->write_dma_io16w(addr2, data & 0xffff, &wait_tmp);
			if(!wr_table[laddr].wait_registered) *wait = wait_tmp;
		} else {
			wr_table[laddr].dev->write_io16w(addr2, data & 0xffff, &wait_tmp);
			if(!wr_table[laddr].wait_registered) *wait = wait_tmp;
		}
	} else {
		int wait_l = 0, wait_h = 0;
		write_port8(addr    , (data     ) & 0xff, is_dma, &wait_l);
		write_port8(addr + 2, (data >> 8) & 0xff, is_dma, &wait_h);
		*wait = wait_l + wait_h;
	}
}

uint32_t IO::read_port16(uint32_t addr, bool is_dma, int *wait)
{
	if(bus_width >= 16 && !(addr & 1)) {
		uint32_t laddr = addr & IO_ADDR_MASK, haddr = addr & ~IO_ADDR_MASK;
		uint32_t addr2 = haddr | rd_table[laddr].addr;
		uint32_t val;
		int wait_tmp = 0;
		
		*wait = rd_table[laddr].wait;
		
		if(rd_table[laddr].value_registered) {
			val = rd_table[laddr].value;
		} else if(is_dma) {
			val = rd_table[laddr].dev->read_dma_io16w(addr2, &wait_tmp);
			if(!rd_table[laddr].wait_registered) *wait = wait_tmp;
		} else {
			val = rd_table[laddr].dev->read_io16w(addr2, &wait_tmp);
			if(!rd_table[laddr].wait_registered) *wait = wait_tmp;
		}
#ifdef _IO_DEBUG_LOG
		_TCHAR tmp[256] = {0};
		if(!rd_table[laddr].dev->this_device_id && !rd_table[laddr].value_registered) {
			my_sprintf_s(tmp + _tcslen(tmp), array_length(tmp) - _tcslen(tmp), _T("UNKNOWN:\t"));
		}
		if(cpu_index != 0) {
			my_sprintf_s(tmp + _tcslen(tmp), array_length(tmp) - _tcslen(tmp), _T("CPU=%d\t"), cpu_index);
		}
		my_sprintf_s(tmp + _tcslen(tmp), array_length(tmp) - _tcslen(tmp), _T("%06x\tIN16\t%04x = %04x\n"), get_cpu_pc(cpu_index), addr, val & 0xffff);
		this->out_debug_log(tmp);
#endif
		return val & 0xffff;
	} else {
		int wait_l = 0, wait_h = 0;
		uint32_t val;
		val  = read_port8(addr    , is_dma, &wait_l);
		val |= read_port8(addr + 1, is_dma, &wait_h) << 16;
		*wait = wait_l + wait_h;
		return val;
	}
}

void IO::write_port32(uint32_t addr, uint32_t data, bool is_dma, int *wait)
{
	if(bus_width >= 32 && !(addr & 3)) {
		uint32_t laddr = addr & IO_ADDR_MASK, haddr = addr & ~IO_ADDR_MASK;
		uint32_t addr2 = haddr | wr_table[laddr].addr;
		int wait_tmp = 0;
		
		*wait = wr_table[laddr].wait;
		
#ifdef _IO_DEBUG_LOG
		_TCHAR tmp[256] = {0};
		if(!wr_table[laddr].dev->this_device_id && !wr_table[laddr].is_flipflop) {
			my_sprintf_s(tmp + _tcslen(tmp), array_length(tmp) - _tcslen(tmp), _T("UNKNOWN:\t"));
		}
		if(cpu_index != 0) {
			my_sprintf_s(tmp + _tcslen(tmp), array_length(tmp) - _tcslen(tmp), _T("CPU=%d\t"), cpu_index);
		}
		my_sprintf_s(tmp + _tcslen(tmp), array_length(tmp) - _tcslen(tmp), _T("%06x\tOUT32\t%04x,%08x\n"), get_cpu_pc(cpu_index), addr, data);
		this->out_debug_log(tmp);
#endif
		if(wr_table[laddr].is_flipflop) {
			rd_table[laddr].value = data;
		} else if(is_dma) {
			wr_table[laddr].dev->write_dma_io32w(addr2, data, &wait_tmp);
			if(!wr_table[laddr].wait_registered) *wait = wait_tmp;
		} else {
			wr_table[laddr].dev->write_io32w(addr2, data, &wait_tmp);
			if(!wr_table[laddr].wait_registered) *wait = wait_tmp;
		}
	} else if(!(addr & 1)) {
		int wait_l = 0, wait_h = 0;
		write_port16(addr    , (data      ) & 0xffff, is_dma, &wait_l);
		write_port16(addr + 2, (data >> 16) & 0xffff, is_dma, &wait_h);
		*wait = wait_l + wait_h;
	} else {
		int wait_l = 0, wait_m = 0, wait_h = 0;
		write_port8 (addr    , (data      ) & 0x00ff, is_dma, &wait_l);
		write_port16(addr + 1, (data >>  8) & 0xffff, is_dma, &wait_m);
		write_port8 (addr + 3, (data >> 24) & 0x00ff, is_dma, &wait_h);
		*wait = wait_l + wait_m + wait_h;
	}
}

uint32_t IO::read_port32(uint32_t addr, bool is_dma, int *wait)
{
	if(bus_width >= 32 && !(addr & 3)) {
		uint32_t laddr = addr & IO_ADDR_MASK, haddr = addr & ~IO_ADDR_MASK;
		uint32_t addr2 = haddr | rd_table[laddr].addr;
		uint32_t val;
		int wait_tmp = 0;
		
		*wait = rd_table[laddr].wait;
		
		if(rd_table[laddr].value_registered) {
			val = rd_table[laddr].value;
		} else if(is_dma) {
			val = rd_table[laddr].dev->read_dma_io32w(addr2, &wait_tmp);
			if(!rd_table[laddr].wait_registered) *wait = wait_tmp;
		} else {
			val = rd_table[laddr].dev->read_io32w(addr2, &wait_tmp);
			if(!rd_table[laddr].wait_registered) *wait = wait_tmp;
		}
#ifdef _IO_DEBUG_LOG
		_TCHAR tmp[256] = {0};
		if(!rd_table[laddr].dev->this_device_id && !rd_table[laddr].value_registered) {
			my_sprintf_s(tmp + _tcslen(tmp), array_length(tmp) - _tcslen(tmp), _T("UNKNOWN:\t"));
		}
		if(cpu_index != 0) {
			my_sprintf_s(tmp + _tcslen(tmp), array_length(tmp) - _tcslen(tmp), _T("CPU=%d\t"), cpu_index);
		}
		my_sprintf_s(tmp + _tcslen(tmp), array_length(tmp) - _tcslen(tmp), _T("%06x\tIN32\t%04x = %08x\n"), get_cpu_pc(cpu_index), laddr | haddr, val);
		this->out_debug_log(tmp);
#endif
		return val;
	} else if(!(addr & 1)) {
		int wait_l = 0, wait_h = 0;
		uint32_t val;
		val  = read_port16(addr    , is_dma, &wait_l);
		val |= read_port16(addr + 2, is_dma, &wait_h) << 16;
		*wait = wait_l + wait_h;
		return val;
	} else {
		int wait_l = 0, wait_m = 0, wait_h = 0;
		uint32_t val;
		val  = read_port8 (addr    , is_dma, &wait_l);
		val |= read_port16(addr + 1, is_dma, &wait_m) <<  8;
		val |= read_port8 (addr + 3, is_dma, &wait_h) << 24;
		*wait = wait_l + wait_m + wait_h;
		return val;
	}
}

// register

void IO::set_iomap_single_r(uint32_t addr, DEVICE* device)
{
	IO::initialize(); // subclass may overload initialize()
	
	rd_table[addr & IO_ADDR_MASK].dev = device;
	rd_table[addr & IO_ADDR_MASK].addr = addr & IO_ADDR_MASK;
}

void IO::set_iomap_single_w(uint32_t addr, DEVICE* device)
{
	IO::initialize();
	
	wr_table[addr & IO_ADDR_MASK].dev = device;
	wr_table[addr & IO_ADDR_MASK].addr = addr & IO_ADDR_MASK;
}

void IO::set_iomap_single_rw(uint32_t addr, DEVICE* device)
{
	set_iomap_single_r(addr, device);
	set_iomap_single_w(addr, device);
}

void IO::set_iomap_alias_r(uint32_t addr, DEVICE* device, uint32_t alias)
{
	IO::initialize();
	
	rd_table[addr & IO_ADDR_MASK].dev = device;
	rd_table[addr & IO_ADDR_MASK].addr = alias & IO_ADDR_MASK;
}

void IO::set_iomap_alias_w(uint32_t addr, DEVICE* device, uint32_t alias)
{
	IO::initialize();
	
	wr_table[addr & IO_ADDR_MASK].dev = device;
	wr_table[addr & IO_ADDR_MASK].addr = alias & IO_ADDR_MASK;
}

void IO::set_iomap_alias_rw(uint32_t addr, DEVICE* device, uint32_t alias)
{
	set_iomap_alias_r(addr, device, alias);
	set_iomap_alias_w(addr, device, alias);
}

void IO::set_iomap_range_r(uint32_t s, uint32_t e, DEVICE* device)
{
	IO::initialize();
	
	for(uint32_t i = s; i <= e; i++) {
		rd_table[i & IO_ADDR_MASK].dev = device;
		rd_table[i & IO_ADDR_MASK].addr = i & IO_ADDR_MASK;
	}
}

void IO::set_iomap_range_w(uint32_t s, uint32_t e, DEVICE* device)
{
	IO::initialize();
	
	for(uint32_t i = s; i <= e; i++) {
		wr_table[i & IO_ADDR_MASK].dev = device;
		wr_table[i & IO_ADDR_MASK].addr = i & IO_ADDR_MASK;
	}
}

void IO::set_iomap_range_rw(uint32_t s, uint32_t e, DEVICE* device)
{
	set_iomap_range_r(s, e, device);
	set_iomap_range_w(s, e, device);
}

void IO::set_iovalue_single_r(uint32_t addr, uint32_t value)
{
	IO::initialize();
	
	rd_table[addr & IO_ADDR_MASK].value = value;
	rd_table[addr & IO_ADDR_MASK].value_registered = true;
}

void IO::set_iovalue_range_r(uint32_t s, uint32_t e, uint32_t value)
{
	IO::initialize();
	
	for(uint32_t i = s; i <= e; i++) {
		rd_table[i & IO_ADDR_MASK].value = value;
		rd_table[i & IO_ADDR_MASK].value_registered = true;
	}
}

void IO::set_flipflop_single_rw(uint32_t addr, uint32_t value)
{
	IO::initialize();
	
	wr_table[addr & IO_ADDR_MASK].is_flipflop = true;
	rd_table[addr & IO_ADDR_MASK].value = value;
	rd_table[addr & IO_ADDR_MASK].value_registered = true;
}

void IO::set_flipflop_range_rw(uint32_t s, uint32_t e, uint32_t value)
{
	IO::initialize();
	
	for(uint32_t i = s; i <= e; i++) {
		wr_table[i & IO_ADDR_MASK].is_flipflop = true;
		rd_table[i & IO_ADDR_MASK].value = value;
		rd_table[i & IO_ADDR_MASK].value_registered = true;
	}
}

void IO::set_iowait_single_r(uint32_t addr, int wait)
{
	IO::initialize();
	
	rd_table[addr & IO_ADDR_MASK].wait = wait;
	rd_table[addr & IO_ADDR_MASK].wait_registered = true;
}

void IO::set_iowait_single_w(uint32_t addr, int wait)
{
	IO::initialize();
	
	wr_table[addr & IO_ADDR_MASK].wait = wait;
	wr_table[addr & IO_ADDR_MASK].wait_registered = true;
}

void IO::set_iowait_single_rw(uint32_t addr, int wait)
{
	set_iowait_single_r(addr, wait);
	set_iowait_single_w(addr, wait);
}

void IO::set_iowait_range_r(uint32_t s, uint32_t e, int wait)
{
	IO::initialize();
	
	for(uint32_t i = s; i <= e; i++) {
		rd_table[i & IO_ADDR_MASK].wait = wait;
		rd_table[i & IO_ADDR_MASK].wait_registered = true;
	}
}

void IO::set_iowait_range_w(uint32_t s, uint32_t e, int wait)
{
	IO::initialize();
	
	for(uint32_t i = s; i <= e; i++) {
		wr_table[i & IO_ADDR_MASK].wait = wait;
		wr_table[i & IO_ADDR_MASK].wait_registered = true;
	}
}

void IO::set_iowait_range_rw(uint32_t s, uint32_t e, int wait)
{
	set_iowait_range_r(s, e, wait);
	set_iowait_range_w(s, e, wait);
}

#define STATE_VERSION	1

bool IO::process_state(FILEIO* state_fio, bool loading)
{
	if(!state_fio->StateCheckUint32(STATE_VERSION)) {
		return false;
	}
	if(!state_fio->StateCheckInt32(this_device_id)) {
		return false;
	}
	for(uint32_t i = 0; i < space; i++) {
		state_fio->StateValue(rd_table[i].value);
	}
	return true;
}

