/*
	MITSUBISHI Electric MULTI8 Emulator 'EmuLTI8'

	Author : Takeda.Toshiya
	Date   : 2006.09.15 -

	[ kanji rom ]
*/

#include "kanji.h"
#include "../i8255.h"

void KANJI::initialize()
{
	// load rom image
	FILEIO* fio = new FILEIO();
	if(fio->Fopen(create_local_path(_T("KANJI.ROM")), FILEIO_READ_BINARY)) {
		fio->Fread(rom, sizeof(rom), 1);
		fio->Fclose();
		
		// 8255 Port A, bit6 = 0 (kanji rom exists)
		d_pio->write_signal(SIG_I8255_PORT_A, 0, 0x40);
	} else {
		// 8255 Port A, bit6 = 1 (kanji rom does not exist)
		d_pio->write_signal(SIG_I8255_PORT_A, 0x40, 0x40);
	}
	delete fio;
}

void KANJI::reset()
{
	ptr = 0;
}

void KANJI::write_io8(uint32_t addr, uint32_t data)
{
	switch(addr & 0xff) {
	case 0x40:
		ptr = (ptr & 0xff00) | data;
		break;
	case 0x41:
		ptr = (ptr & 0x00ff) | (data << 8);
		break;
	}
}

uint32_t KANJI::read_io8(uint32_t addr)
{
	switch(addr & 0xff) {
	case 0x40:
		return rom[(ptr << 1) | 0];
	case 0x41:
		return rom[(ptr << 1) | 1];
	}
	return 0xff;
}

#define STATE_VERSION	1

bool KANJI::process_state(FILEIO* state_fio, bool loading)
{
	if(!state_fio->StateCheckUint32(STATE_VERSION)) {
		return false;
	}
	if(!state_fio->StateCheckInt32(this_device_id)) {
		return false;
	}
	state_fio->StateValue(ptr);
	return true;
}

