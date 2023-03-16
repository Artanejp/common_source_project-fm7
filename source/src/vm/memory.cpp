/*
	Skelton for retropc emulator

	Author : Takeda.Toshiya
	Date   : 2010.09.16-

	[ memory ]
*/

// NOTE: this memory bus class invites the cpu is little endian.
// if the cpu is big endian, you need to use the memory bus class for big endian (not implemented yet).

#include "memory.h"

#define ADDR_MASK (space - 1)
#define BANK_MASK (bank_size - 1)

void MEMORY::initialize()
{
	__UNLIKELY_IF((rd_table == NULL) || (wr_table == NULL)) {
		DEVICE::initialize();
	}
	// allocate tables here to support multiple instances with different address range
	if(rd_table == NULL) {
		int64_t bank_num = space / bank_size;
		bank_mask = BANK_MASK;
		addr_mask = ADDR_MASK;

		rd_dummy = (uint8_t *)malloc(bank_size);
		wr_dummy = (uint8_t *)malloc(bank_size);

		rd_table = (bank_t *)calloc(bank_num, sizeof(bank_t));
		wr_table = (bank_t *)calloc(bank_num, sizeof(bank_t));

		// May initialize for security reason. 20230317 K.O
		memset(rd_table, 0x00, sizeof(bank_t) * bank_num);
		memset(wr_table, 0x00, sizeof(bank_t) * bank_num);

		for(int i = 0; i < bank_num; i++) {
			rd_table[i].memory = rd_dummy;

			wr_table[i].memory = wr_dummy;
		}
		for(int i = 0;; i++) {
			if(bank_size == (uint64_t)(1 << i)) {
				addr_shift = i;
				break;
			}
		}
		memset(rd_dummy, 0xff, bank_size);
	}
}

void MEMORY::release()
{
	free(rd_table);
	free(wr_table);
	free(rd_dummy);
	free(wr_dummy);
	rd_table = NULL;
	wr_table = NULL;
	rd_dummy = NULL;
	wr_dummy = NULL;

	DEVICE::release();
}

uint32_t MEMORY::read_data8(uint32_t addr)
{
	const int bank = get_bank(addr);

	__LIKELY_IF(rd_table[bank].device != NULL) {
		return rd_table[bank].device->read_memory_mapped_io8(addr);
	} else {
		return rd_table[bank].memory[addr & bank_mask];
	}
}

void MEMORY::write_data8(uint32_t addr, uint32_t data)
{
	const int bank = get_bank(addr);

	__LIKELY_IF(wr_table[bank].device != NULL) {
		wr_table[bank].device->write_memory_mapped_io8(addr, data);
	} else {
		wr_table[bank].memory[addr & bank_mask] = data;
	}
}

uint32_t MEMORY::read_data16(uint32_t addr)
{
	const uint32_t addr2 = addr & BANK_MASK;

	__LIKELY_IF(bus_width >= 16 && (addr2 + 1) < bank_size) {
		const int bank = get_bank(addr);

		__LIKELY_IF(rd_table[bank].device != NULL) {
			__LIKELY_IF(!(addr & 1)) {
				return rd_table[bank].device->read_memory_mapped_io16(addr);
			} else {
				uint32_t val;
				val  = rd_table[bank].device->read_memory_mapped_io8(addr    );
				val |= rd_table[bank].device->read_memory_mapped_io8(addr + 1) << 8;
				return val;
			}
		} else {
			#ifdef __BIG_ENDIAN__
				uint32_t val;
				val  = rd_table[bank].memory[addr2    ];
				val |= rd_table[bank].memory[addr2 + 1] <<  8;
				return val;
			#else
				return *(uint16_t *)(rd_table[bank].memory + addr2);
			#endif
		}
	} else {
		uint32_t val;
		val  = read_data8(addr    );
 		val |= read_data8(addr + 1) << 8;
 		return val;
 	}
}

