/*
	SHARP MZ-80B Emulator 'EmuZ-80B'
	SHARP MZ-2200 Emulator 'EmuZ-2200'

	Author : Takeda.Toshiya
	Date   : 2013.03.17-

	[ MZ-1R12 (32KB SRAM) ]
*/

#include "mz1r12.h"

void MZ1R12::initialize()
{
	memset(sram, 0, sizeof(sram));
	read_only = false;
	
	FILEIO* fio = new FILEIO();
#ifndef _MZ80B
	if(fio->Fopen(create_local_path(_T("MZ-1E18.ROM")), FILEIO_READ_BINARY)) {
		fio->Fread(sram, sizeof(sram), 1);
		fio->Fclose();
		read_only = true;
	} else
#endif
	if(fio->Fopen(create_local_path(_T("MZ-1R12.BIN")), FILEIO_READ_BINARY)) {
		fio->Fread(sram, sizeof(sram), 1);
		fio->Fclose();
	}
	delete fio;
	
	address = 0;
	crc32 = get_crc32(sram, sizeof(sram));
}

void MZ1R12::release()
{
	if(!read_only && crc32 != get_crc32(sram, sizeof(sram))) {
		FILEIO* fio = new FILEIO();
		if(fio->Fopen(create_local_path(_T("MZ-1R12.BIN")), FILEIO_WRITE_BINARY)) {
			fio->Fwrite(sram, sizeof(sram), 1);
			fio->Fclose();
		}
		delete fio;
	}
}

void MZ1R12::write_io8(uint32_t addr, uint32_t data)
{
	switch(addr & 0xff) {
	case 0xf8:
		address = (address & 0x00ff) | (data << 8);
		break;
	case 0xf9:
		address = (address & 0xff00) | (data << 0);
		break;
	case 0xfa:
		if(!read_only) {
			sram[address & 0x7fff] = data;
		}
		address++;
		break;
	}
}

uint32_t MZ1R12::read_io8(uint32_t addr)
{
	switch(addr & 0xff) {
	case 0xf8:
		address = 0;
		break;
	case 0xf9:
		return sram[(address++) & 0x7fff];
	}
	return 0xff;
}

#define STATE_VERSION	1

bool MZ1R12::process_state(FILEIO* state_fio, bool loading)
{
	if(!state_fio->StateCheckUint32(STATE_VERSION)) {
		return false;
	}
	if(!state_fio->StateCheckInt32(this_device_id)) {
		return false;
	}
	state_fio->StateArray(sram, sizeof(sram), 1);
	state_fio->StateValue(read_only);
	state_fio->StateValue(address);
	state_fio->StateValue(crc32);
	return true;
}

