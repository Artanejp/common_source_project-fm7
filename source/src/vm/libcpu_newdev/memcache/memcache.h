// license:BSD-3-Clause
// copyright-holders:Aaron Giles,Olivier Galibert
/***************************************************************************

    emumem.h

    Functions which handle device memory accesses.

***************************************************************************/

#pragma once

//#ifndef __EMU_H__
//#error Dont include this file directly; include emu.h instead.
//#endif

#ifndef MAME_EMU_EMUMEM_H
#define MAME_EMU_EMUMEM_H

#include <type_traits>

using s8 = std::int8_t;
using u8 = std::uint8_t;
using s16 = std::int16_t;
using u16 = std::uint16_t;
using s32 = std::int32_t;
using u32 = std::uint32_t;
using s64 = std::int64_t;
using u64 = std::uint64_t;

//**************************************************************************
//  CONSTANTS
//**************************************************************************

// address space names for common use
constexpr int AS_PROGRAM = 0; // program address space
constexpr int AS_DATA    = 1; // data address space
constexpr int AS_IO      = 2; // I/O address space
constexpr int AS_OPCODES = 3; // (decrypted) opcodes, when separate from data accesses

// read or write constants
enum class read_or_write
{
	READ = 1,
	WRITE = 2,
	READWRITE = 3
};



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// offsets and addresses are 32-bit (for now...)
using offs_t = u32;

// address map constructors are delegates that build up an address_map
using address_map_constructor = named_delegate<void (address_map &)>;

// struct with function pointers for accessors; use is generally discouraged unless necessary
struct data_accessors
{
	u8      (*read_byte)(address_space &space, offs_t address);
	u16     (*read_word)(address_space &space, offs_t address);
	u16     (*read_word_masked)(address_space &space, offs_t address, u16 mask);
	u32     (*read_dword)(address_space &space, offs_t address);
	u32     (*read_dword_masked)(address_space &space, offs_t address, u32 mask);
	u64     (*read_qword)(address_space &space, offs_t address);
	u64     (*read_qword_masked)(address_space &space, offs_t address, u64 mask);

	void    (*write_byte)(address_space &space, offs_t address, u8 data);
	void    (*write_word)(address_space &space, offs_t address, u16 data);
	void    (*write_word_masked)(address_space &space, offs_t address, u16 data, u16 mask);
	void    (*write_dword)(address_space &space, offs_t address, u32 data);
	void    (*write_dword_masked)(address_space &space, offs_t address, u32 data, u32 mask);
	void    (*write_qword)(address_space &space, offs_t address, u64 data);
	void    (*write_qword_masked)(address_space &space, offs_t address, u64 data, u64 mask);
};


// =====================-> The root class of all handlers

// Handlers the refcounting as part of the interface

class handler_entry
{
	DISABLE_COPYING(handler_entry);

	template<int Width, int AddrShift, endianness_t Endian> friend class address_space_specific;

public:
	// Typing flags
	static constexpr u32 F_DISPATCH    = 0x00000001; // handler that forwards the access to other handlers
	static constexpr u32 F_UNITS       = 0x00000002; // handler that merges/splits an access among multiple handlers (unitmask support)
	static constexpr u32 F_PASSTHROUGH = 0x00000004; // handler that passes through the request to another handler

	// Start/end of range flags
	static constexpr u8 START = 1;
	static constexpr u8 END   = 2;

	// Intermediary structure for reference count checking
	class reflist {
	public:
		void add(const handler_entry *entry);

		void propagate();
		void check();

	private:
		std::unordered_map<const handler_entry *, u32> refcounts;
		std::unordered_set<const handler_entry *> seen;
		std::unordered_set<const handler_entry *> todo;
	};

	handler_entry(address_space *space, u32 flags) { m_space = space; m_refcount = 1; m_flags = flags; }
	virtual ~handler_entry() {}

	inline void ref(int count = 1) const { m_refcount += count; }
	inline void unref(int count = 1) const { m_refcount -= count; if(!m_refcount) delete this; }
	inline u32 flags() const { return m_flags; }

	inline bool is_dispatch() const { return m_flags & F_DISPATCH; }
	inline bool is_units() const { return m_flags & F_UNITS; }
	inline bool is_passthrough() const { return m_flags & F_PASSTHROUGH; }

	virtual void dump_map(std::vector<memory_entry> &map) const;

	virtual std::string name() const = 0;
	virtual void enumerate_references(handler_entry::reflist &refs) const;
	u32 get_refcount() const { return m_refcount; }

protected:
	// Address range storage
	struct range {
		offs_t start;
		offs_t end;

		inline void set(offs_t _start, offs_t _end) {
			start = _start;
			end = _end;
		}