void MEMORY::write_data16(uint32_t addr, uint32_t data)
{
	const uint32_t addr2 = addr & BANK_MASK;

	__LIKELY_IF(bus_width >= 16 && (addr2 + 1) < bank_size) {
		const int bank = get_bank(addr);

		__LIKELY_IF(wr_table[bank].device != NULL) {
			__LIKELY_IF(!(addr & 1)) {
				wr_table[bank].device->write_memory_mapped_io16(addr, data);
			} else {
				wr_table[bank].device->write_memory_mapped_io8(addr    , (data     ) & 0xff);
				wr_table[bank].device->write_memory_mapped_io8(addr + 1, (data >> 8) & 0xff);
			}
		} else {
			#ifdef __BIG_ENDIAN__
				wr_table[bank].memory[addr2    ] = (data     ) & 0xff
				wr_table[bank].memory[addr2 + 1] = (data >> 8) & 0xff
			#else
				*(uint16_t *)(wr_table[bank].memory + addr2) = data;
			#endif
		}
 	} else {
		write_data8(addr    , (data     ) & 0xff);
 		write_data8(addr + 1, (data >> 8) & 0xff);
 	}

}

uint32_t MEMORY::read_data32(uint32_t addr)
{
	const uint32_t addr2 = addr & BANK_MASK;

	__LIKELY_IF(bus_width >= 32 && (addr2 + 3) < bank_size) {
		const int bank = get_bank(addr);

		__LIKELY_IF(rd_table[bank].device != NULL) {
			__LIKELY_IF(!(addr & 3)) {
				return rd_table[bank].device->read_memory_mapped_io32(addr);
			} else if(!(addr & 1)) {
				uint32_t val;
				val  = rd_table[bank].device->read_memory_mapped_io16(addr    );
				val |= rd_table[bank].device->read_memory_mapped_io16(addr + 2) << 16;
				return val;
			} else {
				uint32_t val;
				val  = rd_table[bank].device->read_memory_mapped_io8 (addr    );
				val |= rd_table[bank].device->read_memory_mapped_io16(addr + 1) <<  8;
				val |= rd_table[bank].device->read_memory_mapped_io8 (addr + 3) << 24;
				return val;
			}
		} else {
			#ifdef __BIG_ENDIAN__
				uint32_t val;
				val  = rd_table[bank].memory[addr2    ];
				val |= rd_table[bank].memory[addr2 + 1] <<  8;
				val |= rd_table[bank].memory[addr2 + 2] << 16;
				val |= rd_table[bank].memory[addr2 + 3] << 24;
				return val;
			#else
				return *(uint32_t *)(rd_table[bank].memory + addr2);
			#endif
		}
	} else if(!(addr & 1)) {
		uint32_t val;
		val  = read_data16(addr    );
 		val |= read_data16(addr + 2) << 16;
 		return val;
	} else {
		uint32_t val;
		val  = read_data8 (addr    );
		val |= read_data16(addr + 1) <<  8;
		val |= read_data8 (addr + 3) << 24;
		return val;
 	}
}

void MEMORY::write_data32(uint32_t addr, uint32_t data)
{
	const uint32_t addr2 = addr & BANK_MASK;

	__LIKELY_IF(bus_width >= 32 && (addr2 + 3) < bank_size) {
		const int bank = get_bank(addr);

		if(wr_table[bank].device != NULL) {
			wr_table[bank].device->write_memory_mapped_io32(addr, data);
		} else {
			#ifdef __BIG_ENDIAN__
				wr_table[bank].memory[addr2    ] = (data      ) & 0xff
				wr_table[bank].memory[addr2 + 1] = (data >>  8) & 0xff
				wr_table[bank].memory[addr2 + 2] = (data >> 16) & 0xff
				wr_table[bank].memory[addr2 + 3] = (data >> 24) & 0xff
			#else
				*(uint32_t *)(wr_table[bank].memory + addr2) = data;
			#endif
		}
	} else if(!(addr & 1)) {
		write_data16(addr    , (data      ) & 0xffff);
 		write_data16(addr + 2, (data >> 16) & 0xffff);
	} else {
		write_data8 (addr    , (data      ) & 0x00ff);
		write_data16(addr + 1, (data >>  8) & 0xffff);
		write_data8 (addr + 3, (data >> 24) & 0x00ff);
 	}
}

