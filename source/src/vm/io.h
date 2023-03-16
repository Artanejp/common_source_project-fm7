/*
	Skelton for retropc emulator

	Author : Takeda.Toshiya
	Date   : 2008.12.29 -

	[ i/o bus ]
*/

#ifndef _IO_H_
#define _IO_H_

#include "device.h"

//#ifndef IO_ADDR_MAX
//#define IO_ADDR_MAX 0x100
//#endif

class  DLL_PREFIX IO : public DEVICE
{
private:
	// i/o map
	typedef struct {
		DEVICE* dev;
		uint32_t addr;
		int wait;
		bool wait_registered;
		bool is_flipflop;
	} wr_bank_t;

	typedef struct {
		DEVICE* dev;
		uint32_t addr;
		int wait;
		bool wait_registered;
		uint32_t value;
		bool value_registered;
	} rd_bank_t;

	wr_bank_t *wr_table;
	rd_bank_t *rd_table;

	bool __IO_DEBUG_LOG;
	bool __IO_DEBUG_LOG_DEFAULT; // ToDo: Available to change by UI.

	// ToDo: Make is_dma to constexpr(bool) .
	//       Maybe will make inline below functions. - 20230316 K.O
	void __FASTCALL write_port8(uint32_t addr, uint32_t data, bool is_dma, int *wait);
	uint32_t __FASTCALL read_port8(uint32_t addr, bool is_dma, int *wait);
	void __FASTCALL write_port16(uint32_t addr, uint32_t data, bool is_dma, int *wait);
	uint32_t __FASTCALL read_port16(uint32_t addr, bool is_dma, int *wait);
	void __FASTCALL write_port32(uint32_t addr, uint32_t data, bool is_dma, int *wait);
	uint32_t __FASTCALL read_port32(uint32_t addr, bool is_dma, int *wait);

public:
	IO(VM_TEMPLATE* parent_vm, EMU_TEMPLATE* parent_emu) : DEVICE(parent_vm, parent_emu)
	{
		__IO_DEBUG_LOG = false;
		__IO_DEBUG_LOG_DEFAULT = __IO_DEBUG_LOG;
//#ifdef _IO_DEBUG_LOG
		cpu_index = 0;
//#endif
		space = 0x100;
		bus_width = 8;

		wr_table = NULL;
		rd_table = NULL;

		set_device_name(_T("Generic I/O Bus"));
	}
	~IO() {}

	// common functions
	void initialize() override;
	void release() override;

	void __FASTCALL write_io8(uint32_t addr, uint32_t data) override;
	uint32_t __FASTCALL read_io8(uint32_t addr) override;
	void __FASTCALL write_io16(uint32_t addr, uint32_t data) override;
	uint32_t __FASTCALL read_io16(uint32_t addr) override;
	void __FASTCALL write_io32(uint32_t addr, uint32_t data) override;
	uint32_t __FASTCALL read_io32(uint32_t addr) override;
	void __FASTCALL write_io8w(uint32_t addr, uint32_t data, int* wait) override;
	uint32_t __FASTCALL read_io8w(uint32_t addr, int* wait) override;
	void __FASTCALL write_io16w(uint32_t addr, uint32_t data, int* wait) override;
	uint32_t __FASTCALL read_io16w(uint32_t addr, int* wait) override;
	void __FASTCALL write_io32w(uint32_t addr, uint32_t data, int* wait) override;
	uint32_t __FASTCALL read_io32w(uint32_t addr, int* wait) override;
	void __FASTCALL write_dma_io8(uint32_t addr, uint32_t data) override;
	uint32_t __FASTCALL read_dma_io8(uint32_t addr) override;
	void __FASTCALL write_dma_io16(uint32_t addr, uint32_t data) override;
	uint32_t __FASTCALL read_dma_io16(uint32_t addr) override;
	void __FASTCALL write_dma_io32(uint32_t addr, uint32_t data) override;
	uint32_t __FASTCALL read_dma_io32(uint32_t addr) override;

	bool process_state(FILEIO* state_fio, bool loading) override;

	// unique functions
	void set_iomap_single_r(uint32_t addr, DEVICE* device);
	void set_iomap_single_w(uint32_t addr, DEVICE* device);
	void set_iomap_single_rw(uint32_t addr, DEVICE* device);
	void set_iomap_alias_r(uint32_t addr, DEVICE* device, uint32_t alias);
	void set_iomap_alias_w(uint32_t addr, DEVICE* device, uint32_t alias);
	void set_iomap_alias_rw(uint32_t addr, DEVICE* device, uint32_t alias);

	void set_iomap_range_r(uint32_t s, uint32_t e, DEVICE* device);
	void set_iomap_range_w(uint32_t s, uint32_t e, DEVICE* device);
	void set_iomap_range_rw(uint32_t s, uint32_t e, DEVICE* device);

	void set_iovalue_single_r(uint32_t addr, uint32_t value);
	void set_iovalue_range_r(uint32_t s, uint32_t e, uint32_t value);
	void set_flipflop_single_rw(uint32_t addr, uint32_t value);
	void set_flipflop_range_rw(uint32_t s, uint32_t e, uint32_t value);

	void set_iowait_single_r(uint32_t addr, int wait);
	void set_iowait_single_w(uint32_t addr, int wait);
	void set_iowait_single_rw(uint32_t addr, int wait);
	void set_iowait_range_r(uint32_t s, uint32_t e, int wait);
	void set_iowait_range_w(uint32_t s, uint32_t e, int wait);
	void set_iowait_range_rw(uint32_t s, uint32_t e, int wait);

//#ifdef _IO_DEBUG_LOG
	int cpu_index;
//#endif
	uint32_t space;
	int bus_width;
};


#endif
