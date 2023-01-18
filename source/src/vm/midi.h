/*
	Skelton for retropc emulator

	Author : Takeda.Toshiya
	Date   : 2022.07.03-

	[ midi port ]
*/

#ifndef _MIDI_H_
#define _MIDI_H_

#include "vm.h"
#include "../emu.h"
#include "device.h"

#define SIG_MIDI_OUT	0

class MIDI : public DEVICE
{
private:
	outputs_t outputs;
	
public:
	MIDI(VM_TEMPLATE* parent_vm, EMU* parent_emu) : DEVICE(parent_vm, parent_emu)
	{
		initialize_output_signals(&outputs);
		set_device_name(_T("MIDI port"));
	}
	~MIDI() {}
	
	// common functions
	void initialize();
	void write_signal(int id, uint32_t data, uint32_t mask);
	void event_vline(int v, int clock);
	
	// unique function
	void set_context_in(DEVICE* device, int id, uint32_t mask)
	{
		register_output_signal(&outputs, device, id, mask);
	}
};

#endif

