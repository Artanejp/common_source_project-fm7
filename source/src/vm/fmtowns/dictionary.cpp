/*
	FUJITSU FM Towns Emulator 'eFMTowns'

	Author : Kyuma.Ohta <whatisthis.sowhat _at_ gmail.com>
	Date   : 2019.01.09-

	[ dictionary rom/ram & cmos & RAM area 0x000d0000 - 0x000dffff]
	* MEMORY :
	*   0x000d0000 - 0x000d7fff : DICTIONARY ROM (BANKED)
	*   0x000d8000 - 0x000d9fff : DICTIONARY RAM / GAIJI RAM
	*   0x000da000 - 0x000dffff : RESERVED
	*   0xc2080000 - 0xc20fffff : DICTIONARY ROM (NOT BANKED)
	*   0xc2140000 - 0xc2141fff : DICTIONARY RAM
	* I/O :
	*   0x0484                         : DICTIONARY BANK (for 0xd0000 - 0xd7ffff)
	*   0x3000 - 0x3ffe (even address) : DICTIONARY RAM
*/

#include "./towns_common.h"
#include "./cmos.h"
#include "./dictionary.h"
#include "../../fileio.h"

namespace FMTOWNS {
void DICTIONARY::initialize()
{
	memset(dict_rom, 0xff, sizeof(dict_rom));

	FILEIO* fio = new FILEIO();
	if(fio->Fopen(create_local_path(_T("FMT_DIC.ROM")), FILEIO_READ_BINARY)) { // DICTIONARIES
		fio->Fread(dict_rom, sizeof(dict_rom), 1);
		fio->Fclose();
	}
	delete fio;
}

void DICTIONARY::reset()
{
	dict_bank = 0;
}

uint32_t DICTIONARY::read_dma_data8w(uint32_t addr, int* wait)
{
	return read_memory_mapped_io8w(addr, wait); // OK?
}

uint32_t DICTIONARY::read_memory_mapped_io8w(uint32_t addr, int* wait)
{
	__LIKELY_IF(wait != NULL) {
		*wait = 0; // ToDo
	}
	// 0xd0000 - 0xdffff : primary  is VRAM, secondary is DICTIONARY.
	__LIKELY_IF((addr < 0x000d8000) && (addr >= 0x000d0000)) {
		return dict_rom[(((uint32_t)dict_bank) << 15) | (addr & 0x7fff)];
	}
	__LIKELY_IF((addr >= 0xc2080000) && (addr < 0xc2100000)) {
		return dict_rom[addr & 0x7ffff];
	}
	return 0xff;
}


void DICTIONARY::write_io8(uint32_t addr, uint32_t data)
{
	__LIKELY_IF(addr == 0x0484) {
		dict_bank = data & 0x0f;
	}
}

uint32_t DICTIONARY::read_io8(uint32_t addr)
{
	uint32_t data = 0xff;
	__LIKELY_IF(addr == 0x0484) {
		data = dict_bank & 0x0f;
	}
	return data;
}

void DICTIONARY::write_signal(int ch, uint32_t data, uint32_t mask)
{
	switch(ch) {
	case SIG_FMTOWNS_DICT_BANK:
		dict_bank = (uint8_t)(data & 0x0f);
		break;
	}
}

uint32_t DICTIONARY::read_signal(int ch)
{
	switch(ch) {
	case SIG_FMTOWNS_DICT_BANK:
		return (uint32_t)(dict_bank & 0x0f);
		break;
	}
	return 0x00;
}

uint32_t DICTIONARY::read_debug_data8(uint32_t addr)
{
	// May read ram only
	return dict_rom[addr & 0x7ffff];
}


bool DICTIONARY::write_debug_reg(const _TCHAR *reg, uint32_t data)
{
	if(reg == NULL) return false;
	if(strcasecmp(reg, _T("bank")) == 0) {
		dict_bank = data & 0x0f;
		return true;
	}
	return false;
}

bool DICTIONARY::get_debug_regs_info(_TCHAR *buffer, size_t buffer_len)
{
	if(buffer == NULL) return false;
	if(buffer_len <= 1) return false;
	my_stprintf_s(buffer, buffer_len - 1,
				  _T("BANK=%02X\n"),
				  dict_bank);
	return true;
}

#define STATE_VERSION	2

bool DICTIONARY::process_state(FILEIO* state_fio, bool loading)
{
	if(!state_fio->StateCheckUint32(STATE_VERSION)) {
		return false;
	}
	if(!state_fio->StateCheckInt32(this_device_id)) {
		return false;
	}
	state_fio->StateValue(dict_bank);
	return true;
}
}
