/*
	SHARP MZ-80K Emulator 'EmuZ-80K'
	SHARP MZ-1200 Emulator 'EmuZ-1200'

	Author : Takeda.Toshiya
	Date   : 2010.08.18-

	SHARP MZ-80A Emulator 'EmuZ-80A'
	Modify : Hideki Suga
	Date   : 2014.12.10 -

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
	DEVICE *d_ctc, *d_pio;
#if defined(_MZ1200) || defined(_MZ80A)
	DEVICE *d_disp;
#endif
	
	uint8* rbank[64];
	uint8* wbank[64];
	uint8 wdmy[0x400];
	uint8 rdmy[0x400];
	
	uint8 ram[0xd000];	// RAM 48KB + swap 4KB
#if defined(_MZ80A)
	uint8 e200;		// scroll
	uint8 vram[0x800];	// VRAM 2KB
#else
	uint8 vram[0x400];	// VRAM 1KB
#endif

	uint8 ipl[0x1000];	// IPL 4KB
#if defined(_MZ1200) || defined(_MZ80A)
	uint8 ext[0x1800];	// EXT 6KB1024
#endif
	
	bool tempo, blink;
#if defined(_MZ1200) || defined(_MZ80A)
	bool hblank;
#endif
	
public:
	MEMORY(VM* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu) {}
	~MEMORY() {}
	
	// common functions
	void initialize();
	void reset();
	void event_vline(int v, int clock);
	void event_callback(int event_id, int err);
	
	void write_data8(uint32 addr, uint32 data);
	uint32 read_data8(uint32 addr);
	
	// unitque function
	void set_context_ctc(DEVICE* device)
	{
		d_ctc = device;
	}
	void set_context_pio(DEVICE* device)
	{
		d_pio = device;
	}
#if defined(_MZ1200) || defined(_MZ80A)
	void set_context_disp(DEVICE* device)
	{
		d_disp = device;
	}
#endif
#if defined(_MZ80A)
	uint8* get_e200()
	{
		return &e200;
	}
#endif
	uint8* get_vram()
	{
		return vram;
	}
};

#endif

