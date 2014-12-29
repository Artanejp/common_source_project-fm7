/*
	SHARP MZ-80B Emulator 'EmuZ-80B'
	SHARP MZ-2200 Emulator 'EmuZ-2200'
	SHARP MZ-2500 Emulator 'EmuZ-2500'

	Author : Takeda.Toshiya
	Date   : 2006.12.01 -

	[ MZ-1R13 (Kanji ROM) ]
*/

#include "mz1r13.h"
#include "../../fileio.h"

void MZ1R13::initialize()
{
	// load rom images
	memset(kanji, 0xff, sizeof(kanji));
	memset(dic, 0xff, sizeof(dic));
	
	FILEIO* fio = new FILEIO();
	if(fio->Fopen(emu->bios_path(_T("MZ-1R13_KAN.ROM")), FILEIO_READ_BINARY) || 
	   fio->Fopen(emu->bios_path(_T("KANJI2.ROM")), FILEIO_READ_BINARY)) {
		fio->Fread(kanji, sizeof(kanji), 1);
		fio->Fclose();
	}
	if(fio->Fopen(emu->bios_path(_T("MZ-1R13_DIC.ROM")), FILEIO_READ_BINARY)) {
		fio->Fread(dic, sizeof(dic), 1);
		fio->Fclose();
	}
	delete fio;
	
	address = 0;
	select_kanji = true;
}

void MZ1R13::write_io8(uint32 addr, uint32 data)
{
	switch(addr & 0xff) {
	case 0xb8:
		address = (address & 0xff00) | (data << 0);
		break;
	case 0xb9:
		address = (address & 0x00ff) | (data << 8);
		break;
	case 0xba:
		select_kanji = ((data & 1) != 0);
		break;
	case 0xbb:
		address++;
		break;
	}
}

uint32 MZ1R13::read_io8(uint32 addr)
{
	uint32 offset = (address << 1) | (addr & 1);
	uint8 value = select_kanji ? kanji[offset & 0x1ffff] : dic[offset & 0x3fff];
	
	switch(addr & 0xff) {
	case 0xb9:
		address++;
	case 0xb8:
		return value;
	case 0xbb:
		address++;
	case 0xba:
		return ((value >> 7) & 0x01) | ((value >> 5) & 0x02) | ((value >> 3) & 0x04) | ((value >> 1) & 0x08) |
		       ((value << 1) & 0x10) | ((value << 3) & 0x20) | ((value << 5) & 0x40) | ((value << 7) & 0x80);
	}
	return 0xff;
}

#define STATE_VERSION	1

void MZ1R13::save_state(FILEIO* state_fio)
{
	state_fio->FputUint32(STATE_VERSION);
	state_fio->FputInt32(this_device_id);
	
	state_fio->FputUint16(address);
	state_fio->FputBool(select_kanji);
}

bool MZ1R13::load_state(FILEIO* state_fio)
{
	if(state_fio->FgetUint32() != STATE_VERSION) {
		return false;
	}
	if(state_fio->FgetInt32() != this_device_id) {
		return false;
	}
	address = state_fio->FgetUint16();
	select_kanji = state_fio->FgetBool();
	return true;
}