		inline void intersect(offs_t _start, offs_t _end) {
			if(_start > start)
				start = _start;
			if(_end < end)
				end = _end;
		}
	};

	address_space *m_space;
	mutable u32 m_refcount;
	u32 m_flags;
};

template<int Width, int AddrShift, int Endian> class memory_access_cache
{
	//using NativeType = typename emu::detail::handler_entry_size<Width>::uX;
	static constexpr u32 NATIVE_BYTES = 1 << Width;
	static constexpr u32 NATIVE_MASK = Width + AddrShift >= 0 ? (1 << (Width + AddrShift)) - 1 : 0;

public:
	// construction/destruction
	memory_access_cache(address_space &space,
						handler_entry_read <Width, AddrShift, Endian> *root_read,
						handler_entry_write<Width, AddrShift, Endian> *root_write);

	~memory_access_cache();

	// getters
	address_space &space() const { return m_space; }

	// see if an address is within bounds, update it if not
	void check_address_r(offs_t address) {
		if(address >= m_addrstart_r && address <= m_addrend_r)
			return;
		m_root_read->lookup(address, m_addrstart_r, m_addrend_r, m_cache_r);
	}

	void check_address_w(offs_t address) {
		if(address >= m_addrstart_w && address <= m_addrend_w)
			return;
		m_root_write->lookup(address, m_addrstart_w, m_addrend_w, m_cache_w);
	}

	// accessor methods

	void *read_ptr(offs_t address) {
		check_address_r(address);
		return m_cache_r->get_ptr(address);
	}

	u8 read_data8(offs_t address) { address &= m_addrmask; return Width == 0 ? read_native(address & ~NATIVE_MASK) : memory_read_generic<Width, AddrShift, Endian, 0, true>([this](offs_t offset, NativeType mask) -> NativeType { return read_native(offset, mask); }, address, 0xff); }
	u16 read_data16(offs_t address) { address &= m_addrmask; return Width == 1 ? read_native(address & ~NATIVE_MASK) : memory_read_generic<Width, AddrShift, Endian, 1, true>([this](offs_t offset, NativeType mask) -> NativeType { return read_native(offset, mask); }, address, 0xffff); }
	u16 read_data16(offs_t address, u16 mask) { address &= m_addrmask; return memory_read_generic<Width, AddrShift, Endian, 1, true>([this](offs_t offset, NativeType mask) -> NativeType { return read_native(offset, mask); }, address, mask); }
	u16 read_data16_unaligned(offs_t address) { address &= m_addrmask; return memory_read_generic<Width, AddrShift, Endian, 1, false>([this](offs_t offset, NativeType mask) -> NativeType { return read_native(offset, mask); }, address, 0xffff); }
	u16 read_data16_unaligned(offs_t address, u16 mask) { address &= m_addrmask; return memory_read_generic<Width, AddrShift, Endian, 1, false>([this](offs_t offset, NativeType mask) -> NativeType { return read_native(offset, mask); }, address, mask); }
	u32 read_data32(offs_t address) { address &= m_addrmask; return Width == 2 ? read_native(address & ~NATIVE_MASK) : memory_read_generic<Width, AddrShift, Endian, 2, true>([this](offs_t offset, NativeType mask) -> NativeType { return read_native(offset, mask); }, address, 0xffffffff); }
	u32 read_data32(offs_t address, u32 mask) { address &= m_addrmask; return memory_read_generic<Width, AddrShift, Endian, 2, true>([this](offs_t offset, NativeType mask) -> NativeType { return read_native(offset, mask); }, address, mask); }
	u32 read_data32_unaligned(offs_t address) { address &= m_addrmask; return memory_read_generic<Width, AddrShift, Endian, 2, false>([this](offs_t offset, NativeType mask) -> NativeType { return read_native(offset, mask); }, address, 0xffffffff); }
	u32 read_data32_unaligned(offs_t address, u32 mask) { address &= m_addrmask; return memory_read_generic<Width, AddrShift, Endian, 2, false>([this](offs_t offset, NativeType mask) -> NativeType { return read_native(offset, mask); }, address, mask); }
	u64 read_data64(offs_t address) { address &= m_addrmask; return Width == 3 ? read_native(address & ~NATIVE_MASK) : memory_read_generic<Width, AddrShift, Endian, 3, true>([this](offs_t offset, NativeType mask) -> NativeType { return read_native(offset, mask); }, address, 0xffffffffffffffffU); }
	u64 read_data64(offs_t address, u64 mask) { address &= m_addrmask; return memory_read_generic<Width, AddrShift, Endian, 3, true>([this](offs_t offset, NativeType mask) -> NativeType { return read_native(offset, mask); }, address, mask); }
	u64 read_data64_unaligned(offs_t address) { address &= m_addrmask; return memory_read_generic<Width, AddrShift, Endian, 3, false>([this](offs_t offset, NativeType mask) -> NativeType { return read_native(offset, mask); }, address, 0xffffffffffffffffU); }
	u64 read_data64_unaligned(offs_t address, u64 mask) { address &= m_addrmask; return memory_read_generic<Width, AddrShift, Endian, 3, false>([this](offs_t offset, NativeType mask) -> NativeType { return read_native(offset, mask); }, address, mask); }

	void write_data8(offs_t address, u8 data) { address &= m_addrmask; if (Width == 0) write_native(address & ~NATIVE_MASK, data); else memory_write_generic<Width, AddrShift, Endian, 0, true>([this](offs_t offset, NativeType data, NativeType mask) { write_native(offset, data, mask); }, address, data, 0xff); }
	void write_data16(offs_t address, u16 data) { address &= m_addrmask; if (Width == 1) write_native(address & ~NATIVE_MASK, data); else memory_write_generic<Width, AddrShift, Endian, 1, true>([this](offs_t offset, NativeType data, NativeType mask) { write_native(offset, data, mask); }, address, data, 0xffff); }
	void write_data16(offs_t address, u16 data, u16 mask) { address &= m_addrmask; memory_write_generic<Width, AddrShift, Endian, 1, true>([this](offs_t offset, NativeType data, NativeType mask) { write_native(offset, data, mask); }, address, data, mask); }
	void write_data16_unaligned(offs_t address, u16 data) { address &= m_addrmask; memory_write_generic<Width, AddrShift, Endian, 1, false>([this](offs_t offset, NativeType data, NativeType mask) { write_native(offset, data, mask); }, address, data, 0xffff); }
	void write_data16_unaligned(offs_t address, u16 data, u16 mask) { address &= m_addrmask; memory_write_generic<Width, AddrShift, Endian, 1, false>([this](offs_t offset, NativeType data, NativeType mask) { write_native(offset, data, mask); }, address, data, mask); }
	void write_data32(offs_t address, u32 data) { address &= m_addrmask; if (Width == 2) write_native(address & ~NATIVE_MASK, data); else memory_write_generic<Width, AddrShift, Endian, 2, true>([this](offs_t offset, NativeType data, NativeType mask) { write_native(offset, data, mask); }, address, data, 0xffffffff); }
	void write_data32(offs_t address, u32 data, u32 mask) { address &= m_addrmask; memory_write_generic<Width, AddrShift, Endian, 2, true>([this](offs_t offset, NativeType data, NativeType mask) { write_native(offset, data, mask); }, address, data, mask); }
	void write_data32_unaligned(offs_t address, u32 data) { address &= m_addrmask; memory_write_generic<Width, AddrShift, Endian, 2, false>([this](offs_t offset, NativeType data, NativeType mask) { write_native(offset, data, mask); }, address, data, 0xffffffff); }
	void write_data32_unaligned(offs_t address, u32 data, u32 mask) { address &= m_addrmask; memory_write_generic<Width, AddrShift, Endian, 2, false>([this](offs_t offset, NativeType data, NativeType mask) { write_native(offset, data, mask); }, address, data, mask); }
	void write_data64(offs_t address, u64 data) { address &= m_addrmask; if (Width == 3) write_native(address & ~NATIVE_MASK, data); else memory_write_generic<Width, AddrShift, Endian, 3, true>([this](offs_t offset, NativeType data, NativeType mask) { write_native(offset, data, mask); }, address, data, 0xffffffffffffffffU); }
	void write_data64(offs_t address, u64 data, u64 mask) { address &= m_addrmask; memory_write_generic<Width, AddrShift, Endian, 3, true>([this](offs_t offset, NativeType data, NativeType mask) { write_native(offset, data, mask); }, address, data, mask); }
	void write_data64_unaligned(offs_t address, u64 data) { address &= m_addrmask; memory_write_generic<Width, AddrShift, Endian, 3, false>([this](offs_t offset, NativeType data, NativeType mask) { write_native(offset, data, mask); }, address, data, 0xffffffffffffffffU); }
	void write_data64_unaligned(offs_t address, u64 data, u64 mask) { address &= m_addrmask; memory_write_generic<Width, AddrShift, Endian, 3, false>([this](offs_t offset, NativeType data, NativeType mask) { write_native(offset, data, mask); }, address, data, mask); }

private:
	address_space &             m_space;

	//int                         m_notifier_id;             // id to remove the notifier on destruction

	offs_t                      m_addrmask;                // address mask
	offs_t                      m_addrstart_r;             // minimum valid address for reading
	offs_t                      m_addrend_r;               // maximum valid address for reading
	offs_t                      m_addrstart_w;             // minimum valid address for writing
	offs_t                      m_addrend_w;               // maximum valid address for writing
	handler_entry_read<Width, AddrShift, Endian> *m_cache_r;   // read cache
	handler_entry_write<Width, AddrShift, Endian> *m_cache_w;  // write cache

	handler_entry_read <Width, AddrShift, Endian> *m_root_read;  // decode tree roots
	handler_entry_write<Width, AddrShift, Endian> *m_root_write;

	uint8_t  read_native(offs_t address, uint8_t mask = 0xff);
	uint16_t read_native(offs_t address, uint16_t mask = 0xffff);
	uint32_t read_native(offs_t address, uint32_t mask = 0xffffffff);
	uint64_t read_native(offs_t address, uint64_t mask = 0xffffffffffffffff);
	
	void write_native(offs_t address, uint8_t data, uint8_t mask = 0xff);
	void write_native(offs_t address, uint16_t data, uint16_t mask = 0xffff);
	void write_native(offs_t address, uint32_t data, uint32_t mask = 0xffffffff);
	void write_native(offs_t address, uint64_t data, uint64_t mask = 0xffffffffffffffff);
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
	uint8_t *cache_data;
	bool *is_exists;
	bool is_little_endian;
public:
	cache_line(uint32_t start, uint32_t line_size,  int prefetch_size = -1)
	{
		prefetch_enabled = (prefetch_size > 0) ? true : false;
		prefetch_block_size = prefetch_size;
		enabled = false;
		cache_data = nullptr;
		is_exists = nullptr;
		start_addr = start;
		end_addr = start_addr;
		d_device = NULL;
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
	uint32_t read_data8w(uint32_t addr, int* wait, bool *is_hit)
	{
		if(cache_hit(addr)) {
			if(wait != nullptr) *wait = 0;
			if(is_hit != nullptr) *is_hit = true;
			return (uint32_t)(cache_data[addr - start_addr]);
		}
		
		if(region_check(addr)) {
			if(d_device != NULL) {
				uint32_t data = d_device->read_data8w(addr, wait);
				cache_data[addr - start_addr] = (uint8_t)data;
				is_exists[addr - start_addr] = true;
				if(is_hit != nullptr) *is_hit = true;
				return data;
			} else {
				cache_data[addr - start_addr] = (uint8_t)0x00;
				is_exists[addr - start_addr] = false;
				if(is_hit != nullptr) *is_hit = true;
				return 0xff;
			}
		}
		if(is_hit != nullptr) *is_hit = false;
		return 0xff;
	}
	// ToDo: endian
	uint32_t read_data16w(uint32_t addr, int* wait, bool *is_hit)
	{
		pair16_t n;
		if(cache_hit_range(addr, 2)) {
			n.b.l =  cache_data[addr - start_addr + 0];
			n.b.h =  cache_data[addr - start_addr + 1];
			if(is_hit != NULL) *is_hit = true;
			return (uint32_t)(n.w);
		} else {
			bool bl, bh;
			n.b.l = read_data8w(addr + 0, wait, &bl);
			n.b.h = read_data8w(addr + 0, wait, &bh);
			if(is_hit != NULL) *is_hit = (bl & bh);
			if((bl) && (bh)) return (uint32_t)(n.w);
		}
		return 0xffff;
	}			
	uint32_t read_data32w(uint32_t addr, int* wait, bool *is_hit)
	{
		pair32_t n;
		if(cache_hit_range(addr, 4)) {
			n.b.l  =  cache_data[addr - start_addr + 0];
			n.b.h  =  cache_data[addr - start_addr + 1];
			n.b.h2 =  cache_data[addr - start_addr + 2];
			n.b.h3 =  cache_data[addr - start_addr + 3];
			if(is_hit != NULL) *is_hit = true;
			return n.d;
		} else {
			bool bl, bh, bh2, bh3;
			n.b.l  = read_data8w(addr + 0, wait, &bl);
			n.b.h  = read_data8w(addr + 1, wait, &bh);
			n.b.h2 = read_data8w(addr + 2, wait, &bh2);
			n.b.h2 = read_data8w(addr + 2, wait, &bh3);
			if(is_hit != NULL) *is_hit = (bl & bh & bh2 & bh3);
			if((bl) && (bh) && (bh2) && (bh3)) {
				return n.d;
			}
		}
		return 0xffffffff;
	}			
	void write_data8w(uint32_t addr, uint32_t data, int* wait)
	{
		if(region_check(addr)) {
			if(d_device != NULL) {
				d_device->write_data8w(addr, wait);
				cache_data[addr - start_addr] = (uint8_t)data;
				is_exists[addr - start_addr] = true;
			} else {
				if(wait != nullptr) *wait = 0;
				cache_data[addr - start_addr] = (uint8_t)0xff;
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
