

//**************************************************************************
//  DEBUGGING
//**************************************************************************

#define VERBOSE         (0)

#define VPRINTF(x)  do { if (VERBOSE) printf x; } while (0)

#define VALIDATE_REFCOUNTS 1

void handler_entry::dump_map(std::vector<memory_entry> &map) const
{
	fatalerror("dump_map called on non-dispatching class\n");
}

void handler_entry::reflist::add(const handler_entry *entry)
{
	refcounts[entry]++;
	if(seen.find(entry) == seen.end()) {
		seen.insert(entry);
		todo.insert(entry);
	}
}

void handler_entry::reflist::propagate()
{
	while(!todo.empty()) {
		const handler_entry *entry = *todo.begin();
		todo.erase(todo.begin());
		entry->enumerate_references(*this);
	}
}

void handler_entry::reflist::check()
{
	bool bad = false;
	for(const auto &i : refcounts) {
		if(i.first->get_refcount() != i.second) {
			fprintf(stderr, "Reference count error on handler \"%s\" stored %u real %u.\n",
					i.first->name().c_str(), i.first->get_refcount(), i.second);
			bad = true;
		}
	}
	if(bad)
		abort();
}


// default handler methods

void handler_entry::enumerate_references(handler_entry::reflist &refs) const
{
}

template<int Width, int AddrShift, int Endian> void handler_entry_read<Width, AddrShift, Endian>::populate_nomirror(offs_t start, offs_t end, offs_t ostart, offs_t oend, handler_entry_read<Width, AddrShift, Endian> *handler)
{
	fatalerror("populate called on non-dispatching class\n");
}

template<int Width, int AddrShift, int Endian> void handler_entry_read<Width, AddrShift, Endian>::populate_mirror(offs_t start, offs_t end, offs_t ostart, offs_t oend, offs_t mirror, handler_entry_read<Width, AddrShift, Endian> *handler)
{
	fatalerror("populate called on non-dispatching class\n");
}

template<int Width, int AddrShift, int Endian> void handler_entry_read<Width, AddrShift, Endian>::populate_mismatched_nomirror(offs_t start, offs_t end, offs_t ostart, offs_t oend, const memory_units_descriptor<Width, AddrShift, Endian> &descriptor, u8 rkey, std::vector<mapping> &mappings)
{
	fatalerror("populate_mismatched called on non-dispatching class\n");
}

template<int Width, int AddrShift, int Endian> void handler_entry_read<Width, AddrShift, Endian>::populate_mismatched_mirror(offs_t start, offs_t end, offs_t ostart, offs_t oend, offs_t mirror, const memory_units_descriptor<Width, AddrShift, Endian> &descriptor, std::vector<mapping> &mappings)
{
	fatalerror("populate_mismatched called on non-dispatching class\n");
}

template<int Width, int AddrShift, int Endian> void handler_entry_read<Width, AddrShift, Endian>::populate_passthrough_nomirror(offs_t start, offs_t end, offs_t ostart, offs_t oend, handler_entry_read_passthrough<Width, AddrShift, Endian> *handler, std::vector<mapping> &mappings)
{
	fatalerror("populate_passthrough called on non-dispatching class\n");
}

template<int Width, int AddrShift, int Endian> void handler_entry_read<Width, AddrShift, Endian>::populate_passthrough_mirror(offs_t start, offs_t end, offs_t ostart, offs_t oend, offs_t mirror, handler_entry_read_passthrough<Width, AddrShift, Endian> *handler, std::vector<mapping> &mappings)
{
	fatalerror("populate_passthrough called on non-dispatching class\n");
}

template<int Width, int AddrShift, int Endian> void handler_entry_read<Width, AddrShift, Endian>::lookup(offs_t address, offs_t &start, offs_t &end, handler_entry_read<Width, AddrShift, Endian> *&handler) const
{
	fatalerror("lookup called on non-dispatching class\n");
}

template<int Width, int AddrShift, int Endian> void *handler_entry_read<Width, AddrShift, Endian>::get_ptr(offs_t offset) const
{
	return nullptr;
}

template<int Width, int AddrShift, int Endian> void handler_entry_read<Width, AddrShift, Endian>::detach(const std::unordered_set<handler_entry *> &handlers)
{
	fatalerror("detach called on non-dispatching class\n");
}


template<int Width, int AddrShift, int Endian> void handler_entry_write<Width, AddrShift, Endian>::populate_nomirror(offs_t start, offs_t end, offs_t ostart, offs_t oend, handler_entry_write<Width, AddrShift, Endian> *handler)
{
	fatalerror("populate called on non-dispatching class\n");
}

template<int Width, int AddrShift, int Endian> void handler_entry_write<Width, AddrShift, Endian>::populate_mirror(offs_t start, offs_t end, offs_t ostart, offs_t oend, offs_t mirror, handler_entry_write<Width, AddrShift, Endian> *handler)
{
	fatalerror("populate called on non-dispatching class\n");
}

template<int Width, int AddrShift, int Endian> void handler_entry_write<Width, AddrShift, Endian>::populate_mismatched_nomirror(offs_t start, offs_t end, offs_t ostart, offs_t oend, const memory_units_descriptor<Width, AddrShift, Endian> &descriptor, u8 rkey, std::vector<mapping> &mappings)
{
	fatalerror("populate_mismatched called on non-dispatching class\n");
}

template<int Width, int AddrShift, int Endian> void handler_entry_write<Width, AddrShift, Endian>::populate_mismatched_mirror(offs_t start, offs_t end, offs_t ostart, offs_t oend, offs_t mirror, const memory_units_descriptor<Width, AddrShift, Endian> &descriptor, std::vector<mapping> &mappings)
{
	fatalerror("populate_mismatched called on non-dispatching class\n");
}

template<int Width, int AddrShift, int Endian> void handler_entry_write<Width, AddrShift, Endian>::populate_passthrough_nomirror(offs_t start, offs_t end, offs_t ostart, offs_t oend, handler_entry_write_passthrough<Width, AddrShift, Endian> *handler, std::vector<mapping> &mappings)
{
	fatalerror("populate_passthrough called on non-dispatching class\n");
}

template<int Width, int AddrShift, int Endian> void handler_entry_write<Width, AddrShift, Endian>::populate_passthrough_mirror(offs_t start, offs_t end, offs_t ostart, offs_t oend, offs_t mirror, handler_entry_write_passthrough<Width, AddrShift, Endian> *handler, std::vector<mapping> &mappings)
{
	fatalerror("populate_passthrough called on non-dispatching class\n");
}

template<int Width, int AddrShift, int Endian> void handler_entry_write<Width, AddrShift, Endian>::lookup(offs_t address, offs_t &start, offs_t &end, handler_entry_write<Width, AddrShift, Endian> *&handler) const
{
	fatalerror("lookup called on non-dispatching class\n");
}

template<int Width, int AddrShift, int Endian> void *handler_entry_write<Width, AddrShift, Endian>::get_ptr(offs_t offset) const
{
	return nullptr;
}

template<int Width, int AddrShift, int Endian> void handler_entry_write<Width, AddrShift, Endian>::detach(const std::unordered_set<handler_entry *> &handlers)
{
	fatalerror("detach called on non-dispatching class\n");
}


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
