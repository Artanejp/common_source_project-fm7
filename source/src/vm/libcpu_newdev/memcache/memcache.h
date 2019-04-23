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

