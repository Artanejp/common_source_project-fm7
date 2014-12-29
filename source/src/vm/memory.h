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
		uint8* memory;
	} bank_t;
	
	bank_t *read_table;
	bank_t *write_table;
	
	int addr_shift;
	
	uint8 read_dummy[MEMORY_BANK_SIZE];
	uint8 write_dummy[MEMORY_BANK_SIZE];
	
public:
	MEMORY(VM* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu)
	{
		int bank_num = MEMORY_ADDR_MAX / MEMORY_BANK_SIZE;
		
		read_table = (bank_t *)malloc(sizeof(bank_t) * bank_num);
		write_table = (bank_t *)malloc(sizeof(bank_t) * bank_num);
		
		for(int i = 0; i < bank_num; i++) {
			read_table[i].dev = NULL;
			read_table[i].memory = read_dummy;
			
			write_table[i].dev = NULL;
			write_table[i].memory = write_dummy;
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
	uint32 read_data8(uint32 addr);
	void write_data8(uint32 addr, uint32 data);
	uint32 read_data16(uint32 addr);
	void write_data16(uint32 addr, uint32 data);
	uint32 read_data32(uint32 addr);
	void write_data32(uint32 addr, uint32 data);
	
	// unique functions
	void set_memory_r(uint32 start, uint32 end, uint8 *memory);
	void set_memory_w(uint32 start, uint32 end, uint8 *memory);
	void set_memory_rw(uint32 start, uint32 end, uint8 *memory)
	{
		set_memory_r(start, end, memory);
		set_memory_w(start, end, memory);
	}
	void set_memory_mapped_io_r(uint32 start, uint32 end, DEVICE *device);
	void set_memory_mapped_io_w(uint32 start, uint32 end, DEVICE *device);
	void set_memory_mapped_io_rw(uint32 start, uint32 end, DEVICE *device)
	{
		set_memory_mapped_io_r(start, end, device);
		set_memory_mapped_io_w(start, end, device);
	}
	void unset_memory_r(uint32 start, uint32 end);
	void unset_memory_w(uint32 start, uint32 end);
	void unset_memory_rw(uint32 start, uint32 end)
	{
		unset_memory_r(start, end);
		unset_memory_w(start, end);
	}
	int read_bios(_TCHAR *file_name, uint8 *buffer, int size);
	bool write_bios(_TCHAR *file_name, uint8 *buffer, int size);
	bool read_image(_TCHAR *file_path, uint8 *buffer, int size);
	bool write_image(_TCHAR *file_path, uint8 *buffer, int size);
};

#endif

