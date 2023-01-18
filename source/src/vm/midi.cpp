/*
	Skelton for retropc emulator

	Author : Takeda.Toshiya
	Date   : 2022.07.03-

	[ midi port ]
*/

#include "midi.h"

void MIDI::initialize()
{
	// midi-in is not implemenetd in win32-osd :-(
//	register_vline_event(this);
}

void MIDI::write_signal(int id, uint32_t data, uint32_t mask)
{
	if(id == SIG_MIDI_OUT) {
		emu->send_to_midi(data & mask);
	}
}

void MIDI::event_vline(int v, int clock)
{
	uint8_t data;
	
	while(emu->recv_from_midi(&data)) {
		write_signals(&outputs, data);
	}
}
