/*
	Skelton for retropc emulator

	Author : Takeda.Toshiya
	Date   : 2010.09.16-

	[ memory ]
*/

#ifndef _MEMORY_H_
#define _MEMORY_H_

#include "vm.h"
#include "../emu.h"
#include "device.h"

#ifndef MEMORY_ADDR_MAX
#define MEMORY_ADDR_MAX 0x10000
#endif
#ifndef MEMORY_BANK_SIZE
#define MEMORY_BANK_SIZE 0x1000
#endif

class MEMORY : public DEVICE
{
private:
	typedef struct {
		DEVICE* dev;
		uint8_t* memory;
		int wait;
	} bank_t;
	
	bank_t *read_table;
	bank_t *write_table;
	
	int addr_shift;
	
	uint8_t read_dummy[MEMORY_BANK_SIZE];
	uint8_t write_dummy[MEMORY_BANK_SIZE];
	
public:
	MEMORY(VM* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu)
	{
		int bank_num = MEMORY_ADDR_MAX / MEMORY_BANK_SIZE;
		
		read_table = (bank_t *)malloc(sizeof(bank_t) * bank_num);
		write_table = (bank_t *)malloc(sizeof(bank_t) * bank_num);
		
		for(int i = 0; i < bank_num; i++) {
			read_table[i].dev = NULL;
			read_table[i].memory = read_dummy;
			read_table[i].wait = 0;
			
			write_table[i].dev = NULL;
			write_table[i].memory = write_dummy;
			read_table[i].wait = 0;
		}
		for(int i = 0;; i++) {
			if(MEMORY_BANK_SIZE == (1 << i)) {
				addr_shift = i;
				break;
			}
		}
		memset(read_dummy, 0xff, MEMORY_BANK_SIZE);
	}
	~MEMORY() {}
	
	// common functions
	void release();
	uint32_t read_data8(uint32_t addr);
	void write_data8(uint32_t addr, uint32_t data);
	uint32_t read_data16(uint32_t addr);
	void write_data16(uint32_t addr, uint32_t data);
	uint32_t read_data32(uint32_t addr);
	void write_data32(uint32_t addr, uint32_t data);
	uint32_t read_data8w(uint32_t addr, int* wait);
	void write_data8w(uint32_t addr, uint32_t data, int* wait);
	uint32_t read_data16w(uint32_t addr, int* wait);
	void write_data16w(uint32_t addr, uint32_t data, int* wait);
	uint32_t read_data32w(uint32_t addr, int* wait);
	void write_data32w(uint32_t addr, uint32_t data, int* wait);
	const _TCHAR *get_device_name()
	{
		return _T("Standard Memory Bus");
	}
	
	// unique functions
	void set_memory_r(uint32_t start, uint32_t end, uint8_t *memory);
	void set_memory_w(uint32_t start, uint32_t end, uint8_t *memory);
	void set_memory_rw(uint32_t start, uint32_t end, uint8_t *memory)
	{
		set_memory_r(start, end, memory);
		set_memory_w(start, end, memory);
	}
	void set_memory_mapped_io_r(uint32_t start, uint32_t end, DEVICE *device);
	void set_memory_mapped_io_w(uint32_t start, uint32_t end, DEVICE *device);
	void set_memory_mapped_io_rw(uint32_t start, uint32_t end, DEVICE *device)
	{
		set_memory_mapped_io_r(start, end, device);
		set_memory_mapped_io_w(start, end, device);
	}
	void set_wait_r(uint32_t start, uint32_t end, int wait);
	void set_wait_w(uint32_t start, uint32_t end, int wait);
	void set_wait_rw(uint32_t start, uint32_t end, int wait)
	{
		set_wait_r(start, end, wait);
		set_wait_w(start, end, wait);
	}
	void unset_memory_r(uint32_t start, uint32_t end);
	void unset_memory_w(uint32_t start, uint32_t end);
	void unset_memory_rw(uint32_t start, uint32_t end)
	{
		unset_memory_r(start, end);
		unset_memory_w(start, end);
	}
	int read_bios(const _TCHAR *file_name, uint8_t *buffer, int size);
	bool write_bios(const _TCHAR *file_name, uint8_t *buffer, int size);
	bool read_image(const _TCHAR *file_path, uint8_t *buffer, int size);
	bool write_image(const _TCHAR *file_path, uint8_t *buffer, int size);
};

#endif

