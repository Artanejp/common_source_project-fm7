/*
	GAKKEN TV BOY Emulator 'yaTVBOY'

	Author : tanam
	Date   : 2020.06.13

	[ memory ]
*/

#ifndef _MEMORY_H_
#define _MEMORY_H_

#include "../vm.h"
#include "../../emu.h"
#include "../device.h"
#include "../mc6800.h"
#include "../mc6847.h"

#define SIG_MEMORY_PORT_1	0

class MEMORY : public DEVICE
{
private:
	MC6800 *d_cpu;
	MC6847 *d_vdp;

	uint8_t rom[0x1000];
	uint8_t ram[0x1000];
	uint8_t vram[0x1000];
	
	uint8_t wdmy[0x400];
	uint8_t rdmy[0x400];
	uint8_t* wbank[64];
	uint8_t* rbank[64];

	int shot1;
	int shot2;
	int up;
	int down;
	int left;
	int right;
	bool event;
	bool inserted;
	
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
	void write_signal(int id, uint32_t data, uint32_t mask);
	void event_callback(int event_id, int err);
	bool process_state(FILEIO* state_fio, bool loading);
	// unique functions
	void key_down(int code);
	void key_up(int code);
	void set_context_cpu(MC6800* device)
	{
		d_cpu = device;
	}
	void set_context_vdp(MC6847* device)
	{
		d_vdp = device;
	}
	void open_cart(const _TCHAR* file_path);
	void close_cart();
	bool is_cart_inserted()
	{
		return inserted;
	}
	uint8_t* get_vram()
	{
		return vram;
	}
};

#endif
