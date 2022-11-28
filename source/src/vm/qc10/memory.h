/*
	EPSON QC-10 Emulator 'eQC-10'

	Author : Takeda.Toshiya
	Date   : 2008.02.15 -

	[ memory ]
*/

#ifndef _MEMORY_H_
#define _MEMORY_H_

#include "../vm.h"
#include "../../emu.h"
#include "../device.h"

#define SIG_MEMORY_PCM		0
#define SIG_MEMORY_FDC_IRQ	1
#define SIG_MEMORY_MOTOR	2

class UPD765A;

class MEMORY : public DEVICE
{
private:
	DEVICE *d_pit, *d_pcm;
	UPD765A *d_fdc;
	
	uint8_t* rbank[32];
	uint8_t* wbank[32];
	uint8_t wdmy[0x10000];
	uint8_t rdmy[0x10000];
	uint8_t ipl[0x2000];
	uint8_t ram[0x40000];
	uint8_t cmos[0x800];
	uint32_t cmos_crc32;
	uint8_t bank, psel, csel;
	void update_map();
	
	bool pcm_on, pcm_cont, pcm_pit;
	void update_pcm();
	
	bool fdc_irq, motor;
	
public:
	MEMORY(VM_TEMPLATE* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu)
	{
		set_device_name(_T("Memory Bus"));
	}
	~MEMORY() {}
	
	// common functions
	void initialize();
	void release();
	void reset();
	void write_data8(uint32_t addr, uint32_t data);
	uint32_t read_data8(uint32_t addr);
	void write_io8(uint32_t addr, uint32_t data);
	uint32_t read_io8(uint32_t addr);
	void write_signal(int id, uint32_t data, uint32_t mask);
	bool process_state(FILEIO* state_fio, bool loading);
	
	// unique functions
	void set_context_pit(DEVICE* device)
	{
		d_pit = device;
	}
	void set_context_pcm(DEVICE* device)
	{
		d_pcm = device;
	}
	void set_context_fdc(UPD765A* device)
	{
		d_fdc = device;
	}
};

#endif

