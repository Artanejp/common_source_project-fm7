/*
	Skelton for retropc emulator

	Author : Takeda.Toshiya
	Date   : 2015.01.30-

	[ EPSON TF-20 ]
*/

#ifndef _TF20_H_
#define _TF20_H_

#include "vm.h"
#include "../emu.h"
#include "device.h"

class TF20 : public DEVICE
{
private:
	DEVICE *d_cpu, *d_fdc, *d_pio, *d_sio;
	
	uint8_t rom[0x800];
	uint8_t ram[0x10000];
	bool rom_selected;
	
	uint8_t wdmy[0x800];
	uint8_t rdmy[0x800];
	uint8_t* wbank[32];
	uint8_t* rbank[32];
	
public:
	TF20(VM_TEMPLATE* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu)
	{
		drive_no = 0;
		set_device_name(_T("TF-20 FDD"));
	}
	~TF20() {}
	
	// common functions
	void initialize();
	void reset();
	uint32_t read_data8(uint32_t addr);
	void write_data8(uint32_t addr, uint32_t data);
	uint32_t read_io8(uint32_t addr);
	void write_io8(uint32_t addr, uint32_t data);
	uint32_t get_intr_ack();
	bool process_state(FILEIO* state_fio, bool loading);
	
	// unique functions
	void set_context_cpu(DEVICE* device)
	{
		d_cpu = device;
	}
	void set_context_fdc(DEVICE* device)
	{
		d_fdc = device;
	}
	void set_context_pio(DEVICE* device)
	{
		d_pio = device;
	}
	void set_context_sio(DEVICE* device)
	{
		d_sio = device;
	}
	int drive_no;
};

#endif

