/*
 * Common source code project -> FM-7/77/AV -> Kanji rom
 * (C) 2015 K.Ohta <whatisthis.sowhat _at_ gmail.com>
 * License: GPLv2
 * History:
 *  Feb 11, 2015 : Initial
 */

#include "../../fileio.h"
#include "kanjirom.h"

KANJIROM::KANJIROM(VM *parent_vm, EMU* parent_emu, bool type_2std): MEMORY(parent_vm, parent_emu)
{
	int bank_num = MEMORY_ADDR_MAX / MEMORY_BANK_SIZE;
	int i;
	FILEIO *fio;
	
	read_ok = false;
	
	fio = new FILEIO();
	memset(data_table, 0xff, MEMORY_ADDR_MAX); 
	//	read_table[0].memory = data_table;
	
	if(type_2std) {
		class2 = true;
		if(fio->Fopen(emu->bios_path("KANJI2.ROM"), FILEIO_READ_BINARY)) {
		  fio->Fread(data_table, MEMORY_ADDR_MAX, 1);
			fio->Fclose();
			read_ok = true;
		}
	} else {
		class2 = false;
		if(fio->Fopen(emu->bios_path("KANJI1.ROM"), FILEIO_READ_BINARY)) {
		  fio->Fread(data_table, MEMORY_ADDR_MAX, 1);
			fio->Fclose();
			read_ok = true;
		} else if(fio->Fopen(emu->bios_path("KANJI.ROM"), FILEIO_READ_BINARY)) {
		  fio->Fread(data_table, MEMORY_ADDR_MAX, 1);
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
  //	return data_table[addr & 0x1ffff];
	return data_table[addr];
}

bool KANJIROM::get_readstat(void)
{
	return read_ok;
}

void KANJIROM::release()
{
	MEMORY::release();
}

