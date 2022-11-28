/*
	TOSHIBA PASOPIA Emulator 'EmuPIA'
	TOSHIBA PASOPIA 7 Emulator 'EmuPIA7'

	Author : Takeda.Toshiya
	Date   : 2006.09.20 -

	[ kanji rom pac 2 ]
*/

#include "kanjipac2.h"

void KANJIPAC2::initialize(int id)
{
	FILEIO* fio = new FILEIO();
	if(fio->Fopen(create_local_path(_T("KANJI.ROM")), FILEIO_READ_BINARY)) {
		fio->Fread(rom, sizeof(rom), 1);
		fio->Fclose();
	}
	delete fio;
}

void KANJIPAC2::reset()
{
	ptr = 0;
}

void KANJIPAC2::write_io8(uint32_t addr, uint32_t data)
{
	switch(addr & 0xff) {
	case 0x18:
		ptr = (ptr & 0x1ff00) | data;
		break;
	case 0x19:
		ptr = (ptr & 0x100ff) | (data << 8);
		break;
	case 0x1a:
		ptr = (ptr & 0x0ffff) | ((data & 0x01) << 16);
		break;
	}
}

uint32_t KANJIPAC2::read_io8(uint32_t addr)
{
	return rom[ptr & 0x1ffff];
}

#define STATE_VERSION	1

bool KANJIPAC2::process_state(FILEIO* state_fio, bool loading)
{
	if(!state_fio->StateCheckUint32(STATE_VERSION)) {
		return false;
	}
	state_fio->StateValue(ptr);
	return true;
}