uint32_t MEMORY::read_data8w(uint32_t addr, int* wait)
{
	const int bank = get_bank(addr);

	*wait = rd_table[bank].wait;

	if(rd_table[bank].device != NULL) {
		int wait_tmp = 0;
		uint32_t val = rd_table[bank].device->read_memory_mapped_io8w(addr, &wait_tmp);
		__UNLIKELY_IF(!rd_table[bank].wait_registered) *wait = wait_tmp;
		return val;
	} else {
		return rd_table[bank].memory[addr & BANK_MASK];
	}
}

void MEMORY::write_data8w(uint32_t addr, uint32_t data, int* wait)
{
	const int bank = get_bank(addr);

	*wait = wr_table[bank].wait;

	if(wr_table[bank].device != NULL) {
		int wait_tmp = 0;
		wr_table[bank].device->write_memory_mapped_io8w(addr, data, &wait_tmp);
		__UNLIKELY_IF(!wr_table[bank].wait_registered) *wait = wait_tmp;
	} else {
		wr_table[bank].memory[addr & BANK_MASK] = data;
	}
}

uint32_t MEMORY::read_data16w(uint32_t addr, int* wait)
{
	const uint32_t addr2 = addr & BANK_MASK;

	__LIKELY_IF(bus_width >= 16 && (addr2 + 1) < bank_size) {
		const int bank = get_bank(addr);

		*wait = rd_table[bank].wait * bus_access_times_16(addr2);

		__LIKELY_IF(rd_table[bank].device != NULL) {
			__LIKELY_IF(!(addr & 1)) {
				int wait_tmp = 0;
				uint32_t val = rd_table[bank].device->read_memory_mapped_io16w(addr, &wait_tmp);
				__UNLIKELY_IF(!rd_table[bank].wait_registered) *wait = wait_tmp;
				return val;
			} else {
				int wait_l = 0, wait_h = 0;
				uint32_t val;
				val  = rd_table[bank].device->read_memory_mapped_io8w(addr    , &wait_l);
				val |= rd_table[bank].device->read_memory_mapped_io8w(addr + 1, &wait_h) << 8;
				__UNLIKELY_IF(!rd_table[bank].wait_registered) *wait = wait_l + wait_h;
				return val;
			}
		} else {
			#ifdef __BIG_ENDIAN__
				uint32_t val;
				val  = rd_table[bank].memory[addr2    ];
				val |= rd_table[bank].memory[addr2 + 1] <<  8;
				return val;
			#else
				return *(uint16_t *)(rd_table[bank].memory + addr2);
			#endif
		}
	} else {
		int wait_l = 0, wait_h = 0;
		uint32_t val;
		val  = read_data8w(addr    , &wait_l);
		val |= read_data8w(addr + 1, &wait_h) << 8;
		*wait = wait_l + wait_h;
		return val;
	}
}

void MEMORY::write_data16w(uint32_t addr, uint32_t data, int* wait)
{
	const uint32_t addr2 = addr & BANK_MASK;

	__LIKELY_IF(bus_width >= 16 && (addr2 + 1) < bank_size) {
		const int bank = get_bank(addr);

		*wait = wr_table[bank].wait * bus_access_times_16(addr2);

		__LIKELY_IF(wr_table[bank].device != NULL) {
			__LIKELY_IF(!(addr & 1)) {
				int wait_tmp = 0;
				wr_table[bank].device->write_memory_mapped_io16w(addr, data, &wait_tmp);
				__UNLIKELY_IF(!wr_table[bank].wait_registered) *wait = wait_tmp;
			} else {
				int wait_l = 0, wait_h = 0;
				wr_table[bank].device->write_memory_mapped_io8w(addr    , (data     ) & 0xff, &wait_l);
				wr_table[bank].device->write_memory_mapped_io8w(addr + 1, (data >> 8) & 0xff, &wait_h);
				__UNLIKELY_IF(!wr_table[bank].wait_registered) *wait = wait_l + wait_h;
			}
		} else {
			#ifdef __BIG_ENDIAN__
				wr_table[bank].memory[addr2    ] = (data     ) & 0xff
				wr_table[bank].memory[addr2 + 1] = (data >> 8) & 0xff
			#else
				*(uint16_t *)(wr_table[bank].memory + addr2) = data;
			#endif
		}
	} else {
		int wait_l = 0, wait_h = 0;
		write_data8w(addr    , (data     ) & 0xff, &wait_l);
		write_data8w(addr + 1, (data >> 8) & 0xff, &wait_h);
		*wait = wait_l + wait_h;
	}
}

