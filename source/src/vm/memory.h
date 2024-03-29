/*
	Skelton for retropc emulator

	Author : Takeda.Toshiya
	Date   : 2010.09.16-

	[ memory ]
*/

#ifndef _MEMORY_H_
#define _MEMORY_H_

#include "vm_template.h"
#include "../emu_template.h"
#include "device.h"

class  DLL_PREFIX MEMORY : public DEVICE
{
protected:
	typedef struct {
		DEVICE *device;
		uint8_t *memory;
		bool wait_registered;
		int wait;
	} bank_t;

	uint8_t *rd_dummy;
	uint8_t *wr_dummy;

	int addr_shift;
public:
	MEMORY(VM_TEMPLATE* parent_vm, EMU_TEMPLATE* parent_emu) : DEVICE(parent_vm, parent_emu)
	{
		space = 0x10000;
		bank_size = 0x1000;

		rd_table = wr_table = NULL;
		rd_dummy = wr_dummy = NULL;

		bus_width = 8;
		addr_shift = 0;

		set_device_name(_T("Generic Memory Bus"));
	}
	~MEMORY() {}

	// common functions
	virtual void initialize() override;
	virtual void release() override;

	virtual uint32_t __FASTCALL read_data8(uint32_t addr) override;
	virtual void __FASTCALL write_data8(uint32_t addr, uint32_t data) override;
	virtual uint32_t __FASTCALL read_data16(uint32_t addr) override;
	virtual void __FASTCALL write_data16(uint32_t addr, uint32_t data) override;
	virtual uint32_t __FASTCALL read_data32(uint32_t addr) override;
	virtual void __FASTCALL write_data32(uint32_t addr, uint32_t data) override;
	virtual uint32_t __FASTCALL read_data8w(uint32_t addr, int* wait) override;
	virtual void __FASTCALL write_data8w(uint32_t addr, uint32_t data, int* wait) override;
	virtual uint32_t __FASTCALL read_data16w(uint32_t addr, int* wait) override;
	virtual void __FASTCALL write_data16w(uint32_t addr, uint32_t data, int* wait) override;
	virtual uint32_t __FASTCALL read_data32w(uint32_t addr, int* wait) override;
	virtual void __FASTCALL write_data32w(uint32_t addr, uint32_t data, int* wait) override;

	// unique functions
	inline int get_bank(uint32_t addr) const
	{
		const uint64_t _mask = space - 1;

		return (uint32_t)(((uint64_t)addr & _mask) >> addr_shift);
	}
	inline int bus_access_times_16(uint32_t addr) const
	{
		if(bus_width >= 32) {
			return ((addr & 3) == 3 ? 2 : 1);
		} else if(bus_width >= 16) {
			return ((addr & 1) == 1 ? 2 : 1);
		} else {
			return 2;
		}
	}
	inline int bus_access_times_32(uint32_t addr) const
	{
		if(bus_width >= 32) {
			return ((addr & 3) != 0 ? 2 : 1);
		} else if(bus_width >= 16) {
			return ((addr & 1) == 1 ? 3 : 2);
		} else {
			return 4;
		}
	}
	virtual void set_memory_r(uint32_t start, uint32_t end, uint8_t *memory);
	virtual void set_memory_w(uint32_t start, uint32_t end, uint8_t *memory);
	inline void set_memory_rw(uint32_t start, uint32_t end, uint8_t *memory)
	{
		set_memory_r(start, end, memory);
		set_memory_w(start, end, memory);
	}
	virtual void set_memory_mapped_io_r(uint32_t start, uint32_t end, DEVICE *device);
	virtual void set_memory_mapped_io_w(uint32_t start, uint32_t end, DEVICE *device);
	inline void set_memory_mapped_io_rw(uint32_t start, uint32_t end, DEVICE *device)
	{
		set_memory_mapped_io_r(start, end, device);
		set_memory_mapped_io_w(start, end, device);
	}
	virtual void set_wait_r(uint32_t start, uint32_t end, int wait);
	virtual void set_wait_w(uint32_t start, uint32_t end, int wait);
	inline void set_wait_rw(uint32_t start, uint32_t end, int wait)
	{
		set_wait_r(start, end, wait);
		set_wait_w(start, end, wait);
	}
	virtual void unset_memory_r(uint32_t start, uint32_t end);
	virtual void unset_memory_w(uint32_t start, uint32_t end);
	inline void unset_memory_rw(uint32_t start, uint32_t end)
	{
		unset_memory_r(start, end);
		unset_memory_w(start, end);
	}
	virtual void copy_table_r(uint32_t to, uint32_t start, uint32_t end);
	virtual void copy_table_w(uint32_t to, uint32_t start, uint32_t end);
	inline void copy_table_rw(uint32_t to, uint32_t start, uint32_t end)
	{
		copy_table_r(to, start, end);
		copy_table_w(to, start, end);
	}
	virtual void unset_wait_r(uint32_t start, uint32_t end);
	virtual void unset_wait_w(uint32_t start, uint32_t end);
	inline void unset_wait_rw(uint32_t start, uint32_t end)
	{
		unset_wait_r(start, end);
		unset_wait_w(start, end);
	}
	int read_bios(const _TCHAR *file_name, uint8_t *buffer, int size);
	bool write_bios(const _TCHAR *file_name, uint8_t *buffer, int size);
	bool read_image(const _TCHAR *file_path, uint8_t *buffer, int size);
	bool write_image(const _TCHAR *file_path, uint8_t *buffer, int size);

	bank_t *rd_table;
	bank_t *wr_table;

	uint64_t space;
	uint32_t bank_size;
	int bus_width;
};

#endif
