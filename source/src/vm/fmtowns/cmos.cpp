/*
	FUJITSU FM Towns Emulator 'eFMTowns'

	Author : Kyuma.Ohta <whatisthis.sowhat _at_ gmail.com>
	Date   : 2019.01.09-

	[ CMOS RAM area 0x000D8000h - 0x000D9FFFh , also C2140000h - C2141FFFh ]
	* MEMORY :
	*   0x000d8000 - 0x000d9fff : DICTIONARY RAM / GAIJI RAM
	*   0xc2140000 - 0xc2141fff : DICTIONARY RAM
	* I/O :
	*   0x3000 - 0x3ffe (even address) : DICTIONARY RAM
*/

#include "./towns_common.h"
#include "./cmos.h"
#include "../../fileio.h"
#include "../debugger.h"

namespace FMTOWNS {

void CMOS::initialize()
{
	memset(ram, 0x00, sizeof(ram));

	cmos_dirty = true;
	FILEIO* fio = new FILEIO();
	if(fio->Fopen(create_local_path(_T("FMT_CMOS.BIN")), FILEIO_READ_BINARY)) {
		if(fio->Fread(ram, sizeof(ram), 1) == 1) {
			cmos_dirty = false;
		}
		fio->Fclose();
	}
	delete fio;
}

void CMOS::release()
{
	save_ram();
}

void CMOS::save_ram()
{
	if(cmos_dirty) {
		FILEIO* fio = new FILEIO();
		if(fio->Fopen(create_local_path(_T("FMT_CMOS.BIN")), FILEIO_WRITE_BINARY)) {
			fio->Fwrite(ram, sizeof(ram), 1);
			fio->Fclose();
		}
		delete fio;
		cmos_dirty = false;
	}
}

void CMOS::reset()
{
	// Write CMOS every resetting. 20200511 K.O
	save_ram();
}

uint32_t CMOS::read_memory_mapped_io8(uint32_t addr)
{
	int dummywait;
	return read_memory_mapped_io8w(addr, &dummywait);
}

uint32_t CMOS::read_memory_mapped_io8w(uint32_t addr, int* wait)
{
	__LIKELY_IF(d_debugger != NULL) {
		__UNLIKELY_IF(d_debugger->now_device_debugging)	{
			return d_debugger->read_via_debugger_data8w(addr & 0x7fff, wait);
		}
	}
	return read_via_debugger_data8w(addr & 0x7fff, wait);
}

void CMOS::write_memory_mapped_io8(uint32_t addr, uint32_t data)
{
	int dummywait;
	write_memory_mapped_io8w(addr, data, &dummywait);
}

void CMOS::write_memory_mapped_io8w(uint32_t addr, uint32_t data, int* wait)
{
	__LIKELY_IF(d_debugger != NULL) {
		__UNLIKELY_IF(d_debugger->now_device_debugging)	{
			d_debugger->write_via_debugger_data8w(addr & 0x7fff, data, wait);
			return;
		}
	}
	write_via_debugger_data8w(addr & 0x7fff, data, wait);
}

uint32_t CMOS::read_via_debugger_data8(uint32_t addr)
{
	__LIKELY_IF(addr < 0x2000) {
		return ram[addr & 0x1fff];
	}
	return 0xff;
}

void CMOS::write_via_debugger_data8(uint32_t addr, uint32_t data)
{
	__LIKELY_IF(addr < 0x2000) {
		uint8_t val = ram[addr & 0x1fff];
		if(val != (data & 0xff)) {
			cmos_dirty = true;
			ram[addr & 0x1fff] = data;
		}
	}
}

uint32_t CMOS::read_via_debugger_data8w(uint32_t addr, int* wait)
{
	__LIKELY_IF(wait != NULL) {
		*wait = 0;
	}
	return read_via_debugger_data8(addr);
}

void CMOS::write_via_debugger_data8w(uint32_t addr, uint32_t data, int* wait)
{
	__LIKELY_IF(wait != NULL) {
		*wait = 0;
	}
	write_via_debugger_data8(addr, data);
}

uint32_t CMOS::read_dma_data8w(uint32_t addr, int* wait)
{
	return read_memory_mapped_io8w(addr, wait); // OK?
}

void CMOS::write_dma_data8w(uint32_t addr, uint32_t data, int* wait)
{
	write_memory_mapped_io8w(addr, data, wait); // OK?
}

void CMOS::write_io8(uint32_t addr, uint32_t data)
{
	 __LIKELY_IF((addr >= 0x3000) && (addr < 0x4000)) {
		__LIKELY_IF((addr & 1) == 0) {
			cmos_dirty = true;
			ram[((addr - 0x3000) >> 1) & 0x7ff] = (uint8_t)data;
		}
	}
}

uint32_t CMOS::read_io8(uint32_t addr)
{
	__LIKELY_IF((addr >= 0x3000) && (addr < 0x4000)) {
		__LIKELY_IF((addr & 1) == 0) {
			return ram[((addr - 0x3000) >> 1) & 0x07ff];
		}
	}
	return 0xff;
}

uint32_t CMOS::read_debug_data8(uint32_t addr)
{
	// May read ram only
	return ram[addr & 0x1fff];
}

void CMOS::write_debug_data8(uint32_t addr, uint32_t data)
{
	// May read ram only
	uint8_t val = ram[addr & 0x1fff];
	if(val != (uint8_t)(data & 0xff)) {
		cmos_dirty = true;
		ram[addr & 0x1fff] = data;
	}
}

bool CMOS::get_debug_regs_info(_TCHAR *buffer, size_t buffer_len)
{
	if(buffer == NULL) return false;
	if(buffer_len <= 1) return false;
	my_stprintf_s(buffer, buffer_len - 1,
				  _T("CMOS WROTE: %s\n"),
				  (cmos_dirty) ? _T("DIRTY") : _T("CLEAN"));
	return true;
}

#define STATE_VERSION	1

bool CMOS::process_state(FILEIO* state_fio, bool loading)
{
	if(!state_fio->StateCheckUint32(STATE_VERSION)) {
		return false;
	}
	if(!state_fio->StateCheckInt32(this_device_id)) {
		return false;
	}
	state_fio->StateArray(ram, sizeof(ram), 1);
	if(loading) {
		cmos_dirty = true;
	}
	return true;
}
}
