/*
 * Common source code project -> FM-7/77/AV -> Kanji rom
 * (C) 2015 K.Ohta <whatisthis.sowhat _at_ gmail.com>
 * License: GPLv2
 * History:
 *  Feb 11, 2015 : Initial
 */

#include "../vm.h"
#include "../../emu.h"
#include "../../fileio.h"
#include "fm7_common.h"
#include "kanjirom.h"

KANJIROM::KANJIROM(VM_TEMPLATE* parent_vm, EMU* parent_emu, bool type_2std): DEVICE(parent_vm, parent_emu)
{
	FILEIO *fio;
	read_ok = false;
	
	fio = new FILEIO();
	memset(data_table, 0xff, 0x20000); 
	//	read_table[0].memory = data_table;
	if(type_2std) {
		class2 = true;
		if(fio->Fopen(create_local_path(_T(ROM_KANJI_CLASS2)), FILEIO_READ_BINARY)) {
		  fio->Fread(data_table, 0x20000, 1);
			fio->Fclose();
			read_ok = true;
		}
	} else {
		class2 = false;
		if(fio->Fopen(create_local_path(_T(ROM_KANJI_CLASS1)), FILEIO_READ_BINARY)) {
		  fio->Fread(data_table, 0x20000, 1);
			fio->Fclose();
			read_ok = true;
		} else if(fio->Fopen(create_local_path(_T(ROM_KANJI_CLASS1_FALLBACK)), FILEIO_READ_BINARY)) {
		  fio->Fread(data_table, 0x20000, 1);
			fio->Fclose();
			read_ok = true;
		}
		
	}
	if(class2) {
		set_device_name(_T("FM7_KANJI_CLASS2"));
	} else {
		set_device_name(_T("FM7_KANJI_CLASS1"));
	}
	if(class2) {
		set_device_name(_T("FM7_KANJI_CLASS2"));
	} else {
		set_device_name(_T("FM7_KANJI_CLASS1"));
	}
	this->out_debug_log(_T("KANJIROM READ %s."), read_ok ? "OK" : "FAILED");
	
	kanjiaddr.d = 0;
	delete fio;
	return;
}

KANJIROM::~KANJIROM()
{
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

#define STATE_VERSION 5

bool KANJIROM::decl_state(FILEIO *state_fio, bool loading)
{
	if(!state_fio->StateCheckUint32(STATE_VERSION)) {
		return false;
	}
	if(!state_fio->StateCheckInt32(this_device_id)) {
		return false;
	}
	
	state_fio->StateValue(class2);
	state_fio->StateValue(read_ok);
	state_fio->StateArray(data_table, sizeof(data_table), 1);
	state_fio->StateValue(kanjiaddr.d);

	return true;
}
void KANJIROM::save_state(FILEIO *state_fio)
{
	decl_state(state_fio, false);
}

bool KANJIROM::load_state(FILEIO *state_fio)
{
	bool mb = decl_state(state_fio, true);
	if(!mb) return false;
	kanjiaddr.w.h = 0;
	return true;
}

