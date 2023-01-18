/*
	Skelton for retropc emulator

	Author : K.Ohta <whatisthis.sowhat _at_ gmail.com>
	Date   : 202301.18-

	[Qt/libPortMidi MIDI ]
*/
#include "../emu.h"
#include "../fifo.h"
#include "../types/util_sound.h"
#include "../types/util_endians.h"

#include "qt_main.h"
#include "gui/menu_flags.h"

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
#include <QApplication>
#endif

#include <cstdint>

// Note: Will implement real body.
void OSD_BASE::initialize_midi()
{
}

void OSD_BASE::release_midi()
{
}

void OSD_BASE::send_to_midi(uint8_t data, int ch, double timestamp_usec)
{
	// Need to convert timestamp_usec to relative mSec.
}

bool OSD_BASE::recv_from_midi(uint8_t* data, int ch, double timestamp_usec)
{
	// Need to convert timestamp_usec to relative mSec.
	return false;
}

bool OSD_BASE::send_to_midi_timeout(uint8_t data, int ch, uint64_t timeout_ms, double timestamp_usec)
{
	// Need to convert timestamp_usec to relative mSec.
	return false;
}

bool OSD_BASE::recv_from_midi_timeout(uint8_t* data, int ch, uint64_t timeout_ms, double timestamp_usec)
{
	// Need to convert timestamp_usec to relative mSec.
	return false;
}

void OSD_BASE::notify_timeout_sending_to_midi(int ch)
{
	if(vm != nullptr) {
		vm->notify_timeout_sending_to_midi(ch);
	}
}

void OSD_BASE::notify_timeout_receiving_from_midi(int ch)
{
	if(vm != nullptr) {
		vm->notify_timeout_receiving_from_midi(ch);
	}
}

void OSD_BASE::reset_to_midi(int ch, double timestamp_usec)
{
	//ToDo: Will implement.
}

void OSD_BASE::initialize_midi_device(bool handshake_from_midi, bool handshake_to_midi, int ch)
{
	//ToDo: Will implement.
}

void OSD_BASE::ready_receive_from_midi(int ch, double timestamp_usec)
{
	if(vm != nullptr) {
		vm->ready_receive_from_midi(ch, timestamp_usec);
	}
}

void OSD_BASE::ready_send_to_midi(int ch, double timestamp_usec)
{
	// ToDo: Will Implement.
}

void OSD_BASE::request_stop_to_receive_from_midi(int ch, double timestamp_usec)
{
	// ToDo: Will Implement.
}

void OSD_BASE::request_stop_to_send_to_midi(int ch, double timestamp_usec)
{
	if(vm != nullptr) {
		vm->request_stop_to_send_to_midi(ch, timestamp_usec);
	}
}

