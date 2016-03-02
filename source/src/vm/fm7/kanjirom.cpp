/*
 * Common source code project -> FM-7/77/AV -> Kanji rom
 * (C) 2015 K.Ohta <whatisthis.sowhat _at_ gmail.com>
 * License: GPLv2
 * History:
 *  Feb 11, 2015 : Initial
 */

#include "../../fileio.h"
#include "fm7_common.h"
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
		if(fio->Fopen(create_local_path(_T("KANJI2.ROM")), FILEIO_READ_BINARY)) {
		  fio->Fread(data_table, 0x20000, 1);
			fio->Fclose();
			read_ok = true;
		}
	} else {
		class2 = false;
		if(fio->Fopen(create_local_path(_T("KANJI1.ROM")), FILEIO_READ_BINARY)) {
		  fio->Fread(data_table, 0x20000, 1);
			fio->Fclose();
			read_ok = true;
		} else if(fio->Fopen(create_local_path(_T("KANJI.ROM")), FILEIO_READ_BINARY)) {
		  fio->Fread(data_table, 0x20000, 1);
			fio->Fclose();
			read_ok = true;
		} 
	}
	kanjiaddr.d = 0;
	delete fio;
	return;
}

void KANJIROM::reset(void)
{
	kanjiaddr.d = 0;
}

void KANJIROM::write_data8(uint32_t addr, uint32_t data)
{
	switch(addr) {
	case KANJIROM_ADDR_HI:
		kanjiaddr.b.h = data & 0xff;
		break;
	case KANJIROM_ADDR_LO:
		kanjiaddr.b.l = data & 0xff;
		break;
	}		
	return;
}

uint32_t KANJIROM::read_data8(uint32_t addr)
{
	if(addr == KANJIROM_DATA_HI) {
		return data_table[(kanjiaddr.d << 1) & 0x1ffff];
	} else if(addr == KANJIROM_DATA_LO) {
		return data_table[((kanjiaddr.d << 1) & 0x1ffff) + 1];
	} else if(addr == KANJIROM_READSTAT) {
		return (read_ok) ? 0xffffffff : 0x00000000;
	} else if((addr >= KANJIROM_DIRECTADDR) && (addr < (KANJIROM_DIRECTADDR + 0x20000))) {
		return data_table[addr - KANJIROM_DIRECTADDR];
	}
	return 0x00000000;
}

bool KANJIROM::get_readstat(void)
{
	return read_ok;
}

void KANJIROM::release()
{
}

#define STATE_VERSION 2
void KANJIROM::save_state(FILEIO *state_fio)
{
	state_fio->FputUint32_BE(STATE_VERSION);
	state_fio->FputInt32_BE(this_device_id);

	state_fio->FputBool(class2);
	state_fio->FputBool(read_ok);
	state_fio->Fwrite(data_table, sizeof(data_table), 1);
	state_fio->FputUint16_BE(kanjiaddr.w.l);
}

bool KANJIROM::load_state(FILEIO *state_fio)
{
	uint32_t version;
	version = state_fio->FgetUint32_BE();
	if(this_device_id != state_fio->FgetInt32_BE()) return false;

	if(version >= 1) {
		class2 = state_fio->FgetBool();
		read_ok = state_fio->FgetBool();
		state_fio->Fread(data_table, sizeof(data_table), 1);
		if(version == 1) return true;
	}
	if(version >= 2) {
		kanjiaddr.d = 0;
		kanjiaddr.w.l = state_fio->FgetUint16_BE();
		if(version == 2) return true;
	}
	return false;
}