uint32_t MEMORY::read_data32w(uint32_t addr, int* wait)
{
	const uint32_t addr2 = addr & BANK_MASK;

	__LIKELY_IF(bus_width >= 32 && (addr2 + 3) < bank_size) {
		const int bank = get_bank(addr);

		*wait = rd_table[bank].wait * bus_access_times_32(addr2);

		__LIKELY_IF(rd_table[bank].device != NULL) {
			__LIKELY_IF(!(addr & 3)) {
				int wait_tmp = 0;
				uint32_t val = rd_table[bank].device->read_memory_mapped_io32w(addr, &wait_tmp);
				__UNLIKELY_IF(!rd_table[bank].wait_registered) *wait = wait_tmp;
				return val;
			} else if(!(addr & 1)) {
				int wait_l = 0, wait_h = 0;
				uint32_t val;
				val  = rd_table[bank].device->read_memory_mapped_io16w(addr    , &wait_l);
				val |= rd_table[bank].device->read_memory_mapped_io16w(addr + 2, &wait_h) << 16;
				__UNLIKELY_IF(!rd_table[bank].wait_registered) *wait = wait_l + wait_h;
				return val;
			} else {
				int wait_l = 0, wait_m = 0, wait_h = 0;
				uint32_t val;
				val  = rd_table[bank].device->read_memory_mapped_io8w (addr    , &wait_l);
				val |= rd_table[bank].device->read_memory_mapped_io16w(addr + 1, &wait_m) <<  8;
				val |= rd_table[bank].device->read_memory_mapped_io8w (addr + 3, &wait_h) << 24;
				if(!rd_table[bank].wait_registered) *wait = wait_l + wait_m + wait_h;
				return val;
			}
		} else {
			#ifdef __BIG_ENDIAN__
				uint32_t val;
				val  = rd_table[bank].memory[addr2    ];
				val |= rd_table[bank].memory[addr2 + 1] <<  8;
				val |= rd_table[bank].memory[addr2 + 2] << 16;
				val |= rd_table[bank].memory[addr2 + 3] << 24;
				return val;
			#else
				return *(uint32_t *)(rd_table[bank].memory + addr2);
			#endif
		}
	} else if(!(addr & 1)) {
		int wait_l = 0, wait_h = 0;
		uint32_t val;
		val  = read_data16w(addr    , &wait_l);
		val |= read_data16w(addr + 2, &wait_h) << 16;
		*wait = wait_l + wait_h;
		return val;
	} else {
		int wait_l = 0, wait_m = 0, wait_h = 0;
		uint32_t val;
		val  = read_data8w (addr    , &wait_l);
		val |= read_data16w(addr + 1, &wait_m) <<  8;
		val |= read_data8w (addr + 3, &wait_h) << 24;
		*wait = wait_l + wait_m + wait_h;
		return val;
	}
}

