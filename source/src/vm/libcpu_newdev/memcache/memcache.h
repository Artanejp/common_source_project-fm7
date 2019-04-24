// license:BSD-3-Clause
// copyright-holders:Aaron Giles,Olivier Galibert
/***************************************************************************

    emumem.h

    Functions which handle device memory accesses.

***************************************************************************/

#pragma once
#include "../../device.h"

enum {
	CACHE_LINE_NOCACHE = 0,
	CACHE_LINE_READ = 1,
	CACHE_LINE_WRITE = 2,
	CACHE_LINE_READ_WRITE =3
};

class DEVICE;
class cache_line
{
private:
	DEVICE *d_device;
	bool prefetch_enabled;
	int prefetch_block_size;
	bool enabled;
	uint32_t start_addr;
	uint32_t end_addr;
	int access_type;
	uint8_t *cache_data;
	bool *is_exists;
	bool is_little_endian;
public:
	cache_line(uint32_t start, uint32_t line_size, DEVICE* dev = NULL, int type = CACHE_LINE_READ_WRITE,  int prefetch_size = -1)
	{
		prefetch_enabled = (prefetch_size > 0) ? true : false;
		prefetch_block_size = prefetch_size;
		enabled = false;
		cache_data = nullptr;
		is_exists = nullptr;
		start_addr = start;
		end_addr = start_addr;
		access_type = type;
		d_device = dev;
		if(line_size > 0) {
			cache_data = new uint8_t[line_size];
			is_exists   = new bool[line_size];
			if((cache_data != nullptr) && (is_exists != nullptr)) {
				end_addr = start_addr + line_size - 1;
				for(int i = 0; i < line_size; i++) {
					cache_data[i] = 0x00;
					is_exists[i]  = false;
				}
				enabled = true;
			}
		}
	}
	~cache_line() {
		if(is_exists != nullptr) delete [] is_exists;
		if(cache_data != nullptr) delete [] cache_data;
	}
	bool is_enabled() {
		return enabled;
	}
	bool is_cache_enabled(addr) {
		return resion_check(addr);
	}
	uint32_t get_start_address() {
		if(enabled) {
			return start_addr;
		}
		return 0;
	}
	uint32_t get_end_address() {
		if(enabled) {
			return end_addr;
		}
		return 0;
	}
	uint32_t get_line_size() {
		if(enabled) {
			return end_addr - start_addr + 1;
		}
		return 0;
	}
	inline bool region_check(addr) {
		if(!(enabled)) return false;
		return ((addr >= start_addr) && (addr <= end_addr)) ? true : false;
	}
	inline bool region_check_range(uint32_t addr, uint32_t range)
	{
		if(!(enabled)) return false;
		if(range == 0) return false;
		return ((addr >= start_addr) && ((addr + range - 1) <= end_addr)) ? true : false;
	}
	inline bool cache_hit(uint32_t addr) {
		if(region_check(addr)) {
			return is_exists[addr - start_addr];
		}
		return false;
	}
	inline bool cache_hit_range(uint32_t addr, uint32_t range) {
		if(region_check_range(addr, range)) {
			bool f = true;
			uint32_t base = addr - start_addr;
			for(uint32_t i = 0; i < range; i++) {
				if(!(is_exists[base + i])) {
					f = false;
					break;
				}
			}
			return f;
		}
		return false;
	}
	int set_prefetch_block_size(int bytes)
	{
		prefetch_block_size = bytes;
	}
	bool request_prefetch(uint32_t start)
	{
		if(prefetch_block_size <= 0) return false;
		if(region_check(start)) {
			for(int i = 0; i < prefetch_block_size; i++) {	
				if(!(prefetch(start + i))) return false;
			}
			return true;
		}
		return false;
	}
	bool prefetch(uint32_t addr)
	{
		if(region_check(start)) {
			int wait;
			cache_data[addr - start_addr] = d_device->read_data8w(addr, &wait);
			is_exists[addr - start_addr] = true;
			return true;
		}
		return false;
	}
	bool discard_memory_resion(uint32_t start, uint32_t end)
	{
		if(!(enabled)) return false;
		if(start > end) {
			uint32_t tmp;
			tmp = start;
			start = end;
			end = start;
		}
		if((start < start_addr) && (end < start_addr)) return false;
		if((start > end_addr) && (end > end_addr)) return false;
		if(start < start_addr) {
			start = start_addr;
		}
		if(end > end_addr) {
			end = end_addr;
		}
		if(!(resion_check(start)) || !(resion_check(end))) return false;
		
		uint32_t __begin = start - start_addr;
		uint32_t __end = end - start_addr + 1;
		for(uint32_t a = __begin; a < __end; a++) {
			is_exists[a] = false;
		}
		return true;
	}
	bool discard_cache(uint32_t addr)
	{
		if(resion_check(addr)) {
			is_exists[addr - start_addr] = false;
			return true;
		}
		return true;
	}
	void clear_all()
	{
		if(end_addr < start_addr) return;
		uint32_t _size = end_addr - start_addr + 1;
		memset(cache_data, 0x00, _size * sizeof(uint8_t));
		for(uint32\t i = 0; i < _size; i++) {
			is_exists[i] = false;
		}
	}
	void change_access_mode(int new_mode)
	{
		int old_mode = access_type;
		if((old_mode & 3) == (new_mode & 3)) return;
		if((old_mode & CACHE_LINE_READ) != (new_mode & CACHE_LINE_READ)) { // Discard cache.
			clear_all();
		}
		access_type = new_mode;
	}
	inline uint32_t read_data8w(uint32_t addr, int* wait)
	{
		if((access_type & CACHE_LINE_READ) == 0) {
			is_exists[addr - start_addr] = false;
			return d_device->read_data8w(addr, wait);
		}
		if(cache_hit(addr)) {
			if(wait != nullptr) *wait = 0;
			return (uint32_t)(cache_data[addr - start_addr]);
		}
		
		if(region_check(addr)) {
			if(d_device != NULL) {
				uint32_t data = d_device->read_data8w(addr, wait);
				cache_data[addr - start_addr] = (uint8_t)data;
				is_exists[addr - start_addr] = true;
				return data;
			} else {
				cache_data[addr - start_addr] = (uint8_t)0xff;
				is_exists[addr - start_addr] = false;
				return 0xff;
			}
		}
		return 0xff;
	}
	// ToDo: endian
	uint32_t read_data16w(uint32_t addr, int* wait)
	{
		pair16_t n;
		if((access_type & CACHE_LINE_READ) == 0) {
			//is_exists[addr - start_addr] = false;
			//is_exists[addr - start_addr + 1] = false;
			return d_device->read_data16w(addr, wait);
		}
		if(cache_hit_range(addr, 2)) {
			if(wait != nullptr) *wait = 0;
			n.b.l =  cache_data[addr - start_addr + 0];
			n.b.h =  cache_data[addr - start_addr + 1];
			return (uint32_t)(n.w);
		} else {
			n.b.l = read_data8w(addr + 0, wait);
			n.b.h = read_data8w(addr + 0, wait);
			return (uint32_t)(n.w);
		}
		return 0xffff;
	}			
	uint32_t read_data32w(uint32_t addr, int* wait)
	{
		pair32_t n;
		if((access_type & CACHE_LINE_READ) == 0) {
			return d_device->read_data32w(addr, wait);
		}
		if(cache_hit_range(addr, 4)) {
			if(wait != nullptr) *wait = 0;
			n.b.l  =  cache_data[addr - start_addr + 0];
			n.b.h  =  cache_data[addr - start_addr + 1];
			n.b.h2 =  cache_data[addr - start_addr + 2];
			n.b.h3 =  cache_data[addr - start_addr + 3];
			return n.d;
		} else {
			n.b.l  = read_data8w(addr + 0, wait);
			n.b.h  = read_data8w(addr + 1, wait);
			n.b.h2 = read_data8w(addr + 2, wait);
			n.b.h3 = read_data8w(addr + 3, wait);
			return n.d;
		}
		return 0xffffffff;
	}			
	inline void write_data8w(uint32_t addr, uint32_t data, int* wait)
	{
		if(region_check(addr)) {
			if((access_type & CACHE_TYPE_WRITE) != 0) {
				if(d_device != NULL) {
					d_device->write_data8w(addr, data, wait);
				} else {
					if(wait != nullptr) *wait = 0;
					data = 0xff;
				}
				if((access_type & CACHE_LINE_READ) != 0) {
					cache_data[addr - start_addr] = (uint8_t)data;
					is_exists[addr - start_addr] = true;
				}
			} else if((access_type & CACHE_TYPE_READ) != 0) { // read Only
				if(is_exists[addr - start_addr]) return;
				if(d_device != NULL) {
					cache_data[addr - start_addr] = d_device->read_data8w(addr, wait);
				} else {
					if(wait != nullptr) *wait = 0;
					cache_data[addr - start_addr] = 0xff;
				}
				is_exists[addr - start_addr] = true;
			}
		}
	}
	void write_data16w(uint32_t addr, uint32_t data, int* wait)
	{
		pair16_t n;
		bool bh, bl;
		n.w = data & 0x0000ffff;
		if(wait != nullptr) *wait = 0;
		write_data8w(addr + 0, (uint32_t)(n.b.l), wait);
		write_data8w(addr + 1, (uint32_t)(n.b.h), wait);
	}
	void write_data32w(uint32_t addr, uint32_t data, int* wait)
	{
		pair32_t n;
		bool bl, bh, bh2, bh3;
		n.d = data;
		if(wait != nullptr) *wait = 0;
		write_data8w(addr + 0, (uint32_t)(n.b.l), wait);
		write_data8w(addr + 1, (uint32_t)(n.b.h), wait);
		write_data8w(addr + 2, (uint32_t)(n.b.h2), wait);
		write_data8w(addr + 3, (uint32_t)(n.b.h3), wait);
	}
	void set_context_device(DEVIVE* dev)
	{
		d_device = dev;
	}
};			
