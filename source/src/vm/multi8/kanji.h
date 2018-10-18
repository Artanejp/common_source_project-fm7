/*
	MITSUBISHI Electric MULTI8 Emulator 'EmuLTI8'

	Author : Takeda.Toshiya
	Date   : 2006.09.15 -

	[ kanji rom ]
*/

#ifndef _KANJI_H_
#define _KANJI_H_

#include "../vm.h"
#include "../../emu.h"
#include "../device.h"

class KANJI : public DEVICE
{
private:
	DEVICE* d_pio;
	
	uint8_t rom[0x20000];
	uint32_t ptr;
	
public:
	KANJI(VM_TEMPLATE* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu)
	{
		set_device_name(_T("Kanji ROM"));
	}
	~KANJI() {}
	
	// common functions
	void initialize();
	void reset();
	void write_io8(uint32_t addr, uint32_t data);
	uint32_t read_io8(uint32_t addr);
	bool process_state(FILEIO* state_fio, bool loading);
	
	// unique function
	void set_context_pio(DEVICE* device)
	{
		d_pio = device;
	}
};

#endif