void MEMORY::write_data32w(uint32_t addr, uint32_t data, int* wait)
{
	const uint32_t addr2 = addr & BANK_MASK;

	__LIKELY_IF(bus_width >= 32 && (addr2 + 3) < bank_size) {
		const int bank = get_bank(addr);

		*wait = wr_table[bank].wait * bus_access_times_32(addr2);

		__LIKELY_IF(wr_table[bank].device != NULL) {
			__LIKELY_IF(!(addr & 3)) {
				int wait_tmp = 0;
				wr_table[bank].device->write_memory_mapped_io32w(addr, data, &wait_tmp);
				__UNLIKELY_IF(!wr_table[bank].wait_registered) *wait = wait_tmp;
			} else if(!(addr & 1)) {
				int wait_l = 0, wait_h = 0;
				wr_table[bank].device->write_memory_mapped_io16w(addr    , (data      ) & 0xffff, &wait_l);
				wr_table[bank].device->write_memory_mapped_io16w(addr + 2, (data >> 16) & 0xffff, &wait_h);
				__UNLIKELY_IF(!wr_table[bank].wait_registered) *wait = wait_l + wait_h;
			} else {
				int wait_l = 0, wait_m = 0, wait_h = 0;
				wr_table[bank].device->write_memory_mapped_io8w (addr    , (data      ) & 0x00ff, &wait_l);
				wr_table[bank].device->write_memory_mapped_io16w(addr + 1, (data >>  8) & 0xffff, &wait_m);
				wr_table[bank].device->write_memory_mapped_io8w (addr + 3, (data >> 24) & 0x00ff, &wait_h);
				if(!wr_table[bank].wait_registered) *wait = wait_l + wait_m + wait_h;
			}
		} else {
			#ifdef __BIG_ENDIAN__
				wr_table[bank].memory[addr2    ] = (data      ) & 0xff
				wr_table[bank].memory[addr2 + 1] = (data >>  8) & 0xff
				wr_table[bank].memory[addr2 + 2] = (data >> 16) & 0xff
				wr_table[bank].memory[addr2 + 3] = (data >> 24) & 0xff
			#else
				*(uint32_t *)(wr_table[bank].memory + addr2) = data;
			#endif
		}
	} else if(!(addr & 1)) {
		int wait_l = 0, wait_h = 0;
		write_data16w(addr    , (data      ) & 0xffff, &wait_l);
		write_data16w(addr + 2, (data >> 16) & 0xffff, &wait_h);
		*wait = wait_l + wait_h;
	} else {
		int wait_l = 0, wait_m = 0, wait_h = 0;
		write_data8w (addr    , (data      ) & 0x00ff, &wait_l);
		write_data16w(addr + 1, (data >>  8) & 0xffff, &wait_m);
		write_data8w (addr + 3, (data >> 24) & 0x00ff, &wait_h);
		*wait = wait_l + wait_m + wait_h;
	}
}

uint32_t MEMORY::read_dma_data8(uint32_t addr)
{
	if(!(_MEMORY_DISABLE_DMA_MMIO)) {
		return read_data8(addr);
	}
	int bank = (addr & ADDR_MASK) >> addr_shift;

	if(rd_table[bank].device != NULL) {
//		return rd_table[bank].device->read_memory_mapped_io8(addr);
		return 0xff;
	} else {
		return rd_table[bank].memory[addr & bank_mask];
	}
}

void MEMORY::write_dma_data8(uint32_t addr, uint32_t data)
{
	if(!(_MEMORY_DISABLE_DMA_MMIO)) {
		write_data8(addr, data);
		return;
	}
	int bank = (addr & ADDR_MASK) >> addr_shift;

	if(wr_table[bank].device != NULL) {
//		wr_table[bank].device->write_memory_mapped_io8(addr, data);
	} else {
		wr_table[bank].memory[addr & bank_mask] = data;
	}
}

uint32_t MEMORY::read_dma_data16(uint32_t addr)
{
	if(!(_MEMORY_DISABLE_DMA_MMIO)) {
		return read_data16(addr);
	}
	int bank = (addr & ADDR_MASK) >> addr_shift;

	if(rd_table[bank].device != NULL) {
//		return rd_table[bank].device->read_memory_mapped_io16(addr);
		return 0xffff;
	} else {
		uint32_t val = read_dma_data8(addr);
		val |= read_dma_data8(addr + 1) << 8;
		return val;
	}
}

void MEMORY::write_dma_data16(uint32_t addr, uint32_t data)
{
	if(!(_MEMORY_DISABLE_DMA_MMIO)) {
		write_data16(addr, data);
		return;
	}
	int bank = (addr & ADDR_MASK) >> addr_shift;

	if(wr_table[bank].device != NULL) {
//		wr_table[bank].device->write_memory_mapped_io16(addr, data);
	} else {
		write_dma_data8(addr, data & 0xff);
		write_dma_data8(addr + 1, (data >> 8) & 0xff);
	}
}

