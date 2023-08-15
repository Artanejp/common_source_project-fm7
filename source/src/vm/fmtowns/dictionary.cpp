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
#include "./dictionary.h"
#include "../../fileio.h"

namespace FMTOWNS {
void DICTIONARY::initialize()
{
	memset(dict_rom, 0xff, sizeof(dict_rom));
	memset(dict_ram, 0x00, sizeof(dict_ram));

	FILEIO* fio = new FILEIO();
	if(fio->Fopen(create_local_path(_T("FMT_DIC.ROM")), FILEIO_READ_BINARY)) { // DICTIONARIES
		fio->Fread(dict_rom, sizeof(dict_rom), 1);
		fio->Fclose();
	}

	cmos_dirty = true;
	if(fio->Fopen(create_local_path(_T("FMT_CMOS.BIN")), FILEIO_READ_BINARY)) {
		if(fio->Fread(dict_ram, sizeof(dict_ram), 1) == 1) {
			cmos_dirty = false;
		}
		fio->Fclose();
	}
	delete fio;

}

void DICTIONARY::release()
{
	if(cmos_dirty) {
		FILEIO* fio = new FILEIO();
		if(fio->Fopen(create_local_path(_T("FMT_CMOS.BIN")), FILEIO_WRITE_BINARY)) {
			fio->Fwrite(dict_ram, sizeof(dict_ram), 1);
			fio->Fclose();
		}
		delete fio;
	}
}

void DICTIONARY::reset()
{
	dict_bank = 0;
	// Write CMOD every resetting. 20200511 K.O
	if(cmos_dirty) {
		FILEIO* fio = new FILEIO();
		if(fio->Fopen(create_local_path(_T("FMT_CMOS.BIN")), FILEIO_WRITE_BINARY)) {
			fio->Fwrite(dict_ram, sizeof(dict_ram), 1);
			fio->Fclose();
		}
		delete fio;
		cmos_dirty = false;
	}
}

uint32_t DICTIONARY::read_memory_mapped_io8w(uint32_t addr, int *wait)
{
	uint8_t n_data = 0xff;
	// 0xd0000 - 0xdffff : primary  is VRAM, secondary is DICTIONARY.
	if((addr < 0x000da000) && (addr >= 0x000d0000)) {
		*wait = 6; // temporally.
		if(addr < 0xd8000) {
			return dict_rom[(((uint32_t)dict_bank) << 15) | (addr & 0x7fff)];
		} else {
			return dict_ram[addr & 0x1fff];
		}
	} else if((addr >= 0xc2080000) && (addr < 0xc2100000)) {
		n_data = dict_rom[addr & 0x7ffff];
	} else if((addr >= 0xc2140000) && (addr < 0xc2142000)) {
		n_data = dict_ram[addr & 0x1fff];
	}
	return n_data;
}

void DICTIONARY::write_memory_mapped_io8w(uint32_t addr, uint32_t data, int *wait)
{
	if((addr < 0x000da000) && (addr >= 0x000d8000)) {
		*wait = 6; // Temporally.
		cmos_dirty = true;
		dict_ram[addr & 0x1fff] = data;
		return;
	} else if((addr >= 0xc2140000) && (addr < 0xc2142000)) {
		*wait = 6; // Temporally.
		cmos_dirty = true;
		dict_ram[addr & 0x1fff] = data;
		return;
	}
}



void DICTIONARY::write_io8w(uint32_t addr, uint32_t data, int *wait)
{
	*wait = 6; // Temporally.
	if(addr == 0x0484) {
		dict_bank = data & 0x0f;
	} else if((addr >= 0x3000) && (addr < 0x4000)) {
		cmos_dirty = true;
		dict_ram[((addr - 0x3000) >> 1) & 0x7ff] = (uint8_t)data;
	}
}

uint32_t DICTIONARY::read_io8w(uint32_t addr, int *wait)
{
	*wait = 6; // Temporally.
	uint32_t data = 0xff;
	if(addr == 0x0484) {
		data = dict_bank & 0x0f;
	} else if((addr >= 0x3000) && (addr < 0x4000)) {
		data = dict_ram[((addr - 0x3000) >> 1) & 0x07ff];
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
	return dict_ram[addr & 0x1fff];
}

void DICTIONARY::write_debug_data8(uint32_t addr, uint32_t data)
{
	// May read ram only
	dict_ram[addr & 0x1fff] = data;
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
				  _T("BANK=%02X WROTE=%s\n"),
				  dict_bank, (cmos_dirty) ? _T("Yes") : _T("No "));
	return true;
}

#define STATE_VERSION	1

bool DICTIONARY::process_state(FILEIO* state_fio, bool loading)
{
	if(!state_fio->StateCheckUint32(STATE_VERSION)) {
		return false;
	}
	if(!state_fio->StateCheckInt32(this_device_id)) {
		return false;
	}
	state_fio->StateValue(dict_bank);
	state_fio->StateArray(dict_ram, sizeof(dict_ram), 1);

	if(loading) {
		cmos_dirty = true;
	}/* else {
		FILEIO* fio = new FILEIO();
		if(fio->Fopen(create_local_path(_T("FMT_CMOS.BIN")), FILEIO_WRITE_BINARY)) {
			fio->Fwrite(dict_ram, sizeof(dict_ram), 1);
			fio->Fclose();
		}
		delete fio;
		cmos_dirty = false;
	} */
	return true;
}
}
