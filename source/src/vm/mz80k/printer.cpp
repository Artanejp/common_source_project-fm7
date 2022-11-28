/*
	SHARP MZ-80K/C Emulator 'EmuZ-80K'
	SHARP MZ-1200 Emulator 'EmuZ-1200'
	SHARP MZ-80A Emulator 'EmuZ-80A'

	Author : Hideki Suga
	Date   : 2016.03.18-

	[ printer ]
*/

#include "printer.h"

void PRINTER::write_io8(uint32_t addr, uint32_t data)
{
	switch(addr & 0xff) {
	case 0xfe:
		d_prn->write_signal(SIG_PRINTER_STROBE, data, 0x80);
		d_prn->write_signal(SIG_PRINTER_RESET, data, 0x40);
		break;
	case 0xff:
#if defined(_MZ1200) || defined(_MZ80K)
		out_ch = data & 0xff;
#endif
		d_prn->write_signal(SIG_PRINTER_DATA, data, 0xff);
		break;
	}
}

uint32_t PRINTER::read_io8(uint32_t addr)
{
	switch(addr & 0xff) {
	case 0xfe:
#if defined(_MZ1200) || defined(_MZ80K)
		if(out_ch == 0x05) {
			return 0xf0 | (d_prn->read_signal(SIG_PRINTER_BUSY) ? 1 : 0);
		} else
#endif
		return 0xf2 | (d_prn->read_signal(SIG_PRINTER_BUSY) ? 1 : 0);
	}
	return 0xff;
}

#if defined(_MZ1200) || defined(_MZ80K)
#define STATE_VERSION	1

bool PRINTER::process_state(FILEIO* state_fio, bool loading)
{
	if(!state_fio->StateCheckUint32(STATE_VERSION)) {
		return false;
	}
	if(!state_fio->StateCheckInt32(this_device_id)) {
		return false;
	}
	state_fio->StateValue(out_ch);
	return true;
}
#endif

