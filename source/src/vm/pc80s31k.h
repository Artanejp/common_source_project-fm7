/*
	Skelton for retropc emulator

	Author : Takeda.Toshiya
	Date   : 2011.12.18-

	[ PC-80S31K ]
*/

#ifndef _PC80S31K_H_
#define _PC80S31K_H_

#include "vm.h"
#include "../emu.h"
#include "device.h"

class UPD765A;

class PC80S31K : public DEVICE
{
private:
	UPD765A *d_fdc;
	DEVICE *d_cpu, *d_pio;
	
	uint8 rom[0x2000];	// PC-8801M*
	uint8 ram[0x4000];
	
	uint8 wdmy[0x2000];
	uint8 rdmy[0x2000];
	uint8* wbank[8];
	uint8* rbank[8];
	
public:
	PC80S31K(VM* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu) {}
	~PC80S31K() {}
	
	// common functions
	void initialize();
	void reset();
	uint32 read_data8(uint32 addr);
	uint32 fetch_op(uint32 addr, int *wait);
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
	void set_context_fdc(UPD765A* device)
	{
		d_fdc = device;
	}
	void set_context_pio(DEVICE* device)
	{
		d_pio = device;
	}
};

#endif

