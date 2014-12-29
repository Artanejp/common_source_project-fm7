/*
	NEC PC-100 Emulator 'ePC-100'

	Author : Takeda.Toshiya
	Date   : 2008.07.14 -

	[ kanji rom ]
*/

#include "kanji.h"
#include "../../fileio.h"

void KANJI::initialize()
{
	// init image
	memset(kanji, 0xff, sizeof(kanji));
	
	// load kanji image
	FILEIO* fio = new FILEIO();
	if(fio->Fopen(emu->bios_path(_T("KANJI.ROM")), FILEIO_READ_BINARY)) {
		fio->Fread(kanji, sizeof(kanji), 1);
		fio->Fclose();
	}
	delete fio;
	
	ptr = 0;
	strobe = false;
}

void KANJI::write_io8(uint32 addr, uint32 data)
{
	switch(addr & 0xff) {
	case 0x80:
		ptr = (ptr & 0xff00) | data;
		break;
	case 0x81:
		ptr = (ptr & 0x00ff) | (data << 8);
		break;
	case 0x84:
		strobe = true;
		break;
	case 0x86:
		strobe = false;
		break;
	}
}

uint32 KANJI::read_io8(uint32 addr)
{
	switch(addr & 0xff) {
	case 0x80:
		return kanji[(ptr << 1) | 0];
	case 0x81:
		return kanji[(ptr << 1) | 1];
	}
	return 0xff;
}