uint32_t MEMORY::read_dma_data32(uint32_t addr)
{
	int bank = (addr & ADDR_MASK) >> addr_shift;

	if(rd_table[bank].device != NULL) {
//		return rd_table[bank].device->read_memory_mapped_io32(addr);
		return 0xffffffff;
	} else {
		uint32_t val = read_dma_data16(addr);
		val |= read_dma_data16(addr + 2) << 16;
		return val;
	}
}

void MEMORY::write_dma_data32(uint32_t addr, uint32_t data)
{
	int bank = (addr & ADDR_MASK) >> addr_shift;

	if(wr_table[bank].device != NULL) {
//		wr_table[bank].device->write_memory_mapped_io32(addr, data);
	} else {
		write_dma_data16(addr, data & 0xffff);
		write_dma_data16(addr + 2, (data >> 16) & 0xffff);
	}
}

// register

void MEMORY::set_memory_r(uint32_t start, uint32_t end, uint8_t *memory)
{
	MEMORY::initialize(); // subclass may overload initialize()

	const uint32_t start_bank = start >> addr_shift;
	const uint32_t end_bank = end >> addr_shift;

	for(uint32_t i = start_bank; i <= end_bank; i++) {
		rd_table[i].device = NULL;
		rd_table[i].memory = memory + bank_size * (i - start_bank);
	}
}

void MEMORY::set_memory_w(uint32_t start, uint32_t end, uint8_t *memory)
{
	MEMORY::initialize();

	const uint32_t start_bank = start >> addr_shift;
	const uint32_t end_bank = end >> addr_shift;

	for(uint32_t i = start_bank; i <= end_bank; i++) {
		wr_table[i].device = NULL;
		wr_table[i].memory = memory + bank_size * (i - start_bank);
	}
}

void MEMORY::set_memory_mapped_io_r(uint32_t start, uint32_t end, DEVICE *device)
{
	MEMORY::initialize();

	const uint32_t start_bank = start >> addr_shift;
	const uint32_t end_bank = end >> addr_shift;

	for(uint32_t i = start_bank; i <= end_bank; i++) {
		rd_table[i].device = device;
	}
}

void MEMORY::set_memory_mapped_io_w(uint32_t start, uint32_t end, DEVICE *device)
{
	MEMORY::initialize();

	const uint32_t start_bank = start >> addr_shift;
	const uint32_t end_bank = end >> addr_shift;

	for(uint32_t i = start_bank; i <= end_bank; i++) {
		wr_table[i].device = device;
	}
}

void MEMORY::set_wait_r(uint32_t start, uint32_t end, int wait)
{
	MEMORY::initialize();

	const uint32_t start_bank = start >> addr_shift;
	const uint32_t end_bank = end >> addr_shift;

	for(uint32_t i = start_bank; i <= end_bank; i++) {
		rd_table[i].wait = wait;
		rd_table[i].wait_registered = true;
	}
}

void MEMORY::set_wait_w(uint32_t start, uint32_t end, int wait)
{
	MEMORY::initialize();

	const uint32_t start_bank = start >> addr_shift;
	const uint32_t end_bank = end >> addr_shift;

	for(uint32_t i = start_bank; i <= end_bank; i++) {
		wr_table[i].wait = wait;
		wr_table[i].wait_registered = true;
	}
}

void MEMORY::unset_memory_r(uint32_t start, uint32_t end)
{
	MEMORY::initialize();

	const uint32_t start_bank = start >> addr_shift;
	const uint32_t end_bank = end >> addr_shift;

	// Q: May need to reser wait_registed ? 20230317 K.O
	for(uint32_t i = start_bank; i <= end_bank; i++) {
		rd_table[i].device = NULL;
		rd_table[i].memory = rd_dummy;
	}
}

void MEMORY::unset_memory_w(uint32_t start, uint32_t end)
{
	MEMORY::initialize();

	const uint32_t start_bank = start >> addr_shift;
	const uint32_t end_bank = end >> addr_shift;

	// Q: May need to reser wait_registed ? 20230317 K.O
	for(uint32_t i = start_bank; i <= end_bank; i++) {
		wr_table[i].device = NULL;
		wr_table[i].memory = wr_dummy;
	}
}

