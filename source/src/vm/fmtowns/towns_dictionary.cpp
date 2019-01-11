/*
	FUJITSU FM Towns Emulator 'eFMTowns'

	Author : Kyuma.Ohta <whatisthis.sowhat _at_ gmail.com>
	Date   : 2019.01.09-

	[ dictionary rom/ram & cmos & RAM area 0x000d0000 - 0x000effff]
*/

#include "./towns_common.h"
#include "./towns_dictrom.h"
#include "./towns_sysrom.h"
#include "../../fileio.h"

namespace FMTOWNS {
void DICTIONARY::initialize()
{
	cmos_dirty = false;
	
	memset(dict_rom, 0xff, sizeof(dict_rom));
	memset(dict_ram, 0x00, sizeof(dict_ram));
	memset(ram_0d0, 0x00, sizeof(ram_0d0));
	
	FILEIO* fio = new FILEIO();
	if(fio->Fopen(create_local_path(_T("FMT_DIC.ROM")), FILEIO_READ_BINARY)) { // DICTIONARIES
		fio->Fread(dict_rom, sizeof(dict_rom), 1);
		fio->Fclose();
	}
	
	if(fio->Fopen(create_local_path(_T("FMT_CMOS.BIN")), FILEIO_READ_BINARY)) {
		fio->Fread(dict_ram, sizeof(dict_ram), 1);
		fio->Fclose();
	} else {
		cmos_dirty = true;
	}
	delete fio;

	dict_bank = 0;
	bankd0_dict = false;
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
	bankd0_dict = false;
	
}
uint32_t DICTIONARY::read_data8(uint32_t addr)
{
	uint8_t n_data = 0xff;
	if((addr < 0x000f0000) && (addr >= 0x000d0000)) {
		if(bankd0_dict) {
			if(addr < 0x000d8000) {
				n_data = dict_rom[(addr & 0x7fff) + (((uint32_t)(dict_bank & 0x0f)) << 15)];
			} else if(addr < 0x000da000) {
				n_data = dict_ram[addr & 0x1fff];
			}/* else { // ToDo: Check correctness
				n_data = 0xff;
			}*/
		} else {
			n_data = ram_0d0[addr & 0x1ffff];
		}
	} else if((addr >= 0xc20800000) && (addr < 0xc2100000)) {
		n_data = dict_rom[addr & 0x7ffff];
	} else if((addr >= 0xc21400000) && (addr < 0xc2142000)) {
		n_data = dict_ram[addr & 0x1fff];
	}
	return n_data;
}

void DICTIONARY::write_data8(uint32_t addr, uint32_t data)
{
	if((addr < 0x000f0000) && (addr >= 0x000d0000)) {
		if(bankd0_dict) {
			if((addr >= 0x000d8000) && (addr < 0x000da000)) {
				cmos_dirty = true;
				dict_ram[addr & 0x1fff] = data; // ToDo: Check correctness
			} /* else { // ToDo: Check correctness
			
			}*/
		} else {
			ram_0d0[addr & 0x1ffff] = (uint8_t)data;
		}
	} else if((addr >= 0xc21400000) && (addr < 0xc2142000)) {
		dict_ram[addr & 0x1fff] = (uint8_t)data;
	}
}

uint32_t DICTIONARY::read_data16(uint32_t addr)
{
	pair16_t n;
	addr = addr & 0xfffffffe;
	n.l = (uint8_t)read_data8(addr + 0);
	n.h = (uint8_t)read_data8(addr + 1);
	return (uint32_t)(n.u16);
}

uint32_t DICTIONARY::read_data32(uint32_t addr)
{
	pair32_t n;
	addr = addr & 0xfffffffc;
	n.l  = (uint8_t)read_data8(addr + 0);
	n.h  = (uint8_t)read_data8(addr + 1);
	n.h2 = (uint8_t)read_data8(addr + 2);
	n.h3 = (uint8_t)read_data8(addr + 3);
	return n.u32;
}

void DICTIONARY::write_data16(uint32_t addr, uint32_t data)
{
	pair16_t n;
	addr = addr & 0xfffffffe;
	n.u16 = (uint16_t)data;
	write_data8(addr + 0, n.l);
	write_data8(addr + 1, n.h);
}

void DICTIONARY::write_data32(uint32_t addr, uint32_t data)
{
	pair32_t n;
	addr = addr & 0xfffffffc;
	n.u32 = data;
	write_data8(addr + 0, n.l);
	write_data8(addr + 1, n.h);
	write_data8(addr + 2, n.h2);
	write_data8(addr + 3, n.h3);
}


void DICTIONARY::write_io8(uint32_t addr, uint32_t data)
{
	if(addr == 0x0480) {
		bankd0_dict = ((data & 0x01) != 0);
		d_sysrom->write_signal(SIG_FMTONWS_SYSROMSEL, data, 0x02);
	} else if(addr == 0x0484) {
		dict_bank = data & 0x0f;
	} else if((addr >= 0x3000) && (addr < 0x4000)) {
		if((addr & 0x0001) == 0) { // OK?
			uint32_t naddr = (addr >> 1) & 0x7ff;
			cmos_dirty = true;
			dict_ram[naddr] = (uint8_t)data;
		}
	}
}

uint32_t DICTIONARY::read_io8(uint32_t addr)
{
	uint32_t data;
	if(addr == 0x0480) {
		data = ((bank0_dict) ? 0x01 : 0x00) | ((d_sysrom->read_signal(SIG_FMTOWNS_SYSROMSEL) == 0) ? 0x02 : 0x00);
	} else if(addr == 0x0484) {
		data = dict_bank & 0x0f;
	} else if((addr >= 0x3000) && (addr < 0x4000)) {
		if((addr & 0x0001) == 0) { // OK?
			uint32_t naddr = (addr >> 1) & 0x7ff;
			data = dict_ram[naddr];
		} else {
			data = 0xff;
		}
	} else {
		data = 0xff;
	}
	return data;
}

void DICTIONARY::write_signal(int ch, uint32_t data, uint32_t mask)
{
	switch(ch) {
	case SIG_FMTOWNS_DICTSEL:
		bankd0_dict = ((data & mask) != 0);
		break;
	case SIG_FMTOWNS_DICTBANK:
		dict_bank = (uint8_t)(data & 0x0f);
		break;
	}
}

uint32_t DICTIONARY::read_signal(int ch)
{
	switch(ch) {
	case SIG_FMTOWNS_DICTSEL:
		return ((bankd0_dict) ? 0xffffffff : 0x00000000);
		break;
	case SIG_FMTOWNS_DICTBANK:
		return (uint32_t)(dict_bank & 0x0f);
		break;
	}
	return 0x00;
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
	state_fio->StateValue(bankd0_dict);
	state_fio->StateArray(dict_ram, sizeof(dict_ram), 1);
	state_fio->StateArray(ram_0d0, sizeof(ram_0d0), 1);
	
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
