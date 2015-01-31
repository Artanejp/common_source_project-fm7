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
	
	uint8 rom[0x800];
	uint8 ram[0x10000];
	bool rom_selected;
	
	uint8 wdmy[0x800];
	uint8 rdmy[0x800];
	uint8* wbank[32];
	uint8* rbank[32];
	
public:
	TF20(VM* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu)
	{
		drive_no = 0;
	}
	~TF20() {}
	
	// common functions
	void initialize();
	void reset();
	uint32 read_data8(uint32 addr);
	void write_data8(uint32 addr, uint32 data);
	uint32 read_io8(uint32 addr);
	void write_io8(uint32 addr, uint32 data);
	uint32 intr_ack();
	void save_state(FILEIO* state_fio);
	bool load_state(FILEIO* state_fio);
	
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

