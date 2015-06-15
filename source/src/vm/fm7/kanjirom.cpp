/*
 * Common source code project -> FM-7/77/AV -> Kanji rom
 * (C) 2015 K.Ohta <whatisthis.sowhat _at_ gmail.com>
 * License: GPLv2
 * History:
 *  Feb 11, 2015 : Initial
 */

#include "../../fileio.h"
#include "kanjirom.h"

KANJIROM::KANJIROM(VM *parent_vm, EMU* parent_emu, bool type_2std): DEVICE(parent_vm, parent_emu)
{
	FILEIO *fio;
	read_ok = false;
	
	fio = new FILEIO();
	memset(data_table, 0xff, 0x20000); 
	//	read_table[0].memory = data_table;
	
	if(type_2std) {
		class2 = true;
		if(fio->Fopen(emu->bios_path("KANJI2.ROM"), FILEIO_READ_BINARY)) {
		  fio->Fread(data_table, 0x20000, 1);
			fio->Fclose();
			read_ok = true;
		}
	} else {
		class2 = false;
		if(fio->Fopen(emu->bios_path("KANJI1.ROM"), FILEIO_READ_BINARY)) {
		  fio->Fread(data_table, 0x20000, 1);
			fio->Fclose();
			read_ok = true;
		} else if(fio->Fopen(emu->bios_path("KANJI.ROM"), FILEIO_READ_BINARY)) {
		  fio->Fread(data_table, 0x20000, 1);
			fio->Fclose();
			read_ok = true;
		} 
	}
	delete fio;
	return;
}

void KANJIROM::write_data8(uint32 addr, uint32 data)
{
	return;
}

uint32 KANJIROM::read_data8(uint32 addr)
{
	return data_table[addr & 0x1ffff];
}

bool KANJIROM::get_readstat(void)
{
	return read_ok;
}

void KANJIROM::release()
{
}

#define STATE_VERSION 1
void KANJIROM::save_state(FILEIO *state_fio)
{
	state_fio->FputUint32(STATE_VERSION);
	state_fio->FputInt32(this_device_id);

	state_fio->FputBool(class2);
	state_fio->FputBool(read_ok);
	state_fio->Fwrite(data_table, sizeof(data_table), 1);
}

bool KANJIROM::load_state(FILEIO *state_fio)
{
	uint32 version;
	version = state_fio->FgetUint32();
	if(this_device_id != state_fio->FgetInt32()) return false;

	if(version >= 1) {
		class2 = state_fio->FgetBool();
		read_ok = state_fio->FgetBool();
		state_fio->Fread(data_table, sizeof(data_table), 1);
		if(version == 1) return true;
	}
	return false;
}

