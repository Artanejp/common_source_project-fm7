/*
	SHARP MZ-2200 Emulator 'EmuZ-2200'

	Author : Takeda.Toshiya
	Date   : 2013.03.30-

	[ MZ-1M01 (16bit Board) ]
*/

#ifndef _MZ1M01_H_
#define _MZ1M01_H_

#include "../vm.h"
#include "../../emu.h"
#include "../device.h"

#define SIG_MZ1M01_PORT_A	0
#define SIG_MZ1M01_PORT_B	1

class MZ1M01 : public DEVICE
{
private:
	DEVICE *d_cpu, *d_pic, *d_pio;
	
	uint8_t* rbank[128];	// 1MB / 8KB
	uint8_t* wbank[128];
	uint8_t wdmy[0x2000];
	uint8_t rdmy[0x2000];
	uint8_t ipl[0x2000];	// IPL 8KB
	uint8_t ram[0x20000];	// Main RAM 128KB
	uint8_t kanji[0x20000];	// Kanji ROM 128KB
	
	uint8_t port[2];
	
public:
	MZ1M01(VM_TEMPLATE* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu)
	{
		set_device_name(_T("MZ-1M01 (16bit Board)"));
	}
	~MZ1M01() {}
	
	// common functions
	void initialize();
	void reset();
	void write_data8(uint32_t addr, uint32_t data);
	uint32_t read_data8(uint32_t addr);
	void write_io8(uint32_t addr, uint32_t data);
	uint32_t read_io8(uint32_t addr);
	void write_signal(int id, uint32_t data, uint32_t mask);
	bool process_state(FILEIO* state_fio, bool loading);
	
	// unique function
	void set_context_cpu(DEVICE* device)
	{
		d_cpu = device;
	}
	void set_context_pic(DEVICE* device)
	{
		d_pic = device;
	}
	void set_context_pio(DEVICE* device)
	{
		d_pio = device;
	}
};

#endif

