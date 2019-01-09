/*
	FUJITSU FM Towns Emulator 'eFMTowns'

	Author : Kyuma.Ohta <whatisthis.sowhat _at_ gmail.com>
	Date   : 2019.01.09 -

	[fonts]
*/

#inclide "../towns_common.h"
#include "../../fileio.h"
#include "./fontroms.h"

namespace FMTOWNS {
	
void FONT_ROMS::initialize()
{
	wait_val = 3;
	memset(font_kanji16, sizeof(font_kanji16), 0xff);
	FILEIO* fio = new FILEIO();
	
	if(fio->Fopen(create_local_path(_T("FMT_FNT.ROM")), FILEIO_READ_BINARY)) { // FONT
		fio->Fread(font_kanji16, sizeof(font_kanji16), 1);
		fio->Fclose();
	}
	
#if defined(HAVE_20PIXS_FONT)	
	memset(font_kanji20, sizeof(font_kanji20), 0xff);
	if(fio->Fopen(create_local_path(_T("FMT_F20.ROM")), FILEIO_READ_BINARY)) { // FONT
		fio->Fread(font_kanji20, sizeof(font_kanji20), 1);
		fio->Fclose();
	}
#endif

	delete fio;
}

uint32_t FONT_ROMS::read_data8(uint32_t addr)
{
	if((addr >= 0xc2100000) && (addr < 0xc2140000)) {
		return (uint32_t)(font_kanji16[addr & 0x3ffff]);
	} else if((addr >= 0xc2180000) && (addr < 0xc2200000)) {
#if defined(HAVE_20PIXS_FONT)	
		return (uint32_t)(font_kanji20[addr & 0x7ffff]);
#else
		return 0xff;
#endif
	} else if((addr >= 0x000ca000) && (addr < 0x000ca800)) {
		return (uint32_t)(font_kanji16[0x1e800 + (addr & 0x7ff)]);
	} else if((addr >= 0x000cb000) && (addr < 0x000cc000)) {
		return (uint32_t)(font_kanji16[0x1f000 + (addr & 0x7ff)]);
	}
	return 0xff;
}

uint32_t FONT_ROMS::read_data8w(uint32_t addr, int* wait)
{
	if(wait != NULL) *wait = wait_val;
	return read_data8(addr);
}

void FONT_ROMS::write_data8w(uint32_t addr, uint32_t data, int* wait)
{
	if(wait != NULL) *wait = wait_val;
}

void FONT_ROMS::write_signal(int ch, uint32_t data, uint32_t mask)
{
	if(ch == SIG_FMTOWNS_SET_MEMWAIT) {
		wait_val = (int)data;
	}
}	

#define STATE_VERSION	1

bool FONT_ROMS::process_state(FILEIO* state_fio, bool loading)
{
	if(!state_fio->StateCheckUint32(STATE_VERSION)) {
 		return false;
 	}
	if(!state_fio->StateCheckInt32(this_device_id)) {
 		return false;
 	}
	state_fio->StateValue(wait_val);

	return true;
}

}