void MEMORY::copy_table_w(uint32_t to, uint32_t start, uint32_t end)
{
	MEMORY::initialize();

	const uint32_t start_bank = start >> addr_shift;
	const uint32_t end_bank = end >> addr_shift;
	uint32_t to_bank = to >> addr_shift;
	const uint64_t blocks = space / bank_size;

	for(uint64_t i = start_bank; i <= end_bank; i++) {
		if(to_bank >= blocks) break;
		wr_table[to_bank].device = wr_table[i].device;
		wr_table[to_bank].memory = wr_table[i].memory;
		wr_table[to_bank].wait = wr_table[i].wait;
		wr_table[to_bank].wait_registered = wr_table[i].wait_registered;
		to_bank++;
	}
}

void MEMORY::copy_table_r(uint32_t to, uint32_t start, uint32_t end)
{
	MEMORY::initialize();

	const uint32_t start_bank = start >> addr_shift;
	const uint32_t end_bank = end >> addr_shift;
	uint32_t to_bank = to >> addr_shift;
	const uint64_t blocks = addr_max / bank_size;

	for(uint64_t i = start_bank; i <= end_bank; i++) {
		if(to_bank >= blocks) break;
		rd_table[to_bank].device = rd_table[i].device;
		rd_table[to_bank].memory = rd_table[i].memory;
		rd_table[to_bank].wait = rd_table[i].wait;
		rd_table[to_bank].wait_registered = rd_table[i].wait_registered;
		to_bank++;
	}
}


void MEMORY::unset_wait_r(uint32_t start, uint32_t end)
{
	MEMORY::initialize();

	uint32_t start_bank = start >> addr_shift;
	uint32_t end_bank = end >> addr_shift;

	for(uint32_t i = start_bank; i <= end_bank; i++) {
		rd_table[i].wait = 0;
		rd_table[i].wait_registered = false;
	}
}

void MEMORY::unset_wait_w(uint32_t start, uint32_t end)
{
	MEMORY::initialize();

	uint32_t start_bank = start >> addr_shift;
	uint32_t end_bank = end >> addr_shift;

	for(uint32_t i = start_bank; i <= end_bank; i++) {
		wr_table[i].wait = 0;
		wr_table[i].wait_registered = false;
	}
}


// load/save image

int MEMORY::read_bios(const _TCHAR *file_name, uint8_t *buffer, int size)
{
	FILEIO* fio = new FILEIO();
	int length = 0;
	if(fio->Fopen(create_local_path(file_name), FILEIO_READ_BINARY)) {
		fio->Fread(buffer, size, 1);
		length = fio->Ftell();
		fio->Fclose();
	}
	out_debug_log("LOADING ROM %s REQ_SIZE=%d PRESENTED_SIZE=%d", create_local_path(file_name), size, length);
	delete fio;
	return length;
}

bool MEMORY::write_bios(const _TCHAR *file_name, uint8_t *buffer, int size)
{
	FILEIO* fio = new FILEIO();
	bool result = false;

	if(fio->Fopen(create_local_path(file_name), FILEIO_WRITE_BINARY)) {
		fio->Fwrite(buffer, size, 1);
		fio->Fclose();
		result = true;
	}
	delete fio;
	return result;
}

bool MEMORY::read_image(const _TCHAR *file_path, uint8_t *buffer, int size)
{
	FILEIO* fio = new FILEIO();
	bool result = false;

	if(fio->Fopen(file_path, FILEIO_READ_BINARY)) {
		fio->Fread(buffer, size, 1);
		fio->Fclose();
		result = true;
	}
	delete fio;
	return result;
}

bool MEMORY::write_image(const _TCHAR* file_path, uint8_t* buffer, int size)
{
	FILEIO* fio = new FILEIO();
	bool result = false;

	if(fio->Fopen(file_path, FILEIO_WRITE_BINARY)) {
		fio->Fwrite(buffer, size, 1);
		fio->Fclose();
		result = true;
	}
	delete fio;
	return result;
}
