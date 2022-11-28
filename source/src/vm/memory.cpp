/*
	Skelton for retropc emulator

	Author : Takeda.Toshiya
	Date   : 2010.09.16-

	[ memory ]
*/

// NOTE: this memory bus class invites the cpu is little endian.
// if the cpu is big endian, you need to use the memory bus class for big endian (not implemented yet).

#include "memory.h"

#define ADDR_MASK (addr_max - 1)
#define BANK_MASK (bank_size - 1)

void MEMORY::initialize()
{
	// allocate tables here to support multiple instances with different address range
	if(rd_table == NULL) {
		int bank_num = addr_max / bank_size;
		
		rd_dummy = (uint8_t *)malloc(bank_size);
		wr_dummy = (uint8_t *)malloc(bank_size);
		
		rd_table = (bank_t *)calloc(bank_num, sizeof(bank_t));
		wr_table = (bank_t *)calloc(bank_num, sizeof(bank_t));
		
		for(int i = 0; i < bank_num; i++) {
			rd_table[i].device = NULL;
			rd_table[i].memory = rd_dummy;
			rd_table[i].wait   = 0;
			
			wr_table[i].device = NULL;
			wr_table[i].memory = wr_dummy;
			wr_table[i].wait   = 0;
		}
		for(int i = 0;; i++) {
			if(bank_size == (1 << i)) {
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
}

uint32_t MEMORY::read_data8(uint32_t addr)
{
	int bank = (addr & ADDR_MASK) >> addr_shift;
	
	if(rd_table[bank].device != NULL) {
		return rd_table[bank].device->read_memory_mapped_io8(addr);
	} else {
		return rd_table[bank].memory[addr & BANK_MASK];
	}
}

void MEMORY::write_data8(uint32_t addr, uint32_t data)
{
	int bank = (addr & ADDR_MASK) >> addr_shift;
	
	if(wr_table[bank].device != NULL) {
		wr_table[bank].device->write_memory_mapped_io8(addr, data);
	} else {
		wr_table[bank].memory[addr & BANK_MASK] = data;
	}
}

uint32_t MEMORY::read_data16(uint32_t addr)
{
	uint32_t addr2 = addr & BANK_MASK;
	
	if(addr2 + 1 < bank_size) {
		int bank = (addr & ADDR_MASK) >> addr_shift;
		
		if(rd_table[bank].device != NULL) {
			return rd_table[bank].device->read_memory_mapped_io16(addr);
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
	uint32_t addr2 = addr & BANK_MASK;
	
	if(addr2 + 1 < bank_size) {
		int bank = (addr & ADDR_MASK) >> addr_shift;
		
		if(wr_table[bank].device != NULL) {
			wr_table[bank].device->write_memory_mapped_io16(addr, data);
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
	uint32_t addr2 = addr & BANK_MASK;
	
	if(addr2 + 3 < bank_size) {
		int bank = (addr & ADDR_MASK) >> addr_shift;
		
		if(rd_table[bank].device != NULL) {
			return rd_table[bank].device->read_memory_mapped_io32(addr);
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
	uint32_t addr2 = addr & BANK_MASK;
	
	if(addr2 + 3 < bank_size) {
		int bank = (addr & ADDR_MASK) >> addr_shift;
		
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
	int bank = (addr & ADDR_MASK) >> addr_shift;
	
	*wait = rd_table[bank].wait;
	if(rd_table[bank].device != NULL) {
		return rd_table[bank].device->read_memory_mapped_io8(addr);
	} else {
		return rd_table[bank].memory[addr & BANK_MASK];
	}
}

void MEMORY::write_data8w(uint32_t addr, uint32_t data, int* wait)
{
	int bank = (addr & ADDR_MASK) >> addr_shift;
	
	*wait = wr_table[bank].wait;
	if(wr_table[bank].device != NULL) {
		wr_table[bank].device->write_memory_mapped_io8(addr, data);
	} else {
		wr_table[bank].memory[addr & BANK_MASK] = data;
	}
}

uint32_t MEMORY::read_data16w(uint32_t addr, int* wait)
{
	uint32_t addr2 = addr & BANK_MASK;
	
	if(addr2 + 1 < bank_size) {
		int bank = (addr & ADDR_MASK) >> addr_shift;
		
		*wait = rd_table[bank].wait * 2; // 8bit bus ???
		
		if(rd_table[bank].device != NULL) {
			return rd_table[bank].device->read_memory_mapped_io16(addr);
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
		int wait_0, wait_1;
		uint32_t val;
		val  = read_data8w(addr    , &wait_0);
		val |= read_data8w(addr + 1, &wait_1) << 8;
		*wait = wait_0 + wait_1;
		return val;
	}
}

void MEMORY::write_data16w(uint32_t addr, uint32_t data, int* wait)
{
	uint32_t addr2 = addr & BANK_MASK;
	
	if(addr2 + 1 < bank_size) {
		int bank = (addr & ADDR_MASK) >> addr_shift;
		
		*wait = wr_table[bank].wait * 2; // 8bit bus ???
		
		if(wr_table[bank].device != NULL) {
			wr_table[bank].device->write_memory_mapped_io16(addr, data);
		} else {
			#ifdef __BIG_ENDIAN__
				wr_table[bank].memory[addr2    ] = (data     ) & 0xff
				wr_table[bank].memory[addr2 + 1] = (data >> 8) & 0xff
			#else
				*(uint16_t *)(wr_table[bank].memory + addr2) = data;
			#endif
		}
	} else {
		int wait_0, wait_1;
		write_data8w(addr    , (data     ) & 0xff, &wait_0);
		write_data8w(addr + 1, (data >> 8) & 0xff, &wait_1);
		*wait = wait_0 + wait_1;
	}
}

uint32_t MEMORY::read_data32w(uint32_t addr, int* wait)
{
	uint32_t addr2 = addr & BANK_MASK;
	
	if(addr2 + 3 < bank_size) {
		int bank = (addr & ADDR_MASK) >> addr_shift;
		
		*wait = rd_table[bank].wait * 4; // 8bit bus ???
		
		if(rd_table[bank].device != NULL) {
			return rd_table[bank].device->read_memory_mapped_io32(addr);
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
		int wait_0, wait_1;
		uint32_t val;
		val  = read_data16w(addr    , &wait_0);
		val |= read_data16w(addr + 2, &wait_1) << 16;
		*wait = wait_0 + wait_1;
		return val;
	} else {
		int wait_0, wait_1, wait_2;
		uint32_t val;
		val  = read_data8w (addr    , &wait_0);
		val |= read_data16w(addr + 1, &wait_1) <<  8;
		val |= read_data8w (addr + 3, &wait_2) << 24;
		*wait = wait_0 + wait_1 + wait_2;
		return val;
	}
}

void MEMORY::write_data32w(uint32_t addr, uint32_t data, int* wait)
{
	uint32_t addr2 = addr & BANK_MASK;
	
	if(addr2 + 3 < bank_size) {
		int bank = (addr & ADDR_MASK) >> addr_shift;
		
		*wait = wr_table[bank].wait * 4; // 8bit bus ???
		
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
		int wait_0, wait_1;
		write_data16w(addr    , (data      ) & 0xffff, &wait_0);
		write_data16w(addr + 2, (data >> 16) & 0xffff, &wait_1);
		*wait = wait_0 + wait_1;
	} else {
		int wait_0, wait_1, wait_2;
		write_data8w (addr    , (data      ) & 0x00ff, &wait_0);
		write_data16w(addr + 1, (data >>  8) & 0xffff, &wait_1);
		write_data8w (addr + 3, (data >> 24) & 0x00ff, &wait_2);
		*wait = wait_0 + wait_1 + wait_2;
	}
}

#ifdef MEMORY_DISABLE_DMA_MMIO
uint32_t MEMORY::read_dma_data8(uint32_t addr)
{
	int bank = (addr & ADDR_MASK) >> addr_shift;
	
	if(rd_table[bank].device != NULL) {
//		return rd_table[bank].device->read_memory_mapped_io8(addr);
		return 0xff;
	} else {
		return rd_table[bank].memory[addr & BANK_MASK];
	}
}

void MEMORY::write_dma_data8(uint32_t addr, uint32_t data)
{
	int bank = (addr & ADDR_MASK) >> addr_shift;
	
	if(wr_table[bank].device != NULL) {
//		wr_table[bank].device->write_memory_mapped_io8(addr, data);
	} else {
		wr_table[bank].memory[addr & BANK_MASK] = data;
	}
}

uint32_t MEMORY::read_dma_data16(uint32_t addr)
{
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
#endif

// register

void MEMORY::set_memory_r(uint32_t start, uint32_t end, uint8_t *memory)
{
	MEMORY::initialize(); // subclass may overload initialize()
	
	uint32_t start_bank = start >> addr_shift;
	uint32_t end_bank = end >> addr_shift;
	
	for(uint32_t i = start_bank; i <= end_bank; i++) {
		rd_table[i].device = NULL;
		rd_table[i].memory = memory + bank_size * (i - start_bank);
	}
}

void MEMORY::set_memory_w(uint32_t start, uint32_t end, uint8_t *memory)
{
	MEMORY::initialize();
	
	uint32_t start_bank = start >> addr_shift;
	uint32_t end_bank = end >> addr_shift;
	
	for(uint32_t i = start_bank; i <= end_bank; i++) {
		wr_table[i].device = NULL;
		wr_table[i].memory = memory + bank_size * (i - start_bank);
	}
}

void MEMORY::set_memory_mapped_io_r(uint32_t start, uint32_t end, DEVICE *device)
{
	MEMORY::initialize();
	
	uint32_t start_bank = start >> addr_shift;
	uint32_t end_bank = end >> addr_shift;
	
	for(uint32_t i = start_bank; i <= end_bank; i++) {
		rd_table[i].device = device;
	}
}

void MEMORY::set_memory_mapped_io_w(uint32_t start, uint32_t end, DEVICE *device)
{
	MEMORY::initialize();
	
	uint32_t start_bank = start >> addr_shift;
	uint32_t end_bank = end >> addr_shift;
	
	for(uint32_t i = start_bank; i <= end_bank; i++) {
		wr_table[i].device = device;
	}
}

void MEMORY::set_wait_r(uint32_t start, uint32_t end, int wait)
{
	MEMORY::initialize();
	
	uint32_t start_bank = start >> addr_shift;
	uint32_t end_bank = end >> addr_shift;
	
	for(uint32_t i = start_bank; i <= end_bank; i++) {
		rd_table[i].wait = wait;
	}
}

void MEMORY::set_wait_w(uint32_t start, uint32_t end, int wait)
{
	MEMORY::initialize();
	
	uint32_t start_bank = start >> addr_shift;
	uint32_t end_bank = end >> addr_shift;
	
	for(uint32_t i = start_bank; i <= end_bank; i++) {
		wr_table[i].wait = wait;
	}
}

void MEMORY::unset_memory_r(uint32_t start, uint32_t end)
{
	MEMORY::initialize();
	
	uint32_t start_bank = start >> addr_shift;
	uint32_t end_bank = end >> addr_shift;
	
	for(uint32_t i = start_bank; i <= end_bank; i++) {
		rd_table[i].device = NULL;
		rd_table[i].memory = rd_dummy;
	}
}

void MEMORY::unset_memory_w(uint32_t start, uint32_t end)
{
	MEMORY::initialize();
	
	uint32_t start_bank = start >> addr_shift;
	uint32_t end_bank = end >> addr_shift;
	
	for(uint32_t i = start_bank; i <= end_bank; i++) {
		wr_table[i].device = NULL;
		wr_table[i].memory = wr_dummy;
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
