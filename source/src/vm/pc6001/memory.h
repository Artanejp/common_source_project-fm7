/*
	NEC PC-6001 Emulator 'yaPC-6001'
	NEC PC-6001mkII Emulator 'yaPC-6201'
	NEC PC-6001mkIISR Emulator 'yaPC-6401'
	NEC PC-6601 Emulator 'yaPC-6601'
	NEC PC-6601SR Emulator 'yaPC-6801'

	Author : tanam
	Date   : 2013.07.15-

	[ memory ]
*/

#ifndef _MEMORY_H_
#define _MEMORY_H_

#include "../vm.h"
#include "../../emu.h"
#include "../device.h"

#define SIG_MEMORY_PIO_PORT_C	0

// memory offset
#define RAM_BASE		0
#define RAM_SIZE		0x10000
#define BASICROM_BASE	(RAM_BASE + RAM_SIZE)
#define BASICROM_SIZE	0x8000
#define EXTROM_BASE		(BASICROM_BASE + BASICROM_SIZE)
#define EXTROM_SIZE		0x4000
#define CGROM1_BASE		(EXTROM_BASE + EXTROM_SIZE)
#define CGROM1_SIZE		0x4000
#define EmptyRAM_BASE	(CGROM1_BASE + CGROM1_SIZE)
#define EmptyRAM_SIZE	0x2000

#define MEMORY_SIZE		(EmptyRAM_BASE + EmptyRAM_SIZE)

class MEMORY : public DEVICE
{
private:
	DEVICE *d_cpu;
	uint8_t MEMORY_BASE[MEMORY_SIZE];
	uint8_t *CGROM;
	uint8_t *EXTROM1;
	uint8_t *EXTROM2;
	uint8_t *VRAM;
	uint8_t CGSW93;
	uint8_t CRTKILL;
	bool inserted;
	
public:
	MEMORY(VM_TEMPLATE* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu)
	{
		inserted = false;
		set_device_name(_T("Memory Bus"));
	}
	~MEMORY() {}
	
	// common functions
	void initialize();
	void reset();
	void write_data8(uint32_t addr, uint32_t data);
	uint32_t read_data8(uint32_t addr);
	void write_data8w(uint32_t addr, uint32_t data, int *wait);
	uint32_t read_data8w(uint32_t addr, int *wait);
	uint32_t fetch_op(uint32_t addr, int *wait);
	void write_io8(uint32_t addr, uint32_t data);
	void write_io8w(uint32_t addr, uint32_t data, int* wait);
	uint32_t read_io8w(uint32_t addr, int* wait);
	void event_vline(int v, int clock);
	void event_callback(int event_id, int err);
	void write_signal(int id, uint32_t data, uint32_t mask);
	bool process_state(FILEIO* state_fio, bool loading);
	
	// unique functions
	void set_context_cpu(DEVICE* device)
	{
		d_cpu = device;
	}
	void open_cart(const _TCHAR* file_path);
	void close_cart();
	bool is_cart_inserted()
	{
		return inserted;
	}
	uint8_t* get_vram()
	{
		return MEMORY_BASE + RAM_BASE;
	}
};
#endif
