

//**************************************************************************
//  CACHE MEMORY RANGES
//**************************************************************************

//-------------------------------------------------
//  memory_access_cache - constructor
//-------------------------------------------------

template<int Width, int AddrShift, int Endian> memory_access_cache<Width, AddrShift, Endian>::memory_access_cache(address_space &space,
																												  handler_entry_read <Width, AddrShift, Endian> *root_read,
																												  handler_entry_write<Width, AddrShift, Endian> *root_write)
	: m_space(space),
	  m_addrmask(space.addrmask()),
	  m_addrstart_r(1),
	  m_addrend_r(0),
	  m_addrstart_w(1),
	  m_addrend_w(0),
	  m_cache_r(nullptr),
	  m_cache_w(nullptr),
	  m_root_read(root_read),
	  m_root_write(root_write)
{
	
//	m_notifier_id = space.add_change_notifier([this](read_or_write mode) {
//												  if(u32(mode) & u32(read_or_write::READ)) {
//													  m_addrend_r = 0;
//													  m_addrstart_r = 1;
//													  m_cache_r = nullptr;
//												  }
//												  if(u32(mode) & u32(read_or_write::WRITE)) {
//													  m_addrend_w = 0;
//													  m_addrstart_w = 1;
//													  m_cache_w = nullptr;
//												  }
//											  });
}


//-------------------------------------------------
//  ~memory_access_cache - destructor
//-------------------------------------------------

template<int Width, int AddrShift, int Endian> memory_access_cache<Width, AddrShift, Endian>::~memory_access_cache()
{
//	m_space.remove_change_notifier(m_notifier_id);
}


template class memory_access_cache<0,  1, ENDIANNESS_LITTLE>;
template class memory_access_cache<0,  1, ENDIANNESS_BIG>;
template class memory_access_cache<0,  0, ENDIANNESS_LITTLE>;
template class memory_access_cache<0,  0, ENDIANNESS_BIG>;
template class memory_access_cache<1,  3, ENDIANNESS_LITTLE>;
template class memory_access_cache<1,  3, ENDIANNESS_BIG>;
template class memory_access_cache<1,  0, ENDIANNESS_LITTLE>;
template class memory_access_cache<1,  0, ENDIANNESS_BIG>;
template class memory_access_cache<1, -1, ENDIANNESS_LITTLE>;
template class memory_access_cache<1, -1, ENDIANNESS_BIG>;
template class memory_access_cache<2,  0, ENDIANNESS_LITTLE>;
template class memory_access_cache<2,  0, ENDIANNESS_BIG>;
template class memory_access_cache<2, -1, ENDIANNESS_LITTLE>;
template class memory_access_cache<2, -1, ENDIANNESS_BIG>;
template class memory_access_cache<2, -2, ENDIANNESS_LITTLE>;
template class memory_access_cache<2, -2, ENDIANNESS_BIG>;
template class memory_access_cache<3,  0, ENDIANNESS_LITTLE>;
template class memory_access_cache<3,  0, ENDIANNESS_BIG>;
template class memory_access_cache<3, -1, ENDIANNESS_LITTLE>;
template class memory_access_cache<3, -1, ENDIANNESS_BIG>;
template class memory_access_cache<3, -2, ENDIANNESS_LITTLE>;
template class memory_access_cache<3, -2, ENDIANNESS_BIG>;
template class memory_access_cache<3, -3, ENDIANNESS_LITTLE>;
template class memory_access_cache<3, -3, ENDIANNESS_BIG>;

uint8_t read_native(offs_t offset, uint8_t mask)
{
//	g_profiler.start(PROFILER_MEMREAD);
	
	uint8_t result = m_root_read->read_data8(offset) & mask;
	
//	g_profiler.stop();
	return result;
}

uint16_t read_native(offs_t offset, uint16_t mask)
{
//	g_profiler.start(PROFILER_MEMREAD);
	
	uint16_t result = m_root_read->read_data16(offset) & mask;
	
//	g_profiler.stop();
	return result;
}

uint32_t read_native(offs_t offset, uint32_t mask)
{
//	g_profiler.start(PROFILER_MEMREAD);
	
	uint32_t result = m_root_read->read_data32(offset) & mask;
	
//	g_profiler.stop();
	return result;
}

uint64_t read_native(offs_t offset, uint64_t mask)
{
//	g_profiler.start(PROFILER_MEMREAD);
	
	pair64_t result;
	pair64_t pmask;
	pmask.q = mask;
	result.d.l	= m_root_read->read_data32(offset) & pmask.l;
	result.d.h	= m_root_read->read_data32(offset + 4) & pmask.h;
	
//	g_profiler.stop();
	return result;
}

void write_native(offs_t offset, uint8_t data, uint8_t mask)
{
//	g_profiler.start(PROFILER_MEMWRITE);

	if(mask != 0xff) {
		uint8_t n = m_root_write->read_data8(offset);
		n = n & ~(mask);
		n = n | (data & mask);
		m_root_write->write_data8(offset, n);
	} else {
		m_root_write->write_data8(offset, data);
	}		
	
//	g_profiler.stop();
}

void write_native(offs_t offset, uint16_t data, uint16_t mask)
{
//	g_profiler.start(PROFILER_MEMWRITE);

	if(mask != 0xffff) {
		uint16_t n = m_root_write->read_data16(offset);
		n = n & ~(mask);
		n = n | (data & mask);
		m_root_write->write_data16(offset, n);
	} else {
		m_root_write->write_data16(offset, data);
	}		
	
//	g_profiler.stop();
}

void write_native(offs_t offset, uint32_t data, uint32_t mask)
{
//	g_profiler.start(PROFILER_MEMWRITE);

	if(mask != 0xffffffff) {
		uint32_t n = m_root_write->read_data32(offset);
		n = n & ~(mask);
		n = n | (data & mask);
		m_root_write->write_data32(offset, n);
	} else {
		m_root_write->write_data32(offset, data);
	}		
	
//	g_profiler.stop();
}

void write_native(offs_t offset, uint64_t data, uint64_t mask)
{
//	g_profiler.start(PROFILER_MEMWRITE);

	pair64_t d;
	d.q = data;
	if(mask != ~((uint64_t)0)) {
		pair64_t n;
		pair64_t m;
		m.q = mask;
		n.d.l = m_root_write->read_data32(offset);
		n.d.h = m_root_write->read_data32(offset + 4);
		n.q = n.q & ~(m.q);
		d.q = d.q & m.q;
		d.q = d.q | n.q;
		m_root_write->write_data32(offset, d.d.l);
		m_root_write->write_data32(offset + 4, d.d.h);
	} else {
		m_root_write->write_data32(offset, d.d.l);
		m_root_write->write_data32(offset + 4, d.d.h);
	}		
	
//	g_profiler.stop();
}
