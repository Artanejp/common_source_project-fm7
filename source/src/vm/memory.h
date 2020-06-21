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

//#ifndef MEMORY_ADDR_MAX
//#define MEMORY_ADDR_MAX 0x10000
//#endif
//#ifndef MEMORY_BANK_SIZE
//#define MEMORY_BANK_SIZE 0x1000
//#endif

class VM;
class EMU;
class MEMORY : public DEVICE
{
protected:
	typedef struct {
		DEVICE *device;
		uint8_t *memory;
		int wait;
	} bank_t;
	
	uint8_t *rd_dummy;
	uint8_t *wr_dummy;
	
	bool _MEMORY_DISABLE_DMA_MMIO;
	bool bank_size_was_set;
	bool addr_max_was_set;
public:
	MEMORY(VM_TEMPLATE* parent_vm, EMU_TEMPLATE* parent_emu) : DEVICE(parent_vm, parent_emu)
	{
		// Set temporally values.
		addr_max = 0x10000;
		bank_size = 0x1000;
		bank_size_was_set = false;
		addr_max_was_set = false;
 		
		rd_table = wr_table = NULL;
		rd_dummy = wr_dummy = NULL;

		_MEMORY_DISABLE_DMA_MMIO = false;
		
 		set_device_name(_T("Generic Memory Bus"));
	}
	~MEMORY() {}
	
	// common functions
	virtual void initialize();
	virtual void release();
	virtual uint32_t __FASTCALL read_data8(uint32_t addr);
	virtual void __FASTCALL write_data8(uint32_t addr, uint32_t data);
	virtual uint32_t __FASTCALL read_data16(uint32_t addr);
	virtual void __FASTCALL write_data16(uint32_t addr, uint32_t data);
	virtual uint32_t __FASTCALL read_data32(uint32_t addr);
	virtual void __FASTCALL write_data32(uint32_t addr, uint32_t data);
	virtual uint32_t __FASTCALL read_data8w(uint32_t addr, int* wait);
	virtual void __FASTCALL write_data8w(uint32_t addr, uint32_t data, int* wait);
	virtual uint32_t __FASTCALL read_data16w(uint32_t addr, int* wait);
	virtual void __FASTCALL write_data16w(uint32_t addr, uint32_t data, int* wait);
	virtual uint32_t __FASTCALL read_data32w(uint32_t addr, int* wait);
	virtual void __FASTCALL write_data32w(uint32_t addr, uint32_t data, int* wait);

	virtual uint32_t __FASTCALL read_dma_data8(uint32_t addr);
	virtual void __FASTCALL write_dma_data8(uint32_t addr, uint32_t data);
	virtual uint32_t __FASTCALL read_dma_data16(uint32_t addr);
	virtual void __FASTCALL write_dma_data16(uint32_t addr, uint32_t data);
	virtual uint32_t __FASTCALL read_dma_data32(uint32_t addr);
	virtual void __FASTCALL write_dma_data32(uint32_t addr, uint32_t data);
	
	// unique functions
	virtual void set_memory_r(uint32_t start, uint32_t end, uint8_t *memory);
	virtual void set_memory_w(uint32_t start, uint32_t end, uint8_t *memory);
	void set_memory_rw(uint32_t start, uint32_t end, uint8_t *memory)
	{
		set_memory_r(start, end, memory);
		set_memory_w(start, end, memory);
	}
	virtual void set_memory_mapped_io_r(uint32_t start, uint32_t end, DEVICE *device);
	virtual void set_memory_mapped_io_w(uint32_t start, uint32_t end, DEVICE *device);
	void set_memory_mapped_io_rw(uint32_t start, uint32_t end, DEVICE *device)
	{
		set_memory_mapped_io_r(start, end, device);
		set_memory_mapped_io_w(start, end, device);
	}
	virtual void set_wait_r(uint32_t start, uint32_t end, int wait);
	virtual void set_wait_w(uint32_t start, uint32_t end, int wait);
	void set_wait_rw(uint32_t start, uint32_t end, int wait)
	{
		set_wait_r(start, end, wait);
		set_wait_w(start, end, wait);
	}
	virtual void unset_memory_r(uint32_t start, uint32_t end);
	virtual void unset_memory_w(uint32_t start, uint32_t end);
	void unset_memory_rw(uint32_t start, uint32_t end)
	{
		unset_memory_r(start, end);
		unset_memory_w(start, end);
	}
	virtual void copy_table_r(uint32_t to, uint32_t start, uint32_t end);
	virtual void copy_table_w(uint32_t to, uint32_t start, uint32_t end);
	void copy_table_rw(uint32_t to, uint32_t start, uint32_t end) {
		copy_table_r(to, start, end);
		copy_table_w(to, start, end);
	}
	int read_bios(const _TCHAR *file_name, uint8_t *buffer, int size);
	bool write_bios(const _TCHAR *file_name, uint8_t *buffer, int size);
	bool read_image(const _TCHAR *file_path, uint8_t *buffer, int size);
	bool write_image(const _TCHAR *file_path, uint8_t *buffer, int size);

	// Unique functions.
	void set_addr_max(int64_t size)
	{
		// Allow to modify before initialize() or set_foo_r|w|rw()..
		if(rd_table == NULL) {
			addr_max_was_set = true;
			addr_max = size;
		}
	}
	void set_bank_size(int64_t size)
	{
		if(rd_table == NULL) {
			bank_size_was_set = true;
			bank_size = size;
		}
	}
	uint64_t get_addr_max()
	{
		return addr_max;
	}
	uint64_t get_bank_size()
	{
		return bank_size;
	}
	
	uint64_t addr_max;
	uint64_t bank_size;

	uint64_t addr_mask;
	uint64_t bank_mask;

	bank_t *rd_table;
	bank_t *wr_table;
	
	int addr_shift;
};

#endif

