/*
	Skelton for retropc emulator

	Author : Takeda.Toshiya
	Date   : 2010.09.16-

	[ memory ]
*/

#include "memory.h"
#include "../fileio.h"

#define ADDR_MASK (MEMORY_ADDR_MAX - 1)
#define BANK_MASK (MEMORY_BANK_SIZE - 1)

void MEMORY::release()
{
	free(read_table);
	free(write_table);
}

uint32 MEMORY::read_data8(uint32 addr)
{
	int bank = (addr & ADDR_MASK) >> addr_shift;
	
	if(read_table[bank].dev != NULL) {
		return read_table[bank].dev->read_memory_mapped_io8(addr);
	} else {
		return read_table[bank].memory[addr & BANK_MASK];
	}
}

void MEMORY::write_data8(uint32 addr, uint32 data)
{
	int bank = (addr & ADDR_MASK) >> addr_shift;
	
	if(write_table[bank].dev != NULL) {
		write_table[bank].dev->write_memory_mapped_io8(addr, data);
	} else {
		write_table[bank].memory[addr & BANK_MASK] = data;
	}
}

uint32 MEMORY::read_data16(uint32 addr)
{
	int bank = (addr & ADDR_MASK) >> addr_shift;
	
	if(read_table[bank].dev != NULL) {
		return read_table[bank].dev->read_memory_mapped_io16(addr);
	} else {
		uint32 val = read_data8(addr);
		val |= read_data8(addr + 1) << 8;
		return val;
	}
}

void MEMORY::write_data16(uint32 addr, uint32 data)
{
	int bank = (addr & ADDR_MASK) >> addr_shift;
	
	if(write_table[bank].dev != NULL) {
		write_table[bank].dev->write_memory_mapped_io16(addr, data);
	} else {
		write_data8(addr, data & 0xff);
		write_data8(addr + 1, (data >> 8) & 0xff);
	}
}

uint32 MEMORY::read_data32(uint32 addr)
{
	int bank = (addr & ADDR_MASK) >> addr_shift;
	
	if(read_table[bank].dev != NULL) {
		return read_table[bank].dev->read_memory_mapped_io32(addr);
	} else {
		uint32 val = read_data16(addr);
		val |= read_data16(addr + 2) << 16;
		return val;
	}
}

void MEMORY::write_data32(uint32 addr, uint32 data)
{
	int bank = (addr & ADDR_MASK) >> addr_shift;
	
	if(write_table[bank].dev != NULL) {
		write_table[bank].dev->write_memory_mapped_io32(addr, data);
	} else {
		write_data16(addr, data & 0xffff);
		write_data16(addr + 2, (data >> 16) & 0xffff);
	}
}

// register

void MEMORY::set_memory_r(uint32 start, uint32 end, uint8 *memory)
{
	uint32 start_bank = start >> addr_shift;
	uint32 end_bank = end >> addr_shift;
	
	for(uint32 i = start_bank; i <= end_bank; i++) {
		read_table[i].dev = NULL;
		read_table[i].memory = memory + MEMORY_BANK_SIZE * (i - start_bank);
	}
}

void MEMORY::set_memory_w(uint32 start, uint32 end, uint8 *memory)
{
	uint32 start_bank = start >> addr_shift;
	uint32 end_bank = end >> addr_shift;
	
	for(uint32 i = start_bank; i <= end_bank; i++) {
		write_table[i].dev = NULL;
		write_table[i].memory = memory + MEMORY_BANK_SIZE * (i - start_bank);
	}
}

void MEMORY::set_memory_mapped_io_r(uint32 start, uint32 end, DEVICE *device)
{
	uint32 start_bank = start >> addr_shift;
	uint32 end_bank = end >> addr_shift;
	
	for(uint32 i = start_bank; i <= end_bank; i++) {
		read_table[i].dev = device;
	}
}

void MEMORY::set_memory_mapped_io_w(uint32 start, uint32 end, DEVICE *device)
{
	uint32 start_bank = start >> addr_shift;
	uint32 end_bank = end >> addr_shift;
	
	for(uint32 i = start_bank; i <= end_bank; i++) {
		write_table[i].dev = device;
	}
}

void MEMORY::unset_memory_r(uint32 start, uint32 end)
{
	uint32 start_bank = start >> addr_shift;
	uint32 end_bank = end >> addr_shift;
	
	for(uint32 i = start_bank; i <= end_bank; i++) {
		read_table[i].dev = NULL;
		read_table[i].memory = read_dummy;
	}
}

void MEMORY::unset_memory_w(uint32 start, uint32 end)
{
	uint32 start_bank = start >> addr_shift;
	uint32 end_bank = end >> addr_shift;
	
	for(uint32 i = start_bank; i <= end_bank; i++) {
		write_table[i].dev = NULL;
		write_table[i].memory = write_dummy;
	}
}

// load/save image

int MEMORY::read_bios(_TCHAR *file_name, uint8 *buffer, int size)
{
	FILEIO* fio = new FILEIO();
	int length = 0;
	
	if(fio->Fopen(emu->bios_path(file_name), FILEIO_READ_BINARY)) {
		fio->Fread(buffer, size, 1);
		length = fio->Ftell();
		fio->Fclose();
	}
	delete fio;
	return length;
}

bool MEMORY::write_bios(_TCHAR *file_name, uint8 *buffer, int size)
{
	FILEIO* fio = new FILEIO();
	bool result = false;
	
	if(fio->Fopen(emu->bios_path(file_name), FILEIO_WRITE_BINARY)) {
		fio->Fwrite(buffer, size, 1);
		fio->Fclose();
		result = true;
	}
	delete fio;
	return result;
}

bool MEMORY::read_image(_TCHAR *file_path, uint8 *buffer, int size)
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

bool MEMORY::write_image(_TCHAR* file_path, uint8* buffer, int size)
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
