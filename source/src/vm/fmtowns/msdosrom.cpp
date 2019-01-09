/*
	FUJITSU FM Towns Emulator 'eFMTowns'

	Author : Kyuma.Ohta <whatisthis.sowhat _at_ gmail.com>
	Date   : 2019.01.09 -

	[MSDOS ROM]
*/

#include "./towns_common.h"
#include "./msdosrom.h"

namespace FMTOWNS {
	
void MSDOSROM::initialize()
{
	memset(rom, 0xff, sizeof(rom));
	FILEIO *fio;
	if(fio->Fopen(create_local_path(_T("FMT_DOS.ROM")), FILEIO_READ_BINARY)) { // MSDOS
		fio->Fread(rom, sizeof(rom), 1);
		fio->Fclose();
	}
	delete fio;

	wait_val = 3;
}

uint32_t MSDOSROM::read_data8(uint32_t addr)	
{
	uint8_t d = 0xff;
	if((addr >= 0xc2000000) && (addr < 0xc2080000)) {
		d = rom[addr & 0x7ffff];
	}
	return (uint32_t)d;
}

uint32_t MSDOSROM::read_data8w(uint32_t addr, int* wait)	
{
	if(wait != NULL) *wait = wait_val;
	return read_data8(addr);
}
void MSDOSROM::write_data8w(uint32_t addr, uint32_t data, int* wait)	
{
	if(wait != NULL) *wait = wait_val;
}

void MSDOSROM::write_signal(int ch, uint32_t data, uint32_t mask)
{
	switch(ch) {
	case SIG_FMTOWNS_SET_MEMWAIT:
		wait_val = (int)data;
		break;
	}
}

uint32_t MSDOSROM::read_signal(int ch)
{
	switch(ch) {
	case SIG_FMTOWNS_SET_MEMWAIT:
		return (uint32_t)wait_val;
		break;
	}
	return 0;
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
	state_fio->StateValue(wait_val);
	return true;
}

}
