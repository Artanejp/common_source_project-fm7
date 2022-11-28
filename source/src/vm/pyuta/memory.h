/*
	TOMY PyuTa Emulator 'ePyuTa'

	Author : Takeda.Toshiya
	Date   : 2007.07.15 -

	[ memory ]
*/

#ifndef _MEMORY_H_
#define _MEMORY_H_

#include "../vm.h"
#include "../../emu.h"
#include "../device.h"

class MEMORY : public DEVICE
{
private:
	DEVICE *d_cmt, *d_cpu, *d_psg, *d_vdp;
	
	uint8_t ipl[0x8000];	// ipl rom (32k)
	uint8_t basic[0x4000];	// basic rom (16k)
	uint8_t cart[0x8000];	// cartridge (32k)
	
	uint8_t wdmy[0x1000];
	uint8_t rdmy[0x1000];
	uint8_t* wbank[16];
	uint8_t* rbank[16];
	
	bool cmt_signal, cmt_remote;
	bool has_extrom, cart_enabled;
	int ctype;
	
	const uint8_t *key;
	const uint32_t *joy;
	
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
	void write_io8(uint32_t addr, uint32_t data);
	uint32_t read_io8(uint32_t addr);
	void write_signal(int id, uint32_t data, uint32_t mask);
	bool process_state(FILEIO* state_fio, bool loading);
	
	// unique functions
	void set_context_cmt(DEVICE* device)
	{
		d_cmt = device;
	}
	void set_context_cpu(DEVICE* device)
	{
		d_cpu = device;
	}
	void set_context_psg(DEVICE* device)
	{
		d_psg = device;
	}
	void set_context_vdp(DEVICE* device)
	{
		d_vdp = device;
	}
	void open_cart(const _TCHAR* file_path);
	void close_cart();
	bool is_cart_inserted()
	{
		return (ctype != 0);
	}
};

#endif

