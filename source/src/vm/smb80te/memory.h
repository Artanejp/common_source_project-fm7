/*
	SHARP SM-B-80TE Emulator 'eSM-B-80TE'

	Author : Takeda.Toshiya
	Date   : 2016.12.29-

	[ memory ]
*/

#ifndef _MEMORY_H_
#define _MEMORY_H_

#include "../vm.h"
#include "../../emu.h"
#include "../device.h"

#define SIG_MEMORY_PIO1_PA	0
#define SIG_MEMORY_PIO1_PB	1

class MEMORY : public DEVICE
{
private:
	// contexts
	DEVICE *d_cpu, *d_drec, *d_pio1;
	
	// memory
	uint8_t ram[0x1000];
	uint8_t mon[0x800];
	uint8_t user[0x800];
	
	uint8_t wdmy[0x800];
	uint8_t rdmy[0x800];
	uint8_t* wbank[32];
	uint8_t* rbank[32];
	
	uint8_t pio1_pa, pio1_pb;
	uint8_t shift_reg;
	uint32_t a15_mask;
	bool led;
	int seg[8][8];
	
	void update_kb();
	
public:
	MEMORY(VM_TEMPLATE* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu)
	{
		set_device_name(_T("Memory Bus"));
	}
	~MEMORY() {}
	
	// common functions
	void initialize();
	void reset();
	void write_data8(uint32_t addr, uint32_t data);
	uint32_t read_data8(uint32_t addr);
	uint32_t fetch_op(uint32_t addr, int *wait);
	void write_io8(uint32_t addr, uint32_t data);
	uint32_t read_io8(uint32_t addr);
	void write_signal(int id, uint32_t data, uint32_t mask);
	void event_vline(int v, int clock);
	bool process_state(FILEIO* state_fio, bool loading);
	
	// unique functions
	void set_context_cpu(DEVICE* device)
	{
		d_cpu = device;
	}
	void set_context_drec(DEVICE* device)
	{
		d_drec = device;
	}
	void set_context_pio1(DEVICE* device)
	{
		d_pio1 = device;
	}
	void draw_screen();
	void load_ram(const _TCHAR* file_path);
	void save_ram(const _TCHAR* file_path);
};

#endif

