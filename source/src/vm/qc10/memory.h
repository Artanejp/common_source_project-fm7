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
	
	uint8* rbank[32];
	uint8* wbank[32];
	uint8 wdmy[0x10000];
	uint8 rdmy[0x10000];
	uint8 ipl[0x2000];
	uint8 ram[0x40000];
	uint8 cmos[0x800];
	uint32 cmos_crc32;
	uint8 bank, psel, csel;
	void update_map();
	
	bool pcm_on, pcm_cont, pcm_pit;
	void update_pcm();
	
	bool fdc_irq, motor;
	
public:
	MEMORY(VM* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu) {}
	~MEMORY() {}
	
	// common functions
	void initialize();
	void release();
	void reset();
	void write_data8(uint32 addr, uint32 data);
	uint32 read_data8(uint32 addr);
	void write_io8(uint32 addr, uint32 data);
	uint32 read_io8(uint32 addr);
	void write_signal(int id, uint32 data, uint32 mask);
	void save_state(FILEIO* state_fio);
	bool load_state(FILEIO* state_fio);
	
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

